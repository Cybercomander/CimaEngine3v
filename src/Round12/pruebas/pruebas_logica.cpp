// Pruebas de la lógica de ROUND 12 (sin SFML). Regresa != 0 si algo falla.
#include <Round12/Logica/Stats.hpp>
#include <Round12/Logica/ModDanio.hpp>
#include <Round12/Logica/Combos.hpp>
#include <Round12/Logica/Conteo.hpp>
#include <Round12/Logica/Jueces.hpp>
#include <Round12/Logica/Eventos.hpp>
#include <Round12/Logica/Partida.hpp>
#include <Round12/Logica/Runner.hpp>
#include <Round12/Logica/MapaColision.hpp>
#include <cstdio>
#include <fstream>
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

    void pruebasPartida()
    {
        auto& p = Partida::Get();
        p.nueva({{TipoPaso::PELEA,"semifinal"},{TipoPaso::CINEMATICA,"epilogo"},{TipoPaso::FIN,"fin"}},false);
        // la semifinal contra Ricardo se pierde siempre y aun así la historia avanza
        CHECK(p.alTerminarPelea(Objetivo::PERDER_FORZADO,false,false));
        CHECK(p.historia().actual().id == "epilogo");
        p.nueva({{TipoPaso::PELEA,"javier"},{TipoPaso::FIN,"fin"}},false);
        CHECK(!p.alTerminarPelea(Objetivo::GANAR,false,true));
        CHECK(p.historia().actual().id == "javier");
        CHECK(!p.alTerminarPelea(Objetivo::SOBREVIVIR,false,false));
        CHECK(p.alTerminarPelea(Objetivo::SOBREVIVIR,false,true));
        CHECK(p.historia().fin());
        p.mejorar(&Atributos::poder,+5);
        CHECK(p.diego().poder == 10);
        CHECK(!p.tiene(Habilidad::COMBOS));
        p.desbloquear(Habilidad::COMBOS);
        CHECK(p.tiene(Habilidad::COMBOS));
        CHECK(p.nuevasHabilidades().size() == 1);
    }

    void pruebasEventos()
    {
        auto& e = Eventos::Get();
        int a = 0, b = 0, c = 0;
        e.suscribir([&](Evento, float v){ a += (int)v; });
        e.suscribir([&](Evento, float v){ b += (int)v; });
        int id = e.suscribir([&](Evento, float){ c++; });
        e.desuscribir(id);
        e.emitir(Evento::CAMPANA,2.f);
        CHECK(a == 2 && b == 2 && c == 0);
        e.limpiar();
    }

    void pruebasRunner()
    {
        Runner::Config cfg;
        Runner r{cfg};
        // dt grande: el barrido detecta un obstáculo delgado que quedó entre dos posiciones
        r.paso(0.5f,+1,false,false,false);
        r.paso(0.5f,+1,false,false,false);
        CHECK(r.chocaCon(Rect{60.f,0.f,2.f,16.f}));
        CHECK(r.vel < 40.f);
        Runner aire{cfg};
        float t = 0.f;
        while(aire.aire > 0.f && t < 60.f) { aire.paso(1.f/30.f,+1,false,false,false); t += 1.f/30.f; }
        CHECK(t > 15.f && t < 19.f);
        CHECK(aire.fallo());
        Runner salto{cfg};
        salto.paso(1.f/30.f,0,true,false,false);
        for(int i = 0; i < 10; ++i) salto.paso(1.f/30.f,0,false,false,false);
        CHECK(salto.y > 16.f); // saltando pasa por encima de un castillo de 16 px
        Runner cp{cfg};
        cp.x = 3000.f; cp.tiempo = 20.f; cp.checkpoint();
        cp.x = 4000.f; cp.aire = 0.f;
        cp.reaparecer();
        CHECK(cp.x == 3000.f && cp.tiempo == 60.f && cp.aire == 100.f);
    }

    void pruebasMapa()
    {
        const char* ruta = "r12_mapa_prueba.txt";
        { std::ofstream f{ruta}; f<<"3 2\n1 1 0\n0 1 1\n"; }
        auto rects = MapaColision::cargar(ruta,1);
        std::remove(ruta);
        CHECK(rects.size() == 2);
        if(rects.size() == 2)
        {
            CHECK(rects[0].x == 0 && rects[0].y == 0 && rects[0].w == 16 && rects[0].h == 8);
            CHECK(rects[1].x == 8 && rects[1].y == 8 && rects[1].w == 16 && rects[1].h == 8);
        }
        CHECK(MapaColision::cargar("no_existe.txt",1).empty());
    }
}

int main()
{
    pruebasPartida();
    pruebasEventos();
    pruebasRunner();
    pruebasMapa();
    pruebasStats();
    pruebasCombos();
    pruebasConteo();
    pruebasJueces();
    std::cout<<revisiones<<" revisiones, "<<fallas<<" fallas\n";
    return fallas ? 1 : 0;
}
