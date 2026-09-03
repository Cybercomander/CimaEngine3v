#include "Escena_Menu.hpp"
#include <Motor/Primitivos/GestorAssets.hpp>
#include <Motor/Primitivos/GestorEscenas.hpp>
#include <Motor/Render/Render.hpp>
#include <Juego/Figuras/Figuras.hpp>
#include <Juego/objetos/Texto.hpp>
#include <Juego/Componentes/IJComponentes.hpp>
#include <Juego/Sistemas/Sistemas.hpp>
#include <cstdlib>
#include <memory>

// Espacio de nombres del juego (IVJ), separado del código del motor (CE)
namespace IVJ
{
    // Constructor: delega en el constructor por defecto de la clase base CE::Escena.
    // Los miembros propios ya tienen valor inicial en el .hpp o se asignan en onInit
    Escena_Menu::Escena_Menu()
        :CE::Escena{}
    {
    }

    // Inicialización de la escena. El motor la llama al cambiar a esta escena
    void Escena_Menu::onInit()
    {
        // Guarda de una sola ejecución: si ya se inicializó, no se vuelve a construir el menú
        if(!inicializar) return;

        // Cada escena registra su propio esquema de botones: así el mismo teclado
        // puede significar cosas distintas según el contexto (menú, combate, exploración).
        // registrarBotones mapea un scancode físico a un nombre de acción lógico
        registrarBotones(sf::Keyboard::Scancode::W,"arriba");
        registrarBotones(sf::Keyboard::Scancode::Up,"arriba");
        registrarBotones(sf::Keyboard::Scancode::S,"abajo");
        registrarBotones(sf::Keyboard::Scancode::Down,"abajo");
        registrarBotones(sf::Keyboard::Scancode::Enter,"aceptar");

        // Separación vertical entre las dos opciones del menú
        salto = 48.f;
        // Se arranca con la opción 1 (Iniciar) seleccionada
        opcion = 1u;

        // Marcador: círculo verde que señala la opción activa
        auto marcador  = std::make_shared<Circulo>(15.f, sf::Color::Green,sf::Color::Black);
        marcador->setPosicion(470.f,295.f);

        // Texto de la primera opción. La fuente se pide al gestor de assets,
        // que la cargó al inicializar el motor con la clave "default_font"
        auto strIniciar = std::make_shared<Texto>(
                CE::GestorAssets::Get().getFont("default_font"),
                "Iniciar"
                );
        strIniciar->setPosicion(510.f,250.f);
        strIniciar->setColor(sf::Color::Green);

        // Texto de la segunda opción
        auto strSalir = std::make_shared<Texto>(
                CE::GestorAssets::Get().getFont("default_font"),
                "Salir"
                );
        strSalir->setPosicion(510.f,300.f);

        // Rectángulo decorativo: por sí mismo no gira. Se le agrega el componente
        // IGirar, que solo guarda los datos (ángulo y radio); el movimiento lo produce
        // el SistemaGirar al recorrer el pool en onUpdate
        auto cuadro = std::make_shared<Rectangulo>(
                150,150,
                sf::Color::White,
                sf::Color::Black
                );
        cuadro->addComponente(std::make_shared<IGirar>(3.1416f,5.5f));
        cuadro->setPosicion(500,300);

        // El pool de la escena toma posesión de los objetos: a partir de aquí
        // el motor los actualiza y los dibuja. El orden importa: el marcador queda
        // en el índice 0 y por eso onInputs lo consulta con getPool()[0]
        objetos.agregarPool(marcador);
        objetos.agregarPool(strIniciar);
        objetos.agregarPool(strSalir);
        objetos.agregarPool(cuadro);

        // Se baja la bandera para que la construcción no se repita
        inicializar=false;
    }

    // Limpieza al descargar la escena; el pool libera solo sus objetos (shared_ptr)
    void Escena_Menu::onFinal()
    {
    }

    // Actualización por frame
    void Escena_Menu::onUpdate(float dt)
    {
        for(auto& obj: objetos.getPool())
        {
            // Lógica propia de cada objeto (polimorfismo: cada figura implementa la suya)
            obj->onUpdate(dt);
            // Sistema: se ejecuta sobre todos los objetos, pero solo actúa en los que
            // tengan el componente IGirar; los demás salen de inmediato
            SistemaGirar(*obj,dt);
        }
    }

    // Procesamiento de las acciones registradas
    void Escena_Menu::onInputs(const CE::Botones& accion)
    {
        switch(accion.getTipo())
        {
            // Acción mientras la tecla se mantiene presionada
            case CE::Botones::TipoAccion::OnPress:
            {
                // Con solo dos opciones, arriba y abajo hacen lo mismo: alternar
                if(accion.getNombre() == "arriba"
                || accion.getNombre() == "abajo")
                {
                    // El marcador es el primer objeto que se agregó al pool
                    auto select = objetos.getPool()[0];
                    auto pos = select->getTransformada()->posicion;
                    // Si la opción activa es 1 (arriba) el marcador baja, y viceversa
                    pos.y = (opcion)? pos.y+salto:pos.y-salto;
                    select->setPosicion(pos.x,pos.y);
                    // XOR con 1 conmuta entre 0 y 1 sin necesidad de condicionales
                    opcion ^=1;
                }

                if(accion.getNombre() == "aceptar")
                {
                    switch(opcion)
                    {
                        case 0:
                            // Opción "Salir": termina el proceso
                            exit(EXIT_SUCCESS);
                            break;
                        case 1:
                            // Opción "Iniciar": pide al gestor cambiar de escena
                            CE::GestorEscenas::Get().cambiarEscena("Init");
                            break;
                    }
                }
                break;
            }
            // Acción al soltar la tecla; sin uso en este menú
            case CE::Botones::TipoAccion::OnRelease:
            {
                break;
            }
            // Sin acción asociada
            case CE::Botones::TipoAccion::None:
            {
                break;
            }
        }
    }

    // Encola cada objeto del pool en la cola de dibujado del render
    void Escena_Menu::onRender()
    {
        for(auto& obj: objetos.getPool())
            CE::Render::Get().AddToDraw(*obj);
    }
}
