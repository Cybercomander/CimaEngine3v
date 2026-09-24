#pragma once
#include "../../Motor/Componentes/IComponentes.hpp"
#include <SFML/Graphics.hpp>

namespace IVJ
{
// Componente con la figura visible de un objeto: un cuadro de SFML con su color.
// Solo guarda la forma; quien lo posee decide dónde colocarla y cuándo dibujarla
class ICFigura : public CE::IComponentes
{
  public:
    // w y h son el ancho y alto en píxeles; el origen queda en el centro del cuadro
    explicit ICFigura(float w, float h, const sf::Color &relleno, const sf::Color &contorno);
    ~ICFigura() override {};
    // Devuelve una copia independiente de la figura
    std::shared_ptr<IComponentes> clonar() const override
    {
        return std::make_shared<ICFigura>(*this);
    };

  public:
    // Forma que se dibuja en pantalla
    sf::RectangleShape figura;
};
} // namespace IVJ
