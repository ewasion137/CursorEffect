#include "Renderer.h"
#include <propidl.h>
#include <cmath>

Renderer::Renderer() = default;

Renderer::~Renderer() {
    Cleanup();
}

bool Renderer::Init(HWND hwnd, int width, int height) {
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pFactory);
    if (FAILED(hr)) return false;

    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_pWicFactory)
    );
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
    if (m_pWicFactory) { m_pWicFactory->Release(); m_pWicFactory = nullptr; }
    if (m_pFactory) { m_pFactory->Release(); m_pFactory = nullptr; }
}

ID2D1Bitmap* Renderer::LoadBitmapFromFile(const std::wstring& filePath) {
    if (!m_pWicFactory || !m_pRenderTarget) return nullptr;

    IWICBitmapDecoder* pDecoder = nullptr;
    HRESULT hr = m_pWicFactory->CreateDecoderFromFilename(
        filePath.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder
    );
    if (FAILED(hr)) return nullptr;

    IWICBitmapFrameDecode* pFrame = nullptr;
    hr = pDecoder->GetFrame(0, &pFrame);
    if (FAILED(hr)) { pDecoder->Release(); return nullptr; }

    IWICFormatConverter* pConverter = nullptr;
    hr = m_pWicFactory->CreateFormatConverter(&pConverter);
    if (FAILED(hr)) { pFrame->Release(); pDecoder->Release(); return nullptr; }

    hr = pConverter->Initialize(
        pFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeCustom
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

AnimatedGif Renderer::LoadGifFromFile(const std::wstring& filePath) {
    AnimatedGif gif;
    if (!m_pWicFactory || !m_pRenderTarget) return gif;

    IWICBitmapDecoder* pDecoder = nullptr;
    HRESULT hr = m_pWicFactory->CreateDecoderFromFilename(
        filePath.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder
    );
    if (FAILED(hr)) return gif;

    UINT frameCount = 0;
    hr = pDecoder->GetFrameCount(&frameCount);
    if (FAILED(hr) || frameCount == 0) {
        pDecoder->Release();
        return gif;
    }

    for (UINT i = 0; i < frameCount; ++i) {
        IWICBitmapFrameDecode* pFrame = nullptr;
        if (FAILED(pDecoder->GetFrame(i, &pFrame))) continue;

        // Читаем задержку кадра из метаданных GIF (/grctlext/Delay)
        float frameDelay = 0.05f; // Дефолт: 20 FPS
        IWICMetadataQueryReader* pMetadata = nullptr;
        if (SUCCEEDED(pFrame->GetMetadataQueryReader(&pMetadata))) {
            PROPVARIANT prop;
            PropVariantInit(&prop);
            if (SUCCEEDED(pMetadata->GetMetadataByName(L"/grctlext/Delay", &prop))) {
                if (prop.vt == VT_UI2) {
                    frameDelay = prop.uiVal * 0.01f; // В GIF задержка в сотых долях секунды
                    if (frameDelay < 0.02f) frameDelay = 0.05f; // Защита от нулевой задержки
                }
                PropVariantClear(&prop);
            }
            pMetadata->Release();
        }

        // Конвертируем кадр в Direct2D битмап
        IWICFormatConverter* pConverter = nullptr;
        if (SUCCEEDED(m_pWicFactory->CreateFormatConverter(&pConverter))) {
            if (SUCCEEDED(pConverter->Initialize(
                pFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeCustom
            ))) {
                ID2D1Bitmap* pBitmap = nullptr;
                if (SUCCEEDED(m_pRenderTarget->CreateBitmapFromWicBitmap(pConverter, nullptr, &pBitmap))) {
                    gif.frames.push_back(pBitmap);
                    gif.frameDelays.push_back(frameDelay);
                    gif.totalDuration += frameDelay;
                }
            }
            pConverter->Release();
        }
        pFrame->Release();
    }

    pDecoder->Release();
    return gif;
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
        D2D1::Point2F(p.x, p.y), currentRadius, currentRadius
    );
    m_pRenderTarget->FillEllipse(ellipse, m_pBrush);
}

void Renderer::DrawParticleSprite(const Particle& p, ID2D1Bitmap* pBitmap) {
    if (!m_pRenderTarget || !pBitmap) return;

    float t = 1.0f - (p.life / p.maxLife);
    float currentRadius = p.startRadius + t * (p.endRadius - p.startRadius);
    if (currentRadius <= 0.1f) return;

    float alpha = p.life / p.maxLife;

    D2D1_RECT_F destRect = D2D1::RectF(
        p.x - currentRadius, p.y - currentRadius, p.x + currentRadius, p.y + currentRadius
    );

    m_pRenderTarget->DrawBitmap(
        pBitmap, destRect, alpha, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, nullptr
    );
}

void Renderer::DrawParticleGif(const Particle& p, const AnimatedGif& gif) {
    if (!m_pRenderTarget || gif.frames.empty()) return;

    float t = 1.0f - (p.life / p.maxLife);
    float currentRadius = p.startRadius + t * (p.endRadius - p.startRadius);
    if (currentRadius <= 0.1f) return;

    float alpha = p.life / p.maxLife;

    // Вычисляем нужный кадр по времени жизни
    size_t currentFrame = 0;
    if (gif.totalDuration > 0.001f) {
        float curTime = std::fmod(p.animTime, gif.totalDuration);
        float accum = 0.0f;
        for (size_t i = 0; i < gif.frames.size(); ++i) {
            accum += gif.frameDelays[i];
            if (curTime <= accum) {
                currentFrame = i;
                break;
            }
        }
    }

    ID2D1Bitmap* pCurrentBitmap = gif.frames[currentFrame];

    D2D1_RECT_F destRect = D2D1::RectF(
        p.x - currentRadius, p.y - currentRadius, p.x + currentRadius, p.y + currentRadius
    );

    m_pRenderTarget->DrawBitmap(
        pCurrentBitmap, destRect, alpha, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, nullptr
    );
}