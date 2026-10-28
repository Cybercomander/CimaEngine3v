#include "Animador.hpp"
#include <Juego/objetos/Entidad.hpp>
#include <utility>

namespace IVJ
{
    Animador::Animador(std::vector<int> frames, float frame_rate, bool ciclo, float rebote, int fila)
        :frames{std::move(frames)},max_tiempo{frame_rate},act_tiempo{frame_rate},
        ciclo{ciclo},rebote{rebote},fila{fila}
    {
    }

    void Animador::setFila(int f)
    {
        if(f == fila) return;
        fila = f;
        if(sprite) pintarFrame();
    }

    void Animador::iniciar(const Entidad& obj)
    {
        sprite = obj.getComponente<CE::ISprite>();
        id_frame = 0;
        fin = false;
        act_tiempo = max_tiempo;
        //pintar de inmediato para no mostrar un frame del nodo anterior
        pintarFrame();
    }

    void Animador::actualizar(float dt)
    {
        if(!sprite) return;
        act_tiempo -= dt;
        if(act_tiempo <= 0)
        {
            act_tiempo = max_tiempo;
            if(id_frame+1 < (int)frames.size())
                id_frame++;
            else if(ciclo)
                id_frame = 0;
            else
                fin = true;
            pintarFrame();
        }
        //Entidad::onUpdate recoloca el sprite cada frame antes del FSM,
        //así que el desplazamiento no se acumula
        if(rebote > 0.f && id_frame%2)
            sprite->m_sprite.move({0.f,-rebote});
    }

    void Animador::mirar(bool izquierda)
    {
        if(!sprite) return;
        float e = sprite->escala;
        sprite->m_sprite.setScale({izquierda ? -e : e, e});
    }

    void Animador::pintarFrame()
    {
        sprite->m_sprite.setTextureRect(sf::IntRect{
                {sprite->width*frames[id_frame],sprite->height*fila},
                {sprite->width,sprite->height}});
    }
}
