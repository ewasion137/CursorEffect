#pragma once
#include <string>

// Структура цвета RGBA (0.0f - 1.0f)
struct ColorRGBA {
    float r = 0.0f;
    float g = 0.7f;
    float b = 1.0f;
    float a = 1.0f;
};

struct Settings {
    // --- Общие ---
    std::string activeSkin = "dot";
    float maxFps = 120.0f;
    bool trailEnabled = true;

    // --- Спавн ---
    float stepDistance = 6.0f;    // Каждые N пикселей спавнить точку
    float particleLife = 0.45f;   // Время жизни в секундах

    // --- Точка (Dot) ---
    float startRadius = 10.0f;
    float endRadius = 0.0f;       // 0.0 = сжимается до исчезновения

    // --- Градиент и цвет ---
    // StartColor: цвет в момент рождения под курсором
    ColorRGBA startColor = { 0.0f, 0.8f, 1.0f, 1.0f }; // Яркий циан
    // EndColor: цвет в момент смерти перед растворением
    ColorRGBA endColor   = { 0.0f, 0.0f, 0.8f, 1.0f }; // Глубокий синий
};