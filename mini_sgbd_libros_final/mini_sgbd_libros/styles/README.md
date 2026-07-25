# Interfaz SFML (Integrante 3)

`UIManager` concentra la capa visual: panel Inicio, catalogo, alta, edicion,
eliminacion, apertura de PDF, busqueda y ordenamiento.

La interfaz usa `StorageManager` para CRUD, indice hash y PDF. El modelo final
usa `codigoDewey` (no genero), por lo que el formulario solicita ese campo.
Cuando el equipo entregue `SearchManager` y `SortManager`, las operaciones de
busqueda por texto y ordenamiento pueden delegarse a esos modulos sin cambiar
las pantallas.
