#include "LineaDialogo.hpp"
#include <sstream>
#include <vector>

namespace Round12
{
    const char* EMOCIONES[8] = {"cansado","celebrando","decidido","escuchando",
                                "hablando","neutral","sonriendo","sorprendido"};

    int indiceEmocion(const std::string& e)
    {
        for(int i = 0; i < 8; ++i) if(e == EMOCIONES[i]) return i;
        return -1;
    }

    std::optional<LineaDialogo> parsearLinea(const std::string& linea)
    {
        if(linea.empty() || linea[0] == '#') return std::nullopt;
        std::vector<std::string> c;
        std::stringstream ss{linea};
        for(std::string s; std::getline(ss,s,'|');) c.push_back(s);
        if(c.size() < 4 || c.size() > 5) return std::nullopt;
        if(indiceEmocion(c[1]) < 0) return std::nullopt;
        if(c.size() == 5 && c[4] != "der") return std::nullopt;
        return LineaDialogo{c[0],c[1],c[2],c[3],c.size() == 5 ? 1 : 0};
    }

    std::string carpetaPersonaje(const std::string& o)
    {
        if(o == "diego") return "diego_ureta";
        if(o == "carlos") return "carlos_sanchez";
        if(o == "brandon") return "brandon_salazar";
        if(o == "javier") return "javier_castaneda";
        if(o == "kevin") return "kevin_ramirez";
        if(o == "ricardo") return "ricardo_oropeza";
        return "";
    }

    namespace
    {
        std::string expresion(const std::string& e, bool boca)
        {
            if(e == "hablando") return boca ? "hablando_abierto" : "hablando_cerrado";
            if(e == "sonriendo" || e == "celebrando") return "sonriendo";
            if(e == "sorprendido") return "sorprendido";
            if(e == "decidido" || e == "cansado") return "serio";
            return "neutral";
        }
    }

    std::string archivoRostro(const std::string& o, const std::string& e, Vista v, int lado, bool boca)
    {
        if(o != "diego" && o != "carlos") return "";
        std::string vista = v == Vista::FRENTE ? "frente"
            : std::string(v == Vista::PERFIL ? "perfil_" : "trescuartos_") + (lado == 0 ? "der" : "izq");
        return carpetaPersonaje(o) + "_dialogo_" + vista + "_" + expresion(e,boca) + ".png";
    }
}
