#include "Animaciones.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <string>

namespace Round12
{
    namespace
    {
        const char* NOMBRES[N_ACCIONES] = {
            "agacharse","agacharse_gancho","agacharse_guardia","agacharse_jab","agacharse_recto","agacharse_uppercut",
            "caer",
            "caminar","caminar_gancho","caminar_guardia","caminar_jab","caminar_recto","caminar_uppercut",
            "esquiva_derecha","esquiva_derecha_gancho","esquiva_derecha_guardia","esquiva_derecha_jab",
            "esquiva_derecha_recto","esquiva_derecha_uppercut",
            "esquiva_izquierda","esquiva_izquierda_gancho","esquiva_izquierda_guardia","esquiva_izquierda_jab",
            "esquiva_izquierda_recto","esquiva_izquierda_uppercut",
            "gancho","guardia","idle","jab","levantarse","recibir_golpe","recto",
            "saltar","saltar_gancho","saltar_guardia","saltar_jab","saltar_recto","saltar_uppercut",
            "uppercut"};

        Accion buscar(const std::string& n)
        {
            for(int i = 0; i < N_ACCIONES; ++i) if(n == NOMBRES[i]) return (Accion)i;
            return Accion::IDLE;
        }

        bool termina(const std::string& s, const char* fin)
        {
            std::string f{fin};
            return s.size() >= f.size() && s.compare(s.size()-f.size(),f.size(),f) == 0;
        }
    }

    const char* nombre(Accion a) { return NOMBRES[(int)a]; }

    float msPorCuadro(Accion a)
    {
        std::string n = nombre(a);
        // animaciones.json; el golpe manda en las variantes compuestas
        if(termina(n,"jab")) return 70.f;
        if(termina(n,"recto")) return 75.f;
        if(termina(n,"gancho")) return 80.f;
        if(termina(n,"uppercut")) return 85.f;
        if(termina(n,"guardia")) return 120.f;
        if(n == "caminar") return 110.f;
        if(n == "esquiva_derecha" || n == "esquiva_izquierda") return 90.f;
        // supuestos (no vienen en animaciones.json)
        if(n == "saltar" || n == "agacharse") return 90.f;
        if(n == "caer" || n == "levantarse") return 100.f;
        if(n == "recibir_golpe") return 70.f;
        return 120.f; // idle
    }

    bool enCiclo(Accion a)
    {
        std::string n = nombre(a);
        return n == "idle" || termina(n,"guardia") || n == "caminar";
    }

    Accion elegirAccion(Postura p, Brazo b)
    {
        static const char* BRAZO[] = {"","guardia","jab","recto","gancho","uppercut"};
        static const char* POSTURA[] = {"","caminar","agacharse","saltar","esquiva_izquierda","esquiva_derecha"};
        std::string pre = POSTURA[(int)p], br = BRAZO[(int)b];
        if(pre.empty()) return br.empty() ? Accion::IDLE : buscar(br);
        return buscar(br.empty() ? pre : pre + "_" + br);
    }

    RectCuadro rectCuadro(Accion a, int dir, int cuadro)
    {
        return {(dir*CUADROS + cuadro)*TAM_CUADRO, (int)a*TAM_CUADRO, TAM_CUADRO, TAM_CUADRO};
    }

    RectCuadro rectDialogo(int emocion, int dir, int cuadro)
    {
        return {(dir*4 + cuadro)*TAM_CUADRO, emocion*TAM_CUADRO, TAM_CUADRO, TAM_CUADRO};
    }

    int direccionHacia(float dx, float dy)
    {
        if(dx == 0.f && dy == 0.f) return -1;
        if(std::fabs(dx) >= std::fabs(dy)) return dx >= 0.f ? DERECHA : IZQUIERDA;
        return dy > 0.f ? ADELANTE : ATRAS;
    }

    std::vector<int> ordenPorProfundidad(const std::vector<float>& p)
    {
        std::vector<int> idx(p.size());
        std::iota(idx.begin(),idx.end(),0);
        std::stable_sort(idx.begin(),idx.end(),[&](int a, int b){ return p[a] < p[b]; });
        return idx;
    }
}
