#include "StorageManager.h"

#include <iostream>

void imprimirLibro(const Libro& libro) {
    std::cout << "ID: " << libro.id
              << " | Titulo: " << libro.titulo
              << " | Autor: " << libro.autor
              << " | Genero: " << libro.genero
              << " | Anio: " << libro.anio
              << " | PDF: " << libro.rutaPdf << '\n';
}

int main() {
    StorageManager storage("data", "biblioteca");

    storage.agregarLibro(crearLibro(1, "Cien anios de soledad", "Gabriel Garcia Marquez",
                                    "Novela", 1967, "cien_anios.pdf"));
    storage.agregarLibro(crearLibro(2, "El principito", "Antoine de Saint-Exupery",
                                    "Fantasia", 1943, "el_principito.pdf"));

    std::cout << "Listado de libros activos:\n";
    for (const Libro& libro : storage.listarLibros()) {
        imprimirLibro(libro);
    }

    std::cout << "\nBusqueda por indice hash, ID = 2:\n";
    auto encontrado = storage.buscarPorID(2);
    if (encontrado.has_value()) {
        imprimirLibro(encontrado.value());
    }

    storage.actualizarLibro(2, crearLibro(2, "El principito", "Antoine de Saint-Exupery",
                                          "Literatura infantil", 1943, "el_principito.pdf"));
    storage.eliminarLibro(1);

    std::cout << "\nDespues de actualizar y eliminar:\n";
    for (const Libro& libro : storage.listarLibros()) {
        imprimirLibro(libro);
    }

    return 0;
}
