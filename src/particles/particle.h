#pragma once
#include "../config/Settings.h"

struct Particle {
    float x, y;
    float life;         // Текущее оставшееся время
    float maxLife;      // Исходное время жизни
    float startRadius;
    float endRadius;
    ColorRGBA startColor;
    ColorRGBA endColor;
};