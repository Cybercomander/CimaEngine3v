// Pruebas de la lógica de ROUND 12 (sin SFML). Regresa != 0 si algo falla.
#include <Round12/Logica/Stats.hpp>
#include <Round12/Logica/ModDanio.hpp>
#include <Round12/Logica/Combos.hpp>
#include <Round12/Logica/Conteo.hpp>
#include <Round12/Logica/Jueces.hpp>
#include <random>
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

    void pruebasCombos()
    {
        DetectorCombos d;
        CHECK(d.registrar(Golpe::JAB,0.0f,false) == Combo::NINGUNO);
        CHECK(d.registrar(Golpe::JAB,0.3f,false) == Combo::NINGUNO);
        CHECK(d.registrar(Golpe::RECTO,0.6f,false) == Combo::C112);
        d.reiniciar();
        d.registrar(Golpe::JAB,0.f,true);
        d.registrar(Golpe::RECTO,0.3f,true);
        CHECK(d.registrar(Golpe::GANCHO,0.6f,true) == Combo::C123);
        CHECK(d.registrar(Golpe::UPPERCUT,0.9f,true) == Combo::FRONTERA);
        d.reiniciar();
        d.registrar(Golpe::JAB,0.f,false);
        d.registrar(Golpe::RECTO,0.3f,false);
        d.registrar(Golpe::GANCHO,0.6f,false);
        // sin concentración máxima no hay Frontera; el final 3-4 sí es combo
        CHECK(d.registrar(Golpe::UPPERCUT,0.9f,false) == Combo::C34);
        d.reiniciar();
        d.registrar(Golpe::JAB,0.f,false);
        d.registrar(Golpe::JAB,0.3f,false);
        // más de 0.45 s entre golpes reinicia la cadena
        CHECK(d.registrar(Golpe::RECTO,0.9f,false) == Combo::NINGUNO);
    }

    void pruebasConteo()
    {
        Conteo c{0};
        // sostener J (repetición del SO) cuenta una sola vez
        for(int i = 0; i < 4; ++i) c.pulsar('J');
        CHECK(c.validas() == 1);
        for(int i = 0; i < 5; ++i) { c.pulsar('K'); c.pulsar('J'); }
        CHECK(c.validas() == 11);
        CHECK(c.levantado());
        Conteo f{0};
        for(int i = 0; i < 100; ++i) f.tick(0.1f);
        CHECK(f.fuera());
        CHECK(f.numero() == 10);
        Conteo dos{1}; // segunda caída: hacen falta 16
        for(int i = 0; i < 6; ++i) { dos.pulsar('J'); dos.pulsar('K'); }
        CHECK(!dos.levantado());
        CHECK(probLevantarRival(perfil(Personaje::RICARDO),0) > probLevantarRival(perfil(Personaje::KEVIN),0));
        CHECK(probLevantarRival(perfil(Personaje::NINO),3) == 0.f);
    }

    void pruebasJueces()
    {
        std::vector<RoundStats> diego(3, RoundStats{30,5,60.f,8.f,0,1});
        std::mt19937 rng{1};
        CHECK(decidir(diego,rng) == Veredicto::DIEGO_UNANIME);
        std::vector<RoundStats> rival(3, RoundStats{4,25,6.f,50.f,1,0});
        CHECK(decidir(rival,rng) == Veredicto::RIVAL_UNANIME);
        std::vector<RoundStats> parejo(3, RoundStats{10,10,20.f,20.f,0,0});
        std::mt19937 a{7}, b{7};
        CHECK(decidir(parejo,a) == decidir(parejo,b));
    }
}

int main()
{
    pruebasStats();
    pruebasCombos();
    pruebasConteo();
    pruebasJueces();
    std::cout<<revisiones<<" revisiones, "<<fallas<<" fallas\n";
    return fallas ? 1 : 0;
}
