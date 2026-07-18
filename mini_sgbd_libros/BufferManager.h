#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "Libro.h"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <queue>
#include <unordered_map>

constexpr int PAGE_SIZE = 4096;
constexpr int REGISTROS_POR_PAGINA = 9;

#pragma pack(push, 1)
struct Pagina {
    int pageId;
    int cantidad;
    Libro registros[REGISTROS_POR_PAGINA];
    char relleno[PAGE_SIZE - sizeof(int) * 2 - sizeof(Libro) * REGISTROS_POR_PAGINA];
};
#pragma pack(pop)

struct Frame {
    Pagina pagina;
    bool sucio;
};

class BufferManager {
public:
    explicit BufferManager(std::filesystem::path archivoDatos, std::size_t capacidad = 3);
    ~BufferManager();

    Pagina& obtenerPagina(int pageId);
    int totalPaginas();
    int crearPagina();
    void marcarSucio(int pageId);
    void flush();
    void limpiarBuffer();

private:
    std::filesystem::path archivoDatos;
    std::size_t capacidad;
    std::unordered_map<int, Frame> bufferPool;
    std::queue<int> ordenFifo;

    Pagina leerPaginaDisco(int pageId);
    void escribirPaginaDisco(const Pagina& pagina);
    void reemplazarSiNecesario();
};

#endif
