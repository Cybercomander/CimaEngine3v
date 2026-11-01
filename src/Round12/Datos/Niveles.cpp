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
            // la final llega hasta el round 12 (supuesto por el título del juego)
            {"final_ricardo",{Personaje::RICARDO,12,45.f,Objetivo::GANAR,"mexicali",true,2}}};

        const std::map<std::string,ConfigRecorrido> RECORRIDOS = {
            {"gym_tutorial",{"01_sanchez_boxing_gym",{Estacion::COSTAL,Estacion::MANOPLAS,Estacion::CUERDA,Estacion::AGUA},
                560.f,"gym_controles",{},"musica_gym",true}},
            {"pasillo_mexicali",{"03_gimnasio_mexicali",{},760.f,"",
                {{120.f,"pasillo_vendaje"},{420.f,"pasillo_carlos"}},"publico"}}};

        const std::map<std::string,ConfigRunner> RUNNERS = {
            {"playa",{"05_playa_rosarito",9000.f,180.f,0,0.f,true,5}},
            {"playa_dif2",{"05_playa_rosarito",9000.f,150.f,0,0.f,true,6}},
            {"circuito",{"06_circuito_rosarito",4500.f,100.f,3,26.f,false,7}},
            {"circuito_dif2",{"06_circuito_rosarito",4500.f,85.f,3,26.f,false,8}}};

        const std::map<std::string,ConfigSantaFe> SANTA_FE = {
            {"santa_fe",{120.f}},{"santa_fe_dif2",{100.f}}};

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
    const ConfigSantaFe& santaFe(const std::string& id) { return buscar(SANTA_FE,id,"santa fe"); }

    bool existeNivel(TipoPaso t, const std::string& id)
    {
        switch(t)
        {
            case TipoPaso::PELEA:     return PELEAS.count(id);
            case TipoPaso::RECORRIDO: return RECORRIDOS.count(id);
            case TipoPaso::RUNNER:    return RUNNERS.count(id);
            case TipoPaso::SANTA_FE:  return SANTA_FE.count(id);
            default:                  return true;
        }
    }
}
