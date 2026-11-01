#include "Piso.hpp"

namespace Round12
{
    bool dentroPoligono(const std::vector<Pt>& poli, Pt p)
    {
        bool dentro = false;
        for(size_t i = 0, j = poli.size()-1; i < poli.size(); j = i++)
        {
            const Pt& a = poli[i]; const Pt& b = poli[j];
            if((a.y > p.y) != (b.y > p.y) && p.x < (b.x-a.x)*(p.y-a.y)/(b.y-a.y) + a.x)
                dentro = !dentro;
        }
        return dentro;
    }

    bool Piso::dentro(Pt p) const
    {
        for(const auto& r : bloqueos)
            if(p.x >= r.x && p.x < r.x+r.w && p.y >= r.y && p.y < r.y+r.h) return false;
        for(const auto& poli : poligonos) if(dentroPoligono(poli,p)) return true;
        return false;
    }

    Pt Piso::limitar(Pt antes, Pt despues) const
    {
        if(dentro(despues)) return despues;
        if(dentro({despues.x,antes.y})) return {despues.x,antes.y};
        if(dentro({antes.x,despues.y})) return {antes.x,despues.y};
        return antes;
    }
}
