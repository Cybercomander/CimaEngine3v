#include "ICParte.hpp"
#include <algorithm>

namespace IVJ
{
// Los componentes internos quedan construidos junto con la parte:
//  - el rastro empieza vacío
//  - la figura toma el tamaño y color recibidos
//  - la transformación se ubica en la posición inicial, sin velocidad ni giro
//  - el timer usa el retraso como límite; se acota a COLA_CAPACIDAD - 1 porque la cola
//    llega a guardar 'retraso + 1' posiciones justo antes de soltar la más antigua
//  - la parte todavía no persigue a nadie
ICParte::ICParte(const CE::Vector2D &pos_inicial, int w, int h, int retraso, const sf::Color &relleno)
    : CE::IComponentes{}, posiciones{}, parte{static_cast<float>(w), static_cast<float>(h), relleno, sf::Color::Black},
      pos{pos_inicial, CE::Vector2D{0.f, 0.f}, 0.f}, timer{std::clamp(retraso, 1, COLA_CAPACIDAD - 1)},
      hacerAccion{false}
{
    pos.pos_prev = pos_inicial;
    parte.figura.setPosition({pos_inicial.x, pos_inicial.y});
}
} // namespace IVJ
