// Incluye el encabezado de esta misma clase: trae la declaración de Escena_Figuras
// y, en cadena, todos los includes del .hpp (Botones, Render, Alumno, iostream, memory, etc.)
#include "Escena_figuras.hpp"
// std::istringstream: permite tratar una cadena ya leída (un renglón del archivo) como si
// fuera un flujo de entrada, para ir extrayendo sus campos con el operador >>
#include <sstream>
// std::uint8_t: entero de 8 bits sin signo, el tipo que usa sf::Color para cada canal
#include <cstdint>

// Espacio de nombres del juego (IVJ), separa el código del juego del código del motor (CE)
namespace IVJ
{
    // Función auxiliar libre (no es método de la clase). 'static' la deja visible solo dentro
    // de este archivo .cpp, de modo que no choca con otros símbolos del programa.
    // Convierte los tres enteros leídos del archivo en un color de SFML
    static sf::Color aColor(int r, int g, int b)
    {
        // Cada canal de sf::Color es un std::uint8_t (0..255), por eso hay que convertir con
        // static_cast. No se leen directamente en std::uint8_t porque el operador >> trataría
        // ese tipo como un carácter y guardaría el código del dígito en vez del número
        return sf::Color{static_cast<std::uint8_t>(r),
                        static_cast<std::uint8_t>(g),
                        static_cast<std::uint8_t>(b)};
    }

    // Definición del constructor por defecto de Escena_Figuras
    Escena_Figuras::Escena_Figuras()
        // Lista de inicialización: construye la parte heredada (la clase base CE::Escena)
        // con su constructor por defecto antes de entrar al cuerpo del constructor
        :CE::Escena{}{
        // Cuerpo vacío: no hay nada más que inicializar aquí.
        // El miembro propio 'inicializar' ya se inicializa en el .hpp con {1}
    }


    // Método de inicialización de la escena. Lo llama el motor cuando la escena se carga.
    // 'override' en el .hpp indica que sobrescribe el método virtual puro de CE::Escena
    void Escena_Figuras::onInit()
    {   // Guarda de una sola ejecución: si 'inicializar' vale 0 (false), sale de inmediato.
        // Evita que el contenido de onInit se repita si el motor vuelve a llamar al método
        if(!inicializar) return;

        insertFiguras();

        // Baja la bandera a 0 (false) para que las siguientes llamadas a onInit()
        // se corten en el 'return' de arriba
        inicializar=false;
        // Al salir del método, 'al' se destruye (se llama su destructor) y
        // 'mi_apuntador' libera solo su memoria al perder la última referencia
    }

    void Escena_Figuras::insertFiguras(){
        // ASSETS es un macro definido en CMakeLists.txt con la ruta absoluta a assets/,
        // así la carga no depende del directorio desde el que se ejecute el programa
        std::ifstream archivo{ASSETS "/archive.txt"};
        if(!archivo.is_open())
        {
            std::cerr << "No se pudo abrir archive.txt\n";
            return;
        }

        // Cadena donde se copiará cada renglón completo del archivo
        std::string linea;
        // Contador de renglones: sirve para nombrar cada figura y para señalar dónde hubo un error
        int num_linea{0};

        // Lee el archivo renglón por renglón. getline se detiene en cada salto de línea y
        // devuelve el flujo, que al llegar al final del archivo se evalúa como falso y corta el ciclo
        while(std::getline(archivo,linea))
        {
            // Un renglón más leído; se cuenta aunque después se descarte
            ++num_linea;
            // Ignora renglones vacíos (por ejemplo un salto de línea suelto al final del archivo)
            if(linea.empty()) continue;

            // Envuelve el renglón en un flujo de solo lectura para poder extraer sus campos
            // con >>, que separa automáticamente por espacios
            std::istringstream campos{linea};

            // Primer campo del renglón: el nombre del tipo de figura ("Rectangulo", "Circulo", ...)
            std::string tipo;
            campos >> tipo;

            // Posición donde se colocará la figura dentro de la escena
            float x{0.f}, y{0.f};
            // Canales rojo, verde y azul del color de relleno
            int rr{0}, rg{0}, rb{0};
            // Canales rojo, verde y azul del color de contorno
            int cr{0}, cg{0}, cb{0};

            // Apuntador a la clase base. Aquí cabe cualquier figura concreta gracias al
            // polimorfismo: es la ventaja de que todas hereden de Figuras
            std::shared_ptr<Figuras> figura;

            // El rectángulo es el único que necesita dos medidas, por eso se trata aparte
            if(tipo=="Rectangulo")
            {
                // Ancho y alto propios del rectángulo
                float ancho{0.f}, alto{0.f};
                // Extrae los 10 números en el mismo orden en que están escritos en el archivo.
                // Si falta un campo o alguno no es numérico, el flujo entra en estado de error
                // y la condición se cumple, así que ese renglón se descarta
                if(!(campos >> ancho >> alto >> x >> y >> rr >> rg >> rb >> cr >> cg >> cb))
                {
                    std::cerr << "Linea " << num_linea << ": rectangulo incompleto, se ignora\n";
                    continue;
                }
                // Construye el rectángulo en el heap; make_shared devuelve el shared_ptr que
                // ya es dueño de esa memoria, de modo que no hay que liberarla a mano
                figura = std::make_shared<Rectangulo>(ancho,alto,aColor(rr,rg,rb),aColor(cr,cg,cb));
            }
            else
            {
                // Las demás figuras quedan definidas por el radio de la circunferencia que las
                // circunscribe, así que su renglón trae un número menos
                float radio{0.f};
                if(!(campos >> radio >> x >> y >> rr >> rg >> rb >> cr >> cg >> cb))
                {
                    std::cerr << "Linea " << num_linea << ": figura incompleta, se ignora\n";
                    continue;
                }
                // Se arman una sola vez los dos colores y se reutilizan en la rama que corresponda
                const sf::Color relleno{aColor(rr,rg,rb)};
                const sf::Color contorno{aColor(cr,cg,cb)};

                // Compara el texto leído contra cada tipo conocido para saber qué clase instanciar
                if(tipo=="Circulo")
                    figura = std::make_shared<Circulo>(radio,relleno,contorno);
                else if(tipo=="Triangulo")
                    figura = std::make_shared<Triangulo>(radio,relleno,contorno);
                else if(tipo=="Pentagono")
                    figura = std::make_shared<Pentagono>(radio,relleno,contorno);
                else if(tipo=="Hexagono")
                    figura = std::make_shared<Hexagono>(radio,relleno,contorno);
                else if(tipo=="Octagono")
                    figura = std::make_shared<Octagono>(radio,relleno,contorno);
                else
                {
                    // Ningún tipo coincidió: se avisa y se pasa al siguiente renglón
                    std::cerr << "Linea " << num_linea << ": tipo desconocido '" << tipo << "'\n";
                    continue;
                }
            }

            // Escribe la posición en el componente ITransform del objeto. Cada onUpdate de la
            // figura vuelca esa posición sobre su forma de SFML justo antes de dibujarla
            figura->setPosicion(x,y);
            // Renombra el objeto para distinguirlo en la lista de objetos del editor,
            // en lugar del "Objeto N" que le pone el constructor de CE::Objeto
            figura->getNombre()->nombre = tipo + " " + std::to_string(num_linea);
            // Guarda la figura en el pool heredado de CE::Escena. A partir de aquí el pool
            // comparte la propiedad del objeto y onUpdate/onRender ya lo recorren solos
            objetos.agregarPool(figura);
        }

        // Reporta cuántos objetos quedaron cargados, para verificar de un vistazo que el
        // archivo se leyó completo
        std::cout << "Figuras cargadas: " << objetos.getPool().size() << "\n";
    }

    // Método de limpieza de la escena. El motor lo llama al descargar o cambiar de escena.
    // Está vacío porque esta escena todavía no reserva recursos que haya que liberar a mano
    void Escena_Figuras::onFinal(){
    }

    // Actualización lógica de la escena, se llama una vez por frame.
    // 'dt' (delta time) es el tiempo en segundos transcurrido desde el frame anterior
    void Escena_Figuras::onUpdate(float dt){
        // Recorre con un for-range todos los objetos del pool heredado de CE::Escena.
        // 'auto&' toma cada elemento por referencia para no copiar los apuntadores
        for(auto& obj: objetos.getPool()){
            // Llama al update de cada objeto propagando el delta time, para que su
            // movimiento o animación sea independiente de los cuadros por segundo
            obj->onUpdate(dt);
        }
    }

    // Manejo de entradas (teclado / controles). El motor lo llama cuando ocurre una
    // acción registrada. 'accion' se recibe por referencia constante para no copiarla
    void Escena_Figuras::onInputs(const CE::Botones& accion){
        // Consulta qué tipo de acción llegó y ramifica según su valor
        switch(accion.getTipo())
        {
            // Caso: la tecla o botón se acaba de presionar
            case CE::Botones::TipoAccion::OnPress:{
                // Sin comportamiento asignado todavía; sale del switch
                break;
            }
            // Caso: la tecla o botón se acaba de soltar
            case CE::Botones::TipoAccion::OnRelease:{
                // Sin comportamiento asignado todavía; sale del switch
                break;
            }
            // Caso: no hay acción válida (estado neutro)
            case CE::Botones::TipoAccion::None:{
                // No hace nada; sale del switch
                break;
            }
        }
    }

    // Dibujado de la escena, se llama una vez por frame después de onUpdate
    void Escena_Figuras::onRender(){
        // Recorre por referencia todos los objetos del pool de la escena
        for(auto& obj: objetos.getPool())
            // CE::Render::Get() obtiene la instancia única (singleton) del renderizador,
            // y AddToDraw encola el objeto (desreferenciado con *) para dibujarlo este frame
            CE::Render::Get().AddToDraw(*obj);
    }
}
