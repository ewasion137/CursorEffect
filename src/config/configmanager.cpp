#include "ConfigManager.h"
#include <windows.h>
#include <shlobj.h>
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

std::filesystem::path ConfigManager::GetSkinsDir() {
    return GetConfigDir() / "skins";
}

void ConfigManager::Init() {
    auto dir = GetConfigDir();
    auto skinsDir = GetSkinsDir();
    
    std::error_code ec;
    if (!std::filesystem::exists(dir, ec)) {
        std::filesystem::create_directories(dir, ec);
    }
    if (!std::filesystem::exists(skinsDir, ec)) {
        std::filesystem::create_directories(skinsDir, ec);
    }

    auto cfg = GetConfigFile();
    if (!std::filesystem::exists(cfg, ec)) {
        Settings defaultSettings;
        Save(defaultSettings);
    }
    MarkConfigFileAsRead();
}

bool ConfigManager::HasConfigFileChanged() {
    auto cfg = GetConfigFile();
    std::error_code ec;
    if (!std::filesystem::exists(cfg, ec)) return false;

    auto curTime = std::filesystem::last_write_time(cfg, ec);
    if (ec) return false;

    return curTime != s_lastWriteTime;
}

void ConfigManager::MarkConfigFileAsRead() {
    auto cfg = GetConfigFile();
    std::error_code ec;
    if (std::filesystem::exists(cfg, ec)) {
        s_lastWriteTime = std::filesystem::last_write_time(cfg, ec);
    }
}

std::vector<std::string> ConfigManager::GetAvailableSkins() {
    std::vector<std::string> skins;
    auto skinsDir = GetSkinsDir();
    std::error_code ec;
    if (!std::filesystem::exists(skinsDir, ec)) return skins;

    for (const auto& entry : std::filesystem::directory_iterator(skinsDir, ec)) {
        if (entry.is_directory(ec)) {
            skins.push_back(entry.path().filename().string());
        }
    }
    std::sort(skins.begin(), skins.end());
    return skins;
}

bool ConfigManager::ParseBool(const std::string& str, bool defaultValue) {
    std::string s = ToLower(Trim(str));
    if (s == "true" || s == "1" || s == "yes" || s == "on" || s == "enable" || s == "enabled") return true;
    if (s == "false" || s == "0" || s == "no" || s == "off" || s == "disable" || s == "disabled") return false;
    return defaultValue;
}

ColorRGBA ConfigManager::ParseColor(const std::string& str, const ColorRGBA& defaultColor) {
    std::string s = Trim(str);
    if (s.empty()) return defaultColor;

    // Поддержка HEX формата: #RGB, #RRGGBB, #RRGGBBAA
    if (s[0] == '#') {
        std::string hex = s.substr(1);
        if (hex.length() == 3) { // #RGB -> #RRGGBB
            std::string expanded;
            for (char c : hex) { expanded += c; expanded += c; }
            hex = expanded;
        }

        try {
            if (hex.length() == 6) {
                unsigned long val = std::stoul(hex, nullptr, 16);
                return ColorRGBA{
                    ((val >> 16) & 0xFF) / 255.0f,
                    ((val >> 8) & 0xFF) / 255.0f,
                    (val & 0xFF) / 255.0f,
                    1.0f
                };
            } else if (hex.length() == 8) {
                unsigned long val = std::stoul(hex, nullptr, 16);
                return ColorRGBA{
                    ((val >> 24) & 0xFF) / 255.0f,
                    ((val >> 16) & 0xFF) / 255.0f,
                    ((val >> 8) & 0xFF) / 255.0f,
                    (val & 0xFF) / 255.0f
                };
            }
        } catch (...) {
            return defaultColor;
        }
    }

    // Поддержка формата чисел: "R G B A" или "R G B"
    std::istringstream ss(s);
    float r = 0, g = 0, b = 0, a = 1.0f;
    if (ss >> r >> g >> b) {
        if (!(ss >> a)) {
            a = 1.0f;
        }
        // Если числа заданы в диапазоне 0..255, масштабируем
        if (r > 1.0f || g > 1.0f || b > 1.0f) {
            r /= 255.0f;
            g /= 255.0f;
            b /= 255.0f;
            if (a > 1.0f) a /= 255.0f;
        }
        return ColorRGBA{ r, g, b, a };
    }

    return defaultColor;
}

void ConfigManager::Save(const Settings& s) {
    std::ofstream file(GetConfigFile());
    if (!file.is_open()) return;

    file << "# ========================================================\n";
    file << "# CursorEffect Configuration\n";
    file << "# Changes in this file are applied automatically on save!\n";
    file << "# ========================================================\n\n";

    file << "# Active skin name (folder inside ~/.cureff/skins/ or 'default')\n";
    file << "active_skin = " << s.activeSkin << "\n\n";

    file << "# Trail rendering mode: dot | sprite | gif\n";
    file << "mode = " << s.mode << "\n";
    file << "sprite_file = " << s.spriteFile << "\n";
    file << "gif_file = " << s.gifFile << "\n\n";

    file << "# Performance\n";
    file << "max_fps = " << s.maxFps << "\n";
    file << "trail_enabled = " << (s.trailEnabled ? "true" : "false") << "\n\n";

    file << "# Trail dynamics\n";
    file << "step_distance = " << s.stepDistance << "\n";
    file << "particle_life = " << s.particleLife << "\n";
    file << "start_radius = " << s.startRadius << "\n";
    file << "end_radius = " << s.endRadius << "\n\n";

    file << "# Particle colors (#RRGGBB, #RRGGBBAA or R G B A)\n";
    file << "start_color = " << s.startColor.ToHexString(true) << "\n";
    file << "end_color = " << s.endColor.ToHexString(true) << "\n\n";

    file << "# Behavior\n";
    file << "exit_on_escape = " << (s.exitOnEscape ? "true" : "false") << "\n";
    file << "hot_reload = " << (s.hotReload ? "true" : "false") << "\n";

    file.close();
    MarkConfigFileAsRead();
}

Settings ConfigManager::Load() {
    Settings s;
    std::ifstream file(GetConfigFile());
    if (!file.is_open()) {
        s.Validate();
        return s;
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

        if (key == "active_skin") {
            s.activeSkin = val;
        } else if (key == "mode") {
            s.mode = ToLower(val);
        } else if (key == "sprite_file") {
            s.spriteFile = val;
        } else if (key == "gif_file") {
            s.gifFile = val;
        } else if (key == "max_fps") {
            try { s.maxFps = std::stof(val); } catch (...) {}
        } else if (key == "trail_enabled") {
            s.trailEnabled = ParseBool(val, s.trailEnabled);
        } else if (key == "step_distance") {
            try { s.stepDistance = std::stof(val); } catch (...) {}
        } else if (key == "particle_life") {
            try { s.particleLife = std::stof(val); } catch (...) {}
        } else if (key == "start_radius") {
            try { s.startRadius = std::stof(val); } catch (...) {}
        } else if (key == "end_radius") {
            try { s.endRadius = std::stof(val); } catch (...) {}
        } else if (key == "start_color") {
            s.startColor = ParseColor(val, s.startColor);
        } else if (key == "end_color") {
            s.endColor = ParseColor(val, s.endColor);
        } else if (key == "exit_on_escape") {
            s.exitOnEscape = ParseBool(val, s.exitOnEscape);
        } else if (key == "hot_reload") {
            s.hotReload = ParseBool(val, s.hotReload);
        }
    }

    s.Validate();
    MarkConfigFileAsRead();
    return s;
}