#pragma once
#include "../../Motor/Componentes/IComponentes.hpp"
#include "../../Motor/Primitivos/Objetos.hpp"
#include "../Maquinas/FSM.hpp"
#include "Motor/Primitivos/CEPool.hpp"
#include "Motor/Utils/Vector2D.hpp"
#include <Juego/Figuras/Figuras.hpp>
namespace IVJ
{
class FSM; // refefencia circular
// Maquina estado componente
class IMaquinaEstado : public CE::IComponentes
{
  public:
    explicit IMaquinaEstado();
    ~IMaquinaEstado() override {};

    std::shared_ptr<IComponentes> clonar() const override
    {
        return std::make_shared<IMaquinaEstado>(*this);
    };

  public:
    std::shared_ptr<FSM> fsm;
    bool congelar{false};
};

// practica de camaras o también para debug
class ITriangulo : public CE::IComponentes
{
  public:
    explicit ITriangulo(float r, float ang);
    ~ITriangulo() override {};
    std::shared_ptr<IComponentes> clonar() const override
    {
        return std::make_shared<ITriangulo>(*this);
    };

  public:
    sf::CircleShape tri_shape;
    float angulo;
};
// componente QUEST con un arreglo de int de 8 bit

class IRayo : public CE::IComponentes
{
  public:
    explicit IRayo(CE::Vector2D &pos, float &sig, const float magnitud = 1.f);
    ~IRayo() override {};
    std::shared_ptr<IComponentes> clonar() const override
    {
        return std::make_shared<IRayo>(*this);
    };

  private:
    float magnitud;
    float &dir;
    CE::Vector2D &lp1;
    CE::Vector2D lp2;

  public:
    CE::Vector2D &getP1() const;
    CE::Vector2D &getP2();
    float getMagnitud() const
    {
        return magnitud;
    }
};

// Datos para que un objeto describa una trayectoria circular.
// Por si solo no hace nada: el SistemaGirar es quien lo lee y mueve al objeto
class IGirar : public CE::IComponentes
{
  public:
    // 'explicit' evita conversiones implícitas al construirlo con un solo valor
    explicit IGirar(const float ang, const float r);

    virtual ~IGirar() override {};
    // Copia el componente en un objeto nuevo en lugar de compartir la referencia,
    // para que dos entidades clonadas no giren con el mismo estado
    std::shared_ptr<IComponentes> clonar() const override
    {
        return std::make_shared<IGirar>(*this);
    };

  public:
    // Ángulo actual en radianes; el sistema lo incrementa cada frame
    float angulo;
    // Radio de la circunferencia que describe el objeto
    float radio;
};

// Datos de un movimiento de arriba hacia abajo.
// Lo ejecuta el SistemaMoverVertical; lo usa un cuadro de la escena Menu
class IMoverVertical : public CE::IComponentes
{
  public:
    // amp = pixeles que sube y baja, vel = radianes por segundo,
    // fase = desfase inicial para que no todos oscilen al mismo tiempo
    explicit IMoverVertical(const float amp, const float vel, const float fase = 0.f);
    virtual ~IMoverVertical() override {};
    // Devuelve una copia independiente del componente, no una referencia compartida
    std::shared_ptr<IComponentes> clonar() const override
    {
        return std::make_shared<IMoverVertical>(*this);
    };

  public:
    // Amplitud del recorrido vertical en pixeles
    float amplitud;
    // Velocidad angular de la oscilacion en radianes por segundo
    float velocidad;
    // Angulo acumulado; avanza cada frame y alimenta al seno
    float fase;
    // Punto de partida del objeto. Se guarda la primera vez que corre el sistema,
    // asi el movimiento siempre se calcula respecto a esa posicion y no se desvia
    CE::Vector2D origen;
    // Indica si 'origen' ya fue capturado
    bool origen_listo{false};
};

// Datos de un avance horizontal con subidas y bajadas, que dibuja una onda (~~~~).
// Lo ejecuta el SistemaMoverOnda; lo usa un cuadro de la escena Menu
class IMoverOnda : public CE::IComponentes
{
  public:
    // amp = altura de las crestas, frec = ondas completas por recorrido,
    // vel = pixeles por segundo, recorrido = distancia antes de regresar al inicio
    explicit IMoverOnda(const float amp, const float frec, const float vel, const float recorrido);
    virtual ~IMoverOnda() override {};
    // Devuelve una copia independiente del componente
    std::shared_ptr<IComponentes> clonar() const override
    {
        return std::make_shared<IMoverOnda>(*this);
    };

  public:
    // Altura de la onda en pixeles
    float amplitud;
    // Numero de ondas completas dentro del recorrido
    float frecuencia;
    // Velocidad de avance horizontal en pixeles por segundo
    float velocidad;
    // Longitud del tramo que recorre antes de reiniciar
    float recorrido;
    // Distancia horizontal avanzada hasta ahora dentro del recorrido
    float avance{0.f};
    // Punto de partida, capturado en la primera ejecucion del sistema
    CE::Vector2D origen;
    // Indica si 'origen' ya fue capturado
    bool origen_listo{false};
};

// Datos de un giro alrededor de un centro fijo, recalculado cada frame desde el centro.
// Lo ejecuta el SistemaMoverCircular; lo usa un cuadro de la escena Menu
class IMoverCircular : public CE::IComponentes
{
  public:
    // radio = distancia al centro en pixeles, vel = radianes por segundo,
    // ang = angulo inicial en radianes
    explicit IMoverCircular(const float radio, const float vel, const float ang = 0.f);
    virtual ~IMoverCircular() override {};
    // Devuelve una copia independiente del componente
    std::shared_ptr<IComponentes> clonar() const override
    {
        return std::make_shared<IMoverCircular>(*this);
    };

  public:
    // Radio de la circunferencia que recorre el objeto
    float radio;
    // Velocidad angular en radianes por segundo
    float velocidad;
    // Angulo actual de la orbita, en radianes
    float angulo;
    // Centro de la orbita, capturado en la primera ejecucion del sistema
    CE::Vector2D centro;
    // Indica si 'centro' ya fue capturado
    bool centro_listo{false};
};

class IInteractuable : public CE::IComponentes
{
  public:
    IInteractuable() = default;
    virtual ~IInteractuable() override {};
    virtual void onInteractuar(CE::Objeto &obj) = 0;

    std::shared_ptr<IComponentes> clonar() const override = 0;

  public:
    bool interactuado{false};
};

class IIndicador : public IInteractuable
{
  public:
    explicit IIndicador(const sf::Texture &ref, float escala, Entidad *p, const sf::Color &c);
    ~IIndicador() override {};
    void onInteractuar(CE::Objeto &obj) override;
    void onRender();
    std::shared_ptr<IComponentes> clonar() const override
    {
        return nullptr;
    };

  public:
    sf::Sprite sprite;
    sf::Shader shader;
    sf::Color color;
    float escala;
    int frame_activo{0};
    bool activo{false};

  private:
    Entidad *parent;
};

class IDialogo : public IInteractuable
{
  public:
    IDialogo();
    ~IDialogo() {};
    void onInteractuar(CE::Objeto &obj) override;
    void onRender();
    std::shared_ptr<IComponentes> clonar() const override
    {
        return nullptr;
    };

  private:
    std::wstring agregarSaltoLinea(const std::wstring &str, size_t max_len);

  public:
    std::wstring texto;
    int id_texto;
    bool activo{false};
};

// Lab 5 Simulación
// Estado interno del ente: indica qué debe hacer en cada frame
class IEstadoInterno : public CE::IComponentes
{
  public:
    enum class Estados
    {
        BUSCAR,
        ENMOVIMIENTOCOMIDA,
        ENMOVIMIENTOCASA,
        CONSUMIR,
        REPRODUCIR // se agrego para reproducir
    };

  public:
    explicit IEstadoInterno(const IEstadoInterno::Estados &estado_inicial);
    virtual ~IEstadoInterno() override {};
    std::shared_ptr<IComponentes> clonar() const override
    {
        return std::make_shared<IEstadoInterno>(*this);
    };

    void setEstadoInterno(const IEstadoInterno::Estados &ne);
    IEstadoInterno::Estados getEstadoInterno() const
    {
        return estado;
    };

  private:
    IEstadoInterno::Estados estado;
};

// Inventario de 1 espacio para cargar la comida de regreso a casa
class IInventarioComida : public CE::IComponentes
{
  public:
    virtual ~IInventarioComida() override {};
    void guardarComida(const std::shared_ptr<Circulo> &c);
    void sacarComida();
    std::weak_ptr<Circulo> getComidaGuardada() const
    {
        return comida;
    };
    std::shared_ptr<IComponentes> clonar() const override
    {
        return std::make_shared<IInventarioComida>(*this);
    };

  private:
    std::weak_ptr<Circulo> comida;
};

// Comida que el ente eligió como objetivo
class ITargetComida : public CE::IComponentes
{
  public:
    virtual ~ITargetComida() override {};
    void setTargetComida(const std::shared_ptr<Circulo> &c);
    void quitarTarget();
    std::weak_ptr<Circulo> getTargetComida() const
    {
        return target;
    };
    std::shared_ptr<IComponentes> clonar() const override
    {
        return std::make_shared<ITargetComida>(*this);
    };

  private:
    std::weak_ptr<Circulo> target;
};

// Punto donde spawneó el ente (su casa)
class IPosicionInicial : public CE::IComponentes
{
  public:
    explicit IPosicionInicial(float x, float y);
    virtual ~IPosicionInicial() override {};
    std::shared_ptr<IComponentes> clonar() const override
    {
        return std::make_shared<IPosicionInicial>(*this);
    };

  public:
    CE::Vector2D pos_init;
};

// Marca si la comida ya fue recogida por algún ente
class ITieneDueño : public CE::IComponentes
{
  public:
    virtual ~ITieneDueño() override {};
    std::shared_ptr<IComponentes> clonar() const override
    {
        return std::make_shared<ITieneDueño>(*this);
    };

  public:
    bool tiene{false};
};

// Comidas consumidas por el ente en la generación actual
class IScore : public CE::IComponentes
{
  public:
    virtual ~IScore() override {};
    std::shared_ptr<IComponentes> clonar() const override
    {
        return std::make_shared<IScore>(*this);
    };

  public:
    int score{0};
};

} // namespace IVJ
