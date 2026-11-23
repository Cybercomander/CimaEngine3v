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
#include <Round12/objetos/CajaDialogo.hpp>
#include <Round12/objetos/FabricaR12.hpp>
#include <algorithm>
#include <cmath>

namespace Round12
{
    namespace
    {
        const std::vector<std::vector<int>> SECUENCIAS = {{1,1,2},{1,2,3},{3,4}};
        const char TECLAS[] = {'J','K','L','I'};
        constexpr float RADIO = 110.f;     // distancia para usar una estación
        char teclaDe(const std::string& a)
        {
            if(a == "jab") return 'J';
            if(a == "recto") return 'K';
            if(a == "gancho") return 'L';
            if(a == "uppercut") return 'I';
            return 0;
        }
        int numero(char t) { return t == 'J' ? 1 : t == 'K' ? 2 : t == 'L' ? 3 : t == 'I' ? 4 : 0; }
        const char* tipoDe(Estacion e)
        {
            switch(e)
            {
                case Estacion::COSTAL: return "costal";
                case Estacion::MANOPLAS: return "manoplas";
                case Estacion::CUERDA: return "cuerda";
                case Estacion::AGUA: return "agua";
                default: return "platano";
            }
        }
        const char* nombreDe(Estacion e)
        {
            switch(e)
            {
                case Estacion::COSTAL: return "el costal";
                case Estacion::MANOPLAS: return "las manoplas con Carlos";
                case Estacion::CUERDA: return "la cuerda";
                case Estacion::AGUA: return "el garrafón";
                default: return "el plátano del casillero";
            }
        }
        float distancia(Pt a, Pt b) { return std::hypot(a.x-b.x,a.y-b.y); }
    }

    Pt Escena_Recorrido::pies(const std::shared_ptr<IVJ::Entidad>& e) const
    {
        auto p = e->getTransformada()->posicion;
        return {p.x,p.y};
    }

    void Escena_Recorrido::reiniciar()
    {
        cfg = recorrido(id);
        esc = &escenario(cfg.escenario);
        auto& p = Partida::Get();
        piso = {};
        for(const auto& poli : esc->piso) piso.agregar(poli);
        for(const auto& b : esc->bloqueos) piso.bloquear(b);

        diego = FabricaR12::boxeador(Personaje::DIEGO,cfg.entrada.x,cfg.entrada.y,ATRAS,std::make_shared<IdleR12>());
        auto* b = diego->getComponente<IBoxeador>();
        b->atr = p.diego();
        b->jugador = true;
        b->puede_esquivar = false;
        barras = barrasIniciales(p.diego());
        carlos = FabricaR12::npc(Personaje::CARLOS,esc->escalera_izq.x + 120.f,esc->escalera_izq.y,IZQUIERDA);
        ricardo = cfg.ricardo_en_gradas ? FabricaR12::npc(Personaje::RICARDO,1730.f,560.f,ADELANTE) : nullptr;
        puestos.clear();
        for(auto e : cfg.estaciones)
        {
            Pt pie{0,0};
            for(const auto& st : esc->estaciones) if(st.tipo == tipoDe(e)) pie = st.pie;
            puestos.push_back({e,pie,false,nombreDe(e)});
        }

        lista.limpiar();
        for(const auto& pr : esc->props) lista.agregar(FabricaR12::prop(pr));
        if(esc->ring) lista.agregar(FabricaR12::cuerdas(*esc));
        lista.agregar(FabricaR12::nodo(diego));
        lista.agregar(FabricaR12::nodo(carlos));
        if(ricardo) lista.agregar(FabricaR12::nodo(ricardo));

        hud.limpiar();
        agregarBandas(hud);
        agregarBarras(hud,barras,false,"Diego");
        hud.agregar(std::make_unique<Retrato>([this]{
                    return std::string("sprites/personajes/diego_ureta/retratos/") +
                        (barras.sed > 70.f ? "cansado" : mini != Mini::NINGUNO ? "decidido" : "neutral") + ".png"; },
                    sf::Vector2f{4.f/1080.f,4.f/720.f},48.f));
        mini = Mini::NINGUNO;
        activo = nullptr;
        sig_dialogo = 0;
        terminado = false;
        camino.clear();
        aviso.clear();
        en_dialogo = !cfg.dialogo_inicio.empty() &&
            dialogo.cargar(std::string(R12_DATOS) + "/dialogos/" + cfg.dialogo_inicio + ".txt");
        letras = 0.f;
    }

    Escena_Recorrido::Puesto* Escena_Recorrido::cerca()
    {
        Pt d = pies(diego);
        for(auto& p : puestos)
            if(distancia(p.pie,d) < RADIO && (!p.hecho || p.e == Estacion::AGUA)) return &p;
        return nullptr;
    }

    bool Escena_Recorrido::listoParaRing() const
    {
        return std::all_of(puestos.begin(),puestos.end(),[](const Puesto& p){
                return p.hecho || p.e == Estacion::AGUA || p.e == Estacion::PLATANO; });
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
        if(p.e == Estacion::PLATANO)
        {
            barras.resistencia_max *= 1.25f;
            barras.resistencia = barras.resistencia_max;
            p.hecho = true;
            Eventos::Get().emitir(Evento::ITEM);
            mensaje("Plátano del casillero: la resistencia subió");
            return;
        }
        activo = &p;
        t_mini = 0.f;
        progreso = reps = pos_seq = 0;
        ultimo_pulso = -1;
        mostrando = true;
        soltarControles();
        // Diego voltea hacia la estación
        auto pd = pies(diego);
        int d = direccionHacia(p.pie.x - pd.x,p.pie.y - pd.y);
        if(d >= 0) diego->getComponente<IVJ::IDireccion>()->dir = d;
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
        if(n && mini != Mini::CUERDA) diego->getComponente<CE::IControl>()->botones = BITS[n-1]; // anima el golpe
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
            auto largo = largoTexto(dialogo.actual().texto);
            if(letras < largo) { letras = (float)largo; return; }
            letras = 0.f;
            if(!dialogo.siguiente()) en_dialogo = false;
            return;
        }
        if(mini != Mini::NINGUNO) { if(char t = teclaDe(a)) tecla(t); return; }
        if(a == "jab" || a == "aceptar")
            if(auto* c = cerca()) empezar(*c);
    }

    // Carlos espera junto a la siguiente estación pendiente (en manoplas, frente a Diego)
    void Escena_Recorrido::moverCarlos(float dt)
    {
        if(puestos.empty()) return;
        Pt meta = esc->escalera_izq;
        for(auto& p : puestos)
            if(!p.hecho && p.e != Estacion::AGUA && p.e != Estacion::PLATANO) { meta = p.pie; break; }
        meta.x += meta.x > 960.f ? -150.f : 150.f;
        auto& pc = carlos->getTransformada()->posicion;
        auto* a = carlos->getComponente<IAnimR12>();
        float dx = meta.x - pc.x, dy = meta.y - pc.y, d = std::hypot(dx,dy);
        if(d > 6.f && mini != Mini::MANOPLAS)
        {
            float v = std::min(d,260.f*dt);
            Pt sig = piso.limitar({pc.x,pc.y},{pc.x + dx/d*v,pc.y + dy/d*v});
            pc.x = sig.x; pc.y = sig.y;
            a->dialogo = false;
            a->dir = direccionHacia(dx,dy);
            a->poner(Accion::CAMINAR);
        }
        else if(mini == Mini::MANOPLAS && mostrando)
        {
            // Carlos marca la combinación con su demostración de golpes (sin guantes, GDD)
            const auto& seq = SECUENCIAS[std::min(progreso,(int)SECUENCIAS.size()-1)];
            int i = (int)(t_mini/0.5f);
            a->dialogo = false;
            a->dir = direccionHacia(diego->getTransformada()->posicion.x - pc.x,0.f);
            // cada golpe de la secuencia reinicia la animación (1-1-2 repite el jab)
            if(i < (int)seq.size()) a->poner(elegirAccion(Postura::PIE,brazoDe((Golpe)(seq[i]-1))),i != demo_i);
            else a->poner(Accion::GUARDIA);
            demo_i = i;
        }
        else
        {
            demo_i = -1;
            a->dialogo = false;
            a->dir = direccionHacia(diego->getTransformada()->posicion.x - pc.x,0.f);
            a->poner(mini == Mini::MANOPLAS ? Accion::GUARDIA : Accion::IDLE);
        }
        a->actualizar(dt);
    }

    // durante un diálogo el que habla usa su emoción y los demás escuchan (hoja de diálogo)
    void Escena_Recorrido::animarDialogo(float dt)
    {
        const std::pair<const char*,std::shared_ptr<IVJ::Entidad>> gente[] = {
            {"diego",diego},{"carlos",carlos},{"ricardo",ricardo}};
        for(auto& [quien,e] : gente)
        {
            if(!e) continue;
            auto* a = e->getComponente<IAnimR12>();
            if(!en_dialogo || dialogo.fin()) { a->dialogo = false; continue; }
            const auto& l = dialogo.actual();
            int emo = l.orador == quien ? indiceEmocion(l.emocion) : indiceEmocion("escuchando");
            a->hablar(emo,dt);
        }
    }

    void Escena_Recorrido::actualizar(float dt)
    {
        if(terminado) return;
        if(JuegoR12::prueba.autopiloto) piloto(dt);
        letras += 40.f*dt;
        reloj += dt;
        aviso_t -= dt;
        vaiven = std::max(0.f,vaiven - dt);
        auto* ctrl = diego->getComponente<CE::IControl>();
        auto botones = ctrl->botones;
        if(en_dialogo || mini != Mini::NINGUNO)
            ctrl->arr = ctrl->abj = ctrl->izq = ctrl->der = false;
        else
        {
            *ctrl = control;
            ctrl->botones = 0; // caminando, J activa las estaciones en vez de golpear
            botones = 0;
        }
        ctrl->botones = botones;
        diego->inputFSM();
        ctrl->botones = 0;
        diego->onUpdate(dt);
        SistemaCaminar(*diego,dt,piso);
        moverCarlos(dt);
        animarDialogo(dt);
        if(mini == Mini::CUERDA)
        {
            // salta en cada pulso de la música: los 6 cuadros de saltar en medio segundo
            auto* a = diego->getComponente<IAnimR12>();
            a->poner(Accion::SALTAR);
            a->cuadro = std::min(CUADROS-1,(int)(std::fmod(t_mini + 0.25f,0.5f)/0.5f*CUADROS));
        }
        for(auto* e : {diego.get(),carlos.get(),ricardo.get()})
            if(e) { e->onUpdate(0.f); aplicarCuadro(*e); }
        Pt d = pies(diego);

        // sed y aire mientras entrena (no hay pelea aquí)
        barras.sed = std::min(100.f, barras.sed + (mini != Mini::NINGUNO ? 1.2f : 0.3f)*dt);
        barras.aire = std::min(100.f, barras.aire + 6.f*dt);
        // diálogos que se activan al pasar por un punto (llegada a Mexicali)
        if(!en_dialogo && sig_dialogo < cfg.dialogos_en_punto.size() &&
           distancia(d,cfg.dialogos_en_punto[sig_dialogo].first) < 160.f)
        {
            en_dialogo = dialogo.cargar(std::string(R12_DATOS) + "/dialogos/" + cfg.dialogos_en_punto[sig_dialogo].second + ".txt");
            letras = 0.f;
            sig_dialogo++;
        }

        if(mini != Mini::NINGUNO)
        {
            t_mini += dt;
            if(mini == Mini::COSTAL && t_mini > 1.5f) { t_mini = 0.f; mensaje("¡Más rápido!",1.f); }
            if(mini == Mini::MANOPLAS)
            {
                const auto& seq = SECUENCIAS[std::min(progreso,(int)SECUENCIAS.size()-1)];
                if(mostrando && t_mini >= 0.5f*seq.size() + 0.4f) { mostrando = false; t_mini = 0.f; }
                else if(!mostrando && t_mini > (pos_seq ? 0.6f : 3.f))
                { mensaje("Carlos: «Muy lento»",1.5f); mostrando = true; t_mini = 0.f; pos_seq = 0; }
            }
            if(mini == Mini::CUERDA && t_mini > 16.3f) terminarMini(progreso >= 22);
        }

        bool en_escalera = distancia(d,esc->escalera_izq) < RADIO;
        bool dialogos_listos = sig_dialogo >= cfg.dialogos_en_punto.size();
        if(en_escalera && !listoParaRing() && aviso_t <= 0.f) mensaje("Carlos: «Primero termina las estaciones»");
        if(en_escalera && listoParaRing() && dialogos_listos && mini == Mini::NINGUNO && !en_dialogo)
        {
            terminado = true;
            Partida::Get().historia().avanzar();
            irAPaso();
        }
    }

    // --autopiloto: camina a cada estación, la hace y sube al ring (para probar sin teclado)
    void Escena_Recorrido::piloto(float dt)
    {
        control.izq = control.der = control.arr = control.abj = false;
        piloto_t -= dt;
        if(en_dialogo) { if(piloto_t <= 0.f) { piloto_t = 0.25f; alPresionar("jab",true); alPresionar("jab",true); } return; }
        if(mini == Mini::COSTAL) { if(piloto_t <= 0.f) { piloto_t = 0.25f; const char* n[] = {"jab","recto","gancho","uppercut"}; alPresionar(n[progreso/5],true); } return; }
        if(mini == Mini::MANOPLAS)
        {
            if(!mostrando && piloto_t <= 0.f)
            {
                const char* n[] = {"","jab","recto","gancho","uppercut"};
                alPresionar(n[SECUENCIAS[progreso][pos_seq]],true);
                piloto_t = 0.2f;
            }
            return;
        }
        if(mini == Mini::CUERDA) { if(enPulso(t_mini,0.5f,0.03f)) alPresionar("jab",true); return; }
        // destino: estación pendiente, punto de diálogo pendiente o la escalera
        Pt meta = esc->escalera_izq;
        bool estacion = false;
        for(auto& p : puestos) if(!p.hecho) { meta = p.pie; estacion = true; break; }
        if(!estacion && sig_dialogo < cfg.dialogos_en_punto.size()) meta = cfg.dialogos_en_punto[sig_dialogo].first;
        Pt d = pies(diego);
        if(estacion && distancia(d,meta) < RADIO*0.8f) { if(piloto_t <= 0.f) { piloto_t = 0.3f; alPresionar("jab",true); } return; }
        if(camino.empty() || distancia(camino.back(),meta) > 1.f) camino = ruta(piso,d,meta);
        while(!camino.empty() && distancia(camino.front(),d) < 25.f) camino.erase(camino.begin());
        if(camino.empty()) return;
        Pt sig = camino.front();
        if(sig.x > d.x + 6.f) control.der = true;
        if(sig.x < d.x - 6.f) control.izq = true;
        if(sig.y > d.y + 6.f) control.abj = true;
        if(sig.y < d.y - 6.f) control.arr = true;
    }

    void Escena_Recorrido::dibujarMundo()
    {
        auto& t = CE::Render::Get().GetTextura();
        dibujarCapa(t,rutaCapa(*esc,"decorado"));
        lista.dibujar(t);
        // marcadores de estaciones pendientes (ítems provisionales flotando)
        for(auto& p : puestos)
        {
            if(p.hecho && p.e != Estacion::AGUA) continue;
            const char* archivo = p.e == Estacion::COSTAL ? "iconos_golpes" : p.e == Estacion::MANOPLAS ? "manoplas" :
                p.e == Estacion::CUERDA ? "cuerda" : p.e == Estacion::AGUA ? "botella" : "platano";
            sf::Sprite s{textura(std::string("provisionales/sprites/") + archivo + ".png"),sf::IntRect{{0,0},{16,16}}};
            if(p.e == Estacion::CUERDA) s.setTextureRect({{0,0},{32,16}});
            s.setOrigin({s.getTextureRect().size.x/2.f,16.f});
            s.setScale({ESCALA_PERSONAJE,ESCALA_PERSONAJE});
            s.setPosition({p.pie.x,p.pie.y - 210.f + 10.f*std::sin(reloj*4.f)});
            t.draw(s);
        }
        // golpe acertado al costal: destello
        if(mini == Mini::COSTAL && vaiven > 0.f && activo)
        {
            sf::CircleShape c{40.f*vaiven/0.4f + 10.f};
            c.setOrigin({c.getRadius(),c.getRadius()});
            c.setPosition({activo->pie.x,activo->pie.y - 120.f});
            c.setFillColor(sf::Color(255,240,180,140));
            t.draw(c);
        }
        // la cuerda gira alrededor de Diego
        if(mini == Mini::CUERDA)
        {
            int f = (int)(t_mini/0.125f) % 4;
            sf::Sprite s{textura("provisionales/sprites/cuerda.png"),sf::IntRect{{32*f,0},{32,16}}};
            s.setOrigin({16.f,8.f});
            s.setScale({ESCALA_PERSONAJE*1.2f,ESCALA_PERSONAJE*2.f});
            auto pd = pies(diego);
            s.setPosition({pd.x,pd.y - 80.f});
            t.draw(s);
        }
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
            linea("COSTAL: presiona la tecla que aparece (" + std::to_string(progreso) + "/20)",0.12f,0.04f,COLOR_CONCENTRACION);
            icono(progreso/5,tam.x/2.f - 8*esc,tam.y*0.19f,esc);
        }
        if(mini == Mini::MANOPLAS)
        {
            const auto& seq = SECUENCIAS[std::min(progreso,(int)SECUENCIAS.size()-1)];
            linea(mostrando ? "MANOPLAS: mira la combinación de Carlos..." : "¡Repítela!",0.12f,0.04f,COLOR_CONCENTRACION);
            float x0 = tam.x/2.f - seq.size()*9*esc;
            for(size_t i = 0; i < seq.size(); ++i)
                if((mostrando && t_mini >= 0.5f*i) || (int)i < pos_seq) icono(seq[i]-1,x0 + i*18*esc,tam.y*0.19f,esc);
            linea("Repeticiones: " + std::to_string(reps) + "/2   Combinación " + std::to_string(progreso+1) + "/3",0.34f,0.03f);
        }
        if(mini == Mini::CUERDA)
        {
            linea("CUERDA: presiona J justo en el pulso (" + std::to_string(progreso) + "/22)",0.12f,0.04f,COLOR_CONCENTRACION);
            float r = enPulso(t_mini,0.5f,0.08f) ? tam.y*0.05f : tam.y*0.03f;
            sf::CircleShape c{r};
            c.setOrigin({r,r});
            c.setPosition({tam.x/2.f,tam.y*0.25f});
            c.setFillColor(enPulso(t_mini,0.5f,0.08f) ? COLOR_CONCENTRACION : sf::Color(120,120,120));
            t.draw(c);
        }
        if(mini == Mini::NINGUNO && !en_dialogo)
        {
            if(auto* c = cerca()) linea(std::string("J: ") + (c->e == Estacion::AGUA ? "tomar agua en " : "usar ") + c->nombre,0.84f,0.035f,COLOR_AIRE);
            int hechas = (int)std::count_if(puestos.begin(),puestos.end(),[](const Puesto& p){
                    return p.hecho && (p.e == Estacion::COSTAL || p.e == Estacion::MANOPLAS || p.e == Estacion::CUERDA); });
            if(!puestos.empty())
                linea("Estaciones: " + std::to_string(hechas) + "/3" +
                      (listoParaRing() ? "  -  ¡sube al ring por la escalera roja!" : ""),0.93f,0.028f);
            else linea("Camina a la escalera roja para subir al ring",0.93f,0.028f);
        }
        if(aviso_t > 0.f) linea(aviso,0.58f,0.035f,COLOR_CONCENTRACION);
        if(en_dialogo && !dialogo.fin()) dibujarCajaDialogo(t,tam,dialogo.actual(),letras,Vista::PERFIL,reloj);
    }
}
