#include "config/ConfigManager.h"
#include "core/Window.h"
#include "graphics/Renderer.h"
#include "particles/ParticleSystem.h"
#include "skins/SkinManager.h"
#include <chrono>
#include <objbase.h>

int main() {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    // 1. Инициализация и загрузка глобального конфига
    ConfigManager::Init();
    Settings settings = ConfigManager::Load();

    // 2. Загрузка активного скина из папки ~/.cureff/skins/<active_skin>/
    std::filesystem::path skinPath = ConfigManager::GetConfigDir() / "skins" / settings.activeSkin;
    Skin activeSkin = SkinManager::LoadSkin(skinPath);

    // Если в конфиге скина заданы свои параметры — переопределяем их
    if (activeSkin.particleLife > 0.0f) settings.particleLife = activeSkin.particleLife;
    if (activeSkin.stepDistance > 0.0f) settings.stepDistance = activeSkin.stepDistance;
    if (activeSkin.baseSize > 0.0f)     settings.startRadius = activeSkin.baseSize;

    // 3. Создаем окно
    Window window;
    if (!window.Init()) {
        CoUninitialize();
        return -1;
    }

    // 4. Инициализация рендера
    Renderer renderer;
    if (!renderer.Init(window.GetHwnd(), window.GetWidth(), window.GetHeight())) {
        CoUninitialize();
        return -1;
    }

    // 5. Загрузка ресурсов скина
    AnimatedGif skinGif;
    ID2D1Bitmap* pSpriteBitmap = nullptr;

    if (activeSkin.type == "gif") {
        std::filesystem::path gifPath = skinPath / activeSkin.file;
        skinGif = renderer.LoadGifFromFile(gifPath.wstring());
    } else if (activeSkin.type == "sprite") {
        std::filesystem::path spritePath = skinPath / activeSkin.file;
        pSpriteBitmap = renderer.LoadBitmapFromFile(spritePath.wstring());
    }

    // 6. Инициализация системы частиц
    ParticleSystem particleSystem(settings);

    POINT lastMousePos = { -1, -1 };
    auto lastTime = std::chrono::high_resolution_clock::now();
    bool running = true;

    // Главный цикл
    while (running) {
        window.PollEvents(running);

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

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

        // Отрисовка
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

        int sleepMs = static_cast<int>(1000.0f / settings.maxFps);
        if (sleepMs < 1) sleepMs = 1;
        Sleep(sleepMs);
    }

    // Очистка
    skinGif.Cleanup();
    if (pSpriteBitmap) pSpriteBitmap->Release();
    renderer.Cleanup();
    CoUninitialize();

    return 0;
}