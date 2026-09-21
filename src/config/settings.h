#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>

struct ColorRGBA {
    float r = 0.0f;
    float g = 0.8f;
    float b = 1.0f;
    float a = 1.0f;

    bool operator==(const ColorRGBA& o) const {
        return r == o.r && g == o.g && b == o.b && a == o.a;
    }
    bool operator!=(const ColorRGBA& o) const {
        return !(*this == o);
    }

    std::string ToHexString(bool includeAlpha = false) const {
        auto toByte = [](float v) -> int {
            return (std::clamp)(static_cast<int>(v * 255.0f + 0.5f), 0, 255);
        };
        std::ostringstream ss;
        ss << "#"
           << std::hex << std::uppercase << std::setfill('0')
           << std::setw(2) << toByte(r)
           << std::setw(2) << toByte(g)
           << std::setw(2) << toByte(b);
        if (includeAlpha) {
            ss << std::setw(2) << toByte(a);
        }
        return ss.str();
    }
};

struct Settings {
    // Основные настройки
    std::string activeSkin = "default";
    std::string mode = "dot";              // "dot", "sprite", "gif"
    std::string spriteFile = "texture.png";
    std::string gifFile = "animation.gif";

    // Параметры частиц и производительности
    float maxFps = 120.0f;
    bool trailEnabled = true;
    float stepDistance = 6.0f;
    float particleLife = 0.45f;

    float startRadius = 18.0f;
    float endRadius = 0.0f;

    ColorRGBA startColor = { 0.0f, 0.8f, 1.0f, 1.0f }; // Яркий лазурный
    ColorRGBA endColor   = { 0.0f, 0.2f, 0.9f, 0.0f }; // Плавное угасание в прозрачный

    // Поведение программы
    bool exitOnEscape = false;             // Защита от случайного выхода при работе
    bool hotReload = true;                 // Автоматическая подгрузка изменений файла конфига

    // Валидация и нормализация данных
    void Validate() {
        maxFps = (std::clamp)(maxFps, 15.0f, 360.0f);
        stepDistance = (std::max)(1.0f, stepDistance);
        particleLife = (std::max)(0.02f, particleLife);
        startRadius = (std::max)(0.0f, startRadius);
        endRadius = (std::max)(0.0f, endRadius);

        startColor.r = (std::clamp)(startColor.r, 0.0f, 1.0f);
        startColor.g = (std::clamp)(startColor.g, 0.0f, 1.0f);
        startColor.b = (std::clamp)(startColor.b, 0.0f, 1.0f);
        startColor.a = (std::clamp)(startColor.a, 0.0f, 1.0f);

        endColor.r = (std::clamp)(endColor.r, 0.0f, 1.0f);
        endColor.g = (std::clamp)(endColor.g, 0.0f, 1.0f);
        endColor.b = (std::clamp)(endColor.b, 0.0f, 1.0f);
        endColor.a = (std::clamp)(endColor.a, 0.0f, 1.0f);
    }
};