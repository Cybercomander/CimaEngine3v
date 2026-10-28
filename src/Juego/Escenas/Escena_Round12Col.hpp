#pragma once
#include <Motor/Primitivos/Escena.hpp>
namespace IVJ
{
    // Lab 9 Colisiones con los assets de ROUND12 (Sanchez Boxing GYM, mundo nativo 640x192):
    // - el combate es dentro del ring: el tilemap marca con 'c' las cuerdas y los
    //   límites de carril (assets/mapas/r12_sanchez_gym_col.txt)
    // - IBoundingBox en el cuerpo de cada boxeador (cabeza a pies, sin brazos)
    // - Carlos Sánchez en la esquina, como objeto individual tomado de su hoja de poses
    // - todas las animaciones de 4 direcciones con los controles del GDD (Avance 4):
    //   WASD moverse, J jab, K recto, L gancho, I uppercut, Shift guardia, Q/E esquivas
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
