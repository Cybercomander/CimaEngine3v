#pragma once
#include "../GUI/GLayer.hpp"
#include "../Primitivos/Objetos.hpp"
#include "../Utils/Utils.hpp"
#include <string>
#include <vector>

namespace CE
{
/**
 * @class GHistograma
 * @brief Panel ImGui con el histograma de un valor de los entes de la simulación (Lab 5).
 *
 * Cada instancia grafica una sola medida (score, alcance o hp máximo), por eso
 * el motor crea una ventana por medida.
 *
 * @note No distingue la escena: en escenas sin entes con IScore sale vacío.
 */
class GHistograma : public GLayer
{
  public:
    /** @brief Valor de cada ente que se va a graficar */
    enum class Medida
    {
        SCORE,   ///< comida consumida en la generación (IScore)
        ALCANCE, ///< rango de visión (stats->agi)
        HP_MAX   ///< energía máxima (stats->hp_max)
    };

    struct Histograma
    {
        std::vector<float> bins; // conteo por bin
        float minVal = 0.0f;     // inicio del eje x
        float maxVal = 1.0f;     // fin del eje x
        float binWidth = 0.0f;
        float peak = 0.0f;
        size_t n = 0;          // entes graficados
        float promedio = 0.0f; // promedio del valor en los entes
        float datoMin = 0.0f;  // valor más chico encontrado
        float datoMax = 0.0f;  // valor más grande encontrado
    };

  public:
    explicit GHistograma(Medida m);
    ~GHistograma() override {};

    void OnInit(const MotorConfig &des) override;
    void OnUpdate(float dt) override;
    void OnRender(void) override;

  private:
    GHistograma::Histograma crearHistograma(const std::vector<std::shared_ptr<CE::Objeto>> &objetos);
    float getValor(CE::Objeto &ente) const;

  private:
    Medida medida;
    std::string titulo;
    int num_bins;
    float min_val;
    float max_val;
    GHistograma::Histograma histograma;
};
} // namespace CE
