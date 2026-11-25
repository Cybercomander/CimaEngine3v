#include "Niveles.hpp"
#include <iostream>
#include <map>

namespace Round12
{
    namespace
    {
        using H = Habilidad;
        const sf::Color PIEL{180,140,110};

        const std::map<std::string,ConfigPelea> PELEAS = {
            {"sparring_ninos",{Personaje::NINO,1,60.f,Objetivo::GANAR,"gym",false,1,PIEL}},
            {"sparring_brandon",{Personaje::BRANDON,3,60.f,Objetivo::SOBREVIVIR,"gym",false,1,sf::Color::White,false,
                {H::ESQUIVA},{H::CONTRAGOLPE},{{&Atributos::defensa,1},{&Atributos::concentracion,1}}}},
            {"torneo_local_1",{Personaje::NINO,2,60.f,Objetivo::GANAR,"visitante",true,1,PIEL,true}},
            {"torneo_local_2",{Personaje::NINO,2,60.f,Objetivo::GANAR,"visitante",true,1,sf::Color(150,110,90),true}},
            {"javier",{Personaje::JAVIER,3,60.f,Objetivo::GANAR,"mexicali",true,1,sf::Color::White,false,{H::FRONTERA}}},
            {"kevin",{Personaje::KEVIN,3,60.f,Objetivo::GANAR,"mexicali",true,1,sf::Color::White,false,{H::FRONTERA}}},
            {"semifinal_ricardo",{Personaje::RICARDO,3,60.f,Objetivo::PERDER_FORZADO,"mexicali",true,1}},
            {"revancha_javier",{Personaje::JAVIER,3,60.f,Objetivo::GANAR,"mexicali",true,2}},
            {"revancha_kevin",{Personaje::KEVIN,3,60.f,Objetivo::GANAR,"mexicali",true,2}},
            // la final llega hasta el round 12 (supuesto por el título del juego);
            // dificultad 1: Ricardo ya es el rival más fuerte y con +1 se perdía 1 de cada 5 veces
            {"final_ricardo",{Personaje::RICARDO,12,45.f,Objetivo::GANAR,"mexicali",true,1}}};

        const std::map<std::string,ConfigRecorrido> RECORRIDOS = {
            {"gym_tutorial",{"gym",{Estacion::AGUA,Estacion::PLATANO,Estacion::COSTAL,Estacion::MANOPLAS,Estacion::CUERDA},
                "gym_controles",{},"musica_gym",{960,1010}}},
            {"pasillo_mexicali",{"mexicali",{},"",
                {{{1150,925},"pasillo_vendaje"},{{620,925},"pasillo_carlos"}},"publico",{1450,975},true}}};

        const std::map<std::string,ConfigRunner> RUNNERS = {
            // ~20 s a ritmo normal (60 px/s, 80 % en la orilla); el límite deja margen en fácil
            {"playa",{1100.f,45.f,0,0.f,true,5}},
            {"playa_dif2",{1100.f,35.f,0,0.f,true,6}},
            {"circuito",{4500.f,100.f,3,26.f,false,7}},
            {"circuito_dif2",{4500.f,85.f,3,26.f,false,8}}};

        template<typename M>
        const typename M::mapped_type& buscar(const M& m, const std::string& id, const char* tipo)
        {
            auto it = m.find(id);
            if(it != m.end()) return it->second;
            std::cerr<<"[niveles] no hay "<<tipo<<" '"<<id<<"', se usa el primero\n";
            return m.begin()->second;
        }
    }

    const ConfigPelea& pelea(const std::string& id) { return buscar(PELEAS,id,"pelea"); }
    const ConfigRecorrido& recorrido(const std::string& id) { return buscar(RECORRIDOS,id,"recorrido"); }
    const ConfigRunner& runner(const std::string& id) { return buscar(RUNNERS,id,"runner"); }

    bool existeNivel(TipoPaso t, const std::string& id)
    {
        switch(t)
        {
            case TipoPaso::PELEA:     return PELEAS.count(id);
            case TipoPaso::RECORRIDO: return RECORRIDOS.count(id);
            case TipoPaso::RUNNER:    return RUNNERS.count(id);
            default:                  return true;
        }
    }
}
