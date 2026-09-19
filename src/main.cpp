#include "config/ConfigManager.h"
#include "core/Window.h"
#include "graphics/Renderer.h"
#include "particles/ParticleSystem.h"
#include <chrono>
#include <objbase.h>

int main() {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    ConfigManager::Init();
    Settings settings = ConfigManager::Load();

    Window window;
    if (!window.Init()) {
        CoUninitialize();
        return -1;
    }

    Renderer renderer;
    if (!renderer.Init(window.GetHwnd(), window.GetWidth(), window.GetHeight())) {
        CoUninitialize();
        return -1;
    }

    // Ресурсы
    ID2D1Bitmap* pSpriteBitmap = nullptr;
    AnimatedGif gifAnimation;

    if (settings.mode == "sprite") {
        std::filesystem::path path = ConfigManager::GetConfigDir() / settings.spriteFile;
        pSpriteBitmap = renderer.LoadBitmapFromFile(path.wstring());
    } else if (settings.mode == "gif") {
        std::filesystem::path path = ConfigManager::GetConfigDir() / settings.gifFile;
        gifAnimation = renderer.LoadGifFromFile(path.wstring());
    }

    ParticleSystem particleSystem(settings);

    POINT lastMousePos = { -1, -1 };
    auto lastTime = std::chrono::high_resolution_clock::now();
    bool running = true;

    while (running) {
        window.PollEvents(running);

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) break;

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
            if (settings.mode == "gif" && !gifAnimation.frames.empty()) {
                renderer.DrawParticleGif(particle, gifAnimation);
            } else if (settings.mode == "sprite" && pSpriteBitmap) {
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

    gifAnimation.Cleanup();
    if (pSpriteBitmap) pSpriteBitmap->Release();
    renderer.Cleanup();
    CoUninitialize();
    return 0;
}