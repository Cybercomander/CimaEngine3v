#include "EnteVibora.hpp"
#include <Juego/Componentes/CPartesCuerpo.hpp>

namespace IVJ
{
// La cabeza es un círculo con el origen en su centro, así la posición del ITransform
// coincide con el centro del círculo
EnteVibora::EnteVibora(float radio, const sf::Color &relleno, const sf::Color &contorno)
    : CE::Objeto{}, cabeza{radio, 32}, radio{radio}, area{}
{
    cabeza.setFillColor(relleno);
    cabeza.setOutlineColor(contorno);
    cabeza.setOutlineThickness(2.f);
    cabeza.setOrigin({radio, radio});
}

void EnteVibora::onUpdate(float dt)
{
    // 1) Movimiento de la cabeza: la posición avanza con la velocidad escalada por dt
    CE::Vector2D anterior = transform->posicion;
    transform->posicion.x += transform->velocidad.x * dt;
    transform->posicion.y += transform->velocidad.y * dt;

    // Salida por los bordes: la cabeza se considera fuera cuando su centro rebasa el borde
    // por más de un radio (el círculo ya no se ve). Entonces reaparece un radio antes del
    // borde opuesto, también fuera de la vista, y entra deslizándose a la pantalla
    if (area.size.x > 0.f && area.size.y > 0.f)
    {
        auto &p = transform->posicion;
        float izq = area.position.x - radio;
        float der = area.position.x + area.size.x + radio;
        float arr = area.position.y - radio;
        float abj = area.position.y + area.size.y + radio;
        if (p.x < izq)
            p.x = der;
        else if (p.x > der)
            p.x = izq;
        if (p.y < arr)
            p.y = abj;
        else if (p.y > abj)
            p.y = arr;
    }
    transform->pos_prev = anterior;
    cabeza.setPosition({transform->posicion.x, transform->posicion.y});

    // 2) Acomodo del cuerpo: solo aplica si la escena le dio un ICPartesCuerpo
    auto cuerpo = getComponente<ICPartesCuerpo>();
    if (!cuerpo)
        return;

    // Cada parte observa a su líder: la primera a la cabeza, las demás a la parte anterior.
    // 'lider_se_movio' y 'pos_lider' describen a ese líder en este frame
    bool lider_se_movio = !(transform->posicion == anterior);
    CE::Vector2D pos_lider = transform->posicion;

    for (auto &parte : cuerpo->partes)
    {
        bool parte_se_movio = false;

        // Solo un líder en movimiento deja rastro. Así, si la cabeza se detiene,
        // el cuerpo también se detiene sin encimarse sobre ella
        if (lider_se_movio)
        {
            // La posición actual del líder entra al final del rastro
            parte.posiciones.encolar(pos_lider);

            if (parte.hacerAccion)
            {
                // Persecución: la parte ocupa la posición más antigua del rastro.
                // Como entra una y sale una, el rastro conserva siempre el mismo largo,
                // que es la distancia (en frames) entre la parte y su líder
                parte.pos.pos_prev = parte.pos.posicion;
                parte.pos.posicion = parte.posiciones.desencolar();
                parte_se_movio = true;
            }
            else
            {
                // Espera: el timer cuenta posiciones acumuladas; al llegar al límite
                // el rastro ya mide lo que debe medir y la parte empieza a perseguir
                parte.timer.curr_frame++;
                if (parte.timer.termino())
                    parte.hacerAccion = true;
            }
        }

        // La figura refleja la posición lógica de la parte
        parte.parte.figura.setPosition({parte.pos.posicion.x, parte.pos.posicion.y});

        // Esta parte se convierte en el líder de la siguiente
        lider_se_movio = parte_se_movio;
        pos_lider = parte.pos.posicion;
    }
}

void EnteVibora::draw(sf::RenderTarget &target, sf::RenderStates state) const
{
    state.transform *= getTransform();

    // El cuerpo se dibuja de la última parte a la primera, de modo que las partes
    // cercanas a la cabeza (y la cabeza misma) quedan por encima de las lejanas
    if (auto cuerpo = getComponente<ICPartesCuerpo>())
    {
        for (auto it = cuerpo->partes.rbegin(); it != cuerpo->partes.rend(); ++it)
            target.draw(it->parte.figura, state);
    }
    target.draw(cabeza, state);
}
} // namespace IVJ
