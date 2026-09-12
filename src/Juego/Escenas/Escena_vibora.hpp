#pragma once

#include <Juego/objetos/EnteVibora.hpp>
#include <Motor/Primitivos/Escena.hpp>
#include <memory>

namespace IVJ
{
// Escena del laboratorio de la víbora: una cabeza controlada con el teclado
// cuyo cuerpo la persigue y crece una parte cada SEGUNDOS_NUEVA_PARTE segundos
class Escena_vibora : public CE::Escena
{
  public:
    // Constructor por defecto. 'explicit' evita conversiones implícitas
    explicit Escena_vibora();
    // Destructor virtual: al destruir la escena por un apuntador a CE::Escena
    // se ejecuta el destructor de la clase real
    virtual ~Escena_vibora() {};
    // Registra los botones y crea la víbora con su cuerpo inicial
    void onInit() override;
    // Limpia la escena cuando el motor la descarga
    void onFinal() override;
    // Hace crecer la víbora con el tiempo y actualiza los objetos
    void onUpdate(float dt) override;
    // Cambia la dirección de la cabeza según las flechas / WASD
    void onInputs(const CE::Botones &accion) override;
    // Encola los objetos de la escena en el sistema de render
    void onRender() override;

  private:
    // Segundos que pasan entre cada parte nueva del cuerpo
    static constexpr float SEGUNDOS_NUEVA_PARTE = 3.f;
    // Velocidad de la cabeza en píxeles por segundo
    static constexpr float VELOCIDAD_CABEZA = 180.f;

    // Bandera de una sola ejecución: evita repetir el contenido de onInit
    // si el motor vuelve a cargar la escena
    bool inicializar{true};
    // Segundos acumulados desde que se agregó la última parte
    float tiempo_nueva_parte{0.f};
    // Apuntador a la cabeza; el pool comparte la propiedad del mismo objeto
    std::shared_ptr<EnteVibora> vibora;
};
} // namespace IVJ
