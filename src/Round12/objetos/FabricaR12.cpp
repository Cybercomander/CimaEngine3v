#include "FabricaR12.hpp"
#include <Motor/Primitivos/GestorAssets.hpp>
#include <Motor/Primitivos/CEPool.hpp>
#include <Juego/Componentes/IJComponentes.hpp>
#include <Round12/Rutas.hpp>
#include <iostream>
#include <filesystem>
#include <map>
#include <set>

namespace Round12
{
    sf::Texture& texturaFaltante()
    {
        static sf::Texture t{sf::Image({8,8},sf::Color::Magenta)};
        t.setRepeated(true);
        return t;
    }

    sf::Texture& textura(const std::string& relativa, bool suave)
    {
        static std::set<std::string> cargadas;
        auto& g = CE::GestorAssets::Get();
        std::string k = "r12g_" + relativa;
        static std::set<std::string> faltantes;
        if(faltantes.count(relativa)) return texturaFaltante();
        if(cargadas.insert(relativa).second)
        {
            std::string ruta = std::string(R12_RECURSOS) + "/" + relativa;
            // GestorAssets cierra el programa si falta el archivo; mejor un cuadro magenta
            if(!std::filesystem::exists(ruta))
            {
                std::cerr<<"[r12] falta "<<ruta<<std::endl;
                faltantes.insert(relativa);
                cargadas.erase(relativa);
                return texturaFaltante();
            }
            // área vacía = imagen completa
            g.agregarTextura(k,ruta,CE::Vector2D{0,0},CE::Vector2D{0,0});
            g.getTextura(k).setSmooth(suave);
        }
        return g.getTextura(k);
    }

    const IVJ::TileMap& mapa(const std::string& nombre)
    {
        static std::map<std::string,std::unique_ptr<IVJ::TileMap>> mapas;
        auto& m = mapas[nombre];
        if(!m)
        {
            m = std::make_unique<IVJ::TileMap>();
            // formato con 'n' por tile: solo lo lee la versión con pool (no agrega nada)
            static CE::Pool sin_colision{1};
            if(!m->loadTileMap(std::string(R12_RECURSOS) + "/motor/" + nombre + ".txt",sin_colision))
                std::cerr<<"[round12] no se pudo cargar el mapa "<<nombre<<"\n";
        }
        return *m;
    }

    void aplicarCuadro(IVJ::Entidad& e)
    {
        auto* a = e.getComponente<IAnimR12>();
        auto* s = e.getComponente<CE::ISprite>();
        if(!a || !s) return;
        // cambia de hoja solo cuando hace falta (la textura ya está cargada)
        auto& tex = textura("motor/hojas/" + a->hoja + (a->dialogo ? "_dialogo.png" : ".png"));
        if(&s->m_sprite.getTexture() != &tex) s->m_sprite.setTexture(tex);
        auto r = a->dialogo ? rectDialogo(a->emocion,a->dir,a->cuadro % 4) : rectCuadro(a->accion,a->dir,a->cuadro);
        s->m_sprite.setTextureRect(sf::IntRect{{r.x,r.y},{r.w,r.h}});
    }

    void dibujarCapa(sf::RenderTarget& t, const std::string& relativa)
    {
        sf::Sprite s{textura(relativa,true)};
        auto tam = s.getTexture().getSize();
        s.setScale({ANCHO_MUNDO/tam.x, ALTO_MUNDO/tam.y});
        t.draw(s);
    }

    namespace FabricaR12
    {
        namespace
        {
            std::shared_ptr<IVJ::Entidad> personaje(Personaje p, float x, float pies_y, int dir, sf::Color tinte)
            {
                auto ente = std::make_shared<IVJ::Entidad>();
                auto& tex = textura(std::string("motor/hojas/") + hoja(p) + ".png");
                textura(std::string("motor/hojas/") + hoja(p) + "_dialogo.png");
                auto sprite = std::make_shared<CE::ISprite>(tex,ESCALA_PERSONAJE);
                sprite->width = sprite->height = TAM_CUADRO;
                sprite->m_sprite.setTextureRect(sf::IntRect{{0,0},{TAM_CUADRO,TAM_CUADRO}});
                sprite->m_sprite.setOrigin({40.f,69.f});
                sprite->m_sprite.setScale({ESCALA_PERSONAJE,ESCALA_PERSONAJE}); // la izquierda ya viene espejeada
                sprite->m_sprite.setColor(tinte);
                auto direccion = std::make_shared<IVJ::IDireccion>();
                direccion->dir = dir;
                auto anim = std::make_shared<IAnimR12>();
                anim->dir = dir;
                anim->hoja = hoja(p);
                ente->addComponente(sprite).addComponente(direccion).addComponente(anim)
                    .addComponente(std::make_shared<CE::IControl>());
                ente->setPosicion(x,pies_y);
                return ente;
            }
        }

        std::shared_ptr<IVJ::Entidad> boxeador(Personaje p, float x, float pies_y, int dir,
                const std::shared_ptr<IVJ::FSM>& inicial, sf::Color tinte)
        {
            auto ente = personaje(p,x,pies_y,dir,tinte);
            auto box = std::make_shared<IBoxeador>();
            box->quien = p;
            box->atr = perfil(p);
            box->barras = barrasIniciales(box->atr);
            box->tinte = tinte;
            auto me = std::make_shared<IVJ::IMaquinaEstado>();
            me->fsm = inicial;
            ente->addComponente(box).addComponente(me);
            ente->setFSM(me->fsm);
            ente->onUpdate(0.f);
            aplicarCuadro(*ente);
            return ente;
        }

        std::shared_ptr<IVJ::Entidad> npc(Personaje p, float x, float pies_y, int dir)
        {
            auto ente = personaje(p,x,pies_y,dir,sf::Color::White);
            ente->onUpdate(0.f);
            aplicarCuadro(*ente);
            return ente;
        }

        sf::Sprite spriteObjeto(const std::string& objeto, Pt pie)
        {
            sf::Sprite s{textura("objetos/" + objeto + ".png")};
            s.setOrigin({48.f,120.f});
            s.setScale({ESCALA_OBJETO,ESCALA_OBJETO});
            s.setPosition({pie.x,pie.y});
            return s;
        }

        std::shared_ptr<NodoSprite> prop(const PropR12& p)
        {
            return std::make_shared<NodoSprite>(spriteObjeto(p.objeto,p.pie),p.pie.y);
        }

        std::shared_ptr<NodoDibujo> nodo(const std::shared_ptr<IVJ::Entidad>& e)
        {
            std::weak_ptr<IVJ::Entidad> w = e;
            return std::make_shared<NodoFuncion>(
                [w]{ auto e = w.lock(); return e ? e->getTransformada()->posicion.y : -1e9f; },
                [w](sf::RenderTarget& t){ if(auto e = w.lock()) t.draw(*e); });
        }

        std::shared_ptr<NodoSprite> cuerdas(const EscenarioR12& e)
        {
            sf::Sprite s{textura(rutaCapa(e,"primer_plano"),true)};
            s.setScale({ANCHO_MUNDO/s.getTexture().getSize().x, ALTO_MUNDO/s.getTexture().getSize().y});
            return std::make_shared<NodoSprite>(s,e.ring_frente_y);
        }

        namespace
        {
            std::shared_ptr<IVJ::Entidad> simple(const std::string& rel, sf::IntRect r, float x, float pies_y)
            {
                auto e = std::make_shared<IVJ::Entidad>();
                auto s = std::make_shared<CE::ISprite>(textura(rel),1.f);
                s->width = r.size.x;
                s->height = r.size.y;
                s->m_sprite.setTextureRect(r);
                s->m_sprite.setOrigin({r.size.x/2.f,(float)r.size.y});
                e->addComponente(s)
                    .addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{(float)r.size.x,(float)r.size.y}));
                e->setPosicion(x,pies_y);
                e->onUpdate(0.f);
                return e;
            }
        }

        std::shared_ptr<IVJ::Entidad> decorado(const std::string& relativa, sf::IntRect rect, float x, float pies_y)
        {
            return simple(relativa,rect,x,pies_y);
        }

        std::shared_ptr<IVJ::Entidad> item(TipoItem t, float x, float y)
        {
            static const char* archivos[] = {"botella","isotonica","platano","toalla"};
            auto e = simple(std::string("provisionales/sprites/") + archivos[(int)t] + ".png",{{0,0},{16,16}},x,y);
            e->addComponente(std::make_shared<IItem>(t));
            return e;
        }

        std::shared_ptr<IVJ::Entidad> obstaculo(TipoObstaculo t, float x, float y)
        {
            static const char* archivos[] = {"castillo","hielera","sombrilla","pelota","perro","cangrejo"};
            auto e = simple(std::string("provisionales/sprites/") + archivos[(int)t] + ".png",{{0,0},{16,16}},x,y);
            auto o = std::make_shared<IObstaculo>(t);
            if(t == TipoObstaculo::PERRO) o->vel = 30.f;
            if(t == TipoObstaculo::CANGREJO) o->vel = 12.f;
            if(t == TipoObstaculo::PELOTA) o->vel = 20.f;
            e->addComponente(o);
            return e;
        }
    }
}
