#pragma once
#include <Juego/objetos/Entidad.hpp>
#include <Round12/Logica/Stats.hpp>
#include <Round12/Logica/Piso.hpp>

namespace Round12
{
    // px de arte -> px del escenario 1920x1080 (ESCALA_PERSONAJE)
    constexpr float ESCALA_MUNDO = 32.f/9.f;
    // alcance de cada golpe (pies a pies, px del escenario)
    float alcance(Golpe g);
    // el golpe de atk llega a def: distancia, misma profundidad y mirando hacia él
    bool SistemaAlcance(IVJ::Entidad& atk, IVJ::Entidad& def, Golpe g);
    // cada boxeador mira hacia el otro (en el ring, 4 direcciones)
    void SistemaMirarse(IVJ::Entidad& a, IVJ::Entidad& b);
    // los cuerpos no se enciman a la misma profundidad
    void SistemaSepararCuerpos(IVJ::Entidad& a, IVJ::Entidad& b, float minimo = 50.f);
    // camina según el IControl (nodo "Caminar", o "Golpe" en movimiento a la mitad) sin salir del piso
    void SistemaCaminar(IVJ::Entidad& e, float dt, const Piso& piso);
    // versión de rectángulo (corredor, escenas sin polígonos)
    void SistemaCaminar(IVJ::Entidad& e, float dt, float x0, float x1, float y0, float y1);
}
