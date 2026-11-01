#pragma once
#include <Motor/App/ALayer.hpp>
#include <Motor/Primitivos/Escena.hpp>
#include <string>

namespace Round12
{
    // GameLayer de ROUND 12. Registra sus escenas (prefijo R12_) en el GestorEscenas
    // compartido; el Selector decide cuándo está activo.
    class JuegoR12 : public CE::GameLayer
    {
        public:
            JuegoR12();
            ~JuegoR12() override {};
            void OnInit() override;
            void OnInputs(std::optional<sf::Event>& eventos) override;
            void OnInputs(CE::InputBuffer& buffer) override;
            void OnInputs() override {}
            void OnUpdate(float dt) override;
            void OnRender() override;

            // entra a la última escena R12 (o al título la primera vez)
            void activar();
            // al salir: detiene audio y suelta los controles
            void desactivar();
            bool pideLanzador() const { return pedir_lanzador; }
            void limpiarPeticion() { pedir_lanzador = false; }
            void pedirLanzador() { pedir_lanzador = true; }
            static JuegoR12* instancia() { return actual; }
        private:
            CE::Escena* escena() const;
            bool pedir_lanzador{false};
            std::string ultima{"R12_Titulo"};
            static inline JuegoR12* actual{nullptr};
    };
}
