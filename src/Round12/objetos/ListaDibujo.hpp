#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include <vector>

namespace Round12
{
    // Composición: todo lo que se dibuja entre el decorado y el primer plano.
    // Se ordena por la y de los pies: lo de adelante tapa a lo de atrás.
    class NodoDibujo
    {
        public:
            virtual ~NodoDibujo() = default;
            virtual float profundidad() const = 0;
            virtual void dibujar(sf::RenderTarget& t) const = 0;
            bool visible{true};
    };

    // sprite fijo (objeto del paquete, cuerdas frontales): la profundidad no cambia
    class NodoSprite : public NodoDibujo
    {
        public:
            NodoSprite(sf::Sprite s, float prof) : sprite{std::move(s)}, prof{prof} {}
            float profundidad() const override { return prof; }
            void dibujar(sf::RenderTarget& t) const override { if(visible) t.draw(sprite); }
            sf::Sprite sprite;
            float prof;
    };

    // cualquier cosa con posición variable (boxeadores, NPC): se consulta cada cuadro
    class NodoFuncion : public NodoDibujo
    {
        public:
            NodoFuncion(std::function<float()> prof, std::function<void(sf::RenderTarget&)> f)
                :prof{std::move(prof)},f{std::move(f)} {}
            float profundidad() const override { return prof(); }
            void dibujar(sf::RenderTarget& t) const override { if(visible) f(t); }
        private:
            std::function<float()> prof;
            std::function<void(sf::RenderTarget&)> f;
    };

    class ListaDibujo
    {
        public:
            void agregar(std::shared_ptr<NodoDibujo> n) { nodos.push_back(std::move(n)); }
            void limpiar() { nodos.clear(); }
            void dibujar(sf::RenderTarget& t) const;
        private:
            std::vector<std::shared_ptr<NodoDibujo>> nodos;
    };
}
