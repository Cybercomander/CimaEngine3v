#pragma once
#include "../Bosses/FSMBoss.hpp"
#include "Animador.hpp"

// Familia RIVAL: TODOS los enemigos (Brandon, Javier, Kevin, Ricardo) usan estos
// mismos 2 nodos. Funciona porque comparten tamaño (44x56), número de poses y
// la misma regla de inputs: IRangoAggro + ITarget (el jugador).
// Heredan de FSMBoss porque es la interfaz de nodos que reciben un target.
namespace IVJ
{
    class IdleRival : public FSMBoss
    {
        public:
            IdleRival();
            FSM* onInputs(Entidad& parent, CE::Vector2D& target) override;
            void onEntrar(const Entidad& obj) override;
            void onSalir(const Entidad& obj) override;
            void onUpdate(const Entidad& obj,float dt) override;
        private:
            Animador anim;
    };

    // Lanza jabs en ciclo mientras el jugador siga dentro del rango
    class JabRival : public FSMBoss
    {
        public:
            JabRival();
            FSM* onInputs(Entidad& parent, CE::Vector2D& target) override;
            void onEntrar(const Entidad& obj) override;
            void onSalir(const Entidad& obj) override;
            void onUpdate(const Entidad& obj,float dt) override;
        private:
            Animador anim;
    };
}
