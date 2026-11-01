#include "CerebroRival.hpp"
#include <Juego/Maquinas/Boxeadores/NodosBoxeo.hpp>
#include <Round12/Componentes/ComponentesR12.hpp>
#include <cmath>

namespace Round12
{
    namespace BB = IVJ::BotonBox;

    void CerebroRival::pensar(float dt, const IVJ::Entidad& yo, const IVJ::Entidad& diego, CE::IControl& c)
    {
        auto* b = yo.getComponente<IBoxeador>();
        auto* bd = diego.getComponente<IBoxeador>();
        auto& yo_nc = const_cast<IVJ::Entidad&>(yo);
        auto& di_nc = const_cast<IVJ::Entidad&>(diego);
        auto p = yo_nc.getTransformada()->posicion, pd = di_nc.getTransformada()->posicion;
        float dx = pd.x - p.x, dy = pd.y - p.y, dist = std::abs(dx);

        EstiloIA e = base;
        if(b->quien == Personaje::JAVIER && b->barras.aire < 30.f) e.agresividad = 0.3f; // se cansa

        // movimiento continuo: buscar la distancia ideal y el carril de Diego
        c.izq = c.der = c.arr = c.abj = false;
        if(dist > e.distancia_ideal + 4.f) (dx > 0 ? c.der : c.izq) = true;
        else if(dist < e.distancia_ideal - 6.f) (dx > 0 ? c.izq : c.der) = true;
        if(std::abs(dy) > 3.f) (dy > 0 ? c.abj : c.arr) = true;

        reloj -= dt;
        if(reloj > 0.f) return;
        reloj = 0.25f;
        c.botones = 0; // soltar: así el siguiente golpe es una tecla nueva
        std::uniform_real_distribution<float> u(0.f,1.f);
        bool diego_ataca = bd && (bd->golpe_lanzado || bd->destello > 0.f);
        if(diego_ataca && u(rng) < e.prob_esquiva)
        {
            c.botones = u(rng) < 0.5f ? BB::ESQ_IZQ : BB::ESQ_DER;
            return;
        }
        siguiente = elegirGolpe(e.pesos_golpe,u(rng));
        bool alcanza = dist < 34.f && std::abs(dy) < 8.f;
        bool con_aire = b->barras.aire > costoAire(siguiente,b->atr) + 5.f;
        if(alcanza && con_aire && u(rng) < e.agresividad)
        {
            static const std::uint32_t BITS[] = {BB::JAB,BB::RECTO,BB::GANCHO,BB::UPPERCUT};
            c.botones = BITS[(int)siguiente];
            return;
        }
        if(u(rng) < e.prob_guardia || !con_aire) c.botones = BB::GUARDIA;
    }
}
