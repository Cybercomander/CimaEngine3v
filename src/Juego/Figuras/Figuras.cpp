// Incluye el encabezado de esta misma clase: trae las declaraciones de Figuras y de todas
// sus derivadas, más CE::Objeto y SFML/Graphics.hpp en cadena
#include "Figuras.hpp"

// Espacio de nombres del juego (IVJ), separa el código del juego del código del motor (CE)
namespace IVJ
{
    // Definición del constructor de la clase base Figuras
    Figuras::Figuras(int l, const sf::Color& relleno, const sf::Color& contorno)
        // Lista de inicialización: primero construye la parte heredada CE::Objeto (que crea
        // los componentes nombre, transform y stats), y después inicializa los miembros propios
        // en el orden en que fueron declarados en el .hpp: lados, color de relleno y de contorno
        :CE::Objeto{},f_lados{l},f_crelleno{relleno},f_ccontorno{contorno}
    {
        // Cuerpo vacío: toda la inicialización se resolvió en la lista de arriba,
        // que es más eficiente porque construye los miembros en vez de construirlos y luego asignarlos
    }

    // RECTÁNGULO
    // Definición del constructor de Rectangulo
    Rectangulo::Rectangulo(float ancho, float largo, const sf::Color& relleno, const sf::Color& contorno)
    // Lista de inicialización: llama al constructor de la base Figuras declarando 4 lados y
    // pasándole los colores; crea la forma de SFML con el tamaño {ancho,largo};
    // y guarda copia de las dimensiones en los miembros públicos w y h
    :Figuras{4,relleno,contorno}, rect_img{sf::RectangleShape({ancho,largo})},w{ancho},h{largo}
    {
        // Aplica a la forma el color de relleno que quedó guardado en la base
        rect_img.setFillColor(f_crelleno);
        // Aplica a la forma el color del borde que quedó guardado en la base
        rect_img.setOutlineColor(f_ccontorno);
        // Da 3 píxeles de grosor al borde; sin esto el contorno no sería visible
        rect_img.setOutlineThickness(3);
        // Mueve el origen (punto de anclaje) al centro del rectángulo. Por defecto SFML
        // ancla en la esquina superior izquierda; centrarlo hace que la posición y la
        // rotación se interpreten desde el centro de la figura
        rect_img.setOrigin({ancho/2.f,largo/2.f});
    }
    //RECTÁNGULO
    // Implementación del dibujado. El sistema de render la invoca al procesar la cola de dibujo;
    // es 'const' porque no debe alterar el estado de la figura
    void Rectangulo::draw(sf::RenderTarget& target, sf::RenderStates state) const
    {
        // Combina la transformación acumulada del estado de render con la transformación propia
        // del objeto (heredada de sf::Transformable). Nota: 'state' es una copia local y no se
        // reenvía a target.draw, así que el posicionamiento efectivo lo hace setPosition en onUpdate
        state.transform *= getTransform();
        // Envía la forma rectangular al target (ventana o textura) para que se pinte
        target.draw(rect_img);
        // Crea un pequeño círculo de radio 10 que sirve como marcador visual del pivote,
        // útil para depurar dónde quedó el punto de anclaje de la figura
        sf::CircleShape pivote{10.f};
        // Lo pinta con el color de contorno de la figura para que contraste con el relleno
        pivote.setFillColor(f_ccontorno);
        // Centra el origen del marcador en su propio centro, así queda justo sobre el punto marcado
        pivote.setOrigin({10.f,10.f});
        // Coloca el marcador en la misma posición que el rectángulo, es decir, sobre su pivote
        pivote.setPosition(rect_img.getPosition());
        // Dibuja el marcador encima del rectángulo, ya que se envía después
        target.draw(pivote);
    }
    // Actualización lógica por frame del rectángulo
    void Rectangulo::onUpdate(float dt)
    {
        // Sincroniza la forma gráfica con la posición lógica: lee el componente ITransform
        // heredado de CE::Objeto y la vuelca en la forma de SFML. Así la lógica del juego
        // trabaja con el componente y el render solo refleja su resultado
        rect_img.setPosition({transform->posicion.x,transform->posicion.y});
        // Descarta explícitamente el parámetro para silenciar la advertencia de "parámetro no usado":
        // esta figura aún no aplica movimiento dependiente del tiempo
        (void)dt;
    }

    //CIRCULO
    // Definición del constructor de Circulo
    Circulo::Circulo(float radio, const sf::Color &relleno, const sf::Color& contorno)
    // Lista de inicialización: la base registra 32 lados (el círculo se aproxima con un polígono
    // de 32 puntos), construye la forma de SFML con ese radio y esos 32 puntos, y guarda el radio
    : Figuras{32,relleno,contorno},circ_img{radio,32},radio{radio}
    {
        // Pinta el interior con el color de relleno recibido
        circ_img.setFillColor(relleno);
        // Pinta el borde con el color de contorno recibido
        circ_img.setOutlineColor(contorno);
        // Grosor de 3 píxeles para que el borde sea visible
        circ_img.setOutlineThickness(3);
        //anchor
        // Centra el origen: en un sf::CircleShape el punto {radio,radio} es su centro geométrico,
        // de modo que posición y rotación quedan referidas al centro y no a la esquina del cuadro
        circ_img.setOrigin({radio,radio});
    }
    // Actualización lógica por frame del círculo
    void Circulo::onUpdate(float dt)
    {
        // Copia la posición del componente ITransform hacia la forma gráfica de SFML
        circ_img.setPosition({transform->posicion.x,transform->posicion.y});
        // Marca el delta time como usado; todavía no hay animación que dependa de él
        (void)dt;
    }
    // Implementación del dibujado del círculo
    void Circulo::draw(sf::RenderTarget& target, sf::RenderStates state) const
    {
        // Acumula la transformación propia del objeto sobre la del estado de render recibido
        state.transform *= getTransform();
        // Envía la forma circular al target para que se pinte
        target.draw(circ_img);
        // Marcador de pivote de radio 10 para depuración visual
        sf::CircleShape pivote{10.f};
        // Coloreado con el color de contorno para distinguirlo del relleno
        pivote.setFillColor(f_ccontorno);
        // Origen al centro del marcador, de modo que quede centrado sobre el punto de anclaje
        pivote.setOrigin({10.f,10.f});
        // Se posiciona exactamente donde está el círculo, es decir, sobre su pivote
        pivote.setPosition(circ_img.getPosition());
        // Se dibuja al final para que quede encima de la figura
        target.draw(pivote);
    }

    //PENTAGONO
    // Definición del constructor de Pentagono
    Pentagono::Pentagono(float radio, const sf::Color &relleno, const sf::Color& contorno)
    // La base registra 5 lados y la forma de SFML se construye con 5 puntos sobre la
    // circunferencia de ese radio, lo que produce un pentágono regular
    : Figuras{5,relleno,contorno},circ_img{radio,5},radio{radio}
    {
        // Color interior del polígono
        circ_img.setFillColor(relleno);
        // Color del borde del polígono
        circ_img.setOutlineColor(contorno);
        // Borde de 3 píxeles de grosor
        circ_img.setOutlineThickness(3);
        //anchor
        // Origen al centro de la circunferencia circunscrita, para posicionar y rotar desde el centro
        circ_img.setOrigin({radio,radio});
    }
    // Actualización lógica por frame del pentágono
    void Pentagono::onUpdate(float dt)
    {
        // Vuelca la posición del componente ITransform en la forma gráfica
        circ_img.setPosition({transform->posicion.x,transform->posicion.y});
        // Silencia la advertencia por el parámetro dt aún sin uso
        (void)dt;
    }
    // Implementación del dibujado del pentágono
    void Pentagono::draw(sf::RenderTarget& target, sf::RenderStates state) const
    {
        // Combina la transformación del objeto con la del estado de render
        state.transform *= getTransform();
        // Dibuja el polígono en el target
        target.draw(circ_img);
        // Marcador de pivote de radio 10 para depuración visual
        sf::CircleShape pivote{10.f};
        // Coloreado con el color de contorno de la figura
        pivote.setFillColor(f_ccontorno);
        // Origen centrado en el propio marcador
        pivote.setOrigin({10.f,10.f});
        // Se sitúa sobre la posición (pivote) del pentágono
        pivote.setPosition(circ_img.getPosition());
        // Se dibuja después para quedar por encima
        target.draw(pivote);
    }

    //HEXÁGONO
    // Definición del constructor de Hexagono
    Hexagono::Hexagono(float radio, const sf::Color &relleno, const sf::Color& contorno)
    // La base registra 6 lados y la forma se construye con 6 puntos sobre la circunferencia,
    // generando un hexágono regular
    : Figuras{6,relleno,contorno},circ_img{radio,6},radio{radio}
    {
        // Color interior del polígono
        circ_img.setFillColor(relleno);
        // Color del borde del polígono
        circ_img.setOutlineColor(contorno);
        // Borde de 3 píxeles de grosor
        circ_img.setOutlineThickness(3);
        //anchor
        // Origen al centro para que la posición y la rotación se refieran al centro de la figura
        circ_img.setOrigin({radio,radio});
    }
    // Actualización lógica por frame del hexágono
    void Hexagono::onUpdate(float dt)
    {
        // Sincroniza la forma gráfica con la posición del componente ITransform
        circ_img.setPosition({transform->posicion.x,transform->posicion.y});
        // Marca dt como usado; no hay lógica dependiente del tiempo todavía
        (void)dt;
    }
    // Implementación del dibujado del hexágono
    void Hexagono::draw(sf::RenderTarget& target, sf::RenderStates state) const
    {
        // Acumula la transformación propia del objeto sobre el estado de render
        state.transform *= getTransform();
        // Dibuja el polígono en el target
        target.draw(circ_img);
        // Marcador de pivote de radio 10 para depuración visual
        sf::CircleShape pivote{10.f};
        // Coloreado con el color de contorno de la figura
        pivote.setFillColor(f_ccontorno);
        // Origen centrado en el propio marcador
        pivote.setOrigin({10.f,10.f});
        // Se sitúa sobre la posición (pivote) del hexágono
        pivote.setPosition(circ_img.getPosition());
        // Se dibuja al final para quedar encima de la figura
        target.draw(pivote);
    }

    //Octagono
    // Definición del constructor de Octagono
    Octagono::Octagono(float radio, const sf::Color &relleno, const sf::Color& contorno)
    // La base registra 8 lados y la forma se construye con 8 puntos sobre la circunferencia,
    // generando un octágono regular
    : Figuras{8,relleno,contorno},circ_img{radio,8},radio{radio}
    {
        // Color interior del polígono
        circ_img.setFillColor(relleno);
        // Color del borde del polígono
        circ_img.setOutlineColor(contorno);
        // Borde de 3 píxeles de grosor
        circ_img.setOutlineThickness(3);
        //anchor
        // Origen al centro de la circunferencia circunscrita
        circ_img.setOrigin({radio,radio});
    }
    // Actualización lógica por frame del octágono
    void Octagono::onUpdate(float dt)
    {
        // Vuelca la posición del componente ITransform en la forma gráfica
        circ_img.setPosition({transform->posicion.x,transform->posicion.y});
        // Silencia la advertencia por el parámetro dt aún sin uso
        (void)dt;
    }
    // Implementación del dibujado del octágono
    void Octagono::draw(sf::RenderTarget& target, sf::RenderStates state) const
    {
        // Combina la transformación del objeto con la del estado de render
        state.transform *= getTransform();
        // Dibuja el polígono en el target
        target.draw(circ_img);
        // Marcador de pivote de radio 10 para depuración visual
        sf::CircleShape pivote{10.f};
        // Coloreado con el color de contorno de la figura
        pivote.setFillColor(f_ccontorno);
        // Origen centrado en el propio marcador
        pivote.setOrigin({10.f,10.f});
        // Se sitúa sobre la posición (pivote) del octágono
        pivote.setPosition(circ_img.getPosition());
        // Se dibuja al final para quedar encima de la figura
        target.draw(pivote);
    }

    //Triangulo
    // Definición del constructor de Triangulo
    Triangulo::Triangulo(float radio, const sf::Color &relleno, const sf::Color& contorno)
    // La base registra 3 lados y la forma se construye con 3 puntos sobre la circunferencia:
    // es el polígono mínimo que puede generar sf::CircleShape
    : Figuras{3,relleno,contorno},circ_img{radio,3},radio{radio}
    {
        // Color interior del triángulo
        circ_img.setFillColor(relleno);
        // Color del borde del triángulo
        circ_img.setOutlineColor(contorno);
        // Borde de 3 píxeles de grosor
        circ_img.setOutlineThickness(3);
        //anchor
        // Origen al centro de la circunferencia circunscrita, no al centroide del triángulo
        circ_img.setOrigin({radio,radio});
    }
    // Actualización lógica por frame del triángulo
    void Triangulo::onUpdate(float dt)
    {
        // Sincroniza la forma gráfica con la posición del componente ITransform
        circ_img.setPosition({transform->posicion.x,transform->posicion.y});
        // Marca dt como usado; no hay lógica dependiente del tiempo todavía
        (void)dt;
    }
    // Implementación del dibujado del triángulo
    void Triangulo::draw(sf::RenderTarget& target, sf::RenderStates state) const
    {
        // Acumula la transformación propia del objeto sobre el estado de render
        state.transform *= getTransform();
        // Dibuja el polígono en el target
        target.draw(circ_img);
        // Marcador de pivote de radio 10 para depuración visual
        sf::CircleShape pivote{10.f};
        // Coloreado con el color de contorno de la figura
        pivote.setFillColor(f_ccontorno);
        // Origen centrado en el propio marcador
        pivote.setOrigin({10.f,10.f});
        // Se sitúa sobre la posición (pivote) del triángulo
        pivote.setPosition(circ_img.getPosition());
        // Se dibuja al final para quedar encima de la figura
        target.draw(pivote);
    }

}
