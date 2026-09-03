#include "ClasePrueba.hpp"

namespace IVJ {
    // Todo este bloque es la implementación del constructor en 2 partes //
    Alumno::Alumno(const char *const nombre, long matricula, int semestre) // Este es el pedido de argumentos que requiere
        : nombre{nombre}, matricula{matricula}, semestre{semestre} // Esta es la entrada de los argumentos que se requieren
    {
        std::cout << "Se creó el objeto: " << nombre << "\n"; // Este es el contenido (método) que ejecuta el constructor
    }

    Alumno::~Alumno() {
        std::cout << "Se destruyo el objeto: " << nombre << "\n";
    }



}