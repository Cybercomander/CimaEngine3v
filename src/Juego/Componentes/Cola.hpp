#pragma once

namespace IVJ
{
// Capacidad por defecto de toda Cola: número máximo de elementos que puede guardar a la vez
inline constexpr int COLA_CAPACIDAD = 64;

// Cola (FIFO) circular de capacidad fija, hecha a mano sin std::queue.
// Los datos viven en un arreglo propio; 'inicio' señala al elemento más antiguo
// y 'fin' a la casilla libre donde entra el siguiente. Ambos índices dan la vuelta
// al llegar al final del arreglo (aritmética módulo CAPACIDAD), así ninguna operación
// necesita recorrer ni recorrer memoria: encolar y desencolar son O(1).
// Al ser un arreglo por valor, copiar una Cola produce una copia independiente.
template <typename T, int CAPACIDAD = COLA_CAPACIDAD> class Cola
{
    public:
    // Una cola nueva está vacía: ambos índices en cero y sin elementos
    Cola() : datos{}, inicio{0}, fin{0}, cantidad{0}
    {
    }

    // Coloca un elemento al final de la cola.
    // Devuelve false (y no guarda nada) cuando la cola ya está llena
    bool encolar(const T &dato)
    {
        if (estaLlena())
            return false;
        datos[fin] = dato;
        fin = (fin + 1) % CAPACIDAD;
        cantidad++;
        return true;
    }

    // Retira y devuelve el elemento más antiguo de la cola.
    // Precondición: la cola no está vacía (se consulta antes con estaVacia())
    T desencolar()
    {
        T dato = datos[inicio];
        inicio = (inicio + 1) % CAPACIDAD;
        cantidad--;
        return dato;
    }

    // Elemento más antiguo, sin retirarlo
    const T &frente() const
    {
        return datos[inicio];
    }

    // La cola está vacía cuando no guarda elementos
    bool estaVacia() const
    {
        return cantidad == 0;
    }

    // La cola está llena cuando ocupa todas las casillas del arreglo
    bool estaLlena() const
    {
        return cantidad == CAPACIDAD;
    }

    // Número de elementos guardados actualmente
    int getTamanio() const
    {
        return cantidad;
    }

    // Deja la cola vacía; las casillas se reutilizan en los siguientes encolar
    void vaciar()
    {
        inicio = 0;
        fin = 0;
        cantidad = 0;
    }

  private:
    // Almacenamiento fijo de los elementos
    T datos[CAPACIDAD];
    // Índice del elemento más antiguo (el próximo en salir)
    int inicio;
    // Índice de la casilla donde entra el próximo elemento
    int fin;
    // Elementos guardados actualmente
    int cantidad;
};
} // namespace IVJ
