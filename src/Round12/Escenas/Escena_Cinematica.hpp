#pragma once
#include "EscenaR12.hpp"
#include <vector>

namespace Round12
{
    struct LineaDialogo { std::string orador; int pose; std::string fondo, texto; };

    // Iterador sobre las líneas de un archivo de diálogo
    class Dialogo
    {
        public:
            // formato por línea: orador|pose|fondo|texto  ('#' = comentario)
            bool cargar(const std::string& ruta);
            const LineaDialogo& actual() const { return lineas[i]; }
            bool siguiente() { return ++i < lineas.size(); }
            bool fin() const { return i >= lineas.size(); }
            void reiniciar() { i = 0; }
        private:
            std::vector<LineaDialogo> lineas;
            size_t i{0};
    };

    // R12_Cinematica: subtítulos sin voces (GDD); J avanza, Esc salta
    class Escena_Cinematica : public EscenaR12
    {
        public:
            bool permitePausa() const override { return false; }
        protected:
            void reiniciar() override;
            void actualizar(float dt) override;
            void dibujarMundo() override {}
            void dibujarPantalla(sf::RenderTarget& t, sf::Vector2f tam) override;
            const char* musica() const override { return "musica_menu"; }
            void alPresionar(const std::string& accion, bool presionado) override;
        private:
            Dialogo dialogo;
            float letras{0.f};
            bool terminar{false};
    };
}
