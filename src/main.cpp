#include <windows.h>
#include <d2d1.h>
#include <vector>
#include <chrono>
#include <cmath>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

// --- СТРУКТУРА ПАРТИКЛА ---
struct Particle {
    float x, y;
    float maxLife;
    float life;
    float initialRadius;
};

// Глобальные переменные
ID2D1Factory*          g_pD2DFactory   = nullptr;
ID2D1HwndRenderTarget* g_pRenderTarget = nullptr;
ID2D1SolidColorBrush*  g_pBrush        = nullptr;

std::vector<Particle>  g_Particles;
POINT                  g_LastMousePos  = { -1, -1 };

void SpawnParticles(float fromX, float fromY, float toX, float toY) {
    float dx = toX - fromX;
    float dy = toY - fromY;
    float dist = std::sqrt(dx * dx + dy * dy);

    float step = 6.0f;
    int count = static_cast<int>(dist / step);
    if (count == 0) count = 1;

    for (int i = 0; i <= count; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(count);
        Particle p;
        p.x = fromX + dx * t;
        p.y = fromY + dy * t;
        p.maxLife = 0.45f;
        p.life = p.maxLife;
        p.initialRadius = 10.0f;
        g_Particles.push_back(p);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int main() {
    SetProcessDPIAware();

    HINSTANCE hInstance = GetModuleHandle(nullptr);

    // 1. Регистрация класса окна (Явно используем W-версию)
    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = L"CursorEffectOverlay";
    RegisterClassExW(&wc);

    // 2. Размеры виртуального экрана (все мониторы)
    int screenX = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int screenY = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int screenW = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int screenH = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    // 3. Создаем окно (CreateWindowExW)
    HWND hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        L"CursorEffectOverlay",
        L"CursorEffect",
        WS_POPUP,
        screenX, screenY, screenW, screenH,
        nullptr, nullptr, hInstance, nullptr
    );

    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    ShowWindow(hwnd, SW_SHOW);

    // 4. Инициализация Direct2D
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_pD2DFactory);
    
    RECT rc;
    GetClientRect(hwnd, &rc);
    g_pD2DFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)),
        &g_pRenderTarget
    );

    g_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.7f, 1.0f, 1.0f), &g_pBrush);

    // 5. Главный цикл
    auto lastTime = std::chrono::high_resolution_clock::now();
    bool running = true;
    MSG msg;

    while (running) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) running = false;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        // Выход по клавише ESC
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        POINT mousePos;
        GetCursorPos(&mousePos);

        float localMouseX = static_cast<float>(mousePos.x - screenX);
        float localMouseY = static_cast<float>(mousePos.y - screenY);

        if (g_LastMousePos.x != -1) {
            if (mousePos.x != g_LastMousePos.x || mousePos.y != g_LastMousePos.y) {
                float prevX = static_cast<float>(g_LastMousePos.x - screenX);
                float prevY = static_cast<float>(g_LastMousePos.y - screenY);
                SpawnParticles(prevX, prevY, localMouseX, localMouseY);
            }
        }
        g_LastMousePos = mousePos;

        // Обновление частиц
        for (size_t i = 0; i < g_Particles.size();) {
            g_Particles[i].life -= dt;
            if (g_Particles[i].life <= 0.0f) {
                g_Particles[i] = g_Particles.back();
                g_Particles.pop_back();
            } else {
                ++i;
            }
        }

        // Отрисовка
        g_pRenderTarget->BeginDraw();
        g_pRenderTarget->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f));

        for (const auto& p : g_Particles) {
            float progress = p.life / p.maxLife;
            float currentRadius = p.initialRadius * progress;

            g_pBrush->SetColor(D2D1::ColorF(0.0f, 0.7f * progress, 1.0f * progress, 1.0f));

            D2D1_ELLIPSE ellipse = D2D1::Ellipse(
                D2D1::Point2F(p.x, p.y),
                currentRadius,
                currentRadius
            );
            g_pRenderTarget->FillEllipse(ellipse, g_pBrush);
        }

        g_pRenderTarget->EndDraw();

        Sleep(8);
    }

    if (g_pBrush) g_pBrush->Release();
    if (g_pRenderTarget) g_pRenderTarget->Release();
    if (g_pD2DFactory) g_pD2DFactory->Release();
    DestroyWindow(hwnd);

    return 0;
}