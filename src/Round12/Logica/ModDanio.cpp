#include "ModDanio.hpp"

namespace Round12
{
    namespace { constexpr float DANIO_BASE[] = {3.f,5.f,8.f,9.f}; }

    float DanioBase::calcular(const ContextoGolpe& c) const
    {
        return DANIO_BASE[(int)c.g]*(0.5f + 0.1f*c.atk.poder);
    }
    float ModPocoAire::ajustar(const ContextoGolpe& c, float d) const { return c.b_atk.aire < 25.f ? d*0.5f : d; }
    float ModBloqueo::ajustar(const ContextoGolpe& c, float d) const { return c.bloqueado ? d*(1.f - 0.07f*c.def.defensa) : d; }
    float ModCombo::ajustar(const ContextoGolpe& c, float d) const { return c.cierra_combo ? d*1.25f : d; }
    float ModContragolpe::ajustar(const ContextoGolpe& c, float d) const { return c.contragolpe ? d*2.f : d; }
    float ModFrontera::ajustar(const ContextoGolpe& c, float d) const { return c.frontera ? d*2.f : d; }

    std::unique_ptr<ModDanio> cadenaDanio()
    {
        std::unique_ptr<ModDanio> d = std::make_unique<DanioBase>();
        d = std::make_unique<ModPocoAire>(std::move(d));
        d = std::make_unique<ModBloqueo>(std::move(d));
        d = std::make_unique<ModCombo>(std::move(d));
        d = std::make_unique<ModContragolpe>(std::move(d));
        return std::make_unique<ModFrontera>(std::move(d));
    }
}
