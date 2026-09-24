#include "GHistograma.hpp"
#include <Juego/Componentes/IJComponentes.hpp>
#include <Motor/Primitivos/GestorEscenas.hpp>
#include <algorithm>
#include <cstdio>
#include <imgui.h>
#include <memory>
#include <vector>

namespace CE
{
// Cada medida define su ventana y su eje x. Los rangos de ALCANCE y HP_MAX son
// los mismos con los que Escena_Sim sortea los stats (agi 10-255, hp_max 80-200),
// así el eje no cambia entre generaciones y se ve cómo se mueve la distribución
GHistograma::GHistograma(Medida m) : GLayer{}, medida{m}
{
    switch (medida)
    {
    case Medida::SCORE:
        // un lote por valor de score: lote 0 = score 0, lote 1 = score 1, ...
        titulo = "Histograma Score";
        num_bins = 10;
        min_val = 0.f;
        max_val = 10.f;
        break;
    case Medida::ALCANCE:
        // lotes de 25: [10,35), [35,60), ..., [235,260)
        titulo = "Histograma Alcance (agi)";
        num_bins = 10;
        min_val = 10.f;
        max_val = 260.f;
        break;
    case Medida::HP_MAX:
        // lotes de 10: [80,90), [90,100), ..., [190,200]
        titulo = "Histograma HP máximo";
        num_bins = 12;
        min_val = 80.f;
        max_val = 200.f;
        break;
    }
}
void GHistograma::OnInit(const MotorConfig &des)
{
    (void)des;
}
void GHistograma::OnUpdate(float dt)
{
    (void)dt;
    // se recalcula cada frame: los valores cambian aunque el número de entes no cambie
    histograma = crearHistograma(CE::GestorEscenas::Get().getEscenaActual().getPool().getPool());
}
void GHistograma::OnRender(void)
{
    if (ImGui::Begin(titulo.c_str()))
    {
        char overlay[96];
        snprintf(overlay, sizeof(overlay), "n=%zu  prom=%.1f  [%.0f, %.0f]", histograma.n, histograma.promedio,
                 histograma.datoMin, histograma.datoMax);

        ImGui::PlotHistogram("##hist", histograma.bins.data(), (int)histograma.bins.size(),
                             0,                                     // offset
                             overlay, 0.0f, histograma.peak * 1.1f, // escala Y
                             ImVec2(-1, 150));                      // -1 = ancho disponible
        ImGui::Text("eje x: [%.0f, %.0f]  ancho de lote: %.1f", histograma.minVal, histograma.maxVal,
                    histograma.binWidth);
    }
    ImGui::End();
}
float GHistograma::getValor(CE::Objeto &ente) const
{
    switch (medida)
    {
    case Medida::SCORE:
        return (float)ente.getComponente<IVJ::IScore>()->score;
    case Medida::ALCANCE:
        return (float)ente.getStats()->agi;
    case Medida::HP_MAX:
        return (float)ente.getStats()->hp_max;
    }
    return 0.f;
}
GHistograma::Histograma GHistograma::crearHistograma(const std::vector<std::shared_ptr<CE::Objeto>> &objetos)
{
    GHistograma::Histograma hist;
    hist.minVal = min_val;
    hist.maxVal = max_val;
    hist.binWidth = (max_val - min_val) / num_bins;
    hist.bins.assign(num_bins, 0.0f);

    std::vector<float> data;
    for (auto &ente : objetos)
    {
        // solo los entes de la simulación tienen IScore
        if (!ente->tieneComponente<IVJ::IScore>())
            continue;
        data.push_back(getValor(*ente));
    }
    if (data.empty())
        return hist;

    auto mm = std::minmax_element(data.begin(), data.end());
    hist.datoMin = *mm.first;
    hist.datoMax = *mm.second;
    hist.n = data.size();

    float suma = 0.f;
    for (float valor : data)
    {
        suma += valor;
        // interpolamos el valor para ver en qué lote cae; el clamp evita escribir
        // fuera del arreglo con valores fuera del eje (p. ej. score >= 10)
        int idx = (int)((valor - hist.minVal) / hist.binWidth);
        idx = std::clamp(idx, 0, num_bins - 1);
        hist.bins[idx]++;
    }
    hist.promedio = suma / data.size();

    hist.peak = *std::max_element(hist.bins.begin(), hist.bins.end());
    return hist;
}

} // namespace CE
