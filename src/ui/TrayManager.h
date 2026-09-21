#pragma once
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <functional>

#define WM_TRAYICON (WM_APP + 101)

class TrayManager {
public:
    TrayManager();
    ~TrayManager();

    bool Init(HWND hwnd, const std::wstring& logoPath = L"logo.png");
    void Remove();

    void ShowBalloon(const std::wstring& title, const std::wstring& message);
    void HandleTrayMessage(HWND hwnd, LPARAM lParam);

    // Коллбеки для событий из меню
    std::function<void(bool enabled)> onToggleTrail;
    std::function<void(const std::string& skinName)> onSelectSkin;
    std::function<void()> onReloadConfig;
    std::function<void()> onOpenConfigDir;
    std::function<void()> onAbout;
    std::function<void()> onExit;

    // Геттер состояния для меню
    std::function<bool()> getTrailEnabled;
    std::function<std::string()> getActiveSkin;

private:
    void ShowContextMenu(HWND hwnd);
    HICON LoadIconFromFileOrResource(const std::wstring& filePath);
    HICON CreateIconFromPng(const std::wstring& filePath);

    HWND m_hwnd = nullptr;
    NOTIFYICONDATAW m_nid = {};
    HICON m_hIcon = nullptr;
    bool m_installed = false;
};
