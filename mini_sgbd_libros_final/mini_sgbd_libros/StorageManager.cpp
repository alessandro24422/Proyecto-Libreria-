#include "StorageManager.h"

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

//==============================================================
// Constructor
//==============================================================

StorageManager::StorageManager(
    const std::filesystem::path& carpetaBase,
    const std::filesystem::path& carpetaBiblioteca)
    : carpetaBase(carpetaBase),
      carpetaBiblioteca(carpetaBiblioteca),
      archivoDatos(carpetaBase / "libros.dat"),
      archivoIndice(carpetaBase / "indice_hash.dat"),
      buffer(archivoDatos, 3)
{
    std::filesystem::create_directories(this->carpetaBase);
    std::filesystem::create_directories(this->carpetaBiblioteca);

    cargarIndiceHash();

    if (indiceHash.empty())
    {
        reconstruirIndiceHash();
    }
}

//==============================================================
// Validaciones
//==============================================================

bool StorageManager::idValido(int id) const
{
    return id > 0;
}

bool StorageManager::datosValidos(const Libro& libro) const
{
    return
        idValido(libro.id) &&
        std::strlen(libro.titulo) > 0 &&
        std::strlen(libro.autor) > 0 &&
        std::strlen(libro.codigoDewey) > 0 &&
        libro.anio > 0;
}
//==============================================================
// Obtiene la carpeta principal del Sistema Dewey.
// Ejemplos:
//
// 005.13 -> 000
// 150.2  -> 100
// 512.8  -> 500
//==============================================================

std::string StorageManager::obtenerCarpetaDewey(
    const Libro& libro) const
{
    if (std::strlen(libro.codigoDewey) == 0)
    {
        return "000";
    }

    char categoria = libro.codigoDewey[0];

    if (categoria >= '0' && categoria <= '9')
    {
        return std::string(1, categoria) + "00";
    }

    return "000";
}

//==============================================================
// Copia el PDF a la carpeta correspondiente según Dewey.
//==============================================================

std::string StorageManager::copiarPdf(
    const Libro& libro)
{
    std::filesystem::path origen(libro.rutaPdf);

    if (!std::filesystem::exists(origen))
    {
        return origen.string();
    }

    std::filesystem::path destinoCarpeta =
        carpetaBiblioteca /
        obtenerCarpetaDewey(libro);

    std::filesystem::create_directories(destinoCarpeta);

    std::filesystem::path destino =
        destinoCarpeta /
        origen.filename();

    if (std::filesystem::absolute(origen) !=
        std::filesystem::absolute(destino))
    {
        std::filesystem::copy_file(
            origen,
            destino,
            std::filesystem::copy_options::overwrite_existing);
    }

    return destino.string();
}

//==============================================================
// Busca un espacio libre dentro de las páginas.
//==============================================================

std::optional<UbicacionRegistro>StorageManager::buscarEspacioLibre()
{
    int paginas = buffer.totalPaginas();

    for (int pageId = 0; pageId < paginas; pageId++)
    {
        Pagina& pagina = buffer.obtenerPagina(pageId);

        // reutilizar registros eliminados
        for (int slot = 0;
             slot < pagina.cantidadRegistros;
             slot++)
        {
            if (!pagina.registros[slot].activo)
            {
                return UbicacionRegistro{
                    pageId,
                    slot
                };
            }
        }

        // espacio disponible al final
        if (pagina.cantidadRegistros <
            REGISTROS_POR_PAGINA)
        {
            return UbicacionRegistro{
                pageId,
                pagina.cantidadRegistros
            };
        }
    }

    return std::nullopt;
}

//==============================================================
// Inserta un nuevo libro.
//==============================================================

bool StorageManager::agregarLibro(const Libro& libro)
{
    // Validar datos
    if (!datosValidos(libro))
    {
        return false;
    }

    // Evitar IDs duplicados
    if (indiceHash.count(libro.id) > 0)
    {
        return false;
    }

    Libro nuevo = libro;

    nuevo.activo = true;

    // Copiar PDF a la biblioteca
    copiarCampo(
        nuevo.rutaPdf,
        TAM_RUTA_PDF,
        copiarPdf(libro));

    //----------------------------------------------------------
    // Buscar espacio libre
    //----------------------------------------------------------

    auto ubicacion = buscarEspacioLibre();

    if (!ubicacion.has_value())
    {
        int nuevaPagina = buffer.crearPagina();

        ubicacion = UbicacionRegistro{
            nuevaPagina,
            0
        };
    }

    //----------------------------------------------------------
    // Obtener la página
    //----------------------------------------------------------

    Pagina& pagina =
        buffer.obtenerPagina(
            ubicacion->pageId);

    //----------------------------------------------------------
    // Escribir registro
    //----------------------------------------------------------

    pagina.registros[
        ubicacion->slot] = nuevo;

    //----------------------------------------------------------
    // Actualizar cantidad de registros
    //----------------------------------------------------------

    if (ubicacion->slot >=
        pagina.cantidadRegistros)
    {
        pagina.cantidadRegistros =
            ubicacion->slot + 1;
    }

    //----------------------------------------------------------
    // Actualizar índice
    //----------------------------------------------------------

    indiceHash[nuevo.id] = *ubicacion;

    guardarIndiceHash();

    //----------------------------------------------------------
    // Marcar página sucia
    //----------------------------------------------------------

    buffer.marcarSucio(
        ubicacion->pageId);

    return true;
}

//==============================================================
// Eliminación lógica de un libro.
//==============================================================

bool StorageManager::eliminarLibro(int id)
{
    //----------------------------------------------------------
    // Buscar el libro en el índice
    //----------------------------------------------------------

    auto it = indiceHash.find(id);

    if (it == indiceHash.end())
    {
        return false;
    }

    //----------------------------------------------------------
    // Obtener la página
    //----------------------------------------------------------

    Pagina& pagina =
        buffer.obtenerPagina(it->second.pageId);

    Libro& libro =
        pagina.registros[it->second.slot];

    //----------------------------------------------------------
    // Verificar que siga activo
    //----------------------------------------------------------

    if (!libro.activo)
    {
        return false;
    }

    //----------------------------------------------------------
    // Eliminación lógica
    //----------------------------------------------------------

    libro.activo = false;

    //----------------------------------------------------------
    // Actualizar índice
    //----------------------------------------------------------

    indiceHash.erase(it);

    guardarIndiceHash();

    //----------------------------------------------------------
    // Marcar página como modificada
    //----------------------------------------------------------

    buffer.marcarSucio(
        pagina.pageId);

    return true;
}

//==============================================================
// Actualiza la información de un libro existente.
//==============================================================

bool StorageManager::actualizarLibro(int id,const Libro& libroActualizado)
{
    //----------------------------------------------------------
    // Buscar el libro
    //----------------------------------------------------------

    auto it = indiceHash.find(id);

    if (it == indiceHash.end())
    {
        return false;
    }

    //----------------------------------------------------------
    // Validar datos
    //----------------------------------------------------------

    if (!datosValidos(libroActualizado))
    {
        return false;
    }

    //----------------------------------------------------------
    // Obtener página
    //----------------------------------------------------------

    Pagina& pagina =
        buffer.obtenerPagina(it->second.pageId);

    //----------------------------------------------------------
    // Crear nuevo registro
    //----------------------------------------------------------

    Libro actualizado = libroActualizado;

    // El ID nunca cambia
    actualizado.id = id;

    actualizado.activo = true;

    copiarCampo(
        actualizado.rutaPdf,
        TAM_RUTA_PDF,
        copiarPdf(libroActualizado));

    //----------------------------------------------------------
    // Reemplazar registro
    //----------------------------------------------------------

    pagina.registros[it->second.slot] =
        actualizado;

    //----------------------------------------------------------
    // Marcar página sucia
    //----------------------------------------------------------

    buffer.marcarSucio(
        pagina.pageId);

    //----------------------------------------------------------
    // Guardar índice
    //----------------------------------------------------------

    guardarIndiceHash();

    return true;
}

//==============================================================
// Busca un libro por su ID utilizando el índice Hash.
//==============================================================

std::optional<Libro> StorageManager::buscarPorID(int id)
{
    //----------------------------------------------------------
    // Buscar en el índice
    //----------------------------------------------------------

    auto it = indiceHash.find(id);

    if (it == indiceHash.end())
    {
        return std::nullopt;
    }

    //----------------------------------------------------------
    // Obtener la página
    //----------------------------------------------------------

    Pagina& pagina =
        buffer.obtenerPagina(it->second.pageId);

    const Libro& libro =
        pagina.registros[it->second.slot];

    //----------------------------------------------------------
    // Verificar que siga activo
    //----------------------------------------------------------

    if (!libro.activo)
    {
        return std::nullopt;
    }

    return libro;
}

//==============================================================
// Carga todos los libros activos.
//==============================================================

std::vector<Libro> StorageManager::cargarLibros()
{
    std::vector<Libro> libros;

    int paginas = buffer.totalPaginas();

    for (int pageId = 0;
         pageId < paginas;
         pageId++)
    {
        Pagina& pagina =
            buffer.obtenerPagina(pageId);

        for (int i = 0;
             i < pagina.cantidadRegistros;
             i++)
        {
            if (pagina.registros[i].activo)
            {
                libros.push_back(
                    pagina.registros[i]);
            }
        }
    }

    return libros;
}

//==============================================================
// Devuelve todos los libros.
//==============================================================

std::vector<Libro> StorageManager::listarLibros()
{
    return cargarLibros();
}

//==============================================================
// Reemplaza el catálogo completo. Se conserva por compatibilidad
// con las demostraciones y deja el índice y el buffer coherentes.
//==============================================================

void StorageManager::guardarLibros(const std::vector<Libro>& libros)
{
    buffer.limpiarBuffer();

    std::ofstream datos(archivoDatos,
                        std::ios::binary | std::ios::trunc);
    datos.close();

    indiceHash.clear();
    guardarIndiceHash();

    for (const Libro& libro : libros)
    {
        agregarLibro(libro);
    }

    buffer.flush();
}

BufferManager& StorageManager::obtenerBuffer()
{
    return buffer;
}

//==============================================================
// Abre el PDF asociado al libro.
//==============================================================

bool StorageManager::abrirPdf(int id)
{
    auto libro = buscarPorID(id);

    if (!libro.has_value())
    {
        return false;
    }

    std::filesystem::path ruta(libro->rutaPdf);

    if (!std::filesystem::exists(ruta))
    {
        return false;
    }

#ifdef _WIN32
    std::string comando =
        "start \"\" \"" + ruta.string() + "\"";
#elif __APPLE__
    std::string comando =
        "open \"" + ruta.string() + "\"";
#else
    std::string comando =
        "xdg-open \"" + ruta.string() + "\"";
#endif

    return std::system(comando.c_str()) == 0;
}

//==============================================================
// Guarda el índice Hash en disco.
//==============================================================

void StorageManager::guardarIndiceHash()
{
    std::ofstream archivo(
        archivoIndice,
        std::ios::binary | std::ios::trunc);

    if (!archivo)
    {
        return;
    }

    std::size_t cantidad = indiceHash.size();

    archivo.write(
        reinterpret_cast<const char*>(&cantidad),
        sizeof(cantidad));

    for (const auto& [id, ubicacion] : indiceHash)
    {
        archivo.write(
            reinterpret_cast<const char*>(&id),
            sizeof(id));

        archivo.write(
            reinterpret_cast<const char*>(&ubicacion),
            sizeof(UbicacionRegistro));
    }
}

//==============================================================
// Carga el índice Hash desde disco.
//==============================================================

void StorageManager::cargarIndiceHash()
{
    indiceHash.clear();

    std::ifstream archivo(
        archivoIndice,
        std::ios::binary);

    if (!archivo)
    {
        return;
    }

    std::size_t cantidad = 0;

    archivo.read(
        reinterpret_cast<char*>(&cantidad),
        sizeof(cantidad));

    for (std::size_t i = 0; i < cantidad; i++)
    {
        int id;

        UbicacionRegistro ubicacion;

        archivo.read(
            reinterpret_cast<char*>(&id),
            sizeof(id));

        archivo.read(
            reinterpret_cast<char*>(&ubicacion),
            sizeof(UbicacionRegistro));

        if (archivo)
        {
            indiceHash[id] = ubicacion;
        }
    }
}

//==============================================================
// Reconstruye el índice Hash recorriendo libros.dat.
//==============================================================

void StorageManager::reconstruirIndiceHash()
{
    indiceHash.clear();

    int paginas = buffer.totalPaginas();

    for (int pageId = 0;
         pageId < paginas;
         pageId++)
    {
        Pagina& pagina =
            buffer.obtenerPagina(pageId);

        for (int slot = 0;
             slot < pagina.cantidadRegistros;
             slot++)
        {
            const Libro& libro =
                pagina.registros[slot];

            if (libro.activo)
            {
                indiceHash[libro.id] =
                {
                    pageId,
                    slot
                };
            }
        }
    }

    guardarIndiceHash();
}
