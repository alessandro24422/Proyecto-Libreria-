#ifndef LIBRO_H
#define LIBRO_H

#include <cstring>
#include <string>

//==============================================================
// Configuración de tamaños de los campos.
// Se utilizan arreglos de tamaño fijo para facilitar el
// almacenamiento binario de los registros.
//==============================================================

constexpr int TAM_TITULO = 100;
constexpr int TAM_AUTOR = 80;
constexpr int TAM_CODIGO_DEWEY = 10;
constexpr int TAM_RUTA_PDF = 220;

//==============================================================
// Registro de un libro.
//
// Cada instancia representa un registro físico almacenado
// dentro del archivo binario "libros.dat".
//==============================================================

#pragma pack(push, 1)

struct Libro
{
    int id;                                     // Identificador único

    char titulo[TAM_TITULO];                    // Título del libro

    char autor[TAM_AUTOR];                      // Autor principal

    char codigoDewey[TAM_CODIGO_DEWEY];         // Ej: 005, 510, 985

    int anio;                                   // Año de publicación

    char rutaPdf[TAM_RUTA_PDF];                 // Ruta del archivo PDF

    bool activo;                                // true = vigente
                                                 // false = eliminado lógicamente
};

#pragma pack(pop)

//==============================================================
// Copia de forma segura una cadena hacia un campo del registro.
//==============================================================

inline void copiarCampo(char* destino,std::size_t tam,const std::string& texto)
{
    std::memset(destino, 0, tam);
    std::strncpy(destino, texto.c_str(), tam - 1);
}

//==============================================================
// Constructor auxiliar para crear registros Libro.
//==============================================================

inline Libro crearLibro(int id,const std::string& titulo,const std::string& autor,const std::string& codigoDewey,int anio,const std::string& rutaPdf)
{
    Libro libro{};

    libro.id = id;

    copiarCampo(libro.titulo,TAM_TITULO,titulo);

    copiarCampo(libro.autor,TAM_AUTOR,autor);

    copiarCampo(libro.codigoDewey,TAM_CODIGO_DEWEY,codigoDewey);

    libro.anio = anio;

    copiarCampo(libro.rutaPdf,TAM_RUTA_PDF,rutaPdf);

    libro.activo = true;

    return libro;
}

#endif // LIBRO_H