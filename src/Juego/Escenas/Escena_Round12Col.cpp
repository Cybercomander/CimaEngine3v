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
#include <cstdint>
#include <map>
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
        constexpr float RANGO_RIVAL = 28.f; //centro a centro: medio cuerpo + brazo extendido
        //lona del ring: el marco de colisión deja los pies entre y=148 y y=160
        constexpr float PIES_RING = 154.f;
        constexpr float PIES_PISO = 184.f;  //piso del gimnasio, frente al ring

        //caja del cuerpo sin brazos, medida en las hojas 80x80: x 33-47, y 18-70
        //(cabeza a pies); los guantes quedan fuera para usarlos después como golpe
        const CE::Vector2D CAJA_CUERPO{14.f,52.f};
        //el pivote va al centro del cuerpo para que la caja quede centrada en él
        constexpr float ORIGEN_X = 40.f, ORIGEN_Y = 44.f;
        constexpr float PIES_A_CENTRO = 70.f-ORIGEN_Y; //los pies están en y=70 del cuadro

        // Fábrica: arma cualquier boxeador con su hoja de animaciones 80x80,
        // dirección, máquina de estados y caja de colisión del cuerpo.
        // pies_y: altura de los pies en el mundo
        std::shared_ptr<Entidad> crearBoxeador(const std::string& key, float x, float pies_y,
                int dir, const std::shared_ptr<FSM>& inicial)
        {
            auto ente = std::make_shared<Entidad>();
            //referencia la textura del gestor (sin suavizado, pixel art)
            auto sprite = std::make_shared<CE::ISprite>(CE::GestorAssets::Get().getTextura(key),1.f);
            sprite->width = 80;
            sprite->height = 80;
            sprite->m_sprite.setTextureRect(sf::IntRect{{0,0},{80,80}});
            sprite->m_sprite.setOrigin({ORIGEN_X,ORIGEN_Y});
            auto direccion = std::make_shared<IDireccion>();
            direccion->dir = dir;
            auto me = std::make_shared<IMaquinaEstado>();
            me->fsm = inicial;
            //IDireccion antes de setFSM: onEntrar la lee para elegir la fila
            ente->addComponente(sprite)
                .addComponente(direccion)
                .addComponente(std::make_shared<CE::IBoundingBox>(CAJA_CUERPO))
                .addComponente(me);
            ente->setPosicion(x,pies_y-PIES_A_CENTRO);
            ente->setFSM(me->fsm);
            return ente;
        }

        // Comando: cada botón registrado activa un bit de IControl::botones (GDD Avance 4)
        const std::map<std::string,std::uint32_t> BOTONES_COMBATE = {
            {"jab",BotonBox::JAB},{"recto",BotonBox::RECTO},
            {"gancho",BotonBox::GANCHO},{"uppercut",BotonBox::UPPERCUT},
            {"guardia",BotonBox::GUARDIA},
            {"esquiva_izq",BotonBox::ESQ_IZQ},{"esquiva_der",BotonBox::ESQ_DER}};
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
            //controles del GDD (Avance 4)
            registrarBotones(sf::Keyboard::Scancode::J,"jab");
            registrarBotones(sf::Keyboard::Scancode::K,"recto");
            registrarBotones(sf::Keyboard::Scancode::L,"gancho");
            registrarBotones(sf::Keyboard::Scancode::I,"uppercut");
            registrarBotones(sf::Keyboard::Scancode::LShift,"guardia");
            registrarBotones(sf::Keyboard::Scancode::RShift,"guardia");
            registrarBotones(sf::Keyboard::Scancode::Q,"esquiva_izq");
            registrarBotones(sf::Keyboard::Scancode::E,"esquiva_der");

            //segunda forma: el mapa marca con 'c' el marco del ring (cuerdas y límites
            //de carril) y el TileMap agrega esos tiles al pool con su IBoundingBox
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

            //Diego y Brandon (sparring del GDD) en el ring
            diego = crearBoxeador("r12a_diego_ureta",120.f,PIES_RING,
                    IDireccion::DERECHA,std::make_shared<IdleJugador>());
            diego->addComponente(std::make_shared<CE::IControl>());

            //los 4 rivales reutilizan GuardiaRival/GolpeRival; los que esperan turno
            //están en el piso del gimnasio, frente al ring
            struct Lugar { float x, pies; };
            const Lugar lugares[] = {{330.f,PIES_RING},{20.f,PIES_PISO},{604.f,PIES_PISO},{626.f,PIES_PISO}};
            for(int i = 1; i < 5; ++i)
            {
                auto rival = crearBoxeador("r12a_"+boxeadores[i],lugares[i-1].x,lugares[i-1].pies,
                        IDireccion::IZQUIERDA,std::make_shared<GuardiaRival>());
                rival->addComponente(std::make_shared<ITarget>(diego.get()))
                    .addComponente(std::make_shared<IRangoAggro>(RANGO_RIVAL));
                objetos.agregarPool(rival);
            }

            //primera forma + objeto individual de una hoja: Carlos Sánchez con su
            //cronómetro (pose 4 de spritesheet_npc.png, 44x60) en la esquina de Diego
            assets.agregarTextura("r12_carlos_npc",
                    R12 "/sprites/personajes/carlos_sanchez/spritesheet_npc.png",
                    CE::Vector2D{0.f,0.f}, CE::Vector2D{220.f,60.f});
            auto carlos = std::make_shared<Entidad>();
            auto carlos_sprite = std::make_shared<CE::ISprite>(assets.getTextura("r12_carlos_npc"),1.f);
            carlos_sprite->m_sprite.setTextureRect(sf::IntRect{{44*4,0},{44,60}});
            //pivote al centro del cuerpo (pies en y=58 del cuadro) para centrar la caja
            constexpr float CARLOS_ALTO = 56.f;
            carlos_sprite->m_sprite.setOrigin({22.f,58.f-CARLOS_ALTO/2.f});
            carlos->addComponente(carlos_sprite)
                .addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{14.f,CARLOS_ALTO}));
            carlos->setPosicion(66.f,150.f-CARLOS_ALTO/2.f);
            carlos->onUpdate(0.f); //no se mueve: colocar el sprite una vez
            objetos.agregarPool(carlos);

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
        //solo se desplaza caminando: reposo, guardia, golpes y esquivas no
        bool camina = diego->getComponente<IMaquinaEstado>()->fsm->getNombre() == "CaminarJugador";
        float vel = camina ? VELOCIDAD : 0.f;
        diego->getTransformada()->velocidad = CE::Vector2D{vel,vel*0.6f};
        //orden del lab 9: onUpdate guarda pos_prev, luego mover y al final colisionar
        diego->onUpdate(dt);
        SistemaMoverPlano(diego,dt);
        for(auto& obj: objetos.getPool())
        {
            obj->inputFSM();
            obj->onUpdate(dt);
            //cuerdas, límites de carril, Brandon y Carlos regresan a Diego a su posición anterior
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
        auto boton = BOTONES_COMBATE.find(n);
        if(boton != BOTONES_COMBATE.end())
        {
            if(presionado) control->botones |= boton->second;
            else           control->botones &= ~boton->second;
        }
    }

    void Escena_Round12Col::onRender()
    {
        for(auto& al: tiles_layers)
            CE::Render::Get().AddToDraw(al);

        //profundidad: se dibuja primero lo que está más arriba en pantalla;
        //Diego va al final para quedar al frente cuando empata en altura
        std::vector<CE::Objeto*> orden;
        orden.reserve(objetos.getPool().size()+1);
        for(auto& obj: objetos.getPool())
            orden.push_back(obj.get());
        orden.push_back(diego.get());
        std::stable_sort(orden.begin(),orden.end(),[](CE::Objeto* a, CE::Objeto* b){
                return a->getTransformada()->posicion.y < b->getTransformada()->posicion.y;});

#if DEBUG
        //los rangos van abajo de todos los personajes para no taparlos
        for(auto* obj: orden)
            if(auto rango = obj->getComponente<IRangoAggro>())
            {
                auto p = obj->getTransformada()->posicion;
                rango->marcador.setPosition({p.x,p.y});
                CE::Render::Get().AddToDraw(rango->marcador);
            }
#endif
        for(auto* obj: orden)
            CE::Render::Get().AddToDraw(*obj);
    }
}
