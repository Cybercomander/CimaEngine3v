#include "Escena_SantaFe.hpp"
#include <Juego/Componentes/IJComponentes.hpp>
#include <Motor/Render/Render.hpp>
#include <Round12/JuegoR12.hpp>
#include <Round12/Logica/Eventos.hpp>
#include <Round12/Logica/MapaColision.hpp>
#include <Round12/Maquinas/NodosR12.hpp>
#include <Round12/Rutas.hpp>
#include <Round12/objetos/FabricaR12.hpp>
#include <algorithm>
#include <cmath>

namespace Round12
{
    namespace
    {
        constexpr float MUNDO_H = 640.f, X0 = 44.f, X1 = 276.f, META = 100.f;
        constexpr float GRAVEDAD = 420.f, SALTO = 190.f, PASO = 70.f;
    }

    void Escena_SantaFe::cargar()
    {
        usarCamara();
        // las rampas del parque (valor 2 en la capa de colisión) sirven de plataforma
        plataformas = MapaColision::cargar(std::string(R12_RECURSOS) + "/colisiones/04_skate_santa_fe_colisiones.txt",2);
        plataformas.erase(std::unique(plataformas.begin(),plataformas.end(),[](const Rect& a, const Rect& b){
                    return a.x == b.x && std::abs(a.y - b.y) <= 8.f; }),plataformas.end());
        // escalones y barandales entre rampas: un salto (~43 px) alcanza el siguiente
        const float xs[] = {90.f,140.f,190.f,230.f,190.f,140.f};
        int i = 0;
        for(float y = MUNDO_H - 28.f; y > META - 10.f; y -= 28.f, ++i)
        {
            bool choca = std::any_of(plataformas.begin(),plataformas.end(),[y](const Rect& r){ return std::abs(r.y - y) < 14.f; });
            if(!choca) plataformas.push_back(Rect{xs[i % 6] - 28.f,y,56.f,6.f});
        }
        plataformas.push_back(Rect{X0,META - 10.f,X1 - X0,6.f}); // la cima
        std::sort(plataformas.begin(),plataformas.end(),[](const Rect& a, const Rect& b){ return a.y > b.y; });
        // puntos de agua a un tercio y dos tercios de la subida
        for(float alto : {MUNDO_H*2.f/3.f, MUNDO_H/3.f})
        {
            size_t mejor = 0;
            for(size_t k = 0; k < plataformas.size(); ++k)
                if(std::abs(plataformas[k].y - alto) < std::abs(plataformas[mejor].y - alto)) mejor = k;
            puntos_agua.push_back(mejor);
        }
    }

    void Escena_SantaFe::reiniciar()
    {
        tiempo = cp_tiempo = santaFe(id).tiempo;
        x = cp_x = 160.f;
        pies = cp_pies = MUNDO_H - 8.f;
        vy = 0.f;
        aire = 100.f;
        sed = 0.f;
        terminado = false;
        aviso_t = 0.f;
        objetivo = nullptr;
        diego = FabricaR12::boxeador(Personaje::DIEGO,x,pies,IVJ::IDireccion::DERECHA,std::make_shared<IdleR12>());
        diego->getComponente<IBoxeador>()->atr = Partida::Get().diego();
        diego->getComponente<IBoxeador>()->jugador = true;
        hud.limpiar();
        hud.agregar(std::make_unique<Panel>(sf::FloatRect{{0.f,0.f},{1.f,0.16f}},sf::Color(0,0,0,140)));
        static const float CIEN = 100.f;
        hud.agregar(std::make_unique<Barra>("AIRE",&aire,&CIEN,COLOR_AIRE,sf::Vector2f{0.14f,0.04f},0.3f));
        hud.agregar(std::make_unique<Barra>("SED",&sed,&CIEN,COLOR_SED,sf::Vector2f{0.14f,0.08f},0.3f));
        hud.agregar(std::make_unique<Etiqueta>([this]{
                    int t = std::max(0,(int)std::ceil(tiempo));
                    return "Tiempo " + std::to_string(t/60) + ":" + (t%60 < 10 ? "0" : "") + std::to_string(t%60);
                    },sf::Vector2f{0.6f,0.03f},0.04f,COLOR_CONCENTRACION));
        hud.agregar(std::make_unique<Etiqueta>([this]{
                    return "Altura: " + std::to_string((int)((MUNDO_H - pies)/8.f)) + " / " + std::to_string((int)((MUNDO_H - META)/8.f)) + " m";
                    },sf::Vector2f{0.6f,0.09f},0.03f));
    }

    void Escena_SantaFe::reaparecer()
    {
        x = cp_x;
        pies = cp_pies;
        vy = 0.f;
        aire = 100.f;
        tiempo = std::max(cp_tiempo,40.f);
    }

    // --autopiloto: busca la siguiente plataforma de arriba y salta hacia ella
    void Escena_SantaFe::piloto()
    {
        control.izq = control.der = control.arr = false;
        // el objetivo solo cambia en el suelo: en el aire sigue al escalón que eligió
        auto& sig = objetivo;
        if(en_suelo || !sig)
        {
            sig = nullptr;
            for(auto& p : plataformas)
                if(p.y < pies - 4.f && (!sig || p.y > sig->y)) sig = &p;
        }
        if(!sig) return;
        float cx = std::clamp(x,sig->x + 2.f,sig->x + sig->w - 2.f);
        if(cx > x + 2.f) control.der = true;
        if(cx < x - 2.f) control.izq = true;
        if(en_suelo && std::abs(cx - x) < 8.f && aire > 6.f) control.arr = true; // salta desde el borde
    }

    void Escena_SantaFe::actualizar(float dt)
    {
        if(terminado) return;
        if(JuegoR12::prueba.autopiloto) piloto();
        aviso_t -= dt;
        tiempo -= dt;
        sed = std::min(100.f,sed + 0.6f*dt);
        float v = PASO*(sed > 70.f ? 0.8f : 1.f);
        x = std::clamp(x + ((control.der ? 1.f : 0.f) - (control.izq ? 1.f : 0.f))*v*dt,X0,X1);
        if(control.arr && en_suelo && aire >= 3.f) { vy = -SALTO; aire -= 3.f; en_suelo = false; }
        if(en_suelo) aire = std::min(100.f,aire + 12.f*dt);
        float antes = pies;
        vy += GRAVEDAD*dt;
        pies += vy*dt;
        en_suelo = false;
        if(pies >= MUNDO_H - 8.f) { pies = MUNDO_H - 8.f; vy = 0.f; en_suelo = true; }
        // plataformas de un solo sentido: se aterriza al bajar sobre ellas
        for(size_t k = 0; k < plataformas.size(); ++k)
        {
            const auto& p = plataformas[k];
            if(vy >= 0.f && antes <= p.y && pies >= p.y && x >= p.x - 4.f && x <= p.x + p.w + 4.f)
            {
                pies = p.y;
                vy = 0.f;
                en_suelo = true;
                if(std::find(puntos_agua.begin(),puntos_agua.end(),k) != puntos_agua.end() && cp_pies > p.y)
                {
                    cp_x = x; cp_pies = p.y; cp_tiempo = tiempo;
                    sed = 0.f;
                    aviso = "Punto de agua: sed a cero y punto de control";
                    aviso_t = 2.5f;
                    Eventos::Get().emitir(Evento::ITEM);
                }
            }
        }
        if(pies > cp_pies + 90.f) { aviso = "¡Caíste! Vuelves al último punto de agua"; aviso_t = 2.5f; reaparecer(); }
        if(tiempo <= 0.f) { aviso = "Se acabó el tiempo: vuelves al último punto de agua"; aviso_t = 2.5f; reaparecer(); }
        if(pies <= META)
        {
            terminado = true;
            auto& pa = Partida::Get();
            pa.mejorar(&Atributos::resistencia,1);
            pa.resumen = "Parque de skate de Santa Fe: llegaste a la cima";
            pa.historia().avanzar();
            irAPaso();
            return;
        }
        auto* c = diego->getComponente<CE::IControl>();
        *c = control;
        c->arr = c->abj = false;
        c->botones = 0;
        diego->setPosicion(x,pies - FabricaR12::PIES_A_CENTRO);
        diego->inputFSM();
        diego->onUpdate(dt);
        moverCamara(160.f,std::clamp(pies - 40.f,96.f,MUNDO_H - 96.f));
    }

    void Escena_SantaFe::dibujarMundo()
    {
        auto& r = CE::Render::Get();
        r.AddToDraw(mapa("04_skate_santa_fe"));
        for(size_t k = 0; k < plataformas.size(); ++k)
        {
            const auto& p = plataformas[k];
            sf::RectangleShape s{{p.w,std::max(4.f,std::min(p.h,8.f))}};
            s.setPosition({p.x,p.y});
            s.setFillColor(sf::Color(150,150,160));
            s.setOutlineColor(sf::Color(40,40,50));
            s.setOutlineThickness(1.f);
            r.AddToDraw(s);
            if(std::find(puntos_agua.begin(),puntos_agua.end(),k) != puntos_agua.end())
            {
                sf::Sprite g{textura("provisionales/sprites/garrafon.png")};
                g.setPosition({p.x + p.w - 18.f,p.y - 24.f});
                r.AddToDraw(g);
            }
        }
        r.AddToDraw(*diego);
    }

    void Escena_SantaFe::dibujarPantalla(sf::RenderTarget& t, sf::Vector2f tam)
    {
        if(aviso_t > 0.f)
        {
            auto a = texto(aviso,(unsigned)(tam.y*0.035f),COLOR_CONCENTRACION,{0.f,tam.y*0.2f});
            centrarX(a,tam.x/2.f);
            t.draw(a);
        }
        if(tiempo > santaFe(id).tiempo - 4.f)
        {
            auto a = texto("Sube hasta arriba: A/D moverse, W saltar (gasta aire)",(unsigned)(tam.y*0.03f),
                    sf::Color::White,{0.f,tam.y*0.9f});
            centrarX(a,tam.x/2.f);
            t.draw(a);
        }
    }
}
