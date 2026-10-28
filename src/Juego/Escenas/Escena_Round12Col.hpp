#pragma once
#include <Motor/Primitivos/Escena.hpp>
namespace IVJ
{
    // Lab 9 Colisiones con los assets de ROUND12 (Sanchez Boxing GYM, mundo nativo 640x192):
    // - tilemap de ROUND12 con 'c'/'n' (assets/mapas/r12_sanchez_gym_col.txt)
    // - IBoundingBox en los pies de cada boxeador y en el límite inferior del ring
    // - Carlos Sánchez como objeto individual tomado de su hoja de poses
    // - nuevas hojas de animación de 4 direcciones (guardia, caminar, correr, jab, recto)
    class Escena_Round12Col : public CE::Escena
    {
        public:
            explicit Escena_Round12Col();
            virtual ~Escena_Round12Col(){};
            void onInit() override;
            void onFinal() override;
            void onUpdate(float dt) override;
            void onInputs(const CE::Botones& accion) override;
            void onRender() override;
            std::shared_ptr<Entidad> getJugador() override {return diego;}
        private:
            int inicializar{1};
            int id_camara{0};
            std::shared_ptr<Entidad> diego;
            // punto que sigue la cámara: x de Diego limitada al mapa, y fija
            std::shared_ptr<Entidad> foco;
    };
}
