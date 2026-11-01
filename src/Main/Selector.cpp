#include "Selector.hpp"
#include <Motor/Camaras/CamarasGestor.hpp>
#include <Motor/Primitivos/GestorEscenas.hpp>
#include <Round12/objetos/Audio.hpp>
#include <imgui.h>
#include <iostream>

namespace
{
    bool esR12(const std::string& clave) { return clave.rfind("R12_",0) == 0; }
}

Selector::Selector(Modo inicial, int ciclos_prueba)
    :CE::GameLayer{},labs{std::make_unique<IVJ::Juego>()},
    r12{std::make_unique<Round12::JuegoR12>()},inicial{inicial},ciclos{ciclos_prueba}
{
    termino = false;
}

void Selector::OnInit()
{
    // orden fijo: Labs usa los índices de cámara 1-4, así que va primero
    labs->OnInit();
    auto& cams = CE::GestorCamaras::Get().getListaCamaras();
    for(size_t i = 1; i < cams.size() && i < 5; ++i)
        camaras_labs.push_back(cams[i]);
    r12->OnInit();
    auto& g = CE::GestorEscenas::Get();
    n_escenas = g.getKeys().size();
    for(auto& k : g.getKeys())
        (esR12(k) ? claves_r12 : claves_labs).push_back(k);
    cambiar(ciclos > 0 ? Modo::LABS : inicial);
}

bool Selector::EstaCorriendo()
{
    return !termino && labs->EstaCorriendo() && r12->EstaCorriendo();
}

CE::GameLayer* Selector::activo() const
{
    if(modo == Modo::LABS) return labs.get();
    if(modo == Modo::ROUND12) return r12.get();
    return nullptr;
}

void Selector::cambiar(Modo m)
{
    if(m == modo) return;
    auto& g = CE::GestorEscenas::Get();
    if(modo == Modo::ROUND12) r12->desactivar();
    if(modo == Modo::LABS)
    {
        soltarLabs();
        // el combo de escenas de Debug puede dejar una escena R12: no se guarda como de Labs
        if(!esR12(g.getEscenaActual().nombre)) escena_labs = g.getEscenaActual().nombre;
    }
    if(modo != Modo::LANZADOR) previo = modo;
    modo = m;
    if(modo == Modo::LABS && g.getEscenaActual().nombre != escena_labs)
        g.cambiarEscena(escena_labs);
    if(modo == Modo::ROUND12)
        r12->activar();
}

// sin esto, soltar una tecla en el lanzador deja al jugador de Labs caminando solo
void Selector::soltarLabs()
{
    CE::InputBuffer sueltas{32};
    for(int t : teclas_labs.soltarTodas())
    {
        std::optional<sf::Event> ev = sf::Event{sf::Event::KeyReleased{sf::Keyboard::Key::Unknown,
            (sf::Keyboard::Scancode)t,false,false,false,false}};
        sueltas.pushData(ev);
    }
    labs->OnInputs(sueltas);
}

void Selector::OnInputs(std::optional<sf::Event>& eventos)
{
    if(eventos && eventos->getIf<sf::Event::Closed>())
        termino = true;
}

void Selector::OnInputs(CE::InputBuffer& buffer)
{
    bool cambio = false;
    while(!buffer.estaVacio())
    {
        auto& ev = buffer.popData();
        if(!ev) continue;
        if(ev->getIf<sf::Event::Closed>()) { termino = true; continue; }
        // lo que llega después de cambiar de juego en el mismo frame se descarta
        if(cambio) continue;
        if(const auto* p = ev->getIf<sf::Event::KeyPressed>())
        {
            if(p->scancode == sf::Keyboard::Scancode::F1)
            {
                if(modo == Modo::LANZADOR && previo != Modo::LANZADOR) cambiar(previo);
                else cambiar(Modo::LANZADOR);
                cambio = true;
                continue;
            }
            if(modo == Modo::LANZADOR)
            {
                if(p->scancode == sf::Keyboard::Scancode::Num1 || p->scancode == sf::Keyboard::Scancode::Numpad1)
                { cambiar(Modo::LABS); cambio = true; }
                if(p->scancode == sf::Keyboard::Scancode::Num2 || p->scancode == sf::Keyboard::Scancode::Numpad2)
                { cambiar(Modo::ROUND12); cambio = true; }
                continue;
            }
        }
        if(modo == Modo::LABS)
        {
            if(const auto* p = ev->getIf<sf::Event::KeyPressed>()) teclas_labs.presionar((int)p->scancode);
            if(const auto* r = ev->getIf<sf::Event::KeyReleased>()) teclas_labs.soltar((int)r->scancode);
        }
        if(modo != Modo::LANZADOR)
            filtrado.pushData(ev);
    }
    if(auto* a = activo())
        a->OnInputs(filtrado);
    filtrado.clearData();
}

void Selector::OnUpdate(float dt)
{
    if(ciclos > 0) pasoPrueba();
    if(auto* a = activo())
        a->OnUpdate(dt);
    if(r12->pideLanzador())
    {
        r12->limpiarPeticion();
        cambiar(Modo::LANZADOR);
    }
}

void Selector::OnRender()
{
    if(auto* a = activo())
        a->OnRender();
    menu();
}

void Selector::menu()
{
    if(ImGui::BeginMainMenuBar())
    {
        if(ImGui::BeginMenu("Juego"))
        {
            if(ImGui::MenuItem("Lanzador","F1",modo == Modo::LANZADOR)) cambiar(Modo::LANZADOR);
            if(ImGui::MenuItem("CimaEngine Labs","1",modo == Modo::LABS)) cambiar(Modo::LABS);
            if(ImGui::MenuItem("ROUND 12","2",modo == Modo::ROUND12)) cambiar(Modo::ROUND12);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    if(modo != Modo::LANZADOR) return;
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos({vp->WorkPos.x + vp->WorkSize.x*0.5f, vp->WorkPos.y + vp->WorkSize.y*0.5f},
            ImGuiCond_Always, {0.5f,0.5f});
    ImGui::Begin("Elige juego",nullptr,ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking);
    if(ImGui::Button("1  CimaEngine Labs",{260,60})) cambiar(Modo::LABS);
    ImGui::SameLine();
    if(ImGui::Button("2  ROUND 12",{260,60})) cambiar(Modo::ROUND12);
    ImGui::TextDisabled("Teclas 1 / 2 para elegir. F1 regresa aquí desde cualquier juego.");
    ImGui::End();
}

// cada 30 frames cambia de juego y de escena; tras cada cambio revisa que
// el estado compartido (escenas, cámaras, audio) no se haya mezclado
void Selector::pasoPrueba()
{
    if(++frames % 30) return;
    if(cambios >= ciclos*2)
    {
        std::cout<<"[prueba-selector] "<<cambios<<" cambios, "<<n_fallas<<" fallas\n";
        termino = true;
        return;
    }
    auto& g = CE::GestorEscenas::Get();
    int vuelta = cambios/2;
    if(modo == Modo::LABS)
    {
        // D sostenida en Labs al cambiar: no debe quedarse pegada
        CE::InputBuffer b{4};
        std::optional<sf::Event> d = sf::Event{sf::Event::KeyPressed{sf::Keyboard::Key::D,
            sf::Keyboard::Scancode::D,false,false,false,false}};
        b.pushData(d);
        OnInputs(b);
        cambiar(Modo::ROUND12);
        if(!teclas_labs.soltarTodas().empty()) falla("una tecla de Labs quedó pegada al cambiar de juego");
        g.cambiarEscena(claves_r12[vuelta % claves_r12.size()]);
    }
    else
    {
        cambiar(Modo::LABS);
        g.cambiarEscena(claves_labs[vuelta % claves_labs.size()]);
    }
    cambios++;
    verificar();
}

void Selector::verificar()
{
    auto& g = CE::GestorEscenas::Get();
    auto actual = g.getEscenaActual().nombre;
    if(esR12(actual) != (modo == Modo::ROUND12))
        falla("escena '"+actual+"' no pertenece al juego activo");
    auto& cams = CE::GestorCamaras::Get().getListaCamaras();
    for(size_t i = 0; i < camaras_labs.size(); ++i)
        if(i+1 >= cams.size() || cams[i+1] != camaras_labs[i])
            falla("la cámara "+std::to_string(i+1)+" de Labs cambió de lugar");
    if(modo == Modo::LABS && Round12::Audio::sonando())
        falla("la música de ROUND 12 sigue sonando en Labs");
    if(g.getKeys().size() != n_escenas)
        falla("cambió el número de escenas registradas");
}

void Selector::falla(const std::string& motivo)
{
    std::cout<<"[prueba-selector] FALLA: "<<motivo<<"\n";
    n_fallas++;
}
