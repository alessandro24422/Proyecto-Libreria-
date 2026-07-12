#include "BufferManager.h"

#include <stdexcept>

BufferManager::BufferManager(std::filesystem::path archivoDatos, std::size_t capacidad)
    : archivoDatos(std::move(archivoDatos)), capacidad(capacidad) {
    std::filesystem::create_directories(this->archivoDatos.parent_path());
    std::fstream archivo(this->archivoDatos, std::ios::binary | std::ios::app);
}

BufferManager::~BufferManager() {
    flush();
}

Pagina& BufferManager::obtenerPagina(int pageId) {
    auto it = bufferPool.find(pageId);
    if (it != bufferPool.end()) {
        return it->second.pagina;
    }

    reemplazarSiNecesario();
    bufferPool.emplace(pageId, Frame{leerPaginaDisco(pageId), false});
    ordenFifo.push(pageId);
    return bufferPool.at(pageId).pagina;
}

int BufferManager::totalPaginas() {
    if (!std::filesystem::exists(archivoDatos)) {
        return 0;
    }
    const auto bytes = std::filesystem::file_size(archivoDatos);
    return static_cast<int>(bytes / sizeof(Pagina));
}

int BufferManager::crearPagina() {
    const int pageId = totalPaginas();
    Pagina pagina{};
    pagina.pageId = pageId;
    pagina.cantidad = 0;
    escribirPaginaDisco(pagina);
    return pageId;
}

void BufferManager::marcarSucio(int pageId) {
    auto it = bufferPool.find(pageId);
    if (it != bufferPool.end()) {
        it->second.sucio = true;
    }
}

void BufferManager::flush() {
    for (auto& par : bufferPool) {
        if (par.second.sucio) {
            escribirPaginaDisco(par.second.pagina);
            par.second.sucio = false;
        }
    }
}

void BufferManager::limpiarBuffer() {
    bufferPool.clear();
    ordenFifo = std::queue<int>();
}

Pagina BufferManager::leerPaginaDisco(int pageId) {
    if (pageId < 0 || pageId >= totalPaginas()) {
        throw std::out_of_range("La pagina solicitada no existe.");
    }

    Pagina pagina{};
    std::ifstream archivo(archivoDatos, std::ios::binary);
    archivo.seekg(static_cast<std::streamoff>(pageId) * sizeof(Pagina));
    archivo.read(reinterpret_cast<char*>(&pagina), sizeof(Pagina));
    return pagina;
}

void BufferManager::escribirPaginaDisco(const Pagina& pagina) {
    std::fstream archivo(archivoDatos, std::ios::binary | std::ios::in | std::ios::out);
    if (!archivo) {
        archivo.open(archivoDatos, std::ios::binary | std::ios::out);
        archivo.close();
        archivo.open(archivoDatos, std::ios::binary | std::ios::in | std::ios::out);
    }

    archivo.seekp(static_cast<std::streamoff>(pagina.pageId) * sizeof(Pagina));
    archivo.write(reinterpret_cast<const char*>(&pagina), sizeof(Pagina));
}

void BufferManager::reemplazarSiNecesario() {
    while (bufferPool.size() >= capacidad && !ordenFifo.empty()) {
        const int victima = ordenFifo.front();
        ordenFifo.pop();

        auto it = bufferPool.find(victima);
        if (it == bufferPool.end()) {
            continue;
        }

        if (it->second.sucio) {
            escribirPaginaDisco(it->second.pagina);
        }
        bufferPool.erase(it);
        return;
    }
}
