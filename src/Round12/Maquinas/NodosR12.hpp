#pragma once
#include <Juego/Maquinas/FSM.hpp>
#include <Juego/Maquinas/Boxeadores/NodosBoxeo.hpp>
#include <Round12/Logica/Stats.hpp>

// Estado: nodos de la máquina de estados de cualquier boxeador de ROUND 12.
// Diego y los rivales usan los mismos nodos; a los rivales los maneja un
// CerebroRival que escribe en su IControl (Adaptador).
namespace Round12
{
    class IBoxeador;

    class NodoR12 : public IVJ::FSM
    {
        public:
            NodoR12(const char* n, IVJ::AccionBox a, bool ciclo, std::vector<int> frames = {0,1,2,3,4,5});
            void onEntrar(const IVJ::Entidad& obj) override;
            void onSalir(const IVJ::Entidad& obj) override;
            void onUpdate(const IVJ::Entidad& obj, float dt) override;
        protected:
            virtual void entrar() {}
            virtual void salir() {}
            virtual void paso(float dt) { (void)dt; }
            IVJ::AnimBoxeo anim;
            IBoxeador* box{nullptr};
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

    class CaminarR12 : public NodoR12
    {
        public:
            CaminarR12();
            FSM* onInputs(const CE::IControl& c) override;
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

    // preparación del golpe: es la señal que se puede leer con concentración alta
    class PrepararR12 : public NodoR12
    {
        public:
            PrepararR12(Golpe g, float duracion);
            FSM* onInputs(const CE::IControl& c) override;
        protected:
            void paso(float dt) override;
            void salir() override;
        private:
            Golpe g;
            float duracion;
    };

    class GolpeR12 : public NodoR12
    {
        public:
            explicit GolpeR12(Golpe g);
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
        private:
            IVJ::ColaGolpes cola;
    };

    class AturdidoR12 : public NodoR12
    {
        public:
            AturdidoR12();
            FSM* onInputs(const CE::IControl& c) override;
        protected:
            void entrar() override;
            void salir() override;
    };

    // en la lona; solo la escena lo saca (conteo)
    class CaidoR12 : public NodoR12
    {
        public:
            CaidoR12();
            FSM* onInputs(const CE::IControl&) override { return nullptr; }
        protected:
            void entrar() override;
            void paso(float dt) override;
    };

    class LevantarseR12 : public NodoR12
    {
        public:
            LevantarseR12();
            FSM* onInputs(const CE::IControl& c) override;
        protected:
            void salir() override;
    };

    // duración (s) de cada cuadro según animaciones.json
    float duracionCuadro(Golpe g);
}
