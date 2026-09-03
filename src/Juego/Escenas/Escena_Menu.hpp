// Guarda de inclusión: el archivo se procesa una sola vez por unidad de traducción
#pragma once
#include <Motor/Primitivos/Escena.hpp>
#include <Juego/Figuras/Figuras.hpp>

namespace IVJ
{
    // Escena del menú principal del juego.
    // Hereda de CE::Escena, por lo que el motor la puede registrar en el gestor de
    // escenas y llamarle los hooks del ciclo de vida (init, update, inputs, render, final).
    // Su función es mostrar las opciones "Iniciar" y "Salir" y permitir elegir una
    // con el teclado usando el esquema de botones que ella misma registra
    class Escena_Menu : public CE::Escena
    {
        public:
            // Constructor por defecto. 'explicit' evita conversiones implícitas
            explicit Escena_Menu();
            // Destructor virtual: al destruir la escena por un apuntador a CE::Escena
            // se ejecuta el destructor de la clase real
            virtual ~Escena_Menu(){};
            // Inicializa la escena: registra los botones y crea los objetos del menú
            void onInit() override;
            // Limpia la escena cuando el motor la descarga
            void onFinal() override;
            // Actualiza cada frame la lógica de la escena
            void onUpdate(float dt) override;
            // Procesa las acciones mapeadas del teclado (arriba, abajo, aceptar)
            void onInputs(const CE::Botones& accion) override;
            // Encola los objetos de la escena en el sistema de render
            void onRender() override;
        private:
            // Bandera de una sola ejecución: evita repetir el contenido de onInit
            // si el motor vuelve a cargar la escena
            int inicializar{1};
            // Distancia vertical en píxeles que recorre el marcador al cambiar de opción
            float salto;
            // Opción actualmente seleccionada: 1 = Iniciar, 0 = Salir.
            // Al ser solo dos opciones se alterna con el operador XOR (opcion ^= 1)
            unsigned int opcion;
    };
}
