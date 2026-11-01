#include "JuegoR12.hpp"
#include <Motor/Primitivos/GestorEscenas.hpp>
#include <Round12/Escenas/EscenaR12.hpp>
#include <Round12/Escenas/Escena_Titulo.hpp>
#include <Round12/Escenas/Escena_Cinematica.hpp>
#include <Round12/Escenas/Escena_Resultados.hpp>
#include <Round12/Escenas/Escena_Ring.hpp>
#include <Round12/Escenas/Escena_Recorrido.hpp>
#include <Round12/Escenas/Escena_Runner.hpp>
#include <Round12/Escenas/Escena_SantaFe.hpp>
#include <Round12/Logica/Partida.hpp>
#include <Round12/objetos/Audio.hpp>
#include <Round12/objetos/Hud.hpp>
#include <algorithm>
#include <iostream>
#include <memory>

namespace Round12
{
    namespace
    {
        const char* clave(TipoPaso t)
        {
            switch(t)
            {
                case TipoPaso::CINEMATICA: return "R12_Cinematica";
                case TipoPaso::RECORRIDO:  return "R12_Recorrido";
                case TipoPaso::PELEA:      return "R12_Ring";
                case TipoPaso::RUNNER:     return "R12_Runner";
                case TipoPaso::SANTA_FE:   return "R12_SantaFe";
                case TipoPaso::RESULTADOS: return "R12_Resultados";
                default:                   return "R12_Titulo";
            }
        }
    }

    void irATitulo()
    {
        CE::GestorEscenas::Get().cambiarEscena("R12_Titulo");
    }

    void irAPaso()
    {
        auto& g = CE::GestorEscenas::Get();
        auto& h = Partida::Get().historia();
        if(h.fin() && !JuegoR12::prueba.paso.empty())
        {
            std::cout<<"[prueba-r12] fin del paso de prueba"<<std::endl;
            JuegoR12::instancia()->terminar();
        }
        // pasos de una escena todavía no registrada se saltan (con aviso)
        for(int intentos = 0; intentos < 64; ++intentos)
        {
            if(h.fin()) { irATitulo(); return; }
            auto& todas = g.getTodasEscenas();
            auto it = todas.find(clave(h.actual().tipo));
            if(it != todas.end())
            {
                if(auto e = std::dynamic_pointer_cast<EscenaR12>(it->second))
                    e->configurar(h.actual().id);
                g.cambiarEscena(it->first);
                return;
            }
            std::cerr<<"[round12] sin escena para el paso '"<<h.actual().id<<"'\n";
            if(!h.avanzar()) break;
        }
        irATitulo();
    }

    void reiniciarPaso()
    {
        auto& g = CE::GestorEscenas::Get();
        if(auto* e = dynamic_cast<EscenaR12*>(&g.getEscenaActual()))
        {
            e->pedirReinicio();
            g.cambiarEscena(e->nombre);
        }
    }

    JuegoR12::JuegoR12()
        :CE::GameLayer{}
    {
        termino = false;
        actual = this;
        menu_pausa = {
            {"Continuar",[this]{ pausado = false; }},
            {"Reiniciar nivel",[this]{ pausado = false; reiniciarPaso(); }},
            {"Menú principal",[this]{ pausado = false; irATitulo(); }}};
    }

    void JuegoR12::OnInit()
    {
        auto& g = CE::GestorEscenas::Get();
        g.registrarEscena("R12_Titulo",std::make_shared<Escena_Titulo>());
        g.registrarEscena("R12_Cinematica",std::make_shared<Escena_Cinematica>());
        g.registrarEscena("R12_Resultados",std::make_shared<Escena_Resultados>());
        g.registrarEscena("R12_Ring",std::make_shared<Escena_Ring>());
        g.registrarEscena("R12_Recorrido",std::make_shared<Escena_Recorrido>());
        g.registrarEscena("R12_Runner",std::make_shared<Escena_Runner>());
        g.registrarEscena("R12_SantaFe",std::make_shared<Escena_SantaFe>());
        Audio::iniciar();
    }

    CE::Escena* JuegoR12::escena() const
    {
        return &CE::GestorEscenas::Get().getEscenaActual();
    }

    EscenaR12* JuegoR12::escenaR12() const
    {
        return dynamic_cast<EscenaR12*>(escena());
    }

    void JuegoR12::activar()
    {
        auto& g = CE::GestorEscenas::Get();
        if(!prueba.paso.empty() && ultima == "R12_Titulo")
        {
            // --r12-paso=tipo:id
            static const std::pair<const char*,TipoPaso> TIPOS[] = {
                {"cine",TipoPaso::CINEMATICA},{"recorrido",TipoPaso::RECORRIDO},{"pelea",TipoPaso::PELEA},
                {"runner",TipoPaso::RUNNER},{"santafe",TipoPaso::SANTA_FE},{"resultados",TipoPaso::RESULTADOS},
                {"demo",TipoPaso::FIN},{"historia",TipoPaso::FIN}};
            auto dos = prueba.paso.find(':');
            std::string tipo = prueba.paso.substr(0,dos), id = dos == std::string::npos ? "" : prueba.paso.substr(dos+1);
            if(tipo == "demo" || tipo == "historia") Partida::Get().nueva(tipo == "demo");
            else
                for(auto& [n,t] : TIPOS)
                    if(tipo == n) Partida::Get().nueva({{t,id},{TipoPaso::FIN,"fin"}},true);
            ultima = "R12_Prueba";
            irAPaso();
            return;
        }
        if(g.getEscenaActual().nombre != ultima)
            g.cambiarEscena(ultima);
        else
            escena()->onInit(); // misma escena (vino del lanzador): solo cámara y música
    }

    void JuegoR12::desactivar()
    {
        ultima = escena()->nombre;
        if(ultima.rfind("R12_",0) != 0) ultima = "R12_Titulo";
        if(auto* e = escenaR12()) e->soltarControles();
        Audio::detener();
    }

    void JuegoR12::OnInputs(std::optional<sf::Event>& eventos)
    {
        if(eventos && eventos->getIf<sf::Event::Closed>())
            termino = true;
    }

    bool JuegoR12::manejarPausa(sf::Keyboard::Scancode tecla)
    {
        using SC = sf::Keyboard::Scancode;
        auto* e = escenaR12();
        if(!e || !e->permitePausa()) return false;
        if(tecla == SC::Escape)
        {
            pausado = !pausado;
            opcion = 0;
            e->soltarControles();
            return true;
        }
        if(!pausado) return false;
        int n = (int)menu_pausa.size();
        if(tecla == SC::W || tecla == SC::Up) opcion = (opcion + n - 1) % n;
        if(tecla == SC::S || tecla == SC::Down) opcion = (opcion + 1) % n;
        if(tecla == SC::J || tecla == SC::Enter || tecla == SC::Space) menu_pausa[opcion].second();
        return true;
    }

    // mismo despacho que IVJ::Juego: tecla -> acción registrada de la escena
    void JuegoR12::OnInputs(CE::InputBuffer& buffer)
    {
        while(!buffer.estaVacio())
        {
            auto& ev = buffer.popData();
            if(!ev) continue;
            if(ev->getIf<sf::Event::Closed>()) { termino = true; continue; }
            sf::Keyboard::Scancode scan;
            CE::Botones::TipoAccion tipo;
            if(const auto* p = ev->getIf<sf::Event::KeyPressed>())
            {
                if(manejarPausa(p->scancode)) continue;
                scan = p->scancode; tipo = CE::Botones::TipoAccion::OnPress;
            }
            else if(const auto* r = ev->getIf<sf::Event::KeyReleased>())
            { scan = r->scancode; tipo = CE::Botones::TipoAccion::OnRelease; }
            else continue;
            if(pausado) continue;
            auto* e = escena();
            auto it = e->getBotones().find(scan);
            if(it != e->getBotones().end())
                e->onInputs(CE::Botones(it->second,tipo,scan));
        }
    }

    void JuegoR12::OnUpdate(float dt)
    {
        if(pausado && !(escenaR12() && escenaR12()->permitePausa())) pausado = false;
        if(pausado) return;
        // el primer frame del motor y los cambios de juego traen dt enormes
        for(int i = 0; i < prueba.rapido && !termino; ++i)
            escena()->onUpdate(std::min(dt,1.f/30.f));
    }

    void JuegoR12::OnRender()
    {
        escena()->onRender();
        if(pausado) dibujarPausa();
    }

    void JuegoR12::dibujarPausa()
    {
        EnPantalla p;
        sf::RectangleShape velo{p.tam};
        velo.setFillColor(sf::Color(0,0,0,170));
        p.rt.draw(velo);
        auto t = texto("PAUSA",(unsigned)(p.tam.y*0.08f),sf::Color::White,{0.f,p.tam.y*0.25f});
        centrarX(t,p.tam.x/2.f);
        p.rt.draw(t);
        for(int i = 0; i < (int)menu_pausa.size(); ++i)
        {
            auto o = texto((i == opcion ? "> " : "  ") + menu_pausa[i].first,(unsigned)(p.tam.y*0.045f),
                    i == opcion ? COLOR_CONCENTRACION : sf::Color::White,{0.f,p.tam.y*(0.42f + i*0.08f)});
            centrarX(o,p.tam.x/2.f);
            p.rt.draw(o);
        }
        auto ayuda = texto("W/S elegir  ·  J o Enter aceptar  ·  Esc continuar",(unsigned)(p.tam.y*0.03f),
                sf::Color(200,200,200),{0.f,p.tam.y*0.8f});
        centrarX(ayuda,p.tam.x/2.f);
        p.rt.draw(ayuda);
    }
}
