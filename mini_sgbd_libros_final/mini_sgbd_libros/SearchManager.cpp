#include "SearchManager.h"

#include "QueryOperators.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <memory>
#include <sstream>

SearchManager::SearchManager(StorageManager& storage) : storage(storage) {}

std::optional<Libro> SearchManager::buscarPorID(int id) { return storage.buscarPorID(id); }

std::vector<Libro> SearchManager::buscarPorTitulo(const std::string& titulo) {
    std::vector<Libro> resultado;
    for (const Libro& libro : storage.listarLibros())
        if (contieneSinMayusculas(libro.titulo, titulo)) resultado.push_back(libro);
    return resultado;
}

std::vector<Libro> SearchManager::buscarPorAutor(const std::string& autor) {
    std::vector<Libro> resultado;
    for (const Libro& libro : storage.listarLibros())
        if (contieneSinMayusculas(libro.autor, autor)) resultado.push_back(libro);
    return resultado;
}

std::vector<Libro> SearchManager::buscarPorCodigoDewey(const std::string& codigoDewey) {
    std::vector<Libro> resultado;
    for (const Libro& libro : storage.listarLibros())
        if (contieneSinMayusculas(libro.codigoDewey, codigoDewey)) resultado.push_back(libro);
    return resultado;
}

std::vector<Libro> SearchManager::buscarPorAnio(int anio) {
    std::vector<Libro> resultado;
    for (const Libro& libro : storage.listarLibros()) if (libro.anio == anio) resultado.push_back(libro);
    return resultado;
}

ResultadoConsulta SearchManager::ejecutarConsulta(const std::string& consulta) {
    const auto inicio = std::chrono::steady_clock::now();
    ResultadoConsulta resultado;
    const std::string texto = limpiar(consulta);
    const std::string textoMayusculas = mayusculas(texto);
    constexpr const char* SELECT = "SELECT *";

    if (textoMayusculas.rfind(SELECT, 0) != 0) {
        resultado.mensaje = "Consulta invalida: debe comenzar con SELECT *.";
    } else {
        std::string filtro, orden;
        const std::size_t posWhere = textoMayusculas.find(" WHERE ");
        const std::size_t posOrder = textoMayusculas.find(" ORDER BY ");
        const std::size_t despuesSelect = std::char_traits<char>::length(SELECT);
        if ((posWhere != std::string::npos && posWhere < despuesSelect) ||
            (posOrder != std::string::npos && posOrder < despuesSelect) ||
            (posWhere == std::string::npos && posOrder == std::string::npos && texto.size() != despuesSelect) ||
            (posWhere != std::string::npos && posOrder != std::string::npos && posOrder < posWhere)) {
            resultado.mensaje = "Sintaxis SELECT invalida.";
        } else {
            if (posWhere != std::string::npos) {
                const std::size_t inicioFiltro = posWhere + 7;
                filtro = limpiar(texto.substr(inicioFiltro, (posOrder == std::string::npos ? texto.size() : posOrder) - inicioFiltro));
            }
            if (posOrder != std::string::npos) orden = limpiar(texto.substr(posOrder + 10));
            if ((posWhere != std::string::npos && filtro.empty()) ||
                (posOrder != std::string::npos && orden.empty())) {
                resultado.mensaje = "WHERE u ORDER BY requieren un valor.";
            }
        }

        FilterOperator::Predicate predicado;
        if (resultado.mensaje.empty() && !filtro.empty()) {
            std::istringstream parser(filtro);
            std::string campo, operador, valor;
            parser >> campo >> operador;
            std::getline(parser, valor);
            valor = limpiar(valor);
            if (valor.size() >= 2 && valor.front() == '"' && valor.back() == '"') valor = valor.substr(1, valor.size() - 2);

            const std::string campoNormalizado = mayusculas(campo);
            const std::string operadorNormalizado = mayusculas(operador);
            const bool esNumerico = campoNormalizado == "ID" || campoNormalizado == "ANIO";
            const bool operadorValido = esNumerico ? operador == "=" : (operador == "=" || operadorNormalizado == "CONTAINS");
            if (valor.empty() || !operadorValido) {
                resultado.mensaje = "WHERE invalido.";
            } else if (esNumerico) {
                try {
                    const int numero = std::stoi(valor);
                    predicado = [campoNormalizado, numero](const Libro& libro) {
                        return campoNormalizado == "ID" ? libro.id == numero : libro.anio == numero;
                    };
                } catch (...) { resultado.mensaje = "ID y anio deben tener un valor numerico."; }
            } else if (campoNormalizado == "TITULO" || campoNormalizado == "AUTOR" || campoNormalizado == "CODIGODEWEY" || campoNormalizado == "DEWEY") {
                predicado = [campoNormalizado, valor, operadorNormalizado](const Libro& libro) {
                        const std::string actual = campoNormalizado == "TITULO" ? libro.titulo : (campoNormalizado == "AUTOR" ? libro.autor : libro.codigoDewey);
                        return operadorNormalizado == "CONTAINS" ? contieneSinMayusculas(actual, valor) : mayusculas(actual) == mayusculas(valor);
                    };
            } else { resultado.mensaje = "Campo WHERE no soportado."; }
        }

        SortOperator::Comparator comparador;
        if (resultado.mensaje.empty() && !orden.empty()) {
            std::istringstream parser(orden);
            std::string campo, direccion, extra;
            parser >> campo >> direccion >> extra;
            const std::string campoNormalizado = mayusculas(campo);
            const std::string direccionNormalizada = mayusculas(direccion);
            if (!extra.empty() || (direccionNormalizada != "" && direccionNormalizada != "ASC" && direccionNormalizada != "DESC")) resultado.mensaje = "Direccion ORDER BY invalida.";
            else {
                const bool ascendente = direccionNormalizada != "DESC";
                if (campoNormalizado == "TITULO" || campoNormalizado == "AUTOR" || campoNormalizado == "ANIO") {
                    comparador = [campoNormalizado, ascendente](const Libro& a, const Libro& b) {
                        bool menor = false;
                        bool mayor = false;
                        if (campoNormalizado == "TITULO") { menor = mayusculas(a.titulo) < mayusculas(b.titulo); mayor = mayusculas(b.titulo) < mayusculas(a.titulo); }
                        else if (campoNormalizado == "AUTOR") { menor = mayusculas(a.autor) < mayusculas(b.autor); mayor = mayusculas(b.autor) < mayusculas(a.autor); }
                        else { menor = a.anio < b.anio; mayor = b.anio < a.anio; }
                        if (menor) return ascendente;
                        if (mayor) return !ascendente;
                        return ascendente ? a.id < b.id : a.id > b.id;
                    };
                }
                else resultado.mensaje = "Campo ORDER BY no soportado.";
            }
        }

        if (resultado.mensaje.empty()) {
            // Pipeline Volcano: Scan -> Filter opcional -> Sort opcional.
            std::unique_ptr<QueryOperator> pipeline = std::make_unique<ScanOperator>(storage);
            if (predicado) pipeline = std::make_unique<FilterOperator>(std::move(pipeline), std::move(predicado));
            if (comparador) pipeline = std::make_unique<SortOperator>(std::move(pipeline), std::move(comparador));

            pipeline->open();
            Libro libro{};
            while (pipeline->next(libro)) resultado.libros.push_back(libro);
            pipeline->close();
        }
    }
    if (resultado.mensaje.empty()) resultado.mensaje = "Consulta ejecutada correctamente.";
    resultado.duracion = std::chrono::steady_clock::now() - inicio;
    return resultado;
}

ComparacionBusquedaId SearchManager::compararBusquedaPorID(int id) {
    ComparacionBusquedaId comparacion;
    auto inicio = std::chrono::steady_clock::now(); comparacion.resultadoIndice = buscarPorID(id);
    comparacion.tiempoIndice = std::chrono::steady_clock::now() - inicio;
    inicio = std::chrono::steady_clock::now(); comparacion.resultadoLineal = buscarPorIDLineal(id);
    comparacion.tiempoLineal = std::chrono::steady_clock::now() - inicio;
    return comparacion;
}

std::optional<Libro> SearchManager::buscarPorIDLineal(int id) {
    for (const Libro& libro : storage.listarLibros()) if (libro.id == id) return libro;
    return std::nullopt;
}

bool SearchManager::contieneSinMayusculas(const std::string& texto, const std::string& termino) { return mayusculas(texto).find(mayusculas(termino)) != std::string::npos; }
std::string SearchManager::limpiar(const std::string& texto) { const auto primero = texto.find_first_not_of(" \t\r\n"); if (primero == std::string::npos) return ""; const auto ultimo = texto.find_last_not_of(" \t\r\n"); return texto.substr(primero, ultimo - primero + 1); }
std::string SearchManager::mayusculas(const std::string& texto) { std::string resultado = texto; std::transform(resultado.begin(), resultado.end(), resultado.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); }); return resultado; }
