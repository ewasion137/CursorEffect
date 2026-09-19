#pragma once
#include <string>

struct ColorRGBA {
    float r = 0.0f;
    float g = 0.7f;
    float b = 1.0f;
    float a = 1.0f;
};

struct Settings {
    std::string activeSkin = "default";
    std::string mode = "dot";              // "dot", "sprite" или "gif"
    std::string spriteFile = "texture.png";
    std::string gifFile = "animation.gif"; // <-- Имя GIF-файла

    float maxFps = 120.0f;
    bool trailEnabled = true;
    float stepDistance = 6.0f;
    float particleLife = 0.45f;

    float startRadius = 20.0f;
    float endRadius = 0.0f;

    ColorRGBA startColor = { 0.0f, 0.8f, 1.0f, 1.0f };
    ColorRGBA endColor   = { 0.0f, 0.0f, 0.8f, 1.0f };
};