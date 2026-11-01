#include "Escena_Runner.hpp"
#include <Juego/Componentes/IJComponentes.hpp>
#include <Motor/Render/Render.hpp>
#include <Round12/JuegoR12.hpp>
#include <Round12/Logica/Eventos.hpp>
#include <Round12/Maquinas/NodosR12.hpp>
#include <Round12/objetos/FabricaR12.hpp>
#include <algorithm>
#include <cmath>
#include <random>

namespace Round12
{
    namespace
    {
        constexpr float PISTA = 1500.f; // largo de una vuelta del circuito
        std::string tiempoTexto(float s)
        {
            int t = std::max(0,(int)std::ceil(s));
            return std::to_string(t/60) + ":" + (t%60 < 10 ? "0" : "") + std::to_string(t%60);
        }
    }

    void Escena_Runner::reiniciar()
    {
        cfg = runner(id);
        Runner::Config rc;
        rc.meta = cfg.meta;
        rc.tiempo = cfg.tiempo;
        rc.mojada = cfg.olas;
        run = std::make_unique<Runner>(rc);
        run->checkpoint();
        diego = FabricaR12::boxeador(Personaje::DIEGO,0.f,SUELO,IVJ::IDireccion::DERECHA,std::make_shared<CaminarR12>());
        auto* b = diego->getComponente<IBoxeador>();
        b->atr = Partida::Get().diego();
        b->jugador = true;
        generar();
        sig_puesto = 0;
        vueltas.clear();
        inicio_vuelta = transcurrido = t_ola = aviso_t = 0.f;
        terminado = false;
        hud.limpiar();
        hud.agregar(std::make_unique<Panel>(sf::FloatRect{{0.f,0.f},{1.f,0.16f}},sf::Color(0,0,0,140)));
        static const float CIEN = 100.f;
        hud.agregar(std::make_unique<Barra>("AIRE",&run->aire,&CIEN,COLOR_AIRE,sf::Vector2f{0.14f,0.04f},0.3f));
        hud.agregar(std::make_unique<Barra>("SED",&run->sed,&CIEN,COLOR_SED,sf::Vector2f{0.14f,0.08f},0.3f));
        hud.agregar(std::make_unique<Etiqueta>([this]{ return "Tiempo " + tiempoTexto(run->tiempo); },
                    sf::Vector2f{0.6f,0.03f},0.04f,COLOR_CONCENTRACION));
        hud.agregar(std::make_unique<Etiqueta>([this]{
                    if(cfg.vueltas)
                        return "Vuelta " + std::to_string(std::min(cfg.vueltas,(int)vueltas.size()+1)) + "/" + std::to_string(cfg.vueltas);
                    float x = run->x;
                    std::string tramo = x < 3000 ? "Tramo 1: arena seca" : x < 6000 ? "Tramo 2: orilla del mar" : "Tramo 3: rumbo al muelle";
                    return tramo + "  -  " + std::to_string((int)std::max(0.f,(cfg.meta - x)/8.f)) + " m";
                    },sf::Vector2f{0.6f,0.09f},0.03f));
    }

    // obstáculos y objetos con semilla fija: el nivel es igual en cada intento
    void Escena_Runner::generar()
    {
        cosas.clear();
        sombras.clear();
        puestos.clear();
        std::mt19937 rng{cfg.semilla};
        std::uniform_real_distribution<float> u(0.f,1.f);
        if(cfg.vueltas)
            for(int v = 1; v < cfg.vueltas; ++v) puestos.push_back(v*PISTA);
        else
            puestos = {3000.f,6000.f};
        float x = 250.f;
        while(x < cfg.meta - 150.f)
        {
            float tramo = cfg.vueltas ? 1 : (x < 3000 ? 1 : x < 6000 ? 2 : 3);
            float sep = tramo == 1 ? 300.f : tramo == 2 ? 220.f : 150.f;
            if(cfg.vueltas) sep = 260.f;
            x += sep*(0.7f + 0.6f*u(rng));
            bool cerca_puesto = std::any_of(puestos.begin(),puestos.end(),[x](float p){ return std::abs(p - x) < 60.f; });
            if(cerca_puesto) continue;
            TipoObstaculo t;
            if(tramo == 3) t = (TipoObstaculo)((int)TipoObstaculo::PELOTA + (int)(u(rng)*3) % 3);
            else t = (TipoObstaculo)((int)(u(rng)*3) % 3);
            Cosa c{Rect{x,0.f,12.f,14.f},FabricaR12::obstaculo(t,x,SUELO)};
            if(auto* o = c.e->getComponente<IObstaculo>()) c.vel = o->vel;
            cosas.push_back(c);
            // isotónica escondida arriba de algunas hieleras (hay que saltar)
            if(t == TipoObstaculo::HIELERA && u(rng) < 0.5f)
            {
                Cosa iso{Rect{x - 6.f,44.f,12.f,12.f},FabricaR12::item(TipoItem::ISOTONICA,x,pantallaY(44.f))};
                iso.item = true; iso.tipo_item = TipoItem::ISOTONICA;
                cosas.push_back(iso);
            }
        }
        for(float p : {1200.f,5200.f})
            if(p < cfg.meta)
            {
                Cosa pl{Rect{p - 6.f,0.f,12.f,12.f},FabricaR12::item(TipoItem::PLATANO,p,SUELO)};
                pl.item = true; pl.tipo_item = TipoItem::PLATANO;
                cosas.push_back(pl);
            }
        for(float s = 700.f; s < cfg.meta; s += 900.f)
            sombras.push_back(Rect{s,0.f,48.f,0.f});
    }

    bool Escena_Runner::enSombra() const
    {
        return std::any_of(sombras.begin(),sombras.end(),[this](const Rect& s){ return run->x > s.x && run->x < s.x + s.w; });
    }

    void Escena_Runner::piloto()
    {
        control.der = run->aire > 55.f;
        control.izq = run->aire < 25.f;
        control.arr = control.abj = false;
        for(auto& c : cosas)
            if(c.activo && !c.item && c.r.x - run->x > 0.f && c.r.x - run->x < 26.f) control.arr = true;
        if(t_ola > 3.6f && t_ola < 4.2f) control.arr = true; // salta la ola
    }

    void Escena_Runner::actualizar(float dt)
    {
        if(terminado) return;
        if(JuegoR12::prueba.autopiloto) piloto();
        transcurrido += dt;
        aviso_t -= dt;
        int ritmo = control.der ? 1 : (control.izq ? -1 : 0);
        run->paso(dt,ritmo,control.arr,control.abj,enSombra());

        for(auto& c : cosas)
        {
            if(!c.activo) continue;
            if(c.vel > 0.f && c.r.x - run->x < 220.f) c.r.x -= c.vel*dt; // perros, cangrejos y pelotas
            if(c.item)
            {
                bool toca = std::abs(c.r.x + c.r.w/2.f - run->x) < 12.f && run->y + run->alto() > c.r.y && run->y < c.r.y + c.r.h;
                if(!toca) continue;
                c.activo = false;
                if(c.tipo_item == TipoItem::ISOTONICA) { run->aire = std::min(100.f,run->aire + 30.f); run->sed = std::max(0.f,run->sed - 40.f); aviso = "¡Bebida isotónica! Aire y sed"; }
                else { run->aire = std::min(100.f,run->aire + 20.f); aviso = "Plátano: recuperas fuerzas"; }
                aviso_t = 2.f;
                Eventos::Get().emitir(Evento::ITEM);
            }
            else if(run->chocaCon(c.r))
                Eventos::Get().emitir(Evento::GOLPE_CONECTADO);
        }
        // olas en la orilla: llegan cada 6 s y duran 2 s
        if(cfg.olas && run->x >= 3000.f && run->x < 6000.f)
        {
            t_ola += dt;
            if(t_ola >= 6.f) { t_ola = 0.f; if(run->ola()) { aviso = "¡Te alcanzó la ola!"; aviso_t = 1.5f; } }
        }
        else t_ola = 0.f;
        // puestos de agua: beber y guardar el punto de control
        if(sig_puesto < puestos.size() && run->x >= puestos[sig_puesto])
        {
            run->beber();
            run->checkpoint();
            sig_puesto++;
            aviso = cfg.vueltas ? "Vuelta completa: agua y punto de control" : "Puesto de agua: sed a cero y punto de control";
            aviso_t = 2.5f;
            if(cfg.vueltas) { vueltas.push_back(transcurrido - inicio_vuelta); inicio_vuelta = transcurrido; }
            Eventos::Get().emitir(Evento::ITEM);
        }
        if(run->fallo())
        {
            aviso = run->aire <= 0.f ? "Te quedaste sin aire: regresas al último puesto de agua" :
                "Se acabó el tiempo: regresas al último puesto de agua";
            aviso_t = 3.f;
            run->reaparecer();
            for(auto& c : cosas)
                if(!c.item && c.vel > 0.f) c.activo = true;
        }
        if(run->llego()) { terminar(); return; }

        diego->setPosicion(run->x,pantallaY(run->y) - FabricaR12::PIES_A_CENTRO + (run->agachado ? 10.f : 0.f));
        auto* ctrl = diego->getComponente<CE::IControl>();
        ctrl->der = true;
        diego->inputFSM();
        diego->onUpdate(dt*run->vel/60.f);
        moverCamara(run->x + 70.f,96.f);
    }

    void Escena_Runner::terminar()
    {
        terminado = true;
        auto& p = Partida::Get();
        if(cfg.vueltas)
        {
            vueltas.push_back(transcurrido - inicio_vuelta);
            bool rapidas = std::all_of(vueltas.begin(),vueltas.end(),[this](float v){ return v <= cfg.meta_vuelta; });
            if(rapidas) p.mejorar(&Atributos::velocidad,1);
            p.resumen = "Circuito: " + std::to_string(cfg.vueltas) + " vueltas" + (rapidas ? " bajo el tiempo meta" : "");
        }
        else
        {
            if(run->tiempo >= 60.f) { p.mejorar(&Atributos::aire,2); p.mejorar(&Atributos::resistencia,1); }
            else p.mejorar(&Atributos::aire,1);
            p.desbloquear(Habilidad::SEGUNDO_AIRE);
            p.resumen = "Llegaste al muelle con " + tiempoTexto(run->tiempo) + " de sobra";
        }
        p.historia().avanzar();
        irAPaso();
    }

    void Escena_Runner::dibujarMundo()
    {
        auto& r = CE::Render::Get();
        float cam = run->x + 70.f;
        float x0 = cam - 144.f;
        const auto& m = mapa(cfg.mapa);
        float w = m.getDimension().x;
        for(float bx = std::floor(x0/w)*w; bx < x0 + 288.f; bx += w)
        {
            sf::RenderStates st;
            st.transform.translate({bx,0.f});
            r.AddToDraw(m,st);
        }
        if(!cfg.vueltas)
        {
            // parallax de la playa: cielo, mar, cerros y palmeras a distintas velocidades
            const std::pair<const char*,float> capas[] = {{"cielo",0.f},{"cerros",0.2f},{"mar",0.4f},{"palmeras",0.7f}};
            for(auto& [capa,f] : capas)
            {
                auto& tex = textura(std::string("provisionales/parallax/") + capa + ".png");
                float off = std::fmod(cam*f,288.f);
                for(int i = 0; i < 2; ++i)
                {
                    sf::Sprite s{tex,sf::IntRect{{0,0},{288,136}}};
                    s.setPosition({x0 - off + i*288.f,0.f});
                    r.AddToDraw(s);
                }
            }
        }
        for(auto& s : sombras)
            if(s.x > x0 - 60.f && s.x < x0 + 300.f)
            {
                sf::Sprite sp{textura("provisionales/sprites/sombra.png")};
                sp.setPosition({s.x,SUELO - 6.f});
                r.AddToDraw(sp);
            }
        auto dibujaFijo = [&](const char* archivo, float x, float pies){
            sf::Sprite s{textura(std::string("provisionales/sprites/") + archivo)};
            auto t = s.getTexture().getSize();
            s.setOrigin({t.x/2.f,(float)t.y});
            s.setPosition({x,pies});
            r.AddToDraw(s);
        };
        for(size_t i = sig_puesto; i < puestos.size(); ++i)
            if(puestos[i] > x0 - 30.f && puestos[i] < x0 + 320.f)
                dibujaFijo(cfg.vueltas ? "garrafon.png" : "puesto_agua.png",puestos[i] + 20.f,SUELO);
        if(cfg.meta - x0 < 340.f)
        {
            if(cfg.vueltas) { sf::RectangleShape meta{{4.f,40.f}}; meta.setPosition({cfg.meta,SUELO - 40.f}); r.AddToDraw(meta); }
            else dibujaFijo("muelle.png",cfg.meta + 30.f,SUELO + 4.f);
        }
        for(auto& c : cosas)
        {
            if(!c.activo || c.r.x < x0 - 30.f || c.r.x > x0 + 320.f) continue;
            auto* s = c.e->getComponente<CE::ISprite>();
            if(c.vel > 0.f && s->m_sprite.getTexture().getSize().x == 32)
                s->m_sprite.setTextureRect({{16*((int)(transcurrido*6.f) % 2),0},{16,16}});
            s->m_sprite.setPosition({c.r.x + c.r.w/2.f,pantallaY(c.r.y)});
            r.AddToDraw(*c.e);
        }
        if(cfg.olas && t_ola > 4.f)
        {
            sf::Sprite ola{textura("provisionales/sprites/ola.png")};
            ola.setPosition({run->x - 32.f + (6.f - t_ola)*20.f,SUELO - 12.f});
            r.AddToDraw(ola);
        }
        r.AddToDraw(*diego);
    }

    void Escena_Runner::dibujarPantalla(sf::RenderTarget& t, sf::Vector2f tam)
    {
        if(aviso_t > 0.f)
        {
            auto a = texto(aviso,(unsigned)(tam.y*0.035f),COLOR_CONCENTRACION,{0.f,tam.y*0.2f});
            centrarX(a,tam.x/2.f);
            t.draw(a);
        }
        if(cfg.olas && t_ola > 4.f)
        {
            auto a = texto("¡Ahí viene una ola! (W para saltar)",(unsigned)(tam.y*0.03f),COLOR_AIRE,{0.f,tam.y*0.26f});
            centrarX(a,tam.x/2.f);
            t.draw(a);
        }
        if(transcurrido < 4.f)
        {
            auto a = texto("D acelera (gasta aire) · A frena (recupera) · W salta · S se agacha",(unsigned)(tam.y*0.03f),
                    sf::Color::White,{0.f,tam.y*0.9f});
            centrarX(a,tam.x/2.f);
            t.draw(a);
        }
    }
}
