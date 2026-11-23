#pragma once
#include "EscenaR12.hpp"
#include <Juego/objetos/Entidad.hpp>
#include <Round12/Datos/Niveles.hpp>
#include <Round12/Logica/Geometria.hpp>
#include <memory>
#include <vector>

namespace Round12
{
    // R12_SantaFe: parque de skate de Santa Fe, recorrido vertical saltando
    // rampas y escalones hasta arriba. A/D moverse, W saltar (gasta aire).
    class Escena_SantaFe : public EscenaR12
    {
        protected:
            void cargar() override;
            void reiniciar() override;
            void actualizar(float dt) override;
            void dibujarMundo() override;
            void dibujarPantalla(sf::RenderTarget& t, sf::Vector2f tam) override;
            const char* musica() const override { return "musica_playa"; }
        private:
            void piloto();
            void reaparecer();
            std::vector<Rect> plataformas;
            std::vector<size_t> puntos_agua;  // índices de plataformas con garrafón
            std::shared_ptr<IVJ::Entidad> diego;
            float x{160.f}, pies{632.f}, vy{0.f};
            bool en_suelo{true};
            float aire{100.f}, sed{0.f}, tiempo{120.f};
            float cp_x{160.f}, cp_pies{632.f}, cp_tiempo{120.f};
            float aviso_t{0.f};
            std::string aviso;
            bool terminado{false};
            const Rect* objetivo{nullptr}; // --autopiloto: escalón al que va
    };
}
