#pragma once
#include "../Bosses/FSMBoss.hpp"
#include "Animador.hpp"
#include <cstdint>
#include <vector>

// Nodos para las hojas de animación completas de ROUND12 (hojas/<personaje>.png):
// frames de 80x80, 6 columnas, fila = direccion*8 + accion.
// Todos los boxeadores (Diego y los 4 rivales) comparten tamaño, número de frames
// y orden de filas, así que una sola familia de nodos sirve para todos.
namespace IVJ
{
    class IDireccion;

    // mismo orden que las filas generadas por tools/round12_assets.py
    enum class AccionBox { CAMINAR = 0, GUARDIA, JAB, RECTO, GANCHO, UPPERCUT, ESQUIVA_IZQ, ESQUIVA_DER };

    // Bits de CE::IControl::botones para el combate (controles del GDD Avance 4)
    namespace BotonBox
    {
        constexpr std::uint32_t JAB      = 1u << 0; // J
        constexpr std::uint32_t RECTO    = 1u << 1; // K
        constexpr std::uint32_t GANCHO   = 1u << 2; // L
        constexpr std::uint32_t UPPERCUT = 1u << 3; // I
        constexpr std::uint32_t GUARDIA  = 1u << 4; // Shift (mantener)
        constexpr std::uint32_t ESQ_IZQ  = 1u << 5; // Q
        constexpr std::uint32_t ESQ_DER  = 1u << 6; // E
        constexpr std::uint32_t GOLPES   = JAB | RECTO | GANCHO | UPPERCUT;
    }

    // Composición: anima una acción en la fila que corresponde a la dirección
    // guardada en el componente IDireccion de la entidad
    class AnimBoxeo
    {
        public:
            AnimBoxeo(AccionBox accion, bool ciclo, std::vector<int> frames = {0,1,2,3,4,5});
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

    // Combinaciones: guarda el siguiente golpe si se presiona una tecla de golpe
    // nueva (no la que se sigue sosteniendo) mientras dura la animación actual
    class ColaGolpes
    {
        public:
            void leer(std::uint32_t botones);
            std::uint32_t siguiente() const { return sig; }
        private:
            bool primera{true};
            std::uint32_t sueltos{0};
            std::uint32_t sig{0};
    };

    // ---- Familia JUGADOR (inputs: IControl) ----
    // reposo: postura relajada (frame 0 de caminar)
    class IdleJugador : public FSM
    {
        public:
            IdleJugador();
            FSM* onInputs(const CE::IControl& control) override;
            void onEntrar(const Entidad& obj) override;
            void onSalir(const Entidad& obj) override;
            void onUpdate(const Entidad& obj,float dt) override;
        private:
            AnimBoxeo anim;
    };

    class CaminarJugador : public FSM
    {
        public:
            CaminarJugador();
            FSM* onInputs(const CE::IControl& control) override;
            void onEntrar(const Entidad& obj) override;
            void onSalir(const Entidad& obj) override;
            void onUpdate(const Entidad& obj,float dt) override;
        private:
            AnimBoxeo anim;
    };

    // Shift sostenido: sube la guardia y no se desplaza
    class BloqueoJugador : public FSM
    {
        public:
            BloqueoJugador();
            FSM* onInputs(const CE::IControl& control) override;
            void onEntrar(const Entidad& obj) override;
            void onSalir(const Entidad& obj) override;
            void onUpdate(const Entidad& obj,float dt) override;
        private:
            AnimBoxeo anim;
    };

    // un nodo para cualquier golpe (jab, recto, gancho, uppercut): se reproduce
    // una vez y encadena el siguiente golpe si se presionó a tiempo (combo)
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
            ColaGolpes cola;
    };

    // Q/E: esquiva lateral; un golpe presionado durante la esquiva sale al
    // terminar (contragolpe)
    class EsquivaJugador : public FSM
    {
        public:
            explicit EsquivaJugador(bool izquierda);
            FSM* onInputs(const CE::IControl& control) override;
            void onEntrar(const Entidad& obj) override;
            void onSalir(const Entidad& obj) override;
            void onUpdate(const Entidad& obj,float dt) override;
        private:
            AnimBoxeo anim;
            ColaGolpes cola;
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

    // lanza uno de los 4 golpes al azar
    class GolpeRival : public FSMBoss
    {
        public:
            explicit GolpeRival(AccionBox golpe);
            FSM* onInputs(Entidad& parent, CE::Vector2D& target) override;
            void onEntrar(const Entidad& obj) override;
            void onSalir(const Entidad& obj) override;
            void onUpdate(const Entidad& obj,float dt) override;
        private:
            AnimBoxeo anim;
    };
}
