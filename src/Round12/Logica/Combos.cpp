#include "Combos.hpp"

namespace Round12
{
    bool DetectorCombos::termina(std::initializer_list<int> seq) const
    {
        if(cadena.size() < seq.size()) return false;
        auto it = cadena.end() - seq.size();
        for(int v : seq)
            if(*it++ != v) return false;
        return true;
    }

    Combo DetectorCombos::registrar(Golpe g, float t, bool concentracion_maxima)
    {
        if(t - ultimo > VENTANA) cadena.clear();
        ultimo = t;
        cadena.push_back((int)g + 1);
        if(cadena.size() > 4) cadena.erase(cadena.begin());
        if(concentracion_maxima && termina({1,2,3,4}))
        {
            cadena.clear();
            return Combo::FRONTERA;
        }
        if(termina({1,1,2})) return Combo::C112;
        if(termina({1,2,3})) return Combo::C123;
        if(termina({3,4}))   return Combo::C34;
        return Combo::NINGUNO;
    }
}
