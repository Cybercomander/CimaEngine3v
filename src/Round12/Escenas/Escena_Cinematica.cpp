#include "Escena_Cinematica.hpp"
#include <Round12/JuegoR12.hpp>
#include <Round12/Logica/Partida.hpp>
#include <Round12/Rutas.hpp>
#include <Motor/Primitivos/GestorAssets.hpp>
#include <Round12/objetos/FabricaR12.hpp>
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
            std::stringstream ss{l};
            LineaDialogo d;
            std::string pose;
            if(!std::getline(ss,d.orador,'|') || !std::getline(ss,pose,'|') ||
               !std::getline(ss,d.fondo,'|') || !std::getline(ss,d.texto))
            {
                std::cerr<<"[cinematica] línea mal formada en "<<ruta<<": "<<l<<"\n";
                continue;
            }
            d.pose = std::atoi(pose.c_str());
            lineas.push_back(d);
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
        if(!terminar) return;
        terminar = false;
        Partida::Get().historia().avanzar();
        irAPaso();
    }

    namespace
    {
        void dibujarOrador(sf::RenderTarget& t, sf::Vector2f tam, const LineaDialogo& l)
        {
            if(l.orador == "narrador" || l.orador == "-") return;
            std::string hoja = l.orador == "carlos" ? "carlos_sanchez/spritesheet_npc.png" :
                l.orador + "/spritesheet_4poses.png";
            const std::pair<const char*,const char*> nombres[] = {
                {"diego","diego_ureta"},{"brandon","brandon_salazar"},{"javier","javier_castaneda"},
                {"kevin","kevin_ramirez"},{"ricardo","ricardo_oropeza"}};
            for(auto& [corto,largo] : nombres)
                if(l.orador == corto) hoja = std::string(largo) + "/spritesheet_4poses.png";
            auto& tex = textura("sprites/personajes/" + hoja);
            int alto = (int)tex.getSize().y;
            sf::Sprite s{tex,sf::IntRect{{44*l.pose,0},{44,alto}}};
            float esc = tam.y*0.5f/alto;
            s.setScale({esc,esc});
            s.setPosition({tam.x*0.06f, tam.y*0.66f - alto*esc});
            t.draw(s);
        }
    }

    void Escena_Cinematica::dibujarPantalla(sf::RenderTarget& t, sf::Vector2f tam)
    {
        if(dialogo.fin()) return;
        const auto& l = dialogo.actual();
        if(l.fondo != "-")
        {
            sf::Sprite fondo{textura("escenarios_1080x720/" + l.fondo + "_1080x720.png")};
            auto ft = fondo.getTexture().getSize();
            fondo.setScale({tam.x/ft.x, tam.y/ft.y});
            t.draw(fondo);
        }
        else
        {
            sf::RectangleShape negro{tam};
            negro.setFillColor(sf::Color::Black);
            t.draw(negro);
        }
        dibujarOrador(t,tam,l);
        sf::RectangleShape caja{{tam.x*0.9f,tam.y*0.26f}};
        caja.setPosition({tam.x*0.05f,tam.y*0.68f});
        caja.setFillColor(sf::Color(10,10,20,225));
        caja.setOutlineColor(sf::Color::White);
        caja.setOutlineThickness(3.f);
        t.draw(caja);
        if(l.orador != "narrador" && l.orador != "-")
        {
            std::string nom = l.orador;
            nom[0] = (char)std::toupper(nom[0]);
            t.draw(texto(nom,(unsigned)(tam.y*0.035f),sf::Color(250,220,60),{tam.x*0.07f,tam.y*0.69f}));
        }
        auto completo = sf::String::fromUtf8(l.texto.begin(),l.texto.end());
        auto parcial = completo.substring(0,std::min<size_t>((size_t)letras,completo.getSize()));
        // ajuste de línea simple por palabras
        sf::String envuelto;
        size_t col = 0, max_col = 58;
        for(size_t i = 0; i < parcial.getSize(); ++i)
        {
            if(parcial[i] == ' ' && col > max_col) { envuelto += '\n'; col = 0; continue; }
            envuelto += parcial[i];
            col++;
        }
        sf::Text cuerpo{CE::GestorAssets::Get().getFont("default_font"),envuelto,(unsigned)(tam.y*0.034f)};
        cuerpo.setPosition({tam.x*0.07f,tam.y*0.745f});
        t.draw(cuerpo);
        auto ayuda = texto("J: continuar   Esc: saltar",(unsigned)(tam.y*0.025f),sf::Color(180,180,180),
                {tam.x*0.72f,tam.y*0.95f});
        t.draw(ayuda);
    }
}
