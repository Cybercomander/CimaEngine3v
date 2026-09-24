#pragma once
#include "../../Motor/Componentes/IComponentes.hpp"
#include "ICParte.hpp"
#include <vector>

namespace IVJ
{
// Componente que agrupa el cuerpo completo de la víbora.
// El orden del vector es el orden de persecución: partes[0] sigue a la cabeza
// y cada partes[i] sigue a partes[i - 1]. Crecer es agregar una parte al final
class ICPartesCuerpo : public CE::IComponentes
{
  public:
    // Posiciones del líder que acumula cada parte antes de perseguirlo.
    // Con la cabeza a ~3 px por frame, 10 frames equivalen a ~30 px de separación
    static constexpr int RETRASO_PARTE = 10;

    // w, h: tamaño de cada parte; num_partes: partes con las que nace el cuerpo;
    // pos_inicial: lugar donde aparecen (normalmente la posición de la cabeza)
    explicit ICPartesCuerpo(int w, int h, int num_partes, const CE::Vector2D &pos_inicial);
    ~ICPartesCuerpo() override {};
    // Devuelve una copia independiente del cuerpo completo
    std::shared_ptr<IComponentes> clonar() const override
    {
        return std::make_shared<ICPartesCuerpo>(*this);
    };

    // Agrega una parte al final del cuerpo. La parte nueva aparece sobre la cola actual
    // (o sobre la cabeza si el cuerpo está vacío) y espera su retraso antes de seguirla
    void agregarParte(const CE::Vector2D &pos_cabeza);

  public:
    // Todas las partes de la víbora, de la más cercana a la cabeza a la última
    std::vector<ICParte> partes;
    // Largo de cada parte en píxeles
    int width;
    // Alto de cada parte en píxeles
    int height;
};
} // namespace IVJ
