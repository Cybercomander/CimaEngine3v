#pragma once
#include <Juego/objetos/Entidad.hpp>
#include <Round12/Logica/Stats.hpp>

namespace Round12
{
    // alcance de cada golpe (centro a centro, px nativos)
    float alcance(Golpe g);
    // el golpe de atk llega a def: distancia, mismo carril y mirando hacia él
    bool SistemaAlcance(IVJ::Entidad& atk, IVJ::Entidad& def, Golpe g);
    // cada boxeador mira hacia el otro (en el ring)
    void SistemaMirarse(IVJ::Entidad& a, IVJ::Entidad& b);
    // los cuerpos no se encimen en el mismo carril
    void SistemaSepararCuerpos(IVJ::Entidad& a, IVJ::Entidad& b, float minimo = 16.f);
    // camina según el IControl si el nodo actual es "Caminar"; limita al rectángulo
    void SistemaCaminar(IVJ::Entidad& e, float dt, float x0, float x1, float y0, float y1);
}
