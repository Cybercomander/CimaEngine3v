#pragma once
#include <Round12/Logica/Geometria.hpp>
#include <string>
#include <vector>

// Escenarios del paquete v3 en coordenadas de la imagen (1920x1080).
// Posiciones medidas sobre escenarios/<carpeta>/decorado_1920x1080.png.
namespace Round12
{
    struct PropR12 { std::string objeto; Pt pie; bool solido{true}; };
    struct EstacionR12 { std::string tipo; Pt pie; float radio{110.f}; };

    struct EscenarioR12
    {
        std::string carpeta;          // ROUND12_Assets/escenarios/<carpeta>/
        bool ring{false};             // tiene cuerdas frontales (primer_plano)
        std::vector<std::vector<Pt>> piso;   // por donde se camina fuera de pelea
        std::vector<Rect> bloqueos;          // muebles pintados
        std::vector<Pt> ring_poligono;       // lona por donde pisan los boxeadores
        float ring_frente_y{0.f};            // profundidad de las cuerdas frontales
        std::vector<PropR12> props;
        std::vector<EstacionR12> estaciones;
        Pt entrada{960,1000}, esquina_diego{0,0}, esquina_rival{0,0}, escalera_izq{0,0}, escalera_der{0,0};
    };

    constexpr float ANCHO_MUNDO = 1920.f, ALTO_MUNDO = 1080.f;
    // gym, visitante, mexicali, santa_fe, circuito
    const EscenarioR12& escenario(const std::string& id);
    std::string rutaCapa(const EscenarioR12& e, const char* capa); // "decorado" / "primer_plano"
}
