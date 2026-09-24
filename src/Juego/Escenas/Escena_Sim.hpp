#pragma once
#include "Motor/Utils/Vector2D.hpp"
#include <Juego/Figuras/Figuras.hpp>
#include <Motor/Componentes/IComponentes.hpp>
#include <Motor/Primitivos/Escena.hpp>
namespace IVJ
{
// Lab 5: simulación de selección natural. Los entes (pentágonos) buscan comida
// (círculos) dentro del mundo (rectángulo) y la llevan a casa para consumirla
class Escena_Sim : public CE::Escena
{
  public:
    explicit Escena_Sim();
    virtual ~Escena_Sim() {};
    void onInit() override;
    void onFinal() override;
    void onUpdate(float dt) override;
    void onInputs(const CE::Botones &accion) override;
    void onRender() override;

  private:
    // Agrega al frente del pool la comida y el mundo de una generación nueva
    void agregarComidaYMundo();

  private:
    int inicializar{1};
    int entes_iniciales;
    int comida_iniciales;
    int generacion;
    bool generacion_termino;
    CE::ITimer timer_generacion;
    CE::ITexto mensaje;
    CE::Vector2D punto_inferior;
    CE::Vector2D punto_superior;
    std::shared_ptr<Rectangulo> mundo;
};
} // namespace IVJ
