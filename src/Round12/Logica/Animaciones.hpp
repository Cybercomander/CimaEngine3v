#pragma once
#include <cstdint>
#include <vector>

namespace Round12
{
    // Las 39 acciones de sprites/personajes/<p>/<dir>/, en orden alfabético:
    // es el mismo orden de filas de motor/hojas/<p>.png (tools/round12_hojas.py).
    enum class Accion : uint8_t
    {
        AGACHARSE, AGACHARSE_GANCHO, AGACHARSE_GUARDIA, AGACHARSE_JAB, AGACHARSE_RECTO, AGACHARSE_UPPERCUT,
        CAER,
        CAMINAR, CAMINAR_GANCHO, CAMINAR_GUARDIA, CAMINAR_JAB, CAMINAR_RECTO, CAMINAR_UPPERCUT,
        ESQUIVA_DERECHA, ESQUIVA_DERECHA_GANCHO, ESQUIVA_DERECHA_GUARDIA, ESQUIVA_DERECHA_JAB,
        ESQUIVA_DERECHA_RECTO, ESQUIVA_DERECHA_UPPERCUT,
        ESQUIVA_IZQUIERDA, ESQUIVA_IZQUIERDA_GANCHO, ESQUIVA_IZQUIERDA_GUARDIA, ESQUIVA_IZQUIERDA_JAB,
        ESQUIVA_IZQUIERDA_RECTO, ESQUIVA_IZQUIERDA_UPPERCUT,
        GANCHO, GUARDIA, IDLE, JAB, LEVANTARSE, RECIBIR_GOLPE, RECTO,
        SALTAR, SALTAR_GANCHO, SALTAR_GUARDIA, SALTAR_JAB, SALTAR_RECTO, SALTAR_UPPERCUT,
        UPPERCUT
    };
    constexpr int N_ACCIONES = 39;
    constexpr int CUADROS = 6;
    constexpr int TAM_CUADRO = 80;
    // direcciones = CE::IDireccion
    enum Dir { ADELANTE = 0, ATRAS = 1, DERECHA = 2, IZQUIERDA = 3 };

    enum class Postura { PIE, CAMINANDO, AGACHADO, SALTANDO, ESQUIVA_IZQ, ESQUIVA_DER };
    enum class Brazo { NADA, GUARDIA, JAB, RECTO, GANCHO, UPPERCUT };

    const char* nombre(Accion a);
    float msPorCuadro(Accion a);
    bool enCiclo(Accion a);
    // postura del cuerpo + lo que hacen los brazos = la animación del paquete
    Accion elegirAccion(Postura p, Brazo b);

    struct RectCuadro
    {
        int x, y, w, h;
        bool operator==(const RectCuadro& o) const { return x == o.x && y == o.y && w == o.w && h == o.h; }
    };
    RectCuadro rectCuadro(Accion a, int dir, int cuadro);
    // hoja de diálogo: 8 emociones x 4 direcciones x 4 cuadros
    RectCuadro rectDialogo(int emocion, int dir, int cuadro);

    // 4 direcciones según hacia dónde está el objetivo; -1 = sin movimiento
    int direccionHacia(float dx, float dy);
    // índices ordenados de atrás hacia adelante (y de los pies); estable en empates
    std::vector<int> ordenPorProfundidad(const std::vector<float>& profundidades);
}
