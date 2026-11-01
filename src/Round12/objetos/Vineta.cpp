#include "Vineta.hpp"
#include "FabricaR12.hpp"

namespace Round12::Vineta
{
    void dibujar(sf::RenderTarget& t, sf::Vector2f tam, float concentracion)
    {
        if(concentracion >= 40.f) return;
        sf::Sprite s{textura("provisionales/sprites/vineta.png")};
        s.setScale({tam.x/288.f, tam.y/192.f});
        s.setColor(sf::Color(255,255,255,(std::uint8_t)((40.f-concentracion)/40.f*255.f)));
        t.draw(s);
    }
}
