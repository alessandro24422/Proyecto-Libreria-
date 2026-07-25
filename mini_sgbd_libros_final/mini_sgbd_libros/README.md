# Mini SGBD de libros - modulo de almacenamiento

Este modulo cubre la parte del Integrante 1:

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

## Compilacion rapida

Con CMake:

```bash
cmake -S . -B build
cmake --build build
./build/mini_sgbd
```

Con g++:

```bash
g++ -std=c++17 main.cpp BufferManager.cpp StorageManager.cpp -o mini_sgbd
./mini_sgbd
```

En Windows, el ejecutable puede quedar como `mini_sgbd.exe`.

## Prueba incluida (main definitivo)

El archivo `main.cpp` es la prueba final e integral del modulo de almacenamiento,
usando **PDFs reales** (no datos ficticios) ubicados en `pdfs_reales/`. Al
ejecutarlo:

1. Limpia `data/` y `biblioteca/` para partir de un estado reproducible.
2. Inserta un catalogo de libros reales clasificados con codigo Dewey
   (Ficciones, Cuentos y Poemas y El inmortal de Borges; El monte de las
   animas de Becquer; Cien anios de soledad; El principito; Primer amor de
   Turgueniev; y un PDF de prueba para Clean Code que demuestra la
   clasificacion fuera de Literatura).
3. Rechaza un ID duplicado.
4. Busca por ID (indice hash).
5. Actualiza un registro.
6. Elimina un registro logicamente y confirma que ya no aparece.
7. Lista el catalogo agrupado por carpeta Dewey (000-900).
8. Verifica que los PDFs quedaron copiados fisicamente en `biblioteca/<carpeta>/`.
9. Muestra estadisticas del Buffer Manager (hits, misses, hit rate).
10. Prueba persistencia real: fuerza el `flush()` del buffer y crea una
    **segunda** instancia de `StorageManager` (simulando reiniciar el
    programa) para confirmar que los libros se recuperan desde disco.
11. Prueba `abrirPdf(id)` (requiere entorno grafico; en un servidor/CI es
    normal que falle).

Para que los PDFs se encuentren, ejecuta el binario desde la carpeta
`mini_sgbd_libros/` (donde vive `pdfs_reales/`).

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
