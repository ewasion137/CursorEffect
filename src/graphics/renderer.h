#pragma once
#include <windows.h>
#include <d2d1.h>
#include "../particles/Particle.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool Init(HWND hwnd, int width, int height);
    void Cleanup();

    void BeginDraw();
    void EndDraw();

    // Отрисовка частицы с математикой градиента цвета и размера
    void DrawParticle(const Particle& p);

private:
    ID2D1Factory*          m_pFactory = nullptr;
    ID2D1HwndRenderTarget* m_pRenderTarget = nullptr;
    ID2D1SolidColorBrush*  m_pBrush = nullptr;
};