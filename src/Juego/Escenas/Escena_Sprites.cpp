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
#include <Juego/Maquinas/Naves/IdleJugadores.hpp>
#include <Juego/Maquinas/Bosses/IdleBoss.hpp>
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

        //Lab 8 animacinoes
        CE::GestorAssets::Get().agregarTextura(
                "nave_sheet",                               //key
                ASSETS "/sprites/naves/player_sheet.png",   //hoja
                CE::Vector2D{0.f,0.f},                      //inicio
                CE::Vector2D{320.f,64.f}                    //dim de la hoja
            );

        CE::GestorAssets::Get().agregarTextura(
                "boss_sheet",
                ASSETS "/sprites/bosses/VileCrustaceanIdleAttack.png",
                CE::Vector2D{0.f,0.f},
                CE::Vector2D{924.f,260.f}
            );

        auto trans = player->getTransformada();
        trans->velocidad = CE::Vector2D{500.f,500.f};
        player->setPosicion(300.f,300.f);
        
        //Lab 8 animaciones
        auto sprite = std::make_shared<CE::ISprite>(
                CE::GestorAssets::Get().getTextura("nave_sheet"), //textura
                64,64,                                      // dim
                1.f);                                       // escala
        
        player->addComponente(sprite);
        player->addComponente(std::make_shared<CE::IControl>());
        
        //lab 8 animaciones
        auto me = std::make_shared<IMaquinaEstado>();
        me->fsm = std::make_shared<IdleJugadores>();
        player->addComponente(me);
        //ejecuta onEntrar para inicializar variables
        player->setFSM(me->fsm);
        

        //boss entidad
        auto boss = std::make_shared<Entidad>();
        auto boss_sprite = std::make_shared<CE::ISprite>(
                CE::GestorAssets::Get().getTextura("boss_sheet"),
                154.f,130.f,
                1.f);
        auto boss_me = std::make_shared<IVJ::IMaquinaEstado>();
        //probar patch con nodo IdleBoss
        boss_me->fsm = std::make_shared<IVJ::IdleBoss>(6,0.25f);
        auto boss_target = std::make_shared<ITarget>(nullptr);
        boss_target->setTarget(*player);
        
        boss->addComponente(boss_sprite)
            .addComponente(boss_me)
            .addComponente(std::make_shared<IRangoAggro>(500.f))
            .addComponente(boss_target)
            .setPosicion(500.f,500.f);
        boss->setFSM(boss_me->fsm);

        objetos.agregarPool(boss);

        inicializar=false;
    }


    void Escena_Sprites::onFinal()
    {
        //reseteamos la camara a la estática al salir/cambiar de escena
        CE::GestorCamaras::Get().setCamaraActiva(0);
    }
    void Escena_Sprites::onUpdate(float dt)
    {
        player->inputFSM();
        player->onUpdate(dt);
        SistemaMover(player,dt);
        for(auto& obj: objetos.getPool())
        {
            obj->inputFSM();
            obj->onUpdate(dt);
            if(obj->tieneComponente<ITarget>())
            {
                auto& tpos = *obj->getComponente<ITarget>()->pos;
                SistemaNPCLookTarget(*obj,tpos);
            }
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
        {   
            //Lab 8 animaciones
            if(obj->tieneComponente<IRangoAggro>())
            {
                auto rango  = obj->getComponente<IRangoAggro>();
                auto pos = obj->getTransformada()->posicion;
                rango->marcador.setPosition({pos.x,pos.y});
                CE::Render::Get().AddToDraw(rango->marcador);
            }

            CE::Render::Get().AddToDraw(*obj);
        }
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
