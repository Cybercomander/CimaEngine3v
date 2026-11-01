#pragma once
#include "Stats.hpp"
#include <vector>

namespace Round12
{
    enum class Combo { NINGUNO, C112, C123, C34, FRONTERA };

    // Reconoce las combinaciones del GDD (1-1-2, 1-2-3, 3-4 y «Frontera» 1-2-3-4)
    // si los golpes llegan dentro de la ventana de tiempo
    class DetectorCombos
    {
        public:
            static constexpr float VENTANA = 0.45f;
            void reiniciar() { cadena.clear(); }
            Combo registrar(Golpe g, float t, bool concentracion_maxima);
        private:
            bool termina(std::initializer_list<int> seq) const;
            std::vector<int> cadena;
            float ultimo{-100.f};
    };
}
