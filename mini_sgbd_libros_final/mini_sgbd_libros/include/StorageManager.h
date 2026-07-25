#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include "BufferManager.h"

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

//==============================================================
// Ubicación física de un registro dentro de una página.
//==============================================================

struct UbicacionRegistro
{
    int pageId;

    int slot;
};

//==============================================================
// Storage Manager.
//
// Responsable del almacenamiento físico de los libros.
//
// Gestiona:
//
// - libros.dat
// - indice_hash.dat
// - Buffer Manager
//==============================================================

class StorageManager
{
public:

    explicit StorageManager(
        const std::filesystem::path& carpetaBase = "data",
        const std::filesystem::path& carpetaBiblioteca = "biblioteca");

    //----------------------------
    // Operaciones CRUD
    //----------------------------

    bool agregarLibro(const Libro& libro);

    bool eliminarLibro(int id);

    bool actualizarLibro(
        int id,
        const Libro& libroActualizado);

    //----------------------------
    // Consultas básicas
    //----------------------------

    std::optional<Libro>
    buscarPorID(int id);

    std::vector<Libro>
    listarLibros();

    std::vector<Libro>
    cargarLibros();

    //----------------------------
    // Compatibilidad
    //----------------------------

    void guardarLibros(
        const std::vector<Libro>& libros);

    BufferManager& obtenerBuffer();

    //----------------------------
    // Utilidades
    //----------------------------

    size_t cantidadIDs() const
    {
        return indiceHash.size();
    }
    bool abrirPdf(int id);

private:

    //----------------------------
    // Rutas
    //----------------------------

    std::filesystem::path carpetaBase;

    std::filesystem::path carpetaBiblioteca;

    std::filesystem::path archivoDatos;

    std::filesystem::path archivoIndice;

    //----------------------------
    // Componentes
    //----------------------------

    BufferManager buffer;

    std::unordered_map<int, UbicacionRegistro>
        indiceHash;

    //----------------------------
    // Índice Hash
    //----------------------------

    void reconstruirIndiceHash();

    void guardarIndiceHash();

    void cargarIndiceHash();

    //----------------------------
    // Validaciones
    //----------------------------

    bool idValido(int id) const;

    bool datosValidos(const Libro& libro) const;

    //----------------------------
    // Espacio libre
    //----------------------------

    std::optional<UbicacionRegistro>
    buscarEspacioLibre();

    //----------------------------
    // Biblioteca
    //----------------------------

    std::string obtenerCarpetaDewey(
        const Libro& libro) const;

    std::string copiarPdf(
        const Libro& libro);
};

#endif
