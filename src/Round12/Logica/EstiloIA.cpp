#include "EstiloIA.hpp"

namespace Round12
{
    EstiloIA estilo(Personaje p)
    {
        switch(p)
        {
            // Brandon: guardia alta y jab constante (rival tutorial)
            case Personaje::BRANDON: return {0.6f,0.1f,32.f,0.4f,{0.7f,0.2f,0.05f,0.05f}};
            // Javier: fajador que avanza y lanza ganchos potentes
            case Personaje::JAVIER:  return {0.1f,0.05f,20.f,0.9f,{0.15f,0.15f,0.5f,0.2f}};
            // Kevin: estilista, golpea y se aleja
            case Personaje::KEVIN:   return {0.2f,0.35f,36.f,0.5f,{0.4f,0.4f,0.1f,0.1f}};
            // Ricardo: completo, contragolpea y controla la distancia con el jab
            case Personaje::RICARDO: return {0.35f,0.4f,28.f,0.7f,{0.4f,0.25f,0.2f,0.15f}};
            default:                 return {0.2f,0.05f,26.f,0.4f,{0.4f,0.3f,0.2f,0.1f}};
        }
    }

    Golpe elegirGolpe(const std::array<float,4>& pesos, float u)
    {
        float total = 0.f;
        for(float w : pesos) total += w;
        float acum = 0.f;
        for(int i = 0; i < 4; ++i)
        {
            acum += pesos[i]/total;
            if(u < acum) return (Golpe)i;
        }
        return Golpe::UPPERCUT;
    }
}
