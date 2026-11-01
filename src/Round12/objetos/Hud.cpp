#include "Hud.hpp"
#include <Motor/Primitivos/GestorAssets.hpp>
#include <Motor/Render/Render.hpp>
#include <Round12/Logica/Stats.hpp>
#include <algorithm>

namespace Round12
{
    sf::Text texto(const std::string& utf8, unsigned tam, sf::Color color, sf::Vector2f pos)
    {
        sf::Text t{CE::GestorAssets::Get().getFont("default_font"),sf::String::fromUtf8(utf8.begin(),utf8.end()),tam};
        t.setFillColor(color);
        t.setOutlineColor(sf::Color::Black);
        t.setOutlineThickness(tam > 20 ? 2.f : 1.f);
        t.setPosition(pos);
        return t;
    }

    void centrarX(sf::Text& t, float centro)
    {
        auto b = t.getLocalBounds();
        t.setPosition({centro - b.size.x/2.f - b.position.x, t.getPosition().y});
    }

    EnPantalla::EnPantalla()
        :rt{CE::Render::Get().GetTextura()}
    {
        previa = rt.getView();
        rt.setView(rt.getDefaultView());
        tam = sf::Vector2f(rt.getSize());
    }
    EnPantalla::~EnPantalla() { rt.setView(previa); }

    void Widget::dibujar(sf::RenderTarget& t, sf::Vector2f tam)
    {
        if(!visible) return;
        for(auto& h : hijos)
            h->dibujar(t,tam);
    }

    void Panel::dibujar(sf::RenderTarget& t, sf::Vector2f tam)
    {
        if(!visible) return;
        sf::RectangleShape r{{area.size.x*tam.x, area.size.y*tam.y}};
        r.setPosition({area.position.x*tam.x, area.position.y*tam.y});
        r.setFillColor(fondo);
        r.setOutlineColor(sf::Color(255,255,255,120));
        r.setOutlineThickness(2.f);
        t.draw(r);
        Widget::dibujar(t,tam);
    }

    void Barra::dibujar(sf::RenderTarget& t, sf::Vector2f tam)
    {
        if(!visible) return;
        float w = ancho*tam.x, h = std::max(8.f, tam.y*0.018f);
        float x = pos.x*tam.x, y = pos.y*tam.y;
        float frac = std::clamp(*valor / std::max(1.f,*maximo), 0.f, 1.f);
        sf::RectangleShape fondo{{w,h}};
        fondo.setPosition({x,y});
        fondo.setFillColor(sf::Color(20,20,30,200));
        fondo.setOutlineColor(sf::Color::Black);
        fondo.setOutlineThickness(2.f);
        sf::RectangleShape lleno{{w*frac,h}};
        lleno.setPosition({espejo ? x + w*(1.f-frac) : x, y});
        lleno.setFillColor(color);
        t.draw(fondo);
        t.draw(lleno);
        auto txt = texto(etiqueta,(unsigned)std::max(10.f,h*0.95f),sf::Color::White);
        auto b = txt.getLocalBounds();
        txt.setPosition({espejo ? x + w + 6.f : x - b.size.x - 6.f, y - h*0.25f});
        t.draw(txt);
    }

    void Etiqueta::dibujar(sf::RenderTarget& t, sf::Vector2f tam)
    {
        if(!visible) return;
        auto txt = texto(fuente(),(unsigned)std::max(10.f,tam_rel*tam.y),color,{pos.x*tam.x,pos.y*tam.y});
        if(centrada) centrarX(txt,pos.x*tam.x);
        t.draw(txt);
    }

    namespace { const float CIEN = BARRA_MAX; }

    void agregarBarras(Widget& raiz, const Barras& b, bool derecha, const std::string& nombre_boxeador)
    {
        float x = derecha ? 0.60f : 0.12f, w = 0.28f;
        raiz.agregar(std::make_unique<Etiqueta>([nombre_boxeador]{ return nombre_boxeador; },
                    sf::Vector2f{derecha ? x : x, 0.015f}, 0.03f));
        const char* et[] = {"RES","AIRE","SED","CONC"};
        const float* val[] = {&b.resistencia,&b.aire,&b.sed,&b.concentracion};
        const float* max[] = {&b.resistencia_max,&CIEN,&CIEN,&CIEN};
        sf::Color col[] = {COLOR_RESISTENCIA,COLOR_AIRE,COLOR_SED,COLOR_CONCENTRACION};
        for(int i = 0; i < 4; ++i)
            raiz.agregar(std::make_unique<Barra>(et[i],val[i],max[i],col[i],
                        sf::Vector2f{x,0.06f + i*0.032f},w,derecha));
    }
}
