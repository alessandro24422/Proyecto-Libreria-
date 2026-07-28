#pragma once

#include "Libro.h"

#include <vector>

// Ordenamientos de la vista de libros. No modifican los registros persistidos.
class SortManager {
public:
    static std::vector<Libro> ordenarPorTitulo(std::vector<Libro> libros, bool ascendente = true);
    static std::vector<Libro> ordenarPorAutor(std::vector<Libro> libros, bool ascendente = true);
    static std::vector<Libro> ordenarPorAnio(std::vector<Libro> libros, bool ascendente = true);
};
