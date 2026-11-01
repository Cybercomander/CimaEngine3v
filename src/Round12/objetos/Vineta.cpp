#include "Vineta.hpp"
#include "FabricaR12.hpp"

namespace Round12::Vineta
{
    void dibujar(sf::RenderTarget& t, sf::Vector2f tam, float concentracion)
    {
        if(concentracion >= 40.f) return;
        // solo sobre el escenario (entre las bandas del HUD)
        float arriba = tam.y*56.f/720.f, alto = tam.y - 2.f*arriba;
        sf::Sprite s{textura("provisionales/sprites/vineta.png")};
        s.setScale({tam.x/288.f, alto/192.f});
        s.setPosition({0.f,arriba});
        s.setColor(sf::Color(255,255,255,(std::uint8_t)((40.f-concentracion)/40.f*255.f)));
        t.draw(s);
    }
}
