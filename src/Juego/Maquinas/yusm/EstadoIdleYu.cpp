#include "EstadoIdleYu.hpp"
#include "EstadoMoverYu.hpp"
#include <Motor/Render/Render.hpp>

namespace IVJ
{
    EstadoIdleYu::EstadoIdleYu(const CE::Vector2D &box,int frames,float frame_rate)
        :FSM{},dim_box{box},sprite{nullptr},frames{frames},
        id_frame{0},frame_rate{frame_rate},frame_actual{frame_rate}
    {
        nombre = "YU_IDLE";
    }

    FSM *EstadoIdleYu::onInputs(const CE::IControl &control) {

        //cambiar de estado a mover dependiendo que presione
        //la hoja mira a la derecha, a la izquierda se voltea (flip)
        if(control.der)
            return new EstadoMoverYu(CE::Vector2D{348.f,425.f},12,0.08f,false);
        if(control.izq)
            return new EstadoMoverYu(CE::Vector2D{348.f,425.f},12,0.08f,true);
        return nullptr;
    }

    void EstadoIdleYu::onEntrar(const Entidad &obj) {
        sprite = &obj.getComponente<CE::ISprite>()->m_sprite;
        id_frame=0;
        padding = 5.f;
        if(obj.tieneComponente<CE::IShader>()) {
            shader = obj.getComponente<CE::IShader>();
            //static: el IShader guarda la dirección del valor (la GUI la edita en vivo)
            static sf::Glsl::Vec3 glowColor{1.0f,1.f,0.f};
            static float glowRadius = 10.f;
            static float rimStrength = 1.f;

            shader->m_shader.setUniform("texture",sf::Shader::CurrentTexture);
            shader->setVector3("glowColor",&glowColor);
            shader->setEscalar("glowRadius",&glowRadius);
            shader->setEscalar("rimStrength",&rimStrength);
        }
        else
            shader = nullptr;
    }

    void EstadoIdleYu::onSalir(const Entidad &obj) {
        (void)obj;
        shader = nullptr;
    }

    void EstadoIdleYu::onUpdate(const Entidad &obj, float dt) {
        (void)obj;
        static float time = 0;
        time +=dt;
        if(shader)
        {
            static sf::Vector2f texsize{sprite->getTexture().getSize()};
            //este es el recuadro que se pinta en coordenadas de textura [0,1]
            static sf::Glsl::Vec4 frameRect{0.f,0.f ,1.f,1.f};
            static sf::Glsl::Vec2 texel{1.f/texsize.x,1.f/texsize.y};
            //fuerza del brillo
            static float glowStrength = 2.f;
            //enviamos los valores al GPU
            shader->setEscalar("time",&time);
            shader->setEscalar("glowStrength",&glowStrength);
            shader->setVector2("texel",&texel);
            shader->setVector4("frameRect",&frameRect);
        }
        frame_actual -= dt;

        //animación
        if(frame_actual <=0)
        {
            int dx = static_cast<int>(dim_box.x+padding);
            int dy = static_cast<int>(dim_box.y+padding);
            sprite->setTextureRect(
                    sf::IntRect{
                            {//posición
                                    dx*id_frame,dy*0
                            },
                            {//tamaño
                                    static_cast<int>(dim_box.x),static_cast<int>(dim_box.y)
                            }
                    });
            id_frame  = (id_frame+1)%frames;
            frame_actual= frame_rate;
        }
    }
}
