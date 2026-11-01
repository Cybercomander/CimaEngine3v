#include<iostream>
#include "../Motor/Motor.hpp"
#include "../Motor/Utils/Utils.hpp"
#include "Selector.hpp"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <string>

// Uso: CimaEngine3v [--juego=labs|round12] [--prueba-selector=N]
//      [--r12-paso=tipo:id] [--autopiloto] [--rapido=N]
// sin --juego arranca en el lanzador para elegir
int main(int argc, char** argv)
{
    std::srand(std::time({}));
    auto modo = Selector::Modo::LANZADOR;
    int prueba = 0;
    for(int i = 1; i < argc; ++i)
    {
        std::string a = argv[i];
        if(a == "--juego=labs") modo = Selector::Modo::LABS;
        else if(a == "--juego=round12") modo = Selector::Modo::ROUND12;
        else if(a.rfind("--prueba-selector=",0) == 0) prueba = std::atoi(a.c_str()+18);
        // pruebas de ROUND 12 sin teclado: --r12-paso=pelea:javier --autopiloto --rapido=8
        else if(a.rfind("--r12-paso=",0) == 0) { Round12::JuegoR12::prueba.paso = a.substr(11); modo = Selector::Modo::ROUND12; }
        else if(a == "--autopiloto") Round12::JuegoR12::prueba.autopiloto = true;
        else if(a.rfind("--rapido=",0) == 0) Round12::JuegoR12::prueba.rapido = std::max(1,std::atoi(a.c_str()+9));
        else std::cerr<<"argumento desconocido: "<<a<<"\n";
    }
    CE::MotorConfig config;
    config.vW=1080;
    config.vH=720;
    config.titulo="CimaEngine3v";
    std::cout<<"Iniciando "<<config.titulo<<"\n";

    auto selector = std::make_unique<Selector>(modo,prueba);
    Selector* sel = selector.get();
    CE::Motor motor{config,std::move(selector)};
    int r = motor.OnRun();
    return sel->fallas() ? 1 : r;
}
