#include "Escena_Round12.hpp"
#include <Motor/Inputs/Botones.hpp>
#include <Motor/Render/Render.hpp>
#include <Motor/Camaras/CamarasGestor.hpp>
#include <Motor/Primitivos/GestorAssets.hpp>
#include <Motor/Componentes/IComponentes.hpp>
#include <Juego/objetos/Entidad.hpp>
#include <Juego/Componentes/IJComponentes.hpp>
#include <Juego/Sistemas/Sistemas.hpp>
#include <Juego/Maquinas/Boxeadores/NodosJugador.hpp>
#include <Juego/Maquinas/Boxeadores/NodosRival.hpp>
#include <algorithm>
#include <memory>
#include <string>

//los assets de ROUND12 viven en la raíz del repo, junto a assets/
#define R12 ASSETS "/../ROUND12_Assets"

namespace IVJ
{
    namespace
    {
        constexpr float ESCALA = 2.f;          //pixel art 44px -> 88px en pantalla
        constexpr float VEL_CAMINAR = 180.f;
        constexpr float VEL_CORRER = 380.f;
        constexpr float RANGO_RIVAL = 120.f;   //distancia a la que el rival lanza jabs

        // Fábrica: arma cualquier boxeador (jugador o rival) con su hoja de 4 poses
        // y su máquina de estados. Solo cambian la textura y el nodo inicial.
        std::shared_ptr<Entidad> crearBoxeador(const std::string& key, int w, int h,
                float x, float y, const std::shared_ptr<FSM>& inicial)
        {
            auto ente = std::make_shared<Entidad>();
            //este constructor referencia la textura del gestor (sin suavizado),
            //el otro la copia con setSmooth(true) y difumina el pixel art
            auto sprite = std::make_shared<CE::ISprite>(
                    CE::GestorAssets::Get().getTextura(key), ESCALA);
            sprite->width = w;
            sprite->height = h;
            sprite->m_sprite.setTextureRect(sf::IntRect{{0,0},{w,h}});
            sprite->m_sprite.setOrigin({w/2.f,h/2.f});

            auto me = std::make_shared<IMaquinaEstado>();
            me->fsm = inicial;
            ente->addComponente(sprite).addComponente(me);
            ente->setPosicion(x,y);
            ente->setFSM(me->fsm); //onEntrar del primer nodo
            return ente;
        }
    }

    Escena_Round12::Escena_Round12()
        :CE::Escena{}
    {
    }

    void Escena_Round12::onInit()
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

            auto& assets = CE::GestorAssets::Get();
            assets.agregarTextura("r12_gym",
                    R12 "/escenarios_1080x720/01_sanchez_boxing_gym_1080x720.png",
                    CE::Vector2D{0.f,0.f}, CE::Vector2D{1080.f,720.f});
            //todas las hojas: 1 fila de 4 poses de 44px de ancho
            assets.agregarTextura("r12_diego",
                    R12 "/sprites/personajes/diego_ureta/spritesheet_4poses.png",
                    CE::Vector2D{0.f,0.f}, CE::Vector2D{176.f,52.f});
            const std::string rivales[] = {
                "brandon_salazar","javier_castaneda","kevin_ramirez","ricardo_oropeza"};
            for(const auto& r : rivales)
                assets.agregarTextura("r12_"+r,
                        R12 "/sprites/personajes/"+r+"/spritesheet_4poses.png",
                        CE::Vector2D{0.f,0.f}, CE::Vector2D{176.f,56.f});

            fondo = std::make_shared<Entidad>();
            fondo->addComponente(std::make_shared<CE::ISprite>(assets.getTextura("r12_gym"),1.f));
            fondo->setPosicion(540.f,360.f);
            fondo->onUpdate(0.f); //coloca el sprite una vez, el fondo no se mueve

            //jugador: familia de nodos controlados por IControl
            diego = crearBoxeador("r12_diego",44,52,150.f,430.f,
                    std::make_shared<IdleBoxeador>());
            diego->addComponente(std::make_shared<CE::IControl>());

            //enemigos: los 4 reutilizan IdleRival/JabRival
            float x = 380.f;
            for(const auto& r : rivales)
            {
                auto rival = crearBoxeador("r12_"+r,44,56,x,430.f,
                        std::make_shared<IdleRival>());
                rival->addComponente(std::make_shared<ITarget>(diego.get()))
                    .addComponente(std::make_shared<IRangoAggro>(RANGO_RIVAL));
                objetos.agregarPool(rival);
                x += 180.f;
            }
            inicializar = false;
        }
        CE::GestorCamaras::Get().setCamaraActiva(2);
        CE::GestorCamaras::Get().getCamaraActiva().lockEnObjeto(diego);
    }

    void Escena_Round12::onFinal()
    {
        CE::GestorCamaras::Get().setCamaraActiva(0);
    }

    void Escena_Round12::onUpdate(float dt)
    {
        diego->inputFSM();
        //la velocidad depende del estado: idle y jab no se desplazan
        auto estado = diego->getComponente<IMaquinaEstado>()->fsm->getNombre();
        float vel = estado == "CorrerBoxeador" ? VEL_CORRER
                  : estado == "CaminarBoxeador" ? VEL_CAMINAR : 0.f;
        diego->getTransformada()->velocidad = CE::Vector2D{vel,vel*0.5f};
        SistemaMoverPlano(diego,dt);
        //mantenerlo sobre el piso del gimnasio
        auto& pos = diego->getTransformada()->posicion;
        diego->setPosicion(std::clamp(pos.x,60.f,1020.f),std::clamp(pos.y,400.f,460.f));
        diego->onUpdate(dt);

        for(auto& obj: objetos.getPool())
        {
            obj->inputFSM();
            obj->onUpdate(dt);
        }
    }

    void Escena_Round12::onInputs(const CE::Botones& accion)
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
    }

    void Escena_Round12::onRender()
    {
        CE::Render::Get().AddToDraw(*fondo);
        for(auto& obj: objetos.getPool())
        {
#if DEBUG
            auto rango = obj->getComponente<IRangoAggro>();
            auto p = obj->getTransformada()->posicion;
            rango->marcador.setPosition({p.x,p.y});
            CE::Render::Get().AddToDraw(rango->marcador);
#endif
            CE::Render::Get().AddToDraw(*obj);
        }
        CE::Render::Get().AddToDraw(*diego);
    }
}
