# Mini SGBD de libros

El proyecto incluye almacenamiento, búsqueda, ordenamiento y un gestor de
consultas básico. La interfaz SFML puede consumir estos módulos sin conocer la
organización interna de los archivos.

## Módulo de almacenamiento

- `Libro.h`: estructura fija del registro para escritura binaria.
- `StorageManager.h/.cpp`: operaciones basicas de almacenamiento.
- `BufferManager.h/.cpp`: lectura/escritura de paginas con Buffer Pool y reemplazo FIFO.
- `data/libros.dat`: archivo binario de registros paginados.
- `data/indice_hash.dat`: indice hash persistente para busqueda rapida por ID.
- `biblioteca/<genero>/`: carpetas donde se organizan los PDFs por genero.

## Funciones principales

- `agregarLibro(libro)`
- `eliminarLibro(id)`
- `actualizarLibro(id, libroActualizado)`
- `cargarLibros()`
- `guardarLibros(libros)`
- `listarLibros()`
- `buscarPorID(id)` usando indice hash
- `abrirPdf(id)` para abrir el PDF con el lector predeterminado del sistema

## Módulo de búsquedas, ordenamiento y consultas

- `SearchManager.h/.cpp`: búsquedas y filtros de la cláusula `WHERE`.
- `SortManager.h/.cpp`: ordenamientos para `ORDER BY` con `std::sort`.

`SearchManager` recibe una referencia al `StorageManager`, por lo que no duplica
los datos ni la lógica de persistencia. `buscarPorID()` utiliza el índice hash
persistente; las búsquedas por texto recorren los libros activos y no distinguen
mayúsculas/minúsculas.

```cpp
StorageManager storage("data", "biblioteca");
SearchManager busquedas(storage);

auto libro = busquedas.buscarPorID(7);             // índice hash
auto autores = busquedas.buscarPorAutor("Borges");
auto titulos = busquedas.buscarPorTitulo("cien");
auto porGenero = busquedas.buscarPorGenero("Novela");
auto porAnio = busquedas.buscarPorAnio(1963);

auto ordenados = SortManager::ordenarPorTitulo(storage.listarLibros());
auto recientes = SortManager::ordenarPorAnio(storage.listarLibros(), false);
```

### Consultas propias (SELECT / WHERE / ORDER BY)

Para cubrir el procesamiento de consultas mínimo, `ejecutarConsulta()` acepta:

```text
SELECT *
SELECT * WHERE autor CONTAINS Borges
SELECT * WHERE anio = 1963 ORDER BY titulo ASC
SELECT * WHERE genero = Novela ORDER BY anio DESC
```

Los campos admitidos en `WHERE` son `id`, `titulo`, `autor`, `genero` y `anio`.
En los campos de texto se puede usar `=` (igualdad) o `CONTAINS` (coincidencia
parcial); para `id` y `anio` el valor debe ser numérico. En `ORDER BY` se admiten
`titulo`, `autor` y `anio`, con `ASC` (valor predeterminado) o `DESC`.

```cpp
ResultadoConsulta resultado = busquedas.ejecutarConsulta(
    "SELECT * WHERE autor CONTAINS Garcia ORDER BY anio DESC");

// resultado.libros, resultado.mensaje y resultado.duracion
ComparacionBusquedaId medicion = busquedas.compararBusquedaPorID(7);
// medicion.tiempoIndice frente a medicion.tiempoLineal
```

`compararBusquedaPorID()` permite registrar en el artículo los tiempos de una
búsqueda indexada y una secuencial sobre los mismos datos.

## Compilacion rapida

Con CMake:

```bash
cmake -S . -B build
cmake --build build
./build/mini_sgbd
```

Con g++:

```bash
g++ -std=c++17 main.cpp BufferManager.cpp StorageManager.cpp SearchManager.cpp SortManager.cpp -o mini_sgbd
./mini_sgbd
```

En Windows, el ejecutable puede quedar como `mini_sgbd.exe`.

## Prueba incluida

El archivo `main.cpp` contiene una demo de pruebas para el modulo de almacenamiento.
Al ejecutarlo valida:

- insercion de libros;
- rechazo de IDs duplicados;
- listado desde `libros.dat`;
- busqueda por ID usando el indice hash;
- actualizacion de registros;
- eliminacion logica;
- reconstruccion del indice hash;
- persistencia al recargar el `StorageManager`;
- preparacion de la funcion `abrirPdf(id)`.
- búsquedas por título, autor y año;
- ordenamientos por título y año;
- una consulta `SELECT` con `WHERE` y `ORDER BY`;
- comparación de tiempos de búsqueda indexada y lineal.

## Como se guardan los datos

Cada pagina ocupa 4096 bytes y contiene hasta 9 registros `Libro`. El `BufferManager`
mantiene una cantidad limitada de paginas en memoria. Si el buffer se llena, se expulsa
la pagina que ingreso primero, aplicando FIFO. Si una pagina expulsada fue modificada,
se escribe antes en `libros.dat`.

El indice hash guarda pares `ID -> pagina/slot`, por eso `buscarPorID()` no necesita
recorrer todo el archivo.

## PDFs por genero

Al agregar o actualizar un libro, el gestor crea la carpeta:

```text
biblioteca/<genero_normalizado>/
```

Si la ruta del PDF existe, lo copia a esa carpeta. Si aun no existe, guarda la ruta
esperada para que la interfaz o el equipo pueda colocar luego el archivo ahi.

## Abrir un PDF

La funcion `abrirPdf(id)` busca el libro por su ID usando el indice hash y abre el
archivo guardado en `rutaPdf`.

```cpp
if (!storage.abrirPdf(2)) {
    std::cout << "No se encontro el libro o el archivo PDF no existe.\n";
}
```

En Windows usa `start`, en macOS usa `open` y en Linux usa `xdg-open`.
