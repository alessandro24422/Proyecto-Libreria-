#include "BufferManager.h"

#include <iostream>
#include <stdexcept>

//==============================================================
// Constructor
//==============================================================

BufferManager::BufferManager(
    const std::filesystem::path& archivoDatos,
    std::size_t capacidadBuffer)
    : archivoDatos(archivoDatos),
      capacidadBuffer(capacidadBuffer)
{
    // Crear carpeta contenedora si no existe.
    if (!this->archivoDatos.parent_path().empty())
    {
        std::filesystem::create_directories(
            this->archivoDatos.parent_path());
    }

    // Crear el archivo si no existe.
    std::fstream archivo(
        this->archivoDatos,
        std::ios::binary | std::ios::app);

    archivo.close();
}

//==============================================================
// Destructor
//==============================================================

BufferManager::~BufferManager()
{
    flush();
}

//==============================================================
// Información general
//==============================================================

bool BufferManager::existePagina(int pageId) const
{
    return pageId >= 0 && pageId < totalPaginas();
}

int BufferManager::totalPaginas() const
{
    if (!std::filesystem::exists(archivoDatos))
    {
        return 0;
    }

    const auto bytes = std::filesystem::file_size(archivoDatos);

    return static_cast<int>(bytes / PAGE_SIZE);
}

bool BufferManager::estaEnBuffer(int pageId) const
{
    return frames.count(pageId) > 0;
}

//==============================================================
// Obtención de páginas
//==============================================================

Pagina& BufferManager::obtenerPagina(int pageId)
{
    if (!existePagina(pageId))
    {
        throw std::out_of_range(
            "La pagina solicitada no existe.");
    }

    //==========================================================
    // Buffer Hit
    //==========================================================

    auto it = frames.find(pageId);

    if (it != frames.end())
    {
        estadisticas.bufferHits++;

        return it->second.pagina;
    }

    //==========================================================
    // Buffer Miss
    //==========================================================

    estadisticas.bufferMisses++;

    // Reemplazar si el buffer está lleno.
    if (frames.size() >= capacidadBuffer)
    {
        reemplazarPagina();
    }

    // Cargar desde disco.
    Pagina pagina = leerPaginaDesdeDisco(pageId);

    frames.emplace(pageId, BufferFrame{pagina, false});

    colaFIFO.push(pageId);

    return frames.at(pageId).pagina;
}

//==============================================================
// Creación de páginas
//==============================================================

int BufferManager::crearPagina()
{
    const int nuevoPageId = totalPaginas();

    Pagina pagina{};
    pagina.pageId = nuevoPageId;
    pagina.cantidadRegistros = 0;

    escribirPaginaEnDisco(pagina);

    return nuevoPageId;
}

//==============================================================
// Marcar página sucia
//==============================================================

void BufferManager::marcarSucio(int pageId)
{
    auto it = frames.find(pageId);

    if (it != frames.end())
    {
        it->second.sucio = true;
    }
}

//==============================================================
// Escritura de páginas sucias
//==============================================================

void BufferManager::flush()
{
    for (auto& [pageId, frame] : frames)
    {
        if (frame.sucio)
        {
            escribirPaginaEnDisco(frame.pagina);

            frame.sucio = false;
        }
    }
}

//==============================================================
// Limpieza del buffer
//==============================================================

void BufferManager::limpiarBuffer()
{
    flush();

    frames.clear();

    colaFIFO = std::queue<int>{};
}

//==============================================================
// Estadísticas
//==============================================================

const EstadisticasBuffer&
BufferManager::obtenerEstadisticas() const
{
    return estadisticas;
}

//==============================================================
// Estado del Buffer Pool
//==============================================================

void BufferManager::imprimirEstadoBuffer() const
{
    std::cout << '\n'<<"===== BUFFER POOL ====="<<'\n';

    if (frames.empty())
    {
        std::cout << "Buffer vacio."<<'\n';
        return;
    }

    for (const auto& [pageId, frame] : frames)
    {
        std::cout << "Pagina: " << pageId
                  << " | Registros: "
                  << frame.pagina.cantidadRegistros
                  << " | Sucia: "
                  << (frame.sucio ? "SI" : "NO")
                  << '\n';
    }

    std::cout << "======================="<<'\n';
}

double BufferManager::obtenerHitRate() const
{
    int total = estadisticas.bufferHits + estadisticas.bufferMisses;

    if (total == 0)
        return 0.0;

    return static_cast<double>(estadisticas.bufferHits) / total;
}


//==============================================================
// Lectura desde disco
//==============================================================

Pagina BufferManager::leerPaginaDesdeDisco(int pageId)
{
    std::ifstream archivo(archivoDatos, std::ios::binary);

    if (!archivo)
    {
        throw std::runtime_error(
            "No se pudo abrir el archivo de datos.");
    }

    archivo.seekg(
        static_cast<std::streamoff>(pageId) * PAGE_SIZE);

    Pagina pagina{};

    archivo.read(reinterpret_cast<char*>(&pagina),
                 sizeof(Pagina));

    if (!archivo)
    {
        throw std::runtime_error(
            "Error al leer la pagina desde disco.");
    }

    estadisticas.lecturasDisco++;

    return pagina;
}

//==============================================================
// Escritura en disco
//==============================================================

void BufferManager::escribirPaginaEnDisco(const Pagina& pagina)
{
    std::fstream archivo(
        archivoDatos,
        std::ios::binary |
        std::ios::in |
        std::ios::out);

    if (!archivo)
    {
        // Crear archivo si no existe.
        archivo.open(archivoDatos,
                     std::ios::binary | std::ios::out);

        archivo.close();

        archivo.open(archivoDatos,
                     std::ios::binary |
                     std::ios::in |
                     std::ios::out);
    }

    archivo.seekp(
        static_cast<std::streamoff>(pagina.pageId) * PAGE_SIZE);

    archivo.write(reinterpret_cast<const char*>(&pagina),
                  sizeof(Pagina));

    archivo.flush();

    estadisticas.escriturasDisco++;
}

//==============================================================
// Reemplazo FIFO
//==============================================================

void BufferManager::reemplazarPagina()
{
    if (colaFIFO.empty())
    {
        throw std::runtime_error(
            "No hay paginas para reemplazar.");
    }

    const int victima = colaFIFO.front();
    colaFIFO.pop();

    auto it = frames.find(victima);

    if (it == frames.end())
    {
        return;
    }

    // Si la página está sucia, escribirla antes de expulsarla.
    if (it->second.sucio)
    {
        escribirPaginaEnDisco(it->second.pagina);
    }

    frames.erase(it);
}