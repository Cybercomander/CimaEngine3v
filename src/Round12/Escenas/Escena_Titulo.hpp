#pragma once
#include <Motor/Primitivos/Escena.hpp>

namespace Round12
{
    class Escena_Titulo : public CE::Escena
    {
        public:
            void onInit() override;
            void onFinal() override {}
            void onUpdate(float) override {}
            void onInputs(const CE::Botones&) override {}
            void onRender() override;
    };
}
