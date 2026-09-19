#pragma once
#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include <string>
#include "../particles/Particle.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool Init(HWND hwnd, int width, int height);
    void Cleanup();

    void BeginDraw();
    void EndDraw();

    // Загрузка текстуры через WIC
    ID2D1Bitmap* LoadBitmapFromFile(const std::wstring& filePath);

    // Отрисовка точки
    void DrawParticleDot(const Particle& p);

    // Отрисовка спрайта/картинки
    void DrawParticleSprite(const Particle& p, ID2D1Bitmap* pBitmap);

private:
    ID2D1Factory*          m_pFactory = nullptr;
    IWICImagingFactory*    m_pWicFactory = nullptr;
    ID2D1HwndRenderTarget* m_pRenderTarget = nullptr;
    ID2D1SolidColorBrush*  m_pBrush = nullptr;
};