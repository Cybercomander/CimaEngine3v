#include "Runner.hpp"
#include <algorithm>

namespace Round12
{
    namespace { constexpr float GRAVEDAD = 420.f, SALTO = 160.f; }

    Rect obstaculoRunner(TipoChoque t, float x)
    {
        switch(t)
        {
            case TipoChoque::ALTO: return {x,30.f,14.f,40.f};   // agachado mide 26: pasa abajo
            case TipoChoque::ROMPIBLE: return {x,0.f,14.f,16.f};
            default: return {x,0.f,12.f,14.f};
        }
    }

    void Runner::paso(float dt, int ritmo, bool saltar, bool agachar, bool en_sombra)
    {
        float base = ritmo < 0 ? LENTO : (ritmo > 0 ? MAXIMA : RITMO);
        if(cfg.mojada && x >= cfg.orilla_ini && x < cfg.orilla_fin) base *= 0.8f;
        if(lento > 0.f) { lento -= dt; base *= factor_lento; }
        vel = base;
        invulnerable = std::max(0.f, invulnerable - dt);

        // aire: correr a tope cansa; trotar o frenar recupera
        float d_aire = ritmo > 0 ? -6.f : (ritmo < 0 ? 4.f : 1.f);
        if(en_sombra) d_aire += 8.f;
        if(d_aire > 0.f && sed > 70.f) d_aire *= 0.5f;
        aire = std::clamp(aire + d_aire*dt, 0.f, 100.f);
        sed = std::min(100.f, sed + 0.8f*dt);
        tiempo = std::max(0.f, tiempo - dt);

        agachado = agachar && enSuelo();
        if(saltar && enSuelo()) vy = SALTO;
        vy -= GRAVEDAD*dt;
        y = std::max(0.f, y + vy*dt);
        if(y <= 0.f) vy = 0.f;
        x_prev = x;
        x += vel*dt;
    }

    bool Runner::chocaCon(const Rect& o)
    {
        if(invulnerable > 0.f) return false;
        float x0 = std::min(x_prev,x) - ANCHO/2.f, x1 = std::max(x_prev,x) + ANCHO/2.f;
        bool en_x = x1 > o.x && x0 < o.x + o.w;
        bool en_y = y < o.y + o.h && y + alto() > o.y;
        if(!(en_x && en_y)) return false;
        lento = 1.f; factor_lento = 30.f/RITMO;
        vel = 30.f;
        aire = std::max(0.f, aire - 10.f);
        invulnerable = 1.f;
        return true;
    }

    bool Runner::ola()
    {
        if(!enSuelo()) return false;
        lento = 1.5f; factor_lento = 0.5f;
        aire = std::max(0.f, aire - 8.f);
        return true;
    }

    void Runner::reaparecer()
    {
        x = x_prev = cp_x;
        y = vy = 0.f;
        tiempo = std::max(cp_tiempo, 60.f);
        aire = 100.f;
        sed = 0.f;
        lento = invulnerable = 0.f;
    }
}
