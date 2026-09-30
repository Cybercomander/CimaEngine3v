#pragma once
#include <Motor/Primitivos/Escena.hpp>
namespace IVJ
{
    class Escena_Sprites : public CE::Escena
    {
        public:
            explicit Escena_Sprites(std::shared_ptr<Entidad>& pref);
            virtual ~Escena_Sprites(){};
            void onInit() override;
            void onFinal() override;
            void onUpdate(float dt) override;
            void onInputs(const CE::Botones& accion) override;
            void onRender() override;
            std::shared_ptr<Entidad> getJugador() override {return player;}
        private:
            int inicializar{1};
            std::shared_ptr<Entidad>& player;
    };
}
