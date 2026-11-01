#pragma once
#include <functional>
#include <map>

namespace Round12
{
    enum class Evento { GOLPE_CONECTADO, GOLPE_FUERTE, BLOQUEO, CAIDA, CAMPANA, ITEM, NIVEL_COMPLETO };

    // Observador: el combate emite y el HUD, el audio y el público escuchan
    class Eventos
    {
        public:
            using Oyente = std::function<void(Evento,float)>;
            static Eventos& Get() { static Eventos e; return e; }
            int suscribir(Oyente o) { oyentes[++sig] = std::move(o); return sig; }
            void desuscribir(int id) { oyentes.erase(id); }
            void emitir(Evento e, float valor = 0.f) { for(auto& [id,o] : oyentes) o(e,valor); }
            void limpiar() { oyentes.clear(); }
        private:
            Eventos() = default;
            std::map<int,Oyente> oyentes;
            int sig{0};
    };
}
