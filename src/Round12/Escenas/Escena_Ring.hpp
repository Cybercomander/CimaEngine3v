#pragma once
#include "EscenaR12.hpp"
#include <Juego/objetos/Entidad.hpp>
#include <Round12/Datos/Niveles.hpp>
#include <Round12/Logica/Combate.hpp>
#include <Round12/Logica/Combos.hpp>
#include <Round12/Maquinas/CerebroRival.hpp>
#include <memory>
#include <random>

namespace Round12
{
    class FasePelea;

    // R12_Ring: cualquier pelea del juego (sparrings, torneos, final). Las fases
    // (presentación, round, caída, esquina, decisión) son estados en FasesPelea.
    class Escena_Ring : public EscenaR12
    {
        public:
            Escena_Ring();
            ~Escena_Ring() override;

            // un cuadro de combate (lo usa la fase EnRound)
            void simular(float dt);
            void colocarEnEsquinas();
            void cambiarFase(std::unique_ptr<FasePelea> f);
            // resultado final: avanza la historia o repite la pelea
            void terminar(bool gano, bool de_pie, const std::string& como);

            ConfigPelea cfg;
            const Escenario* esc{nullptr};
            std::unique_ptr<Combate> combate;
            std::unique_ptr<CerebroRival> cerebro;
            std::unique_ptr<CerebroRival> piloto; // --autopiloto: Diego juega solo
            std::shared_ptr<IVJ::Entidad> diego, rival, carlos;
            DetectorCombos combos;
            std::mt19937 rng;
            int round_actual{1};
            float reloj{0.f};
            float tiempo_total{0.f};
            float consejo{0.f};      // segundos que se muestra el próximo golpe del rival
            int caidas_diego{0}, caidas_rival{0};
            int caida_pendiente{0};  // 1 = Diego, 2 = rival
            int seguidos{0};         // golpes de Diego que conectaron sin fallar (Frontera)
            struct Lanzamiento { bool combo{false}, contragolpe{false}, frontera{false}; } pendiente;
            bool esquivaba[2]{false,false};
        protected:
            void cargar() override;
            void reiniciar() override;
            void actualizar(float dt) override;
            void dibujarMundo() override;
            void dibujarPantalla(sf::RenderTarget& t, sf::Vector2f tam) override;
            const char* musica() const override { return "musica_pelea"; }
            void alPresionar(const std::string& accion, bool presionado) override;
        private:
            void lanzamiento(IVJ::Entidad& e, bool es_diego);
            void impacto(IVJ::Entidad& atk, IVJ::Entidad& def, bool atk_diego);
            std::unique_ptr<FasePelea> fase;
            std::unique_ptr<FasePelea> siguiente;
    };
}
