#ifndef SORT_MANAGER_H
#define SORT_MANAGER_H

#include "Libro.h"

#include <vector>

class SortManager {
public:
    static std::vector<Libro> ordenarPorTitulo(std::vector<Libro> libros,
                                                bool ascendente = true);
    static std::vector<Libro> ordenarPorAutor(std::vector<Libro> libros,
                                               bool ascendente = true);
    static std::vector<Libro> ordenarPorAnio(std::vector<Libro> libros,
                                              bool ascendente = true);
};

#endif
