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

        // Cada escena registra su propio esquema: registrarBotones mapea un scancode
        // físico a un nombre de acción, así una tecla cambia de significado por contexto
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
        // Se le pone un nombre descriptivo para identificarlo en la lista de objetos
        // del editor, en lugar del "Objeto N" que le asigna el constructor de CE::Objeto
        marcador->getNombre()->nombre = "Ruedita de seleccion";

        // Texto de la primera opción. La fuente se pide al gestor de assets,
        // que la cargó al inicializar el motor con la clave "default_font"
        auto strIniciar = std::make_shared<Texto>(
                CE::GestorAssets::Get().getFont("default_font"),
                "Iniciar"
                );
        strIniciar->setPosicion(510.f,250.f);
        strIniciar->setColor(sf::Color::Green);
        strIniciar->getNombre()->nombre = "txt Iniciar";

        // Texto de la segunda opción
        auto strSalir = std::make_shared<Texto>(
                CE::GestorAssets::Get().getFont("default_font"),
                "Salir"
                );
        strSalir->setPosicion(510.f,300.f);
        strSalir->getNombre()->nombre = "txt Salir";

        // Los tres cuadros blancos son el mismo tipo de objeto y se ven igual: solo los
        // distingue el componente que llevan, que su sistema ejecuta en onUpdate

        // Cuadro con vaivén vertical: sube y baja 80 píxeles desde donde nace
        auto cuadro_vertical = std::make_shared<Rectangulo>(
                80.f,80.f,
                sf::Color::White,
                sf::Color::Black
                );
        cuadro_vertical->setPosicion(150.f,360.f);
        cuadro_vertical->addComponente(std::make_shared<IMoverVertical>(80.f,2.f));
        // El nombre dice qué sistema lo mueve, para reconocerlo en la lista de objetos
        cuadro_vertical->getNombre()->nombre = "Sistem Vertical";

        // Cuadro con movimiento ondulatorio: avanza hacia la derecha subiendo y bajando,
        // de modo que su trayectoria dibuja una onda (~~~~) a lo largo de 700 píxeles
        auto cuadro_onda = std::make_shared<Rectangulo>(
                80.f,80.f,
                sf::Color::White,
                sf::Color::Black
                );
        cuadro_onda->setPosicion(120.f,620.f);
        cuadro_onda->addComponente(std::make_shared<IMoverOnda>(50.f,2.f,150.f,700.f));
        cuadro_onda->getNombre()->nombre = "S istem Onda";

        // Cuadro con movimiento circular: gira alrededor del punto donde fue colocado,
        // a 90 píxeles de distancia de ese centro
        auto cuadro_circular = std::make_shared<Rectangulo>(
                80.f,80.f,
                sf::Color::White,
                sf::Color::Black
                );
        cuadro_circular->setPosicion(880.f,400.f);
        cuadro_circular->addComponente(std::make_shared<IMoverCircular>(90.f,1.5f));
        cuadro_circular->getNombre()->nombre = "Sistem Circular";

        // El pool toma posesión de los objetos y el motor ya los actualiza y dibuja.
        // El orden importa: el marcador queda en el índice 0 que consulta onInputs
        objetos.agregarPool(marcador);
        objetos.agregarPool(strIniciar);
        objetos.agregarPool(strSalir);
        // Los cuadros de demostración se agregan al final, después del marcador,
        // para que este siga siendo el objeto en el índice 0 que consulta onInputs
        objetos.agregarPool(cuadro_vertical);
        objetos.agregarPool(cuadro_onda);
        objetos.agregarPool(cuadro_circular);

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
            // Cada sistema revisa primero si el objeto trae su componente y, si no, se
            // retira: por eso los textos y el marcador no se mueven y cada cuadro sí
            SistemaMoverVertical(*obj,dt);
            SistemaMoverOnda(*obj,dt);
            SistemaMoverCircular(*obj,dt);
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
