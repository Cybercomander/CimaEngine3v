#pragma once
#include <iostream>
#include <string>
#include <memory>

namespace IVJ {
    class Alumno {
        // Constructor público
        public:
            explicit Alumno(const char * const nombre, long matricula, int semestre);
            // Destructor se utiliza para destruir el objeto creado con el constructor
            ~Alumno();

        // GETTERS Y SETTERS
        std::string getNombre() const {
            return nombre;
        };

        long getMatricula() const {
            return matricula;
        };

        int getSemestre() const {
            return semestre;
        };

        void setNombre(const char * const nombre);
        void setMatricula(long matricula);
        void setSemestre(int semestre);


        // Constructor privado
        private :
            std::string nombre;
            long matricula;
            int semestre;

    };
}