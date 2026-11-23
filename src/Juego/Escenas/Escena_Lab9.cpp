#include "Escena_Lab9.hpp"
#include <Motor/Inputs/Botones.hpp>
#include <Motor/Render/Render.hpp>
#include <Motor/Camaras/Camaras.hpp>
#include <Motor/Camaras/CamarasGestor.hpp>
#include <Motor/Primitivos/GestorAssets.hpp>
#include <Motor/Componentes/IComponentes.hpp>
#include <Juego/objetos/Entidad.hpp>
#include <Juego/objetos/TileMap.hpp>
#include <Juego/Componentes/IJComponentes.hpp>
#include <Juego/Sistemas/Sistemas.hpp>
#include <Juego/Maquinas/Naves/IdleJugadores.hpp>
#include <Juego/Maquinas/Bosses/IdleBoss.hpp>
#include <cstdlib>
#include <memory>

namespace IVJ
{
    Escena_Lab9::Escena_Lab9()
        :CE::Escena{},player{std::make_shared<Entidad>()}
    {
    }

    void Escena_Lab9::onInit()
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

        //cargar mapa
        tiles_layers.push_back(TileMap());
        tiles_layers.push_back(TileMap());
        tiles_layers.push_back(TileMap());

        //lab 9 colisiones: layer con "c"/"n"; los tiles con c van al pool con IBoundingBox
        if(!tiles_layers[0].loadTileMap(ASSETS "/mapas/playa_col_layer1.txt",objetos))
            exit(EXIT_FAILURE);
        if(!tiles_layers[1].loadTileMap(ASSETS "/mapas/playa_layer2.txt"))
            exit(EXIT_FAILURE);
        if(!tiles_layers[2].loadTileMap(ASSETS "/mapas/playa_layer3.txt"))
            exit(EXIT_FAILURE);

        //Lab 8 animaciones
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
        //lab 9 colisiones: primera forma, componente individual
        player->addComponente(std::make_shared<CE::IBoundingBox>(
            CE::Vector2D{64.f,64.f} //dimensiones del sprite
        ));

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
                154,130,
                1.f);
        auto boss_me = std::make_shared<IVJ::IMaquinaEstado>();
        //nodo inicial IdleBoss: cambia a AttackBoss al acercarse el jugador
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

        //lab 9 colisiones: objeto individual tomado del atlas
        auto techo = std::make_shared<Entidad>();
        techo->getStats()->hp_max=100;
        techo->getStats()->hp=100;
        techo->getStats()->def=150;

        //cargar todo el atlas para poder poner cosas individuales
        CE::GestorAssets::Get().agregarTextura("atlas_playa",
                ASSETS "/atlas/playa_atlas.png",
                CE::Vector2D{0.f,0.f},
                CE::Vector2D{2048,768});

        auto techo_sprite = std::make_shared<CE::ISprite>(
            CE::GestorAssets::Get().getTextura("atlas_playa"),
            128,256, //dim
            1.f);
        //acomodar el sprite en el area deseada del atlas
        techo_sprite->m_sprite.setTextureRect(
                sf::IntRect{
                {64*28,64*6}, //220 indice
                {128,256} //dos tiles, 4 tiles en dimensiones
                });
        //acomodar el pivote para centrar bien el bounding box en el centro
        //128/2 = 64, 256/2 = 128
        techo_sprite->m_sprite.setOrigin({64,128});
        techo->addComponente(techo_sprite)
            .addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{128,256}));

        techo->setPosicion(1300,175);

        objetos.agregarPool(techo);

        inicializar=false;
    }

    void Escena_Lab9::onFinal()
    {
        //reseteamos la camara a la estática al salir/cambiar de escena
        CE::GestorCamaras::Get().setCamaraActiva(0);
    }

    void Escena_Lab9::onUpdate(float dt)
    {
        player->inputFSM();   //ejecutar los inputs de la maquina
        player->onUpdate(dt); //aquí se actualiza la maquina (<Entidad>)
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
            //lab 9: los objetos con IBoundingBox funcionan como paredes
            SistemaColAABBMid(*player, *obj, true);
        }
    }

    void Escena_Lab9::onInputs(const CE::Botones& accion)
    {
        if(accion.getTipo() == CE::Botones::TipoAccion::None) return;
        bool presionado = accion.getTipo() == CE::Botones::TipoAccion::OnPress;
        auto control = player->getComponente<CE::IControl>();
        const auto& n = accion.getNombre();
        if(n == "arriba")    control->arr = presionado;
        if(n == "abajo")     control->abj = presionado;
        if(n == "izquierda") control->izq = presionado;
        if(n == "derecha")   control->der = presionado;
    }

    void Escena_Lab9::onRender()
    {
        for(auto& al: tiles_layers)
            CE::Render::Get().AddToDraw(al);

        for(auto& obj: objetos.getPool())
        {
            //Lab 8 animaciones: circulo del rango
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
#endif
    }
}
