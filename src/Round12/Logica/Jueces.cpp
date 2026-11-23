#include "Jueces.hpp"

namespace Round12
{
    Veredicto decidir(const std::vector<RoundStats>& rounds, std::mt19937& rng, Tarjetas* tarjetas)
    {
        std::uniform_real_distribution<float> ruido(-1.f,1.f);
        int por_diego = 0, por_rival = 0;
        Tarjetas t;
        for(int j = 0; j < 3; ++j)
        {
            for(const auto& r : rounds)
            {
                float pd = r.conectados_diego + r.danio_diego/10.f + 3.f*r.caidas_rival + ruido(rng);
                float pr = r.conectados_rival + r.danio_rival/10.f + 3.f*r.caidas_diego + ruido(rng);
                int d = 10, v = 10;
                if(pd > pr) v = r.caidas_rival ? 8 : 9;
                else if(pr > pd) d = r.caidas_diego ? 8 : 9;
                t.diego[j] += d;
                t.rival[j] += v;
            }
            if(t.diego[j] > t.rival[j]) por_diego++;
            else if(t.rival[j] > t.diego[j]) por_rival++;
        }
        if(tarjetas) *tarjetas = t;
        if(por_diego == 3) return Veredicto::DIEGO_UNANIME;
        if(por_rival == 3) return Veredicto::RIVAL_UNANIME;
        if(por_diego >= 2) return Veredicto::DIEGO_DIVIDIDA;
        if(por_rival >= 2) return Veredicto::RIVAL_DIVIDIDA;
        return Veredicto::EMPATE;
    }

    bool ganaDiego(Veredicto v)
    {
        return v == Veredicto::DIEGO_UNANIME || v == Veredicto::DIEGO_DIVIDIDA;
    }

    const char* texto(Veredicto v)
    {
        switch(v)
        {
            case Veredicto::DIEGO_UNANIME:  return "Diego gana por decisión unánime";
            case Veredicto::DIEGO_DIVIDIDA: return "Diego gana por decisión dividida";
            case Veredicto::RIVAL_UNANIME:  return "El rival gana por decisión unánime";
            case Veredicto::RIVAL_DIVIDIDA: return "El rival gana por decisión dividida";
            default:                        return "Empate";
        }
    }
}
