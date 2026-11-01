#pragma once
#include <Motor/App/ALayer.hpp>
#include <Motor/Primitivos/Escena.hpp>
#include <Round12/Logica/FiltroTeclas.hpp>
#include <functional>
#include <string>
#include <vector>

namespace Round12
{
    class EscenaR12;

    // modo de prueba (sin teclado): arranca en un paso y Diego juega solo
    struct PruebaR12 { std::string paso; bool autopiloto{false}; int rapido{1}; };

    // GameLayer de ROUND 12. Registra sus escenas (prefijo R12_) en el GestorEscenas
    // compartido; el Selector decide cuándo está activo. Esc = pausa (GDD).
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

            // entra a la última escena R12 (o al título la primera vez) sin reiniciarla
            void activar();
            // al salir: detiene audio y suelta los controles
            void desactivar();
            bool pideLanzador() const { return pedir_lanzador; }
            void limpiarPeticion() { pedir_lanzador = false; }
            void pedirLanzador() { pedir_lanzador = true; }
            static JuegoR12* instancia() { return actual; }

            static inline PruebaR12 prueba{};
            void terminar() { termino = true; }
        private:
            CE::Escena* escena() const;
            EscenaR12* escenaR12() const;
            bool manejarPausa(sf::Keyboard::Scancode tecla);
            void dibujarPausa();
        private:
            bool pedir_lanzador{false};
            std::string ultima{"R12_Titulo"};
            bool pausado{false};
            FiltroTeclas teclas;
            int opcion{0};
            // Comando: cada opción del menú de pausa es una acción
            std::vector<std::pair<std::string,std::function<void()>>> menu_pausa;
            static inline JuegoR12* actual{nullptr};
    };

    // lleva al paso actual de la historia (Partida) configurando su escena
    void irAPaso();
    void irATitulo();
    // reinicia la escena actual de ROUND 12 (mismo paso)
    void reiniciarPaso();
}
