#pragma once
#include "EscenaR12.hpp"
#include <Juego/objetos/Entidad.hpp>
#include <Round12/Datos/Niveles.hpp>
#include <Round12/Logica/Camino.hpp>
#include <Round12/Logica/Runner.hpp>
#include <Round12/Componentes/ComponentesR12.hpp>
#include <Round12/objetos/Isometrico.hpp>
#include <memory>
#include <vector>

namespace Round12
{
    // R12_Runner: Playa de Rosarito (carrera lateral de 3 tramos con olas y puestos de agua;
    // en la meta, la zona del muelle armada con tiles isométricos) y circuito atlético de
    // Rosarito (vueltas sobre la pista ovalada del escenario). A frena, D acelera, W salta,
    // S se agacha y J/K/L/I golpean (rompen cajas y botes).
    class Escena_Runner : public EscenaR12
    {
        protected:
            void cargar() override { usarCamara(); }
            void reiniciar() override;
            void actualizar(float dt) override;
            void dibujarMundo() override;
            void dibujarPantalla(sf::RenderTarget& t, sf::Vector2f tam) override;
            const char* musica() const override { return "musica_playa"; }
            void alPresionar(const std::string& accion, bool presionado) override;
        private:
            struct Cosa
            {
                Rect r;                          // px de arte; y = altura sobre el suelo
                TipoChoque choque{TipoChoque::BAJO};
                std::string objeto;              // objetos/<n>.png o provisionales/sprites/<n>.png
                bool provisional{false};
                float vel{0.f};
                bool activo{true};
                bool item{false};
                TipoItem tipo_item{TipoItem::BOTELLA};
            };
            void generar();
            bool enSombra() const;
            void terminar();
            void piloto();
            void animarDiego(float dt);
            void actualizarFinal(float dt);
            // px de arte del corredor -> escenario
            Pt alMundo(float x, float altura) const;
            void dibujarCosa(sf::RenderTarget& t, const Cosa& c) const;

            ConfigRunner cfg;
            bool circuito{false};
            std::unique_ptr<Runner> run;
            std::unique_ptr<Camino> pista;
            std::shared_ptr<IVJ::Entidad> diego, carlos;
            std::vector<Cosa> cosas;
            std::vector<Rect> sombras;
            std::vector<float> puestos;     // puestos de agua (checkpoints)
            std::vector<float> adornos;     // torre de salvavidas / tablas de surf al fondo
            size_t sig_puesto{0};
            std::vector<float> vueltas;     // tiempo de cada vuelta (circuito)
            float inicio_vuelta{0.f};
            float t_ola{0.f};
            float aviso_t{0.f};
            std::string aviso;
            float transcurrido{0.f};
            float golpe_t{0.f}, choque_t{0.f};
            Brazo brazo{Brazo::NADA};
            bool terminado{false};
            // zona final de la playa (diorama isométrico)
            bool en_final{false};
            DioramaIso final_iso;
            std::vector<Pt> paseo;
            size_t paseo_i{0};
    };
}
