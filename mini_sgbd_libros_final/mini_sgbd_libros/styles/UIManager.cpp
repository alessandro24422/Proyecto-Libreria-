#include "UIManager.h"

#include "SortManager.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <stdexcept>
#include <utility>

namespace {
const sf::Color FONDO(15, 23, 42), PANEL(30, 41, 59), PANEL_CLARO(51, 65, 85);
const sf::Color TEXTO(241, 245, 249), TENUE(148, 163, 184), ACENTO(56, 189, 248), EXITO(52, 211, 153);

std::string minusculas(std::string valor) {
    std::transform(valor.begin(), valor.end(), valor.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return valor;
}

std::string resumir(const char* valor, std::size_t limite) {
    std::string texto(valor);
    return texto.size() > limite ? texto.substr(0, limite - 3) + "..." : texto;
}
}

UIManager::UIManager(StorageManager& storage)
    : storage_(storage), busquedas_(storage), ventana_(sf::VideoMode(1280, 760), "Libros | Mini SGBD", sf::Style::Close) {
    ventana_.setFramerateLimit(60);
    cargarFuente();
    recargarCatalogo();
}

bool UIManager::cargarFuente() {
    const std::array<std::string, 5> rutas = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf",
        "C:/Windows/Fonts/segoeui.ttf", "C:/Windows/Fonts/arial.ttf", "assets/fonts/Roboto-Regular.ttf"
    };
    for (const auto& ruta : rutas) if (fuente_.loadFromFile(ruta)) return true;
    aviso_ = "No se encontro una fuente compatible.";
    return false;
}

void UIManager::recargarCatalogo() { librosVista_ = storage_.listarLibros(); seleccionado_ = -1; }

void UIManager::ejecutar() { while (ventana_.isOpen()) { procesarEventos(); dibujar(); } }

void UIManager::procesarEventos() {
    sf::Event evento{};
    while (ventana_.pollEvent(evento)) {
        if (evento.type == sf::Event::Closed) ventana_.close();
        if (evento.type == sf::Event::TextEntered) manejarTexto(evento.text.unicode);
        if (evento.type == sf::Event::MouseButtonPressed && evento.mouseButton.button == sf::Mouse::Left)
            manejarClick({evento.mouseButton.x, evento.mouseButton.y});
    }
}

void UIManager::manejarTexto(sf::Uint32 unicode) {
    std::string* destino = nullptr;
    switch (campoActivo_) {
        case Campo::Id: destino = &id_; break; case Campo::Anio: destino = &anio_; break;
        case Campo::Titulo: destino = &titulo_; break; case Campo::Autor: destino = &autor_; break;
        case Campo::Dewey: destino = &dewey_; break; case Campo::Pdf: destino = &pdf_; break;
        case Campo::Consulta: destino = &consulta_; break; default: return;
    }
    if (unicode == 8) { if (!destino->empty()) destino->pop_back(); return; }
    if (unicode < 32 || unicode > 126 || destino->size() >= 120) return;
    if ((campoActivo_ == Campo::Id || campoActivo_ == Campo::Anio) && !std::isdigit(static_cast<unsigned char>(unicode))) return;
    destino->push_back(static_cast<char>(unicode));
}

void UIManager::manejarClick(sf::Vector2i mouse) {
    const std::array<sf::FloatRect, 5> nav = {{{18, 142, 194, 46}, {18, 198, 194, 46}, {18, 254, 194, 46}, {18, 310, 194, 46}, {18, 366, 194, 46}}};
    const std::array<Vista, 5> vistas = {Vista::Inicio, Vista::Catalogo, Vista::Formulario, Vista::Buscar, Vista::Ordenar};
    for (std::size_t i = 0; i < nav.size(); ++i) if (dentro(nav[i], mouse)) {
        vista_ = vistas[i]; campoActivo_ = Campo::Ninguno; aviso_.clear();
        if (vista_ == Vista::Catalogo) recargarCatalogo();
        if (vista_ == Vista::Formulario) limpiarFormulario();
        return;
    }
    if (vista_ == Vista::Inicio) {
        if (dentro({265, 177, 258, 145}, mouse)) { vista_ = Vista::Catalogo; recargarCatalogo(); }
        else if (dentro({543, 177, 258, 145}, mouse)) vista_ = Vista::Buscar;
        else if (dentro({821, 177, 258, 145}, mouse)) vista_ = Vista::Ordenar;
        return;
    }
    if (vista_ == Vista::Catalogo) {
        for (std::size_t i = 0; i < librosVista_.size() && i < 8; ++i)
            if (dentro({260, 185 + 42.f * static_cast<float>(i), 960, 40}, mouse)) { seleccionado_ = static_cast<int>(i); return; }
        if (dentro({260, 585, 130, 44}, mouse)) { recargarCatalogo(); aviso_ = "Catalogo actualizado."; }
        else if (dentro({405, 585, 130, 44}, mouse)) prepararEdicion();
        else if (dentro({550, 585, 130, 44}, mouse)) eliminarSeleccionado();
        else if (dentro({695, 585, 150, 44}, mouse)) abrirPdfSeleccionado();
        return;
    }
    if (vista_ == Vista::Formulario) {
        const std::array<std::pair<sf::FloatRect, Campo>, 6> campos = {{{{260, 170, 210, 48}, Campo::Id}, {{500, 170, 210, 48}, Campo::Anio}, {{260, 275, 900, 48}, Campo::Titulo}, {{260, 380, 900, 48}, Campo::Autor}, {{260, 485, 430, 48}, Campo::Dewey}, {{730, 485, 430, 48}, Campo::Pdf}}};
        for (const auto& item : campos) if (dentro(item.first, mouse)) { campoActivo_ = item.second; return; }
        if (dentro({260, 600, 190, 46}, mouse)) guardarFormulario();
        else if (dentro({465, 600, 150, 46}, mouse)) limpiarFormulario();
        return;
    }
    if (vista_ == Vista::Buscar) {
        if (dentro({260, 170, 140, 42}, mouse)) criterioBusqueda_ = "ID";
        else if (dentro({415, 170, 140, 42}, mouse)) criterioBusqueda_ = "Titulo";
        else if (dentro({570, 170, 140, 42}, mouse)) criterioBusqueda_ = "Autor";
        else if (dentro({260, 265, 560, 48}, mouse)) campoActivo_ = Campo::Consulta;
        else if (dentro({840, 265, 145, 48}, mouse)) buscar();
        return;
    }
    if (vista_ == Vista::Ordenar) {
        if (dentro({260, 185, 160, 46}, mouse)) criterioOrden_ = "Titulo";
        else if (dentro({435, 185, 160, 46}, mouse)) criterioOrden_ = "Autor";
        else if (dentro({610, 185, 160, 46}, mouse)) criterioOrden_ = "Anio";
        else if (dentro({260, 310, 180, 46}, mouse)) ordenar();
    }
}

void UIManager::guardarFormulario() {
    try {
        if (id_.empty() || anio_.empty() || titulo_.empty() || autor_.empty() || dewey_.empty())
            throw std::invalid_argument("Complete ID, anio, titulo, autor y codigo Dewey.");
        const Libro libro = crearLibro(std::stoi(id_), titulo_, autor_, dewey_, std::stoi(anio_), pdf_);
        const bool correcto = idEditando_ >= 0 ? storage_.actualizarLibro(idEditando_, libro) : storage_.agregarLibro(libro);
        if (!correcto) throw std::invalid_argument("No se pudo guardar: ID repetido o datos invalidos.");
        aviso_ = idEditando_ >= 0 ? "Libro actualizado correctamente." : "Libro guardado correctamente.";
        limpiarFormulario(); recargarCatalogo(); vista_ = Vista::Catalogo;
    } catch (const std::exception& error) { aviso_ = error.what(); }
}

void UIManager::prepararEdicion() {
    if (seleccionado_ < 0 || seleccionado_ >= static_cast<int>(librosVista_.size())) { aviso_ = "Seleccione un libro para editar."; return; }
    const Libro& libro = librosVista_[seleccionado_];
    idEditando_ = libro.id; id_ = std::to_string(libro.id); anio_ = std::to_string(libro.anio);
    titulo_ = libro.titulo; autor_ = libro.autor; dewey_ = libro.codigoDewey; pdf_ = libro.rutaPdf;
    vista_ = Vista::Formulario; campoActivo_ = Campo::Ninguno; aviso_ = "Editando ID " + id_ + ".";
}

void UIManager::eliminarSeleccionado() {
    if (seleccionado_ < 0 || seleccionado_ >= static_cast<int>(librosVista_.size())) { aviso_ = "Seleccione un libro para eliminar."; return; }
    const bool eliminado = storage_.eliminarLibro(librosVista_[seleccionado_].id);
    recargarCatalogo(); aviso_ = eliminado ? "Libro eliminado logicamente." : "No se pudo eliminar el libro.";
}

void UIManager::abrirPdfSeleccionado() {
    if (seleccionado_ < 0 || seleccionado_ >= static_cast<int>(librosVista_.size())) { aviso_ = "Seleccione un libro para abrir su PDF."; return; }
    aviso_ = storage_.abrirPdf(librosVista_[seleccionado_].id) ? "Se solicito abrir el PDF." : "No se encontro el PDF de este libro.";
}

void UIManager::buscar() {
    if (consulta_.empty()) { aviso_ = "Ingrese un valor de busqueda."; return; }
    if (consulta_.size() >= 6 && minusculas(consulta_.substr(0, 6)) == "select") {
        ResultadoConsulta resultado = busquedas_.ejecutarConsulta(consulta_);
        librosVista_ = std::move(resultado.libros);
        seleccionado_ = -1;
        vista_ = Vista::Catalogo;
        aviso_ = resultado.mensaje + " " + std::to_string(resultado.duracion.count()) + " ns.";
        return;
    }
    if (criterioBusqueda_ == "ID") {
        try { auto libro = storage_.buscarPorID(std::stoi(consulta_)); if (libro) librosVista_.push_back(*libro); }
        catch (const std::exception&) { aviso_ = "El ID debe ser numerico."; return; }
    } else if (criterioBusqueda_ == "Titulo") librosVista_ = busquedas_.buscarPorTitulo(consulta_);
    else librosVista_ = busquedas_.buscarPorAutor(consulta_);
    seleccionado_ = -1; vista_ = Vista::Catalogo; aviso_ = std::to_string(librosVista_.size()) + " resultado(s) encontrado(s).";
}

void UIManager::ordenar() {
    const std::vector<Libro> libros = storage_.listarLibros();
    if (criterioOrden_ == "Autor") librosVista_ = SortManager::ordenarPorAutor(libros);
    else if (criterioOrden_ == "Anio") librosVista_ = SortManager::ordenarPorAnio(libros);
    else librosVista_ = SortManager::ordenarPorTitulo(libros);
    seleccionado_ = -1; vista_ = Vista::Catalogo; aviso_ = "Catalogo ordenado por " + criterioOrden_ + ".";
}

void UIManager::limpiarFormulario() { idEditando_ = -1; campoActivo_ = Campo::Ninguno; id_.clear(); anio_.clear(); titulo_.clear(); autor_.clear(); dewey_.clear(); pdf_.clear(); }

void UIManager::dibujar() {
    ventana_.clear(FONDO); dibujarNavegacion();
    switch (vista_) { case Vista::Inicio: dibujarInicio(); break; case Vista::Catalogo: dibujarCatalogo(); break; case Vista::Formulario: dibujarFormulario(); break; case Vista::Buscar: dibujarBuscar(); break; case Vista::Ordenar: dibujarOrdenar(); break; }
    if (!aviso_.empty()) { sf::RectangleShape caja({960, 38}); caja.setPosition(260, 700); caja.setFillColor(PANEL_CLARO); ventana_.draw(caja); texto(aviso_, 274, 709, 14, TEXTO); }
    ventana_.display();
}

void UIManager::dibujarNavegacion() {
    sf::RectangleShape lateral({230, 760}); lateral.setFillColor(PANEL); ventana_.draw(lateral);
    texto("LIBROS", 25, 30, 28, ACENTO, true); texto("Mini SGBD | Base de Datos II", 25, 70, 13, TENUE); texto("NAVEGACION", 25, 112, 12, TENUE, true);
    boton({18, 142, 194, 46}, "Inicio", vista_ == Vista::Inicio); boton({18, 198, 194, 46}, "Catalogo", vista_ == Vista::Catalogo); boton({18, 254, 194, 46}, "Agregar libro", vista_ == Vista::Formulario); boton({18, 310, 194, 46}, "Buscar", vista_ == Vista::Buscar); boton({18, 366, 194, 46}, "Ordenar", vista_ == Vista::Ordenar);
    texto("Estado del sistema", 25, 670, 13, TENUE, true); texto("Interfaz conectada", 25, 696, 15, EXITO); texto("Almacenamiento binario / FIFO", 25, 718, 12, TENUE);
}

void UIManager::dibujarInicio() {
    texto("Panel principal", 265, 34, 30, TEXTO, true); texto("Gestion visual del catalogo de la biblioteca", 265, 76, 16, TENUE); sf::RectangleShape linea({860, 1}); linea.setPosition(265, 112); linea.setFillColor(PANEL_CLARO); ventana_.draw(linea);
    const std::array<std::string, 3> titulos = {"Catalogo", "Consultas", "Ordenamientos"}; const std::array<std::string, 3> detalles = {"Visualiza, actualiza o elimina libros.", "Busca por ID, titulo o autor.", "Organiza por titulo, autor o anio."}; const std::array<std::string, 3> estados = {"StorageManager conectado", "Consulta visual disponible", "Ordenamiento visual disponible"};
    for (int i = 0; i < 3; ++i) { const float x = 265 + 278.f * i; sf::RectangleShape tarjeta({258, 145}); tarjeta.setPosition(x, 177); tarjeta.setFillColor(PANEL); tarjeta.setOutlineColor(PANEL_CLARO); tarjeta.setOutlineThickness(1); ventana_.draw(tarjeta); texto(titulos[i], x + 20, 205, 21, TEXTO, true); texto(detalles[i], x + 20, 244, 13, TENUE); texto(estados[i], x + 20, 289, 12, ACENTO); }
    texto("Flujo del sistema", 265, 370, 20, TEXTO, true); texto("Interfaz SFML  ->  Gestores de consulta  ->  Almacenamiento binario / FIFO", 265, 410, 16, TENUE);
}

void UIManager::dibujarCatalogo() {
    texto("Catalogo de libros", 260, 34, 28, TEXTO, true); texto("Seleccione una fila para editar, eliminar o abrir su PDF.", 260, 75, 15, TENUE); sf::RectangleShape cabecera({960, 42}); cabecera.setPosition(260, 135); cabecera.setFillColor(PANEL_CLARO); ventana_.draw(cabecera);
    texto("ID", 278, 147, 13, TENUE, true); texto("TITULO", 365, 147, 13, TENUE, true); texto("AUTOR", 700, 147, 13, TENUE, true); texto("ANIO", 1080, 147, 13, TENUE, true);
    for (std::size_t i = 0; i < librosVista_.size() && i < 8; ++i) { const float y = 185 + 42.f * i; sf::RectangleShape fila({960, 40}); fila.setPosition(260, y); fila.setFillColor(static_cast<int>(i) == seleccionado_ ? sf::Color(14, 116, 144) : PANEL); ventana_.draw(fila); const Libro& libro = librosVista_[i]; texto(std::to_string(libro.id), 278, y + 11, 14, TEXTO); texto(resumir(libro.titulo, 35), 365, y + 11, 14, TEXTO); texto(resumir(libro.autor, 30), 700, y + 11, 14, TEXTO); texto(std::to_string(libro.anio), 1080, y + 11, 14, TEXTO); }
    if (librosVista_.empty()) texto("No hay libros para mostrar.", 550, 285, 19, TENUE); boton({260, 585, 130, 44}, "Actualizar", true); boton({405, 585, 130, 44}, "Editar"); boton({550, 585, 130, 44}, "Eliminar"); boton({695, 585, 150, 44}, "Abrir PDF");
}

void UIManager::dibujarFormulario() {
    texto(idEditando_ >= 0 ? "Editar libro" : "Agregar libro", 260, 34, 28, TEXTO, true); texto("Los datos se validan y persisten mediante StorageManager.", 260, 75, 15, TENUE); campo({260, 170, 210, 48}, "ID", id_, Campo::Id, "Ej. 101"); campo({500, 170, 210, 48}, "Anio", anio_, Campo::Anio, "Ej. 1967"); campo({260, 275, 900, 48}, "Titulo", titulo_, Campo::Titulo, "Titulo completo"); campo({260, 380, 900, 48}, "Autor", autor_, Campo::Autor, "Nombre del autor"); campo({260, 485, 430, 48}, "Codigo Dewey", dewey_, Campo::Dewey, "Ej. 863"); campo({730, 485, 430, 48}, "Ruta PDF (opcional)", pdf_, Campo::Pdf, "Ej. pdfs_reales/libro.pdf"); boton({260, 600, 190, 46}, idEditando_ >= 0 ? "Guardar cambios" : "Guardar libro", true); boton({465, 600, 150, 46}, "Limpiar");
}

void UIManager::dibujarBuscar() {
    texto("Buscar libros", 260, 34, 28, TEXTO, true); texto("ID usa el indice hash. Tambien admite SELECT * con WHERE y ORDER BY.", 260, 75, 15, TENUE); boton({260, 170, 140, 42}, "Por ID", criterioBusqueda_ == "ID"); boton({415, 170, 140, 42}, "Por titulo", criterioBusqueda_ == "Titulo"); boton({570, 170, 140, 42}, "Por autor", criterioBusqueda_ == "Autor"); campo({260, 265, 560, 48}, "Consulta", consulta_, Campo::Consulta, "Valor o SELECT * WHERE autor CONTAINS Borges"); boton({840, 265, 145, 48}, "Buscar", true); texto("Los resultados se muestran en Catalogo.", 260, 380, 16, TENUE);
}

void UIManager::dibujarOrdenar() {
    texto("Ordenar catalogo", 260, 34, 28, TEXTO, true); texto("La vista ordenada se mostrara en Catalogo.", 260, 75, 15, TENUE); boton({260, 185, 160, 46}, "Titulo", criterioOrden_ == "Titulo"); boton({435, 185, 160, 46}, "Autor", criterioOrden_ == "Autor"); boton({610, 185, 160, 46}, "Anio", criterioOrden_ == "Anio"); boton({260, 310, 180, 46}, "Aplicar orden", true);
}

void UIManager::texto(const std::string& valor, float x, float y, unsigned tamano, sf::Color color, bool negrita) { sf::Text etiqueta(valor, fuente_, tamano); etiqueta.setPosition(x, y); etiqueta.setFillColor(color); etiqueta.setStyle(negrita ? sf::Text::Bold : sf::Text::Regular); ventana_.draw(etiqueta); }
void UIManager::boton(const sf::FloatRect& area, const std::string& etiqueta, bool activo) { sf::RectangleShape forma({area.width, area.height}); forma.setPosition(area.left, area.top); forma.setFillColor(activo ? ACENTO : PANEL_CLARO); forma.setOutlineColor(activo ? ACENTO : sf::Color(71, 85, 105)); forma.setOutlineThickness(1); ventana_.draw(forma); texto(etiqueta, area.left + 13, area.top + 12, 14, activo ? FONDO : TEXTO, activo); }
void UIManager::campo(const sf::FloatRect& area, const std::string& etiqueta, const std::string& valor, Campo idCampo, const std::string& ejemplo) { texto(etiqueta, area.left, area.top - 25, 14, TEXTO, true); sf::RectangleShape forma({area.width, area.height}); forma.setPosition(area.left, area.top); forma.setFillColor(PANEL); forma.setOutlineColor(campoActivo_ == idCampo ? ACENTO : PANEL_CLARO); forma.setOutlineThickness(campoActivo_ == idCampo ? 2.f : 1.f); ventana_.draw(forma); texto(valor.empty() ? ejemplo : valor, area.left + 13, area.top + 14, 14, valor.empty() ? TENUE : TEXTO); }
bool UIManager::dentro(const sf::FloatRect& area, sf::Vector2i punto) const { return area.contains(static_cast<float>(punto.x), static_cast<float>(punto.y)); }
