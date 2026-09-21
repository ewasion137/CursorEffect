cmake_minimum_required(VERSION 3.20)
project(CursorEffect LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(UNICODE _UNICODE NOMINMAX)

if(MSVC)
    add_compile_options(/utf-8)
endif()

include_directories(src resources)

set(SOURCES
    src/main.cpp
    src/config/ConfigManager.cpp
    src/core/Window.cpp
    src/graphics/Renderer.cpp
    src/particles/ParticleSystem.cpp
    src/skins/SkinManager.cpp
    src/ui/TrayManager.cpp
    resources/app.rc
)

# WIN32 hides the console window entirely
add_executable(CursorEffect WIN32 ${SOURCES})

target_link_libraries(CursorEffect PRIVATE
    d2d1
    windowscodecs # WIC for PNG/GIF/icons
    user32
    gdi32
    shell32
    ole32         # COM (WIC)
)