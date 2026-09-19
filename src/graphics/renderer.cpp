#include "Renderer.h"

Renderer::Renderer() = default;

Renderer::~Renderer() {
    Cleanup();
}

bool Renderer::Init(HWND hwnd, int width, int height) {
    // 1. Создаем Direct2D фабрику
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pFactory);
    if (FAILED(hr)) return false;

    // 2. Создаем WIC фабрику для чтения картинок
    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_pWicFactory)
    );
    if (FAILED(hr)) return false;

    // 3. Создаем RenderTarget
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
    if (m_pWicFactory) { m_pWicFactory->Release(); m_pWicFactory = nullptr; }
    if (m_pFactory) { m_pFactory->Release(); m_pFactory = nullptr; }
}

ID2D1Bitmap* Renderer::LoadBitmapFromFile(const std::wstring& filePath) {
    if (!m_pWicFactory || !m_pRenderTarget) return nullptr;

    IWICBitmapDecoder* pDecoder = nullptr;
    HRESULT hr = m_pWicFactory->CreateDecoderFromFilename(
        filePath.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &pDecoder
    );
    if (FAILED(hr)) return nullptr;

    IWICBitmapFrameDecode* pFrame = nullptr;
    hr = pDecoder->GetFrame(0, &pFrame);
    if (FAILED(hr)) { pDecoder->Release(); return nullptr; }

    IWICFormatConverter* pConverter = nullptr;
    hr = m_pWicFactory->CreateFormatConverter(&pConverter);
    if (FAILED(hr)) { pFrame->Release(); pDecoder->Release(); return nullptr; }

    // Конвертируем в 32bpp PBGRA (нативный формат для Direct2D с прозрачностью)
    hr = pConverter->Initialize(
        pFrame,
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0f,
        WICBitmapPaletteTypeCustom
    );

    ID2D1Bitmap* pD2DBitmap = nullptr;
    if (SUCCEEDED(hr)) {
        m_pRenderTarget->CreateBitmapFromWicBitmap(pConverter, nullptr, &pD2DBitmap);
    }

    pConverter->Release();
    pFrame->Release();
    pDecoder->Release();
    return pD2DBitmap;
}

void Renderer::BeginDraw() {
    if (m_pRenderTarget) {
        m_pRenderTarget->BeginDraw();
        m_pRenderTarget->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f));
    }
}

void Renderer::EndDraw() {
    if (m_pRenderTarget) {
        m_pRenderTarget->EndDraw();
    }
}

void Renderer::DrawParticleDot(const Particle& p) {
    if (!m_pRenderTarget || !m_pBrush) return;

    float t = 1.0f - (p.life / p.maxLife);
    float currentRadius = p.startRadius + t * (p.endRadius - p.startRadius);
    if (currentRadius <= 0.1f) return;

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

void Renderer::DrawParticleSprite(const Particle& p, ID2D1Bitmap* pBitmap) {
    if (!m_pRenderTarget || !pBitmap) return;

    float t = 1.0f - (p.life / p.maxLife);
    float currentRadius = p.startRadius + t * (p.endRadius - p.startRadius);
    if (currentRadius <= 0.1f) return;

    // Альфа-затухание со временем жизни
    float alpha = p.life / p.maxLife;

    D2D1_RECT_F destRect = D2D1::RectF(
        p.x - currentRadius,
        p.y - currentRadius,
        p.x + currentRadius,
        p.y + currentRadius
    );

    m_pRenderTarget->DrawBitmap(
        pBitmap,
        destRect,
        alpha,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
        nullptr
    );
}