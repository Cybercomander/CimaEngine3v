#include "Escena_Recorrido.hpp"
#include <Juego/Componentes/IJComponentes.hpp>
#include <Juego/Maquinas/Boxeadores/NodosBoxeo.hpp>
#include <Motor/Render/Render.hpp>
#include <Round12/JuegoR12.hpp>
#include <Round12/Logica/Eventos.hpp>
#include <Round12/Logica/Ritmo.hpp>
#include <Round12/Maquinas/NodosR12.hpp>
#include <Round12/Rutas.hpp>
#include <Round12/Sistemas/SistemasR12.hpp>
#include <Round12/objetos/Audio.hpp>
#include <Round12/objetos/FabricaR12.hpp>
#include <algorithm>
#include <cmath>

namespace Round12
{
    namespace
    {
        constexpr float PISO0 = 168.f, PISO1 = 186.f; // pies sobre el piso frente al ring
        constexpr float PARED = 520.f;                // el ring se abre al terminar las estaciones
        const std::vector<std::vector<int>> SECUENCIAS = {{1,1,2},{1,2,3},{3,4}};
        const char TECLAS[] = {'J','K','L','I'};
        char teclaDe(const std::string& a)
        {
            if(a == "jab") return 'J';
            if(a == "recto") return 'K';
            if(a == "gancho") return 'L';
            if(a == "uppercut") return 'I';
            return 0;
        }
        int numero(char t) { return t == 'J' ? 1 : t == 'K' ? 2 : t == 'L' ? 3 : t == 'I' ? 4 : 0; }
    }

    void Escena_Recorrido::reiniciar()
    {
        cfg = recorrido(id);
        auto& p = Partida::Get();
        ancho = cfg.mapa.rfind("03",0) == 0 ? 800.f : 640.f;
        diego = FabricaR12::boxeador(Personaje::DIEGO,40.f,178.f,IVJ::IDireccion::DERECHA,std::make_shared<IdleR12>());
        auto* b = diego->getComponente<IBoxeador>();
        b->atr = p.diego();
        b->jugador = true;
        b->puede_esquivar = false;
        barras = barrasIniciales(p.diego());
        carlos = FabricaR12::decorado("sprites/personajes/carlos_sanchez/spritesheet_npc.png",{{88,0},{44,60}},300.f,186.f);
        puestos.clear();
        for(auto e : cfg.estaciones)
        {
            switch(e)
            {
                case Estacion::COSTAL:
                    puestos.push_back({e,160.f,FabricaR12::decorado("provisionales/sprites/costal.png",{{0,0},{16,40}},160.f,170.f),false,"el costal"});
                    break;
                case Estacion::MANOPLAS:
                    puestos.push_back({e,280.f,FabricaR12::decorado("provisionales/sprites/manoplas.png",{{0,0},{16,16}},288.f,160.f),false,"las manoplas"});
                    break;
                case Estacion::CUERDA:
                    puestos.push_back({e,400.f,FabricaR12::decorado("provisionales/sprites/cuerda.png",{{0,0},{32,16}},400.f,190.f),false,"la cuerda"});
                    break;
                case Estacion::AGUA:
                    puestos.push_back({e,480.f,FabricaR12::decorado("provisionales/sprites/garrafon.png",{{0,0},{16,24}},480.f,180.f),false,"el garrafón"});
                    break;
            }
        }
        platano = cfg.platano ? FabricaR12::item(TipoItem::PLATANO,70.f,184.f) : nullptr;
        hud.limpiar();
        hud.agregar(std::make_unique<Panel>(sf::FloatRect{{0.f,0.f},{1.f,0.2f}},sf::Color(0,0,0,140)));
        agregarBarras(hud,barras,false,"Diego");
        mini = Mini::NINGUNO;
        activo = nullptr;
        sig_dialogo = 0;
        terminado = false;
        aviso.clear();
        en_dialogo = !cfg.dialogo_inicio.empty() &&
            dialogo.cargar(std::string(R12_DATOS) + "/dialogos/" + cfg.dialogo_inicio + ".txt");
        letras = 0.f;
    }

    Escena_Recorrido::Puesto* Escena_Recorrido::cerca()
    {
        float x = diego->getTransformada()->posicion.x;
        for(auto& p : puestos)
            if(std::abs(p.x - x) < 24.f && (!p.hecho || p.e == Estacion::AGUA)) return &p;
        return nullptr;
    }

    bool Escena_Recorrido::listoParaRing() const
    {
        return std::all_of(puestos.begin(),puestos.end(),[](const Puesto& p){ return p.hecho || p.e == Estacion::AGUA; });
    }

    void Escena_Recorrido::empezar(Puesto& p)
    {
        if(p.e == Estacion::AGUA)
        {
            barras.sed = 0.f;
            p.hecho = true;
            Eventos::Get().emitir(Evento::ITEM);
            mensaje("Tomaste agua del garrafón: la sed bajó por completo");
            return;
        }
        activo = &p;
        t_mini = 0.f;
        progreso = reps = pos_seq = 0;
        ultimo_pulso = -1;
        mostrando = true;
        soltarControles();
        if(p.e == Estacion::COSTAL) mini = Mini::COSTAL;
        if(p.e == Estacion::MANOPLAS) mini = Mini::MANOPLAS;
        if(p.e == Estacion::CUERDA) { mini = Mini::CUERDA; Audio::musica("musica_cuerda"); }
    }

    void Escena_Recorrido::terminarMini(bool exito)
    {
        auto& p = Partida::Get();
        if(!exito)
        {
            mensaje("Carlos: «Otra vez, desde el principio»");
            if(mini == Mini::CUERDA) { t_mini = 0.f; progreso = 0; ultimo_pulso = -1; Audio::musica("musica_cuerda"); }
            return;
        }
        activo->hecho = true;
        if(mini == Mini::COSTAL) { p.mejorar(&Atributos::poder,1); mensaje("¡Costal terminado! Poder +1"); }
        if(mini == Mini::MANOPLAS)
        {
            p.desbloquear(Habilidad::COMBOS);
            p.mejorar(&Atributos::velocidad,1);
            mensaje("¡Aprendiste las combinaciones 1-1-2, 1-2-3 y 3-4! Velocidad +1");
        }
        if(mini == Mini::CUERDA) { p.mejorar(&Atributos::aire,1); mensaje("¡Cuerda terminada! Aire +1"); }
        mini = Mini::NINGUNO;
        activo = nullptr;
        Audio::musica(cfg.musica);
    }

    void Escena_Recorrido::tecla(char t)
    {
        static const std::uint32_t BITS[] = {IVJ::BotonBox::JAB,IVJ::BotonBox::RECTO,IVJ::BotonBox::GANCHO,IVJ::BotonBox::UPPERCUT};
        int n = numero(t);
        if(n) diego->getComponente<CE::IControl>()->botones = BITS[n-1]; // anima el golpe
        switch(mini)
        {
            case Mini::COSTAL:
                if(t == TECLAS[progreso/5]) { progreso++; t_mini = 0.f; vaiven = 0.4f; Eventos::Get().emitir(Evento::GOLPE_CONECTADO); }
                else mensaje("Esa no: mira la indicación",1.f);
                if(progreso >= 20) terminarMini(true);
                break;
            case Mini::MANOPLAS:
            {
                if(mostrando) return;
                const auto& seq = SECUENCIAS[progreso];
                if(n != seq[pos_seq]) { mensaje("Carlos: «No, fíjate bien»",1.5f); mostrando = true; t_mini = 0.f; pos_seq = 0; return; }
                Eventos::Get().emitir(Evento::GOLPE_CONECTADO);
                t_mini = 0.f;
                if(++pos_seq < (int)seq.size()) return;
                pos_seq = 0;
                mostrando = true;
                if(++reps >= 2) { reps = 0; progreso++; }
                if(progreso >= (int)SECUENCIAS.size()) terminarMini(true);
                break;
            }
            case Mini::CUERDA:
            {
                if(t != 'J') return;
                int pulso = pulsoMasCercano(t_mini);
                if(enPulso(t_mini) && pulso != ultimo_pulso && pulso >= 1 && pulso <= 32) { progreso++; ultimo_pulso = pulso; }
                break;
            }
            default: break;
        }
    }

    void Escena_Recorrido::alPresionar(const std::string& a, bool p)
    {
        if(!p) return;
        if(en_dialogo)
        {
            if(a != "jab" && a != "aceptar") return;
            const auto& txt = dialogo.actual().texto;
            auto largo = sf::String::fromUtf8(txt.begin(),txt.end()).getSize();
            if(letras < largo) { letras = (float)largo; return; }
            letras = 0.f;
            if(!dialogo.siguiente()) en_dialogo = false;
            return;
        }
        if(mini != Mini::NINGUNO) { if(char t = teclaDe(a)) tecla(t); return; }
        if(a == "jab" || a == "aceptar")
            if(auto* c = cerca()) empezar(*c);
    }

    void Escena_Recorrido::actualizar(float dt)
    {
        if(terminado) return;
        if(JuegoR12::prueba.autopiloto) piloto(dt);
        letras += 40.f*dt;
        aviso_t -= dt;
        vaiven = std::max(0.f,vaiven - dt);
        auto* ctrl = diego->getComponente<CE::IControl>();
        if(en_dialogo || mini != Mini::NINGUNO)
        {
            ctrl->arr = ctrl->abj = ctrl->izq = ctrl->der = false;
        }
        else
        {
            *ctrl = control;
            ctrl->botones = 0; // caminando, J activa las estaciones en vez de golpear
        }
        diego->inputFSM();
        ctrl->botones = 0;
        diego->onUpdate(dt);
        float limite = listoParaRing() ? ancho - 20.f : PARED;
        SistemaCaminar(*diego,dt,20.f,limite,PISO0,PISO1);
        float x = diego->getTransformada()->posicion.x;
        if(!listoParaRing() && x >= PARED - 1.f && aviso_t <= 0.f)
            mensaje("Carlos: «Primero termina las estaciones»");

        // sed y aire mientras entrena (no hay pelea aquí)
        barras.sed = std::min(100.f, barras.sed + (mini != Mini::NINGUNO ? 1.2f : 0.3f)*dt);
        barras.aire = std::min(100.f, barras.aire + 6.f*dt);
        if(platano && std::abs(platano->getTransformada()->posicion.x - x) < 10.f)
        {
            barras.resistencia_max *= 1.25f;
            barras.resistencia = barras.resistencia_max;
            platano.reset();
            Eventos::Get().emitir(Evento::ITEM);
            mensaje("Plátano del casillero: la resistencia subió");
        }
        // diálogos que se activan al pasar por un punto (pasillo de Mexicali)
        if(sig_dialogo < cfg.dialogos_en_x.size() && x >= cfg.dialogos_en_x[sig_dialogo].first)
        {
            en_dialogo = dialogo.cargar(std::string(R12_DATOS) + "/dialogos/" + cfg.dialogos_en_x[sig_dialogo].second + ".txt");
            letras = 0.f;
            sig_dialogo++;
        }

        if(mini != Mini::NINGUNO)
        {
            t_mini += dt;
            if(mini == Mini::COSTAL && t_mini > 1.5f) { t_mini = 0.f; mensaje("¡Más rápido!",1.f); }
            if(mini == Mini::MANOPLAS)
            {
                if(mostrando && t_mini >= 1.5f) { mostrando = false; t_mini = 0.f; }
                else if(!mostrando && t_mini > (pos_seq ? 0.6f : 3.f))
                { mensaje("Carlos: «Muy lento»",1.5f); mostrando = true; t_mini = 0.f; pos_seq = 0; }
            }
            if(mini == Mini::CUERDA && t_mini > 16.3f) terminarMini(progreso >= 22);
        }

        if(listoParaRing() && x >= cfg.x_ring && mini == Mini::NINGUNO && !en_dialogo)
        {
            terminado = true;
            Partida::Get().historia().avanzar();
            irAPaso();
            return;
        }
        moverCamara(std::clamp(x,144.f,ancho - 144.f),96.f);
    }

    // --autopiloto: hace las estaciones solo (para probar sin teclado)
    void Escena_Recorrido::piloto(float dt)
    {
        piloto_t -= dt;
        control.izq = control.arr = control.abj = false;
        control.der = mini == Mini::NINGUNO && !en_dialogo;
        if(piloto_t > 0.f) return;
        piloto_t = 0.25f;
        if(en_dialogo) { alPresionar("jab",true); alPresionar("jab",true); return; }
        if(mini == Mini::COSTAL) { const char* n[] = {"jab","recto","gancho","uppercut"}; alPresionar(n[progreso/5],true); return; }
        if(mini == Mini::MANOPLAS && !mostrando)
        {
            const char* n[] = {"","jab","recto","gancho","uppercut"};
            alPresionar(n[SECUENCIAS[progreso][pos_seq]],true);
            piloto_t = 0.2f;
            return;
        }
        if(mini == Mini::CUERDA)
        {
            piloto_t = 0.f;
            if(enPulso(t_mini,0.5f,0.03f)) alPresionar("jab",true);
            return;
        }
        if(auto* c = cerca(); c && !c->hecho) { control.der = false; alPresionar("jab",true); }
    }

    void Escena_Recorrido::dibujarMundo()
    {
        auto& r = CE::Render::Get();
        r.AddToDraw(mapa(cfg.mapa));
        for(auto& p : puestos)
        {
            if(p.e == Estacion::COSTAL)
            {
                auto* s = p.sprite->getComponente<CE::ISprite>();
                int f = vaiven > 0.f ? (vaiven > 0.2f ? 1 : 2) : 0;
                s->m_sprite.setTextureRect({{16*f,0},{16,40}});
            }
            if(p.e == Estacion::CUERDA && mini == Mini::CUERDA)
            {
                int f = (int)(t_mini/0.125f) % 4;
                p.sprite->getComponente<CE::ISprite>()->m_sprite.setTextureRect({{32*f,0},{32,16}});
            }
            r.AddToDraw(*p.sprite);
        }
        if(!cfg.estaciones.empty()) r.AddToDraw(*carlos);
        if(platano) r.AddToDraw(*platano);
        if(mini == Mini::CUERDA && enPulso(t_mini,0.5f,0.06f))
            diego->getComponente<CE::ISprite>()->m_sprite.move({0.f,-4.f}); // salta en el pulso
        r.AddToDraw(*diego);
    }

    void Escena_Recorrido::dibujarPantalla(sf::RenderTarget& t, sf::Vector2f tam)
    {
        auto linea = [&](const std::string& s, float y, float alto, sf::Color c = sf::Color::White){
            auto txt = texto(s,(unsigned)(tam.y*alto),c,{0.f,tam.y*y});
            centrarX(txt,tam.x/2.f);
            t.draw(txt);
        };
        auto icono = [&](int golpe, float x, float y, float esc){
            sf::Sprite s{textura("provisionales/sprites/iconos_golpes.png"),sf::IntRect{{16*golpe,0},{16,16}}};
            s.setScale({esc,esc});
            s.setPosition({x,y});
            t.draw(s);
        };
        float esc = tam.y*0.007f;
        if(mini == Mini::COSTAL)
        {
            linea("COSTAL: presiona la tecla que aparece (" + std::to_string(progreso) + "/20)",0.25f,0.04f,COLOR_CONCENTRACION);
            icono(progreso/5,tam.x/2.f - 8*esc,tam.y*0.32f,esc);
        }
        if(mini == Mini::MANOPLAS)
        {
            const auto& seq = SECUENCIAS[std::min(progreso,(int)SECUENCIAS.size()-1)];
            linea(mostrando ? "MANOPLAS: Carlos marca la combinación..." : "¡Repítela!",0.25f,0.04f,COLOR_CONCENTRACION);
            float x0 = tam.x/2.f - seq.size()*9*esc;
            for(size_t i = 0; i < seq.size(); ++i)
                if(mostrando || (int)i < pos_seq) icono(seq[i]-1,x0 + i*18*esc,tam.y*0.32f,esc);
            linea("Repeticiones: " + std::to_string(reps) + "/2   Combinación " + std::to_string(progreso+1) + "/3",0.47f,0.03f);
        }
        if(mini == Mini::CUERDA)
        {
            linea("CUERDA: presiona J justo en el pulso (" + std::to_string(progreso) + "/22)",0.25f,0.04f,COLOR_CONCENTRACION);
            float r = enPulso(t_mini,0.5f,0.08f) ? tam.y*0.05f : tam.y*0.03f;
            sf::CircleShape c{r};
            c.setOrigin({r,r});
            c.setPosition({tam.x/2.f,tam.y*0.38f});
            c.setFillColor(enPulso(t_mini,0.5f,0.08f) ? COLOR_CONCENTRACION : sf::Color(120,120,120));
            t.draw(c);
        }
        if(mini == Mini::NINGUNO && !en_dialogo)
        {
            if(auto* c = cerca()) linea(std::string("J: ") + (c->e == Estacion::AGUA ? "tomar agua en " : "entrenar en ") + c->nombre,0.8f,0.04f,COLOR_AIRE);
            int hechas = (int)std::count_if(puestos.begin(),puestos.end(),[](const Puesto& p){ return p.hecho && p.e != Estacion::AGUA; });
            if(!puestos.empty())
                linea("Estaciones: " + std::to_string(hechas) + "/3" + (listoParaRing() ? "  -  ¡sube al ring!" : ""),0.22f,0.03f);
        }
        if(aviso_t > 0.f) linea(aviso,0.66f,0.035f,COLOR_CONCENTRACION);
        if(en_dialogo && !dialogo.fin())
        {
            sf::RectangleShape caja{{tam.x*0.9f,tam.y*0.22f}};
            caja.setPosition({tam.x*0.05f,tam.y*0.74f});
            caja.setFillColor(sf::Color(10,10,20,230));
            caja.setOutlineColor(sf::Color::White);
            caja.setOutlineThickness(3.f);
            t.draw(caja);
            const auto& l = dialogo.actual();
            std::string nom = l.orador;
            if(!nom.empty()) nom[0] = (char)std::toupper(nom[0]);
            t.draw(texto(nom,(unsigned)(tam.y*0.033f),COLOR_CONCENTRACION,{tam.x*0.07f,tam.y*0.75f}));
            auto completo = sf::String::fromUtf8(l.texto.begin(),l.texto.end());
            auto parcial = completo.substring(0,std::min<size_t>((size_t)letras,completo.getSize()));
            sf::String envuelto;
            size_t col = 0;
            for(size_t i = 0; i < parcial.getSize(); ++i)
            {
                if(parcial[i] == ' ' && col > 58) { envuelto += '\n'; col = 0; continue; }
                envuelto += parcial[i];
                col++;
            }
            auto cuerpo = texto("",(unsigned)(tam.y*0.032f),sf::Color::White,{tam.x*0.07f,tam.y*0.8f});
            cuerpo.setString(envuelto);
            t.draw(cuerpo);
        }
    }
}
