#include "NodosBoxeo.hpp"
#include <cmath>

namespace IVJ
{
    namespace
    {
        constexpr int ACCIONES_POR_DIR = 8;
        constexpr float ESPERA_RIVAL = 0.6f;

        int fila(int dir, AccionBox accion)
        {
            return dir*ACCIONES_POR_DIR + (int)accion;
        }
        bool hayDireccion(const CE::IControl& c)
        {
            return c.arr || c.abj || c.izq || c.der;
        }
        // lo horizontal tiene prioridad para que en diagonal se vea de lado
        int dirDeControl(const CE::IControl& c, int actual)
        {
            if(c.izq) return IDireccion::IZQUIERDA;
            if(c.der) return IDireccion::DERECHA;
            if(c.arr) return IDireccion::ATRAS;
            if(c.abj) return IDireccion::ADELANTE;
            return actual;
        }
        int dirHacia(Entidad& parent, CE::Vector2D& target)
        {
            auto d = target - parent.getTransformada()->posicion;
            if(std::abs(d.x) >= std::abs(d.y))
                return d.x < 0 ? IDireccion::IZQUIERDA : IDireccion::DERECHA;
            return d.y < 0 ? IDireccion::ATRAS : IDireccion::ADELANTE;
        }
        bool jugadorCerca(Entidad& parent, CE::Vector2D& target)
        {
            auto rango = parent.getComponente<IRangoAggro>();
            return rango && rango->estaDentroRango(parent,target);
        }
    }

    // ---- AnimBoxeo ----
    AnimBoxeo::AnimBoxeo(AccionBox accion, float frame_rate, bool ciclo)
        :accion{accion},anim{{0,1,2,3,4,5},frame_rate,ciclo}
    {
    }
    void AnimBoxeo::entrar(const Entidad& obj)
    {
        dir = obj.getComponente<IDireccion>();
        anim.setFila(fila(direccion(),accion));
        anim.iniciar(obj);
    }
    void AnimBoxeo::mirar(int d)
    {
        if(dir) dir->dir = d;
        anim.setFila(fila(d,accion));
    }
    int AnimBoxeo::direccion() const
    {
        return dir ? dir->dir : IDireccion::DERECHA;
    }

    // ---- GuardiaJugador (idle) ----
    GuardiaJugador::GuardiaJugador()
        :FSM{},anim{AccionBox::GUARDIA,0.12f,true}
    {
        nombre = "GuardiaJugador";
    }
    FSM* GuardiaJugador::onInputs(const CE::IControl& control)
    {
        if(control.acc)
            return new GolpeJugador(AccionBox::JAB);
        if(control.sacc)
            return new GolpeJugador(AccionBox::RECTO);
        if(hayDireccion(control))
            return new CaminarJugador(control.run);
        return nullptr;
    }
    void GuardiaJugador::onEntrar(const Entidad& obj){ anim.entrar(obj); }
    void GuardiaJugador::onSalir(const Entidad& obj){ (void)obj; }
    void GuardiaJugador::onUpdate(const Entidad& obj,float dt){ (void)obj; anim.actualizar(dt); }

    // ---- CaminarJugador / CorrerJugador ----
    CaminarJugador::CaminarJugador(bool turbo)
        :FSM{},turbo{turbo},anim{AccionBox::CAMINAR, turbo ? 0.06f : 0.11f, true}
    {
        nombre = turbo ? "CorrerJugador" : "CaminarJugador";
    }
    FSM* CaminarJugador::onInputs(const CE::IControl& control)
    {
        if(control.acc)
            return new GolpeJugador(AccionBox::JAB);
        if(control.sacc)
            return new GolpeJugador(AccionBox::RECTO);
        if(!hayDireccion(control))
            return new GuardiaJugador();
        if(control.run != turbo)
            return new CaminarJugador(control.run);
        anim.mirar(dirDeControl(control,anim.direccion()));
        return nullptr;
    }
    void CaminarJugador::onEntrar(const Entidad& obj){ anim.entrar(obj); }
    void CaminarJugador::onSalir(const Entidad& obj){ (void)obj; }
    void CaminarJugador::onUpdate(const Entidad& obj,float dt){ (void)obj; anim.actualizar(dt); }

    // ---- GolpeJugador ----
    GolpeJugador::GolpeJugador(AccionBox golpe)
        :FSM{},anim{golpe, golpe == AccionBox::JAB ? 0.07f : 0.075f, false}
    {
        nombre = golpe == AccionBox::JAB ? "JabJugador" : "RectoJugador";
    }
    FSM* GolpeJugador::onInputs(const CE::IControl& control)
    {
        (void)control;
        //el golpe no se interrumpe
        if(anim.terminado())
            return new GuardiaJugador();
        return nullptr;
    }
    void GolpeJugador::onEntrar(const Entidad& obj){ anim.entrar(obj); }
    void GolpeJugador::onSalir(const Entidad& obj){ (void)obj; }
    void GolpeJugador::onUpdate(const Entidad& obj,float dt){ (void)obj; anim.actualizar(dt); }

    // ---- GuardiaRival ----
    GuardiaRival::GuardiaRival(float espera)
        :FSMBoss{},espera{espera},anim{AccionBox::GUARDIA,0.12f,true}
    {
        nombre = "GuardiaRival";
    }
    FSM* GuardiaRival::onInputs(Entidad& parent, CE::Vector2D& target)
    {
        //acecho: siempre voltea hacia el jugador
        anim.mirar(dirHacia(parent,target));
        if(espera <= 0.f && jugadorCerca(parent,target))
            return new GolpeRival();
        return nullptr;
    }
    void GuardiaRival::onEntrar(const Entidad& obj){ anim.entrar(obj); }
    void GuardiaRival::onSalir(const Entidad& obj){ (void)obj; }
    void GuardiaRival::onUpdate(const Entidad& obj,float dt)
    {
        (void)obj;
        espera -= dt;
        anim.actualizar(dt);
    }

    // ---- GolpeRival ----
    GolpeRival::GolpeRival()
        :FSMBoss{},anim{AccionBox::JAB,0.07f,false}
    {
        nombre = "GolpeRival";
    }
    FSM* GolpeRival::onInputs(Entidad& parent, CE::Vector2D& target)
    {
        (void)parent;(void)target;
        if(anim.terminado())
            return new GuardiaRival(ESPERA_RIVAL);
        return nullptr;
    }
    void GolpeRival::onEntrar(const Entidad& obj){ anim.entrar(obj); }
    void GolpeRival::onSalir(const Entidad& obj){ (void)obj; }
    void GolpeRival::onUpdate(const Entidad& obj,float dt){ (void)obj; anim.actualizar(dt); }
}
