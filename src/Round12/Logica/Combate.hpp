#pragma once
#include "Stats.hpp"
#include "Jueces.hpp"
#include "Partida.hpp"
#include <random>
#include <vector>

namespace Round12
{
    struct EstadoFisico { bool guardia{false}, moviendo{false}; };

    // Fachada: reglas de una pelea (barras, golpes, rounds y decisión) en un solo lugar.
    // "diego" = true se refiere a Diego; false al rival.
    class Combate
    {
        public:
            Combate(Atributos diego, Atributos rival, Objetivo objetivo, bool segundo_aire);
            // cobra el aire del golpe; false si no alcanza (el golpe sale sin fuerza)
            bool lanzar(bool de_diego, Golpe g);
            bool esquivar(bool de_diego);
            // golpe que conectó: aplica daño y efectos; regresa el daño.
            // Agachado: jab, recto y gancho pasan por arriba (0); el uppercut entra x1.5.
            float recibir(bool a_diego, Golpe g, bool bloqueado, bool cierra_combo, bool contragolpe, bool frontera,
                          bool agachado = false);
            void tick(float dt, EstadoFisico diego, EstadoFisico rival);
            void caida(bool diego);
            void descanso(); // entre rounds: aire lleno y un poco de resistencia
            void nuevoRound();
            Barras& barras(bool diego) { return diego ? b_diego : b_rival; }
            const Atributos& atributos(bool diego) const { return diego ? a_diego : a_rival; }
            RoundStats& round() { return rounds_.back(); }
            const std::vector<RoundStats>& rounds() const { return rounds_; }
            Veredicto veredicto(std::mt19937& rng, Tarjetas* t = nullptr);
            bool segundoAireUsado() const { return segundo_usado; }
        private:
            Atributos a_diego, a_rival;
            Barras b_diego, b_rival;
            Objetivo objetivo;
            bool segundo_aire, segundo_usado{false};
            std::vector<RoundStats> rounds_;
    };
}
