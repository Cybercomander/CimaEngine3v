#pragma once
#include "Geometria.hpp"

namespace Round12
{
    // Física del corredor (playa y circuito) sin gráficos. x crece hacia la meta,
    // y es la altura sobre la arena (0 = suelo). Obstáculos: Rect con y desde el suelo.
    class Runner
    {
        public:
            struct Config
            {
                float meta{9000.f};
                float tiempo{180.f};
                float orilla_ini{3000.f}, orilla_fin{6000.f}; // arena mojada
                bool mojada{true};
            };
            static constexpr float ANCHO = 14.f, ALTO = 52.f;
            static constexpr float LENTO = 40.f, RITMO = 60.f, MAXIMA = 85.f;

            explicit Runner(Config c) : cfg{c}, tiempo{c.tiempo} {}
            // ritmo: -1 frena (A), 0 normal, +1 acelera (D)
            void paso(float dt, int ritmo, bool saltar, bool agachar, bool en_sombra);
            // barrido de x_prev a x: no atraviesa obstáculos aunque dt sea grande
            bool chocaCon(const Rect& o);
            // una ola alcanza a Diego si no va en el aire
            bool ola();
            void checkpoint() { cp_x = x; cp_tiempo = tiempo; }
            void reaparecer();
            void beber() { sed = 0.f; }
            bool fallo() const { return tiempo <= 0.f || aire <= 0.f; }
            bool llego() const { return x >= cfg.meta; }
            bool enSuelo() const { return y <= 0.f; }
            float alto() const { return agachado ? ALTO/2.f : ALTO; }

            Config cfg;
            float x{0.f}, x_prev{0.f}, y{0.f}, vy{0.f}, vel{RITMO};
            float aire{100.f}, sed{0.f}, tiempo;
            bool agachado{false};
        private:
            float lento{0.f}, factor_lento{1.f}, invulnerable{0.f};
            float cp_x{0.f}, cp_tiempo{0.f};
    };
}
