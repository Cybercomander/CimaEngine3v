#pragma once
#include "../Bosses/FSMBoss.hpp"
#include "Animador.hpp"

// Nodos para las hojas de animación completas de ROUND12 (hojas/<personaje>.png):
// frames de 80x80, 6 columnas, fila = direccion*8 + accion.
// Todos los boxeadores (Diego y los 4 rivales) comparten tamaño, número de frames
// y orden de filas, así que una sola familia de nodos sirve para todos.
namespace IVJ
{
    class IDireccion;

    // mismo orden que las filas generadas por tools/round12_assets.py
    enum class AccionBox { CAMINAR = 0, GUARDIA, JAB, RECTO, GANCHO, UPPERCUT, ESQUIVA_IZQ, ESQUIVA_DER };

    // Composición: anima una acción en la fila que corresponde a la dirección
    // guardada en el componente IDireccion de la entidad
    class AnimBoxeo
    {
        public:
            AnimBoxeo(AccionBox accion, float frame_rate, bool ciclo);
            void entrar(const Entidad& obj);
            void actualizar(float dt) { anim.actualizar(dt); }
            bool terminado() const { return anim.terminado(); }
            void mirar(int dir);
            int direccion() const;
        private:
            AccionBox accion;
            Animador anim;
            IDireccion* dir{nullptr};
    };

    // ---- Familia JUGADOR (inputs: IControl) ----
    class GuardiaJugador : public FSM
    {
        public:
            GuardiaJugador();
            FSM* onInputs(const CE::IControl& control) override;
            void onEntrar(const Entidad& obj) override;
            void onSalir(const Entidad& obj) override;
            void onUpdate(const Entidad& obj,float dt) override;
        private:
            AnimBoxeo anim;
    };

    // turbo=false "CaminarJugador", turbo=true "CorrerJugador"
    class CaminarJugador : public FSM
    {
        public:
            explicit CaminarJugador(bool turbo);
            FSM* onInputs(const CE::IControl& control) override;
            void onEntrar(const Entidad& obj) override;
            void onSalir(const Entidad& obj) override;
            void onUpdate(const Entidad& obj,float dt) override;
        private:
            bool turbo;
            AnimBoxeo anim;
    };

    // un nodo para cualquier golpe: se reproduce una vez y regresa a guardia
    class GolpeJugador : public FSM
    {
        public:
            explicit GolpeJugador(AccionBox golpe);
            FSM* onInputs(const CE::IControl& control) override;
            void onEntrar(const Entidad& obj) override;
            void onSalir(const Entidad& obj) override;
            void onUpdate(const Entidad& obj,float dt) override;
        private:
            AnimBoxeo anim;
    };

    // ---- Familia RIVAL (inputs: ITarget + IRangoAggro) ----
    // espera: segundos en guardia antes de poder volver a golpear
    class GuardiaRival : public FSMBoss
    {
        public:
            explicit GuardiaRival(float espera = 0.f);
            FSM* onInputs(Entidad& parent, CE::Vector2D& target) override;
            void onEntrar(const Entidad& obj) override;
            void onSalir(const Entidad& obj) override;
            void onUpdate(const Entidad& obj,float dt) override;
        private:
            float espera;
            AnimBoxeo anim;
    };

    class GolpeRival : public FSMBoss
    {
        public:
            GolpeRival();
            FSM* onInputs(Entidad& parent, CE::Vector2D& target) override;
            void onEntrar(const Entidad& obj) override;
            void onSalir(const Entidad& obj) override;
            void onUpdate(const Entidad& obj,float dt) override;
        private:
            AnimBoxeo anim;
    };
}
