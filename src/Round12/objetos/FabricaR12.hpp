#pragma once
#include <Juego/objetos/Entidad.hpp>
#include <Juego/objetos/TileMap.hpp>
#include <Round12/Componentes/ComponentesR12.hpp>
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

namespace Round12
{
    // textura de ROUND12_Assets (ruta relativa), cargada una vez y sin suavizado
    sf::Texture& textura(const std::string& relativa);

    // tilemap de ROUND12_Assets/motor/<nombre>.txt, cargado una vez
    const IVJ::TileMap& mapa(const std::string& nombre);

    // Fábrica de entidades de ROUND 12
    namespace FabricaR12
    {
        // pivote al centro del cuerpo: los pies quedan 26 px abajo de la posición
        constexpr float PIES_A_CENTRO = 26.f;
        std::shared_ptr<IVJ::Entidad> boxeador(Personaje p, float x, float pies_y, int dir,
                const std::shared_ptr<IVJ::FSM>& inicial, sf::Color tinte = sf::Color::White);
        std::shared_ptr<IVJ::Entidad> item(TipoItem t, float x, float y);
        std::shared_ptr<IVJ::Entidad> obstaculo(TipoObstaculo t, float x, float y);
        // sprite suelto (decoración): rect en la textura, pivote abajo al centro
        std::shared_ptr<IVJ::Entidad> decorado(const std::string& relativa, sf::IntRect rect, float x, float pies_y);
    }
}
