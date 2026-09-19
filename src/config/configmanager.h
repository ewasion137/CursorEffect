#pragma once
#include "Settings.h"
#include <filesystem>

class ConfigManager {
public:
    static void Init();                 // Создает папку ~/.cureff если ее нет
    static Settings Load();             // Загружает настройки из settings.cfg
    static void Save(const Settings& s);// Сохраняет текущие настройки в файл

    static std::filesystem::path GetConfigDir();
    static std::filesystem::path GetConfigFile();
};