#pragma once
#include "EscenaR12.hpp"
#include <Juego/objetos/Entidad.hpp>
#include <Round12/Datos/Niveles.hpp>
#include <Round12/Logica/Runner.hpp>
#include <Round12/Componentes/ComponentesR12.hpp>
#include <memory>
#include <vector>

namespace Round12
{
    // R12_Runner: Playa de Rosarito (3 tramos, olas, puestos de agua) y circuito
    // atlético de Rosarito (vueltas contra el reloj). A frena, D acelera, W salta, S agacha.
    class Escena_Runner : public EscenaR12
    {
        protected:
            void cargar() override { usarCamara(); }
            void reiniciar() override;
            void actualizar(float dt) override;
            void dibujarMundo() override;
            void dibujarPantalla(sf::RenderTarget& t, sf::Vector2f tam) override;
            const char* musica() const override { return "musica_playa"; }
        private:
            struct Cosa
            {
                Rect r;                          // y = altura sobre la arena
                std::shared_ptr<IVJ::Entidad> e;
                float vel{0.f};
                bool activo{true};
                bool item{false};
                TipoItem tipo_item{TipoItem::BOTELLA};
            };
            void generar();
            bool enSombra() const;
            float pantallaY(float altura) const { return SUELO - altura; }
            void terminar();
            void piloto();

            static constexpr float SUELO = 176.f;
            ConfigRunner cfg;
            std::unique_ptr<Runner> run;
            std::shared_ptr<IVJ::Entidad> diego;
            std::vector<Cosa> cosas;
            std::vector<Rect> sombras;
            std::vector<float> puestos;     // puestos de agua (checkpoints)
            size_t sig_puesto{0};
            std::vector<float> vueltas;     // tiempo de cada vuelta (circuito)
            float inicio_vuelta{0.f};
            float t_ola{0.f};
            float aviso_t{0.f};
            std::string aviso;
            float transcurrido{0.f};
            bool terminado{false};
    };
}
