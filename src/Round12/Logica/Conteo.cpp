#include "Conteo.hpp"
#include <algorithm>

namespace Round12
{
    Conteo::Conteo(int caidas_previas)
        :necesarias{10 + 6*caidas_previas}
    {
    }

    void Conteo::tick(float dt)
    {
        if(!levantado()) t = std::min(10.f, t + dt);
    }

    void Conteo::pulsar(char tecla)
    {
        if(fuera() || tecla == ultima) return;
        ultima = tecla;
        n_validas++;
    }

    int Conteo::numero() const
    {
        return std::min(10, (int)t + 1);
    }

    float probLevantarRival(const Atributos& a, int caidas_previas)
    {
        return std::clamp(a.resistencia/12.f - 0.2f*caidas_previas, 0.f, 0.95f);
    }
}
