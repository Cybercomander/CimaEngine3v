#pragma once
#include <Juego/Maquinas/FSM.hpp>
#include <Juego/Maquinas/Boxeadores/NodosBoxeo.hpp>
#include <Round12/Logica/Stats.hpp>
#include <Round12/Logica/Animaciones.hpp>

// Estado: nodos de la máquina de estados de cualquier boxeador de ROUND 12.
// Diego y los rivales usan los mismos nodos; a los rivales los maneja un
// CerebroRival que escribe en su IControl (Adaptador). Cada nodo fija la
// postura y el brazo, y elegirAccion() da la animación del paquete v3.
namespace Round12
{
    class IBoxeador;
    class IAnimR12;

    class NodoR12 : public IVJ::FSM
    {
        public:
            NodoR12(const char* n, Postura p, Brazo b);
            NodoR12(const char* n, Accion fija); // caer, levantarse, recibir_golpe
            void onEntrar(const IVJ::Entidad& obj) override;
            void onSalir(const IVJ::Entidad& obj) override;
            void onUpdate(const IVJ::Entidad& obj, float dt) override;
        protected:
            virtual void entrar() {}
            virtual void salir() {}
            virtual void paso(float dt) { (void)dt; }
            void animar(Postura p, Brazo b, bool reiniciar = false);
            void animar(Accion a, bool reiniciar = false);
            bool animTerminada() const;
            Postura postura;
            Brazo brazo;
            int fija{-1};
            IBoxeador* box{nullptr};
            IAnimR12* anim{nullptr};
            CE::ISprite* sprite{nullptr};
            const IVJ::Entidad* ente{nullptr};
            float t{0.f};      // tiempo en el nodo (ya escalado por velocidad/aire)
    };

    class IdleR12 : public NodoR12
    {
        public:
            IdleR12();
            FSM* onInputs(const CE::IControl& c) override;
    };

    // caminar; con Shift camina en guardia (caminar_guardia)
    class CaminarR12 : public NodoR12
    {
        public:
            CaminarR12();
            FSM* onInputs(const CE::IControl& c) override;
        protected:
            void salir() override;
    };

    class GuardiaR12 : public NodoR12
    {
        public:
            GuardiaR12();
            FSM* onInputs(const CE::IControl& c) override;
        protected:
            void entrar() override;
            void salir() override;
    };

    // Q+E juntas: se agacha 0.5 s (los golpes a la cabeza pasan por arriba)
    class AgachadoR12 : public NodoR12
    {
        public:
            AgachadoR12();
            FSM* onInputs(const CE::IControl& c) override;
        protected:
            void entrar() override;
            void salir() override;
    };

    // preparación del golpe: es la señal que se puede leer con concentración alta
    class PrepararR12 : public NodoR12
    {
        public:
            PrepararR12(Golpe g, float duracion, Postura p);
            FSM* onInputs(const CE::IControl& c) override;
        protected:
            void paso(float dt) override;
            void salir() override;
        private:
            Golpe g;
            float duracion;
    };

    // la postura da la variante: caminar_jab, agacharse_gancho, esquiva_izquierda_recto, saltar_uppercut...
    class GolpeR12 : public NodoR12
    {
        public:
            GolpeR12(Golpe g, Postura p = Postura::PIE);
            FSM* onInputs(const CE::IControl& c) override;
        protected:
            void entrar() override;
            void salir() override;
            void paso(float dt) override;
        private:
            Golpe g;
            bool impacto{false};
            IVJ::ColaGolpes cola;
    };

    class EsquivaR12 : public NodoR12
    {
        public:
            explicit EsquivaR12(bool izquierda);
            FSM* onInputs(const CE::IControl& c) override;
        protected:
            void entrar() override;
            void salir() override;
            void paso(float dt) override;
        private:
            IVJ::ColaGolpes cola;
    };

    class AturdidoR12 : public NodoR12
    {
        public:
            AturdidoR12();
            FSM* onInputs(const CE::IControl& c) override;
    };

    // en la lona; solo la escena lo saca (conteo)
    class CaidoR12 : public NodoR12
    {
        public:
            CaidoR12();
            FSM* onInputs(const CE::IControl&) override { return nullptr; }
        protected:
            void entrar() override;
    };

    class LevantarseR12 : public NodoR12
    {
        public:
            LevantarseR12();
            FSM* onInputs(const CE::IControl& c) override;
        protected:
            void salir() override;
    };

    Brazo brazoDe(Golpe g);
    // duración (s) de cada cuadro según animaciones.json
    float duracionCuadro(Golpe g);
}
