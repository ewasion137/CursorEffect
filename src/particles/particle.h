#pragma once
#include "../config/Settings.h"

struct Particle {
    float x, y;
    float life;
    float maxLife;
    float startRadius;
    float endRadius;
    ColorRGBA startColor;
    ColorRGBA endColor;
    
    float animTime = 0.0f;
    float angle = 0.0f; // <-- Угол направления движения мыши (в радианах)
};