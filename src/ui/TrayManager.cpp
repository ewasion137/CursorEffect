#include "TrayManager.h"
#include "../config/ConfigManager.h"
#include "../../resources/resource.h"
#include <wincodec.h>
#include <vector>

namespace {
    constexpr UINT IDM_TITLE        = 3000;
    constexpr UINT IDM_TOGGLE_TRAIL = 3001;
    constexpr UINT IDM_RELOAD       = 3002;
    constexpr UINT IDM_OPEN_DIR     = 3003;
    constexpr UINT IDM_ABOUT        = 3004;
    constexpr UINT IDM_EXIT         = 3005;
    constexpr UINT IDM_SKIN_DEFAULT = 3100;
    constexpr UINT IDM_SKIN_BASE    = 3200;
}

TrayManager::TrayManager() = default;

TrayManager::~TrayManager() {
    Remove();
}

HICON TrayManager::CreateIconFromPng(const std::wstring& filePath) {
    IWICImagingFactory* pFactory = nullptr;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pFactory)
    );
    if (FAILED(hr) || !pFactory) return nullptr;

    IWICBitmapDecoder* pDecoder = nullptr;
    hr = pFactory->CreateDecoderFromFilename(
        filePath.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder
    );
    if (FAILED(hr) || !pDecoder) {
        pFactory->Release();
        return nullptr;
    }

    IWICBitmapFrameDecode* pFrame = nullptr;
    hr = pDecoder->GetFrame(0, &pFrame);
    if (FAILED(hr) || !pFrame) {
        pDecoder->Release();
        pFactory->Release();
        return nullptr;
    }

    int iconSize = GetSystemMetrics(SM_CXSMICON);
    if (iconSize <= 0) iconSize = 16;

    IWICBitmapScaler* pScaler = nullptr;
    hr = pFactory->CreateBitmapScaler(&pScaler);
    if (SUCCEEDED(hr)) {
        pScaler->Initialize(pFrame, iconSize, iconSize, WICBitmapInterpolationModeHighQualityCubic);
    }

    IWICFormatConverter* pConverter = nullptr;
    hr = pFactory->CreateFormatConverter(&pConverter);
    if (FAILED(hr)) {
        if (pScaler) pScaler->Release();
        pFrame->Release();
        pDecoder->Release();
        pFactory->Release();
        return nullptr;
    }

    hr = pConverter->Initialize(
        pScaler ? (IWICBitmapSource*)pScaler : (IWICBitmapSource*)pFrame,
        GUID_WICPixelFormat32bppBGRA,
        WICBitmapDitherTypeNone,
        nullptr, 0.0f, WICBitmapPaletteTypeCustom
    );

    HICON hIcon = nullptr;
    if (SUCCEEDED(hr)) {
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = iconSize;
        bmi.bmiHeader.biHeight = -iconSize; // top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* pBits = nullptr;
        HDC hdc = GetDC(nullptr);
        HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
        ReleaseDC(nullptr, hdc);

        if (hBitmap && pBits) {
            UINT stride = iconSize * 4;
            UINT bufferSize = stride * iconSize;
            hr = pConverter->CopyPixels(nullptr, stride, bufferSize, static_cast<BYTE*>(pBits));
            if (SUCCEEDED(hr)) {
                HBITMAP hMask = CreateBitmap(iconSize, iconSize, 1, 1, nullptr);
                ICONINFO ii = {};
                ii.fIcon = TRUE;
                ii.hbmColor = hBitmap;
                ii.hbmMask = hMask;
                hIcon = CreateIconIndirect(&ii);
                if (hMask) DeleteObject(hMask);
            }
            DeleteObject(hBitmap);
        }
    }

    if (pConverter) pConverter->Release();
    if (pScaler) pScaler->Release();
    pFrame->Release();
    pDecoder->Release();
    pFactory->Release();
    return hIcon;
}

HICON TrayManager::LoadIconFromFileOrResource(const std::wstring& filePath) {
    HINSTANCE hInst = GetModuleHandleW(nullptr);
    HICON hIcon = (HICON)LoadImageW(
        hInst, MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR
    );
    if (hIcon) return hIcon;

    if (std::filesystem::exists(filePath)) {
        hIcon = CreateIconFromPng(filePath);
        if (hIcon) return hIcon;
    }

    return LoadIconW(nullptr, IDI_APPLICATION);
}

bool TrayManager::Init(HWND hwnd, const std::wstring& logoPath) {
    m_hwnd = hwnd;
    m_hIcon = LoadIconFromFileOrResource(logoPath);

    ZeroMemory(&m_nid, sizeof(m_nid));
    m_nid.cbSize = sizeof(NOTIFYICONDATAW);
    m_nid.hWnd = m_hwnd;
    m_nid.uID = 1;
    m_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAYICON;
    m_nid.hIcon = m_hIcon;

    wcscpy_s(m_nid.szTip, L"CursorEffect\nLeft-click: Open folder | Right-click: Menu");

    if (Shell_NotifyIconW(NIM_ADD, &m_nid)) {
        m_installed = true;
        ShowBalloon(L"CursorEffect", L"Running in system tray.\nRight-click icon for settings.");
        return true;
    }
    return false;
}

void TrayManager::Remove() {
    if (m_installed) {
        Shell_NotifyIconW(NIM_DELETE, &m_nid);
        m_installed = false;
    }
    if (m_hIcon) {
        DestroyIcon(m_hIcon);
        m_hIcon = nullptr;
    }
}

void TrayManager::ShowBalloon(const std::wstring& title, const std::wstring& message) {
    if (!m_installed) return;

    m_nid.uFlags |= NIF_INFO;
    wcscpy_s(m_nid.szInfoTitle, title.c_str());
    wcscpy_s(m_nid.szInfo, message.c_str());
    m_nid.dwInfoFlags = NIIF_INFO;

    Shell_NotifyIconW(NIM_MODIFY, &m_nid);
}

void TrayManager::HandleTrayMessage(HWND hwnd, LPARAM lParam) {
    switch (lParam) {
        case WM_RBUTTONUP:
        case WM_CONTEXTMENU: {
            ShowContextMenu(hwnd);
            break;
        }
        case WM_LBUTTONDBLCLK: {
            if (onOpenConfigDir) {
                onOpenConfigDir();
            }
            break;
        }
    }
}

void TrayManager::ShowContextMenu(HWND hwnd) {
    HMENU hMenu = CreatePopupMenu();
    if (!hMenu) return;

    // Header title
    AppendMenuW(hMenu, MF_STRING | MF_DISABLED, IDM_TITLE, L"CursorEffect 1.0");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);

    // Trail toggle
    bool trailOn = getTrailEnabled ? getTrailEnabled() : true;
    UINT trailFlags = MF_STRING | (trailOn ? MF_CHECKED : MF_UNCHECKED);
    AppendMenuW(hMenu, trailFlags, IDM_TOGGLE_TRAIL, L"Enable Trail");

    // Skins submenu
    HMENU hSkinMenu = CreatePopupMenu();
    std::string currentSkin = getActiveSkin ? getActiveSkin() : "default";

    UINT defaultFlags = MF_STRING | ((currentSkin.empty() || currentSkin == "default") ? MF_CHECKED : MF_UNCHECKED);
    AppendMenuW(hSkinMenu, defaultFlags, IDM_SKIN_DEFAULT, L"Default (Dot)");

    auto skins = ConfigManager::GetAvailableSkins();
    if (!skins.empty()) {
        AppendMenuW(hSkinMenu, MF_SEPARATOR, 0, nullptr);
    }
    for (size_t i = 0; i < skins.size(); ++i) {
        UINT skinFlags = MF_STRING | (skins[i] == currentSkin ? MF_CHECKED : MF_UNCHECKED);
        std::wstring skinNameW(skins[i].begin(), skins[i].end());
        AppendMenuW(hSkinMenu, skinFlags, IDM_SKIN_BASE + static_cast<UINT>(i), skinNameW.c_str());
    }
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSkinMenu, L"Skins");

    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, IDM_RELOAD, L"Reload Config");
    AppendMenuW(hMenu, MF_STRING, IDM_OPEN_DIR, L"Open Config Folder...");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, IDM_ABOUT, L"About...");
    AppendMenuW(hMenu, MF_STRING, IDM_EXIT, L"Exit");

    POINT pt;
    GetCursorPos(&pt);

    SetForegroundWindow(hwnd);
    UINT cmd = TrackPopupMenuEx(
        hMenu,
        TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY,
        pt.x, pt.y,
        hwnd,
        nullptr
    );
    PostMessageW(hwnd, WM_NULL, 0, 0);

    if (cmd == IDM_TOGGLE_TRAIL) {
        if (onToggleTrail) onToggleTrail(!trailOn);
    } else if (cmd == IDM_SKIN_DEFAULT) {
        if (onSelectSkin) onSelectSkin("default");
    } else if (cmd >= IDM_SKIN_BASE && (cmd - IDM_SKIN_BASE) < skins.size()) {
        if (onSelectSkin) onSelectSkin(skins[cmd - IDM_SKIN_BASE]);
    } else if (cmd == IDM_RELOAD) {
        if (onReloadConfig) onReloadConfig();
    } else if (cmd == IDM_OPEN_DIR) {
        if (onOpenConfigDir) onOpenConfigDir();
    } else if (cmd == IDM_ABOUT) {
        if (onAbout) onAbout();
    } else if (cmd == IDM_EXIT) {
        if (onExit) onExit();
    }

    DestroyMenu(hSkinMenu);
    DestroyMenu(hMenu);
}
