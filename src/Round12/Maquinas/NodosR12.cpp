#include "NodosR12.hpp"
#include <Juego/objetos/Entidad.hpp>
#include <Juego/Componentes/IJComponentes.hpp>
#include <Round12/Componentes/ComponentesR12.hpp>

namespace Round12
{
    namespace BB = IVJ::BotonBox;

    namespace
    {
        bool hayDireccion(const CE::IControl& c) { return c.arr || c.abj || c.izq || c.der; }

        Golpe golpeDeBit(std::uint32_t bits)
        {
            if(bits & BB::JAB)    return Golpe::JAB;
            if(bits & BB::RECTO)  return Golpe::RECTO;
            if(bits & BB::GANCHO) return Golpe::GANCHO;
            return Golpe::UPPERCUT;
        }

        IVJ::FSM* golpe(Golpe g, const IBoxeador* b, Postura p)
        {
            if(b->en_aire) p = Postura::SALTANDO;
            float prep = b->jugador ? 0.f : 0.5f - 0.02f*b->atr.velocidad;
            if(prep <= 0.f) return new GolpeR12(g,p);
            return new PrepararR12(g,prep,p);
        }

        // prioridad: agacharse, esquiva, golpe, guardia (como en IVJ::NodosBoxeo)
        IVJ::FSM* combate(const CE::IControl& c, IBoxeador* b, bool en_guardia)
        {
            if(b->pide_agacharse) { b->pide_agacharse = false; return new AgachadoR12(); }
            if(b->puede_esquivar && (c.botones & BB::ESQ_IZQ)) return new EsquivaR12(true);
            if(b->puede_esquivar && (c.botones & BB::ESQ_DER)) return new EsquivaR12(false);
            if(c.botones & BB::GOLPES)
                return golpe(golpeDeBit(c.botones),b,hayDireccion(c) ? Postura::CAMINANDO : Postura::PIE);
            if(!en_guardia && (c.botones & BB::GUARDIA)) return new GuardiaR12();
            return nullptr;
        }
    }

    Brazo brazoDe(Golpe g) { return (Brazo)((int)Brazo::JAB + (int)g); }

    float duracionCuadro(Golpe g)
    {
        return msPorCuadro(elegirAccion(Postura::PIE,brazoDe(g)))/1000.f;
    }

    // ---- NodoR12 ----
    NodoR12::NodoR12(const char* n, Postura p, Brazo b) :IVJ::FSM{},postura{p},brazo{b}
    {
        nombre = n;
    }
    NodoR12::NodoR12(const char* n, Accion a) :NodoR12{n,Postura::PIE,Brazo::NADA} { fija = (int)a; }
    void NodoR12::onEntrar(const IVJ::Entidad& obj)
    {
        ente = &obj;
        box = obj.getComponente<IBoxeador>();
        anim = obj.getComponente<IAnimR12>();
        sprite = obj.getComponente<CE::ISprite>();
        if(fija >= 0) animar((Accion)fija,true);
        else animar(postura,brazo,true);
        entrar();
    }
    void NodoR12::onSalir(const IVJ::Entidad& obj)
    {
        (void)obj;
        if(!box) return; // setFSM sale del nodo inicial antes de haber entrado
        salir();
    }
    void NodoR12::animar(Postura p, Brazo b, bool reiniciar) { animar(elegirAccion(p,b),reiniciar); }
    void NodoR12::animar(Accion a, bool reiniciar) { if(anim) anim->poner(a,reiniciar); }
    bool NodoR12::animTerminada() const { return !anim || anim->termino; }
    void NodoR12::onUpdate(const IVJ::Entidad& obj, float dt)
    {
        (void)obj;
        if(box) dt /= escalaAnimacion(box->barras,box->atr);
        t += dt;
        if(anim)
        {
            if(auto* d = ente->getComponente<IVJ::IDireccion>()) anim->dir = d->dir;
            anim->actualizar(dt);
            auto r = rectCuadro(anim->accion,anim->dir,anim->cuadro);
            sprite->m_sprite.setTextureRect(sf::IntRect{{r.x,r.y},{r.w,r.h}});
        }
        paso(dt);
    }

    // ---- Idle / Caminar ----
    // en pelea (mira_fijo) espera en guardia; fuera del ring, idle
    IdleR12::IdleR12() :NodoR12{"Idle",Postura::PIE,Brazo::NADA} {}
    IVJ::FSM* IdleR12::onInputs(const CE::IControl& c)
    {
        animar(Postura::PIE,box->mira_fijo ? Brazo::GUARDIA : Brazo::NADA);
        if(auto n = combate(c,box,false)) return n;
        if(hayDireccion(c)) return new CaminarR12();
        return nullptr;
    }

    CaminarR12::CaminarR12() :NodoR12{"Caminar",Postura::CAMINANDO,Brazo::NADA} {}
    void CaminarR12::salir() { box->guardia = false; }
    IVJ::FSM* CaminarR12::onInputs(const CE::IControl& c)
    {
        bool g = c.botones & BB::GUARDIA;
        box->guardia = g;
        animar(Postura::CAMINANDO,g ? Brazo::GUARDIA : Brazo::NADA);
        if(box->pide_agacharse || (c.botones & (BB::ESQ_IZQ|BB::ESQ_DER|BB::GOLPES)))
            if(auto n = combate(c,box,true)) return n;
        if(!hayDireccion(c)) return g ? (IVJ::FSM*)new GuardiaR12() : new IdleR12();
        if(!box->mira_fijo)
            if(auto* d = ente->getComponente<IVJ::IDireccion>())
                d->dir = c.izq ? IZQUIERDA : c.der ? DERECHA : c.arr ? ATRAS : ADELANTE;
        return nullptr;
    }

    // ---- Guardia ----
    GuardiaR12::GuardiaR12() :NodoR12{"Guardia",Postura::PIE,Brazo::GUARDIA} {}
    void GuardiaR12::entrar() { box->guardia = true; }
    void GuardiaR12::salir() { box->guardia = false; }
    IVJ::FSM* GuardiaR12::onInputs(const CE::IControl& c)
    {
        if(auto n = combate(c,box,true)) return n;
        if(hayDireccion(c)) return new CaminarR12();
        if(!(c.botones & BB::GUARDIA)) return new IdleR12();
        return nullptr;
    }

    // ---- Agachado ----
    AgachadoR12::AgachadoR12() :NodoR12{"Agachado",Postura::AGACHADO,Brazo::NADA} {}
    void AgachadoR12::entrar() { box->agachado = true; }
    void AgachadoR12::salir() { box->agachado = false; box->guardia = false; }
    IVJ::FSM* AgachadoR12::onInputs(const CE::IControl& c)
    {
        bool g = c.botones & BB::GUARDIA;
        box->guardia = g;
        if(g) animar(Postura::AGACHADO,Brazo::GUARDIA);
        if(c.botones & BB::GOLPES) return golpe(golpeDeBit(c.botones),box,Postura::AGACHADO);
        return t >= 0.5f ? new IdleR12() : nullptr;
    }

    // ---- Preparar ----
    PrepararR12::PrepararR12(Golpe g, float duracion, Postura p)
        :NodoR12{"Preparar",p == Postura::AGACHADO ? p : Postura::PIE,Brazo::GUARDIA},g{g},duracion{duracion} {}
    void PrepararR12::paso(float) { box->destello = std::max(0.f,duracion - t); }
    void PrepararR12::salir() { box->destello = 0.f; }
    IVJ::FSM* PrepararR12::onInputs(const CE::IControl&)
    {
        return t >= duracion ? new GolpeR12(g,postura) : nullptr;
    }

    // ---- Golpe ----
    GolpeR12::GolpeR12(Golpe g, Postura p) :NodoR12{"Golpe",p,brazoDe(g)},g{g} {}
    void GolpeR12::entrar()
    {
        box->ultimo = g;
        box->golpe_lanzado = true; // la escena cobra el aire y decide si alcanza
        box->golpe_fallido = false;
        box->agachado = postura == Postura::AGACHADO;
    }
    void GolpeR12::salir() { box->golpe_activo = false; box->agachado = false; }
    void GolpeR12::paso(float)
    {
        // el impacto es el cuadro 3 de 6 (brazo extendido)
        if(!impacto && anim && anim->cuadro >= 3)
        {
            impacto = true;
            box->golpe_activo = !box->golpe_fallido;
        }
    }
    IVJ::FSM* GolpeR12::onInputs(const CE::IControl& c)
    {
        cola.leer(c.botones);
        if(!animTerminada()) return nullptr;
        // combinación: el siguiente golpe sale en la postura de ahora
        if(cola.siguiente()) return new GolpeR12(golpeDeBit(cola.siguiente()),
                hayDireccion(c) && postura != Postura::AGACHADO ? Postura::CAMINANDO : postura);
        if(c.botones & BB::GUARDIA) return new GuardiaR12();
        return new IdleR12();
    }

    // ---- Esquiva ----
    EsquivaR12::EsquivaR12(bool izq)
        :NodoR12{izq ? "EsquivaIzq" : "EsquivaDer",izq ? Postura::ESQUIVA_IZQ : Postura::ESQUIVA_DER,Brazo::NADA} {}
    void EsquivaR12::entrar() { box->esquivando = true; box->golpe_lanzado = false; }
    void EsquivaR12::salir() { box->esquivando = false; }
    void EsquivaR12::paso(float)
    {
        // con Shift la esquiva sale cubierta (esquiva_*_guardia)
        if(auto* c = ente->getComponente<CE::IControl>())
            if((c->botones & BB::GUARDIA) && brazo == Brazo::NADA) { brazo = Brazo::GUARDIA; animar(postura,brazo); }
    }
    IVJ::FSM* EsquivaR12::onInputs(const CE::IControl& c)
    {
        cola.leer(c.botones);
        if(box->pide_agacharse) { box->pide_agacharse = false; return new AgachadoR12(); }
        // contragolpe desde la esquiva: esquiva_<lado>_<golpe>
        if(cola.siguiente()) return new GolpeR12(golpeDeBit(cola.siguiente()),postura);
        if(!animTerminada()) return nullptr;
        return new IdleR12();
    }

    // ---- Aturdido (recibir_golpe) ----
    AturdidoR12::AturdidoR12() :NodoR12{"Aturdido",Accion::RECIBIR_GOLPE} {}
    IVJ::FSM* AturdidoR12::onInputs(const CE::IControl&)
    {
        return t >= 0.25f ? new IdleR12() : nullptr;
    }

    // ---- Caído ----
    CaidoR12::CaidoR12() :NodoR12{"Caido",Accion::CAER} {}
    void CaidoR12::entrar()
    {
        box->caido = true;
        box->guardia = box->esquivando = box->agachado = false;
        // la animación caer queda en su último cuadro, sobre la lona
    }

    // ---- Levantarse ----
    LevantarseR12::LevantarseR12() :NodoR12{"Levantarse",Accion::LEVANTARSE} {}
    void LevantarseR12::salir() { box->caido = false; }
    IVJ::FSM* LevantarseR12::onInputs(const CE::IControl&)
    {
        return animTerminada() ? new IdleR12() : nullptr;
    }
}
