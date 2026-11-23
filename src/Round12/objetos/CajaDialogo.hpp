#pragma once
#include <Round12/Logica/LineaDialogo.hpp>
#include <SFML/Graphics.hpp>

namespace Round12
{
    // Caja de diálogo abajo del escenario: rostro de 224 (Diego y Carlos, la boca se mueve
    // mientras se escribe) o retrato de 96 (los demás), nombre y texto con máquina de escribir.
    // vista: TRESCUARTOS en cinemáticas, PERFIL en globos dentro del juego.
    void dibujarCajaDialogo(sf::RenderTarget& t, sf::Vector2f tam, const LineaDialogo& l,
                            float letras, Vista vista, float tiempo);
    // largo del texto en caracteres (UTF-8)
    size_t largoTexto(const std::string& utf8);
}
