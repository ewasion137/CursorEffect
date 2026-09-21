#pragma once
#include "Curve.h"
#include <string>
#include <filesystem>

struct Skin {
    std::string name = "default";
    std::string type = "dot";           // "dot", "sprite", "gif"
    std::string file = "animation.gif";

    float baseSize = 32.0f;
    float particleLife = 0.6f;
    float stepDistance = 6.0f;
    bool alignToMotion = true;          // Поворачивать ли по ходу движения мыши

    // Кривые из Роблокса
    bool hasSizeCurve = false;
    NumberSequence sizeCurve;

    bool hasSquashCurve = false;
    NumberSequence squashCurve;

    std::filesystem::path folderPath;
};