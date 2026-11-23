#pragma once
#include "Geometria.hpp"
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace Round12
{
    // Diorama de tiles isométricos (tilesets/atlas_isometrico_64x48.png, 8x4 tiles de 64x48,
    // huella 64x32). Archivo: "cols filas" y la matriz de índices (-1 = vacío).
    struct MatrizIso
    {
        int cols{0}, filas{0};
        std::vector<int> v;
        int en(int c, int f) const { return v[f*cols + c]; }
        std::set<int> usados() const { std::set<int> s(v.begin(),v.end()); s.erase(-1); return s; }
    };
    std::optional<MatrizIso> leerIso(const std::string& ruta);
    // esquina superior del rombo de la celda (c,f)
    inline Pt celdaIso(int c, int f, Pt origen, float esc) { return {origen.x + (c-f)*32.f*esc, origen.y + (c+f)*16.f*esc}; }
}
