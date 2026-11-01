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
#include <Round12/Logica/EstiloIA.hpp>
#include <Round12/Logica/Combate.hpp>
#include <Round12/Logica/Ritmo.hpp>
#include <Round12/Datos/Niveles.hpp>
#include <Round12/Rutas.hpp>
#include <cstdio>
#include <filesystem>
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

    void pruebasIA()
    {
        CHECK(elegirGolpe({0.7f,0.2f,0.05f,0.05f},0.75f) == Golpe::RECTO);
        CHECK(elegirGolpe({0.7f,0.2f,0.05f,0.05f},0.1f) == Golpe::JAB);
        CHECK(elegirGolpe({0.7f,0.2f,0.05f,0.05f},0.999f) == Golpe::UPPERCUT);
        CHECK(estilo(Personaje::JAVIER).pesos_golpe[2] > estilo(Personaje::BRANDON).pesos_golpe[2]);
        CHECK(estilo(Personaje::KEVIN).distancia_ideal > estilo(Personaje::JAVIER).distancia_ideal);
    }

    void pruebasCombate()
    {
        auto d = perfil(Personaje::DIEGO), b = perfil(Personaje::BRANDON);
        Combate limpio{d,b,Objetivo::SOBREVIVIR,false};
        float r0 = limpio.barras(false).resistencia;
        limpio.recibir(false,Golpe::JAB,false,false,false,false);
        float sin_guardia = r0 - limpio.barras(false).resistencia;
        Combate bloq{d,b,Objetivo::SOBREVIVIR,false};
        bloq.recibir(false,Golpe::JAB,true,false,false,false);
        CHECK(r0 - bloq.barras(false).resistencia < sin_guardia);
        CHECK(limpio.round().conectados_diego == 1);

        Combate sin_aire{d,b,Objetivo::GANAR,false};
        sin_aire.barras(true).aire = 3.f;
        CHECK(!sin_aire.lanzar(true,Golpe::UPPERCUT));
        CERCA(sin_aire.barras(true).aire, 3.f);
        CHECK(sin_aire.lanzar(true,Golpe::JAB) == false); // 3 < costo del jab
        sin_aire.barras(true).aire = 50.f;
        CHECK(sin_aire.lanzar(true,Golpe::JAB));
        CHECK(sin_aire.barras(true).aire < 50.f);

        Combate segundo{d,b,Objetivo::GANAR,true};
        segundo.barras(true).aire = 5.f;
        segundo.tick(0.01f,{},{});
        CHECK(segundo.barras(true).aire > 50.f);
        segundo.barras(true).aire = 5.f;
        segundo.tick(0.01f,{},{});
        CHECK(segundo.barras(true).aire < 10.f); // solo una vez por pelea

        Combate forzado{d,perfil(Personaje::RICARDO),Objetivo::PERDER_FORZADO,false};
        for(int i = 0; i < 100; ++i) forzado.recibir(false,Golpe::UPPERCUT,false,false,false,false);
        CHECK(forzado.barras(false).resistencia >= 1.f);
        for(int i = 0; i < 100; ++i) forzado.recibir(true,Golpe::UPPERCUT,false,false,false,false);
        CHECK(forzado.barras(true).resistencia >= 1.f); // en la semifinal no hay nocaut
        std::mt19937 rng{3};
        CHECK(forzado.veredicto(rng) == Veredicto::RIVAL_UNANIME);

        Combate cabeza{d,b,Objetivo::GANAR,false};
        float c0 = cabeza.barras(true).concentracion;
        cabeza.recibir(true,Golpe::GANCHO,false,false,false,false);
        CHECK(cabeza.barras(true).concentracion < c0);
    }

    void pruebasRitmo()
    {
        CHECK(enPulso(1.08f));
        CHECK(enPulso(0.95f));
        CHECK(!enPulso(1.2f));
        CHECK(!enPulso(0.25f));
        CHECK(pulsoMasCercano(1.08f) == 2);
        CHECK(pulsoMasCercano(0.95f) == 2);
    }

    void revisarSecuencia(const std::vector<Paso>& pasos)
    {
        CHECK(!pasos.empty() && pasos.front().id == "prologo");
        CHECK(!pasos.empty() && pasos.back().tipo == TipoPaso::FIN);
        for(auto& p : pasos)
        {
            if(!existeNivel(p.tipo,p.id)) { std::cout<<"  sin nivel: "<<p.id<<"\n"; CHECK(false); }
            if(p.tipo == TipoPaso::CINEMATICA)
            {
                auto ruta = std::string(R12_DATOS) + "/dialogos/" + p.id + ".txt";
                if(!std::filesystem::exists(ruta)) { std::cout<<"  sin diálogo: "<<ruta<<"\n"; CHECK(false); }
            }
        }
    }

    void pruebasHistoria()
    {
        auto demo = secuenciaDemo();
        revisarSecuencia(demo);
        revisarSecuencia(secuenciaHistoria());
        // los 3 niveles del demo en orden: gimnasio, playa y Mexicali contra Javier
        std::vector<std::string> niveles;
        for(auto& p : demo)
            if(p.id == "gym_tutorial" || p.id == "playa" || p.id == "javier") niveles.push_back(p.id);
        CHECK((niveles == std::vector<std::string>{"gym_tutorial","playa","javier"}));
        bool hay_semifinal = false;
        for(auto& p : secuenciaHistoria())
            if(p.id == "semifinal_ricardo") hay_semifinal = pelea(p.id).objetivo == Objetivo::PERDER_FORZADO;
        CHECK(hay_semifinal);
        CHECK(pelea("final_ricardo").rounds == 12);
    }
}

int main()
{
    pruebasHistoria();
    pruebasRitmo();
    pruebasCombate();
    pruebasIA();
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
