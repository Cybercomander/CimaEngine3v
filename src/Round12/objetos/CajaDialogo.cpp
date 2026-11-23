#include "CajaDialogo.hpp"
#include <Motor/Primitivos/GestorAssets.hpp>
#include <Round12/objetos/FabricaR12.hpp>
#include <Round12/objetos/Hud.hpp>
#include <cmath>

namespace Round12
{
    size_t largoTexto(const std::string& s) { return sf::String::fromUtf8(s.begin(),s.end()).getSize(); }

    void dibujarCajaDialogo(sf::RenderTarget& t, sf::Vector2f tam, const LineaDialogo& l,
                            float letras, Vista vista, float tiempo)
    {
        bool narrador = l.orador == "narrador" || l.orador == "-";
        sf::FloatRect caja{{tam.x*0.04f,tam.y*0.665f},{tam.x*0.92f,tam.y*0.245f}};
        sf::RectangleShape fondo{caja.size};
        fondo.setPosition(caja.position);
        fondo.setFillColor(sf::Color(10,10,20,228));
        fondo.setOutlineColor(sf::Color::White);
        fondo.setOutlineThickness(3.f);
        t.draw(fondo);

        float lado_retrato = caja.size.y - 12.f;
        float x_texto = caja.position.x + 18.f, ancho_texto = caja.size.x - 36.f;
        if(!narrador)
        {
            bool escribiendo = letras < largoTexto(l.texto);
            bool boca = escribiendo && std::fmod(tiempo,0.24f) < 0.12f;
            std::string rostro = archivoRostro(l.orador,l.emocion,vista,l.lado,boca);
            std::string carpeta = carpetaPersonaje(l.orador);
            sf::Sprite s{rostro.empty()
                ? textura("sprites/personajes/" + carpeta + "/retratos/" + l.emocion + ".png")
                : textura("sprites/personajes/" + carpeta + "/dialogo_rostros/" + rostro,true)};
            float k = lado_retrato/s.getTexture().getSize().y;
            s.setScale({k,k});
            float x = l.lado == 0 ? caja.position.x + 6.f : caja.position.x + caja.size.x - 6.f - lado_retrato;
            s.setPosition({x,caja.position.y + 6.f});
            t.draw(s);
            if(l.lado == 0) x_texto += lado_retrato + 6.f;
            ancho_texto -= lado_retrato + 6.f;
            std::string nom = l.orador;
            nom[0] = (char)std::toupper(nom[0]);
            t.draw(texto(nom,(unsigned)(tam.y*0.033f),COLOR_CONCENTRACION,{x_texto,caja.position.y + 6.f}));
        }
        auto completo = sf::String::fromUtf8(l.texto.begin(),l.texto.end());
        auto parcial = completo.substring(0,std::min<size_t>((size_t)letras,completo.getSize()));
        unsigned tam_letra = (unsigned)(tam.y*0.031f);
        // ajuste de línea por palabras según el ancho disponible
        size_t max_col = (size_t)(ancho_texto/(tam_letra*0.55f));
        sf::String envuelto;
        size_t col = 0;
        for(size_t i = 0; i < parcial.getSize(); ++i)
        {
            if(parcial[i] == ' ' && col > max_col - 8) { envuelto += '\n'; col = 0; continue; }
            envuelto += parcial[i];
            col++;
        }
        sf::Text cuerpo{CE::GestorAssets::Get().getFont("default_font"),envuelto,tam_letra};
        cuerpo.setOutlineColor(sf::Color::Black);
        cuerpo.setOutlineThickness(1.f);
        cuerpo.setPosition({x_texto,caja.position.y + (narrador ? 14.f : tam.y*0.05f)});
        t.draw(cuerpo);
    }
}
