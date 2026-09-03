#pragma once

    #include <Motor/Inputs/Botones.hpp>
    #include <Motor/Primitivos/GestorAssets.hpp>
    #include <Motor/Primitivos/GestorEscenas.hpp>
    #include <Motor/Render/Render.hpp>
    #include <Juego/Figuras/Figuras.hpp>
    #include <Juego/objetos/Texto.hpp>
    #include <Juego/Componentes/IJComponentes.hpp>
    #include <Juego/Sistemas/Sistemas.hpp>
    #include <Juego/Figuras/ClasePrueba.hpp>
    #include <iostream>
    #include <fstream>
    #include <string>
    #include <vector>
    #include <memory>

namespace IVJ{
    class Escena_Figuras : public CE::Escena
    {
        public:

            explicit Escena_Figuras();
            virtual ~Escena_Figuras(){};
            void onInit() override;
            void insertFiguras();
            void onFinal() override;
            void onUpdate(float dt) override;
            void onInputs(const CE::Botones& accion) override;
            void onRender() override;
        private:
            int inicializar{1};
            
    };
}