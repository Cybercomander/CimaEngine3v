#pragma once
#include <Motor/Primitivos/Escena.hpp>
#include <Motor/Componentes/IComponentes.hpp>
#include <Round12/objetos/Hud.hpp>
#include <string>

namespace Round12
{
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

            // cámara propia (vista del mundo nativo, 288x192 por defecto)
            void usarCamara(float w = 288.f, float h = 192.f);
            void moverCamara(float x, float y);

            std::string id;
            CE::IControl control;
            Widget hud;
        private:
            bool cargada{false};
            bool pendiente{true};
            int id_camara{0};
    };
}
