#pragma once
#include <Motor/App/ALayer.hpp>
#include <Motor/Camaras/Camaras.hpp>
#include <Juego/Juego.hpp>
#include <Round12/JuegoR12.hpp>
#include <Round12/Logica/FiltroTeclas.hpp>
#include <memory>
#include <string>
#include <vector>

// Proxy: el motor solo ve un GameLayer; por dentro se reenvía al juego elegido
// (CimaEngine Labs = src/Juego o ROUND 12 = src/Round12). F1, el menú "Juego"
// y la pantalla de lanzador cambian de juego en Debug y en Release.
class Selector : public CE::GameLayer
{
    public:
        enum class Modo { LANZADOR, LABS, ROUND12 };
        // ciclos_prueba > 0: alterna solo entre juegos y escenas, verifica y termina
        explicit Selector(Modo inicial, int ciclos_prueba = 0);
        void OnInit() override;
        bool EstaCorriendo() override;
        void OnInputs(std::optional<sf::Event>& eventos) override;
        void OnInputs(CE::InputBuffer& buffer) override;
        void OnInputs() override {}
        void OnUpdate(float dt) override;
        void OnRender() override;
        void cambiar(Modo m);
        int fallas() const { return n_fallas; }
    private:
        CE::GameLayer* activo() const;
        void menu();
        void pasoPrueba();
        void verificar();
        void falla(const std::string& motivo);
    private:
        std::unique_ptr<IVJ::Juego> labs;
        std::unique_ptr<Round12::JuegoR12> r12;
        Modo modo{Modo::LANZADOR};
        Modo inicial;
        Modo previo{Modo::LANZADOR};
        std::string escena_labs{"Init"};
        CE::InputBuffer filtrado{32};
        Round12::FiltroTeclas teclas_labs; // abajo en Labs: se sueltan al cambiar de juego
        void soltarLabs();
        // prueba de compatibilidad
        int ciclos;
        int frames{0};
        int cambios{0};
        int n_fallas{0};
        size_t n_escenas{0};
        std::vector<std::shared_ptr<CE::Camara>> camaras_labs;
        std::vector<std::string> claves_labs, claves_r12;
};
