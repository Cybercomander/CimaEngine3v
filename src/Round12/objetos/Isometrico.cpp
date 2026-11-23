#include "Isometrico.hpp"
#include <Round12/objetos/FabricaR12.hpp>
#include <iostream>

namespace Round12
{
    bool DioramaIso::cargar(const std::string& ruta)
    {
        auto r = leerIso(ruta);
        if(!r) { std::cerr<<"[iso] no se pudo leer "<<ruta<<"\n"; m = {}; return false; }
        m = *r;
        return true;
    }

    void DioramaIso::dibujar(sf::RenderTarget& t, Pt origen, float esc) const
    {
        auto& atlas = textura("tilesets/atlas_isometrico_64x48.png");
        sf::Sprite s{atlas};
        s.setScale({esc,esc});
        s.setOrigin({32.f,0.f});
        for(int suma = 0; suma <= m.cols + m.filas - 2; ++suma)
            for(int c = 0; c < m.cols; ++c)
            {
                int f = suma - c;
                if(f < 0 || f >= m.filas) continue;
                int i = m.en(c,f);
                if(i < 0) continue;
                s.setTextureRect({{(i%8)*64,(i/8)*48},{64,48}});
                Pt p = celdaIso(c,f,origen,esc);
                s.setPosition({p.x,p.y});
                t.draw(s);
            }
    }
}
