#include "SkinManager.h"
#include <fstream>
#include <sstream>

Skin SkinManager::LoadSkin(const std::filesystem::path& skinFolder) {
    Skin skin;
    skin.name = skinFolder.filename().string();
    skin.folderPath = skinFolder;

    std::filesystem::path cfgPath = skinFolder / "skin.cfg";
    std::ifstream file(cfgPath);
    if (!file.is_open()) return skin;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        auto eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;

        std::string key = line.substr(0, eqPos);
        std::string val = line.substr(eqPos + 1);

        // Убираем пробелы
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        val.erase(0, val.find_first_not_of(" \t"));
        val.erase(val.find_last_not_of(" \t") + 1);

        if (key == "type") skin.type = val;
        else if (key == "file") skin.file = val;
        else if (key == "base_size") skin.baseSize = std::stof(val);
        else if (key == "particle_life") skin.particleLife = std::stof(val);
        else if (key == "step_distance") skin.stepDistance = std::stof(val);
        else if (key == "align_to_motion") skin.alignToMotion = (val == "true" || val == "1");
        else if (key == "size_sequence") {
            skin.sizeCurve = NumberSequence::Parse(val);
            skin.hasSizeCurve = !skin.sizeCurve.keypoints.empty();
        }
        else if (key == "squash_sequence") {
            skin.squashCurve = NumberSequence::Parse(val);
            skin.hasSquashCurve = !skin.squashCurve.keypoints.empty();
        }
    }
    return skin;
}

void SkinManager::CreateDefaultSkin(const std::filesystem::path& skinsDir) {
    auto Dir = skinsDir / "error";
    if (!std::filesystem::exists(Dir)) {
        std::filesystem::create_directories(Dir);
    }

    auto cfgPath = Dir / "skin.cfg";
    if (!std::filesystem::exists(cfgPath)) {
        std::ofstream file(cfgPath);
        file << "# Error Skin Config\n";
        file << "type = gif\n";
        file << "file = animation.gif\n";
        file << "base_size = 35.0\n";
        file << "particle_life = 0.65\n";
        file << "step_distance = 7.0\n";
        file << "align_to_motion = true\n\n";

        // Твои оригинальные строки из Роблокса!
        file << "size_sequence = 0 0 0 0.495982 0.75 0.75 1 0.5625 0\n";
        file << "squash_sequence = 0 0 0 0.49713 -0.0749998 0.6375 1 0 0\n";
    }
}