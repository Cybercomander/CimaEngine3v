#include "Escena_Round12Col.hpp"
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
#include <Juego/Maquinas/Boxeadores/NodosBoxeo.hpp>
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

//los assets de ROUND12 viven en la raíz del repo, junto a assets/
#define R12 ASSETS "/../ROUND12_Assets"

namespace IVJ
{
    namespace
    {
        //mundo nativo del nivel: 80x24 tiles de 8px
        constexpr float MUNDO_W = 640.f;
        constexpr float MUNDO_H = 192.f;
        //vista 3:2 (igual que el viewport 1080x720) con la altura del mundo: zoom 3.75x
        constexpr float VISTA_W = 288.f;
        constexpr float VELOCIDAD = 50.f;
        constexpr float VEL_CORRER = 100.f;
        constexpr float RANGO_RIVAL = 30.f; //alcance del jab de un rival
        constexpr float PISO_Y = 172.f;     //el suelo transitable va de y=152 a y=192
        //caja de colisión en los pies: el ancla de las hojas es (40,70)
        const CE::Vector2D CAJA_PIES{16.f,6.f};

        // Fábrica: arma cualquier boxeador con su hoja de animaciones 80x80,
        // dirección, máquina de estados y caja de colisión en los pies
        std::shared_ptr<Entidad> crearBoxeador(const std::string& key, float x, float y,
                int dir, const std::shared_ptr<FSM>& inicial)
        {
            auto ente = std::make_shared<Entidad>();
            //referencia la textura del gestor (sin suavizado, pixel art)
            auto sprite = std::make_shared<CE::ISprite>(CE::GestorAssets::Get().getTextura(key),1.f);
            sprite->width = 80;
            sprite->height = 80;
            sprite->m_sprite.setTextureRect(sf::IntRect{{0,0},{80,80}});
            sprite->m_sprite.setOrigin({40.f,70.f}); //pies
            auto direccion = std::make_shared<IDireccion>();
            direccion->dir = dir;
            auto me = std::make_shared<IMaquinaEstado>();
            me->fsm = inicial;
            //IDireccion antes de setFSM: onEntrar la lee para elegir la fila
            ente->addComponente(sprite)
                .addComponente(direccion)
                .addComponente(std::make_shared<CE::IBoundingBox>(CAJA_PIES))
                .addComponente(me);
            ente->setPosicion(x,y);
            ente->setFSM(me->fsm);
            return ente;
        }
    }

    Escena_Round12Col::Escena_Round12Col()
        :CE::Escena{}
    {
    }

    void Escena_Round12Col::onInit()
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
            registrarBotones(sf::Keyboard::Scancode::LShift,"correr");
            registrarBotones(sf::Keyboard::Scancode::Space,"golpe");
            registrarBotones(sf::Keyboard::Scancode::J,"golpe");
            registrarBotones(sf::Keyboard::Scancode::K,"recto");

            //segunda forma: el mapa marca con 'c' los tiles de pared que tocan el suelo
            //y el TileMap los agrega al pool con su IBoundingBox
            tiles_layers.push_back(TileMap());
            if(!tiles_layers[0].loadTileMap(ASSETS "/mapas/r12_sanchez_gym_col.txt",objetos))
                exit(EXIT_FAILURE);

            //hojas de animación (tools/round12_assets.py): 6 frames x 32 filas de 80px
            auto& assets = CE::GestorAssets::Get();
            const std::string boxeadores[] = {"diego_ureta",
                "brandon_salazar","javier_castaneda","kevin_ramirez","ricardo_oropeza"};
            for(const auto& b : boxeadores)
                assets.agregarTextura("r12a_"+b,
                        R12 "/ROUND12_Sprites_Animaciones/hojas/"+b+".png",
                        CE::Vector2D{0.f,0.f}, CE::Vector2D{480.f,2560.f});

            diego = crearBoxeador("r12a_diego_ureta",60.f,PISO_Y,
                    IDireccion::DERECHA,std::make_shared<GuardiaJugador>());
            diego->addComponente(std::make_shared<CE::IControl>());

            //los 4 rivales reutilizan GuardiaRival/GolpeRival
            float x = 200.f;
            for(int i = 1; i < 5; ++i)
            {
                //alternar altura para que se pueda pasar entre ellos
                float y = PISO_Y + (i%2 ? -10.f : 10.f);
                auto rival = crearBoxeador("r12a_"+boxeadores[i],x,y,
                        IDireccion::IZQUIERDA,std::make_shared<GuardiaRival>());
                rival->addComponente(std::make_shared<ITarget>(diego.get()))
                    .addComponente(std::make_shared<IRangoAggro>(RANGO_RIVAL));
                objetos.agregarPool(rival);
                x += 110.f;
            }

            //primera forma + objeto individual de una hoja: Carlos Sánchez con su
            //cronómetro (pose 4 de spritesheet_npc.png, cada pose de 44x60)
            assets.agregarTextura("r12_carlos_npc",
                    R12 "/sprites/personajes/carlos_sanchez/spritesheet_npc.png",
                    CE::Vector2D{0.f,0.f}, CE::Vector2D{220.f,60.f});
            auto carlos = std::make_shared<Entidad>();
            auto carlos_sprite = std::make_shared<CE::ISprite>(assets.getTextura("r12_carlos_npc"),1.f);
            carlos_sprite->m_sprite.setTextureRect(sf::IntRect{{44*4,0},{44,60}});
            //pivote en los pies para que la caja quede en el piso
            carlos_sprite->m_sprite.setOrigin({22.f,58.f});
            carlos->addComponente(carlos_sprite)
                .addComponente(std::make_shared<CE::IBoundingBox>(CAJA_PIES));
            carlos->setPosicion(MUNDO_W-50.f,PISO_Y);
            carlos->onUpdate(0.f); //no se mueve: colocar el sprite una vez
            objetos.agregarPool(carlos);

            //primera forma: pared invisible bajo el ring (la capa de colisión no la trae)
            auto limite = std::make_shared<Entidad>();
            limite->addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{MUNDO_W,8.f}));
            limite->setPosicion(MUNDO_W/2.f,MUNDO_H+4.f);
            objetos.agregarPool(limite);

            //cámara propia con zoom al mundo nativo; sigue al foco (ventana de ancho 0)
            foco = std::make_shared<Entidad>();
            //la cámara copia la posición del foco en su primer frame: debe nacer ya en su lugar
            foco->setPosicion(VISTA_W/2.f,MUNDO_H/2.f);
            auto& camaras = CE::GestorCamaras::Get();
            camaras.agregarCamara(std::make_shared<CE::CamaraSnapVentana>(
                    CE::Vector2D{VISTA_W/2.f,MUNDO_H/2.f}, //posición
                    CE::Vector2D{VISTA_W,MUNDO_H},         //dimensiones de la vista
                    CE::Vector2D{0.f,MUNDO_H}));           //ventana
            id_camara = (int)camaras.getListaCamaras().size()-1;
            inicializar = false;
        }
        CE::GestorCamaras::Get().setCamaraActiva(id_camara);
        CE::GestorCamaras::Get().getCamaraActiva().lockEnObjeto(foco);
    }

    void Escena_Round12Col::onFinal()
    {
        CE::GestorCamaras::Get().setCamaraActiva(0);
    }

    void Escena_Round12Col::onUpdate(float dt)
    {
        diego->inputFSM();
        //la velocidad depende del estado: guardia y golpes no se desplazan
        auto estado = diego->getComponente<IMaquinaEstado>()->fsm->getNombre();
        float vel = estado == "CorrerJugador" ? VEL_CORRER
                  : estado == "CaminarJugador" ? VELOCIDAD : 0.f;
        diego->getTransformada()->velocidad = CE::Vector2D{vel,vel*0.6f};
        //orden del lab 9: onUpdate guarda pos_prev, luego mover y al final colisionar
        diego->onUpdate(dt);
        SistemaMoverPlano(diego,dt);
        for(auto& obj: objetos.getPool())
        {
            obj->inputFSM();
            obj->onUpdate(dt);
            //paredes, rivales, Carlos y el límite regresan a Diego a su posición anterior
            SistemaColAABBMid(*diego,*obj,true);
        }
        float x = std::clamp(diego->getTransformada()->posicion.x,VISTA_W/2.f,MUNDO_W-VISTA_W/2.f);
        foco->setPosicion(x,MUNDO_H/2.f);
    }

    void Escena_Round12Col::onInputs(const CE::Botones& accion)
    {
        if(accion.getTipo() == CE::Botones::TipoAccion::None) return;
        bool presionado = accion.getTipo() == CE::Botones::TipoAccion::OnPress;
        auto control = diego->getComponente<CE::IControl>();
        const auto& n = accion.getNombre();
        if(n == "arriba")    control->arr = presionado;
        if(n == "abajo")     control->abj = presionado;
        if(n == "izquierda") control->izq = presionado;
        if(n == "derecha")   control->der = presionado;
        if(n == "correr")    control->run = presionado;
        if(n == "golpe")     control->acc = presionado;
        if(n == "recto")     control->sacc = presionado;
    }

    void Escena_Round12Col::onRender()
    {
        for(auto& al: tiles_layers)
            CE::Render::Get().AddToDraw(al);

        //profundidad: se dibuja primero lo que está más arriba en pantalla
        std::vector<CE::Objeto*> orden;
        orden.reserve(objetos.getPool().size()+1);
        for(auto& obj: objetos.getPool())
            orden.push_back(obj.get());
        orden.push_back(diego.get());
        std::sort(orden.begin(),orden.end(),[](CE::Objeto* a, CE::Objeto* b){
                return a->getTransformada()->posicion.y < b->getTransformada()->posicion.y;});

        for(auto* obj: orden)
        {
#if DEBUG
            if(auto rango = obj->getComponente<IRangoAggro>())
            {
                auto p = obj->getTransformada()->posicion;
                rango->marcador.setPosition({p.x,p.y});
                CE::Render::Get().AddToDraw(rango->marcador);
            }
#endif
            CE::Render::Get().AddToDraw(*obj);
        }
    }
}
