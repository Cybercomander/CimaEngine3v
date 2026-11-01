#include "JuegoR12.hpp"
#include <Motor/Primitivos/GestorEscenas.hpp>
#include <Round12/Escenas/Escena_Titulo.hpp>
#include <algorithm>
#include <memory>

namespace Round12
{
    JuegoR12::JuegoR12()
        :CE::GameLayer{}
    {
        termino = false;
        actual = this;
    }

    void JuegoR12::OnInit()
    {
        auto& g = CE::GestorEscenas::Get();
        g.registrarEscena("R12_Titulo",std::make_shared<Escena_Titulo>());
    }

    CE::Escena* JuegoR12::escena() const
    {
        return &CE::GestorEscenas::Get().getEscenaActual();
    }

    void JuegoR12::activar()
    {
        CE::GestorEscenas::Get().cambiarEscena(ultima);
    }

    void JuegoR12::desactivar()
    {
        ultima = escena()->nombre;
    }

    void JuegoR12::OnInputs(std::optional<sf::Event>& eventos)
    {
        if(eventos->getIf<sf::Event::Closed>())
            termino = true;
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
            { scan = p->scancode; tipo = CE::Botones::TipoAccion::OnPress; }
            else if(const auto* r = ev->getIf<sf::Event::KeyReleased>())
            { scan = r->scancode; tipo = CE::Botones::TipoAccion::OnRelease; }
            else continue;
            auto* e = escena();
            auto it = e->getBotones().find(scan);
            if(it != e->getBotones().end())
                e->onInputs(CE::Botones(it->second,tipo,scan));
        }
    }

    void JuegoR12::OnUpdate(float dt)
    {
        // el primer frame del motor y los cambios de juego traen dt enormes
        escena()->onUpdate(std::min(dt,1.f/30.f));
    }

    void JuegoR12::OnRender()
    {
        escena()->onRender();
    }
}
