#include "Escena_Runner.hpp"
#include <Juego/Componentes/IJComponentes.hpp>
#include <Juego/Maquinas/Boxeadores/NodosBoxeo.hpp>
#include <Motor/Render/Render.hpp>
#include <Round12/JuegoR12.hpp>
#include <Round12/Logica/Eventos.hpp>
#include <Round12/Logica/LineaDialogo.hpp>
#include <Round12/Rutas.hpp>
#include <Round12/Sistemas/SistemasR12.hpp>
#include <Round12/objetos/FabricaR12.hpp>
#include <algorithm>
#include <cmath>
#include <random>

namespace Round12
{
    namespace
    {
        constexpr float PISTA = 1500.f;     // largo de una vuelta del circuito (px de arte)
        constexpr float K = ESCALA_MUNDO;   // px de arte -> px del escenario
        constexpr float SUELO = 930.f;      // la arena de la tira de la playa, en el escenario
        constexpr float ESC_ISO = ESCALA_OBJETO;
        const Pt ORIGEN_ISO{960.f,150.f};
        // línea central de la pista ovalada (escenarios/06_circuito_atletico_rosarito)
        const std::vector<Pt> OVALO = {{300,318},{1620,318},{1780,380},{1850,500},{1860,640},{1850,780},
                                       {1780,900},{1620,965},{300,965},{140,900},{70,780},{60,640},
                                       {70,500},{140,380}};
        std::string tiempoTexto(float s)
        {
            int t = std::max(0,(int)std::ceil(s));
            return std::to_string(t/60) + ":" + (t%60 < 10 ? "0" : "") + std::to_string(t%60);
        }
        Pt centroCelda(int c, int f) { Pt p = celdaIso(c,f,ORIGEN_ISO,ESC_ISO); return {p.x,p.y + 16.f*ESC_ISO}; }
    }

    Pt Escena_Runner::alMundo(float x, float altura) const
    {
        if(circuito)
        {
            Pt p = pista->punto(std::fmod(x,PISTA)/PISTA*pista->largo());
            return {p.x,p.y - altura*K};
        }
        return {x*K,SUELO - altura*K};
    }

    void Escena_Runner::reiniciar()
    {
        cfg = runner(id);
        circuito = cfg.vueltas > 0;
        pista = std::make_unique<Camino>(OVALO);
        Runner::Config rc;
        rc.meta = cfg.meta;
        rc.tiempo = cfg.tiempo;
        rc.mojada = cfg.olas;
        run = std::make_unique<Runner>(rc);
        run->checkpoint();
        Pt p0 = alMundo(0.f,0.f);
        diego = FabricaR12::npc(Personaje::DIEGO,p0.x,p0.y,DERECHA);
        carlos = FabricaR12::npc(Personaje::CARLOS,0.f,0.f,IZQUIERDA);
        generar();
        sig_puesto = 0;
        vueltas.clear();
        inicio_vuelta = transcurrido = t_ola = aviso_t = golpe_t = choque_t = 0.f;
        terminado = en_final = false;
        moverCamara(960.f,540.f);
        hud.limpiar();
        agregarBandas(hud);
        static const float CIEN = 100.f;
        hud.agregar(std::make_unique<Barra>("AIRE",&run->aire,&CIEN,COLOR_AIRE,sf::Vector2f{0.10f,0.012f},0.27f));
        hud.agregar(std::make_unique<Barra>("SED",&run->sed,&CIEN,COLOR_SED,sf::Vector2f{0.10f,0.045f},0.27f));
        hud.agregar(std::make_unique<Retrato>([this]{
                    return std::string("sprites/personajes/diego_ureta/retratos/") +
                        (choque_t > 0.f ? "sorprendido" : run->aire < 25.f ? "cansado" : golpe_t > 0.f ? "decidido" : "neutral") + ".png"; },
                    sf::Vector2f{4.f/1080.f,4.f/720.f},48.f));
        hud.agregar(std::make_unique<Etiqueta>([this]{ return "Tiempo " + tiempoTexto(run->tiempo); },
                    sf::Vector2f{0.5f,0.006f},0.034f,COLOR_CONCENTRACION,true));
        hud.agregar(std::make_unique<Etiqueta>([this]{
                    if(circuito)
                        return "Vuelta " + std::to_string(std::min(cfg.vueltas,(int)vueltas.size()+1)) + "/" + std::to_string(cfg.vueltas);
                    float x = run->x;
                    std::string tramo = x < 3000 ? "Tramo 1: arena seca" : x < 6000 ? "Tramo 2: orilla del mar" : "Tramo 3: rumbo al muelle";
                    return tramo + "  -  " + std::to_string((int)std::max(0.f,(cfg.meta - x)/8.f)) + " m";
                    },sf::Vector2f{0.75f,0.02f},0.028f,sf::Color::White,true));
        hud.agregar(std::make_unique<Etiqueta>([]{
                    return std::string("D acelera · A frena · W salta · S se agacha · J K L I rompen cajas");
                    },sf::Vector2f{0.5f,0.94f},0.022f,sf::Color(190,190,190),true));
    }

    // obstáculos y objetos con semilla fija: el nivel es igual en cada intento
    void Escena_Runner::generar()
    {
        cosas.clear();
        sombras.clear();
        puestos.clear();
        adornos.clear();
        std::mt19937 rng{cfg.semilla};
        std::uniform_real_distribution<float> u(0.f,1.f);
        auto obstaculo = [&](TipoChoque t, float x, const std::string& obj, bool prov = false, float vel = 0.f){
            Cosa c;
            c.r = obstaculoRunner(t,x);
            c.choque = t;
            c.objeto = obj;
            c.provisional = prov;
            c.vel = vel;
            cosas.push_back(c);
        };
        if(circuito)
        {
            // cada vuelta: 4 vallas (se saltan), 2 conos (se rompen) y una banderola (agachado)
            for(int v = 0; v < cfg.vueltas; ++v)
            {
                float b = v*PISTA;
                if(v > 0) puestos.push_back(b);
                for(float f : {0.15f,0.4f,0.65f,0.9f}) obstaculo(TipoChoque::BAJO,b + f*PISTA,"valla");
                for(float f : {0.28f,0.78f}) obstaculo(TipoChoque::ROMPIBLE,b + f*PISTA,"cono");
                obstaculo(TipoChoque::ALTO,b + 0.53f*PISTA,"banderola");
            }
            return;
        }
        puestos = {3000.f,6000.f};
        float x = 250.f;
        while(x < cfg.meta - 150.f)
        {
            int tramo = x < 3000 ? 1 : x < 6000 ? 2 : 3;
            float sep = tramo == 1 ? 300.f : tramo == 2 ? 220.f : 150.f;
            x += sep*(0.7f + 0.6f*u(rng));
            bool cerca_puesto = std::any_of(puestos.begin(),puestos.end(),[x](float p){ return std::abs(p - x) < 60.f; });
            if(cerca_puesto) continue;
            float r = u(rng);
            if(tramo == 3 && r < 0.5f)
            {
                // en el último tramo hay animales y pelotas que se mueven
                const char* m[] = {"pelota","perro","cangrejo"};
                const float v[] = {20.f,30.f,12.f};
                int i = (int)(u(rng)*3) % 3;
                obstaculo(TipoChoque::BAJO,x,m[i],true,v[i]);
                continue;
            }
            if(r < 0.25f) obstaculo(TipoChoque::BAJO,x,u(rng) < 0.5f ? "cono" : "banca");
            else if(r < 0.45f) obstaculo(TipoChoque::ALTO,x,"banderola");
            else
            {
                obstaculo(TipoChoque::ROMPIBLE,x,u(rng) < 0.6f ? "caja_madera" : "bote_basura");
                // isotónica arriba de algunas cajas (hay que saltar)
                if(u(rng) < 0.35f)
                {
                    Cosa iso;
                    iso.r = Rect{x - 6.f,44.f,12.f,12.f};
                    iso.item = true; iso.tipo_item = TipoItem::ISOTONICA;
                    iso.objeto = "isotonica"; iso.provisional = true;
                    cosas.push_back(iso);
                }
            }
        }
        for(float p : {1200.f,5200.f})
            if(p < cfg.meta)
            {
                Cosa pl;
                pl.r = Rect{p - 6.f,0.f,12.f,12.f};
                pl.item = true; pl.tipo_item = TipoItem::PLATANO;
                pl.objeto = "platano"; pl.provisional = true;
                cosas.push_back(pl);
            }
        for(float s = 700.f; s < cfg.meta; s += 900.f) sombras.push_back(Rect{s,0.f,48.f,0.f});
        for(float a = 450.f; a < cfg.meta; a += 1250.f) adornos.push_back(a);
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
        {
            if(!c.activo || c.item) continue;
            float d = c.r.x - run->x;
            if(c.choque == TipoChoque::ROMPIBLE && run->alcanzaGolpe(c.r) && d > 5.f && golpe_t <= 0.f) alPresionar("jab",true);
            else if(c.choque == TipoChoque::ALTO && d > -20.f && d < 30.f) control.abj = true;
            else if(c.choque == TipoChoque::BAJO && d > 0.f && d < 26.f) control.arr = true;
        }
        if(t_ola > 3.6f && t_ola < 4.2f) control.arr = true; // salta la ola
    }

    void Escena_Runner::alPresionar(const std::string& a, bool p)
    {
        if(!p) return;
        if(en_final) { if(a == "jab" || a == "aceptar") paseo_i = paseo.size(); return; }
        if(golpe_t > 0.f) return;
        Brazo b = a == "jab" ? Brazo::JAB : a == "recto" ? Brazo::RECTO : a == "gancho" ? Brazo::GANCHO :
            a == "uppercut" ? Brazo::UPPERCUT : Brazo::NADA;
        if(b == Brazo::NADA) return;
        brazo = b;
        golpe_t = 0.45f;
        run->golpear();
        for(auto& c : cosas)
            if(c.activo && c.choque == TipoChoque::ROMPIBLE && !c.item && run->alcanzaGolpe(c.r))
            {
                c.activo = false;
                aviso = "¡Lo quitaste del camino!";
                aviso_t = 1.f;
                Eventos::Get().emitir(Evento::GOLPE_FUERTE);
                break;
            }
    }

    // postura del corredor + brazo = animación del paquete (saltar_jab, agacharse_gancho, caminar_recto...)
    void Escena_Runner::animarDiego(float dt)
    {
        auto* a = diego->getComponente<IAnimR12>();
        Postura p = !run->enSuelo() ? Postura::SALTANDO : run->agachado ? Postura::AGACHADO : Postura::CAMINANDO;
        Brazo b = golpe_t > 0.f ? brazo : (control.botones & IVJ::BotonBox::GUARDIA) ? Brazo::GUARDIA : Brazo::NADA;
        Accion acc = choque_t > 0.f ? Accion::RECIBIR_GOLPE : elegirAccion(p,b);
        a->poner(acc);
        a->actualizar(dt*std::max(0.5f,run->vel/Runner::RITMO));
        if(p == Postura::SALTANDO && b == Brazo::NADA)
            a->cuadro = std::clamp((int)((run->y/32.f)*3.f) + (run->vy < 0.f ? 3 : 0),0,CUADROS-1);
        if(circuito)
        {
            Pt t = pista->tangente(std::fmod(run->x,PISTA)/PISTA*pista->largo());
            a->dir = direccionHacia(t.x,t.y);
        }
        else a->dir = DERECHA;
        Pt w = alMundo(run->x,run->y);
        diego->setPosicion(w.x,w.y);
        diego->onUpdate(0.f);
        aplicarCuadro(*diego);
    }

    void Escena_Runner::actualizar(float dt)
    {
        if(terminado) return;
        if(en_final) { actualizarFinal(dt); return; }
        if(JuegoR12::prueba.autopiloto) piloto();
        transcurrido += dt;
        aviso_t -= dt;
        golpe_t = std::max(0.f,golpe_t - dt);
        choque_t = std::max(0.f,choque_t - dt);
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
            else if(run->chocaCon(c.r)) { choque_t = 0.4f; Eventos::Get().emitir(Evento::GOLPE_CONECTADO); }
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
            aviso = circuito ? "Vuelta completa: agua y punto de control" : "Puesto de agua: sed a cero y punto de control";
            aviso_t = 2.5f;
            if(circuito) { vueltas.push_back(transcurrido - inicio_vuelta); inicio_vuelta = transcurrido; }
            Eventos::Get().emitir(Evento::ITEM);
        }
        if(run->fallo())
        {
            aviso = run->aire <= 0.f ? "Te quedaste sin aire: regresas al último puesto de agua" :
                "Se acabó el tiempo: regresas al último puesto de agua";
            aviso_t = 3.f;
            run->reaparecer();
            for(auto& c : cosas)
                if(!c.item) c.activo = true;
        }
        animarDiego(dt);
        if(!circuito) moverCamara(std::max(960.f,run->x*K + 450.f),540.f);
        if(run->llego())
        {
            if(circuito) { terminar(); return; }
            // la meta es el muelle: Diego camina por la zona isométrica hasta Carlos
            en_final = true;
            final_iso.cargar(std::string(R12_DATOS) + "/iso/playa_final.txt");
            paseo = {centroCelda(9,10),centroCelda(6,7),centroCelda(6,4),centroCelda(4,4),centroCelda(6,6)};
            paseo_i = 1;
            diego->setPosicion(paseo[0].x,paseo[0].y);
            Pt pc = centroCelda(7,5);
            carlos->setPosicion(pc.x,pc.y);
            moverCamara(960.f,540.f);
            aviso = "¡Llegaste al muelle!";
            aviso_t = 3.f;
        }
    }

    // zona del muelle: un paseo en las 4 direcciones hasta Carlos (J lo salta)
    void Escena_Runner::actualizarFinal(float dt)
    {
        aviso_t -= dt;
        auto* a = diego->getComponente<IAnimR12>();
        auto& pd = diego->getTransformada()->posicion;
        if(paseo_i < paseo.size())
        {
            Pt m = paseo[paseo_i];
            float dx = m.x - pd.x, dy = m.y - pd.y, d = std::hypot(dx,dy);
            if(d < 6.f) paseo_i++;
            else
            {
                float v = std::min(d,280.f*dt);
                pd.x += dx/d*v; pd.y += dy/d*v;
                a->dir = direccionHacia(dx,dy);
                a->dialogo = false;
                a->poner(Accion::CAMINAR);
                a->actualizar(dt);
            }
        }
        else
        {
            a->hablar(indiceEmocion("celebrando"),dt);
            static float espera = 0.f;
            espera += dt;
            if(espera > 1.5f || JuegoR12::prueba.autopiloto) { espera = 0.f; terminar(); return; }
        }
        auto* ac = carlos->getComponente<IAnimR12>();
        ac->dir = direccionHacia(pd.x - carlos->getTransformada()->posicion.x,0.f);
        ac->hablar(indiceEmocion(paseo_i < paseo.size() ? "hablando" : "sonriendo"),dt);
        for(auto* e : {diego.get(),carlos.get()}) { e->onUpdate(0.f); aplicarCuadro(*e); }
    }

    void Escena_Runner::terminar()
    {
        terminado = true;
        auto& p = Partida::Get();
        if(circuito)
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

    void Escena_Runner::dibujarCosa(sf::RenderTarget& t, const Cosa& c) const
    {
        Pt w = alMundo(c.r.x + c.r.w/2.f, c.choque == TipoChoque::ALTO && !c.item ? 0.f : c.r.y);
        if(!c.provisional) { t.draw(FabricaR12::spriteObjeto(c.objeto,w)); return; }
        sf::Sprite s{textura("provisionales/sprites/" + c.objeto + ".png"),sf::IntRect{{0,0},{16,16}}};
        if(c.vel > 0.f && s.getTexture().getSize().x == 32)
            s.setTextureRect({{16*((int)(transcurrido*6.f) % 2),0},{16,16}});
        s.setOrigin({8.f,16.f});
        s.setScale({K,K});
        s.setPosition({w.x,w.y});
        t.draw(s);
    }

    void Escena_Runner::dibujarMundo()
    {
        auto& t = CE::Render::Get().GetTextura();
        if(en_final)
        {
            dibujarCapa(t,"escenarios/05_playa_rosarito/05_playa_rosarito_zoom.png");
            sf::RectangleShape velo{{1920.f,1080.f}};
            velo.setFillColor(sf::Color(10,40,60,150));
            t.draw(velo);
            final_iso.dibujar(t,ORIGEN_ISO,ESC_ISO);
            ListaDibujo l;
            Pt torre = centroCelda(9,3);
            l.agregar(std::make_shared<NodoSprite>(FabricaR12::spriteObjeto("torre_salvavidas",torre),torre.y));
            Pt surf = centroCelda(10,6);
            l.agregar(std::make_shared<NodoSprite>(FabricaR12::spriteObjeto("surf",surf),surf.y));
            l.agregar(FabricaR12::nodo(carlos));
            l.agregar(FabricaR12::nodo(diego));
            l.dibujar(t);
            return;
        }
        if(circuito)
        {
            const auto& esc = escenario("circuito");
            dibujarCapa(t,rutaCapa(esc,"decorado"));
            ListaDibujo l;
            for(const auto& pr : esc.props) l.agregar(FabricaR12::prop(pr));
            float base = std::floor(run->x/PISTA)*PISTA;
            for(const auto& c : cosas)
                if(c.activo && c.r.x >= base && c.r.x < base + PISTA)
                    l.agregar(std::make_shared<NodoFuncion>([this,&c]{ return alMundo(c.r.x,0.f).y; },
                                [this,&c](sf::RenderTarget& rt){ dibujarCosa(rt,c); }));
            l.agregar(FabricaR12::nodo(diego));
            l.dibujar(t);
            dibujarCapa(t,rutaCapa(esc,"primer_plano"));
            return;
        }
        // playa: la tira de Rosarito repetida (1920x384 escalada a 1080 de alto)
        float cam = std::max(960.f,run->x*K + 450.f), x0 = cam - 960.f;
        auto& tira = textura("escenarios/05_playa_rosarito/05_playa_rosarito_zoom.png",true);
        float esc = 1080.f/tira.getSize().y, w = tira.getSize().x*esc;
        for(float bx = std::floor(x0/w)*w; bx < x0 + 1920.f; bx += w)
        {
            sf::Sprite s{tira};
            s.setScale({esc,esc});
            s.setPosition({bx,0.f});
            t.draw(s);
        }
        auto visible = [&](float xa){ float xw = xa*K; return xw > x0 - 300.f && xw < x0 + 2200.f; };
        for(float a : adornos)
            if(visible(a)) t.draw(FabricaR12::spriteObjeto(((int)a/1250) % 2 ? "surf" : "torre_salvavidas",{a*K,SUELO - 90.f}));
        for(auto& s : sombras)
            if(visible(s.x)) t.draw(FabricaR12::spriteObjeto("sombrilla",{(s.x + s.w/2.f)*K,SUELO - 20.f}));
        for(size_t i = sig_puesto; i < puestos.size(); ++i)
            if(visible(puestos[i]))
            {
                t.draw(FabricaR12::spriteObjeto("fuente_agua",{puestos[i]*K + 60.f,SUELO - 30.f}));
                t.draw(FabricaR12::spriteObjeto("bandera",{puestos[i]*K - 60.f,SUELO - 30.f}));
            }
        for(auto& c : cosas)
            if(c.activo && visible(c.r.x)) dibujarCosa(t,c);
        if(cfg.olas && t_ola > 4.f)
        {
            sf::Sprite ola{textura("provisionales/sprites/ola.png")};
            ola.setScale({K,K});
            ola.setPosition({(run->x - 32.f + (6.f - t_ola)*20.f)*K,SUELO - 12.f*K});
            t.draw(ola);
        }
        t.draw(*diego);
    }

    void Escena_Runner::dibujarPantalla(sf::RenderTarget& t, sf::Vector2f tam)
    {
        if(aviso_t > 0.f)
        {
            auto a = texto(aviso,(unsigned)(tam.y*0.035f),COLOR_CONCENTRACION,{0.f,tam.y*0.13f});
            centrarX(a,tam.x/2.f);
            t.draw(a);
        }
        if(cfg.olas && t_ola > 4.f && !en_final)
        {
            auto a = texto("¡Ahí viene una ola! (W para saltar)",(unsigned)(tam.y*0.03f),COLOR_AIRE,{0.f,tam.y*0.19f});
            centrarX(a,tam.x/2.f);
            t.draw(a);
        }
    }
}
