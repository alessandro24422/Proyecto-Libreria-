# Mini SGBD de libros

El proyecto incluye almacenamiento y procesamiento de consultas:

- `Libro.h`: estructura fija del registro para escritura binaria.
- `StorageManager.h/.cpp`: operaciones basicas de almacenamiento.
- `BufferManager.h/.cpp`: lectura/escritura de paginas con Buffer Pool y reemplazo FIFO.
- `data/libros.dat`: archivo binario de registros paginados.
- `data/indice_hash.dat`: indice hash persistente para busqueda rapida por ID.
- `biblioteca/<genero>/`: carpetas donde se organizan los PDFs por genero.
- `QueryResult.h`: resultado y metricas de una consulta.
- `SearchManager.h/.cpp`: busquedas por indice Hash y filtros parciales.
- `SortManager.h/.cpp`: ordenamiento con `std::sort`.
- `QueryManager.h/.cpp`: parser y ejecutor de consultas `SELECT`.

## Funciones principales

- `agregarLibro(libro)`
- `eliminarLibro(id)`
- `actualizarLibro(id, libroActualizado)`
- `cargarLibros()`
- `guardarLibros(libros)`
- `listarLibros()`
- `buscarPorID(id)` usando indice hash

## Motor de consultas

```cpp
StorageManager storage("data", "biblioteca");
QueryManager consultas(storage);
QueryResult resultado = consultas.ejecutar("SELECT * WHERE titulo = \"C++\"");
```

Consultas admitidas (las palabras clave no distinguen mayusculas):

- `SELECT *`
- `SELECT * WHERE id = 5`
- `SELECT * WHERE titulo = "C++"`
- `SELECT * WHERE autor = "Bjarne"`
- `SELECT * WHERE anio = 2024`
- `SELECT * WHERE editorial = "Pearson"`
- `SELECT * ORDER BY titulo ASC`
- `SELECT * ORDER BY anio DESC`

Los filtros textuales, de anio y editorial son parciales e insensibles a
mayusculas. `QueryResult` expone los registros, tiempo en microsegundos,
registros leidos/encontrados, uso de indice y la consulta original.

`Libro` ahora contiene `editorial`. La estructura conserva el mismo tamano
binario que la version anterior, por compatibilidad con los archivos de datos;
la ruta PDF admite hasta 140 caracteres.

## Compilacion rapida

Con CMake:

```bash
cmake -S . -B build
cmake --build build
./build/mini_sgbd
```

Con g++:

```bash
g++ -std=c++17 main.cpp BufferManager.cpp StorageManager.cpp SearchManager.cpp SortManager.cpp QueryManager.cpp -o mini_sgbd
./mini_sgbd
```

En Windows, el ejecutable puede quedar como `mini_sgbd.exe`.

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
