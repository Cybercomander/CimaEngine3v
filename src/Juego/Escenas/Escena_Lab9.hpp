#pragma once
#include <Motor/Primitivos/Escena.hpp>
namespace IVJ
{
    // Escena del laboratorio 9 tal como la propone la guía: la escena del lab 8
    // más colisiones (IBoundingBox en el jugador, tiles con 'c' del layer 1,
    // techo tomado del atlas) resueltas con SistemaColAABBMid.
    class Escena_Lab9 : public CE::Escena
    {
        public:
            explicit Escena_Lab9();
            virtual ~Escena_Lab9(){};
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
