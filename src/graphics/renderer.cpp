#include "Renderer.h"

Renderer::Renderer() = default;

Renderer::~Renderer() {
    Cleanup();
}

bool Renderer::Init(HWND hwnd, int width, int height) {
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pFactory);
    if (FAILED(hr)) return false;

    hr = m_pFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(width, height)),
        &m_pRenderTarget
    );
    if (FAILED(hr)) return false;

    hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &m_pBrush);
    return SUCCEEDED(hr);
}

void Renderer::Cleanup() {
    if (m_pBrush) { m_pBrush->Release(); m_pBrush = nullptr; }
    if (m_pRenderTarget) { m_pRenderTarget->Release(); m_pRenderTarget = nullptr; }
    if (m_pFactory) { m_pFactory->Release(); m_pFactory = nullptr; }
}

void Renderer::BeginDraw() {
    if (m_pRenderTarget) {
        m_pRenderTarget->BeginDraw();
        // Чистый черный фон, который LWA_COLORKEY превращает в 100% прозрачность
        m_pRenderTarget->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f));
    }
}

void Renderer::EndDraw() {
    if (m_pRenderTarget) {
        m_pRenderTarget->EndDraw();
    }
}

void Renderer::DrawParticle(const Particle& p) {
    if (!m_pRenderTarget || !m_pBrush) return;

    // t: от 0.0 (только заспавнился) до 1.0 (перед смертью)
    float t = 1.0f - (p.life / p.maxLife);

    // 1. Интерполяция радиуса (Start -> End)
    float currentRadius = p.startRadius + t * (p.endRadius - p.startRadius);
    if (currentRadius <= 0.1f) return;

    // 2. Линейная интерполяция градиента цвета (LERP RGBA)
    float r = p.startColor.r + t * (p.endColor.r - p.startColor.r);
    float g = p.startColor.g + t * (p.endColor.g - p.startColor.g);
    float b = p.startColor.b + t * (p.endColor.b - p.startColor.b);
    float a = p.startColor.a + t * (p.endColor.a - p.startColor.a);

    m_pBrush->SetColor(D2D1::ColorF(r, g, b, a));

    D2D1_ELLIPSE ellipse = D2D1::Ellipse(
        D2D1::Point2F(p.x, p.y),
        currentRadius,
        currentRadius
    );
    m_pRenderTarget->FillEllipse(ellipse, m_pBrush);
}