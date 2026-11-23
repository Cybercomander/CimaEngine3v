#include "Stats.hpp"
#include <algorithm>

namespace Round12
{
    namespace
    {
        constexpr float AIRE_BASE[] = {5.f,7.f,11.f,12.f};
        float factorAire(const Atributos& a) { return 1.3f - 0.05f*a.aire; }
    }

    Barras barrasIniciales(const Atributos& a)
    {
        Barras b;
        b.resistencia_max = 40.f + 8.f*a.resistencia;
        b.resistencia = b.resistencia_max;
        b.concentracion = std::min(BARRA_MAX, 50.f + 5.f*a.concentracion);
        return b;
    }

    float costoAire(Golpe g, const Atributos& a) { return AIRE_BASE[(int)g]*factorAire(a); }
    float costoEsquiva(const Atributos& a) { return 6.f*factorAire(a); }

    float regenAire(const Barras& b, const Atributos& a, bool guardia, bool moviendo)
    {
        float r = 8.f + 1.2f*a.aire;
        r *= guardia ? 1.5f : (moviendo ? 0.7f : 1.f);
        return b.sed > 70.f ? r*0.5f : r;
    }

    float factorSed(const Atributos& a) { return 1.5f - 0.1f*a.sed; }
    float sedPorSegundo(const Atributos& a) { return 0.5f*factorSed(a); }
    float sedPorGolpe(const Atributos& a) { return 0.6f*factorSed(a); }

    float escalaAnimacion(const Barras& b, const Atributos& a)
    {
        return (1.5f - 0.05f*a.velocidad) * (b.aire < 25.f ? 1.3f : 1.f);
    }

    float velocidadPasos(const Atributos& a) { return 30.f + 3.f*a.velocidad; }

    int limitar(int v) { return std::clamp(v,1,10); }
}
