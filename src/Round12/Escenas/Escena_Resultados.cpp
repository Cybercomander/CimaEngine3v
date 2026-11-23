#include "Escena_Resultados.hpp"
#include <Round12/JuegoR12.hpp>
#include <Round12/Logica/Partida.hpp>
#include <iostream>

namespace Round12
{
    namespace
    {
        const std::pair<const char*,int Atributos::*> CAMPOS[] = {
            {"Resistencia",&Atributos::resistencia},{"Aire",&Atributos::aire},
            {"Sed (tolerancia)",&Atributos::sed},{"Concentración",&Atributos::concentracion},
            {"Velocidad",&Atributos::velocidad},{"Defensa",&Atributos::defensa},{"Poder",&Atributos::poder}};
    }

    void Escena_Resultados::actualizar(float dt)
    {
        t += dt;
        if(JuegoR12::prueba.autopiloto && t > 1.5f) { t = -1000.f; alPresionar("jab",true); }
    }

    void Escena_Resultados::alPresionar(const std::string& a, bool p)
    {
        if(!p || (a != "jab" && a != "aceptar")) return;
        auto& partida = Partida::Get();
        std::cout<<"[prueba-r12] resultados: "<<partida.resumen<<std::endl;
        partida.alTerminarNivel();
        partida.historia().avanzar();
        irAPaso();
    }

    void Escena_Resultados::dibujarPantalla(sf::RenderTarget& t, sf::Vector2f tam)
    {
        sf::RectangleShape fondo{tam};
        fondo.setFillColor(sf::Color(18,30,24));
        t.draw(fondo);
        auto& p = Partida::Get();
        auto titulo = texto("RESULTADOS",(unsigned)(tam.y*0.08f),sf::Color(250,220,60),{0.f,tam.y*0.05f});
        centrarX(titulo,tam.x/2.f);
        t.draw(titulo);
        if(!p.resumen.empty())
        {
            auto r = texto(p.resumen,(unsigned)(tam.y*0.035f),sf::Color::White,{0.f,tam.y*0.16f});
            centrarX(r,tam.x/2.f);
            t.draw(r);
        }
        const auto& antes = p.antesDelNivel();
        const auto& ahora = p.diego();
        float y = tam.y*0.25f;
        for(auto& [etiqueta,campo] : CAMPOS)
        {
            int d = ahora.*campo - antes.*campo;
            t.draw(texto(etiqueta,(unsigned)(tam.y*0.04f),sf::Color::White,{tam.x*0.25f,y}));
            t.draw(texto(std::to_string(antes.*campo) + "  ->  " + std::to_string(ahora.*campo),
                        (unsigned)(tam.y*0.04f),sf::Color::White,{tam.x*0.55f,y}));
            if(d) t.draw(texto((d > 0 ? "+" : "") + std::to_string(d),(unsigned)(tam.y*0.04f),
                        d > 0 ? sf::Color(90,230,110) : sf::Color(230,90,90),{tam.x*0.72f,y}));
            y += tam.y*0.055f;
        }
        for(auto h : p.nuevasHabilidades())
        {
            t.draw(texto(std::string("Nueva habilidad: ") + Round12::nombre(h),(unsigned)(tam.y*0.035f),
                        sf::Color(110,190,255),{tam.x*0.25f,y}));
            y += tam.y*0.05f;
        }
        auto ayuda = texto("J: continuar",(unsigned)(tam.y*0.03f),sf::Color(200,200,200),{0.f,tam.y*0.92f});
        centrarX(ayuda,tam.x/2.f);
        t.draw(ayuda);
    }
}
