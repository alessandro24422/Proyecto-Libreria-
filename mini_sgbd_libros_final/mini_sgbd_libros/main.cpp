//==============================================================
// MINI SGBD DE LIBROS - MAIN DEFINITIVO
//
// Prueba integral del modulo de almacenamiento (StorageManager +
// BufferManager) usando PDFs reales (no datos ficticios), y
// verificando la organizacion fisica de la "biblioteca" mediante
// el Sistema de Clasificacion Decimal Dewey (CDD).
//
// Requiere que exista la carpeta "pdfs_reales/" en el mismo
// directorio de ejecucion, con los PDFs listados en el catalogo
// de abajo.
//==============================================================

#include "StorageManager.h"
#include "Libro.h"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace fs = std::filesystem;

//==============================================================
// Utilidades de impresion
//==============================================================

static void linea(char c = '-', int n = 70)
{
    std::cout << std::string(n, c) << "\n";
}

static void seccion(const std::string& texto)
{
    std::cout << "\n";
    linea('=');
    std::cout << texto << "\n";
    linea('=');
}

//==============================================================
// Descripcion de un libro real para poblar la biblioteca.
//==============================================================

struct LibroDemo
{
    int id;
    std::string titulo;
    std::string autor;
    std::string dewey;
    int anio;
    std::string rutaPdf;
};

//==============================================================
// Nombres legibles de las 10 clases principales del sistema
// Dewey (solo para mostrar el listado agrupado de forma clara).
//==============================================================

static const std::map<std::string, std::string> NOMBRES_DEWEY =
{
    {"000", "000 - Generalidades / Computacion"},
    {"100", "100 - Filosofia y Psicologia"},
    {"200", "200 - Religion"},
    {"300", "300 - Ciencias sociales"},
    {"400", "400 - Lenguas"},
    {"500", "500 - Ciencias naturales y Matematicas"},
    {"600", "600 - Tecnologia y ciencias aplicadas"},
    {"700", "700 - Artes y recreacion"},
    {"800", "800 - Literatura"},
    {"900", "900 - Historia y geografia"},
};

int main()
{
    seccion("MINI SGBD DE LIBROS - PRUEBA FINAL CON PDFs REALES");

    //----------------------------------------------------------
    // 0) Entorno limpio.
    //
    // Se borran "data/" y "biblioteca/" para que la prueba sea
    // siempre reproducible desde cero, sin importar corridas
    // anteriores.
    //----------------------------------------------------------

    std::error_code ec;
    fs::remove_all("data", ec);
    fs::remove_all("biblioteca", ec);

    StorageManager storage; // usa "data" y "biblioteca" por defecto

    std::cout << "IDs cargados al iniciar : " << storage.cantidadIDs() << "\n";
    std::cout << "Libros existentes       : " << storage.listarLibros().size() << "\n";

    //----------------------------------------------------------
    // 1) Catalogo de libros REALES (PDFs genuinos) clasificados
    //    con el sistema Dewey.
    //
    // Todos, menos el ultimo, son PDFs completos y reales.
    // El ultimo (Clean Code) es un PDF de prueba (stub) que se
    // incluye solo para demostrar que el sistema tambien
    // clasifica correctamente libros fuera de Literatura (800).
    //----------------------------------------------------------

    std::vector<LibroDemo> catalogo =
    {
        {101, "Ficciones",                  "Jorge Luis Borges",         "863",   1944, "pdfs_reales/borges_ficciones.pdf"},
        {102, "Cuentos y Poemas",           "Jorge Luis Borges",         "863",   1998, "pdfs_reales/borges_cuentos_y_poemas.pdf"},
        {103, "El inmortal",                "Jorge Luis Borges",         "863",   1949, "pdfs_reales/borges_el_inmortal.pdf"},
        {104, "El monte de las animas",     "Gustavo Adolfo Becquer",    "863",   1861, "pdfs_reales/becquer_monte_de_las_animas.pdf"},
        {105, "Cien anios de soledad",      "Gabriel Garcia Marquez",    "863",   1967, "pdfs_reales/cien_anios_de_soledad.pdf"},
        {106, "El principito",              "Antoine de Saint-Exupery",  "843",   1943, "pdfs_reales/el_principito.pdf"},
        {107, "Primer amor",                "Ivan Turgueniev",           "891.7", 1860, "pdfs_reales/turgenev_primer_amor.pdf"},
        {108, "Clean Code (PDF de prueba)", "Robert C. Martin",         "005.1", 2008, "pdfs_reales/clean_code_stub.pdf"},
    };

    seccion("1) INSERCION DE LIBROS REALES");

    for (const auto& l : catalogo)
    {
        Libro libro = crearLibro(l.id, l.titulo, l.autor, l.dewey, l.anio, l.rutaPdf);

        bool ok = storage.agregarLibro(libro);

        std::cout
            << "ID " << std::setw(3) << l.id
            << " [Dewey " << std::setw(6) << std::left << l.dewey << std::right << "] "
            << std::left << std::setw(28) << l.titulo << std::right
            << " -> " << (ok ? "INSERTADO" : "RECHAZADO")
            << "\n";
    }

    //----------------------------------------------------------
    // 2) Prueba de rechazo de ID duplicado.
    //----------------------------------------------------------

    seccion("2) PRUEBA DE ID DUPLICADO");
    {
        Libro repetido = crearLibro(
            101,
            "Ficciones (intento duplicado)",
            "Jorge Luis Borges",
            "863",
            1944,
            catalogo.front().rutaPdf);

        bool ok = storage.agregarLibro(repetido);

        std::cout
            << "Reinsertar ID 101 -> "
            << (ok ? "INSERTADO (ERROR: no debia permitirlo)"
                   : "RECHAZADO (correcto)")
            << "\n";
    }

    //----------------------------------------------------------
    // 3) Busqueda por ID (indice hash).
    //----------------------------------------------------------

    seccion("3) BUSQUEDA POR ID (indice hash)");

    for (int id : {105, 999})
    {
        auto libro = storage.buscarPorID(id);

        if (libro.has_value())
        {
            std::cout
                << "ID " << id << " -> "
                << libro->titulo << " (" << libro->autor << ")\n";
        }
        else
        {
            std::cout << "ID " << id << " -> NO ENCONTRADO\n";
        }
    }

    //----------------------------------------------------------
    // 4) Actualizacion de un registro existente.
    //----------------------------------------------------------

    seccion("4) ACTUALIZACION DE REGISTRO");
    {
        auto original = storage.buscarPorID(106);

        if (original.has_value())
        {
            Libro actualizado = crearLibro(
                106,
                "El principito (edicion revisada)",
                original->autor,
                original->codigoDewey,
                original->anio,
                original->rutaPdf);

            bool ok = storage.actualizarLibro(106, actualizado);

            std::cout << "Actualizar ID 106 -> " << (ok ? "OK" : "FALLO") << "\n";

            auto verificado = storage.buscarPorID(106);

            if (verificado.has_value())
            {
                std::cout << "Nuevo titulo: " << verificado->titulo << "\n";
            }
        }
    }

    //----------------------------------------------------------
    // 5) Eliminacion logica.
    //----------------------------------------------------------

    seccion("5) ELIMINACION LOGICA");
    {
        bool ok = storage.eliminarLibro(108);

        std::cout << "Eliminar ID 108 -> " << (ok ? "OK" : "FALLO") << "\n";

        auto verificado = storage.buscarPorID(108);

        std::cout
            << "Buscar ID 108 tras eliminar -> "
            << (verificado.has_value()
                    ? "TODAVIA EXISTE (ERROR)"
                    : "NO ENCONTRADO (correcto, eliminacion logica ok)")
            << "\n";
    }

    //----------------------------------------------------------
    // 6) Listado agrupado por carpeta Dewey.
    //----------------------------------------------------------

    seccion("6) CATALOGO AGRUPADO POR CARPETA DEWEY");
    {
        auto libros = storage.listarLibros();

        std::map<std::string, std::vector<Libro>> porCarpeta;

        for (const auto& l : libros)
        {
            std::string carpeta = (std::strlen(l.codigoDewey) > 0)
                ? std::string(1, l.codigoDewey[0]) + "00"
                : "000";

            porCarpeta[carpeta].push_back(l);
        }

        for (const auto& [carpeta, lista] : porCarpeta)
        {
            auto it = NOMBRES_DEWEY.find(carpeta);

            std::cout
                << "\n["
                << (it != NOMBRES_DEWEY.end() ? it->second : carpeta)
                << "]\n";

            for (const auto& l : lista)
            {
                std::cout
                    << "  - (" << l.codigoDewey << ") "
                    << l.titulo << " - " << l.autor
                    << " (" << l.anio << ")\n";
            }
        }
    }

    //----------------------------------------------------------
    // 7) Verificacion fisica: los PDF deben haber quedado
    //    copiados dentro de biblioteca/<carpeta_dewey>/.
    //----------------------------------------------------------

    seccion("7) VERIFICACION FISICA DE ARCHIVOS EN biblioteca/");

    for (const auto& l : storage.listarLibros())
    {
        bool existe = fs::exists(l.rutaPdf);

        std::cout
            << (existe ? "[OK]    " : "[FALTA] ")
            << l.rutaPdf
            << "\n";
    }

    //----------------------------------------------------------
    // 8) Estadisticas del Buffer Manager.
    //----------------------------------------------------------

    seccion("8) ESTADISTICAS DEL BUFFER MANAGER");

    storage.obtenerBuffer().imprimirEstadoBuffer();

    auto estadisticas = storage.obtenerBuffer().obtenerEstadisticas();

    std::cout << "\n";
    std::cout << "Hits             : " << estadisticas.bufferHits << "\n";
    std::cout << "Misses           : " << estadisticas.bufferMisses << "\n";
    std::cout << "Lecturas disco   : " << estadisticas.lecturasDisco << "\n";
    std::cout << "Escrituras disco : " << estadisticas.escriturasDisco << "\n";
    std::cout
        << "Hit rate         : "
        << storage.obtenerBuffer().obtenerHitRate() * 100
        << "%\n";

    //----------------------------------------------------------
    // 9) Prueba de persistencia real.
    //
    // Se crea una SEGUNDA instancia de StorageManager (como si
    // el programa se hubiera cerrado y vuelto a abrir) para
    // confirmar que los datos siguen en disco y se recargan
    // correctamente mediante el indice hash.
    //----------------------------------------------------------

    seccion("9) PRUEBA DE PERSISTENCIA (REINICIO DEL SGBD)");
    {
        // En un programa real, al cerrar la aplicacion el
        // destructor de BufferManager haria flush() de las
        // paginas sucias automaticamente. Como aqui "storage"
        // (la primera instancia) sigue viva dentro del mismo
        // proceso, forzamos el flush manualmente para simular
        // ese cierre antes de "reabrir" el SGBD.
        storage.obtenerBuffer().flush();

        StorageManager storage2;

        auto libros2 = storage2.listarLibros();

        std::cout
            << "Libros recuperados tras reiniciar: "
            << libros2.size() << "\n\n";

        for (const auto& l : libros2)
        {
            std::cout
                << "  - ID " << l.id
                << " | " << l.titulo
                << " | Dewey " << l.codigoDewey
                << "\n";
        }
    }

    //----------------------------------------------------------
    // 10) Apertura de un PDF real con el lector del sistema.
    //
    // Requiere un entorno grafico (Windows/macOS/Linux con
    // sesion de escritorio). En un servidor o contenedor sin
    // interfaz grafica es normal que devuelva false.
    //----------------------------------------------------------

    seccion("10) APERTURA DE PDF (abrirPdf)");
    {
        bool ok = storage.abrirPdf(105);

        std::cout
            << "abrirPdf(105) -> "
            << (ok
                    ? "comando de apertura ejecutado"
                    : "no se pudo abrir (normal si no hay entorno grafico)")
            << "\n";
    }

    seccion("FIN DE LA PRUEBA");

    return 0;
}
