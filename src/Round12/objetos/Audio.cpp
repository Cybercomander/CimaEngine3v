#include "Audio.hpp"
#include <Motor/Primitivos/GestorAssets.hpp>
#include <Round12/Logica/Eventos.hpp>
#include <Round12/Rutas.hpp>
#include <filesystem>
#include <iostream>
#include <set>
#include <string>

namespace Round12::Audio
{
    namespace
    {
        std::string actual;
        std::set<std::string> musicas, efectos, faltantes;

        bool existe(const std::string& clave)
        {
            std::string ruta = std::string(R12_PROV) + "/audio/" + clave + ".wav";
            if(std::filesystem::exists(ruta)) return true;
            if(faltantes.insert(clave).second)
                std::cerr<<"[audio] falta "<<ruta<<"\n";
            return false;
        }
        std::string ruta(const std::string& clave) { return std::string(R12_PROV) + "/audio/" + clave + ".wav"; }
    }

    void iniciar()
    {
        static bool hecho = false;
        if(hecho) return;
        hecho = true;
        Eventos::Get().suscribir([](Evento e, float){
            switch(e)
            {
                case Evento::GOLPE_CONECTADO: efecto("golpe_suave"); break;
                case Evento::GOLPE_FUERTE:    efecto("golpe_fuerte"); efecto("publico"); break;
                case Evento::BLOQUEO:         efecto("bloqueo"); break;
                case Evento::CAIDA:           efecto("caida"); break;
                case Evento::CAMPANA:         efecto("campana"); break;
                case Evento::ITEM:            efecto("menu"); break;
                default: break;
            }
        });
    }

    void musica(const char* clave)
    {
        if(!clave) { detener(); return; }
        if(actual == clave && sonando()) return;
        detener();
        if(!existe(clave)) return;
        auto& g = CE::GestorAssets::Get();
        std::string k = std::string("r12g_") + clave;
        if(musicas.insert(clave).second)
            g.agregarMusica(k,ruta(clave));
        auto& m = g.getMusica(k);
        m.setLooping(true);
        m.setVolume(45.f);
        m.play();
        actual = clave;
    }

    void efecto(const char* clave)
    {
        if(!existe(clave)) return;
        auto& g = CE::GestorAssets::Get();
        std::string k = std::string("r12g_") + clave;
        if(efectos.insert(clave).second)
            g.agregarSonido(k,ruta(clave));
        g.getSonido(k).play();
    }

    void detener()
    {
        auto& g = CE::GestorAssets::Get();
        for(auto& m : musicas) g.getMusica("r12g_"+m).stop();
        for(auto& e : efectos) g.getSonido("r12g_"+e).stop();
        actual.clear();
    }

    bool sonando()
    {
        auto& g = CE::GestorAssets::Get();
        for(auto& m : musicas)
            if(g.getMusica("r12g_"+m).getStatus() == sf::SoundSource::Status::Playing) return true;
        return false;
    }
}
