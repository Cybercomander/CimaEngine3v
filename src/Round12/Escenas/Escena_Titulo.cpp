#include "Escena_Titulo.hpp"
#include <Round12/JuegoR12.hpp>
#include <Round12/Logica/Partida.hpp>
#include <Round12/objetos/Audio.hpp>
#include <Round12/objetos/FabricaR12.hpp>

namespace Round12
{
    namespace
    {
        const char* OPCIONES[] = {"Demo (3 niveles)","Historia completa","Controles","Volver al lanzador"};
        constexpr int N = 4;
        const char* CONTROLES[] = {
            "W A S D  -  moverse / cambiar de carril",
            "J  jab (1)    K  recto (2)    L  gancho (3)    I  uppercut (4)",
            "Shift (mantener)  -  guardia",
            "Q / E  -  esquiva lateral",
            "Combinaciones: 1-1-2, 1-2-3, 3-4  ·  «Frontera» 1-2-3-4",
            "Esc  -  pausa    ·    F1  -  lanzador de juegos"};
    }

    void Escena_Titulo::alPresionar(const std::string& a, bool p)
    {
        if(!p) return;
        if(controles) { controles = false; return; }
        if(a == "arriba") { opcion = (opcion + N - 1) % N; Audio::efecto("menu"); }
        if(a == "abajo")  { opcion = (opcion + 1) % N; Audio::efecto("menu"); }
        if(a != "jab" && a != "aceptar") return;
        switch(opcion)
        {
            case 0: Partida::Get().nueva(true);  irAPaso(); break;
            case 1: Partida::Get().nueva(false); irAPaso(); break;
            case 2: controles = true; break;
            default: if(auto* j = JuegoR12::instancia()) j->pedirLanzador(); break;
        }
    }

    void Escena_Titulo::dibujarPantalla(sf::RenderTarget& t, sf::Vector2f tam)
    {
        sf::Sprite fondo{textura("escenarios_1080x720/01_sanchez_boxing_gym_1080x720.png")};
        auto ft = fondo.getTexture().getSize();
        fondo.setScale({tam.x/ft.x, tam.y/ft.y});
        fondo.setColor(sf::Color(110,110,110));
        t.draw(fondo);
        auto titulo = texto("ROUND 12",(unsigned)(tam.y*0.14f),sf::Color(250,220,60),{0.f,tam.y*0.08f});
        centrarX(titulo,tam.x/2.f);
        t.draw(titulo);
        auto sub = texto("El camino de Diego a «Guerra en la Frontera»",(unsigned)(tam.y*0.035f),
                sf::Color::White,{0.f,tam.y*0.27f});
        centrarX(sub,tam.x/2.f);
        t.draw(sub);
        if(controles)
        {
            for(int i = 0; i < 6; ++i)
            {
                auto l = texto(CONTROLES[i],(unsigned)(tam.y*0.035f),sf::Color::White,{0.f,tam.y*(0.4f + i*0.07f)});
                centrarX(l,tam.x/2.f);
                t.draw(l);
            }
            return;
        }
        for(int i = 0; i < N; ++i)
        {
            auto o = texto((i == opcion ? "> " : "") + std::string(OPCIONES[i]),(unsigned)(tam.y*0.05f),
                    i == opcion ? sf::Color(250,220,60) : sf::Color::White,{0.f,tam.y*(0.42f + i*0.09f)});
            centrarX(o,tam.x/2.f);
            t.draw(o);
        }
        auto ayuda = texto("W/S elegir  ·  J o Enter aceptar",(unsigned)(tam.y*0.03f),
                sf::Color(200,200,200),{0.f,tam.y*0.9f});
        centrarX(ayuda,tam.x/2.f);
        t.draw(ayuda);
    }
}
