#include "NodosJugador.hpp"

namespace IVJ
{
    static bool hayDireccion(const CE::IControl& c)
    {
        return c.arr || c.abj || c.izq || c.der;
    }

    // IDLE
    IdleBoxeador::IdleBoxeador()
        :FSM{},anim{{0,1},0.45f}
    {
        nombre = "IdleBoxeador";
    }
    FSM* IdleBoxeador::onInputs(const CE::IControl& control)
    {
        if(control.acc)
            return new JabBoxeador();
        if(hayDireccion(control))
            return new MoverBoxeador(control.run);
        return nullptr;
    }
    void IdleBoxeador::onEntrar(const Entidad& obj){ anim.iniciar(obj); }
    void IdleBoxeador::onSalir(const Entidad& obj){ (void)obj; }
    void IdleBoxeador::onUpdate(const Entidad& obj,float dt){ (void)obj; anim.actualizar(dt); }

    // CAMINAR / CORRER
    MoverBoxeador::MoverBoxeador(bool turbo)
        :FSM{},turbo{turbo},
        anim{{2,2}, turbo ? 0.09f : 0.2f, true, turbo ? 10.f : 5.f}
    {
        nombre = turbo ? "CorrerBoxeador" : "CaminarBoxeador";
    }
    FSM* MoverBoxeador::onInputs(const CE::IControl& control)
    {
        if(control.acc)
            return new JabBoxeador();
        if(!hayDireccion(control))
            return new IdleBoxeador();
        if(control.run != turbo)
            return new MoverBoxeador(control.run);
        //solo cambia de lado con entrada horizontal, si no conserva el último
        if(control.izq)
            anim.mirar(true);
        else if(control.der)
            anim.mirar(false);
        return nullptr;
    }
    void MoverBoxeador::onEntrar(const Entidad& obj){ anim.iniciar(obj); }
    void MoverBoxeador::onSalir(const Entidad& obj){ (void)obj; }
    void MoverBoxeador::onUpdate(const Entidad& obj,float dt){ (void)obj; anim.actualizar(dt); }

    // JAB
    JabBoxeador::JabBoxeador()
        :FSM{},anim{{2,3,3,2},0.07f,false}
    {
        nombre = "JabBoxeador";
    }
    FSM* JabBoxeador::onInputs(const CE::IControl& control)
    {
        (void)control;
        //el golpe no se interrumpe: regresa a idle al terminar
        if(anim.terminado())
            return new IdleBoxeador();
        return nullptr;
    }
    void JabBoxeador::onEntrar(const Entidad& obj){ anim.iniciar(obj); }
    void JabBoxeador::onSalir(const Entidad& obj){ (void)obj; }
    void JabBoxeador::onUpdate(const Entidad& obj,float dt){ (void)obj; anim.actualizar(dt); }
}
