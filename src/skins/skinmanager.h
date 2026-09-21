#pragma once
#include "Skin.h"
#include <filesystem>

class SkinManager {
public:
    static Skin LoadSkin(const std::filesystem::path& skinFolder);
};