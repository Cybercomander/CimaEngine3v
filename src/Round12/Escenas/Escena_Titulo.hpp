#pragma once
#include "EscenaR12.hpp"

namespace Round12
{
    // R12_Titulo: Demo / Historia / Controles / Volver al lanzador
    class Escena_Titulo : public EscenaR12
    {
        public:
            bool permitePausa() const override { return false; }
        protected:
            void reiniciar() override { opcion = 0; controles = false; }
            void actualizar(float) override {}
            void dibujarMundo() override {}
            void dibujarPantalla(sf::RenderTarget& t, sf::Vector2f tam) override;
            const char* musica() const override { return "musica_menu"; }
            void alPresionar(const std::string& accion, bool presionado) override;
        private:
            int opcion{0};
            bool controles{false};
    };
}
