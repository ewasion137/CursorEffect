#include "Window.h"

Window::Window() = default;

Window::~Window() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
    }
}

LRESULT CALLBACK Window::StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

bool Window::Init() {
    SetProcessDPIAware();
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc   = StaticWindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = L"CursorEffectOverlayClass";
    RegisterClassExW(&wc);

    // Учитываем конфигурации с несколькими мониторами
    m_screenX = GetSystemMetrics(SM_XVIRTUALSCREEN);
    m_screenY = GetSystemMetrics(SM_YVIRTUALSCREEN);
    m_screenW = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    m_screenH = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    m_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        wc.lpszClassName,
        L"CursorEffectOverlay",
        WS_POPUP,
        m_screenX, m_screenY, m_screenW, m_screenH,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!m_hwnd) return false;

    // Ключ прозрачности: черные пиксели окна становятся прозрачным сквозным холстом
    SetLayeredWindowAttributes(m_hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    ShowWindow(m_hwnd, SW_SHOW);
    return true;
}

void Window::PollEvents(bool& running) {
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            running = false;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}