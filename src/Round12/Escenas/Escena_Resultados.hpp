#pragma once
#include "EscenaR12.hpp"

namespace Round12
{
    // R12_Resultados: mejora de estadísticas al terminar cada nivel (GDD)
    class Escena_Resultados : public EscenaR12
    {
        public:
            bool permitePausa() const override { return false; }
        protected:
            void actualizar(float) override {}
            void dibujarMundo() override {}
            void dibujarPantalla(sf::RenderTarget& t, sf::Vector2f tam) override;
            const char* musica() const override { return "musica_menu"; }
            void alPresionar(const std::string& accion, bool presionado) override;
    };
}
