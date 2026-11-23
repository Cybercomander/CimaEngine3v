#pragma once
#include "Stats.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace Round12
{
    enum class Habilidad : std::uint8_t { COMBOS = 1, ESQUIVA = 2, CONTRAGOLPE = 4, SEGUNDO_AIRE = 8, FRONTERA = 16 };
    enum class TipoPaso { CINEMATICA, RECORRIDO, PELEA, RUNNER, SANTA_FE, RESULTADOS, FIN };
    enum class Objetivo { SOBREVIVIR, GANAR, PERDER_FORZADO };

    struct Paso { TipoPaso tipo; std::string id; };

    // Iterador sobre los pasos de la historia (o del demo)
    class Historia
    {
        public:
            Historia() = default;
            explicit Historia(std::vector<Paso> pasos) : pasos{std::move(pasos)} {}
            const Paso& actual() const { return pasos.at(i); }
            bool avanzar() { if(i + 1 >= pasos.size()) return false; ++i; return true; }
            bool fin() const { return pasos.empty() || actual().tipo == TipoPaso::FIN; }
            size_t indice() const { return i; }
        private:
            std::vector<Paso> pasos{{TipoPaso::FIN,"fin"}};
            size_t i{0};
    };

    std::vector<Paso> secuenciaDemo();
    std::vector<Paso> secuenciaHistoria();
    const char* nombre(Habilidad h);

    // Singleton: progreso de la partida (atributos de Diego, habilidades y paso actual)
    class Partida
    {
        public:
            static Partida& Get() { static Partida p; return p; }
            void nueva(bool demo) { nueva(demo ? secuenciaDemo() : secuenciaHistoria(), demo); }
            void nueva(std::vector<Paso> pasos, bool demo);
            Atributos& diego() { return atr; }
            const Atributos& antesDelNivel() const { return antes; }
            bool tiene(Habilidad h) const { return habilidades & (std::uint8_t)h; }
            void desbloquear(Habilidad h);
            void mejorar(int Atributos::*campo, int delta);
            Historia& historia() { return hist; }
            bool esDemo() const { return demo; }
            unsigned semilla() const { return sem; }
            // regresa true si la historia avanzó; si no, el mismo paso se repite
            bool alTerminarPelea(Objetivo o, bool gano, bool de_pie);
            // tras la pantalla de resultados: nueva línea base para el siguiente nivel
            void alTerminarNivel();
            const std::vector<Habilidad>& nuevasHabilidades() const { return nuevas; }
            // texto libre del último nivel (tiempo, veredicto) para la pantalla de resultados
            std::string resumen;
        private:
            Partida() = default;
            Atributos atr{perfil(Personaje::DIEGO)};
            Atributos antes{perfil(Personaje::DIEGO)};
            std::uint8_t habilidades{0};
            std::vector<Habilidad> nuevas;
            Historia hist;
            bool demo{true};
            unsigned sem{12};
    };
}
