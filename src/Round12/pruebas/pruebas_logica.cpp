// Pruebas de la lógica de ROUND 12 (sin SFML). Regresa != 0 si algo falla.
#include <Round12/Logica/Stats.hpp>
#include <Round12/Logica/ModDanio.hpp>
#include <cmath>
#include <iostream>

using namespace Round12;

namespace
{
    int fallas = 0;
    int revisiones = 0;
}
#define CHECK(c) do{ ++revisiones; if(!(c)){ ++fallas; std::cout<<__FILE__<<":"<<__LINE__<<" FALLA: "<<#c<<"\n"; } }while(0)
#define CERCA(a,b) CHECK(std::fabs((a)-(b)) < 0.01f)

namespace
{
    float danio(Golpe g, Personaje atk, Personaje def, bool bloq = false, bool contra = false, float aire = 100.f)
    {
        auto a = perfil(atk), d = perfil(def);
        auto b = barrasIniciales(a);
        b.aire = aire;
        ContextoGolpe ctx{g,a,b,d,bloq,false,contra,false};
        return cadenaDanio()->calcular(ctx);
    }

    void pruebasStats()
    {
        CERCA(barrasIniciales(perfil(Personaje::DIEGO)).resistencia_max, 88.f);
        CERCA(barrasIniciales(perfil(Personaje::DIEGO)).concentracion, 85.f);
        CERCA(danio(Golpe::JAB,Personaje::DIEGO,Personaje::BRANDON), 3.9f);
        CERCA(danio(Golpe::GANCHO,Personaje::DIEGO,Personaje::RICARDO,true), 8.f*1.3f*(1.f-0.63f));
        CERCA(danio(Golpe::JAB,Personaje::DIEGO,Personaje::BRANDON,false,true),
              2.f*danio(Golpe::JAB,Personaje::DIEGO,Personaje::BRANDON));
        CERCA(danio(Golpe::JAB,Personaje::DIEGO,Personaje::BRANDON,false,false,20.f), 1.95f);
        auto d = perfil(Personaje::DIEGO);
        auto b = barrasIniciales(d);
        float sin_sed = regenAire(b,d,false,false);
        b.sed = 80.f;
        CERCA(regenAire(b,d,false,false), sin_sed*0.5f);
        CHECK(costoAire(Golpe::UPPERCUT,d) > costoAire(Golpe::JAB,d));
        CERCA(velocidadPasos(d), 51.f);
    }
}

int main()
{
    pruebasStats();
    std::cout<<revisiones<<" revisiones, "<<fallas<<" fallas\n";
    return fallas ? 1 : 0;
}
