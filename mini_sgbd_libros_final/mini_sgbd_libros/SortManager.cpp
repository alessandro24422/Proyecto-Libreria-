#include "SortManager.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace {
std::string clave(const char* valor) {
    std::string resultado(valor);
    std::transform(resultado.begin(), resultado.end(), resultado.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return resultado;
}

template <typename Comparador>
std::vector<Libro> ordenar(std::vector<Libro> libros, Comparador comparador, bool ascendente) {
    std::sort(libros.begin(), libros.end(), [comparador, ascendente](const Libro& a, const Libro& b) {
        if (comparador(a, b)) return ascendente;
        if (comparador(b, a)) return !ascendente;
        return ascendente ? a.id < b.id : a.id > b.id;
    });
    return libros;
}
} // namespace

std::vector<Libro> SortManager::ordenarPorTitulo(std::vector<Libro> libros, bool ascendente) {
    return ordenar(std::move(libros), [](const Libro& a, const Libro& b) {
        return clave(a.titulo) < clave(b.titulo);
    }, ascendente);
}

std::vector<Libro> SortManager::ordenarPorAutor(std::vector<Libro> libros, bool ascendente) {
    return ordenar(std::move(libros), [](const Libro& a, const Libro& b) {
        return clave(a.autor) < clave(b.autor);
    }, ascendente);
}

std::vector<Libro> SortManager::ordenarPorAnio(std::vector<Libro> libros, bool ascendente) {
    return ordenar(std::move(libros), [](const Libro& a, const Libro& b) {
        return a.anio < b.anio;
    }, ascendente);
}
