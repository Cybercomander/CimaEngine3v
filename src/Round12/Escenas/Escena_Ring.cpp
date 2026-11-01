#include "Escena_Ring.hpp"
#include <Juego/Componentes/IJComponentes.hpp>
#include <Motor/Render/Render.hpp>
#include <Round12/JuegoR12.hpp>
#include <Round12/Maquinas/FasesPelea.hpp>
#include <Round12/Maquinas/NodosR12.hpp>
#include <Round12/Sistemas/SistemasR12.hpp>
#include <Round12/objetos/FabricaR12.hpp>
#include <Round12/objetos/Vineta.hpp>
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>

namespace Round12
{
    namespace
    {
        IBoxeador* box(IVJ::Entidad& e) { return e.getComponente<IBoxeador>(); }

        Atributos rivalDe(const ConfigPelea& c, const std::string& id, std::mt19937& rng)
        {
            Atributos a = perfil(c.rival);
            if(c.aleatorio)
            {
                std::uniform_int_distribution<int> d(4,6);
                for(int Atributos::*m : {&Atributos::resistencia,&Atributos::aire,&Atributos::sed,
                        &Atributos::concentracion,&Atributos::velocidad,&Atributos::defensa,&Atributos::poder})
                    a.*m = d(rng);
            }
            (void)id;
            int extra = c.dificultad - 1;
            for(int Atributos::*m : {&Atributos::resistencia,&Atributos::aire,&Atributos::sed,
                    &Atributos::concentracion,&Atributos::velocidad,&Atributos::defensa,&Atributos::poder})
                a.*m = limitar(a.*m + extra);
            return a;
        }
    }

    Escena_Ring::Escena_Ring() = default;
    Escena_Ring::~Escena_Ring() = default;

    void Escena_Ring::cargar()
    {
        usarCamara();
    }

    void Escena_Ring::reiniciar()
    {
        auto& p = Partida::Get();
        cfg = pelea(id);
        esc = &escenario(cfg.escenario);
        lona = {};
        lona.agregar(esc->ring_poligono);
        rng.seed(p.semilla() + (unsigned)std::hash<std::string>{}(id));
        for(auto h : cfg.al_iniciar) p.desbloquear(h);

        Atributos a_rival = rivalDe(cfg,id,rng);
        combate = std::make_unique<Combate>(p.diego(),a_rival,cfg.objetivo,p.tiene(Habilidad::SEGUNDO_AIRE));
        cerebro = std::make_unique<CerebroRival>(estilo(cfg.rival),rng());
        if(JuegoR12::prueba.autopiloto)
            piloto = std::make_unique<CerebroRival>(EstiloIA{0.3f,0.3f,26.f,0.8f,{0.3f,0.3f,0.25f,0.15f}},rng());
        combos.reiniciar();
        round_actual = 1;
        tiempo_total = consejo = 0.f;
        caidas_diego = caidas_rival = caida_pendiente = seguidos = 0;
        pendiente = {};

        diego = FabricaR12::boxeador(Personaje::DIEGO,esc->esquina_diego.x,esc->esquina_diego.y,DERECHA,std::make_shared<IdleR12>());
        auto* bd = box(*diego);
        bd->atr = p.diego();
        bd->jugador = true;
        bd->mira_fijo = true;
        bd->puede_esquivar = p.tiene(Habilidad::ESQUIVA);
        rival = FabricaR12::boxeador(cfg.rival,esc->esquina_rival.x,esc->esquina_rival.y,IZQUIERDA,std::make_shared<IdleR12>(),cfg.tinte);
        auto* br = box(*rival);
        br->atr = a_rival;
        br->mira_fijo = true;
        // Carlos abajo de la esquina de Diego, junto a la escalera
        carlos_casa = {esc->escalera_izq.x - 170.f,esc->escalera_izq.y - 150.f};
        carlos_meta = carlos_casa;
        carlos_habla = false;
        carlos = FabricaR12::npc(Personaje::CARLOS,carlos_casa.x,carlos_casa.y,DERECHA);
        for(int i = 0; i < 2; ++i) { golpeado[i] = sonrisa[i] = 0.f; emo_fase[i].clear(); }

        lista.limpiar();
        for(const auto& pr : esc->props) lista.agregar(FabricaR12::prop(pr));
        if(esc->ring) lista.agregar(FabricaR12::cuerdas(*esc));
        lista.agregar(FabricaR12::nodo(carlos));
        lista.agregar(FabricaR12::nodo(diego));
        // el rival se dibuja con sus efectos de concentración
        lista.agregar(std::make_shared<NodoFuncion>(
            [this]{ return rival->getTransformada()->posicion.y; },
            [this](sf::RenderTarget& t){ dibujarRival(t); }));

        hud.limpiar();
        agregarBandas(hud);
        agregarBarras(hud,combate->barras(true),false,"Diego");
        agregarBarras(hud,combate->barras(false),true,Round12::nombre(cfg.rival));
        const std::string rival_hoja = hoja(cfg.rival);
        hud.agregar(std::make_unique<Retrato>([this]{
                    return "sprites/personajes/diego_ureta/retratos/" + emocion(0) + ".png"; },
                    sf::Vector2f{4.f/1080.f,4.f/720.f},48.f));
        hud.agregar(std::make_unique<Retrato>([this,rival_hoja]{
                    return "sprites/personajes/" + rival_hoja + "/retratos/" + emocion(1) + ".png"; },
                    sf::Vector2f{1028.f/1080.f,4.f/720.f},48.f));
        hud.agregar(std::make_unique<Etiqueta>([this]{
                    return "ROUND " + std::to_string(round_actual) + "/" + std::to_string(cfg.rounds);
                    },sf::Vector2f{0.5f,0.004f},0.03f,COLOR_CONCENTRACION,true));
        hud.agregar(std::make_unique<Etiqueta>([this]{
                    int s = std::max(0,(int)std::ceil(reloj));
                    return std::to_string(s/60) + ":" + (s%60 < 10 ? "0" : "") + std::to_string(s%60);
                    },sf::Vector2f{0.5f,0.038f},0.026f,sf::Color::White,true));
        hud.agregar(std::make_unique<Etiqueta>([]{
                    return std::string("J K L I golpes · Shift guardia · Q/E esquiva · Q+E agacharse");
                    },sf::Vector2f{0.5f,0.968f},0.018f,sf::Color(190,190,190),true));
        fase.reset();
        cambiarFase(std::make_unique<Presentacion>());
    }

    void Escena_Ring::colocarEnEsquinas()
    {
        // al centro de la lona, frente a frente
        diego->setPosicion(960.f - 180.f,615.f);
        rival->setPosicion(960.f + 180.f,615.f);
        for(auto* e : {diego.get(),rival.get()})
        {
            auto* b = box(*e);
            b->barras = combate->barras(e == diego.get());
            e->getComponente<CE::IControl>()->botones = 0;
            if(b->caido || e->getComponente<IVJ::IMaquinaEstado>()->fsm->getNombre() != "Idle")
                e->setFSM(std::make_shared<IdleR12>());
            b->caido = false;
        }
        SistemaMirarse(*diego,*rival);
    }

    void Escena_Ring::cambiarFase(std::unique_ptr<FasePelea> f)
    {
        // la fase actual puede pedir el cambio desde su propio actualizar
        siguiente = std::move(f);
    }

    void Escena_Ring::lanzamiento(IVJ::Entidad& e, bool es_diego)
    {
        auto* b = box(e);
        if(!b->golpe_lanzado) return;
        b->golpe_lanzado = false;
        b->golpe_fallido = !combate->lanzar(es_diego,b->ultimo);
        if(!es_diego) return;
        auto& p = Partida::Get();
        bool conc_max = combate->barras(true).concentracion >= 100.f && p.tiene(Habilidad::FRONTERA);
        auto c = combos.registrar(b->ultimo,tiempo_total,conc_max);
        pendiente.frontera = c == Combo::FRONTERA;
        pendiente.combo = c != Combo::NINGUNO && (p.tiene(Habilidad::COMBOS) || pendiente.frontera);
        pendiente.contragolpe = b->ventana_contragolpe > 0.f && p.tiene(Habilidad::CONTRAGOLPE);
        b->ventana_contragolpe = 0.f;
    }

    void Escena_Ring::impacto(IVJ::Entidad& atk, IVJ::Entidad& def, bool atk_diego)
    {
        auto* ba = box(atk);
        auto* bd = box(def);
        if(!ba->golpe_activo) return;
        ba->golpe_activo = false;
        if(bd->caido) return;
        if(!SistemaAlcance(atk,def,ba->ultimo)) { if(atk_diego) seguidos = 0; return; }
        // agachado: jab, recto y gancho pasan por arriba (cuenta como esquiva)
        if(bd->esquivando || (bd->agachado && ba->ultimo != Golpe::UPPERCUT))
        {
            if(atk_diego) seguidos = 0;
            else bd->ventana_contragolpe = 0.4f; // esquiva exitosa: ventana de contragolpe
            return;
        }
        Lanzamiento l = atk_diego ? pendiente : Lanzamiento{};
        combate->recibir(!atk_diego,ba->ultimo,bd->guardia,l.combo,l.contragolpe,l.frontera,bd->agachado);
        golpeado[atk_diego ? 1 : 0] = 0.4f;
        if(l.combo) sonrisa[atk_diego ? 0 : 1] = 1.f;
        if(atk_diego)
        {
            seguidos++;
            // «Frontera» completa: los 4 golpes conectaron y tira al rival
            if(l.frontera && seguidos >= 4 && cfg.objetivo != Objetivo::PERDER_FORZADO)
                combate->barras(false).resistencia = 0.f;
        }
        // solo los golpes fuertes aturden (si no, una ráfaga de jabs deja al rival sin responder)
        bool fuerte = ba->ultimo == Golpe::GANCHO || ba->ultimo == Golpe::UPPERCUT || l.combo || l.contragolpe;
        if(!bd->guardia && fuerte) def.setFSM(std::make_shared<AturdidoR12>());
    }

    void Escena_Ring::simular(float dt)
    {
        tiempo_total += dt;
        consejo = std::max(0.f,consejo - dt);
        for(int i = 0; i < 2; ++i) { golpeado[i] = std::max(0.f,golpeado[i] - dt); sonrisa[i] = std::max(0.f,sonrisa[i] - dt); }
        if(piloto) piloto->pensar(dt,*diego,*rival,control);
        *diego->getComponente<CE::IControl>() = control;
        cerebro->pensar(dt,*rival,*diego,*rival->getComponente<CE::IControl>());
        SistemaMirarse(*diego,*rival);
        diego->inputFSM();
        rival->inputFSM();
        diego->onUpdate(dt);
        rival->onUpdate(dt);
        for(auto* e : {diego.get(),rival.get()})
            SistemaCaminar(*e,dt,lona);
        SistemaSepararCuerpos(*diego,*rival);

        IVJ::Entidad* ents[2] = {diego.get(),rival.get()};
        for(int i = 0; i < 2; ++i)
        {
            auto* b = box(*ents[i]);
            // la esquiva cobra aire una vez al empezar; sin aire no sale
            if(b->esquivando && !esquivaba[i] && !combate->esquivar(i == 0))
                ents[i]->setFSM(std::make_shared<IdleR12>());
            esquivaba[i] = b->esquivando;
            lanzamiento(*ents[i],i == 0);
            b->ventana_contragolpe = std::max(0.f,b->ventana_contragolpe - dt);
        }
        impacto(*diego,*rival,true);
        impacto(*rival,*diego,false);

        auto estado = [](IVJ::Entidad& e){
            auto n = e.getComponente<IVJ::IMaquinaEstado>()->fsm->getNombre();
            return EstadoFisico{n == "Guardia", n == "Caminar"};
        };
        combate->tick(dt,estado(*diego),estado(*rival));
        box(*diego)->barras = combate->barras(true);
        box(*rival)->barras = combate->barras(false);

        if(combate->barras(true).resistencia <= 0.f) caida_pendiente = 1;
        else if(combate->barras(false).resistencia <= 0.f) caida_pendiente = 2;
    }

    void Escena_Ring::actualizar(float dt)
    {
        if(siguiente) { fase = std::move(siguiente); fase->entrar(*this); }
        if(!fase) return;
        fase->actualizar(*this,dt);
        // Carlos: camina hacia su meta; al llegar habla (dialogo/hablando) o espera (idle)
        auto& pc = carlos->getTransformada()->posicion;
        float dx = carlos_meta.x - pc.x, dy = carlos_meta.y - pc.y, d = std::hypot(dx,dy);
        auto* a = carlos->getComponente<IAnimR12>();
        if(d > 4.f)
        {
            float v = std::min(d,300.f*dt);
            pc.x += dx/d*v; pc.y += dy/d*v;
            a->dialogo = false;
            a->dir = direccionHacia(dx,dy);
            a->poner(Accion::CAMINAR);
        }
        else if(carlos_habla)
        {
            a->dir = direccionHacia(diego->getTransformada()->posicion.x - pc.x,0.f);
            if(!a->dialogo) { a->dialogo = true; a->emocion = 4; a->cuadro = 0; a->t = 0.f; } // hablando
            // la hoja de diálogo tiene 4 cuadros en ciclo
            a->t += dt*1000.f;
            if(a->t >= 150.f) { a->t = 0.f; a->cuadro = (a->cuadro + 1) % 4; }
        }
        else { a->dialogo = false; a->dir = DERECHA; a->poner(Accion::IDLE); }
        if(!a->dialogo) a->actualizar(dt);
        carlos->onUpdate(0.f);
        aplicarCuadro(*carlos);
    }

    void Escena_Ring::mandarCarlos(bool a_esquina)
    {
        carlos_meta = a_esquina ? Pt{esc->esquina_diego.x - 150.f,esc->ring_frente_y + 40.f} : carlos_casa;
        carlos_habla = a_esquina;
    }

    std::string Escena_Ring::emocion(int i) const
    {
        if(!emo_fase[i].empty()) return emo_fase[i];
        const auto& b = combate->barras(i == 0);
        if(golpeado[i] > 0.f) return "sorprendido";
        if(sonrisa[i] > 0.f) return "sonriendo";
        if(b.aire < 25.f) return "cansado";
        if(i == 0 && b.concentracion >= 100.f) return "decidido";
        return "neutral";
    }

    void Escena_Ring::terminar(bool gano, bool de_pie, const std::string& como)
    {
        auto& p = Partida::Get();
        bool avanza = cfg.objetivo == Objetivo::PERDER_FORZADO || (cfg.objetivo == Objetivo::GANAR ? gano : de_pie);
        if(avanza)
        {
            for(auto h : cfg.al_terminar) p.desbloquear(h);
            for(auto& [campo,delta] : cfg.mejoras) p.mejorar(campo,delta);
            p.resumen = std::string(Round12::nombre(cfg.rival)) + ": " + como;
        }
        std::cout<<"[prueba-r12] pelea "<<id<<": "<<como<<" (rounds "<<round_actual<<", caídas Diego "
            <<caidas_diego<<", rival "<<caidas_rival<<")"<<std::endl;
        if(p.alTerminarPelea(cfg.objetivo,gano,de_pie)) irAPaso();
        else if(!JuegoR12::prueba.paso.empty()) JuegoR12::instancia()->terminar(); // prueba: una vuelta
        else reiniciarPaso();
    }

    void Escena_Ring::alPresionar(const std::string& a, bool p)
    {
        // Q+E juntas: agacharse
        if(p && (a == "esquiva_izq" || a == "esquiva_der") && diego && box(*diego)->puede_esquivar
                && agacharse.presionar(a == "esquiva_izq" ? 'Q' : 'E',tiempo_total))
            box(*diego)->pide_agacharse = true;
        if(fase) fase->tecla(*this,a,p);
    }

    void Escena_Ring::dibujarMundo()
    {
        auto& t = CE::Render::Get().GetTextura();
        dibujarCapa(t,rutaCapa(*esc,"decorado"));
        lista.dibujar(t);
    }

    void Escena_Ring::dibujarRival(sf::RenderTarget& t)
    {
        auto* s = rival->getComponente<CE::ISprite>();
        float conc = combate->barras(true).concentracion;
        // concentración baja: el rival se ve borroso (dos copias desplazadas)
        if(conc < 40.f)
        {
            sf::Sprite copia = s->m_sprite;
            copia.setColor(sf::Color(255,255,255,80));
            for(float dx : {-7.f,7.f}) { copia.move({dx,0.f}); t.draw(copia); copia.move({-dx,0.f}); }
        }
        t.draw(*rival);
        // concentración alta: destello antes del golpe del rival (señal de lectura)
        if(conc >= 70.f && box(*rival)->destello > 0.f)
        {
            sf::Sprite brillo = s->m_sprite;
            brillo.setColor(sf::Color(255,255,255,(std::uint8_t)(140 + 100*std::sin(tiempo_total*40.f))));
            t.draw(brillo,sf::RenderStates{sf::BlendAdd});
        }
    }

    void Escena_Ring::dibujarPantalla(sf::RenderTarget& t, sf::Vector2f tam)
    {
        Vineta::dibujar(t,tam,combate->barras(true).concentracion);
        if(consejo > 0.f)
        {
            sf::Sprite icono{textura("provisionales/sprites/iconos_golpes.png"),
                sf::IntRect{{16*(int)cerebro->proximo(),0},{16,16}}};
            icono.setScale({tam.y*0.006f,tam.y*0.006f});
            icono.setPosition({tam.x*0.5f - 8.f*tam.y*0.006f,tam.y*0.27f});
            t.draw(icono);
            auto txt = texto("Carlos: ¡ahí viene!",(unsigned)(tam.y*0.03f),COLOR_CONCENTRACION,{0.f,tam.y*0.38f});
            centrarX(txt,tam.x/2.f);
            t.draw(txt);
        }
        if(fase) fase->dibujar(*this,t,tam);
    }
}
