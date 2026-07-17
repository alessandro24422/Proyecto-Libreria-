#ifndef SORT_MANAGER_H
#define SORT_MANAGER_H

#include "Libro.h"

#include <vector>

enum class CampoOrdenamiento {
    Id,
    Titulo,
    Autor,
    Anio
};

enum class DireccionOrdenamiento {
    Ascendente,
    Descendente
};

class SortManager {
public:
    void ordenar(std::vector<Libro>& libros,
                 CampoOrdenamiento campo,
                 DireccionOrdenamiento direccion) const;
};

#endif
