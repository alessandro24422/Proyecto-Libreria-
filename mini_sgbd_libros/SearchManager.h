#ifndef SEARCH_MANAGER_H
#define SEARCH_MANAGER_H

#include "StorageManager.h"

#include <chrono>
#include <optional>
#include <string>
#include <vector>

// Resultado de una consulta y el tiempo que tomó ejecutarla. El tiempo permite
// comparar el acceso por índice con un recorrido secuencial.
struct ResultadoConsulta {
    std::vector<Libro> libros;
    std::chrono::nanoseconds duracion{0};
    std::string mensaje;
};

struct ComparacionBusquedaId {
    std::optional<Libro> resultadoIndice;
    std::optional<Libro> resultadoLineal;
    std::chrono::nanoseconds tiempoIndice{0};
    std::chrono::nanoseconds tiempoLineal{0};
};

class SearchManager {
public:
    explicit SearchManager(StorageManager& storage);

    // La búsqueda por ID delega en el índice hash persistente de StorageManager.
    std::optional<Libro> buscarPorID(int id) const;
    std::vector<Libro> buscarPorTitulo(const std::string& titulo) const;
    std::vector<Libro> buscarPorAutor(const std::string& autor) const;
    std::vector<Libro> buscarPorGenero(const std::string& genero) const;
    std::vector<Libro> buscarPorAnio(int anio) const;

    // SELECT propio: SELECT * [WHERE campo (=|CONTAINS) valor]
    //                 [ORDER BY titulo|autor|anio [ASC|DESC]]
    ResultadoConsulta ejecutarConsulta(const std::string& consulta) const;
    ComparacionBusquedaId compararBusquedaPorID(int id) const;

private:
    StorageManager& storage;

    std::optional<Libro> buscarPorIDLineal(int id) const;
    static bool contieneSinMayusculas(const std::string& texto, const std::string& termino);
    static std::string limpiar(const std::string& texto);
    static std::string mayusculas(const std::string& texto);
};

#endif
