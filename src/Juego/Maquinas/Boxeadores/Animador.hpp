#pragma once
#include <Motor/Componentes/IComponentes.hpp>
#include <vector>

namespace IVJ
{
    class Entidad;
    // Composición: cada nodo de boxeador TIENE un Animador en lugar de repetir
    // el control de frames/frame rate. Sirve para cualquier hoja de una sola fila
    // (todas las de ROUND12: Diego 44x52, rivales 44x56).
    class Animador
    {
        public:
            // frames: índices de la hoja (0 frente_firme, 1 frente_guardia,
            //         2 lateral_firme, 3 lateral_jab)
            // ciclo: true repite la animación, false se detiene en el último frame
            // rebote: pixeles que sube el sprite en los frames impares (simula pasos)
            // fila: renglón de la hoja donde está la animación (hojas con varias animaciones)
            Animador(std::vector<int> frames, float frame_rate, bool ciclo = true, float rebote = 0.f, int fila = 0);
            void iniciar(const Entidad& obj);
            void actualizar(float dt);
            bool terminado() const { return fin; }
            // cambia de renglón sin reiniciar el ciclo (p. ej. voltear mientras camina)
            void setFila(int f);
            // las poses laterales miran a la derecha; a la izquierda se refleja el sprite
            void mirar(bool izquierda);
        private:
            void pintarFrame();
        private:
            CE::ISprite* sprite{nullptr};
            std::vector<int> frames;
            float max_tiempo;
            float act_tiempo;
            bool ciclo;
            float rebote;
            int fila;
            int id_frame{0};
            bool fin{false};
    };
}
