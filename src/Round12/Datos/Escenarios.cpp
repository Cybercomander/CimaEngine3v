#include "Escenarios.hpp"
#include <iostream>
#include <map>

namespace Round12
{
    namespace
    {
        // los tres gimnasios comparten la geometría del ring (mismo overlay de cuerdas)
        const std::vector<Pt> LONA = {{520,470},{1400,470},{1630,740},{290,740}};
        const Rect CAJA_RING{170,400,1590,520}; // ring + faldón: no se cruza caminando
        const std::vector<Pt> PISO_GYM = {{0,450},{130,330},{340,255},{1580,255},{1790,330},{1920,440},
                                          {1920,1080},{0,1080}};

        EscenarioR12 gimnasio(const std::string& carpeta, std::vector<Rect> muebles, std::vector<PropR12> props,
                              std::vector<EstacionR12> estaciones = {})
        {
            EscenarioR12 e;
            e.carpeta = carpeta;
            e.ring = true;
            e.piso = {PISO_GYM};
            e.bloqueos = std::move(muebles);
            e.bloqueos.push_back(CAJA_RING);
            e.ring_poligono = LONA;
            e.ring_frente_y = 770.f;
            e.props = std::move(props);
            e.estaciones = std::move(estaciones);
            e.entrada = {960,1010};
            e.esquina_diego = {380,700};
            e.esquina_rival = {1540,700};
            e.escalera_izq = {300,960};
            e.escalera_der = {1620,960};
            return e;
        }

        std::map<std::string,EscenarioR12> crear()
        {
            std::map<std::string,EscenarioR12> m;
            // bancas de abajo, garrafón/banca de la izquierda, pesas de la derecha
            const std::vector<Rect> MUEBLES_GYM = {{100,1010,360,70},{1460,1010,370,70},{0,280,270,170},{1790,180,130,260}};
            m["gym"] = gimnasio("01_sanchez_boxing_gym_tijuana",MUEBLES_GYM,
                {{"cartel_sanchez",{1850,800},false},{"silla",{215,770},false},
                 {"caja_madera",{1250,330}},{"bote_basura",{620,330}}},
                {{"agua",{120,470}},{"platano",{1480,300}},{"costal",{1680,320}},
                 {"cuerda",{560,990}},{"manoplas",{960,600}}});
            m["visitante"] = gimnasio("02_gimnasio_visitante",MUEBLES_GYM,
                {{"gradas",{960,1075},false},{"banca",{660,1000}},{"silla",{215,770},false},
                 {"lockers",{60,640}},{"costal",{1850,640}},{"pesas",{90,880}},{"fuente_agua",{1840,900}}});
            m["mexicali"] = gimnasio("03_gimnasio_mexicali_torneo",
                {{0,200,330,520},{1590,200,330,520},{640,930,640,150},{100,1010,360,70},{1460,1010,370,70},
                 {0,830,150,140},{1770,830,150,140}},
                {{"cartel_mexicali",{1080,330}},{"valla",{560,1000}},{"banderola",{1400,1000},false},
                 {"silla",{215,770},false},{"gradas",{760,320}}});

            EscenarioR12 sf;
            sf.carpeta = "04_skatepark_santa_fe_tijuana";
            sf.piso = {{{0,560},{230,330},{700,250},{1220,250},{1700,330},{1920,560},{1920,1080},{0,1080}}};
            sf.bloqueos = {{10,360,220,170},{1690,360,220,170},{80,970,400,110},{1440,970,400,110},{1830,280,90,90}};
            sf.props = {{"rampa_skate",{520,520},false},{"escaleras",{1400,520},false},{"barandal",{760,700}},
                        {"cono",{1180,680}},{"maceta",{300,620}},{"arbol",{120,1060},false},{"arbusto",{1800,1060},false},
                        {"poste_luz",{1640,620}},{"sombra_muro",{960,330},false},{"bandera",{960,860},false},
                        {"caja_madera",{1500,820}}};
            sf.entrada = {960,1000};
            m["santa_fe"] = sf;

            EscenarioR12 ci;
            ci.carpeta = "06_circuito_atletico_rosarito";
            ci.piso = {{{0,300},{1920,300},{1920,1080},{0,1080}}};
            ci.bloqueos = {{170,1010,350,70},{1390,1010,350,70},{1800,370,120,130}};
            ci.props = {{"valla",{960,960},false},{"cono",{1700,640},false},{"bandera",{1160,955},false},
                        {"banderola",{80,640},false},{"arbol",{40,1075},false},{"arbusto",{1880,1075},false},
                        {"maceta",{1880,300},false},{"sombra_muro",{960,300},false}};
            ci.entrada = {960,1000};
            m["circuito"] = ci;
            return m;
        }
    }

    const EscenarioR12& escenario(const std::string& id)
    {
        static const auto TODOS = crear();
        auto it = TODOS.find(id);
        if(it != TODOS.end()) return it->second;
        std::cerr<<"[escenarios] no hay '"<<id<<"', se usa el gym\n";
        return TODOS.at("gym");
    }

    std::string rutaCapa(const EscenarioR12& e, const char* capa)
    {
        return "escenarios/" + e.carpeta + "/" + capa + "_1920x1080.png";
    }
}
