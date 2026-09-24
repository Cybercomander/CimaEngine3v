#include "Escena_Sim.hpp"
#include <Juego/Componentes/IJComponentes.hpp>
#include <Juego/Figuras/Figuras.hpp>
#include <Juego/Sistemas/Sistemas.hpp>
#include <Juego/objetos/Texto.hpp>
#include <Motor/GUI/GLogger.hpp>
#include <Motor/Primitivos/GestorAssets.hpp>
#include <Motor/Primitivos/GestorEscenas.hpp>
#include <Motor/Render/Render.hpp>
#include <algorithm>
#include <memory>
#include <random>
#include <string>
namespace IVJ
{
// ITimer recibe el tiempo máximo en frames: 60 frames = 1 seg, 60*30 = 30 seg por generación.
// ITexto no tiene constructor por defecto (sf::Text ocupa un font), por eso va en la lista de inicialización
Escena_Sim::Escena_Sim()
    : CE::Escena{}, timer_generacion{60 * 30},
    mensaje{CE::GestorAssets::Get().getFont("default_font"), "Presiona Enter para continuar"}
{
    punto_inferior.x = 50.f;
    punto_inferior.y = 50.f;
    punto_superior.x = 1000.f;
    punto_superior.y = 650.f;
    entes_iniciales = 100;
    comida_iniciales = 150;
    generacion = 0;
    generacion_termino = false;

    float ancho = punto_superior.x - punto_inferior.x;
    float largo = punto_superior.y - punto_inferior.y;

    mundo = std::make_shared<Rectangulo>(ancho, largo, sf::Color::White, sf::Color::Black);

    float mx = punto_inferior.x + ancho / 2.f;
    float my = punto_inferior.y + largo / 2.f;

    mundo->setPosicion(mx, my);

    mensaje.m_texto.setPosition({mx, my});
    mensaje.m_texto.setFillColor(sf::Color::Yellow);
}

void Escena_Sim::agregarComidaYMundo()
{
    std::random_device rd;
    std::mt19937 rand(rd());
    float x;
    float y;
    // comida
    // deben estar dentro del mundo
    for (int i = 0; i < comida_iniciales; i++)
    {
        x = std::uniform_real_distribution(100.f + punto_inferior.x, punto_superior.x - 100.f)(rand);
        y = std::uniform_real_distribution(100.f + punto_inferior.y, punto_superior.y - 100.f)(rand);
        auto comida = std::make_shared<Circulo>(5.f, sf::Color::Green, sf::Color::Green);
        // agregar si tiene dueño
        comida->addComponente(std::make_shared<ITieneDueño>());
        comida->setPosicion(x, y);
        objetos.agregarPoolEnfrete(comida);
    }
    // el mundo y la comida tienen hp = 0, así que borrarPool los elimina al terminar
    // cada generación; por eso se vuelven a agregar al frente en cada una
    objetos.agregarPoolEnfrete(mundo);
}

void Escena_Sim::onInit()
{
    // La guía agrega la comida antes de esta guarda y llama onInit() al presionar Enter.
    // Pero el combo "Escenas" de la GUI también llama onInit() al regresar a esta escena,
    // lo que duplicaba la comida y el mundo; por eso la comida vive en agregarComidaYMundo()
    if (!inicializar)
        return;

    agregarComidaYMundo();

    registrarBotones(sf::Keyboard::Scancode::Enter, "Ok");

    std::random_device rd;
    std::mt19937 rand(rd());
    float x;
    float y;
    float angulo;
    // entes iniciales
    // donde respawn la idea es sobre la linea del mundo
    for (int i = 0; i < entes_iniciales; i++)
    {
        const float num = std::uniform_real_distribution(0.0f, 1.0f)(rand);
        if (num <= 0.5) // abajo
        {
            y = punto_superior.y;
            angulo = 0.f;
        }
        else // arriba
        {
            y = punto_inferior.y;
            angulo = 180.f;
        }
        x = std::uniform_real_distribution(punto_inferior.x, punto_superior.x)(rand);

        auto ente = std::make_shared<Pentagono>(15.f, sf::Color::Red, sf::Color::Black);
        // stats: hp_max = energia máxima, hp = energia actual, agi = alcance
        auto &stats = ente->getStats();
        auto &vel = ente->getTransformada()->velocidad;
        vel.x = std::uniform_real_distribution(1.f, 10.f)(rand);
        vel.y = std::uniform_real_distribution(1.f, 10.f)(rand);
        // cada ente tiene una energia máxima distinta entre 80 y 200, e inicia lleno
        stats->hp_max = std::uniform_int_distribution(80, 200)(rand);
        stats->hp = stats->hp_max;
        stats->agi = std::uniform_int_distribution(10, 255)(rand);
        ente->setPosicion(x, y);
        // usamos el getter de la figura para rotarla
        ente->getShape().setRotation(sf::degrees(angulo));
        // guardar el ángulo para la reproducción
        ente->getTransformada()->angulo = angulo;

        // agregar estadointerno inicial
        ente->addComponente(std::make_shared<IEstadoInterno>(IEstadoInterno::Estados::BUSCAR))
            // agregar inventario
            .addComponente(std::make_shared<IInventarioComida>())
            // agregar target nulo
            .addComponente(std::make_shared<ITargetComida>())
            // agregar la posicion inicial
            .addComponente(std::make_shared<IPosicionInicial>(x, y))
            // agregar score
            .addComponente(std::make_shared<IScore>());

        objetos.agregarPool(ente);
    }

    inicializar = false;
}

void Escena_Sim::onFinal()
{
}

void Escena_Sim::onUpdate(float dt)
{
    if (!generacion_termino)
    {
        // Se itera por índice y copiando el shared_ptr: SistemaReproducirEnte hace push_back
        // al pool, y con el range-for de la guía, si ese push_back realoja el vector,
        // los iteradores quedan inválidos (comportamiento indefinido)
        for (std::size_t i = 0; i < objetos.getPool().size(); i++)
        {
            auto obj = objetos[i];
            obj->onUpdate(dt);
            SistemaBuscarComida(*obj, objetos.getPool());
            // si no vio comida en su rango (agi), camina hacia enfrente
            SistemaCaminarEnfrente(*obj, dt, punto_inferior, punto_superior);
            SistemaMoveraComidaoCasa(*obj, dt);
            SistemaConsumirComida(*obj);
            SistemaReproducirEnte(*obj, objetos);
        }

        timer_generacion.frame_actual++;
        if (timer_generacion.frame_actual >= timer_generacion.frame_maximo)
        {
            generacion_termino = true;
            timer_generacion.frame_actual = 0;
        }
    }

#if DEBUG
    // el GLogger solo vacía su cola en Debug; en Release estos logs se acumularían cada frame
    CE::GLogger::Get().agregarLog("Generacion: " + std::to_string(generacion + 1), CE::GLogger::Niveles::LOG_SEVERO);
    CE::GLogger::Get().agregarLog(std::to_string(timer_generacion.frame_actual) + "/" +
                                    std::to_string(timer_generacion.frame_maximo),
                                CE::GLogger::Niveles::LOG_SEVERO);
    CE::GLogger::Get().agregarLog("Objetos en pool: " + std::to_string(objetos.getPool().size()),
                                CE::GLogger::Niveles::LOG_SEVERO);
    CE::GLogger::Get().agregarLog("Comida por generacion: " + std::to_string(comida_iniciales),
                                CE::GLogger::Niveles::LOG_SEVERO);
#endif
}

void Escena_Sim::onInputs(const CE::Botones &accion)
{
    switch (accion.getTipo())
    {
    case CE::Botones::TipoAccion::OnPress: {

        break;
    }
    case CE::Botones::TipoAccion::OnRelease: {

        if (accion.getNombre() == "Ok" && generacion_termino == true)
        {
            generacion_termino = false;
            // borrar para la siguiente generación y resetear
            for (auto &obj : objetos.getPool())
            {
                if (obj->tieneComponente<IEstadoInterno>() && obj->tieneComponente<ITargetComida>() &&
                    obj->tieneComponente<IInventarioComida>() && obj->tieneComponente<IScore>())
                {
                    // resetear
                    obj->getComponente<IEstadoInterno>()->setEstadoInterno(IEstadoInterno::Estados::BUSCAR);
                    obj->getComponente<ITargetComida>()->quitarTarget();
                    obj->getComponente<IInventarioComida>()->sacarComida();
                    auto pos_init = obj->getComponente<IPosicionInicial>()->pos_init;
                    obj->setPosicion(pos_init.x, pos_init.y);
                    // marcar muerto la siguiente generación
                    if (obj->getComponente<IScore>()->score == 0)
                    {
                        obj->getStats()->hp = 0;
                    }
                    obj->getComponente<IScore>()->score = 0;
                }
            }
            // elimina entes sin comida, la comida y el mundo (todos con hp <= 0)
            objetos.borrarPool();
            // cada generación nueva tiene 10 comidas menos (150, 140, 130, ...), sin bajar de 0
            comida_iniciales = std::max(0, comida_iniciales - 10);
            agregarComidaYMundo();
            generacion++;
        }
        break;
    }
    case CE::Botones::TipoAccion::None: {
        break;
    }
    }
}

void Escena_Sim::onRender()
{
    if (generacion_termino)
    {
        // render texto
        CE::Render::Get().AddToDraw(mensaje.m_texto);
    }
    else
    {
        for (auto &obj : objetos.getPool())
            CE::Render::Get().AddToDraw(*obj);

        // DEBUG: linea del ente a su comida objetivo
        for (auto &obj : objetos.getPool())
        {
            // solo ente tiene ITarget, comida ni mundo lo tienen
            if (!obj->tieneComponente<ITargetComida>())
                continue;
            auto target = obj->getComponente<ITargetComida>()->getTargetComida().lock();
            // si aún no tiene target, no pintamos la linea
            if (!target)
                continue;
            auto po = obj->getTransformada()->posicion;
            auto pt = target->getTransformada()->posicion;

            sf::VertexArray linea(sf::PrimitiveType::Lines, 2);
            linea[0].position = sf::Vector2f({po.x, po.y});
            linea[0].color = sf::Color::Black;
            linea[1].position = sf::Vector2f({pt.x, pt.y});
            linea[1].color = sf::Color::Black;
            CE::Render::Get().AddToDraw(linea);
        }
    }
}
} // namespace IVJ
