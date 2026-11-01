#pragma once
#include <Round12/Datos/Perfiles.hpp>
#include <Round12/Logica/Partida.hpp>
#include <SFML/Graphics/Color.hpp>
#include <string>
#include <utility>
#include <vector>

// Configuración de cada paso de la historia (el id de Paso indexa estas tablas)
namespace Round12
{
    using Mejora = std::pair<int Atributos::*, int>;

    struct Escenario
    {
        const char* mapa;     // ROUND12_Assets/motor/<mapa>.txt y colisiones/<mapa>_colisiones.txt
        float ancho;          // ancho del mundo en px nativos
        float ring_x0, ring_x1; // entre los postes
        bool publico;
    };
    constexpr float RING_PIES_FONDO = 148.f, RING_PIES_FRENTE = 160.f; // lona (carriles)
    const Escenario& escenario(const std::string& nombre);

    struct ConfigPelea
    {
        Personaje rival;
        int rounds;
        float segundos;
        Objetivo objetivo;
        std::string escenario;
        bool jueces;
        int dificultad;          // 2 = revanchas: atributos del rival +1
        sf::Color tinte{sf::Color::White};
        bool aleatorio{false};   // torneo local: rival y atributos al azar
        std::vector<Habilidad> al_iniciar{}, al_terminar{};
        std::vector<Mejora> mejoras{};
    };
    const ConfigPelea& pelea(const std::string& id);

    enum class Estacion { COSTAL, MANOPLAS, CUERDA, AGUA };
    struct ConfigRecorrido
    {
        std::string mapa;
        std::vector<Estacion> estaciones;
        float x_ring;
        std::string dialogo_inicio;
        std::vector<std::pair<float,std::string>> dialogos_en_x;
        const char* musica;
        bool platano{false};
    };
    const ConfigRecorrido& recorrido(const std::string& id);

    struct ConfigRunner
    {
        std::string mapa;
        float meta, tiempo;
        int vueltas;          // 0 = playa (meta lineal)
        float meta_vuelta;    // segundos por vuelta para el bono (circuito)
        bool olas;
        unsigned semilla;
    };
    const ConfigRunner& runner(const std::string& id);

    struct ConfigSantaFe { float tiempo; };
    const ConfigSantaFe& santaFe(const std::string& id);

    // todos los ids existen (para la prueba de la historia)
    bool existeNivel(TipoPaso t, const std::string& id);
}
