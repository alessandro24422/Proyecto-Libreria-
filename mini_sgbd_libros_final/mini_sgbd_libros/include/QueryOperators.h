#pragma once

#include "StorageManager.h"

#include <functional>
#include <memory>
#include <vector>

// Interfaz Volcano: cada operador produce un registro por llamada a next().
class QueryOperator {
public:
    virtual ~QueryOperator() = default;
    virtual void open() = 0;
    virtual bool next(Libro& libro) = 0;
    virtual void close() = 0;
};

// Operador físico de exploración de los registros activos.
class ScanOperator final : public QueryOperator {
public:
    explicit ScanOperator(StorageManager& storage);
    void open() override;
    bool next(Libro& libro) override;
    void close() override;

private:
    StorageManager& storage_;
    std::vector<Libro> libros_;
    std::size_t posicion_{0};
    bool abierto_{false};
};

// Operador físico de selección. Solicita registros a su hijo hasta hallar uno válido.
class FilterOperator final : public QueryOperator {
public:
    using Predicate = std::function<bool(const Libro&)>;

    FilterOperator(std::unique_ptr<QueryOperator> hijo, Predicate predicado);
    void open() override;
    bool next(Libro& libro) override;
    void close() override;

private:
    std::unique_ptr<QueryOperator> hijo_;
    Predicate predicado_;
    bool abierto_{false};
};

// Operador físico de ordenamiento. Materializa la entrada al abrirse y la entrega con next().
class SortOperator final : public QueryOperator {
public:
    using Comparator = std::function<bool(const Libro&, const Libro&)>;

    SortOperator(std::unique_ptr<QueryOperator> hijo, Comparator comparador);
    void open() override;
    bool next(Libro& libro) override;
    void close() override;

private:
    std::unique_ptr<QueryOperator> hijo_;
    Comparator comparador_;
    std::vector<Libro> ordenados_;
    std::size_t posicion_{0};
    bool abierto_{false};
};
