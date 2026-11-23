#include "Combate.hpp"
#include "Eventos.hpp"
#include "ModDanio.hpp"
#include <algorithm>

namespace Round12
{
    Combate::Combate(Atributos diego, Atributos rival, Objetivo o, bool segundo)
        :a_diego{diego},a_rival{rival},b_diego{barrasIniciales(diego)},b_rival{barrasIniciales(rival)},
        objetivo{o},segundo_aire{segundo},rounds_(1)
    {
    }

    bool Combate::lanzar(bool de_diego, Golpe g)
    {
        auto& b = barras(de_diego);
        const auto& a = atributos(de_diego);
        float costo = costoAire(g,a);
        if(b.aire < costo) return false;
        b.aire -= costo;
        b.sed = std::min(BARRA_MAX, b.sed + sedPorGolpe(a));
        return true;
    }

    bool Combate::esquivar(bool de_diego)
    {
        auto& b = barras(de_diego);
        float costo = costoEsquiva(atributos(de_diego));
        if(b.aire < costo) return false;
        b.aire -= costo;
        return true;
    }

    float Combate::recibir(bool a_diego, Golpe g, bool bloqueado, bool cierra_combo, bool contragolpe, bool frontera,
                           bool agachado)
    {
        if(agachado && g != Golpe::UPPERCUT) return 0.f;
        static const auto cadena = cadenaDanio();
        bool atk_diego = !a_diego;
        ContextoGolpe ctx{g,atributos(atk_diego),barras(atk_diego),atributos(a_diego),
            bloqueado,cierra_combo,contragolpe,frontera};
        float d = cadena->calcular(ctx) * (agachado ? 1.5f : 1.f);
        auto& b = barras(a_diego);
        // en la semifinal contra Ricardo no hay nocaut de nadie (GDD, capítulo 4)
        float minimo = objetivo == Objetivo::PERDER_FORZADO ? 1.f : 0.f;
        b.resistencia = std::max(minimo, b.resistencia - d);
        bool cabeza = !bloqueado && (g == Golpe::GANCHO || g == Golpe::UPPERCUT);
        if(cabeza) b.concentracion -= 14.f - atributos(a_diego).concentracion;
        if(cierra_combo) b.concentracion -= 10.f;
        b.concentracion = std::max(0.f, b.concentracion);
        auto& r = round();
        if(atk_diego) { r.conectados_diego++; r.danio_diego += d; }
        else          { r.conectados_rival++; r.danio_rival += d; }
        if(bloqueado) Eventos::Get().emitir(Evento::BLOQUEO,d);
        else Eventos::Get().emitir(cabeza || cierra_combo || d >= 8.f ? Evento::GOLPE_FUERTE : Evento::GOLPE_CONECTADO,d);
        return d;
    }

    void Combate::tick(float dt, EstadoFisico diego, EstadoFisico rival)
    {
        for(bool es_diego : {true,false})
        {
            auto& b = barras(es_diego);
            const auto& a = atributos(es_diego);
            auto e = es_diego ? diego : rival;
            b.aire = std::min(AIRE_MAX, b.aire + regenAire(b,a,e.guardia,e.moviendo)*dt);
            b.sed = std::min(BARRA_MAX, b.sed + sedPorSegundo(a)*dt);
            if(b.aire < 20.f) b.concentracion -= 2.f*dt;
            if(e.guardia) b.concentracion += 2.f*dt;
            b.concentracion = std::clamp(b.concentracion, 0.f, BARRA_MAX);
        }
        // Segundo aire: una vez por pelea, cuando el aire está casi vacío
        if(segundo_aire && !segundo_usado && b_diego.aire < 10.f)
        {
            segundo_usado = true;
            b_diego.aire += 50.f;
        }
    }

    void Combate::caida(bool diego)
    {
        (diego ? round().caidas_diego : round().caidas_rival)++;
        Eventos::Get().emitir(Evento::CAIDA);
    }

    void Combate::descanso()
    {
        for(bool es_diego : {true,false})
        {
            auto& b = barras(es_diego);
            b.aire = AIRE_MAX;
            b.resistencia = std::min(b.resistencia_max, b.resistencia + b.resistencia_max*0.12f);
        }
        b_rival.sed = std::max(0.f, b_rival.sed - 30.f); // el rival también toma agua
    }

    void Combate::nuevoRound() { rounds_.emplace_back(); }

    Veredicto Combate::veredicto(std::mt19937& rng, Tarjetas* t)
    {
        auto v = decidir(rounds_,rng,t);
        if(objetivo == Objetivo::PERDER_FORZADO && t)
            for(int j = 0; j < 3; ++j)
                if(t->diego[j] >= t->rival[j]) std::swap(t->diego[j],t->rival[j]), t->rival[j] += t->rival[j] == t->diego[j];
        return objetivo == Objetivo::PERDER_FORZADO ? Veredicto::RIVAL_UNANIME : v;
    }
}
