#include "NodosRival.hpp"

namespace IVJ
{
    static bool jugadorCerca(Entidad& parent, CE::Vector2D& target)
    {
        auto rango = parent.getComponente<IRangoAggro>();
        return rango && rango->estaDentroRango(parent,target);
    }
    static bool targetALaIzquierda(Entidad& parent, CE::Vector2D& target)
    {
        return target.x < parent.getTransformada()->posicion.x;
    }

    // IDLE
    IdleRival::IdleRival()
        :FSMBoss{},anim{{0,1},0.5f}
    {
        nombre = "IdleRival";
    }
    FSM* IdleRival::onInputs(Entidad& parent, CE::Vector2D& target)
    {
        if(jugadorCerca(parent,target))
            return new JabRival();
        return nullptr;
    }
    void IdleRival::onEntrar(const Entidad& obj){ anim.iniciar(obj); }
    void IdleRival::onSalir(const Entidad& obj){ (void)obj; }
    void IdleRival::onUpdate(const Entidad& obj,float dt){ (void)obj; anim.actualizar(dt); }

    // JAB
    JabRival::JabRival()
        :FSMBoss{},anim{{2,3},0.25f}
    {
        nombre = "JabRival";
    }
    FSM* JabRival::onInputs(Entidad& parent, CE::Vector2D& target)
    {
        if(!jugadorCerca(parent,target))
            return new IdleRival();
        anim.mirar(targetALaIzquierda(parent,target));
        return nullptr;
    }
    void JabRival::onEntrar(const Entidad& obj){ anim.iniciar(obj); }
    void JabRival::onSalir(const Entidad& obj){ (void)obj; }
    void JabRival::onUpdate(const Entidad& obj,float dt){ (void)obj; anim.actualizar(dt); }
}
