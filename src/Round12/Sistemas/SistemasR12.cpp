#include "SistemasR12.hpp"
#include <Juego/Componentes/IJComponentes.hpp>
#include <Round12/Componentes/ComponentesR12.hpp>
#include <algorithm>
#include <cmath>

namespace Round12
{
    float alcance(Golpe g)
    {
        constexpr float A[] = {30.f,30.f,24.f,20.f};
        return A[(int)g];
    }

    bool SistemaAlcance(IVJ::Entidad& atk, IVJ::Entidad& def, Golpe g)
    {
        auto pa = atk.getTransformada()->posicion, pd = def.getTransformada()->posicion;
        float dx = pd.x - pa.x;
        auto* dir = atk.getComponente<IVJ::IDireccion>();
        bool mira = !dir || (dx >= 0) == (dir->dir == IVJ::IDireccion::DERECHA);
        return mira && std::abs(dx) <= alcance(g) && std::abs(pd.y - pa.y) <= 8.f;
    }

    void SistemaMirarse(IVJ::Entidad& a, IVJ::Entidad& b)
    {
        bool a_izq = a.getTransformada()->posicion.x <= b.getTransformada()->posicion.x;
        if(auto* d = a.getComponente<IVJ::IDireccion>()) d->dir = a_izq ? IVJ::IDireccion::DERECHA : IVJ::IDireccion::IZQUIERDA;
        if(auto* d = b.getComponente<IVJ::IDireccion>()) d->dir = a_izq ? IVJ::IDireccion::IZQUIERDA : IVJ::IDireccion::DERECHA;
    }

    void SistemaSepararCuerpos(IVJ::Entidad& a, IVJ::Entidad& b, float minimo)
    {
        auto& pa = a.getTransformada()->posicion;
        auto& pb = b.getTransformada()->posicion;
        if(std::abs(pa.y - pb.y) > 10.f) return;
        float dx = pb.x - pa.x;
        if(std::abs(dx) >= minimo) return;
        float empuje = (minimo - std::abs(dx))/2.f * (dx >= 0 ? 1.f : -1.f);
        pa.x -= empuje;
        pb.x += empuje;
    }

    void SistemaCaminar(IVJ::Entidad& e, float dt, float x0, float x1, float y0, float y1)
    {
        auto* me = e.getComponente<IVJ::IMaquinaEstado>();
        auto* c = e.getComponente<CE::IControl>();
        auto* b = e.getComponente<IBoxeador>();
        auto& p = e.getTransformada()->posicion;
        if(me && c && b && me->fsm && me->fsm->getNombre() == "Caminar")
        {
            float v = velocidadPasos(b->atr);
            p.x += ((c->der ? 1.f : 0.f) - (c->izq ? 1.f : 0.f))*v*dt;
            p.y += ((c->abj ? 1.f : 0.f) - (c->arr ? 1.f : 0.f))*v*0.6f*dt;
        }
        p.x = std::clamp(p.x,x0,x1);
        p.y = std::clamp(p.y,y0,y1);
    }
}
