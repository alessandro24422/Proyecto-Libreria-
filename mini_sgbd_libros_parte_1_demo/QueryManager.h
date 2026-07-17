#ifndef QUERY_MANAGER_H
#define QUERY_MANAGER_H

#include "QueryResult.h"
#include "SearchManager.h"
#include "SortManager.h"

#include <string>

class QueryManager {
public:
    explicit QueryManager(StorageManager& storage);

    QueryResult ejecutar(const std::string& consulta);

private:
    StorageManager& storage;
    SearchManager searchManager;
    SortManager sortManager;
};

#endif
