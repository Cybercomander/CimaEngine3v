
#include "Escena_Init.hpp"
#include <Motor/Inputs/Botones.hpp>
#include <Motor/Primitivos/GestorAssets.hpp>
#include <Motor/Primitivos/GestorEscenas.hpp>
#include <Motor/Render/Render.hpp>
#include <Juego/Figuras/Figuras.hpp>
#include <Juego/objetos/Texto.hpp>
#include <Juego/Componentes/IJComponentes.hpp>
#include <Juego/Sistemas/Sistemas.hpp>
#include <Juego/Figuras/ClasePrueba.hpp>
#include <iostream>
#include <memory>

// Espacio de nombres del juego (IVJ), separa el código del juego del código del motor (CE)
namespace IVJ
{
    // Definición del constructor por defecto de Escena_Init
    Escena_Init::Escena_Init()
        // Lista de inicialización: construye la parte heredada (la clase base CE::Escena)
        // con su constructor por defecto antes de entrar al cuerpo del constructor
        :CE::Escena{}
    {
        // Cuerpo vacío: no hay nada más que inicializar aquí.
        // El miembro propio 'inicializar' ya se inicializa en el .hpp con {1}
    }

    // Método de inicialización de la escena. Lo llama el motor cuando la escena se carga.
    // 'override' en el .hpp indica que sobrescribe el método virtual puro de CE::Escena
    void Escena_Init::onInit()
    {
        // Guarda de una sola ejecución: si 'inicializar' vale 0 (false), sale de inmediato.
        // Evita que el contenido de onInit se repita si el motor vuelve a llamar al método
        if(!inicializar) return;

        // Crea un objeto Alumno en la pila (memoria automática) con inicialización uniforme {}.
        // Los argumentos corresponden al constructor Alumno(const char* nombre, long matricula, int semestre)
        Alumno al{"Omar", 30198, 6};
        // Consulta el nombre con el getter (devuelve una copia del std::string) y lo imprime en
        // consola seguido de un salto de línea, para verificar que el constructor guardó bien el dato
        std::cout << al.getNombre() << "\n";

        // Declaración de apuntador de tipo entero
        // Al construirse sin argumentos, el shared_ptr nace vacío (equivale a nullptr):
        // todavía no apunta a ningún entero y su contador de referencias no existe
        std::shared_ptr<int> mi_apuntador;
        // Inicializador del apuntador de tipo entero generado
        // make_shared reserva en el heap el entero y su bloque de control en una sola asignación,
        // inicializa el valor en 0 y devuelve el shared_ptr que toma la propiedad de esa memoria
        mi_apuntador = std::make_shared<int>();

        // Desreferencia el apuntador con '*' para leer el entero apuntado (0) y lo imprime.
        // Demuestra que el shared_ptr ya apunta a memoria válida y se usa como un apuntador normal
        std::cout << *mi_apuntador << "\n";

        // Baja la bandera a 0 (false) para que las siguientes llamadas a onInit()
        // se corten en el 'return' de arriba
        inicializar=false;
        // Al salir del método, 'al' se destruye (se llama su destructor) y
        // 'mi_apuntador' libera solo su memoria al perder la última referencia
    }

    // Método de limpieza de la escena. El motor lo llama al descargar o cambiar de escena.
    // Está vacío porque esta escena no reserva recursos que haya que liberar a mano:
    // todo lo que crea vive en la pila o en apuntadores inteligentes
    void Escena_Init::onFinal()
    {
    }

    // Actualización lógica de la escena, se llama una vez por frame.
    // 'dt' (delta time) es el tiempo en segundos transcurrido desde el frame anterior
    void Escena_Init::onUpdate(float dt)
    {
        // Recorre con un for-range todos los objetos del pool heredado de CE::Escena.
        // 'auto&' toma cada elemento por referencia para no copiar los apuntadores
        for(auto& obj: objetos.getPool())
        {
            // Llama al update de cada objeto propagando el delta time, para que su
            // movimiento o animación sea independiente de los cuadros por segundo
            obj->onUpdate(dt);
        }
    }

    // Manejo de entradas (teclado / controles). El motor lo llama cuando ocurre una
    // acción registrada. 'accion' se recibe por referencia constante para no copiarla
    void Escena_Init::onInputs(const CE::Botones& accion)
    {
        // Consulta qué tipo de acción llegó y ramifica según su valor
        switch(accion.getTipo())
        {
            // Caso: la tecla o botón se acaba de presionar
            case CE::Botones::TipoAccion::OnPress:
            {
                // Sin comportamiento asignado todavía; sale del switch
                break;
            }
            // Caso: la tecla o botón se acaba de soltar
            case CE::Botones::TipoAccion::OnRelease:
            {
                // Sin comportamiento asignado todavía; sale del switch
                break;
            }
            // Caso: no hay acción válida (estado neutro)
            case CE::Botones::TipoAccion::None:
            {
                // No hace nada; sale del switch
                break;
            }
        }
    }

    // Dibujado de la escena, se llama una vez por frame después de onUpdate
    void Escena_Init::onRender()
    {
        // Recorre por referencia todos los objetos del pool de la escena
        for(auto& obj: objetos.getPool())
            // CE::Render::Get() obtiene la instancia única (singleton) del renderizador,
            // y AddToDraw encola el objeto (desreferenciado con *) para dibujarlo este frame
            CE::Render::Get().AddToDraw(*obj);
    }
}
