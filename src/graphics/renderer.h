#pragma once
#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include <string>
#include <vector>
#include "../particles/Particle.h"
#include "../skins/Skin.h"

// Контейнер для разобранной гифки
struct AnimatedGif {
    std::vector<ID2D1Bitmap*> frames;     // Все кадры на видеокарте
    std::vector<float> frameDelays;       // Задержка каждого кадра в секундах
    float totalDuration = 0.0f;           // Длительность полного круга

    void Cleanup() {
        for (auto* f : frames) {
            if (f) f->Release();
        }
        frames.clear();
        frameDelays.clear();
        totalDuration = 0.0f;
    }
};

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool Init(HWND hwnd, int width, int height);
    void Cleanup();

    void BeginDraw();
    void EndDraw();


    ID2D1Bitmap* LoadBitmapFromFile(const std::wstring& filePath);
    AnimatedGif LoadGifFromFile(const std::wstring& filePath); // <-- Загрузка GIF

    void DrawParticleDot(const Particle& p);
    void DrawParticleSprite(const Particle& p, ID2D1Bitmap* pBitmap);
    void DrawParticleGif(const Particle& p, const AnimatedGif& gif, const Skin& skin); // <-- Отрисовка кадра GIF

private:
    ID2D1Factory*          m_pFactory = nullptr;
    IWICImagingFactory*    m_pWicFactory = nullptr;
    ID2D1HwndRenderTarget* m_pRenderTarget = nullptr;
    ID2D1SolidColorBrush*  m_pBrush = nullptr;
};