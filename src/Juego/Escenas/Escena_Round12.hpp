#pragma once
#include <Motor/Primitivos/Escena.hpp>
namespace IVJ
{
    // Lab 8 Animaciones con los sprites propios de ROUND12 (Sanchez Boxing GYM).
    // Diego: idle, caminar, correr (Shift) y jab (Espacio).
    // Rivales: los 4 comparten los mismos nodos IdleRival/JabRival.
    class Escena_Round12 : public CE::Escena
    {
        public:
            explicit Escena_Round12();
            virtual ~Escena_Round12(){};
            void onInit() override;
            void onFinal() override;
            void onUpdate(float dt) override;
            void onInputs(const CE::Botones& accion) override;
            void onRender() override;
            std::shared_ptr<Entidad> getJugador() override {return diego;}
        private:
            int inicializar{1};
            std::shared_ptr<Entidad> diego;
            std::shared_ptr<Entidad> fondo;
    };
}
