#pragma once
#include "../../Motor/Primitivos/Objetos.hpp"
#include <SFML/Graphics.hpp>

namespace IVJ
{
// Cabeza de la víbora. Es un CE::Objeto con un círculo propio como figura.
// La cabeza avanza según la velocidad de su ITransform; el cuerpo no es parte de
// la clase sino un componente ICPartesCuerpo que se le agrega desde la escena.
// Si el componente existe, onUpdate acomoda cada parte detrás de su líder
class EnteVibora : public CE::Objeto
{
  public:
    // radio: tamaño de la cabeza; relleno y contorno: colores del círculo
    explicit EnteVibora(float radio, const sf::Color &relleno, const sf::Color &contorno);
    ~EnteVibora() override {};

    // Mueve la cabeza y hace que las partes del cuerpo la persigan
    void onUpdate(float dt) override;
    // Dibuja el cuerpo (de la cola hacia la cabeza) y encima la cabeza
    void draw(sf::RenderTarget &target, sf::RenderStates state) const override;

    // Área visible del mundo (esquina superior izquierda + tamaño, en coordenadas del mundo).
    // Cuando la cabeza sale por completo por un borde, reaparece por el borde opuesto.
    // Con un área de tamaño 0 la cabeza se mueve sin límites
    void setArea(const sf::FloatRect &area_visible)
    {
        area = area_visible;
    }

  private:
    // Figura de la cabeza
    sf::CircleShape cabeza;
    // Radio de la cabeza; también es el margen para salir y entrar por los bordes
    float radio;
    // Área visible del mundo
    sf::FloatRect area;
};
} // namespace IVJ
