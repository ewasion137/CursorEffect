#pragma once
#include "Skin.h"
#include <filesystem>
#include <vector>

class SkinManager {
public:
    static Skin LoadSkin(const std::filesystem::path& skinFolder);
    static void CreateDefaultSkin(const std::filesystem::path& skinsDir);
};