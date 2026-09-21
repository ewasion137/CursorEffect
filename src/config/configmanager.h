#pragma once
#include "Settings.h"
#include <filesystem>
#include <vector>
#include <string>

class ConfigManager {
public:
    static void Init();
    static Settings Load();
    static void Save(const Settings& s);

    static std::filesystem::path GetConfigDir();
    static std::filesystem::path GetConfigFile();
    static std::filesystem::path GetSkinsDir();

    // Горячая перезагрузка (Hot-Reloading)
    static bool HasConfigFileChanged();
    static void MarkConfigFileAsRead();

    // Динамический поиск установленных скинов
    static std::vector<std::string> GetAvailableSkins();

    // Умные хелперы для разбора типов данных
    static bool ParseBool(const std::string& str, bool defaultValue = false);
    static ColorRGBA ParseColor(const std::string& str, const ColorRGBA& defaultColor);

private:
    inline static std::filesystem::file_time_type s_lastWriteTime{};
};