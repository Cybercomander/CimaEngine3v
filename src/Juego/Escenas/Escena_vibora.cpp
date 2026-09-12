#include "Escena_vibora.hpp"
#include <Juego/Componentes/CPartesCuerpo.hpp>
#include <Motor/Primitivos/GestorEscenas.hpp>
#include <Motor/Render/Render.hpp>

// Espacio de nombres del juego (IVJ), separado del código del motor (CE)
namespace IVJ
{
Escena_vibora::Escena_vibora() : CE::Escena{}
{
}

// La escena queda lista con:
//  - flechas y WASD como direcciones, Escape para volver al menú
//  - la cabeza en el centro del área de juego avanzando hacia la derecha
//  - un componente ICPartesCuerpo con 3 partes iniciales
void Escena_vibora::onInit()
{
    if (!inicializar)
        return;

    registrarBotones(sf::Keyboard::Scancode::W, "arriba");
    registrarBotones(sf::Keyboard::Scancode::Up, "arriba");
    registrarBotones(sf::Keyboard::Scancode::S, "abajo");
    registrarBotones(sf::Keyboard::Scancode::Down, "abajo");
    registrarBotones(sf::Keyboard::Scancode::A, "izquierda");
    registrarBotones(sf::Keyboard::Scancode::Left, "izquierda");
    registrarBotones(sf::Keyboard::Scancode::D, "derecha");
    registrarBotones(sf::Keyboard::Scancode::Right, "derecha");
    registrarBotones(sf::Keyboard::Scancode::Escape, "salir");

    // La víbora nace en el centro de la vista de la cámara (coordenadas del mundo)
    auto centro = CE::Render::Get().GetTextura().getView().getCenter();
    float cx = centro.x;
    float cy = centro.y;

    // Cabeza circular de radio 16; las partes del cuerpo son cuadros de 26x26
    vibora = std::make_shared<EnteVibora>(16.f, sf::Color{240, 200, 40}, sf::Color::Black);
    vibora->getNombre()->nombre = "Cabeza Vibora";
    vibora->setPosicion(cx, cy);
    vibora->getTransformada()->velocidad = CE::Vector2D{VELOCIDAD_CABEZA, 0.f};

    // El cuerpo es un componente de la cabeza: nace con 3 partes sobre ella
    vibora->addComponente(std::make_shared<ICPartesCuerpo>(26, 26, 3, CE::Vector2D{cx, cy}));

    objetos.agregarPool(vibora);
    inicializar = false;
}

void Escena_vibora::onFinal()
{
}

void Escena_vibora::onUpdate(float dt)
{
    // El área de juego es lo que muestra la cámara, no el tamaño en píxeles de la textura:
    // el motor aplica a la textura una sf::View de 1080x720 unidades del mundo y la estira
    // al tamaño del viewport, así que los bordes visibles son los de esa vista
    const auto &vista = CE::Render::Get().GetTextura().getView();
    sf::Vector2f tam = vista.getSize();
    vibora->setArea(sf::FloatRect{vista.getCenter() - tam / 2.f, tam});

    // Crecimiento: cada SEGUNDOS_NUEVA_PARTE segundos se agrega una parte al final del cuerpo.
    // Se resta el intervalo en lugar de reiniciar a cero para no perder la fracción sobrante
    tiempo_nueva_parte += dt;
    if (tiempo_nueva_parte >= SEGUNDOS_NUEVA_PARTE)
    {
        tiempo_nueva_parte -= SEGUNDOS_NUEVA_PARTE;
        if (auto cuerpo = vibora->getComponente<ICPartesCuerpo>())
            cuerpo->agregarParte(vibora->getTransformada()->posicion);
    }

    // La cabeza se mueve y acomoda su cuerpo dentro de su propio onUpdate
    for (auto &obj : objetos.getPool())
        obj->onUpdate(dt);
}

// Cada dirección fija la velocidad de la cabeza sobre un solo eje.
// Un giro de 180° se ignora: la cabeza chocaría de inmediato con su propio cuerpo
void Escena_vibora::onInputs(const CE::Botones &accion)
{
    if (accion.getTipo() != CE::Botones::TipoAccion::OnPress)
        return;

    const auto &nombre = accion.getNombre();
    if (nombre == "salir")
    {
        CE::GestorEscenas::Get().cambiarEscena("Menu");
        return;
    }

    auto &vel = vibora->getTransformada()->velocidad;
    CE::Vector2D nueva = vel;
    if (nombre == "arriba")
        nueva = CE::Vector2D{0.f, -VELOCIDAD_CABEZA};
    else if (nombre == "abajo")
        nueva = CE::Vector2D{0.f, VELOCIDAD_CABEZA};
    else if (nombre == "izquierda")
        nueva = CE::Vector2D{-VELOCIDAD_CABEZA, 0.f};
    else if (nombre == "derecha")
        nueva = CE::Vector2D{VELOCIDAD_CABEZA, 0.f};

    bool es_reversa = (nueva.x == -vel.x && nueva.x != 0.f) || (nueva.y == -vel.y && nueva.y != 0.f);
    if (!es_reversa)
        vel = nueva;
}

// Encola cada objeto del pool en la cola de dibujado del render
void Escena_vibora::onRender()
{
    for (auto &obj : objetos.getPool())
        CE::Render::Get().AddToDraw(*obj);
}
} // namespace IVJ
