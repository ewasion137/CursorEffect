#include "config/ConfigManager.h"
#include "core/Window.h"
#include "graphics/Renderer.h"
#include "particles/ParticleSystem.h"
#include <chrono>

int main() {
    // 1. Инициализация и чтение конфигов (~/.cureff/settings.cfg)
    ConfigManager::Init();
    Settings settings = ConfigManager::Load();

    // 2. Создание окна оверлея
    Window window;
    if (!window.Init()) return -1;

    // 3. Инициализация Direct2D рендера
    Renderer renderer;
    if (!renderer.Init(window.GetHwnd(), window.GetWidth(), window.GetHeight())) return -1;

    // 4. Движок частиц
    ParticleSystem particleSystem(settings);

    POINT lastMousePos = { -1, -1 };
    auto lastTime = std::chrono::high_resolution_clock::now();
    bool running = true;

    // Главный игровой цикл
    while (running) {
        window.PollEvents(running);

        // Быстрый выход по кнопке ESC
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }

        // Расчет deltaTime
        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Трекинг позиции мыши
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

        // Физика и жизнь частиц
        particleSystem.Update(dt);

        // Отрисовка кадра
        renderer.BeginDraw();
        for (const auto& particle : particleSystem.GetParticles()) {
            renderer.DrawParticle(particle);
        }
        renderer.EndDraw();

        // Ограничение частоты кадров (~120 FPS)
        int sleepMs = static_cast<int>(1000.0f / settings.maxFps);
        if (sleepMs < 1) sleepMs = 1;
        Sleep(sleepMs);
    }

    return 0;
}