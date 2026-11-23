#pragma once
#include <Round12/Logica/Conteo.hpp>
#include <Round12/Logica/Jueces.hpp>
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

namespace Round12
{
    class Escena_Ring;

    // Estado: cada fase de la pelea decide cuándo pasa a la siguiente
    class FasePelea
    {
        public:
            virtual ~FasePelea() = default;
            virtual void entrar(Escena_Ring&) {}
            virtual void actualizar(Escena_Ring& r, float dt) = 0;
            virtual void tecla(Escena_Ring&, const std::string&, bool) {}
            virtual void dibujar(Escena_Ring&, sf::RenderTarget&, sf::Vector2f) {}
        protected:
            float t{0.f};
    };

    class Presentacion : public FasePelea
    {
        public:
            void entrar(Escena_Ring& r) override;
            void actualizar(Escena_Ring& r, float dt) override;
            void dibujar(Escena_Ring& r, sf::RenderTarget& rt, sf::Vector2f tam) override;
    };

    class EnRound : public FasePelea
    {
        public:
            void actualizar(Escena_Ring& r, float dt) override;
    };

    class Caida : public FasePelea
    {
        public:
            explicit Caida(bool es_diego) : es_diego{es_diego}, conteo{0} {}
            void entrar(Escena_Ring& r) override;
            void actualizar(Escena_Ring& r, float dt) override;
            void tecla(Escena_Ring& r, const std::string& a, bool p) override;
            void dibujar(Escena_Ring& r, sf::RenderTarget& rt, sf::Vector2f tam) override;
        private:
            bool es_diego;
            Conteo conteo;
            bool se_levanta{false};
            int levanta_en{7};
    };

    class Esquina : public FasePelea
    {
        public:
            void entrar(Escena_Ring& r) override;
            void actualizar(Escena_Ring& r, float dt) override;
            void tecla(Escena_Ring& r, const std::string& a, bool p) override;
            void dibujar(Escena_Ring& r, sf::RenderTarget& rt, sf::Vector2f tam) override;
        private:
            bool elegido{false};
            std::string eleccion;
    };

    class Decision : public FasePelea
    {
        public:
            void entrar(Escena_Ring& r) override;
            void actualizar(Escena_Ring& r, float dt) override;
            void tecla(Escena_Ring& r, const std::string& a, bool p) override;
            void dibujar(Escena_Ring& r, sf::RenderTarget& rt, sf::Vector2f tam) override;
        private:
            Veredicto v{Veredicto::EMPATE};
            Tarjetas tarjetas;
            bool listo{false};
    };

    // mensaje final (KO / TKO / decisión); J o 4 s para seguir
    class FinPelea : public FasePelea
    {
        public:
            FinPelea(bool gano, bool de_pie, std::string como) : gano{gano}, de_pie{de_pie}, como{std::move(como)} {}
            void actualizar(Escena_Ring& r, float dt) override;
            void tecla(Escena_Ring& r, const std::string& a, bool p) override;
            void dibujar(Escena_Ring& r, sf::RenderTarget& rt, sf::Vector2f tam) override;
        private:
            bool gano, de_pie;
            std::string como;
            bool hecho{false};
    };
}
