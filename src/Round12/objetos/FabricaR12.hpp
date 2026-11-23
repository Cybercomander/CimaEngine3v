#pragma once
#include <Juego/objetos/Entidad.hpp>
#include <Juego/objetos/TileMap.hpp>
#include <Round12/Componentes/ComponentesR12.hpp>
#include <Round12/Datos/Escenarios.hpp>
#include <Round12/objetos/ListaDibujo.hpp>
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

namespace Round12
{
    // textura de ROUND12_Assets (ruta relativa), cargada una vez. Si falta: cuadro magenta.
    // suave = true para lo que se reduce (decorados 1920x1080, rostros de 224)
    sf::Texture& textura(const std::string& relativa, bool suave = false);

    // tilemap de ROUND12_Assets/motor/<nombre>.txt, cargado una vez
    const IVJ::TileMap& mapa(const std::string& nombre);

    // 2 px de pantalla por px de arte (la vista 1920x1080 se ve a 1080x608)
    constexpr float ESCALA_PERSONAJE = 32.f/9.f;
    constexpr float ESCALA_OBJETO = 16.f/9.f;

    // Fábrica de entidades de ROUND 12
    namespace FabricaR12
    {
        // la posición de la entidad son los pies (origen 40,69 del cuadro de 80x80)
        std::shared_ptr<IVJ::Entidad> boxeador(Personaje p, float x, float pies_y, int dir,
                const std::shared_ptr<IVJ::FSM>& inicial, sf::Color tinte = sf::Color::White);
        // Carlos u otro personaje sin máquina de estados: la escena maneja su IAnimR12
        std::shared_ptr<IVJ::Entidad> npc(Personaje p, float x, float pies_y, int dir);
        std::shared_ptr<IVJ::Entidad> item(TipoItem t, float x, float y);
        std::shared_ptr<IVJ::Entidad> obstaculo(TipoObstaculo t, float x, float y);
        // sprite suelto (decoración): rect en la textura, pivote abajo al centro
        std::shared_ptr<IVJ::Entidad> decorado(const std::string& relativa, sf::IntRect rect, float x, float pies_y);
        // objeto de ROUND12_Assets/objetos/ (pivote 48,120)
        std::shared_ptr<NodoSprite> prop(const PropR12& p);
        sf::Sprite spriteObjeto(const std::string& objeto, Pt pie);
        // nodo de la lista de dibujo para una entidad (profundidad = y de los pies)
        std::shared_ptr<NodoDibujo> nodo(const std::shared_ptr<IVJ::Entidad>& e);
        // cuerdas frontales del escenario como un nodo más (profundidad = ring_frente_y)
        std::shared_ptr<NodoSprite> cuerdas(const EscenarioR12& e);
    }

    // copia el cuadro actual de IAnimR12 al sprite (hoja normal o de diálogo)
    void aplicarCuadro(IVJ::Entidad& e);
    // decorado del escenario a pantalla completa del mundo
    void dibujarCapa(sf::RenderTarget& t, const std::string& relativa);
}
