#pragma once

#include "SearchManager.h"

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

// Interfaz SFML (Integrante 3). Usa exclusivamente la API publica de StorageManager.
class UIManager {
public:
    explicit UIManager(StorageManager& storage);
    void ejecutar();

private:
    enum class Vista { Inicio, Catalogo, Formulario, Buscar, Ordenar };
    enum class Campo { Ninguno, Id, Anio, Titulo, Autor, Dewey, Pdf, Consulta };

    StorageManager& storage_;
    SearchManager busquedas_;
    sf::RenderWindow ventana_;
    sf::Font fuente_;
    Vista vista_{Vista::Inicio};
    Campo campoActivo_{Campo::Ninguno};
    std::vector<Libro> librosVista_;
    int seleccionado_{-1};
    int idEditando_{-1};
    std::string id_, anio_, titulo_, autor_, dewey_, pdf_, consulta_, aviso_;
    std::string criterioBusqueda_{"Titulo"};
    std::string criterioOrden_{"Titulo"};

    bool cargarFuente();
    void recargarCatalogo();
    void procesarEventos();
    void manejarClick(sf::Vector2i mouse);
    void manejarTexto(sf::Uint32 unicode);
    void guardarFormulario();
    void prepararEdicion();
    void eliminarSeleccionado();
    void abrirPdfSeleccionado();
    void buscar();
    void ordenar();
    void limpiarFormulario();

    void dibujar();
    void dibujarNavegacion();
    void dibujarInicio();
    void dibujarCatalogo();
    void dibujarFormulario();
    void dibujarBuscar();
    void dibujarOrdenar();
    void texto(const std::string& valor, float x, float y, unsigned tamano, sf::Color color, bool negrita = false);
    void boton(const sf::FloatRect& area, const std::string& etiqueta, bool activo = false);
    void campo(const sf::FloatRect& area, const std::string& etiqueta, const std::string& valor, Campo idCampo, const std::string& ejemplo);
    bool dentro(const sf::FloatRect& area, sf::Vector2i punto) const;
};
