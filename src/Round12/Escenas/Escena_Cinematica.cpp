#include "Escena_Cinematica.hpp"
#include <Round12/JuegoR12.hpp>
#include <Round12/Logica/Partida.hpp>
#include <Round12/Rutas.hpp>
#include <Motor/Primitivos/GestorAssets.hpp>
#include <Round12/objetos/FabricaR12.hpp>
#include <Round12/objetos/CajaDialogo.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

namespace Round12
{
    bool Dialogo::cargar(const std::string& ruta)
    {
        lineas.clear();
        i = 0;
        std::ifstream f{ruta};
        if(!f) { std::cerr<<"[cinematica] no existe "<<ruta<<"\n"; return false; }
        std::string l;
        while(std::getline(f,l))
        {
            if(l.empty() || l[0] == '#') continue;
            if(auto d = parsearLinea(l)) lineas.push_back(*d);
            else std::cerr<<"[cinematica] línea mal formada en "<<ruta<<": "<<l<<"\n";
        }
        return !lineas.empty();
    }

    void Escena_Cinematica::reiniciar()
    {
        terminar = !dialogo.cargar(std::string(R12_DATOS) + "/dialogos/" + id + ".txt");
        letras = 0.f;
    }

    void Escena_Cinematica::alPresionar(const std::string& a, bool p)
    {
        if(!p) return;
        if(a == "pausa") { terminar = true; return; }
        if((a != "jab" && a != "aceptar") || dialogo.fin()) return;
        auto largo = sf::String::fromUtf8(dialogo.actual().texto.begin(),dialogo.actual().texto.end()).getSize();
        if(letras < largo) { letras = (float)largo; return; }
        letras = 0.f;
        if(!dialogo.siguiente()) terminar = true;
    }

    void Escena_Cinematica::actualizar(float dt)
    {
        letras += 40.f*dt; // máquina de escribir
        reloj += dt;
        if(JuegoR12::prueba.autopiloto && letras > 60.f) terminar = true; // prueba sin teclado
        if(!terminar) return;
        terminar = false;
        Partida::Get().historia().avanzar();
        irAPaso();
    }

    void Escena_Cinematica::dibujarPantalla(sf::RenderTarget& t, sf::Vector2f tam)
    {
        if(dialogo.fin()) return;
        const auto& l = dialogo.actual();
        sf::RectangleShape negro{tam};
        negro.setFillColor(sf::Color::Black);
        t.draw(negro);
        if(l.fondo != "-")
        {
            // el escenario en la franja central, como en el juego
            std::string ruta = l.fondo == "05_playa_rosarito" ? "escenarios/05_playa_rosarito/05_playa_rosarito_nativo.png"
                : "escenarios/" + l.fondo + "/decorado_1920x1080.png";
            sf::Sprite fondo{textura(ruta,true)};
            auto ft = fondo.getTexture().getSize();
            float alto = tam.y*(1.f - 2.f*56.f/720.f);
            fondo.setScale({tam.x/ft.x, alto/ft.y});
            fondo.setPosition({0.f,tam.y*56.f/720.f});
            t.draw(fondo);
        }
        dibujarCajaDialogo(t,tam,l,letras,Vista::TRESCUARTOS,reloj);
        auto ayuda = texto("J: continuar   Esc: saltar",(unsigned)(tam.y*0.025f),sf::Color(180,180,180),
                {tam.x*0.72f,tam.y*0.95f});
        t.draw(ayuda);
    }
}
