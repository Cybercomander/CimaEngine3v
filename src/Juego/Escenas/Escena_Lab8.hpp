#pragma once
#include <Motor/Primitivos/Escena.hpp>
namespace IVJ
{
    // Escena del laboratorio 8 tal como la propone la guía:
    // nave (IdleJugadores/MoverJugadores) y jefe (IdleBoss/AttackBoss) sobre la playa.
    // Tiene su propio jugador para no mezclar componentes con la escena "Sprites".
    class Escena_Lab8 : public CE::Escena
    {
        public:
            explicit Escena_Lab8();
            virtual ~Escena_Lab8(){};
            void onInit() override;
            void onFinal() override;
            void onUpdate(float dt) override;
            void onInputs(const CE::Botones& accion) override;
            void onRender() override;
            std::shared_ptr<Entidad> getJugador() override {return player;}
        private:
            int inicializar{1};
            std::shared_ptr<Entidad> player;
    };
}
