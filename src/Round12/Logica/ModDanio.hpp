#pragma once
#include "Stats.hpp"
#include <memory>

namespace Round12
{
    struct ContextoGolpe
    {
        Golpe g;
        const Atributos& atk;
        const Barras& b_atk;
        const Atributos& def;
        bool bloqueado, cierra_combo, contragolpe, frontera;
    };

    // Decorador: cada modificador envuelve al anterior y ajusta su daño
    class ModDanio
    {
        public:
            virtual ~ModDanio() = default;
            virtual float calcular(const ContextoGolpe& c) const = 0;
    };

    class DanioBase : public ModDanio
    {
        public:
            float calcular(const ContextoGolpe& c) const override;
    };

    class Decorador : public ModDanio
    {
        public:
            explicit Decorador(std::unique_ptr<ModDanio> interno) : interno{std::move(interno)} {}
            float calcular(const ContextoGolpe& c) const override { return ajustar(c,interno->calcular(c)); }
        protected:
            virtual float ajustar(const ContextoGolpe& c, float d) const = 0;
        private:
            std::unique_ptr<ModDanio> interno;
    };

    // con poco aire los golpes pegan a la mitad
    class ModPocoAire : public Decorador { using Decorador::Decorador; float ajustar(const ContextoGolpe&, float) const override; };
    class ModBloqueo : public Decorador { using Decorador::Decorador; float ajustar(const ContextoGolpe&, float) const override; };
    class ModCombo : public Decorador { using Decorador::Decorador; float ajustar(const ContextoGolpe&, float) const override; };
    class ModContragolpe : public Decorador { using Decorador::Decorador; float ajustar(const ContextoGolpe&, float) const override; };
    class ModFrontera : public Decorador { using Decorador::Decorador; float ajustar(const ContextoGolpe&, float) const override; };

    // base -> poco aire -> bloqueo -> combo -> contragolpe -> Frontera
    std::unique_ptr<ModDanio> cadenaDanio();
}
