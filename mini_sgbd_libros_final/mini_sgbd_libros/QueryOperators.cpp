#include "QueryOperators.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

ScanOperator::ScanOperator(StorageManager& storage) : storage_(storage) {}

void ScanOperator::open() {
    libros_ = storage_.listarLibros();
    posicion_ = 0;
    abierto_ = true;
}

bool ScanOperator::next(Libro& libro) {
    if (!abierto_) throw std::logic_error("ScanOperator debe abrirse antes de next().");
    if (posicion_ >= libros_.size()) return false;
    libro = libros_[posicion_++];
    return true;
}

void ScanOperator::close() {
    libros_.clear();
    posicion_ = 0;
    abierto_ = false;
}

FilterOperator::FilterOperator(std::unique_ptr<QueryOperator> hijo, Predicate predicado)
    : hijo_(std::move(hijo)), predicado_(std::move(predicado)) {}

void FilterOperator::open() {
    hijo_->open();
    abierto_ = true;
}

bool FilterOperator::next(Libro& libro) {
    if (!abierto_) throw std::logic_error("FilterOperator debe abrirse antes de next().");
    while (hijo_->next(libro)) if (predicado_(libro)) return true;
    return false;
}

void FilterOperator::close() {
    if (abierto_) hijo_->close();
    abierto_ = false;
}

SortOperator::SortOperator(std::unique_ptr<QueryOperator> hijo, Comparator comparador)
    : hijo_(std::move(hijo)), comparador_(std::move(comparador)) {}

void SortOperator::open() {
    hijo_->open();
    ordenados_.clear();
    Libro libro{};
    while (hijo_->next(libro)) ordenados_.push_back(libro);
    std::sort(ordenados_.begin(), ordenados_.end(), comparador_);
    posicion_ = 0;
    abierto_ = true;
}

bool SortOperator::next(Libro& libro) {
    if (!abierto_) throw std::logic_error("SortOperator debe abrirse antes de next().");
    if (posicion_ >= ordenados_.size()) return false;
    libro = ordenados_[posicion_++];
    return true;
}

void SortOperator::close() {
    if (abierto_) hijo_->close();
    ordenados_.clear();
    posicion_ = 0;
    abierto_ = false;
}
