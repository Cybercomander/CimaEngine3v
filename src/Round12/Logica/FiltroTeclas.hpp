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

    // Q+E casi juntas (<= 0.12 s) = agacharse. Q o E solas siguen siendo esquiva.
    // Recibe solo pulsaciones nuevas (ya filtradas de la repetición del SO).
    struct DetectorAgacharse
    {
        bool presionar(char tecla, float t)
        {
            float& otra = tecla == 'Q' ? t_e : t_q;
            bool junta = t - otra <= 0.12f;
            (tecla == 'Q' ? t_q : t_e) = t;
            if(junta) t_q = t_e = -1.f; // una pulsación doble no cuenta dos veces
            return junta;
        }
        float t_q{-1.f}, t_e{-1.f};
    };
}
