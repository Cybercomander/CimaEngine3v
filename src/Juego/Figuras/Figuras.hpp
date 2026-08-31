// Guarda de inclusión: indica al compilador que este archivo se incluya una sola vez
// por unidad de traducción, evitando redefiniciones de las clases declaradas aquí
#pragma once
#include "../../Motor/Primitivos/Objetos.hpp"
#include <SFML/Graphics.hpp>

namespace IVJ
{
    // Clase base común a todas las figuras geométricas del laboratorio.
    // Hereda públicamente de CE::Objeto para integrarse al ciclo de vida del motor
    // (entrar al pool de la escena, actualizarse y dibujarse como cualquier otro objeto).
    // No dibuja nada por sí misma: solo concentra los datos que comparten todas las figuras
    class Figuras:public CE::Objeto
    {
    public:
        // Constructor de la clase base. 'explicit' impide conversiones implícitas,
        // de modo que una Figuras solo puede crearse escribiendo sus tres argumentos.
        // Los colores se reciben por referencia constante para no copiarlos al pasarlos
        explicit Figuras(int l, const sf::Color& relleno, const sf::Color& contorno);
        // Destructor virtual (marcado 'override' porque CE::Objeto ya lo declara virtual).
        // Ser virtual garantiza que al destruir una figura a través de un apuntador a la base
        // se ejecute el destructor de la clase derivada real
        ~Figuras() override{};
        // Sobrecarga del operador de inserción en flujo, declarada 'friend' para que,
        // sin ser método de la clase, pueda leer los miembros protegidos de Figuras.
        // Se define dentro de la clase, así que es una función libre que se encuentra por ADL.
        // Recibe el shared_ptr porque en la escena las figuras se manejan como apuntadores inteligentes
        friend std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Figuras>&fig)
        {
            // Escribe en el flujo el número de lados y ambos colores.
            // toInteger() empaqueta el color RGBA en un solo entero sin signo para poder imprimirlo
            os<<"Lados: "<<fig->f_lados<<
            "\nRelleno: "<<fig->f_crelleno.toInteger()<<
            "\nContorno:"<<fig->f_ccontorno.toInteger()<<
            "\n";
            // Devuelve el mismo flujo por referencia para permitir encadenar: os << a << b
            return os;
        }
    // 'protected': visible para las clases derivadas (Rectangulo, Circulo, ...) pero
    // no desde fuera de la jerarquía, de ahí que las figuras concretas puedan usarlos
    protected:
        // Número de lados de la figura; describe la geometría sin depender del objeto SFML
        int f_lados;
        // Color de relleno con el que se pinta el interior de la figura
        sf::Color f_crelleno;
        // Color de contorno con el que se pinta el borde (y también el pivote de depuración)
        sf::Color f_ccontorno;
    };


    // Figura concreta: rectángulo. Hereda de Figuras, por lo que también es un CE::Objeto
    class Rectangulo: public Figuras
    {
        public:
            // Constructor: recibe las dos dimensiones y los colores.
            // 'explicit' evita conversiones implícitas desde un solo argumento
            explicit Rectangulo(float ancho, float largo,const sf::Color& relleno,const  sf::Color& contorno);
            // Destructor que sobrescribe el virtual heredado; vacío porque no hay
            // memoria reservada a mano: la forma de SFML es un miembro por valor
            ~Rectangulo() override {};
            // Implementación de la interfaz sf::Drawable: el motor la invoca al dibujar.
            // Es 'const' porque dibujar no debe modificar el estado de la figura
            void draw(sf::RenderTarget& target, sf::RenderStates state) const override;
            // Implementación del método virtual puro de CE::Objeto: actualiza la figura cada frame
            void onUpdate(float dt) override;
        // 'private': la forma gráfica queda encapsulada, solo esta clase la manipula
        private:
            // Forma rectangular de SFML: es la representación visual concreta del objeto
            sf::RectangleShape rect_img;
        public:
            // Ancho lógico del rectángulo, expuesto para consultarlo desde la escena
            float w;
            // Alto lógico del rectángulo, expuesto para consultarlo desde la escena
            float h;
    };

    // Figura concreta: círculo. Se aproxima con un polígono de muchos puntos
    class Circulo: public Figuras
    {
        public:
            // Constructor: un círculo queda definido por su radio más los dos colores
            explicit Circulo(float radio, const sf::Color& relleno,const  sf::Color& contorno);
            // Destructor que sobrescribe el virtual heredado; nada que liberar
            ~Circulo() override {};
            // Dibujado propio de esta figura, invocado por el sistema de render
            void draw(sf::RenderTarget& target, sf::RenderStates state) const override;
            // Actualización por frame de esta figura
            void onUpdate(float dt) override;
        private:
            // Forma circular de SFML; encapsulada para que solo la clase la modifique
            sf::CircleShape circ_img;
        public:
            // Radio lógico de la figura, disponible para la escena
            float radio;
    };

    // Figura concreta: pentágono. Usa sf::CircleShape limitando sus puntos a 5
    class Pentagono: public Figuras
    {
    public:
        // Constructor: el radio es el de la circunferencia que circunscribe al polígono
        explicit Pentagono(float radio, const sf::Color& relleno,const  sf::Color& contorno);
        // Destructor que sobrescribe el virtual heredado
        ~Pentagono() override {};
        // Dibujado propio de esta figura
        void draw(sf::RenderTarget& target, sf::RenderStates state) const override;
        // Actualización por frame de esta figura
        void onUpdate(float dt) override;
    private:
        // Forma de SFML que representa el pentágono (círculo de 5 puntos)
        sf::CircleShape circ_img;
    public:
        // Radio de la circunferencia circunscrita
        float radio;
    };

    // Figura concreta: hexágono. Misma estrategia, con 6 puntos
    class Hexagono: public Figuras
    {
    public:
        // Constructor con radio circunscrito y colores
        explicit Hexagono(float radio, const sf::Color& relleno,const  sf::Color& contorno);
        // Destructor que sobrescribe el virtual heredado
        ~Hexagono() override {};
        // Dibujado propio de esta figura
        void draw(sf::RenderTarget& target, sf::RenderStates state) const override;
        // Actualización por frame de esta figura
        void onUpdate(float dt) override;
    private:
        // Forma de SFML que representa el hexágono (círculo de 6 puntos)
        sf::CircleShape circ_img;
    public:
        // Radio de la circunferencia circunscrita
        float radio;
    };

    // Figura concreta: octágono. Misma estrategia, con 8 puntos
    class Octagono: public Figuras
    {
    public:
        // Constructor con radio circunscrito y colores
        explicit Octagono(float radio, const sf::Color& relleno,const  sf::Color& contorno);
        // Destructor que sobrescribe el virtual heredado
        ~Octagono() override {};
        // Dibujado propio de esta figura
        void draw(sf::RenderTarget& target, sf::RenderStates state) const override;
        // Actualización por frame de esta figura
        void onUpdate(float dt) override;
    private:
        // Forma de SFML que representa el octágono (círculo de 8 puntos)
        sf::CircleShape circ_img;
    public:
        // Radio de la circunferencia circunscrita
        float radio;
    };

    // Figura concreta: triángulo. Es el polígono mínimo, con 3 puntos
    class Triangulo: public Figuras
    {
    public:
        // Constructor con radio circunscrito y colores
        explicit Triangulo(float radio, const sf::Color& relleno,const  sf::Color& contorno);
        // Destructor que sobrescribe el virtual heredado
        ~Triangulo() override {};
        // Dibujado propio de esta figura
        void draw(sf::RenderTarget& target, sf::RenderStates state) const override;
        // Actualización por frame de esta figura
        void onUpdate(float dt) override;
    private:
        // Forma de SFML que representa el triángulo (círculo de 3 puntos)
        sf::CircleShape circ_img;
    public:
        // Radio de la circunferencia circunscrita
        float radio;
    };

}
