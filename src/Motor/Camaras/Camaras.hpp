#pragma once
#include <SFML/Graphics.hpp>
#include "../Primitivos/Objetos.hpp"
#include "../Componentes/IComponentes.hpp"
#include "../Utils/Vector2D.hpp"

namespace CE
{
    /**
     * @class Camara
     * @brief Clase base abstracta para cámaras de juego que gestiona la vista.
     * 
     * Implementa una cámara que puede seguir objetos y está vinculada a un view de SFML.
     * Proporciona transformación (posición, rotación) y métodos de actualización virtual
     * para permitir diferentes estrategias de movimiento (fija, seguimiento, LERP, etc).
     * 
     * @note Utiliza weak_ptr para evitar referencias circulares con el objeto seguido
     */
    class Camara
    {
        public:
            /**
             * @brief Constructor con coordenadas y dimensiones como floats.
             * @param x Posición inicial en X
             * @param y Posición inicial en Y
             * @param w Ancho de la vista de cámara
             * @param h Alto de la vista de cámara
             */
            Camara(float x, float y,float w, float h);
            
            /**
             * @brief Constructor con vectores de posición y dimensión.
             * @param pos Posición inicial como Vector2D
             * @param dim Dimensión como Vector2D (ancho, alto)
             */
            Camara(const Vector2D& pos, const Vector2D& dim);
            
            /** @brief Destructor virtual */
            virtual ~Camara(){};
            
            /**
             * @brief Obtiene el componente de transformación de la cámara.
             * @return Referencia mutable a ITransform
             */
            [[nodiscard]]ITransform& getTransformada()
            {
                return *m_transform;
            }
            
            /**
             * @brief Obtiene la vista SFML de la cámara.
             * @return Referencia constante a sf::View
             */
            [[nodiscard]]sf::View& getView() const
            {
                return *m_view;
            }

            /**
             * @brief Vincula el seguimiento de cámara a un objeto.
             * @param obj Puntero compartido al objeto a seguir
             * 
             * La cámara seguirá la posición de este objeto según su implementación
             * de onUpdate (fija, cuadro limitante, LERP, etc).
             */
            void lockEnObjeto(const std::shared_ptr<Objeto>& obj);

            /**
             * @brief Establece el tamaño de la vista de cámara.
             * @param x Nuevo ancho
             * @param y Nuevo alto
             */
            void setViewSize(float x, float y);

            /**
             * @brief Actualiza la cámara.
             * @param dt Delta time en segundos
             * 
             * Por defecto no realiza cambios. Las subclases sobrescriben
             * para implementar seguimiento del objeto vinculado.
             */
            virtual void onUpdate(float dt);
            
        public:
            /** @brief Ancho de la vista de cámara en píxeles */
            float cam_width;
            
            /** @brief Alto de la vista de cámara en píxeles */
            float cam_height;
            
            /** @brief Contador estático de cámaras totales */
            static int num_camaras;
            
            /** @brief Nombre/identificador de la cámara */
            std::string nombre;
            
            /** @brief Bandera que indica si la cámara está activa */
            bool esta_activa;

        protected:
            /** @brief Vista SFML compartida */
            std::shared_ptr<sf::View> m_view;
            
            /** @brief Referencia débil al objeto siendo seguido (evita ciclos) */
            std::weak_ptr<Objeto> m_lockObj;
            
            /** @brief Componente de transformación (posición, velocidad, rotación) */
            std::shared_ptr<ITransform> m_transform;
    };
    
    /**
     * @class CamaraCuadro
     * @brief Cámara que sigue objetos dentro de un cuadro/área delimitada.
     * 
     * Implementa seguimiento de cámara con límites. La cámara intenta mantener
     * el objeto en la vista pero no se sale de los límites establecidos.
     */
    class CamaraCuadro: public Camara
    {
        public:
            /**
             * @brief Constructor del cuadro limitante.
             * @param pos Posición inicial de la cámara
             * @param dim Dimensiones (ancho, alto) del área limitante
             */
            CamaraCuadro(const Vector2D& pos, const Vector2D& dim);
            
            /** @brief Destructor virtual */
            ~CamaraCuadro() override{};
            
            /**
             * @brief Actualiza la cámara con seguimiento limitado por cuadro.
             * @param dt Delta time en segundos
             * 
             * Mueve la cámara para seguir el objeto pero respetando los límites
             * de X e Y establecidos.
             */
            void onUpdate(float dt) override;
            
        private:
            /** @brief Límite máximo en el eje X */
            float limitex;
            
            /** @brief Límite máximo en el eje Y */
            float limitey;
    };

    /**
     * @class CamaraLERP
     * @brief Cámara que sigue objetos con interpolación linear suave (LERP).
     * 
     * Implementa seguimiento de cámara con suavizado mediante LERP, creando
     * un efecto de seguimiento fluido y cinematográfico.
     */
    class CamaraLERP: public Camara
    {
        public:
            /**
             * @brief Constructor de cámara LERP.
             * @param pos Posición inicial de la cámara
             * @param dim Dimensiones (ancho, alto) de la vista
             */
            CamaraLERP(const Vector2D& pos, const Vector2D& dim);
            
            /** @brief Destructor virtual */
            ~CamaraLERP()override{};
            
            /**
             * @brief Actualiza la cámara con interpolación suave.
             * @param dt Delta time en segundos
             * 
             * Interpola la posición de la cámara hacia el objeto seguido
             * usando LERP para un movimiento suave y natural.
             */
            void onUpdate(float dt)override;
            
        private:
            /** @brief Velocidad de interpolación LERP (0.0 a 1.0 típicamente) */
            float vel;
    };


    /**
     * @class CamaraSnapVentana
     * @brief Cámara que sigue objetos después de que crucen un área (la ventana) de lo contrario no la sigue. Bueno para juegos de plataforma
     * 
     */
    class CamaraSnapVentana: public Camara
    {
        public:
            CamaraSnapVentana(const Vector2D& pos, const Vector2D& dim, const Vector2D& dven);
            ~CamaraSnapVentana() override{};
            void onUpdate(float dt) override;
        public:
            Vector2D m_vdim;
        private:
            bool lock{false};
    };

    /**
     * @class CamaraVentanaPlataforma
     * @brief Cámara ventana con platform snapping (estilo Super Mario Bros 3 / Super Mario World).
     *
     * En ambos ejes se comporta como CamaraSnapVentana: el jugador empuja la cámara
     * al tocar los bordes de la ventana. Además, en cuanto el jugador "aterriza"
     * (deja de moverse en el eje y) la cámara se alinea suavemente con él en y.
     */
    class CamaraVentanaPlataforma: public CamaraSnapVentana
    {
        public:
            CamaraVentanaPlataforma(const Vector2D& pos, const Vector2D& dim, const Vector2D& dven);
            ~CamaraVentanaPlataforma() override{};
            void onUpdate(float dt) override;
        private:
            /** @brief Velocidad del snap vertical (mayor = más rápido) */
            float vel_snap{5.f};
            /** @brief Posición y del jugador en el frame anterior, para saber si aterrizó */
            float y_prev{0.f};
    };

    /**
     * @class CamaraVentanaSnapFoco
     * @brief Cámara ventana + snap al foco (dual-forward-focus de Super Mario World).
     *
     * En x hay 4 líneas: los bordes de la ventana (punteadas, a ±m_vdim.x/2 del centro)
     * y las líneas de foco (sólidas, a ±foco del centro). El jugador queda anclado en la
     * línea sólida trasera para ver más hacia donde avanza; al regresar la cámara no se mueve
     * hasta que cruza la línea punteada contraria, y entonces se desliza (más rápido que él)
     * hasta encuadrarlo en la otra línea sólida. En y usa ventana + platform snapping.
     */
    class CamaraVentanaSnapFoco: public CamaraVentanaPlataforma
    {
        public:
            CamaraVentanaSnapFoco(const Vector2D& pos, const Vector2D& dim, const Vector2D& dven, float foco);
            ~CamaraVentanaSnapFoco() override{};
            void onUpdate(float dt) override;
        public:
            /** @brief Distancia del centro de la cámara a las líneas sólidas (donde se ancla el jugador) */
            float foco;
        private:
            /** @brief Velocidad del re-encuadre en px/s (debe ser mayor que la del jugador) */
            float vel_reencuadre{1000.f};
            /** @brief Hacia dónde mira la cámara: 1 derecha, -1 izquierda */
            int dir{1};
            /** @brief true mientras se desliza para encuadrar al jugador en la otra línea sólida */
            bool reencuadrando{false};
            bool iniciada{false};
    };
}
