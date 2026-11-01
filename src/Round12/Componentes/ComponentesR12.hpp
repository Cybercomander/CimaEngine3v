#pragma once
#include <Motor/Componentes/IComponentes.hpp>
#include <Round12/Logica/Stats.hpp>
#include <memory>
#include <string>

namespace Round12
{
    // estado de combate de un boxeador (Diego o rival)
    class IBoxeador : public CE::IComponentes
    {
        public:
            std::shared_ptr<CE::IComponentes> clonar() const override { return std::make_shared<IBoxeador>(*this); }
            Personaje quien{Personaje::DIEGO};
            Atributos atr{perfil(Personaje::DIEGO)};
            Barras barras{barrasIniciales(perfil(Personaje::DIEGO))};
            bool segundo_aire_usado{false};
            float ventana_contragolpe{0.f}; // > 0: el siguiente golpe es contragolpe
            Golpe ultimo{Golpe::JAB};
            bool golpe_activo{false};       // cuadro de impacto del golpe actual (lo consume la escena)
            bool golpe_lanzado{false};      // el golpe empezó (cobra aire una sola vez)
            bool golpe_fallido{false};      // no alcanzó el aire: animación sin daño
            bool esquivando{false};
            bool guardia{false};
            bool caido{false};
            float destello{0.f};            // señal visual previa al golpe del rival
            sf::Color tinte{sf::Color::White};
    };

    enum class TipoItem { BOTELLA, ISOTONICA, PLATANO, TOALLA };
    class IItem : public CE::IComponentes
    {
        public:
            explicit IItem(TipoItem t) : tipo{t} {}
            std::shared_ptr<CE::IComponentes> clonar() const override { return std::make_shared<IItem>(*this); }
            TipoItem tipo;
            bool tomado{false};
    };

    enum class TipoObstaculo { CASTILLO, HIELERA, SOMBRILLA, PELOTA, PERRO, CANGREJO };
    class IObstaculo : public CE::IComponentes
    {
        public:
            explicit IObstaculo(TipoObstaculo t) : tipo{t} {}
            std::shared_ptr<CE::IComponentes> clonar() const override { return std::make_shared<IObstaculo>(*this); }
            TipoObstaculo tipo;
            float vel{0.f}; // los móviles (perro, cangrejo, pelota) caminan hacia Diego
    };

    // área de activación de una estación del gimnasio
    class IZona : public CE::IComponentes
    {
        public:
            IZona(std::string n, float r) : nombre{std::move(n)}, radio{r} {}
            std::shared_ptr<CE::IComponentes> clonar() const override { return std::make_shared<IZona>(*this); }
            std::string nombre;
            float radio;
            bool hecha{false};
    };
}
