#include "MapaColision.hpp"
#include <fstream>
#include <iostream>

namespace Round12::MapaColision
{
    std::vector<Rect> cargar(const std::string& ruta, int valor, int tile)
    {
        std::vector<Rect> rects;
        std::ifstream f{ruta};
        int cols = 0, filas = 0;
        if(!(f >> cols >> filas) || cols <= 0 || filas <= 0)
        {
            std::cerr<<"[colisiones] no se pudo leer "<<ruta<<"\n";
            return rects;
        }
        for(int r = 0; r < filas; ++r)
        {
            int inicio = -1;
            for(int c = 0; c <= cols; ++c)
            {
                int v = -1;
                if(c < cols && !(f >> v)) { std::cerr<<"[colisiones] "<<ruta<<" incompleto\n"; return rects; }
                bool dentro = v == valor;
                if(dentro && inicio < 0) inicio = c;
                if(!dentro && inicio >= 0)
                {
                    rects.push_back(Rect{(float)inicio*tile,(float)r*tile,(float)(c-inicio)*tile,(float)tile});
                    inicio = -1;
                }
            }
        }
        return rects;
    }
}
