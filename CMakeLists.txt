cmake_minimum_required(VERSION 3.20)
project(CursorEffect LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(UNICODE _UNICODE)

# Автоматически находим все .cpp и .h файлы во всех папках внутри src/
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

# Включаем пути поиска заголовочных файлов, чтобы писать #include "config/Settings.h"
include_directories(src)

add_executable(CursorEffect ${SOURCES} ${HEADERS})

target_link_libraries(CursorEffect PRIVATE
    d2d1
    user32
    gdi32
    shell32   # Для работы с системными папками Windows (USERPROFILE)
)