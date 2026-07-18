#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include "BufferManager.h"

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct UbicacionRegistro {
    int pageId;
    int slot;
};

class StorageManager {
public:
    explicit StorageManager(const std::filesystem::path& carpetaBase = "data",
                            const std::filesystem::path& carpetaBiblioteca = "biblioteca");

    bool agregarLibro(const Libro& libro);
    bool eliminarLibro(int id);
    bool actualizarLibro(int id, const Libro& libroActualizado);

    std::optional<Libro> buscarPorID(int id);
    bool abrirPdf(int id);
    std::vector<Libro> cargarLibros();
    std::vector<Libro> listarLibros();

    void guardarLibros(const std::vector<Libro>& libros);
    void reconstruirIndiceHash();
    void guardarIndiceHash();
    void cargarIndiceHash();

private:
    std::filesystem::path carpetaBase;
    std::filesystem::path carpetaBiblioteca;
    std::filesystem::path archivoDatos;
    std::filesystem::path archivoIndice;
    BufferManager buffer;
    std::unordered_map<int, UbicacionRegistro> indiceHash;

    bool idValido(int id) const;
    bool datosValidos(const Libro& libro) const;
    std::string normalizarGenero(const std::string& genero) const;
    std::string prepararPdfEnBiblioteca(const Libro& libro);
    std::optional<UbicacionRegistro> buscarEspacioLibre();
};

#endif
