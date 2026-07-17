#include "SortManager.h"

#include <algorithm>
#include <cstring>

void SortManager::ordenar(std::vector<Libro>& libros,
                          CampoOrdenamiento campo,
                          DireccionOrdenamiento direccion) const {
    const auto comparar = [campo, direccion](const Libro& izquierdo, const Libro& derecho) {
        int resultado = 0;
        switch (campo) {
            case CampoOrdenamiento::Id:
                resultado = (izquierdo.id > derecho.id) - (izquierdo.id < derecho.id);
                break;
            case CampoOrdenamiento::Titulo:
                resultado = std::strcmp(izquierdo.titulo, derecho.titulo);
                break;
            case CampoOrdenamiento::Autor:
                resultado = std::strcmp(izquierdo.autor, derecho.autor);
                break;
            case CampoOrdenamiento::Anio:
                resultado = (izquierdo.anio > derecho.anio) - (izquierdo.anio < derecho.anio);
                break;
        }
        return direccion == DireccionOrdenamiento::Ascendente ? resultado < 0 : resultado > 0;
    };
    std::sort(libros.begin(), libros.end(), comparar);
}
