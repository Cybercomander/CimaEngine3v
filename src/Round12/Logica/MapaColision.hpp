#pragma once
#include "Geometria.hpp"
#include <string>
#include <vector>

namespace Round12
{
    // Adaptador: lee las capas de colisión TXT de ROUND12_Assets
    // ("cols filas" + matriz 0/1/2) y las convierte a rectángulos del mundo,
    // uniendo las celdas seguidas de una fila. Archivo faltante -> vacío.
    namespace MapaColision
    {
        std::vector<Rect> cargar(const std::string& ruta, int valor, int tile = 8);
    }
}
