#include "SistemasR12.hpp"
#include <Juego/Componentes/IJComponentes.hpp>
#include <Round12/Componentes/ComponentesR12.hpp>
#include <Round12/Logica/Animaciones.hpp>
#include <algorithm>
#include <cmath>

namespace Round12
{
    float alcance(Golpe g)
    {
        constexpr float A[] = {30.f,30.f,24.f,20.f};
        return A[(int)g]*ESCALA_MUNDO;
    }

    bool SistemaAlcance(IVJ::Entidad& atk, IVJ::Entidad& def, Golpe g)
    {
        auto pa = atk.getTransformada()->posicion, pd = def.getTransformada()->posicion;
        float dx = pd.x - pa.x, dy = pd.y - pa.y;
        auto* dir = atk.getComponente<IVJ::IDireccion>();
        int hacia = direccionHacia(dx,dy);
        bool mira = !dir || hacia < 0 || dir->dir == hacia;
        return mira && std::abs(dx) <= alcance(g) && std::abs(dy) <= 28.f;
    }

    void SistemaMirarse(IVJ::Entidad& a, IVJ::Entidad& b)
    {
        auto pa = a.getTransformada()->posicion, pb = b.getTransformada()->posicion;
        int d = direccionHacia(pb.x - pa.x, pb.y - pa.y);
        if(d < 0) return;
        static const int OPUESTA[] = {ATRAS,ADELANTE,IZQUIERDA,DERECHA};
        if(auto* da = a.getComponente<IVJ::IDireccion>()) da->dir = d;
        if(auto* db = b.getComponente<IVJ::IDireccion>()) db->dir = OPUESTA[d];
    }

    void SistemaSepararCuerpos(IVJ::Entidad& a, IVJ::Entidad& b, float minimo)
    {
        auto& pa = a.getTransformada()->posicion;
        auto& pb = b.getTransformada()->posicion;
        if(std::abs(pa.y - pb.y) > 36.f) return;
        float dx = pb.x - pa.x;
        if(std::abs(dx) >= minimo) return;
        float empuje = (minimo - std::abs(dx))/2.f * (dx >= 0 ? 1.f : -1.f);
        pa.x -= empuje;
        pb.x += empuje;
    }

    namespace
    {
        // desplazamiento deseado este cuadro (0 si el nodo no camina)
        CE::Vector2D paso(IVJ::Entidad& e, float dt)
        {
            auto* me = e.getComponente<IVJ::IMaquinaEstado>();
            auto* c = e.getComponente<CE::IControl>();
            auto* b = e.getComponente<IBoxeador>();
            if(!me || !c || !b || !me->fsm) return {0.f,0.f};
            const auto& n = me->fsm->getNombre();
            auto* anim = e.getComponente<IAnimR12>();
            bool golpe_caminando = n == "Golpe" && anim && std::string(nombre(anim->accion)).rfind("caminar_",0) == 0;
            if(n != "Caminar" && !golpe_caminando) return {0.f,0.f};
            float v = velocidadPasos(b->atr)*ESCALA_MUNDO*(golpe_caminando ? 0.5f : 1.f);
            return {((c->der ? 1.f : 0.f) - (c->izq ? 1.f : 0.f))*v*dt,
                    ((c->abj ? 1.f : 0.f) - (c->arr ? 1.f : 0.f))*v*0.75f*dt};
        }
    }

    void SistemaCaminar(IVJ::Entidad& e, float dt, const Piso& piso)
    {
        auto& p = e.getTransformada()->posicion;
        auto d = paso(e,dt);
        auto r = piso.limitar({p.x,p.y},{p.x + d.x,p.y + d.y});
        p.x = r.x; p.y = r.y;
    }

    void SistemaCaminar(IVJ::Entidad& e, float dt, float x0, float x1, float y0, float y1)
    {
        auto& p = e.getTransformada()->posicion;
        auto d = paso(e,dt);
        p.x = std::clamp(p.x + d.x,x0,x1);
        p.y = std::clamp(p.y + d.y,y0,y1);
    }
}
