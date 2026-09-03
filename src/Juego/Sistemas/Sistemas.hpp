#pragma once
#include "../../Motor/Componentes/IComponentes.hpp"
#include "../../Motor/Primitivos/CEPool.hpp"
#include "../../Motor/Primitivos/Objetos.hpp"
#include "../Componentes/IJComponentes.hpp"
#

namespace IVJ
{
[[maybe_unused]] void SistemaControl(CE::Objeto &ente, float dt);
[[maybe_unused]] void SistemaMover(const std::vector<std::shared_ptr<CE::Objeto>> &entes, float dt);
[[maybe_unused]] void SistemaMover(const std::shared_ptr<CE::Objeto> &objeto, float dt);
[[maybe_unused]] bool SistemaColAABB(CE::Objeto &A, CE::Objeto &B, bool resolucion = false);
[[maybe_unused]] bool SistemaColAABBMid(CE::Objeto &A, CE::Objeto &B, bool resolucion = false);

void SistemaMoverBalas(const std::shared_ptr<CE::Objeto> &ente, float dt);

[[maybe_unused]] void SistemaSpawn(CE::Pool &spwans);

[[maybe_unused]] void SistemaPaths(std::vector<std::shared_ptr<CE::Objeto>> &obj);

[[maybe_unused]] bool checkRayoHit(CE::Objeto &npc, CE::Vector2D &p1, CE::Vector2D &p2);

// void SistemaDialogos(CE::Objeto& npc);
[[maybe_unused]] void SistemaDialogos(IVJ::IDialogo *dialogo, CE::Objeto &obj);

bool revisarDistanciaInteraccion(CE::Objeto &jugador, CE::Objeto &npc, float distancia_maxima);

// Sistema de giro: mueve el objeto sobre una circunferencia si trae el componente IGirar.
// [[maybe_unused]] evita la advertencia si la función no se usa (se compila con -Werror)
[[maybe_unused]] void SistemaGirar(CE::Objeto &ente, float dt);

// Sistema de vaiven vertical: sube y baja al objeto si trae el componente IMoverVertical
[[maybe_unused]] void SistemaMoverVertical(CE::Objeto &ente, float dt);

// Sistema de onda: avanza al objeto en horizontal dibujando una onda,
// siempre que traiga el componente IMoverOnda
[[maybe_unused]] void SistemaMoverOnda(CE::Objeto &ente, float dt);

// Sistema de orbita: hace girar al objeto alrededor de un centro fijo
// si trae el componente IMoverCircular
[[maybe_unused]] void SistemaMoverCircular(CE::Objeto &ente, float dt);

// DEBUG
[[maybe_unused]] void pintarLinea(CE::Vector2D &p1, CE::Vector2D &p2, const sf::Color &color = sf::Color::Red);


} // namespace IVJ
