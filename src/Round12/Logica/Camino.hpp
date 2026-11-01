#pragma once
#include "Geometria.hpp"
#include <vector>

namespace Round12
{
    // Adaptador: la distancia 1D del corredor -> un punto sobre un circuito cerrado.
    class Camino
    {
        public:
            explicit Camino(std::vector<Pt> puntos_cerrados);
            float largo() const { return total; }
            Pt punto(float s) const;
            Pt tangente(float s) const; // unitaria
        private:
            std::vector<Pt> pts;
            std::vector<float> acum; // distancia al inicio de cada tramo
            float total{0.f};
            int tramo(float& s) const;
    };
}
