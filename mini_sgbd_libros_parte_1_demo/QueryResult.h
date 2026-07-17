#ifndef QUERY_RESULT_H
#define QUERY_RESULT_H

#include "Libro.h"

#include <chrono>
#include <cstddef>
#include <string>
#include <vector>

struct QueryResult {
    std::vector<Libro> registros;
    std::chrono::microseconds tiempoEjecucion{0};
    std::size_t registrosLeidos = 0;
    std::size_t registrosEncontrados = 0;
    bool utilizoIndice = false;
    std::string consultaEjecutada;

    double tiempoEnMilisegundos() const {
        return static_cast<double>(tiempoEjecucion.count()) / 1000.0;
    }
};

#endif
