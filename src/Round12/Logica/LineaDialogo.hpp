#pragma once
#include <optional>
#include <string>

namespace Round12
{
    // las 8 emociones de retratos/ y de dialogo/<dir>/ (orden = fila de la hoja de diálogo)
    extern const char* EMOCIONES[8];
    int indiceEmocion(const std::string& e); // -1 si no existe

    // orador|emocion|fondo|texto[|der]
    struct LineaDialogo { std::string orador, emocion, fondo, texto; int lado{0}; };
    std::optional<LineaDialogo> parsearLinea(const std::string& linea);

    // rostro de 224 px (solo diego y carlos; "" para los demás).
    // FRENTE: monólogo y resultados; TRESCUARTOS: cinemática con oyente; PERFIL: globos en juego.
    // lado 0 = está a la izquierda (mira a la derecha).
    enum class Vista { FRENTE, TRESCUARTOS, PERFIL };
    std::string archivoRostro(const std::string& orador, const std::string& emocion, Vista v, int lado, bool boca_abierta);
    std::string carpetaPersonaje(const std::string& orador); // diego -> diego_ureta
}
