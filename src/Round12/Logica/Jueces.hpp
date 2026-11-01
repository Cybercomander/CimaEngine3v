#pragma once
#include <random>
#include <vector>

namespace Round12
{
    struct RoundStats
    {
        int conectados_diego{0}, conectados_rival{0};
        float danio_diego{0}, danio_rival{0}; // daño que CAUSÓ cada uno
        int caidas_diego{0}, caidas_rival{0}; // veces que CAYÓ cada uno
    };

    enum class Veredicto { DIEGO_UNANIME, DIEGO_DIVIDIDA, RIVAL_UNANIME, RIVAL_DIVIDIDA, EMPATE };

    // tarjetas de los 3 jueces (sistema de 10 puntos)
    struct Tarjetas { int diego[3]{}, rival[3]{}; };

    Veredicto decidir(const std::vector<RoundStats>& rounds, std::mt19937& rng, Tarjetas* tarjetas = nullptr);
    bool ganaDiego(Veredicto v);
    const char* texto(Veredicto v);
}
