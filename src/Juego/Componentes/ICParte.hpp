#pragma once
#include "../../Motor/Componentes/IComponentes.hpp"
#include "Cola.hpp"
#include "ICFigura.hpp"
#include "ICTimer.hpp"

namespace IVJ
{
// Componente que describe un segmento del cuerpo de la víbora.
// Cada parte persigue a su líder (la cabeza o la parte anterior) reproduciendo,
// con retraso, las posiciones por las que ese líder ya pasó:
//  - 'posiciones' guarda el rastro del líder en orden de llegada (FIFO)
//  - 'timer' cuenta cuántas posiciones del rastro se acumulan antes de empezar a moverse
//  - 'hacerAccion' se activa cuando el timer termina; desde ese momento la parte avanza
//    tomando la posición más antigua del rastro cada vez que el líder se mueve
// El tamaño del retraso (max_frame del timer) es la separación entre la parte y su líder
class ICParte : public CE::IComponentes
{
  public:
    // pos_inicial: lugar donde aparece la parte
    // w, h: tamaño del cuadro en píxeles
    // retraso: posiciones del líder que se acumulan antes de perseguirlo
    // relleno: color del cuadro
    explicit ICParte(const CE::Vector2D &pos_inicial, int w, int h, int retraso, const sf::Color &relleno);
    ~ICParte() override {};
    // Devuelve una copia independiente (incluye una copia del rastro de posiciones)
    std::shared_ptr<IComponentes> clonar() const override
    {
        return std::make_shared<ICParte>(*this);
    };

  public:
    // Rastro de posiciones del líder, auxiliar de la animación de persecución
    Cola<CE::Vector2D> posiciones;
    // Figura que se muestra en pantalla
    ICFigura parte;
    // Transformación de la parte en el espacio
    CE::ITransform pos;
    // Retraso antes de empezar a perseguir al líder
    ICTimer timer;
    // Indica si la parte ya persigue a su líder
    bool hacerAccion;
};
} // namespace IVJ
