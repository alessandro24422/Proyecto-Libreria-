#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "Libro.h"

#include <filesystem>
#include <fstream>
#include <queue>
#include <unordered_map>

//==============================================================
// Configuración del almacenamiento
//==============================================================

constexpr std::size_t PAGE_SIZE = 4096;

// Número de registros que caben en una página.
constexpr std::size_t REGISTROS_POR_PAGINA =
    (PAGE_SIZE - sizeof(int) * 2) / sizeof(Libro);

// Espacio restante de la página.
constexpr std::size_t PAGE_PADDING =
    PAGE_SIZE
    - sizeof(int) * 2
    - sizeof(Libro) * REGISTROS_POR_PAGINA;

//==============================================================
// Página de almacenamiento.
// Cada página ocupa exactamente 4096 bytes.
//==============================================================

#pragma pack(push,1)

struct Pagina
{
    int pageId;

    int cantidadRegistros;

    Libro registros[REGISTROS_POR_PAGINA];

    char relleno[PAGE_PADDING];
};

#pragma pack(pop)

static_assert(sizeof(Pagina) == PAGE_SIZE,
              "Pagina debe ocupar exactamente 4096 bytes.");

//==============================================================
// Frame del Buffer Pool
//==============================================================

struct BufferFrame
{
    Pagina pagina;

    bool sucio = false;
};

//==============================================================
// Estadísticas
//==============================================================

struct EstadisticasBuffer
{
    int bufferHits = 0;

    int bufferMisses = 0;

    int lecturasDisco = 0;

    int escriturasDisco = 0;
};

//==============================================================
// Buffer Manager
//==============================================================

class BufferManager
{
public:

    explicit BufferManager(
        const std::filesystem::path& archivoDatos,
        std::size_t capacidadBuffer = 3);

    ~BufferManager();

    Pagina& obtenerPagina(int pageId);

    int crearPagina();

    bool existePagina(int pageId) const;

    int totalPaginas() const;

    bool estaEnBuffer(int pageId) const;

    void marcarSucio(int pageId);

    void limpiarBuffer();

    void flush();

    const EstadisticasBuffer& obtenerEstadisticas() const;

    void imprimirEstadoBuffer() const;

    double obtenerHitRate() const;

private:

    std::filesystem::path archivoDatos;

    std::size_t capacidadBuffer;

    std::unordered_map<int, BufferFrame> frames;

    std::queue<int> colaFIFO;

    EstadisticasBuffer estadisticas;

    //-------------------------
    // Acceso al almacenamiento
    //-------------------------

    Pagina leerPaginaDesdeDisco(int pageId);

    void escribirPaginaEnDisco(const Pagina& pagina);

    //-------------------------
    // Gestión del Buffer Pool
    //-------------------------

    void reemplazarPagina();
};

#endif