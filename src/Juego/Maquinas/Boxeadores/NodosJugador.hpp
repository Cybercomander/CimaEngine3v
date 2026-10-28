#pragma once
#include "../FSM.hpp"
#include "Animador.hpp"

// Familia JUGADOR: nodos controlados por IControl.
// Cualquier boxeador jugable de ROUND12 puede usarlos (misma hoja de 4 poses).
namespace IVJ
{
    // Respira alternando frente_firme y frente_guardia
    class IdleBoxeador : public FSM
    {
        public:
            IdleBoxeador();
            FSM* onInputs(const CE::IControl& control) override;
            void onEntrar(const Entidad& obj) override;
            void onSalir(const Entidad& obj) override;
            void onUpdate(const Entidad& obj,float dt) override;
        private:
            Animador anim;
    };

    // turbo=false -> "CaminarBoxeador" (velocidad normal)
    // turbo=true  -> "CorrerBoxeador"  (Shift, pasos más rápidos y altos)
    class MoverBoxeador : public FSM
    {
        public:
            explicit MoverBoxeador(bool turbo);
            FSM* onInputs(const CE::IControl& control) override;
            void onEntrar(const Entidad& obj) override;
            void onSalir(const Entidad& obj) override;
            void onUpdate(const Entidad& obj,float dt) override;
        private:
            bool turbo;
            Animador anim;
    };

    // Ataque: lateral_firme -> lateral_jab -> lateral_firme, una sola vez
    class JabBoxeador : public FSM
    {
        public:
            JabBoxeador();
            FSM* onInputs(const CE::IControl& control) override;
            void onEntrar(const Entidad& obj) override;
            void onSalir(const Entidad& obj) override;
            void onUpdate(const Entidad& obj,float dt) override;
        private:
            Animador anim;
    };
}
