#include "Escena_Indiana.hpp"
#include "Motor/Utils/Vector2D.hpp"
#include <Motor/Inputs/Botones.hpp>
#include <Motor/Render/Render.hpp>
#include <Motor/Camaras/Camaras.hpp>
#include <Motor/Camaras/CamarasGestor.hpp>
#include <Juego/objetos/Entidad.hpp>
#include <Juego/objetos/TileMap.hpp>
#include <Motor/Componentes/IComponentes.hpp>
#include <Juego/Sistemas/Sistemas.hpp>
#include <Motor/Primitivos/GestorAssets.hpp>
#include <algorithm>
#include <cstdlib>
#include <memory>
namespace IVJ
{
    Escena_Indiana::Escena_Indiana()
        :CE::Escena{}
    {
    }

    void Escena_Indiana::onInit()
    {
        if(inicializar)
        {
            registrarBotones(sf::Keyboard::Scancode::W,"arriba");
            registrarBotones(sf::Keyboard::Scancode::Up,"arriba");
            registrarBotones(sf::Keyboard::Scancode::S,"abajo");
            registrarBotones(sf::Keyboard::Scancode::Down,"abajo");
            registrarBotones(sf::Keyboard::Scancode::A,"izquierda");
            registrarBotones(sf::Keyboard::Scancode::Left,"izquierda");
            registrarBotones(sf::Keyboard::Scancode::D,"derecha");
            registrarBotones(sf::Keyboard::Scancode::Right,"derecha");

            //mapa de una capa: tileMapP7 cortado en tiles de 16x16
            tiles_layers.push_back(TileMap());
            if(!tiles_layers[0].loadTileMap(ASSETS "/mapas/indiana_layer1.txt"))
                exit(EXIT_FAILURE);
            tiles_layers[0].setScale({escala_mapa,escala_mapa});

            //un sprite por dirección, ambos de 148x125
            CE::GestorAssets::Get().agregarTextura(
                    "indiana_der",
                    ASSETS "/sprites/indiana/indianaDer.png",
                    CE::Vector2D{0.f,0.f},
                    CE::Vector2D{148.f,125.f});
            CE::GestorAssets::Get().agregarTextura(
                    "indiana_izq",
                    ASSETS "/sprites/indiana/indianaIzq.png",
                    CE::Vector2D{0.f,0.f},
                    CE::Vector2D{148.f,125.f});

            //entidad propia: no compartimos el jugador de Camaras/Sprites
            //para no acumular un segundo ISprite en él
            indiana = std::make_shared<Entidad>();
            indiana->getTransformada()->velocidad = CE::Vector2D{300.f,300.f};
            auto mapa = tiles_layers[0].getDimension().escala(escala_mapa);
            indiana->setPosicion(mapa.x/2.f,mapa.y/2.f);

            //este constructor referencia la textura del gestor sin copiarla,
            //así SistemaSpriteDireccion puede intercambiarla
            indiana->addComponente(std::make_shared<CE::ISprite>(
                    CE::GestorAssets::Get().getTextura("indiana_der"),
                    escala_indiana));
            indiana->addComponente(std::make_shared<CE::IControl>());

            inicializar=false;
        }
        //SnapVentana: el personaje se mueve en las 4 direcciones
        CE::GestorCamaras::Get().setCamaraActiva(1);
        CE::GestorCamaras::Get().getCamaraActiva().lockEnObjeto(indiana);
    }

    void Escena_Indiana::onFinal()
    {
        //soltar las teclas para que no siga caminando al regresar
        auto control = indiana->getComponente<CE::IControl>();
        control->arr = control->abj = control->der = control->izq = false;
        CE::GestorCamaras::Get().setCamaraActiva(0);
    }

    void Escena_Indiana::limitarAlMapa()
    {
        auto mapa = tiles_layers[0].getDimension().escala(escala_mapa);
        auto sprite = indiana->getComponente<CE::ISprite>();
        float mw = sprite->width*escala_indiana/2.f;
        float mh = sprite->height*escala_indiana/2.f;
        auto& pos = indiana->getTransformada()->posicion;
        pos.x = std::clamp(pos.x,mw,mapa.x-mw);
        pos.y = std::clamp(pos.y,mh,mapa.y-mh);
    }

    void Escena_Indiana::onUpdate(float dt)
    {
        SistemaMoverPlano(indiana,dt);
        limitarAlMapa();
        SistemaSpriteDireccion(*indiana,
                CE::GestorAssets::Get().getTextura("indiana_der"),
                CE::GestorAssets::Get().getTextura("indiana_izq"));
        //al final para que el sprite tome la posición ya movida
        indiana->onUpdate(dt);
    }

    void Escena_Indiana::onInputs(const CE::Botones& accion)
    {
        auto control = indiana->getComponente<CE::IControl>();
        bool presionado;
        switch(accion.getTipo())
        {
            case CE::Botones::TipoAccion::OnPress:
                presionado = true;
                break;
            case CE::Botones::TipoAccion::OnRelease:
                presionado = false;
                break;
            default:
                return;
        }
        if(accion.getNombre() == "arriba")
            control->arr = presionado;
        if(accion.getNombre() == "abajo")
            control->abj = presionado;
        if(accion.getNombre() == "derecha")
            control->der = presionado;
        if(accion.getNombre() == "izquierda")
            control->izq = presionado;
    }

    void Escena_Indiana::onRender()
    {
        for(auto& al: tiles_layers)
            CE::Render::Get().AddToDraw(al);

        for(auto& obj: objetos.getPool())
            CE::Render::Get().AddToDraw(*obj);
        CE::Render::Get().AddToDraw(*indiana);
    }
}
