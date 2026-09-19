cmake_minimum_required(VERSION 3.20)
project(CursorEffect LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включаем Unicode для Windows API
add_compile_definitions(UNICODE _UNICODE)

add_executable(CursorEffect main.cpp)

target_link_libraries(CursorEffect PRIVATE
    d2d1
    user32
    gdi32
)