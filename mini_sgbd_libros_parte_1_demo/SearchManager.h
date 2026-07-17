#ifndef SEARCH_MANAGER_H
#define SEARCH_MANAGER_H

#include "StorageManager.h"

#include <cstddef>
#include <string>
#include <vector>

enum class CampoBusqueda {
    Id,
    Titulo,
    Autor,
    Anio,
    Editorial
};

struct ResultadoBusqueda {
    std::vector<Libro> registros;
    std::size_t registrosLeidos = 0;
    bool utilizoIndice = false;
};

class SearchManager {
public:
    explicit SearchManager(StorageManager& storage);

    ResultadoBusqueda buscarPorId(int id);
    ResultadoBusqueda buscarPorID(int id);
    ResultadoBusqueda buscarPorTitulo(const std::string& valor);
    ResultadoBusqueda buscarPorAutor(const std::string& valor);
    ResultadoBusqueda buscarPorAnio(const std::string& valor);
    ResultadoBusqueda buscarPorEditorial(const std::string& valor);
    ResultadoBusqueda buscarParcial(CampoBusqueda campo, const std::string& valor);

private:
    StorageManager& storage;

    static bool contieneSinMayusculas(const std::string& texto, const std::string& patron);
};

#endif
