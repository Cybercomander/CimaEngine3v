#include "CPartesCuerpo.hpp"

namespace IVJ
{
// El cuerpo nace con 'num_partes' partes apiladas en la posición inicial;
// se despliegan solas conforme la cabeza avanza, porque cada una espera su retraso
ICPartesCuerpo::ICPartesCuerpo(int w, int h, int num_partes, const CE::Vector2D &pos_inicial)
    : CE::IComponentes{}, partes{}, width{w}, height{h}
{
    partes.reserve(num_partes);
    for (int i = 0; i < num_partes; i++)
        agregarParte(pos_inicial);
}

// La parte nueva aparece donde está la última parte, que es el líder al que perseguirá.
// Los colores alternan entre dos tonos de verde para distinguir los segmentos
void ICPartesCuerpo::agregarParte(const CE::Vector2D &pos_cabeza)
{
    CE::Vector2D pos_nueva = partes.empty() ? pos_cabeza : partes.back().pos.posicion;
    sf::Color color = (partes.size() % 2 == 0) ? sf::Color{60, 170, 60} : sf::Color{110, 210, 90};
    partes.emplace_back(pos_nueva, width, height, RETRASO_PARTE, color);
}
} // namespace IVJ
