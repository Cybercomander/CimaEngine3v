#pragma once
#include "Stats.hpp"

namespace Round12
{
    // Conteo de protección tras una caída: para levantarse hay que presionar
    // J y K alternadas (la misma tecla seguida, p. ej. por repetición, no cuenta)
    class Conteo
    {
        public:
            explicit Conteo(int caidas_previas);
            void tick(float dt);
            void pulsar(char tecla);
            bool levantado() const { return !fuera() && n_validas >= necesarias; }
            bool fuera() const { return t >= 10.f; }
            int numero() const;
            int validas() const { return n_validas; }
            int requeridas() const { return necesarias; }
        private:
            int necesarias;
            int n_validas{0};
            char ultima{0};
            float t{0.f};
    };

    float probLevantarRival(const Atributos& a, int caidas_previas);
}
