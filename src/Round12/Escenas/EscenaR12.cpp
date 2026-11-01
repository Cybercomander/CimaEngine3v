#include "EscenaR12.hpp"
#include <Motor/Camaras/CamarasGestor.hpp>
#include <Juego/Maquinas/Boxeadores/NodosBoxeo.hpp>
#include <Round12/objetos/Audio.hpp>
#include <map>

namespace Round12
{
    namespace
    {
        using SC = sf::Keyboard::Scancode;
        // Comando: cada tecla del GDD es una acción con nombre
        const std::pair<SC,const char*> TECLAS[] = {
            {SC::W,"arriba"},{SC::Up,"arriba"},{SC::S,"abajo"},{SC::Down,"abajo"},
            {SC::A,"izquierda"},{SC::Left,"izquierda"},{SC::D,"derecha"},{SC::Right,"derecha"},
            {SC::J,"jab"},{SC::K,"recto"},{SC::L,"gancho"},{SC::I,"uppercut"},
            {SC::LShift,"guardia"},{SC::RShift,"guardia"},{SC::Q,"esquiva_izq"},{SC::E,"esquiva_der"},
            {SC::Escape,"pausa"},{SC::Enter,"aceptar"},{SC::Space,"aceptar"}};
        const std::map<std::string,std::uint32_t> BITS = {
            {"jab",IVJ::BotonBox::JAB},{"recto",IVJ::BotonBox::RECTO},
            {"gancho",IVJ::BotonBox::GANCHO},{"uppercut",IVJ::BotonBox::UPPERCUT},
            {"guardia",IVJ::BotonBox::GUARDIA},
            {"esquiva_izq",IVJ::BotonBox::ESQ_IZQ},{"esquiva_der",IVJ::BotonBox::ESQ_DER}};
    }

    void EscenaR12::onInit()
    {
        if(!cargada)
        {
            for(auto& [tecla,accion] : TECLAS)
                registrarBotones(tecla,accion);
            id_camara = 0;
            cargar();
            cargada = true;
        }
        if(pendiente)
        {
            soltarControles();
            reiniciar();
            pendiente = false;
        }
        CE::GestorCamaras::Get().setCamaraActiva(id_camara);
        Audio::musica(musica());
    }

    void EscenaR12::onFinal()
    {
        Audio::detener();
        soltarControles();
        CE::GestorCamaras::Get().setCamaraActiva(0);
    }

    void EscenaR12::soltarControles()
    {
        control.arr = control.abj = control.izq = control.der = false;
        control.botones = 0;
    }

    void EscenaR12::usarCamara(float w, float h)
    {
        auto& g = CE::GestorCamaras::Get();
        g.agregarCamara(std::make_shared<CE::Camara>(CE::Vector2D{w/2.f,h/2.f},CE::Vector2D{w,h}));
        id_camara = (int)g.getListaCamaras().size()-1;
        g.getCamara(id_camara).getTransformada().posicion = CE::Vector2D{w/2.f,h/2.f};
    }

    void EscenaR12::moverCamara(float x, float y)
    {
        if(id_camara) CE::GestorCamaras::Get().getCamara(id_camara).getTransformada().posicion = CE::Vector2D{x,y};
    }

    void EscenaR12::onUpdate(float dt)
    {
        actualizar(dt);
    }

    void EscenaR12::onInputs(const CE::Botones& accion)
    {
        if(accion.getTipo() == CE::Botones::TipoAccion::None) return;
        bool p = accion.getTipo() == CE::Botones::TipoAccion::OnPress;
        const auto n = accion.getNombre();
        if(n == "arriba")    control.arr = p;
        if(n == "abajo")     control.abj = p;
        if(n == "izquierda") control.izq = p;
        if(n == "derecha")   control.der = p;
        auto b = BITS.find(n);
        if(b != BITS.end())
            control.botones = p ? (control.botones | b->second) : (control.botones & ~b->second);
        alPresionar(n,p);
    }

    void EscenaR12::onRender()
    {
        dibujarMundo();
        EnPantalla pantalla;
        hud.dibujar(pantalla.rt,pantalla.tam);
        dibujarPantalla(pantalla.rt,pantalla.tam);
    }
}
