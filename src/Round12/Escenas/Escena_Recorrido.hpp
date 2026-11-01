#pragma once
#include "EscenaR12.hpp"
#include "Escena_Cinematica.hpp"
#include <Juego/objetos/Entidad.hpp>
#include <Round12/Datos/Niveles.hpp>
#include <Round12/Logica/Piso.hpp>
#include <memory>
#include <vector>

namespace Round12
{
    // R12_Recorrido: el escenario completo en una pantalla. En el Sanchez Boxing GYM tiene
    // las estaciones sobre lo pintado (costal, manoplas, cuerda, agua, plátano); en Mexicali
    // es la llegada al ring. Se sube al ring por la escalera roja.
    class Escena_Recorrido : public EscenaR12
    {
        protected:
            void cargar() override { usarCamara(); }
            void reiniciar() override;
            void actualizar(float dt) override;
            void dibujarMundo() override;
            void dibujarPantalla(sf::RenderTarget& t, sf::Vector2f tam) override;
            const char* musica() const override { return cfg.musica; }
            void alPresionar(const std::string& accion, bool presionado) override;
        private:
            enum class Mini { NINGUNO, COSTAL, MANOPLAS, CUERDA };
            struct Puesto { Estacion e; Pt pie; bool hecho{false}; const char* nombre; };
            Puesto* cerca();
            bool listoParaRing() const;
            void empezar(Puesto& p);
            void terminarMini(bool exito);
            void tecla(char t);
            void piloto(float dt);
            void moverCarlos(float dt);
            void animarDialogo(float dt);
            void mensaje(const std::string& m, float s = 3.f) { aviso = m; aviso_t = s; }
            Pt pies(const std::shared_ptr<IVJ::Entidad>& e) const;

            ConfigRecorrido cfg;
            const EscenarioR12* esc{nullptr};
            Piso piso;
            std::shared_ptr<IVJ::Entidad> diego, carlos, ricardo;
            std::vector<Puesto> puestos;
            Barras barras;
            Dialogo dialogo;
            bool en_dialogo{false};
            float letras{0.f}, reloj{0.f};
            size_t sig_dialogo{0};
            std::string aviso;
            float aviso_t{0.f};
            // mini-juegos
            Mini mini{Mini::NINGUNO};
            Puesto* activo{nullptr};
            float t_mini{0.f};
            int progreso{0};      // costal: indicación actual; manoplas: secuencia; cuerda: aciertos
            int reps{0};          // manoplas: repeticiones correctas de la secuencia actual
            int pos_seq{0};       // manoplas: tecla dentro de la secuencia
            bool mostrando{false};
            int ultimo_pulso{-1};
            int demo_i{-1};       // golpe de la demostración de Carlos
            float vaiven{0.f};
            // piloto automático
            std::vector<Pt> camino;
            float piloto_t{0.f};
            bool terminado{false};
    };
}
