#pragma once
// Audio de ROUND 12 (WAV en ROUND12_Assets/provisionales/audio). Si un archivo
// falta se avisa una vez y se sigue en silencio: nunca se pide una clave inexistente.
namespace Round12::Audio
{
    void iniciar();                 // se suscribe a los Eventos del combate
    void musica(const char* clave); // en loop; nullptr la detiene
    void efecto(const char* clave);
    void detener();                 // música y efectos (al salir de una escena o de ROUND 12)
    bool sonando();
}
