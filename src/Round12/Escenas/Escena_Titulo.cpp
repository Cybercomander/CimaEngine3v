#include "Escena_Titulo.hpp"
#include <Motor/Camaras/CamarasGestor.hpp>
#include <Motor/Primitivos/GestorAssets.hpp>
#include <Motor/Render/Render.hpp>

namespace Round12
{
    void Escena_Titulo::onInit()
    {
        CE::GestorCamaras::Get().setCamaraActiva(0);
    }
    void Escena_Titulo::onRender()
    {
        sf::Text t{CE::GestorAssets::Get().getFont("default_font"),"ROUND 12",72};
        t.setPosition({380.f,300.f});
        CE::Render::Get().AddToDraw(t);
    }
}
