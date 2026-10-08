#include "Escena_Sprites.hpp"
#include "Motor/Utils/Vector2D.hpp"
#include <Motor/Inputs/Botones.hpp>
#include <Motor/Render/Render.hpp>
#include <Motor/Camaras/Camaras.hpp>
#include <Motor/Camaras/CamarasGestor.hpp>
#include <Juego/objetos/Entidad.hpp>
#include <Juego/Figuras/Figuras.hpp>
#include <Juego/objetos/TileMap.hpp>
#include <Motor/Componentes/IComponentes.hpp>
#include <Juego/Componentes/IJComponentes.hpp>
#include <Juego/Sistemas/Sistemas.hpp>
#include <Motor/Primitivos/GestorAssets.hpp>
#include <cstdlib>
#include <memory>
namespace IVJ
{
    Escena_Sprites::Escena_Sprites(std::shared_ptr<Entidad>& pref)
        :CE::Escena{},player{pref}
    {
    }

    void Escena_Sprites::onInit()
    {
        CE::GestorCamaras::Get().setCamaraActiva(2);
        CE::GestorCamaras::Get().getCamaraActiva().lockEnObjeto(player);
        if(!inicializar) return;
        registrarBotones(sf::Keyboard::Scancode::W,"arriba");
        registrarBotones(sf::Keyboard::Scancode::Up,"arriba");
        registrarBotones(sf::Keyboard::Scancode::S,"abajo");
        registrarBotones(sf::Keyboard::Scancode::Down,"abajo");
        registrarBotones(sf::Keyboard::Scancode::A,"izquierda");
        registrarBotones(sf::Keyboard::Scancode::Left,"izquierda");
        registrarBotones(sf::Keyboard::Scancode::D,"derecha");
        registrarBotones(sf::Keyboard::Scancode::Right,"derecha");
        registrarBotones(sf::Keyboard::Scancode::Enter,"aceptar");

        //cargar mapa 3 layers
        tiles_layers.push_back(TileMap()); //puro mar
        tiles_layers.push_back(TileMap()); // las islas
        tiles_layers.push_back(TileMap()); // los objetos

        if(!tiles_layers[0].loadTileMap(ASSETS "/mapas/playa_layer1.txt"))
            exit(EXIT_FAILURE);
        if(!tiles_layers[1].loadTileMap(ASSETS "/mapas/playa_layer2.txt"))
            exit(EXIT_FAILURE);
        if(!tiles_layers[2].loadTileMap(ASSETS "/mapas/playa_layer3.txt"))
            exit(EXIT_FAILURE);

        //Cargar el sprite
        CE::GestorAssets::Get().agregarTextura(
                "naveb",                                //llave
                ASSETS "/sprites/naves/player_b.png",   //path del sprite
                CE::Vector2D{0.f,0.f},                  //pos dentro de la hoja
                CE::Vector2D{64.f,64.f});               // dimensiones


        auto trans = player->getTransformada();
        trans->velocidad = CE::Vector2D{500.f,500.f};
        player->setPosicion(300.f,300.f);
        
        auto sprite = std::make_shared<CE::ISprite>(
                CE::GestorAssets::Get().getTextura("naveb"), //textura
                64,64,                                      // dim
                1.f);                                       // escala
        
        player->addComponente(sprite);
        player->addComponente(std::make_shared<CE::IControl>());


        inicializar=false;
    }


    void Escena_Sprites::onFinal()
    {
        //reseteamos la camara a la estática al salir/cambiar de escena
        CE::GestorCamaras::Get().setCamaraActiva(0);
    }
    void Escena_Sprites::onUpdate(float dt)
    {
        player->onUpdate(dt);
        SistemaMover(player,dt);
        for(auto& obj: objetos.getPool())
        {
            obj->onUpdate(dt);
        }
    }
    void Escena_Sprites::onInputs(const CE::Botones& accion)
    {
        switch(accion.getTipo())
        {
            case CE::Botones::TipoAccion::OnPress:
            {
                if(accion.getNombre() == "arriba")
                {
                    player->getComponente<CE::IControl>()->arr=true;
                }
                if(accion.getNombre() == "abajo")
                {
                    player->getComponente<CE::IControl>()->abj=true;
                }
                if(accion.getNombre() == "derecha")
                {
                    player->getComponente<CE::IControl>()->der=true;
                }
                if(accion.getNombre() == "izquierda")
                {
                    player->getComponente<CE::IControl>()->izq=true;
                }
                break;
            }
            case CE::Botones::TipoAccion::OnRelease:
            {
                if(accion.getNombre() == "arriba")
                {
                    player->getComponente<CE::IControl>()->arr=false;
                }
                if(accion.getNombre() == "abajo")
                {
                    player->getComponente<CE::IControl>()->abj=false;
                }
                if(accion.getNombre() == "derecha")
                {
                    player->getComponente<CE::IControl>()->der=false;
                }
                if(accion.getNombre() == "izquierda")
                {
                    player->getComponente<CE::IControl>()->izq=false;
                }
                break;
            }
            case CE::Botones::TipoAccion::None:
            {
                break;
            }
        }
    }
    void Escena_Sprites::onRender()
    {
        //renderizamos los layers primero para
        //que se pinten atrás de todo
        for(auto& al: tiles_layers)
            CE::Render::Get().AddToDraw(al);

        for(auto& obj: objetos.getPool())
            CE::Render::Get().AddToDraw(*obj);
        CE::Render::Get().AddToDraw(*player);

#if DEBUG
        auto cam = &CE::GestorCamaras::Get().getCamaraActiva();
        //solo las camaras con ventana tienen algo que dibujar
        auto csv = dynamic_cast<CE::CamaraSnapVentana*>(cam);
        if(!csv) return;
        auto csvpos = csv->getTransformada().posicion;
        sf::RectangleShape debugcam{{csv->m_vdim.x,csv->m_vdim.y}};
        debugcam.setOrigin({csv->m_vdim.x/2.f,csv->m_vdim.y/2.f});
        debugcam.setPosition({csvpos.x,csvpos.y});
        debugcam.setOutlineThickness(3.f);
        debugcam.setOutlineColor(sf::Color::Yellow);
        debugcam.setFillColor(sf::Color::Transparent);
        CE::Render::Get().AddToDraw(debugcam);
        //lineas solidas de foco donde se re-encuadra al jugador
        if(auto csf = dynamic_cast<CE::CamaraVentanaSnapFoco*>(cam))
        {
            sf::RectangleShape linea{{3.f,csv->m_vdim.y}};
            linea.setOrigin({1.5f,csv->m_vdim.y/2.f});
            linea.setFillColor(sf::Color::White);
            for(float lado : {-1.f,1.f})
            {
                linea.setPosition({csvpos.x+lado*csf->foco,csvpos.y});
                CE::Render::Get().AddToDraw(linea);
            }
        }
#endif
    }
}
