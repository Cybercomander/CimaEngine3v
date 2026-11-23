#include "ListaDibujo.hpp"
#include <Round12/Logica/Animaciones.hpp>

namespace Round12
{
    void ListaDibujo::dibujar(sf::RenderTarget& t) const
    {
        std::vector<float> p;
        p.reserve(nodos.size());
        for(auto& n : nodos) p.push_back(n->profundidad());
        for(int i : ordenPorProfundidad(p)) nodos[i]->dibujar(t);
    }
}
