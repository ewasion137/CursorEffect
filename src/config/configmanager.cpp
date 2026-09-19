#include "ConfigManager.h"
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <sstream>

std::filesystem::path ConfigManager::GetConfigDir() {
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path))) {
        return std::filesystem::path(path) / L".cureff";
    }
    return std::filesystem::current_path() / ".cureff";
}

std::filesystem::path ConfigManager::GetConfigFile() {
    return GetConfigDir() / "settings.cfg";
}

void ConfigManager::Init() {
    auto dir = GetConfigDir();
    if (!std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
        std::filesystem::create_directories(dir / "skins");
    }

    auto cfg = GetConfigFile();
    if (!std::filesystem::exists(cfg)) {
        Settings defaultSettings;
        Save(defaultSettings);
    }
}

void ConfigManager::Save(const Settings& s) {
    std::ofstream file(GetConfigFile());
    if (!file.is_open()) return;

    file << "# CursorEffect Configuration\n\n";
    file << "active_skin = " << s.activeSkin << "\n";
    file << "mode = " << s.mode << "\n";
    file << "sprite_file = " << s.spriteFile << "\n";
    file << "max_fps = " << s.maxFps << "\n";
    file << "trail_enabled = " << (s.trailEnabled ? "true" : "false") << "\n";
    file << "step_distance = " << s.stepDistance << "\n";
    file << "gif_file = " << s.gifFile << "\n";
    file << "particle_life = " << s.particleLife << "\n";
    file << "start_radius = " << s.startRadius << "\n";
    file << "end_radius = " << s.endRadius << "\n";
    file << "start_color = " << s.startColor.r << " " << s.startColor.g << " " << s.startColor.b << " " << s.startColor.a << "\n";
    file << "end_color = " << s.endColor.r << " " << s.endColor.g << " " << s.endColor.b << " " << s.endColor.a << "\n";
}

Settings ConfigManager::Load() {
    Settings s;
    std::ifstream file(GetConfigFile());
    if (!file.is_open()) return s;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream is_line(line);
        std::string key, eq;
        if (is_line >> key >> eq && eq == "=") {
            if (key == "active_skin") is_line >> s.activeSkin;
            else if (key == "mode") is_line >> s.mode;
            else if (key == "sprite_file") is_line >> s.spriteFile;
            else if (key == "max_fps") is_line >> s.maxFps;
            else if (key == "trail_enabled") {
                std::string val; is_line >> val;
                s.trailEnabled = (val == "true" || val == "1");
            }
            else if (key == "step_distance") is_line >> s.stepDistance;
            else if (key == "particle_life") is_line >> s.particleLife;
            else if (key == "gif_file") is_line >> s.gifFile;
            else if (key == "start_radius") is_line >> s.startRadius;
            else if (key == "end_radius") is_line >> s.endRadius;
            else if (key == "start_color") {
                is_line >> s.startColor.r >> s.startColor.g >> s.startColor.b >> s.startColor.a;
            }
            else if (key == "end_color") {
                is_line >> s.endColor.r >> s.endColor.g >> s.endColor.b >> s.endColor.a;
            }
        }
    }
    return s;
}