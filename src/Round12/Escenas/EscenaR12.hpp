#pragma once
#include <Motor/Primitivos/Escena.hpp>
#include <Motor/Componentes/IComponentes.hpp>
#include <Round12/objetos/Hud.hpp>
#include <Round12/objetos/ListaDibujo.hpp>
#include <string>

namespace Round12
{
    // ventana 1080x720: banda superior 0-56 y banda inferior 664-720 para el HUD
    constexpr float BANDA_ARRIBA = 56.f/720.f, BANDA_ABAJO = 56.f/720.f;

    // Método plantilla: todas las escenas de ROUND 12 comparten el ciclo
    // (teclas del GDD, cámara propia, música, HUD en pantalla) y solo llenan los ganchos.
    class EscenaR12 : public CE::Escena
    {
        public:
            void onInit() final;
            void onFinal() final;
            void onUpdate(float dt) final;
            void onInputs(const CE::Botones& accion) final;
            void onRender() final;

            // el siguiente onInit arranca el nivel con este id (Datos/Niveles)
            void configurar(const std::string& nuevo_id) { id = nuevo_id; pendiente = true; }
            void pedirReinicio() { pendiente = true; }
            void soltarControles();
            virtual bool permitePausa() const { return true; }
            const std::string& getId() const { return id; }
        protected:
            virtual void cargar() {}                  // una vez: texturas, entidades fijas
            virtual void reiniciar() {}               // al entrar con un paso nuevo o al reiniciar
            virtual void actualizar(float dt) = 0;
            virtual void dibujarMundo() = 0;          // coordenadas del mundo (cámara de la escena)
            virtual void dibujarPantalla(sf::RenderTarget&, sf::Vector2f) {} // HUD extra
            virtual const char* musica() const { return nullptr; }
            virtual void alPresionar(const std::string& accion, bool presionado) { (void)accion; (void)presionado; }

            // cámara propia: el escenario de 1920x1080 completo en la franja central de la
            // ventana (bandas arriba y abajo para el HUD; ver BANDA_*)
            void usarCamara();
            void moverCamara(float x, float y);

            std::string id;
            CE::IControl control;
            Widget hud;
            ListaDibujo lista;   // personajes, objetos y cuerdas ordenados por profundidad
        private:
            bool cargada{false};
            bool pendiente{true};
            int id_camara{0};
    };
}
