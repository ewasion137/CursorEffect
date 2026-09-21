#include "SkinManager.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace {
    std::string Trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }

    std::string ToLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return s;
    }
}

Skin SkinManager::LoadSkin(const std::filesystem::path& skinFolder) {
    Skin skin;
    skin.folderPath = skinFolder;
    skin.name = skinFolder.filename().string();
    skin.type = "dot"; // По умолчанию надежный режим точек

    std::error_code ec;
    if (!std::filesystem::exists(skinFolder, ec)) {
        return skin;
    }

    std::filesystem::path cfgPath = skinFolder / "skin.cfg";
    std::ifstream file(cfgPath);
    if (!file.is_open()) {
        // Если файла нет, проверим наличие анимации или спрайта в папке
        if (std::filesystem::exists(skinFolder / "animation.gif", ec)) {
            skin.type = "gif";
            skin.file = "animation.gif";
        } else if (std::filesystem::exists(skinFolder / "texture.png", ec)) {
            skin.type = "sprite";
            skin.file = "texture.png";
        }
        return skin;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = Trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;
        if (line.rfind("//", 0) == 0) continue;

        auto eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;

        std::string key = ToLower(Trim(line.substr(0, eqPos)));
        std::string val = Trim(line.substr(eqPos + 1));

        try {
            if (key == "type") {
                skin.type = ToLower(val);
            } else if (key == "file") {
                skin.file = val;
            } else if (key == "base_size") {
                skin.baseSize = std::stof(val);
            } else if (key == "particle_life") {
                skin.particleLife = std::stof(val);
            } else if (key == "step_distance") {
                skin.stepDistance = std::stof(val);
            } else if (key == "align_to_motion") {
                std::string lowVal = ToLower(val);
                skin.alignToMotion = (lowVal == "true" || lowVal == "1" || lowVal == "yes" || lowVal == "on");
            } else if (key == "size_sequence") {
                skin.sizeCurve = NumberSequence::Parse(val);
                skin.hasSizeCurve = !skin.sizeCurve.keypoints.empty();
            } else if (key == "squash_sequence") {
                skin.squashCurve = NumberSequence::Parse(val);
                skin.hasSquashCurve = !skin.squashCurve.keypoints.empty();
            }
        } catch (...) {
            // Игнорируем некорректные строки без падения
        }
    }
    return skin;
}