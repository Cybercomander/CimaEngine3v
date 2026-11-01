#include "FabricaR12.hpp"
#include <Motor/Primitivos/GestorAssets.hpp>
#include <Juego/Componentes/IJComponentes.hpp>
#include <Round12/Rutas.hpp>
#include <set>

namespace Round12
{
    sf::Texture& textura(const std::string& relativa)
    {
        static std::set<std::string> cargadas;
        auto& g = CE::GestorAssets::Get();
        std::string k = "r12g_" + relativa;
        if(cargadas.insert(relativa).second)
        {
            // área vacía = imagen completa; GestorAssets termina con mensaje si falta
            g.agregarTextura(k,std::string(R12_RECURSOS) + "/" + relativa,CE::Vector2D{0,0},CE::Vector2D{0,0});
            g.getTextura(k).setSmooth(false);
        }
        return g.getTextura(k);
    }

    namespace FabricaR12
    {
        std::shared_ptr<IVJ::Entidad> boxeador(Personaje p, float x, float pies_y, int dir,
                const std::shared_ptr<IVJ::FSM>& inicial, sf::Color tinte)
        {
            auto ente = std::make_shared<IVJ::Entidad>();
            auto& tex = textura(std::string("ROUND12_Sprites_Animaciones/hojas/") + hoja(p) + ".png");
            auto sprite = std::make_shared<CE::ISprite>(tex,1.f);
            sprite->width = 80;
            sprite->height = 80;
            sprite->m_sprite.setTextureRect(sf::IntRect{{0,0},{80,80}});
            sprite->m_sprite.setOrigin({40.f,44.f});
            sprite->m_sprite.setColor(tinte);
            auto direccion = std::make_shared<IVJ::IDireccion>();
            direccion->dir = dir;
            auto box = std::make_shared<IBoxeador>();
            box->quien = p;
            box->atr = perfil(p);
            box->barras = barrasIniciales(box->atr);
            box->tinte = tinte;
            auto me = std::make_shared<IVJ::IMaquinaEstado>();
            me->fsm = inicial;
            ente->addComponente(sprite)
                .addComponente(direccion)
                .addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{14.f,52.f}))
                .addComponente(std::make_shared<CE::IControl>())
                .addComponente(box)
                .addComponente(me);
            ente->setPosicion(x,pies_y-PIES_A_CENTRO);
            ente->setFSM(me->fsm);
            ente->onUpdate(0.f);
            return ente;
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
