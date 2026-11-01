#pragma once
#include "Stats.hpp"
#include <array>

namespace Round12
{
    // personalidad de cada rival (GDD: defensivo, fajador, estilista, completo)
    struct EstiloIA
    {
        float prob_guardia, prob_esquiva, distancia_ideal, agresividad;
        std::array<float,4> pesos_golpe; // jab, recto, gancho, uppercut
    };

    EstiloIA estilo(Personaje p);
    // u en [0,1): golpe según los pesos acumulados
    Golpe elegirGolpe(const std::array<float,4>& pesos, float u);
}
