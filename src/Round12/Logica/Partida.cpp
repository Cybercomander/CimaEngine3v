#include "Partida.hpp"
#include <random>

namespace Round12
{
    void Partida::nueva(std::vector<Paso> pasos, bool es_demo)
    {
        atr = antes = perfil(Personaje::DIEGO);
        habilidades = 0;
        nuevas.clear();
        resumen.clear();
        hist = Historia{std::move(pasos)};
        demo = es_demo;
        sem = std::random_device{}();
    }

    void Partida::desbloquear(Habilidad h)
    {
        if(tiene(h)) return;
        habilidades |= (std::uint8_t)h;
        nuevas.push_back(h);
    }

    void Partida::mejorar(int Atributos::*campo, int delta)
    {
        atr.*campo = limitar(atr.*campo + delta);
    }

    bool Partida::alTerminarPelea(Objetivo o, bool gano, bool de_pie)
    {
        bool avanza = o == Objetivo::PERDER_FORZADO || (o == Objetivo::GANAR ? gano : de_pie);
        return avanza && hist.avanzar();
    }

    void Partida::alTerminarNivel()
    {
        antes = atr;
        nuevas.clear();
        resumen.clear();
    }

    const char* nombre(Habilidad h)
    {
        switch(h)
        {
            case Habilidad::COMBOS:       return "Combinaciones 1-1-2, 1-2-3 y 3-4";
            case Habilidad::ESQUIVA:      return "Esquiva lateral (Q / E)";
            case Habilidad::CONTRAGOLPE:  return "Contragolpe (doble daño tras esquivar)";
            case Habilidad::SEGUNDO_AIRE: return "Segundo aire";
            default:                      return "Combinación especial «Frontera» (1-2-3-4)";
        }
    }
}
