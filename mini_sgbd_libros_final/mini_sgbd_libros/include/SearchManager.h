#pragma once

#include "StorageManager.h"

#include <chrono>
#include <optional>
#include <string>
#include <vector>

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

// Centraliza búsquedas y el pequeño lenguaje SELECT del proyecto.
class SearchManager {
public:
    explicit SearchManager(StorageManager& storage);

    std::optional<Libro> buscarPorID(int id);
    std::vector<Libro> buscarPorTitulo(const std::string& titulo);
    std::vector<Libro> buscarPorAutor(const std::string& autor);
    std::vector<Libro> buscarPorCodigoDewey(const std::string& codigoDewey);
    std::vector<Libro> buscarPorAnio(int anio);

    // SELECT * [WHERE campo (=|CONTAINS) valor]
    //          [ORDER BY titulo|autor|anio [ASC|DESC]]
    // Campos WHERE: id, titulo, autor, codigoDewey/dewey y anio.
    ResultadoConsulta ejecutarConsulta(const std::string& consulta);
    ComparacionBusquedaId compararBusquedaPorID(int id);

private:
    StorageManager& storage;

    std::optional<Libro> buscarPorIDLineal(int id);
    static bool contieneSinMayusculas(const std::string& texto, const std::string& termino);
    static std::string limpiar(const std::string& texto);
    static std::string mayusculas(const std::string& texto);
};
