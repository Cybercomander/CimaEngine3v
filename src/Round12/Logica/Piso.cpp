#include "Piso.hpp"
#include <algorithm>
#include <cmath>
#include <map>
#include <queue>

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

    std::vector<Pt> ruta(const Piso& piso, Pt a, Pt b, float celda)
    {
        using C = std::pair<int,int>;
        auto centro = [&](C c){ return Pt{a.x + c.first*celda, a.y + c.second*celda}; };
        C meta{(int)std::lround((b.x-a.x)/celda),(int)std::lround((b.y-a.y)/celda)};
        std::map<C,C> previo{{{0,0},{0,0}}};
        std::queue<C> q;
        q.push({0,0});
        bool hay = false;
        while(!q.empty() && previo.size() < 20000)
        {
            C c = q.front(); q.pop();
            if(std::abs(c.first-meta.first) <= 1 && std::abs(c.second-meta.second) <= 1) { meta = c; hay = true; break; }
            for(C d : {C{1,0},C{-1,0},C{0,1},C{0,-1}})
            {
                C n{c.first+d.first,c.second+d.second};
                if(previo.count(n) || !piso.dentro(centro(n))) continue;
                previo[n] = c;
                q.push(n);
            }
        }
        if(!hay) return {};
        std::vector<Pt> r{b};
        for(C c = meta; c != C{0,0}; c = previo[c]) r.push_back(centro(c));
        std::reverse(r.begin(),r.end());
        return r;
    }
}
