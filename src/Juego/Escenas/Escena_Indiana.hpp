#pragma once
#include <Motor/Primitivos/Escena.hpp>
namespace IVJ
{
    // Escena del laboratorio 7: Indiana camina sobre el mapa de tileMapP7
    // y su sprite voltea a la derecha o izquierda según la dirección
    class Escena_Indiana : public CE::Escena
    {
        public:
            explicit Escena_Indiana();
            virtual ~Escena_Indiana(){};
            void onInit() override;
            void onFinal() override;
            void onUpdate(float dt) override;
            void onInputs(const CE::Botones& accion) override;
            void onRender() override;
            std::shared_ptr<Entidad> getJugador() override {return indiana;}
        private:
            // mantiene a Indiana dentro de los bordes del mapa
            void limitarAlMapa();
        private:
            int inicializar{1};
            std::shared_ptr<Entidad> indiana;
            // escala del mapa en el mundo (los tiles son de 16x16)
            float escala_mapa{2.f};
            float escala_indiana{1.5f};
    };
}
