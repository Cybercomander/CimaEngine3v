#pragma once
#include <Juego/objetos/Entidad.hpp>
#include <Round12/Logica/EstiloIA.hpp>
#include <random>

namespace Round12
{
    // Adaptador: convierte las decisiones de la IA en el mismo IControl que usa Diego
    class CerebroRival
    {
        public:
            CerebroRival(EstiloIA estilo, unsigned semilla) : base{estilo}, rng{semilla} {}
            void pensar(float dt, const IVJ::Entidad& yo, const IVJ::Entidad& diego, CE::IControl& salida);
            // golpe que viene (para el consejo del entrenador)
            Golpe proximo() const { return siguiente; }
        private:
            EstiloIA base;
            std::mt19937 rng;
            float reloj{0.f};
            Golpe siguiente{Golpe::JAB};
    };
}
