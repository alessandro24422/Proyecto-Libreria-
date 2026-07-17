#include "QueryManager.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <regex>
#include <stdexcept>

namespace {
std::string recortar(const std::string& texto) {
    const auto inicio = texto.find_first_not_of(" \t\r\n");
    if (inicio == std::string::npos) return "";
    const auto fin = texto.find_last_not_of(" \t\r\n");
    return texto.substr(inicio, fin - inicio + 1);
}

std::string minusculas(std::string texto) {
    std::transform(texto.begin(), texto.end(), texto.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return texto;
}

CampoBusqueda campoBusqueda(const std::string& campo) {
    const std::string nombre = minusculas(campo);
    if (nombre == "titulo") return CampoBusqueda::Titulo;
    if (nombre == "autor") return CampoBusqueda::Autor;
    if (nombre == "anio") return CampoBusqueda::Anio;
    if (nombre == "editorial") return CampoBusqueda::Editorial;
    throw std::invalid_argument("Campo WHERE no soportado: " + campo);
}

CampoOrdenamiento campoOrdenamiento(const std::string& campo) {
    const std::string nombre = minusculas(campo);
    if (nombre == "id") return CampoOrdenamiento::Id;
    if (nombre == "titulo") return CampoOrdenamiento::Titulo;
    if (nombre == "autor") return CampoOrdenamiento::Autor;
    if (nombre == "anio") return CampoOrdenamiento::Anio;
    throw std::invalid_argument("Campo ORDER BY no soportado: " + campo);
}
}

QueryManager::QueryManager(StorageManager& storage)
    : storage(storage), searchManager(storage) {
}

QueryResult QueryManager::ejecutar(const std::string& consulta) {
    const auto inicio = std::chrono::steady_clock::now();
    QueryResult resultado;
    resultado.consultaEjecutada = consulta;

    std::string sql = recortar(consulta);
    if (!sql.empty() && sql.back() == ';') sql.pop_back();

    const std::regex selectTodo(R"(^\s*select\s+\*\s*$)", std::regex::icase);
    const std::regex where(R"consulta(^\s*select\s+\*\s+where\s+([a-zA-Z]+)\s*=\s*(?:"([^"]*)"|'([^']*)'|([0-9]+))\s*$)consulta", std::regex::icase);
    const std::regex orderBy(R"(^\s*select\s+\*\s+order\s+by\s+([a-zA-Z]+)(?:\s+(asc|desc))?\s*$)", std::regex::icase);
    std::smatch coincidencia;
    ResultadoBusqueda busqueda;

    if (std::regex_match(sql, selectTodo)) {
        resultado.registros = storage.listarLibros();
        resultado.registrosLeidos = resultado.registros.size();
    } else if (std::regex_match(sql, coincidencia, where)) {
        const std::string campo = coincidencia[1].str();
        const std::string valor = coincidencia[2].matched ? coincidencia[2].str()
                                : coincidencia[3].matched ? coincidencia[3].str()
                                                         : coincidencia[4].str();
        if (minusculas(campo) == "id") {
            try {
                busqueda = searchManager.buscarPorId(std::stoi(valor));
            } catch (const std::exception&) {
                throw std::invalid_argument("El ID debe ser un entero valido.");
            }
        } else {
            busqueda = searchManager.buscarParcial(campoBusqueda(campo), valor);
        }
        resultado.registros = std::move(busqueda.registros);
        resultado.registrosLeidos = busqueda.registrosLeidos;
        resultado.utilizoIndice = busqueda.utilizoIndice;
    } else if (std::regex_match(sql, coincidencia, orderBy)) {
        resultado.registros = storage.listarLibros();
        resultado.registrosLeidos = resultado.registros.size();
        const DireccionOrdenamiento direccion = !coincidencia[2].matched ||
                minusculas(coincidencia[2].str()) == "asc"
            ? DireccionOrdenamiento::Ascendente
            : DireccionOrdenamiento::Descendente;
        sortManager.ordenar(resultado.registros, campoOrdenamiento(coincidencia[1].str()), direccion);
    } else {
        throw std::invalid_argument("Consulta no soportada. Use SELECT *, WHERE u ORDER BY.");
    }

    resultado.registrosEncontrados = resultado.registros.size();
    resultado.tiempoEjecucion = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - inicio);
    return resultado;
}
