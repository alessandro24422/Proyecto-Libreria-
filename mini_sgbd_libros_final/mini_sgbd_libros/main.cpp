#include "StorageManager.h"
#include "UIManager.h"

int main() {
    StorageManager storage;
    UIManager interfaz(storage);
    interfaz.ejecutar();
    return 0;
}
