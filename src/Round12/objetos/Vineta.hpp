#pragma once
#include <SFML/Graphics.hpp>
namespace Round12::Vineta
{
    // concentración baja (< 40): la pantalla se oscurece en los bordes (GDD)
    void dibujar(sf::RenderTarget& t, sf::Vector2f tam, float concentracion);
}
