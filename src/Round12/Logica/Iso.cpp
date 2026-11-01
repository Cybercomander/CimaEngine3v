#include "Iso.hpp"
#include <fstream>

namespace Round12
{
    std::optional<MatrizIso> leerIso(const std::string& ruta)
    {
        std::ifstream f{ruta};
        MatrizIso m;
        if(!(f >> m.cols >> m.filas) || m.cols <= 0 || m.filas <= 0) return std::nullopt;
        m.v.resize((size_t)m.cols*m.filas);
        for(auto& x : m.v) if(!(f >> x) || x < -1 || x > 31) return std::nullopt;
        return m;
    }
}
