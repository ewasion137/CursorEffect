#include "config/ConfigManager.h"
#include "core/Window.h"
#include "graphics/Renderer.h"
#include "particles/ParticleSystem.h"
#include "skins/SkinManager.h"
#include "ui/TrayManager.h"
#include <chrono>
#include <objbase.h>
#include <shellapi.h>

int WINAPI wWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, PWSTR /*lpCmdLine*/, int /*nShowCmd*/) {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    // 1. Initialize configuration folder (~/.cureff) and load settings
    ConfigManager::Init();
    Settings settings = ConfigManager::Load();

    // 2. Create the transparent overlay window
    Window window;
    if (!window.Init()) {
        CoUninitialize();
        return -1;
    }

    // 3. Initialize Direct2D renderer
    Renderer renderer;
    if (!renderer.Init(window.GetHwnd(), window.GetWidth(), window.GetHeight())) {
        CoUninitialize();
        return -1;
    }

    // 4. Initialize system tray with logo
    TrayManager tray;
    std::filesystem::path logoPath = std::filesystem::current_path() / "logo.png";
    tray.Init(window.GetHwnd(), logoPath.wstring());

    bool running = true;

    // Skin resources
    Skin activeSkin;
    AnimatedGif skinGif;
    ID2D1Bitmap* pSpriteBitmap = nullptr;
    Settings effectiveSettings = settings;

    // Safe and smooth skin reloading routine
    auto reloadActiveSkin = [&](const std::string& skinName) {
        skinGif.Cleanup();
        if (pSpriteBitmap) {
            pSpriteBitmap->Release();
            pSpriteBitmap = nullptr;
        }

        effectiveSettings = settings;
        activeSkin = Skin();

        if (skinName.empty() || skinName == "default") {
            activeSkin.name = "default";
            activeSkin.type = "dot";
            return;
        }

        std::filesystem::path skinPath = ConfigManager::GetSkinsDir() / skinName;
        if (std::filesystem::exists(skinPath)) {
            activeSkin = SkinManager::LoadSkin(skinPath);

            if (activeSkin.type == "gif" && !activeSkin.file.empty()) {
                skinGif = renderer.LoadGifFromFile((skinPath / activeSkin.file).wstring());
            } else if (activeSkin.type == "sprite" && !activeSkin.file.empty()) {
                pSpriteBitmap = renderer.LoadBitmapFromFile((skinPath / activeSkin.file).wstring());
            }

            if (activeSkin.particleLife > 0.0f) effectiveSettings.particleLife = activeSkin.particleLife;
            if (activeSkin.stepDistance > 0.0f) effectiveSettings.stepDistance = activeSkin.stepDistance;
            if (activeSkin.baseSize > 0.0f)     effectiveSettings.startRadius = activeSkin.baseSize;
        } else {
            activeSkin.name = "default";
            activeSkin.type = "dot";
        }
    };

    reloadActiveSkin(settings.activeSkin);

    // 5. Initialize particle system
    ParticleSystem particleSystem(effectiveSettings);

    // Bind Tray callbacks
    tray.getTrailEnabled = [&]() { return settings.trailEnabled; };
    tray.getActiveSkin   = [&]() { return settings.activeSkin; };

    tray.onToggleTrail = [&](bool enabled) {
        settings.trailEnabled = enabled;
        effectiveSettings.trailEnabled = enabled;
        particleSystem.UpdateSettings(effectiveSettings);
        ConfigManager::Save(settings);
    };

    tray.onSelectSkin = [&](const std::string& skinName) {
        settings.activeSkin = skinName;
        reloadActiveSkin(skinName);
        particleSystem.UpdateSettings(effectiveSettings);
        ConfigManager::Save(settings);
    };

    tray.onReloadConfig = [&]() {
        settings = ConfigManager::Load();
        reloadActiveSkin(settings.activeSkin);
        particleSystem.UpdateSettings(effectiveSettings);
        tray.ShowBalloon(L"CursorEffect", L"Configuration reloaded successfully!");
    };

    tray.onOpenConfigDir = [&]() {
        ShellExecuteW(NULL, L"open", ConfigManager::GetConfigDir().c_str(), NULL, NULL, SW_SHOWNORMAL);
    };

    tray.onAbout = [&]() {
        MessageBoxW(
            window.GetHwnd(),
            L"CursorEffect 1.0\n\n"
            L"Custom particle trails and animated skins for mouse cursor.\n"
            L"• Supports GIF animations, sprites and curves\n"
            L"• Real-time config hot-reloading\n"
            L"• System tray integration\n\n"
            L"Config directory: %USERPROFILE%\\.cureff\n"
            L"Author: ewasion137",
            L"About CursorEffect",
            MB_OK | MB_ICONINFORMATION
        );
    };

    tray.onExit = [&]() {
        running = false;
    };

    // Forward tray messages from overlay window
    window.SetMessageHandler([&](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> bool {
        if (uMsg == WM_TRAYICON) {
            tray.HandleTrayMessage(hwnd, lParam);
            return true;
        }
        return false;
    });

    POINT lastMousePos = { -1, -1 };
    auto lastTime = std::chrono::high_resolution_clock::now();
    auto lastConfigCheckTime = lastTime;

    // Main loop
    while (running) {
        window.PollEvents(running);
        if (!running) break;

        // Escape exit only if explicitly enabled in user config
        if (settings.exitOnEscape && (GetAsyncKeyState(VK_ESCAPE) & 0x8000)) {
            break;
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Hot-reload: check file change every 500 ms
        if (settings.hotReload) {
            float checkElapsed = std::chrono::duration<float>(currentTime - lastConfigCheckTime).count();
            if (checkElapsed >= 0.5f) {
                lastConfigCheckTime = currentTime;
                if (ConfigManager::HasConfigFileChanged()) {
                    Settings newSettings = ConfigManager::Load();
                    bool skinChanged = (newSettings.activeSkin != settings.activeSkin);
                    settings = newSettings;
                    if (skinChanged) {
                        reloadActiveSkin(settings.activeSkin);
                    } else {
                        effectiveSettings.maxFps = settings.maxFps;
                        effectiveSettings.trailEnabled = settings.trailEnabled;
                        effectiveSettings.startColor = settings.startColor;
                        effectiveSettings.endColor = settings.endColor;
                        effectiveSettings.exitOnEscape = settings.exitOnEscape;
                        effectiveSettings.hotReload = settings.hotReload;
                        if (activeSkin.name == "default" || activeSkin.type == "dot") {
                            effectiveSettings.stepDistance = settings.stepDistance;
                            effectiveSettings.particleLife = settings.particleLife;
                            effectiveSettings.startRadius = settings.startRadius;
                            effectiveSettings.endRadius = settings.endRadius;
                        }
                    }
                    particleSystem.UpdateSettings(effectiveSettings);
                }
            }
        }

        // Track cursor
        POINT mousePos;
        GetCursorPos(&mousePos);

        float localMouseX = static_cast<float>(mousePos.x - window.GetX());
        float localMouseY = static_cast<float>(mousePos.y - window.GetY());

        if (lastMousePos.x != -1) {
            if (mousePos.x != lastMousePos.x || mousePos.y != lastMousePos.y) {
                float prevLocalX = static_cast<float>(lastMousePos.x - window.GetX());
                float prevLocalY = static_cast<float>(lastMousePos.y - window.GetY());
                particleSystem.SpawnTrail(prevLocalX, prevLocalY, localMouseX, localMouseY);
            }
        }
        lastMousePos = mousePos;

        particleSystem.Update(dt);

        // Render frame
        renderer.BeginDraw();
        for (const auto& particle : particleSystem.GetParticles()) {
            if (activeSkin.type == "gif" && !skinGif.frames.empty()) {
                renderer.DrawParticleGif(particle, skinGif, activeSkin);
            } else if (activeSkin.type == "sprite" && pSpriteBitmap) {
                renderer.DrawParticleSprite(particle, pSpriteBitmap);
            } else {
                renderer.DrawParticleDot(particle);
            }
        }
        renderer.EndDraw();

        // Frame rate limiter
        int sleepMs = static_cast<int>(1000.0f / effectiveSettings.maxFps);
        if (sleepMs < 1) sleepMs = 1;
        Sleep(sleepMs);
    }

    // Clean up
    tray.Remove();
    skinGif.Cleanup();
    if (pSpriteBitmap) {
        pSpriteBitmap->Release();
        pSpriteBitmap = nullptr;
    }
    renderer.Cleanup();
    CoUninitialize();

    return 0;
}