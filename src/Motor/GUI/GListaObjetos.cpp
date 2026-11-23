#include "GListaObjetos.hpp"
#include "../Camaras/CamarasGestor.hpp"
#include "../Primitivos/GestorEscenas.hpp"
#include <imgui.h>
#include <string>
#include <imgui-SFML.h>

namespace CE
{
    void GListaObjetos::OnInit(const MotorConfig& des)
    {
        propiedades.OnInit(des);
    }
    void GListaObjetos::OnUpdate(float dt)
    {
        propiedades.OnUpdate(dt);
    }
    void GListaObjetos::OnRender(void)
    {
        ImGui::Begin("Objetos",nullptr,0);
        
        // solo escenas del mismo juego (prefijo antes de '_', p. ej. R12_): no se cruzan proyectos
        auto grupo = [](const std::string& k){ auto i = k.find('_'); return i == std::string::npos ? std::string{} : k.substr(0,i); };
        const std::string actual = GestorEscenas::Get().getEscenaActual().nombre;
        if(ImGui::BeginCombo("Escenas",actual.c_str(),0))
        {
            for(const auto& k : GestorEscenas::Get().getKeys())
            {
                if(grupo(k) != grupo(actual)) continue;
                const bool seleccion = (k == actual);
                // cambia solo al elegir, no cada frame que el combo está abierto
                if(ImGui::Selectable(k.c_str(),seleccion) && !seleccion)
                    GestorEscenas::Get().cambiarEscena(k);
                if(seleccion)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Spacing();
        if(ImGui::TreeNode("Camaras"))
        {
            auto camaras = GestorCamaras::Get().getListaCamaras();
            for(int i=0; i<(int)camaras.size();i++)
            {
                auto cam = camaras[i];
                auto strCam = cam->nombre;
                //color del boton
                if(cam->esta_activa)
                {
                    ImGui::PushStyleColor(ImGuiCol_Button,0xff00A5FF);
                    ImGui::PushStyleColor(ImGuiCol_Text,0xff000000);
                }
                else
                {
                    ImGui::PushStyleColor(ImGuiCol_Button,0xffffbf00);
                    ImGui::PushStyleColor(ImGuiCol_Text,0xff000000);
                }

                bool boton = ImGui::Button(strCam.c_str(),ImVec2(150,30));
                if(boton)
                {
                    GestorCamaras::Get().setCamaraActiva(i);
                }
                ImGui::PopStyleColor(2);
            }
            ImGui::TreePop();
            ImGui::Spacing();
        }

        if(ImGui::TreeNode("TileMap_Layers"))
        {
            auto &layers = GestorEscenas::Get().getEscenaActual().getTileMapsLayers();
            for(int i=0;i< (int)layers.size();++i)
            {
                std::string id_str = "tile_map "+std::to_string(i+1);
                if(ImGui::Button(id_str.c_str() ) )
                {
                    propiedades.seleccionarTileMap(&layers[i]);
                }
            }
            ImGui::TreePop();
            ImGui::Spacing();
        }

        if(ImGui::TreeNode("Objetos"))
        { 
            //jugador siempre primero
            auto jugador = GestorEscenas::Get().getEscenaActual().getJugador();
            if(jugador)
            {
                if(ImGui::Button(jugador->toString().c_str()))
                {
                    GestorCamaras::Get().getCamaraActiva().lockEnObjeto(jugador);
                    propiedades.seleccionarObjeto(jugador.get());
                }
            }

            auto lista = GestorEscenas::Get().getEscenaActual().getPool();
            for(auto &obj : lista.getPool())
            {
                if(ImGui::Button(obj->toString().c_str()))
                {
                    GestorCamaras::Get().getCamaraActiva().lockEnObjeto(obj);
                    propiedades.seleccionarObjeto(obj.get());
                }

            }
            ImGui::TreePop();
        }
        ImGui::End();

        propiedades.OnRender();

    }
}
