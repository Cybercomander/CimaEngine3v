#include "FasesPelea.hpp"
#include <Round12/Componentes/ComponentesR12.hpp>
#include <Round12/Escenas/Escena_Ring.hpp>
#include <Round12/Logica/Eventos.hpp>
#include <Round12/Maquinas/NodosR12.hpp>
#include <Round12/objetos/Hud.hpp>
#include <Round12/Sistemas/SistemasR12.hpp>
#include <Round12/objetos/FabricaR12.hpp>
#include <algorithm>

namespace Round12
{
    namespace
    {
        void cartel(sf::RenderTarget& rt, sf::Vector2f tam, const std::string& s, float y, float alto,
                sf::Color c = sf::Color::White)
        {
            auto txt = texto(s,(unsigned)(tam.y*alto),c,{0.f,tam.y*y});
            centrarX(txt,tam.x/2.f);
            rt.draw(txt);
        }
        void velo(sf::RenderTarget& rt, sf::Vector2f tam, std::uint8_t a = 150)
        {
            sf::RectangleShape r{tam};
            r.setFillColor(sf::Color(0,0,0,a));
            rt.draw(r);
        }
        IBoxeador* box(IVJ::Entidad& e) { return e.getComponente<IBoxeador>(); }
        void emociones(Escena_Ring& r, const char* diego, const char* rival)
        {
            r.emo_fase[0] = diego;
            r.emo_fase[1] = rival;
        }
    }

    // ---- Presentación ----
    void Presentacion::entrar(Escena_Ring& r)
    {
        r.colocarEnEsquinas();
        r.reloj = r.cfg.segundos;
        Eventos::Get().emitir(Evento::CAMPANA);
        r.mandarCarlos(false);
        emociones(r,"hablando","hablando");
    }
    void Presentacion::actualizar(Escena_Ring& r, float dt)
    {
        t += dt;
        // primero miran a la cámara; después se encaran
        if(t < 1.2f)
            for(auto* e : {r.diego.get(),r.rival.get()})
                e->getComponente<IVJ::IDireccion>()->dir = ADELANTE;
        else
        {
            SistemaMirarse(*r.diego,*r.rival);
            emociones(r,"decidido","decidido");
        }
        r.diego->onUpdate(0.f);
        r.rival->onUpdate(0.f);
        if(t >= 2.f) r.cambiarFase(std::make_unique<EnRound>());
    }
    void Presentacion::dibujar(Escena_Ring& r, sf::RenderTarget& rt, sf::Vector2f tam)
    {
        cartel(rt,tam,"ROUND " + std::to_string(r.round_actual),0.35f,0.12f,COLOR_CONCENTRACION);
        cartel(rt,tam,std::string("Diego  vs  ") + nombre(r.cfg.rival),0.52f,0.04f);
    }

    // ---- Round ----
    void EnRound::actualizar(Escena_Ring& r, float dt)
    {
        t += dt;
        if(t > 1.5f) emociones(r,"",""); // "decidido" solo al empezar el round
        r.simular(dt);
        r.reloj -= dt;
        if(r.caida_pendiente)
        {
            bool d = r.caida_pendiente == 1;
            r.caida_pendiente = 0;
            if(d && r.combate->round().caidas_diego >= 3)
            {
                r.cambiarFase(std::make_unique<FinPelea>(false,false,"Nocaut técnico: tres caídas en un round"));
                return;
            }
            r.cambiarFase(std::make_unique<Caida>(d));
            return;
        }
        if(r.reloj > 0.f) return;
        Eventos::Get().emitir(Evento::CAMPANA);
        if(r.round_actual >= r.cfg.rounds) r.cambiarFase(std::make_unique<Decision>());
        else r.cambiarFase(std::make_unique<Esquina>());
    }

    // ---- Caída y conteo ----
    void Caida::entrar(Escena_Ring& r)
    {
        auto& quien = es_diego ? *r.diego : *r.rival;
        quien.setFSM(std::make_shared<CaidoR12>());
        r.combate->caida(es_diego);
        int previas = es_diego ? r.caidas_diego++ : r.caidas_rival++;
        conteo = Conteo{previas};
        if(!es_diego)
        {
            std::uniform_real_distribution<float> u(0.f,1.f);
            se_levanta = u(r.rng) < probLevantarRival(r.combate->atributos(false),previas);
            levanta_en = 6 + (int)(u(r.rng)*3.f);
        }
        // el que sigue de pie se va a la esquina neutral (deja de golpear)
        auto& otro = es_diego ? *r.rival : *r.diego;
        otro.setFSM(std::make_shared<IdleR12>());
        r.soltarControles();
        emociones(r,es_diego ? "sorprendido" : "celebrando",es_diego ? "celebrando" : "sorprendido");
    }
    void Caida::actualizar(Escena_Ring& r, float dt)
    {
        conteo.tick(dt);
        if(es_diego && r.piloto) conteo.pulsar(conteo.validas() % 2 ? 'K' : 'J');
        r.diego->onUpdate(dt);
        r.rival->onUpdate(dt);
        auto& quien = es_diego ? *r.diego : *r.rival;
        auto& b = r.combate->barras(es_diego);
        bool arriba = es_diego ? conteo.levantado() : (se_levanta && conteo.numero() >= levanta_en);
        if(arriba)
        {
            b.resistencia = b.resistencia_max*0.4f;
            box(quien)->barras = b;
            quien.setFSM(std::make_shared<LevantarseR12>());
            emociones(r,"","");
            r.cambiarFase(std::make_unique<EnRound>());
            return;
        }
        if(conteo.fuera())
            r.cambiarFase(std::make_unique<FinPelea>(!es_diego,!es_diego,
                        es_diego ? "Nocaut: Diego no se levantó" : "¡Nocaut! Diego gana la pelea"));
    }
    void Caida::tecla(Escena_Ring&, const std::string& a, bool p)
    {
        if(!p || !es_diego) return;
        if(a == "jab") conteo.pulsar('J');
        if(a == "recto") conteo.pulsar('K');
    }
    void Caida::dibujar(Escena_Ring&, sf::RenderTarget& rt, sf::Vector2f tam)
    {
        cartel(rt,tam,std::to_string(conteo.numero()),0.25f,0.2f,sf::Color(250,90,90));
        if(es_diego)
        {
            cartel(rt,tam,"¡Levántate! Presiona J y K alternadas",0.5f,0.045f,COLOR_CONCENTRACION);
            sf::RectangleShape fondo{{tam.x*0.4f,tam.y*0.03f}}, lleno{{tam.x*0.4f*std::min(1.f,
                (float)conteo.validas()/conteo.requeridas()),tam.y*0.03f}};
            fondo.setPosition({tam.x*0.3f,tam.y*0.58f});
            lleno.setPosition(fondo.getPosition());
            fondo.setFillColor(sf::Color(30,30,30,200));
            lleno.setFillColor(COLOR_AIRE);
            rt.draw(fondo);
            rt.draw(lleno);
        }
        else cartel(rt,tam,"El rival está en la lona",0.5f,0.045f);
    }

    // ---- Esquina entre rounds ----
    void Esquina::entrar(Escena_Ring& r)
    {
        r.combate->descanso();
        // la toalla se usa sola en cada descanso: quita el oscurecimiento (GDD)
        auto& b = r.combate->barras(true);
        b.concentracion = std::max(b.concentracion,60.f);
        r.soltarControles();
        // cada quien a su esquina; Carlos sube a hablar con Diego
        r.diego->setFSM(std::make_shared<IdleR12>());
        r.rival->setFSM(std::make_shared<IdleR12>());
        r.diego->setPosicion(r.esc->esquina_diego.x,r.esc->esquina_diego.y);
        r.rival->setPosicion(r.esc->esquina_rival.x,r.esc->esquina_rival.y);
        r.diego->getComponente<IVJ::IDireccion>()->dir = IZQUIERDA;
        r.rival->getComponente<IVJ::IDireccion>()->dir = DERECHA;
        r.mandarCarlos(true);
        emociones(r,"escuchando","cansado");
    }
    void Esquina::actualizar(Escena_Ring& r, float dt)
    {
        t += dt;
        r.diego->onUpdate(dt);
        r.rival->onUpdate(dt);
        if(t < 10.f && !(elegido && t > 1.5f)) return;
        r.combate->nuevoRound();
        r.round_actual++;
        r.cambiarFase(std::make_unique<Presentacion>());
    }
    void Esquina::tecla(Escena_Ring& r, const std::string& a, bool p)
    {
        if(!p || elegido) return;
        auto& b = r.combate->barras(true);
        if(a == "jab") { b.sed = 0.f; eleccion = "Tomaste agua: la sed bajó por completo"; elegido = true; }
        if(a == "recto")
        {
            b.concentracion = std::min(100.f,b.concentracion + 30.f);
            r.consejo = 10.f;
            eleccion = "Carlos: «Fíjate en sus hombros, te voy a decir qué viene»";
            elegido = true;
        }
        if(elegido) { t = 0.f; Eventos::Get().emitir(Evento::ITEM); }
    }
    void Esquina::dibujar(Escena_Ring&, sf::RenderTarget& rt, sf::Vector2f tam)
    {
        velo(rt,tam,120);
        cartel(rt,tam,"ESQUINA",0.25f,0.08f,COLOR_CONCENTRACION);
        cartel(rt,tam,"La toalla ya te quitó el sudor de los ojos",0.37f,0.035f);
        if(elegido) { cartel(rt,tam,eleccion,0.5f,0.04f,COLOR_AIRE); return; }
        cartel(rt,tam,"J: tomar agua (baja la sed)",0.5f,0.045f);
        cartel(rt,tam,"K: escuchar a Carlos (sube la concentración)",0.58f,0.045f);
        cartel(rt,tam,std::to_string(10 - (int)t) + " s",0.7f,0.04f,sf::Color(200,200,200));
    }

    // ---- Decisión de los jueces ----
    void Decision::entrar(Escena_Ring& r)
    {
        v = r.combate->veredicto(r.rng,&tarjetas);
        r.soltarControles();
        bool gana = ganaDiego(v) || !r.cfg.jueces;
        emociones(r,gana ? "celebrando" : "cansado",gana ? "cansado" : "celebrando");
    }
    void Decision::actualizar(Escena_Ring& r, float dt)
    {
        t += dt;
        r.diego->onUpdate(dt);
        r.rival->onUpdate(dt);
        if(!listo && t < 5.f) return;
        bool gano = ganaDiego(v);
        r.cambiarFase(std::make_unique<FinPelea>(gano,true,r.cfg.jueces ? texto(v) :
                    std::string("Sparring terminado: sigues de pie")));
    }
    void Decision::tecla(Escena_Ring&, const std::string& a, bool p)
    {
        if(p && (a == "jab" || a == "aceptar") && t > 1.f) listo = true;
    }
    void Decision::dibujar(Escena_Ring& r, sf::RenderTarget& rt, sf::Vector2f tam)
    {
        velo(rt,tam);
        if(!r.cfg.jueces) { cartel(rt,tam,"¡Tiempo!",0.35f,0.1f,COLOR_CONCENTRACION); return; }
        cartel(rt,tam,"DECISIÓN DE LOS JUECES",0.2f,0.06f,COLOR_CONCENTRACION);
        // en Mexicali las tarjetas se levantan sobre la mesa de jueces pintada
        bool mesa = r.esc->carpeta.find("mexicali") != std::string::npos;
        for(int j = 0; j < 3; ++j)
        {
            std::string linea = "Juez " + std::to_string(j+1) + ":  Diego " + std::to_string(tarjetas.diego[j]) +
                "  -  " + std::to_string(tarjetas.rival[j]) + " " + nombre(r.cfg.rival);
            if(!mesa) { cartel(rt,tam,linea,0.35f + j*0.08f,0.04f); continue; }
            sf::RectangleShape tarjeta{{tam.x*0.1f,tam.y*0.11f}};
            tarjeta.setPosition({tam.x*(0.36f + j*0.1f),tam.y*0.70f});
            tarjeta.setFillColor(sf::Color(245,240,225));
            tarjeta.setOutlineColor(sf::Color(160,30,40));
            tarjeta.setOutlineThickness(3.f);
            rt.draw(tarjeta);
            auto n = texto(std::to_string(tarjetas.diego[j]) + "-" + std::to_string(tarjetas.rival[j]),
                    (unsigned)(tam.y*0.045f),sf::Color(30,30,30),{0.f,tam.y*0.725f});
            n.setOutlineThickness(0.f);
            centrarX(n,tam.x*(0.41f + j*0.1f));
            rt.draw(n);
        }
        if(t > 2.f) cartel(rt,tam,texto(v),0.66f,0.05f,ganaDiego(v) ? COLOR_AIRE : sf::Color(250,120,120));
    }

    // ---- Fin ----
    void FinPelea::actualizar(Escena_Ring& r, float dt)
    {
        if(t == 0.f)
        {
            bool bien = gano && r.cfg.objetivo != Objetivo::PERDER_FORZADO;
            emociones(r,bien || (de_pie && r.cfg.objetivo == Objetivo::SOBREVIVIR) ? "celebrando" : "cansado",
                      gano ? "cansado" : "celebrando");
        }
        t += dt;
        r.diego->onUpdate(dt);
        r.rival->onUpdate(dt);
        if(t >= 5.f && !hecho) { hecho = true; r.terminar(gano,de_pie,como); }
    }
    void FinPelea::tecla(Escena_Ring& r, const std::string& a, bool p)
    {
        if(p && (a == "jab" || a == "aceptar") && t > 1.f && !hecho) { hecho = true; r.terminar(gano,de_pie,como); }
    }
    void FinPelea::dibujar(Escena_Ring& r, sf::RenderTarget& rt, sf::Vector2f tam)
    {
        velo(rt,tam);
        bool exito = r.cfg.objetivo == Objetivo::PERDER_FORZADO ||
            (r.cfg.objetivo == Objetivo::GANAR ? gano : de_pie);
        std::string titulo = r.cfg.objetivo == Objetivo::PERDER_FORZADO ? "Ricardo fue demasiado" :
            exito ? (r.cfg.objetivo == Objetivo::GANAR ? "¡VICTORIA!" : "¡AGUANTASTE!") : "DERROTA";
        cartel(rt,tam,titulo,0.3f,0.1f,exito ? COLOR_CONCENTRACION : sf::Color(250,90,90));
        cartel(rt,tam,como,0.47f,0.045f);
        cartel(rt,tam,exito ? "J: continuar" : "J: intentar de nuevo",0.8f,0.035f,sf::Color(200,200,200));
    }
}
