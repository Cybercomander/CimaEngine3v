#include "ICFigura.hpp"

namespace IVJ
{
// La figura nace con su tamaño y colores; el origen centrado hace que la posición
// asignada corresponda al centro del cuadro y no a su esquina superior izquierda
ICFigura::ICFigura(float w, float h, const sf::Color &relleno, const sf::Color &contorno)
    : CE::IComponentes{}, figura{sf::Vector2f{w, h}}
{
    figura.setFillColor(relleno);
    figura.setOutlineColor(contorno);
    figura.setOutlineThickness(2.f);
    figura.setOrigin({w / 2.f, h / 2.f});
}
} // namespace IVJ
