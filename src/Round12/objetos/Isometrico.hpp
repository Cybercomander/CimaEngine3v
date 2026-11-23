#pragma once
#include <Round12/Logica/Iso.hpp>
#include <SFML/Graphics.hpp>

namespace Round12
{
    // dibuja un diorama isométrico con los tiles del paquete (de atrás hacia adelante)
    class DioramaIso
    {
        public:
            bool cargar(const std::string& ruta);
            void dibujar(sf::RenderTarget& t, Pt origen, float esc) const;
            const MatrizIso& matriz() const { return m; }
        private:
            MatrizIso m;
    };
}
