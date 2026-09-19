#pragma once
#include <string>

struct ColorRGBA {
    float r = 0.0f;
    float g = 0.7f;
    float b = 1.0f;
    float a = 1.0f;
};

struct Settings {
    // --- Режимы и скины ---
    std::string activeSkin = "default";
    std::string mode = "dot";              // "dot" или "sprite"
    std::string spriteFile = "texture.png";// Картинка в папке ~/.cureff/

    // --- Общие ---
    float maxFps = 120.0f;
    bool trailEnabled = true;
    float stepDistance = 6.0f;
    float particleLife = 0.45f;

    // --- Размеры ---
    float startRadius = 16.0f;
    float endRadius = 0.0f;

    // --- Цвета и градиент (для Dot) ---
    ColorRGBA startColor = { 0.0f, 0.8f, 1.0f, 1.0f };
    ColorRGBA endColor   = { 0.0f, 0.0f, 0.8f, 1.0f };
};