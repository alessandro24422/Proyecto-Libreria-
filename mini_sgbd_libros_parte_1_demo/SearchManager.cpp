#include "SearchManager.h"

#include <algorithm>
#include <cctype>

SearchManager::SearchManager(StorageManager& storage) : storage(storage) {
}

ResultadoBusqueda SearchManager::buscarPorId(int id) {
    ResultadoBusqueda resultado;
    resultado.utilizoIndice = true;

    const auto libro = storage.buscarPorID(id);
    // Una consulta por hash consulta a lo sumo el registro localizado.
    resultado.registrosLeidos = libro.has_value() ? 1 : 0;
    if (libro.has_value()) {
        resultado.registros.push_back(*libro);
    }
    return resultado;
}

ResultadoBusqueda SearchManager::buscarPorID(int id) {
    return buscarPorId(id);
}

ResultadoBusqueda SearchManager::buscarPorTitulo(const std::string& valor) {
    return buscarParcial(CampoBusqueda::Titulo, valor);
}

ResultadoBusqueda SearchManager::buscarPorAutor(const std::string& valor) {
    return buscarParcial(CampoBusqueda::Autor, valor);
}

ResultadoBusqueda SearchManager::buscarPorAnio(const std::string& valor) {
    return buscarParcial(CampoBusqueda::Anio, valor);
}

ResultadoBusqueda SearchManager::buscarPorEditorial(const std::string& valor) {
    return buscarParcial(CampoBusqueda::Editorial, valor);
}

ResultadoBusqueda SearchManager::buscarParcial(CampoBusqueda campo, const std::string& valor) {
    ResultadoBusqueda resultado;
    const std::vector<Libro> libros = storage.listarLibros();
    resultado.registrosLeidos = libros.size();

    for (const Libro& libro : libros) {
        std::string campoActual;
        switch (campo) {
            case CampoBusqueda::Titulo:    campoActual = libro.titulo; break;
            case CampoBusqueda::Autor:     campoActual = libro.autor; break;
            case CampoBusqueda::Anio:      campoActual = std::to_string(libro.anio); break;
            case CampoBusqueda::Editorial: campoActual = libro.editorial; break;
            case CampoBusqueda::Id:        campoActual = std::to_string(libro.id); break;
        }
        if (contieneSinMayusculas(campoActual, valor)) {
            resultado.registros.push_back(libro);
        }
    }
    return resultado;
}

bool SearchManager::contieneSinMayusculas(const std::string& texto, const std::string& patron) {
    std::string textoNormalizado(texto);
    std::string patronNormalizado(patron);
    const auto aMinusculas = [](unsigned char caracter) {
        return static_cast<char>(std::tolower(caracter));
    };
    std::transform(textoNormalizado.begin(), textoNormalizado.end(), textoNormalizado.begin(), aMinusculas);
    std::transform(patronNormalizado.begin(), patronNormalizado.end(), patronNormalizado.begin(), aMinusculas);
    return textoNormalizado.find(patronNormalizado) != std::string::npos;
}
