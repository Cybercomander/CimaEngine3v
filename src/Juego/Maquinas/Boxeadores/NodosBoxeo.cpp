#include "NodosBoxeo.hpp"
#include <cmath>
#include <cstdlib>
#include <utility>

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
        // segundos por frame recomendados en ROUND12_Sprites_Animaciones/animaciones.json
        float duracion(AccionBox a)
        {
            switch(a)
            {
                case AccionBox::CAMINAR:  return 0.11f;
                case AccionBox::GUARDIA:  return 0.12f;
                case AccionBox::JAB:      return 0.07f;
                case AccionBox::RECTO:    return 0.075f;
                case AccionBox::GANCHO:   return 0.08f;
                case AccionBox::UPPERCUT: return 0.085f;
                default:                  return 0.09f; //esquivas
            }
        }
        // el bit de golpe más bajo presionado -> acción (jab tiene prioridad)
        AccionBox golpeDeBit(std::uint32_t bits)
        {
            if(bits & BotonBox::JAB)    return AccionBox::JAB;
            if(bits & BotonBox::RECTO)  return AccionBox::RECTO;
            if(bits & BotonBox::GANCHO) return AccionBox::GANCHO;
            return AccionBox::UPPERCUT;
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
        // acciones que comparten idle, caminar y bloqueo (prioridad: esquiva, golpe, guardia)
        FSM* accionDeCombate(const CE::IControl& c, bool en_guardia)
        {
            if(c.botones & BotonBox::ESQ_IZQ) return new EsquivaJugador(true);
            if(c.botones & BotonBox::ESQ_DER) return new EsquivaJugador(false);
            if(c.botones & BotonBox::GOLPES)  return new GolpeJugador(golpeDeBit(c.botones));
            if(!en_guardia && (c.botones & BotonBox::GUARDIA)) return new BloqueoJugador();
            return nullptr;
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
    AnimBoxeo::AnimBoxeo(AccionBox accion, bool ciclo, std::vector<int> frames)
        :accion{accion},anim{std::move(frames),duracion(accion),ciclo}
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

    // ---- ColaGolpes ----
    void ColaGolpes::leer(std::uint32_t botones)
    {
        auto golpes = botones & BotonBox::GOLPES;
        //lo que ya estaba presionado al entrar no cuenta como golpe nuevo
        if(primera)
        {
            sueltos = ~golpes;
            primera = false;
            return;
        }
        sueltos |= ~golpes;
        auto nuevos = golpes & sueltos;
        if(nuevos && !sig)
        {
            sig = nuevos;
            sueltos &= ~nuevos;
        }
    }

    // ---- IdleJugador ----
    IdleJugador::IdleJugador()
        :FSM{},anim{AccionBox::CAMINAR,false,{0}}
    {
        nombre = "IdleJugador";
    }
    FSM* IdleJugador::onInputs(const CE::IControl& control)
    {
        if(auto n = accionDeCombate(control,false))
            return n;
        if(hayDireccion(control))
            return new CaminarJugador();
        return nullptr;
    }
    void IdleJugador::onEntrar(const Entidad& obj){ anim.entrar(obj); }
    void IdleJugador::onSalir(const Entidad& obj){ (void)obj; }
    void IdleJugador::onUpdate(const Entidad& obj,float dt){ (void)obj; anim.actualizar(dt); }

    // ---- CaminarJugador ----
    CaminarJugador::CaminarJugador()
        :FSM{},anim{AccionBox::CAMINAR,true}
    {
        nombre = "CaminarJugador";
    }
    FSM* CaminarJugador::onInputs(const CE::IControl& control)
    {
        if(auto n = accionDeCombate(control,false))
            return n;
        if(!hayDireccion(control))
            return new IdleJugador();
        anim.mirar(dirDeControl(control,anim.direccion()));
        return nullptr;
    }
    void CaminarJugador::onEntrar(const Entidad& obj){ anim.entrar(obj); }
    void CaminarJugador::onSalir(const Entidad& obj){ (void)obj; }
    void CaminarJugador::onUpdate(const Entidad& obj,float dt){ (void)obj; anim.actualizar(dt); }

    // ---- BloqueoJugador ----
    BloqueoJugador::BloqueoJugador()
        :FSM{},anim{AccionBox::GUARDIA,true}
    {
        nombre = "BloqueoJugador";
    }
    FSM* BloqueoJugador::onInputs(const CE::IControl& control)
    {
        if(auto n = accionDeCombate(control,true))
            return n;
        if(!(control.botones & BotonBox::GUARDIA))
            return new IdleJugador();
        //con la guardia arriba solo puede voltear
        if(control.izq) anim.mirar(IDireccion::IZQUIERDA);
        else if(control.der) anim.mirar(IDireccion::DERECHA);
        return nullptr;
    }
    void BloqueoJugador::onEntrar(const Entidad& obj){ anim.entrar(obj); }
    void BloqueoJugador::onSalir(const Entidad& obj){ (void)obj; }
    void BloqueoJugador::onUpdate(const Entidad& obj,float dt){ (void)obj; anim.actualizar(dt); }

    // ---- GolpeJugador ----
    GolpeJugador::GolpeJugador(AccionBox golpe)
        :FSM{},anim{golpe,false}
    {
        static const char* nombres[] = {"","","JabJugador","RectoJugador","GanchoJugador","UppercutJugador"};
        nombre = nombres[(int)golpe];
    }
    FSM* GolpeJugador::onInputs(const CE::IControl& control)
    {
        cola.leer(control.botones);
        //el golpe no se interrumpe; al terminar sigue el combo o regresa a reposo
        if(!anim.terminado())
            return nullptr;
        if(cola.siguiente())
            return new GolpeJugador(golpeDeBit(cola.siguiente()));
        return new IdleJugador();
    }
    void GolpeJugador::onEntrar(const Entidad& obj){ anim.entrar(obj); }
    void GolpeJugador::onSalir(const Entidad& obj){ (void)obj; }
    void GolpeJugador::onUpdate(const Entidad& obj,float dt){ (void)obj; anim.actualizar(dt); }

    // ---- EsquivaJugador ----
    EsquivaJugador::EsquivaJugador(bool izquierda)
        :FSM{},anim{izquierda ? AccionBox::ESQUIVA_IZQ : AccionBox::ESQUIVA_DER,false}
    {
        nombre = izquierda ? "EsquivaIzqJugador" : "EsquivaDerJugador";
    }
    FSM* EsquivaJugador::onInputs(const CE::IControl& control)
    {
        cola.leer(control.botones);
        if(!anim.terminado())
            return nullptr;
        if(cola.siguiente())
            return new GolpeJugador(golpeDeBit(cola.siguiente())); //contragolpe
        return new IdleJugador();
    }
    void EsquivaJugador::onEntrar(const Entidad& obj){ anim.entrar(obj); }
    void EsquivaJugador::onSalir(const Entidad& obj){ (void)obj; }
    void EsquivaJugador::onUpdate(const Entidad& obj,float dt){ (void)obj; anim.actualizar(dt); }

    // ---- GuardiaRival ----
    GuardiaRival::GuardiaRival(float espera)
        :FSMBoss{},espera{espera},anim{AccionBox::GUARDIA,true}
    {
        nombre = "GuardiaRival";
    }
    FSM* GuardiaRival::onInputs(Entidad& parent, CE::Vector2D& target)
    {
        //acecho: siempre voltea hacia el jugador
        anim.mirar(dirHacia(parent,target));
        if(espera <= 0.f && jugadorCerca(parent,target))
            return new GolpeRival((AccionBox)((int)AccionBox::JAB + std::rand()%4));
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
    GolpeRival::GolpeRival(AccionBox golpe)
        :FSMBoss{},anim{golpe,false}
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
