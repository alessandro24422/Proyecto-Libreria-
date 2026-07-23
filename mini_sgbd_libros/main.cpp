#include "StorageManager.h"
#include "SearchManager.h"
#include "SortManager.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

void imprimirLibro(const Libro& libro) {
    std::cout << "ID: " << libro.id
              << " | Titulo: " << libro.titulo
              << " | Autor: " << libro.autor
              << " | Genero: " << libro.genero
              << " | Anio: " << libro.anio
              << " | PDF: " << libro.rutaPdf << '\n';
}

void imprimirResultado(const std::string& prueba, bool correcto) {
    std::cout << (correcto ? "[OK] " : "[ERROR] ") << prueba << '\n';
}

void crearPdfDemo(const std::filesystem::path& ruta) {
    std::filesystem::create_directories(ruta.parent_path());
    std::ofstream archivo(ruta, std::ios::binary);
    archivo << "%PDF-1.4\n"
            << "1 0 obj\n"
            << "<< /Type /Catalog /Pages 2 0 R >>\n"
            << "endobj\n"
            << "2 0 obj\n"
            << "<< /Type /Pages /Count 0 >>\n"
            << "endobj\n"
            << "%%EOF\n";
}

int main() {
    StorageManager storage("data", "biblioteca");

    std::cout << "=== Pruebas del modulo de almacenamiento ===\n\n";

    // Limpia la base para que la demo siempre tenga el mismo resultado.
    storage.guardarLibros({});

    crearPdfDemo("pdfs_demo/cien_anios.pdf");
    crearPdfDemo("pdfs_demo/principito.pdf");
    crearPdfDemo("pdfs_demo/clean_code.pdf");

    std::vector<Libro> libros = {
        crearLibro(1, "Cien anios de soledad", "Gabriel Garcia Marquez", "Novela", 1967, "pdfs_demo/cien_anios.pdf"),
        crearLibro(2, "El principito", "Antoine de Saint-Exupery", "Fantasia", 1943, "pdfs_demo/principito.pdf"),
        crearLibro(3, "Clean Code", "Robert C. Martin", "Programacion", 2008, "pdfs_demo/clean_code.pdf"),
        crearLibro(4, "Ficciones", "Jorge Luis Borges", "Cuento", 1944, "ficciones.pdf"),
        crearLibro(5, "Rayuela", "Julio Cortazar", "Novela", 1963, "rayuela.pdf"),
        crearLibro(6, "El senor de los anillos", "J. R. R. Tolkien", "Fantasia", 1954, "lotr.pdf"),
        crearLibro(7, "Dune", "Frank Herbert", "Ciencia ficcion", 1965, "dune.pdf"),
        crearLibro(8, "1984", "George Orwell", "Distopia", 1949, "1984.pdf"),
        crearLibro(9, "Fundacion", "Isaac Asimov", "Ciencia ficcion", 1951, "fundacion.pdf"),
        crearLibro(10, "Don Quijote de la Mancha", "Miguel de Cervantes", "Clasico", 1605, "quijote.pdf"),
        crearLibro(11, "La ciudad y los perros", "Mario Vargas Llosa", "Novela", 1963, "ciudad_perros.pdf"),
        crearLibro(12, "Pedro Paramo", "Juan Rulfo", "Novela", 1955, "pedro_paramo.pdf")
    };

    std::cout << "1) Insercion de registros\n";
    for (const Libro& libro : libros) {
        imprimirResultado(std::string("Agregar ID ") + std::to_string(libro.id),
                          storage.agregarLibro(libro));
    }

    std::cout << "\n2) Validacion de ID duplicado\n";
    bool duplicadoRechazado = !storage.agregarLibro(
        crearLibro(1, "Libro duplicado", "Autor X", "Prueba", 2024, "duplicado.pdf"));
    imprimirResultado("Rechazar otro libro con ID 1", duplicadoRechazado);

    std::cout << "\n3) Listado desde libros.dat\n";
    std::vector<Libro> activos = storage.listarLibros();
    imprimirResultado("Cantidad esperada: 12 libros activos", activos.size() == 12);
    for (const Libro& libro : activos) {
        imprimirLibro(libro);
    }

    std::cout << "\n4) Busqueda por indice hash\n";
    auto encontrado = storage.buscarPorID(5);
    if (encontrado.has_value()) {
        imprimirLibro(encontrado.value());
    }
    imprimirResultado("Encontrar ID 5 sin recorrer todo el archivo", encontrado.has_value());

    std::cout << "\n5) Actualizacion de registro\n";
    bool actualizado = storage.actualizarLibro(
        5, crearLibro(5, "Rayuela", "Julio Cortazar", "Literatura latinoamericana", 1963, "rayuela.pdf"));
    imprimirResultado("Actualizar genero del ID 5", actualizado);
    imprimirLibro(storage.buscarPorID(5).value());

    std::cout << "\n6) Eliminacion logica\n";
    imprimirResultado("Eliminar ID 3", storage.eliminarLibro(3));
    imprimirResultado("ID 3 ya no aparece en busqueda", !storage.buscarPorID(3).has_value());

    std::cout << "\n7) Persistencia e indice hash\n";
    storage.reconstruirIndiceHash();
    StorageManager storageRecargado("data", "biblioteca");
    imprimirResultado("Cargar 11 libros activos despues de recargar",
                      storageRecargado.listarLibros().size() == 11);
    imprimirResultado("Buscar ID 7 despues de recargar",
                      storageRecargado.buscarPorID(7).has_value());

    std::cout << "\n8) Apertura de PDF\n";
    std::cout << "La funcion existe, pero no se ejecuta automaticamente para no abrir ventanas.\n";
    std::cout << "Ejemplo para la interfaz SFML: storage.abrirPdf(1);\n";

    // Pruebas del modulo de consultas. Se usa la instancia recargada para
    // demostrar que estas operaciones trabajan sobre datos persistidos.
    SearchManager busquedas(storageRecargado);

    std::cout << "\n9) Busquedas con filtros WHERE\n";
    const auto porTitulo = busquedas.buscarPorTitulo("quijote");
    imprimirResultado("Buscar titulo parcial 'quijote'", porTitulo.size() == 1 && porTitulo[0].id == 10);
    const auto porAutor = busquedas.buscarPorAutor("Borges");
    imprimirResultado("Buscar autor sin distinguir mayusculas", porAutor.size() == 1 && porAutor[0].id == 4);
    const auto porAnio = busquedas.buscarPorAnio(1963);
    imprimirResultado("Filtrar por anio 1963", porAnio.size() == 2);

    std::cout << "\n10) Ordenamiento\n";
    const auto porTituloOrdenado = SortManager::ordenarPorTitulo(storageRecargado.listarLibros());
    const bool titulosOrdenados = std::is_sorted(porTituloOrdenado.begin(), porTituloOrdenado.end(),
        [](const Libro& primero, const Libro& segundo) {
            return std::string(primero.titulo) < std::string(segundo.titulo);
        });
    imprimirResultado("Ordenar libros por titulo", titulosOrdenados);

    const auto porAnioDescendente = SortManager::ordenarPorAnio(storageRecargado.listarLibros(), false);
    const bool aniosDescendentes = std::is_sorted(porAnioDescendente.begin(), porAnioDescendente.end(),
        [](const Libro& primero, const Libro& segundo) { return primero.anio > segundo.anio; });
    imprimirResultado("Ordenar libros por anio descendente", aniosDescendentes);

    std::cout << "\n11) Consulta SELECT propia\n";
    const ResultadoConsulta consulta = busquedas.ejecutarConsulta(
        "SELECT * WHERE genero = Novela ORDER BY anio DESC");
    imprimirResultado("SELECT con WHERE y ORDER BY", consulta.mensaje == "Consulta ejecutada correctamente." &&
                                                   consulta.libros.size() == 3 &&
                                                   std::is_sorted(consulta.libros.begin(), consulta.libros.end(),
                                                       [](const Libro& primero, const Libro& segundo) {
                                                           return primero.anio > segundo.anio;
                                                       }));
    std::cout << "Tiempo de la consulta: " << consulta.duracion.count() << " ns\n";

    std::cout << "\n12) Evaluacion de indice hash\n";
    const ComparacionBusquedaId medicion = busquedas.compararBusquedaPorID(7);
    imprimirResultado("Indice y busqueda lineal encuentran el mismo ID",
                      medicion.resultadoIndice.has_value() && medicion.resultadoLineal.has_value() &&
                      medicion.resultadoIndice->id == medicion.resultadoLineal->id);
    std::cout << "Busqueda por indice hash: " << medicion.tiempoIndice.count() << " ns\n"
              << "Busqueda lineal: " << medicion.tiempoLineal.count() << " ns\n";

    return 0;
}
