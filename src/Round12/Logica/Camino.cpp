#include "Camino.hpp"
#include <cmath>

namespace Round12
{
    Camino::Camino(std::vector<Pt> p) : pts(std::move(p))
    {
        for(size_t i = 0; i < pts.size(); ++i)
        {
            acum.push_back(total);
            const Pt& a = pts[i]; const Pt& b = pts[(i+1)%pts.size()];
            total += std::hypot(b.x-a.x,b.y-a.y);
        }
    }

    int Camino::tramo(float& s) const
    {
        s = std::fmod(s,total);
        if(s < 0.f) s += total;
        int i = (int)acum.size()-1;
        while(i > 0 && acum[i] > s) --i;
        s -= acum[i];
        return i;
    }

    Pt Camino::punto(float s) const
    {
        int i = tramo(s);
        Pt t = tangente(acum[i] + 0.0001f);
        return {pts[i].x + t.x*s, pts[i].y + t.y*s};
    }

    Pt Camino::tangente(float s) const
    {
        int i = tramo(s);
        const Pt& a = pts[i]; const Pt& b = pts[(i+1)%pts.size()];
        float l = std::hypot(b.x-a.x,b.y-a.y);
        return {(b.x-a.x)/l,(b.y-a.y)/l};
    }
}
