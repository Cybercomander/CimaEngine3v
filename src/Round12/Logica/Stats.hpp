#pragma once
#include <Round12/Datos/Perfiles.hpp>

// Reglas numéricas de las barras del GDD. Todos los valores son de balance
// (supuestos) y viven aquí para ajustarlos en un solo lugar.
namespace Round12
{
    enum class Golpe { JAB = 0, RECTO, GANCHO, UPPERCUT };

    struct Barras
    {
        float resistencia{0}, resistencia_max{0};
        float aire{100}, sed{0}, concentracion{0};
    };

    constexpr float AIRE_MAX = 100.f;
    constexpr float BARRA_MAX = 100.f;

    Barras barrasIniciales(const Atributos& a);
    float costoAire(Golpe g, const Atributos& a);
    float costoEsquiva(const Atributos& a);
    // aire recuperado por segundo
    float regenAire(const Barras& b, const Atributos& a, bool guardia, bool moviendo);
    // multiplicador de lo que sube la sed (más tolerancia = sube menos)
    float factorSed(const Atributos& a);
    float sedPorSegundo(const Atributos& a);
    float sedPorGolpe(const Atributos& a);
    // >1 = animaciones más lentas
    float escalaAnimacion(const Barras& b, const Atributos& a);
    float velocidadPasos(const Atributos& a);
    // el atributo sube o baja sin salir de 1-10
    int limitar(int v);
}
