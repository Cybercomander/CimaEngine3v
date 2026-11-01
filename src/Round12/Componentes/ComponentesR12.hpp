#pragma once
#include <Motor/Componentes/IComponentes.hpp>
#include <Round12/Logica/Stats.hpp>
#include <Round12/Logica/Animaciones.hpp>
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
            bool jugador{false};            // Diego: sin preparación antes de golpear
            bool puede_esquivar{true};      // Diego la aprende en el sparring (GDD)
            bool mira_fijo{false};          // en el ring la escena decide hacia dónde mira
            bool agachado{false};           // jab, recto y gancho le pasan por arriba
            bool pide_agacharse{false};     // Q+E juntas (o la IA): el nodo actual se agacha
            bool en_aire{false};            // corredor / Santa Fe: los golpes salen como saltar_*
            sf::Color tinte{sf::Color::White};
    };

    // animación del paquete v3 (39 acciones x 4 direcciones x 6 cuadros)
    class IAnimR12 : public CE::IComponentes
    {
        public:
            std::shared_ptr<CE::IComponentes> clonar() const override { return std::make_shared<IAnimR12>(*this); }
            void poner(Accion a, bool reiniciar = false)
            {
                if(a == accion && !reiniciar) return;
                accion = a; cuadro = 0; t = 0.f; termino = false;
            }
            // escala_tiempo > 1 = más lento (cansancio)
            void actualizar(float dt, float escala_tiempo = 1.f)
            {
                if(termino) return;
                t += dt*1000.f/escala_tiempo;
                float ms = msPorCuadro(accion);
                while(t >= ms && !termino)
                {
                    t -= ms;
                    if(++cuadro < CUADROS) continue;
                    if(enCiclo(accion)) cuadro = 0;
                    else { cuadro = CUADROS-1; termino = true; }
                }
            }
            Accion accion{Accion::IDLE};
            int dir{DERECHA};
            int cuadro{0};
            float t{0.f};
            bool termino{false};
            bool dialogo{false};  // usa la hoja de diálogo (emocion = fila)
            int emocion{5};       // neutral
            std::string hoja;     // carpeta del personaje (motor/hojas/<hoja>.png)
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
