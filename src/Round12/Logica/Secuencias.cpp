#include "Partida.hpp"

// Orden de los pasos (los ids están en Datos/Niveles.cpp y Datos/dialogos/)
namespace Round12
{
    namespace
    {
        Paso cine(const char* id)  { return {TipoPaso::CINEMATICA,id}; }
        Paso camino(const char* id){ return {TipoPaso::RECORRIDO,id}; }
        Paso pelea(const char* id) { return {TipoPaso::PELEA,id}; }
        Paso correr(const char* id){ return {TipoPaso::RUNNER,id}; }
        Paso subir(const char* id) { return {TipoPaso::SANTA_FE,id}; }
        Paso resultados()          { return {TipoPaso::RESULTADOS,"resultados"}; }
        Paso fin()                 { return {TipoPaso::FIN,"fin"}; }
    }

    // Demo del GDD: Sanchez Boxing GYM -> Playa de Rosarito -> Mexicali contra Javier
    std::vector<Paso> secuenciaDemo()
    {
        return {
            cine("prologo"), camino("gym_tutorial"), pelea("sparring_brandon"), resultados(),
            cine("playa_intro"), correr("playa"), resultados(),
            cine("mexicali_llegada"), camino("pasillo_mexicali"), pelea("javier"), resultados(),
            cine("ricardo_observa"), fin()};
    }

    // Historia completa: capítulos 1 a 4, epílogo y la segunda mitad hasta la final
    std::vector<Paso> secuenciaHistoria()
    {
        return {
            cine("prologo"), camino("gym_tutorial"), resultados(),
            cine("cap1"), subir("santa_fe"), correr("playa"), correr("circuito"),
            pelea("sparring_ninos"), resultados(),
            cine("cap2"), pelea("sparring_brandon"), pelea("torneo_local_1"), pelea("torneo_local_2"), resultados(),
            cine("cap3"), camino("pasillo_mexicali"), pelea("javier"), pelea("kevin"), resultados(),
            cine("cap4"), pelea("semifinal_ricardo"), cine("epilogo"),
            cine("segunda_mitad"), subir("santa_fe_dif2"), correr("playa_dif2"), correr("circuito_dif2"),
            pelea("revancha_javier"), pelea("revancha_kevin"), resultados(),
            cine("final_previa"), pelea("final_ricardo"), cine("creditos"), fin()};
    }
}
