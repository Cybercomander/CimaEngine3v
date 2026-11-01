#pragma once
// Tabla comparativa de estadísticas del GDD (escala 1-10).
// "poder" no viene en la tabla: supuesto según el estilo de cada boxeador.
namespace Round12
{
    enum class Personaje { DIEGO, BRANDON, JAVIER, KEVIN, RICARDO, NINO };

    struct Atributos
    {
        int resistencia, aire, sed, concentracion, velocidad, defensa, poder;
    };

    inline Atributos perfil(Personaje p)
    {
        switch(p)
        {
            //                  res aire sed conc vel def poder
            case Personaje::DIEGO:   return {6,7,6,7,7,5,8};
            case Personaje::BRANDON: return {7,5,6,5,5,6,5};
            case Personaje::JAVIER:  return {7,5,5,4,4,5,9};
            case Personaje::KEVIN:   return {5,8,7,7,9,6,5};
            case Personaje::RICARDO: return {9,9,8,9,8,9,9};
            default:                 return {3,3,3,3,3,3,3}; // niño del barrio
        }
    }

    // hoja de animaciones (ROUND12_Sprites_Animaciones/hojas/<hoja>.png)
    inline const char* hoja(Personaje p)
    {
        switch(p)
        {
            case Personaje::BRANDON: return "brandon_salazar";
            case Personaje::JAVIER:  return "javier_castaneda";
            case Personaje::KEVIN:   return "kevin_ramirez";
            case Personaje::RICARDO: return "ricardo_oropeza";
            default:                 return "diego_ureta"; // el niño usa a Diego con tinte
        }
    }

    inline const char* nombre(Personaje p)
    {
        switch(p)
        {
            case Personaje::DIEGO:   return "Diego Alejandro";
            case Personaje::BRANDON: return "Brandon \"El Güero\" Salazar";
            case Personaje::JAVIER:  return "Javier \"El Toro\" Castañeda";
            case Personaje::KEVIN:   return "Kevin \"The Hawk\" Ramírez";
            case Personaje::RICARDO: return "Ricardo Oropeza";
            default:                 return "Niño del barrio";
        }
    }
}
