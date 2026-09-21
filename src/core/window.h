#pragma once
#include <windows.h>
#include <functional>

class Window {
public:
    Window();
    ~Window();

    bool Init();
    void PollEvents(bool& running);

    HWND GetHwnd() const { return m_hwnd; }
    int GetX() const { return m_screenX; }
    int GetY() const { return m_screenY; }
    int GetWidth() const { return m_screenW; }
    int GetHeight() const { return m_screenH; }

    void SetMessageHandler(std::function<bool(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)> handler) {
        m_msgHandler = handler;
    }

private:
    static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd = nullptr;
    int m_screenX = 0;
    int m_screenY = 0;
    int m_screenW = 0;
    int m_screenH = 0;

    std::function<bool(HWND, UINT, WPARAM, LPARAM)> m_msgHandler;
};