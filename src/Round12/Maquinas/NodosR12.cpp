#include "NodosR12.hpp"
#include <Juego/objetos/Entidad.hpp>
#include <Juego/Componentes/IJComponentes.hpp>
#include <Round12/Componentes/ComponentesR12.hpp>

namespace Round12
{
    using IVJ::AccionBox;
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

        AccionBox accionDe(Golpe g) { return (AccionBox)((int)AccionBox::JAB + (int)g); }

        IVJ::FSM* golpe(Golpe g, const IBoxeador* b)
        {
            float prep = b->jugador ? 0.f : 0.5f - 0.02f*b->atr.velocidad;
            if(prep <= 0.f) return new GolpeR12(g);
            return new PrepararR12(g,prep);
        }

        // prioridad: esquiva, golpe, guardia (como en IVJ::NodosBoxeo)
        IVJ::FSM* combate(const CE::IControl& c, const IBoxeador* b, bool en_guardia)
        {
            if(b->puede_esquivar && (c.botones & BB::ESQ_IZQ)) return new EsquivaR12(true);
            if(b->puede_esquivar && (c.botones & BB::ESQ_DER)) return new EsquivaR12(false);
            if(c.botones & BB::GOLPES) return golpe(golpeDeBit(c.botones),b);
            if(!en_guardia && (c.botones & BB::GUARDIA)) return new GuardiaR12();
            return nullptr;
        }
    }

    float duracionCuadro(Golpe g)
    {
        constexpr float D[] = {0.07f,0.075f,0.08f,0.085f};
        return D[(int)g];
    }

    // ---- NodoR12 ----
    NodoR12::NodoR12(const char* n, AccionBox a, bool ciclo, std::vector<int> frames)
        :IVJ::FSM{},anim{a,ciclo,std::move(frames)}
    {
        nombre = n;
    }
    void NodoR12::onEntrar(const IVJ::Entidad& obj)
    {
        ente = &obj;
        box = obj.getComponente<IBoxeador>();
        sprite = obj.getComponente<CE::ISprite>();
        anim.entrar(obj);
        entrar();
    }
    void NodoR12::onSalir(const IVJ::Entidad& obj)
    {
        (void)obj;
        salir();
    }
    void NodoR12::onUpdate(const IVJ::Entidad& obj, float dt)
    {
        (void)obj;
        if(box)
        {
            dt /= escalaAnimacion(box->barras,box->atr);
            if(box->mira_fijo)
                if(auto* d = ente->getComponente<IVJ::IDireccion>()) anim.mirar(d->dir);
        }
        t += dt;
        anim.actualizar(dt);
        paso(dt);
    }

    // ---- Idle / Caminar ----
    IdleR12::IdleR12() :NodoR12{"Idle",AccionBox::GUARDIA,false,{0}} {}
    IVJ::FSM* IdleR12::onInputs(const CE::IControl& c)
    {
        if(auto n = combate(c,box,false)) return n;
        if(hayDireccion(c)) return new CaminarR12();
        return nullptr;
    }

    CaminarR12::CaminarR12() :NodoR12{"Caminar",AccionBox::CAMINAR,true} {}
    IVJ::FSM* CaminarR12::onInputs(const CE::IControl& c)
    {
        if(auto n = combate(c,box,false)) return n;
        if(!hayDireccion(c)) return new IdleR12();
        if(!box->mira_fijo)
        {
            int d = c.izq ? IVJ::IDireccion::IZQUIERDA : c.der ? IVJ::IDireccion::DERECHA :
                c.arr ? IVJ::IDireccion::ATRAS : IVJ::IDireccion::ADELANTE;
            anim.mirar(d);
        }
        return nullptr;
    }

    // ---- Guardia ----
    GuardiaR12::GuardiaR12() :NodoR12{"Guardia",AccionBox::GUARDIA,true} {}
    void GuardiaR12::entrar() { box->guardia = true; }
    void GuardiaR12::salir() { box->guardia = false; }
    IVJ::FSM* GuardiaR12::onInputs(const CE::IControl& c)
    {
        if(auto n = combate(c,box,true)) return n;
        if(!(c.botones & BB::GUARDIA)) return new IdleR12();
        return nullptr;
    }

    // ---- Preparar ----
    PrepararR12::PrepararR12(Golpe g, float duracion)
        :NodoR12{"Preparar",AccionBox::GUARDIA,false,{0}},g{g},duracion{duracion} {}
    void PrepararR12::paso(float) { box->destello = std::max(0.f,duracion - t); }
    void PrepararR12::salir() { box->destello = 0.f; }
    IVJ::FSM* PrepararR12::onInputs(const CE::IControl&)
    {
        return t >= duracion ? new GolpeR12(g) : nullptr;
    }

    // ---- Golpe ----
    GolpeR12::GolpeR12(Golpe g) :NodoR12{"Golpe",accionDe(g),false},g{g} {}
    void GolpeR12::entrar()
    {
        box->ultimo = g;
        box->golpe_lanzado = true; // la escena cobra el aire y decide si alcanza
        box->golpe_fallido = false;
    }
    void GolpeR12::salir() { box->golpe_activo = false; }
    void GolpeR12::paso(float)
    {
        // el impacto es el cuadro 3 de 6 (brazo extendido)
        if(!impacto && t >= 3.f*duracionCuadro(g))
        {
            impacto = true;
            box->golpe_activo = !box->golpe_fallido;
        }
    }
    IVJ::FSM* GolpeR12::onInputs(const CE::IControl& c)
    {
        cola.leer(c.botones);
        if(!anim.terminado()) return nullptr;
        if(cola.siguiente()) return new GolpeR12(golpeDeBit(cola.siguiente())); // combinación
        if(c.botones & BB::GUARDIA) return new GuardiaR12();
        return new IdleR12();
    }

    // ---- Esquiva ----
    EsquivaR12::EsquivaR12(bool izq)
        :NodoR12{izq ? "EsquivaIzq" : "EsquivaDer",izq ? AccionBox::ESQUIVA_IZQ : AccionBox::ESQUIVA_DER,false} {}
    void EsquivaR12::entrar() { box->esquivando = true; box->golpe_lanzado = false; }
    void EsquivaR12::salir() { box->esquivando = false; }
    IVJ::FSM* EsquivaR12::onInputs(const CE::IControl& c)
    {
        cola.leer(c.botones);
        if(!anim.terminado()) return nullptr;
        if(cola.siguiente()) return new GolpeR12(golpeDeBit(cola.siguiente())); // contragolpe
        return new IdleR12();
    }

    // ---- Aturdido ----
    AturdidoR12::AturdidoR12() :NodoR12{"Aturdido",AccionBox::GUARDIA,false,{0}} {}
    void AturdidoR12::entrar() { sprite->m_sprite.setColor(sf::Color(255,120,120)); }
    void AturdidoR12::salir() { sprite->m_sprite.setColor(box->tinte); }
    IVJ::FSM* AturdidoR12::onInputs(const CE::IControl&)
    {
        return t >= 0.25f ? new IdleR12() : nullptr;
    }

    // ---- Caído ----
    CaidoR12::CaidoR12() :NodoR12{"Caido",AccionBox::GUARDIA,false,{0}} {}
    void CaidoR12::entrar() { box->caido = true; box->guardia = box->esquivando = false; }
    void CaidoR12::paso(float)
    {
        // no hay cuadros de caída: el sprite se acuesta sobre la lona
        sprite->m_sprite.setRotation(sf::degrees(box->jugador ? -90.f : 90.f));
        sprite->m_sprite.move({0.f,20.f});
    }

    // ---- Levantarse ----
    LevantarseR12::LevantarseR12() :NodoR12{"Levantarse",AccionBox::GUARDIA,false,{0,1,2}} {}
    void LevantarseR12::salir() { box->caido = false; }
    IVJ::FSM* LevantarseR12::onInputs(const CE::IControl&)
    {
        return t >= 0.6f ? new IdleR12() : nullptr;
    }
}
