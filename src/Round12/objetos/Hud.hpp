#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Round12
{
    // texto en español (UTF-8) con la fuente del motor
    sf::Text texto(const std::string& utf8, unsigned tam, sf::Color color = sf::Color::White,
            sf::Vector2f pos = {0.f,0.f});
    void centrarX(sf::Text& t, float centro);

    // dibuja en coordenadas de pantalla mientras viva (HUD, menús)
    class EnPantalla
    {
        public:
            EnPantalla();
            ~EnPantalla();
            sf::RenderTexture& rt;
            sf::Vector2f tam;
        private:
            sf::View previa;
    };

    // Composición: el HUD es un árbol de widgets; las posiciones son fracciones de pantalla
    class Widget
    {
        public:
            virtual ~Widget() = default;
            virtual void dibujar(sf::RenderTarget& t, sf::Vector2f tam);
            Widget& agregar(std::unique_ptr<Widget> w) { hijos.push_back(std::move(w)); return *hijos.back(); }
            void limpiar() { hijos.clear(); }
            bool visible{true};
        protected:
            std::vector<std::unique_ptr<Widget>> hijos;
    };

    class Panel : public Widget
    {
        public:
            Panel(sf::FloatRect area, sf::Color fondo) : area{area}, fondo{fondo} {}
            void dibujar(sf::RenderTarget& t, sf::Vector2f tam) override;
        private:
            sf::FloatRect area;
            sf::Color fondo;
    };

    class Barra : public Widget
    {
        public:
            Barra(std::string etiqueta, const float* valor, const float* maximo, sf::Color color,
                    sf::Vector2f pos, float ancho, bool espejo = false)
                :etiqueta{std::move(etiqueta)},valor{valor},maximo{maximo},color{color},pos{pos},ancho{ancho},espejo{espejo} {}
            void dibujar(sf::RenderTarget& t, sf::Vector2f tam) override;
        private:
            std::string etiqueta;
            const float* valor;
            const float* maximo;
            sf::Color color;
            sf::Vector2f pos;
            float ancho;
            bool espejo;
    };

    class Etiqueta : public Widget
    {
        public:
            Etiqueta(std::function<std::string()> fuente, sf::Vector2f pos, float tam_rel,
                    sf::Color color = sf::Color::White, bool centrada = false)
                :fuente{std::move(fuente)},pos{pos},tam_rel{tam_rel},color{color},centrada{centrada} {}
            void dibujar(sf::RenderTarget& t, sf::Vector2f tam) override;
        private:
            std::function<std::string()> fuente;
            sf::Vector2f pos;
            float tam_rel;
            sf::Color color;
            bool centrada;
    };

    // retrato de 96x96 (sprites/personajes/<p>/retratos/<emocion>.png); lado = px de pantalla
    class Retrato : public Widget
    {
        public:
            Retrato(std::function<std::string()> archivo, sf::Vector2f pos, float lado)
                :archivo{std::move(archivo)},pos{pos},lado{lado} {}
            void dibujar(sf::RenderTarget& t, sf::Vector2f tam) override;
        private:
            std::function<std::string()> archivo;
            sf::Vector2f pos;
            float lado;
    };

    // las 4 barras del GDD para un boxeador en la banda superior (el rival a la derecha, en espejo)
    struct Barras;
    void agregarBarras(Widget& raiz, const Barras& b, bool derecha, const std::string& nombre);
    // fondo oscuro de las dos bandas del HUD (arriba y abajo del escenario)
    void agregarBandas(Widget& raiz);

    inline const sf::Color COLOR_RESISTENCIA{220,60,60}, COLOR_AIRE{110,190,255},
        COLOR_SED{255,150,40}, COLOR_CONCENTRACION{250,220,60};
}
