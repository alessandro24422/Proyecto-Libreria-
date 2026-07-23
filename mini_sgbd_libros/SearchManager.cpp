#include "SearchManager.h"

#include "SortManager.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <sstream>

SearchManager::SearchManager(StorageManager& storage) : storage(storage) {}

std::optional<Libro> SearchManager::buscarPorID(int id) const {
    return storage.buscarPorID(id);
}

std::vector<Libro> SearchManager::buscarPorTitulo(const std::string& titulo) const {
    std::vector<Libro> resultado;
    for (const Libro& libro : storage.listarLibros()) {
        if (contieneSinMayusculas(libro.titulo, titulo)) resultado.push_back(libro);
    }
    return resultado;
}

std::vector<Libro> SearchManager::buscarPorAutor(const std::string& autor) const {
    std::vector<Libro> resultado;
    for (const Libro& libro : storage.listarLibros()) {
        if (contieneSinMayusculas(libro.autor, autor)) resultado.push_back(libro);
    }
    return resultado;
}

std::vector<Libro> SearchManager::buscarPorGenero(const std::string& genero) const {
    std::vector<Libro> resultado;
    for (const Libro& libro : storage.listarLibros()) {
        if (contieneSinMayusculas(libro.genero, genero)) resultado.push_back(libro);
    }
    return resultado;
}

std::vector<Libro> SearchManager::buscarPorAnio(int anio) const {
    std::vector<Libro> resultado;
    for (const Libro& libro : storage.listarLibros()) {
        if (libro.anio == anio) resultado.push_back(libro);
    }
    return resultado;
}

ResultadoConsulta SearchManager::ejecutarConsulta(const std::string& consulta) const {
    const auto inicio = std::chrono::steady_clock::now();
    ResultadoConsulta resultado;
    const std::string texto = limpiar(consulta);
    const std::string textoMayusculas = mayusculas(texto);
    constexpr const char* SELECT = "SELECT *";

    if (textoMayusculas.rfind(SELECT, 0) != 0) {
        resultado.mensaje = "Consulta invalida: debe comenzar con SELECT *.";
        resultado.duracion = std::chrono::steady_clock::now() - inicio;
        return resultado;
    }

    std::string filtro;
    std::string orden;
    const std::size_t posWhere = textoMayusculas.find(" WHERE ");
    const std::size_t posOrder = textoMayusculas.find(" ORDER BY ");
    if (posWhere != std::string::npos) {
        const std::size_t inicioFiltro = posWhere + 7;
        filtro = limpiar(texto.substr(inicioFiltro, (posOrder == std::string::npos ? texto.size() : posOrder) - inicioFiltro));
    }
    if (posOrder != std::string::npos) orden = limpiar(texto.substr(posOrder + 10));

    resultado.libros = storage.listarLibros();
    if (!filtro.empty()) {
        std::istringstream parser(filtro);
        std::string campo, operador;
        parser >> campo >> operador;
        std::string valor;
        std::getline(parser, valor);
        valor = limpiar(valor);
        if (valor.size() >= 2 && valor.front() == '"' && valor.back() == '"')
            valor = valor.substr(1, valor.size() - 2);

        const std::string campoNormalizado = mayusculas(campo);
        const std::string operadorNormalizado = mayusculas(operador);
        const bool operadorValido = operador == "=" || operadorNormalizado == "CONTAINS";
        if (valor.empty() || !operadorValido) {
            resultado.libros.clear();
            resultado.mensaje = "WHERE invalido. Use campo = valor o campo CONTAINS valor.";
        } else if (campoNormalizado == "ID" || campoNormalizado == "ANIO") {
            try {
                const int numero = std::stoi(valor);
                resultado.libros.erase(std::remove_if(resultado.libros.begin(), resultado.libros.end(),
                    [campoNormalizado, numero](const Libro& libro) {
                        return campoNormalizado == "ID" ? libro.id != numero : libro.anio != numero;
                    }), resultado.libros.end());
            } catch (...) {
                resultado.libros.clear();
                resultado.mensaje = "ID y anio deben tener un valor numerico.";
            }
        } else {
            if (campoNormalizado != "TITULO" && campoNormalizado != "AUTOR" &&
                campoNormalizado != "GENERO") {
                resultado.libros.clear();
                resultado.mensaje = "Campo WHERE no soportado.";
            } else {
                resultado.libros.erase(std::remove_if(resultado.libros.begin(), resultado.libros.end(),
                    [campoNormalizado, valor, operadorNormalizado](const Libro& libro) {
                        const std::string actual = campoNormalizado == "TITULO" ? libro.titulo
                            : (campoNormalizado == "AUTOR" ? libro.autor : libro.genero);
                        return operadorNormalizado == "CONTAINS"
                            ? !contieneSinMayusculas(actual, valor)
                            : mayusculas(actual) != mayusculas(valor);
                    }), resultado.libros.end());
            }
        }
    }

    if (!orden.empty() && resultado.mensaje.empty()) {
        std::istringstream parser(orden);
        std::string campo, direccion;
        parser >> campo >> direccion;
        const bool ascendente = mayusculas(direccion) != "DESC";
        const std::string campoNormalizado = mayusculas(campo);
        if (campoNormalizado == "TITULO") resultado.libros = SortManager::ordenarPorTitulo(std::move(resultado.libros), ascendente);
        else if (campoNormalizado == "AUTOR") resultado.libros = SortManager::ordenarPorAutor(std::move(resultado.libros), ascendente);
        else if (campoNormalizado == "ANIO") resultado.libros = SortManager::ordenarPorAnio(std::move(resultado.libros), ascendente);
        else resultado.mensaje = "Campo ORDER BY no soportado.";
    }

    if (resultado.mensaje.empty()) resultado.mensaje = "Consulta ejecutada correctamente.";
    resultado.duracion = std::chrono::steady_clock::now() - inicio;
    return resultado;
}

ComparacionBusquedaId SearchManager::compararBusquedaPorID(int id) const {
    ComparacionBusquedaId comparacion;
    auto inicio = std::chrono::steady_clock::now();
    comparacion.resultadoIndice = buscarPorID(id);
    comparacion.tiempoIndice = std::chrono::steady_clock::now() - inicio;
    inicio = std::chrono::steady_clock::now();
    comparacion.resultadoLineal = buscarPorIDLineal(id);
    comparacion.tiempoLineal = std::chrono::steady_clock::now() - inicio;
    return comparacion;
}

std::optional<Libro> SearchManager::buscarPorIDLineal(int id) const {
    for (const Libro& libro : storage.listarLibros()) if (libro.id == id) return libro;
    return std::nullopt;
}

bool SearchManager::contieneSinMayusculas(const std::string& texto, const std::string& termino) {
    return mayusculas(texto).find(mayusculas(termino)) != std::string::npos;
}

std::string SearchManager::limpiar(const std::string& texto) {
    const auto primero = texto.find_first_not_of(" \t\r\n");
    if (primero == std::string::npos) return "";
    const auto ultimo = texto.find_last_not_of(" \t\r\n");
    return texto.substr(primero, ultimo - primero + 1);
}

std::string SearchManager::mayusculas(const std::string& texto) {
    std::string resultado = texto;
    std::transform(resultado.begin(), resultado.end(), resultado.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return resultado;
}
