#include "StorageManager.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <stdexcept>

StorageManager::StorageManager(const std::filesystem::path& carpetaBase,
                               const std::filesystem::path& carpetaBiblioteca)
    : carpetaBase(carpetaBase),
      carpetaBiblioteca(carpetaBiblioteca),
      archivoDatos(carpetaBase / "libros.dat"),
      archivoIndice(carpetaBase / "indice_hash.dat"),
      buffer(archivoDatos, 3) {
    std::filesystem::create_directories(this->carpetaBase);
    std::filesystem::create_directories(this->carpetaBiblioteca);
    cargarIndiceHash();
    if (indiceHash.empty()) {
        reconstruirIndiceHash();
    }
}

bool StorageManager::agregarLibro(const Libro& libro) {
    if (!datosValidos(libro) || indiceHash.count(libro.id) > 0) {
        return false;
    }

    Libro nuevo = libro;
    copiarCampo(nuevo.rutaPdf, TAM_RUTA_PDF, prepararPdfEnBiblioteca(libro));
    nuevo.activo = true;

    auto libre = buscarEspacioLibre();
    if (!libre.has_value()) {
        libre = UbicacionRegistro{buffer.crearPagina(), 0};
    }

    Pagina& pagina = buffer.obtenerPagina(libre->pageId);
    pagina.registros[libre->slot] = nuevo;
    if (libre->slot >= pagina.cantidad) {
        pagina.cantidad = libre->slot + 1;
    }

    indiceHash[nuevo.id] = *libre;
    buffer.marcarSucio(libre->pageId);
    buffer.flush();
    guardarIndiceHash();
    return true;
}

bool StorageManager::eliminarLibro(int id) {
    auto it = indiceHash.find(id);
    if (it == indiceHash.end()) {
        return false;
    }

    Pagina& pagina = buffer.obtenerPagina(it->second.pageId);
    Libro& libro = pagina.registros[it->second.slot];
    if (!libro.activo) {
        indiceHash.erase(it);
        guardarIndiceHash();
        return false;
    }

    libro.activo = false;
    indiceHash.erase(it);
    buffer.marcarSucio(pagina.pageId);
    buffer.flush();
    guardarIndiceHash();
    return true;
}

bool StorageManager::actualizarLibro(int id, const Libro& libroActualizado) {
    auto it = indiceHash.find(id);
    if (it == indiceHash.end() || !datosValidos(libroActualizado)) {
        return false;
    }

    Libro actualizado = libroActualizado;
    actualizado.id = id;
    actualizado.activo = true;
    copiarCampo(actualizado.rutaPdf, TAM_RUTA_PDF, prepararPdfEnBiblioteca(actualizado));

    Pagina& pagina = buffer.obtenerPagina(it->second.pageId);
    pagina.registros[it->second.slot] = actualizado;
    buffer.marcarSucio(pagina.pageId);
    buffer.flush();
    guardarIndiceHash();
    return true;
}

std::optional<Libro> StorageManager::buscarPorID(int id) {
    auto it = indiceHash.find(id);
    if (it == indiceHash.end()) {
        return std::nullopt;
    }

    Pagina& pagina = buffer.obtenerPagina(it->second.pageId);
    Libro libro = pagina.registros[it->second.slot];
    if (!libro.activo) {
        return std::nullopt;
    }
    return libro;
}

std::vector<Libro> StorageManager::cargarLibros() {
    std::vector<Libro> libros;
    const int paginas = buffer.totalPaginas();
    for (int pageId = 0; pageId < paginas; ++pageId) {
        Pagina& pagina = buffer.obtenerPagina(pageId);
        for (int slot = 0; slot < pagina.cantidad; ++slot) {
            if (pagina.registros[slot].activo) {
                libros.push_back(pagina.registros[slot]);
            }
        }
    }
    return libros;
}

std::vector<Libro> StorageManager::listarLibros() {
    return cargarLibros();
}

void StorageManager::guardarLibros(const std::vector<Libro>& libros) {
    buffer.flush();
    std::ofstream limpiar(archivoDatos, std::ios::binary | std::ios::trunc);
    limpiar.close();
    buffer.limpiarBuffer();

    indiceHash.clear();
    for (const Libro& libro : libros) {
        agregarLibro(libro);
    }
    guardarIndiceHash();
}

void StorageManager::reconstruirIndiceHash() {
    indiceHash.clear();
    const int paginas = buffer.totalPaginas();
    for (int pageId = 0; pageId < paginas; ++pageId) {
        Pagina& pagina = buffer.obtenerPagina(pageId);
        for (int slot = 0; slot < pagina.cantidad; ++slot) {
            const Libro& libro = pagina.registros[slot];
            if (libro.activo) {
                indiceHash[libro.id] = UbicacionRegistro{pageId, slot};
            }
        }
    }
    guardarIndiceHash();
}

void StorageManager::guardarIndiceHash() {
    std::ofstream archivo(archivoIndice, std::ios::binary | std::ios::trunc);
    const std::size_t cantidad = indiceHash.size();
    archivo.write(reinterpret_cast<const char*>(&cantidad), sizeof(cantidad));
    for (const auto& par : indiceHash) {
        archivo.write(reinterpret_cast<const char*>(&par.first), sizeof(par.first));
        archivo.write(reinterpret_cast<const char*>(&par.second), sizeof(par.second));
    }
}

void StorageManager::cargarIndiceHash() {
    indiceHash.clear();
    std::ifstream archivo(archivoIndice, std::ios::binary);
    if (!archivo) {
        return;
    }

    std::size_t cantidad = 0;
    archivo.read(reinterpret_cast<char*>(&cantidad), sizeof(cantidad));
    for (std::size_t i = 0; i < cantidad; ++i) {
        int id = 0;
        UbicacionRegistro ubicacion{};
        archivo.read(reinterpret_cast<char*>(&id), sizeof(id));
        archivo.read(reinterpret_cast<char*>(&ubicacion), sizeof(ubicacion));
        if (archivo) {
            indiceHash[id] = ubicacion;
        }
    }
}

bool StorageManager::idValido(int id) const {
    return id > 0;
}

bool StorageManager::datosValidos(const Libro& libro) const {
    return idValido(libro.id) &&
           std::strlen(libro.titulo) > 0 &&
           std::strlen(libro.autor) > 0 &&
           std::strlen(libro.genero) > 0 &&
           libro.anio > 0;
}

std::string StorageManager::normalizarGenero(const std::string& genero) const {
    std::string limpio;
    for (char c : genero) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            limpio.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        } else if (c == ' ' || c == '-' || c == '_') {
            limpio.push_back('_');
        }
    }
    return limpio.empty() ? "sin_genero" : limpio;
}

std::string StorageManager::prepararPdfEnBiblioteca(const Libro& libro) {
    const std::filesystem::path origen(libro.rutaPdf);
    const std::string genero = normalizarGenero(libro.genero);
    const std::filesystem::path carpetaGenero = carpetaBiblioteca / genero;
    std::filesystem::create_directories(carpetaGenero);

    if (origen.extension() != ".pdf" && origen.extension() != ".PDF") {
        return origen.string();
    }

    if (!std::filesystem::exists(origen)) {
        return (carpetaGenero / origen.filename()).string();
    }

    const std::filesystem::path destino = carpetaGenero / origen.filename();
    if (std::filesystem::absolute(origen) != std::filesystem::absolute(destino)) {
        std::filesystem::copy_file(origen, destino, std::filesystem::copy_options::overwrite_existing);
    }
    return destino.string();
}

std::optional<UbicacionRegistro> StorageManager::buscarEspacioLibre() {
    const int paginas = buffer.totalPaginas();
    for (int pageId = 0; pageId < paginas; ++pageId) {
        Pagina& pagina = buffer.obtenerPagina(pageId);

        for (int slot = 0; slot < pagina.cantidad; ++slot) {
            if (!pagina.registros[slot].activo) {
                return UbicacionRegistro{pageId, slot};
            }
        }

        if (pagina.cantidad < REGISTROS_POR_PAGINA) {
            return UbicacionRegistro{pageId, pagina.cantidad};
        }
    }
    return std::nullopt;
}
