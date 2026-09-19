cmake_minimum_required(VERSION 3.20)
project(CursorEffect LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(UNICODE _UNICODE)

# Подключаем папку src для поиска заголовков
include_directories(src)

# Явный список файлов — гарантирует 100% сборку
set(SOURCES
    src/main.cpp
    src/config/ConfigManager.cpp
    src/core/Window.cpp
    src/graphics/Renderer.cpp
    src/particles/ParticleSystem.cpp
)

add_executable(CursorEffect ${SOURCES})

target_link_libraries(CursorEffect PRIVATE
    d2d1
    user32
    gdi32
    shell32
)