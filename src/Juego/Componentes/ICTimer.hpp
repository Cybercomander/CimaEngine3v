#pragma once
#include "../../Motor/Componentes/IComponentes.hpp"

namespace IVJ
{
// Componente contador de frames. Mide cuántas veces ha ocurrido algo (curr_frame)
// frente a un límite (max_frame); por sí solo no avanza, lo incrementa quien lo usa.
// Es la versión del juego del CE::ITimer del motor, con los nombres de la práctica
class ICTimer : public CE::IComponentes
{
  public:
    // El timer nace en cero con el límite indicado
    explicit ICTimer(int max_frame);
    ~ICTimer() override {};
    // Devuelve una copia independiente del timer
    std::shared_ptr<IComponentes> clonar() const override
    {
        return std::make_shared<ICTimer>(*this);
    };

    // El timer terminó cuando el contador alcanzó el límite
    bool termino() const
    {
        return curr_frame >= max_frame;
    }

  public:
    // Frames contados hasta ahora
    int curr_frame;
    // Frames necesarios para considerar terminado el timer
    int max_frame;
};
} // namespace IVJ
