#pragma once
#include <cmath>

namespace Round12
{
    // cuerda: la tecla cuenta si cae dentro de la tolerancia de un pulso de la música
    inline int pulsoMasCercano(float t, float periodo = 0.5f) { return (int)std::lround(t/periodo); }
    inline bool enPulso(float t, float periodo = 0.5f, float tolerancia = 0.12f)
    {
        return std::fabs(t - pulsoMasCercano(t,periodo)*periodo) <= tolerancia;
    }
}
