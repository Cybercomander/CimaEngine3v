#pragma once
#include <set>
#include <vector>

namespace Round12
{
    // Teclas que siguen abajo. Descarta la repetición del sistema operativo
    // (KeyPressed repetidos al sostener) y al cambiar de juego dice cuáles soltar.
    class FiltroTeclas
    {
        public:
            // false si la tecla ya estaba abajo (repetición)
            bool presionar(int tecla) { return abajo.insert(tecla).second; }
            void soltar(int tecla) { abajo.erase(tecla); }
            std::vector<int> soltarTodas()
            {
                std::vector<int> v(abajo.begin(),abajo.end());
                abajo.clear();
                return v;
            }
        private:
            std::set<int> abajo;
    };
}
