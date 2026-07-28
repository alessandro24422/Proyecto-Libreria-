cmake_minimum_required(VERSION 3.10)
project(MiniSGBDLibros)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(mini_sgbd
    main.cpp
    BufferManager.cpp
    StorageManager.cpp
    QueryOperators.cpp
    SearchManager.cpp
    SortManager.cpp
    styles/UIManager.cpp
)

target_include_directories(mini_sgbd PRIVATE include styles)
target_link_libraries(mini_sgbd PRIVATE sfml-graphics sfml-window sfml-system)
