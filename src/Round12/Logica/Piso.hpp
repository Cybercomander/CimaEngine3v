#pragma once
#include "Geometria.hpp"
#include <vector>

namespace Round12
{
    // Adaptador: polígonos del escenario -> reglas de movimiento.
    // Se camina dentro de algún polígono y fuera de los bloqueos (muebles, objetos).
    class Piso
    {
        public:
            void agregar(std::vector<Pt> poligono) { poligonos.push_back(std::move(poligono)); }
            void bloquear(Rect r) { bloqueos.push_back(r); }
            void limpiar() { poligonos.clear(); bloqueos.clear(); }
            bool dentro(Pt p) const;
            // movimiento completo, si no solo x, si no solo y, si no se queda (se desliza por la orilla)
            Pt limitar(Pt antes, Pt despues) const;
            const std::vector<std::vector<Pt>>& lista() const { return poligonos; }
        private:
            std::vector<std::vector<Pt>> poligonos;
            std::vector<Rect> bloqueos;
    };

    bool dentroPoligono(const std::vector<Pt>& poli, Pt p);
}
