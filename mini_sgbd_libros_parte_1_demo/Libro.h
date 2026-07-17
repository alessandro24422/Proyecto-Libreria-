#ifndef LIBRO_H
#define LIBRO_H

#include <cstring>
#include <string>

constexpr int TAM_TITULO = 80;
constexpr int TAM_AUTOR = 80;
constexpr int TAM_GENERO = 40;
// Se conserva el tamano total del registro para no invalidar los datos ya
// almacenados. El espacio liberado de rutaPdf se utiliza para editorial.
constexpr int TAM_RUTA_PDF = 140;
constexpr int TAM_EDITORIAL = 80;

#pragma pack(push, 1)
struct Libro {
    int id;
    char titulo[TAM_TITULO];
    char autor[TAM_AUTOR];
    char genero[TAM_GENERO];
    int anio;
    char rutaPdf[TAM_RUTA_PDF];
    char editorial[TAM_EDITORIAL];
    bool activo;
};
#pragma pack(pop)

inline void copiarCampo(char* destino, int tam, const std::string& valor) {
    std::memset(destino, 0, tam);
    std::strncpy(destino, valor.c_str(), tam - 1);
}

inline Libro crearLibro(int id,
                        const std::string& titulo,
                        const std::string& autor,
                        const std::string& genero,
                        int anio,
                        const std::string& rutaPdf,
                        const std::string& editorial = "") {
    Libro libro{};
    libro.id = id;
    copiarCampo(libro.titulo, TAM_TITULO, titulo);
    copiarCampo(libro.autor, TAM_AUTOR, autor);
    copiarCampo(libro.genero, TAM_GENERO, genero);
    libro.anio = anio;
    copiarCampo(libro.rutaPdf, TAM_RUTA_PDF, rutaPdf);
    copiarCampo(libro.editorial, TAM_EDITORIAL, editorial);
    libro.activo = true;
    return libro;
}

#endif
