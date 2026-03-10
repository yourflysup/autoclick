#include <Windows.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <sec_api/stdio_s.h>

#include "icon/resource.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx11.h"
#include "module/impl/click_input.hpp"

static ID3D11Device* gDevice{};
static ID3D11DeviceContext* gContext{};
static IDXGISwapChain* gSwap{};
static ID3D11RenderTargetView* gRTV{};

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static void CreateRT() {
    ID3D11Texture2D* backBuffer{};
    gSwap->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    gDevice->CreateRenderTargetView(backBuffer, nullptr, &gRTV);
    backBuffer->Release();
}

static void CleanupRT() {
    if (gRTV) {
        gRTV->Release();
        gRTV = nullptr;
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return TRUE;

    if (msg == WM_SIZE && gDevice && wParam != SIZE_MINIMIZED) {
        CleanupRT();
        gSwap->ResizeBuffers(0, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
        CreateRT();
        return 0;
    }

    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

static const char* KeyName(int vk) {
    static char name[32]{};
    const LPARAM sc = static_cast<LPARAM>(MapVirtualKeyW(vk, MAPVK_VK_TO_VSC)) << 16;
    GetKeyNameTextA(static_cast<LONG>(sc), name, 32);
    return name[0] ? name : "?";
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {
    click_input autoclick;

    WNDCLASSEX wc{
        sizeof(WNDCLASSEX),
        CS_CLASSDC,
        WndProc,
        0, 0,
        hInst,
        LoadIconW(hInst, MAKEINTRESOURCEW(IDI_ICON1)),
        LoadCursor(nullptr, IDC_ARROW),
        nullptr,
        nullptr,
        L"Autoclick - v1.0.0",
        LoadIconW(hInst, MAKEINTRESOURCEW(IDI_ICON1))
    };

    RegisterClassEx(&wc);

    HWND hwnd = CreateWindowW(
        wc.lpszClassName,
        wc.lpszClassName,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        200, 200, 720, 480,
        nullptr, nullptr, hInst, nullptr
    );

    auto hIconBig = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_ICON1));
    auto hIconSmall = static_cast<HICON>(LoadImageW(
        hInst,
        MAKEINTRESOURCEW(IDI_ICON1),
        IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR
    ));

    SetClassLongPtrW(hwnd, GCLP_HICON, reinterpret_cast<LONG_PTR>(hIconBig));
    SetClassLongPtrW(hwnd, GCLP_HICONSM, reinterpret_cast<LONG_PTR>(hIconSmall));
    SendMessageW(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIconBig));
    SendMessageW(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIconSmall));

    constexpr BOOL dark = TRUE;
    constexpr COLORREF bg = RGB(18, 18, 22);
    DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark));
    DwmSetWindowAttribute(hwnd, 35, &bg, sizeof(bg));
    DwmSetWindowAttribute(hwnd, 34, &bg, sizeof(bg));

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;

    D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &sd,
        &gSwap,
        &gDevice,
        nullptr,
        &gContext
    );

    CreateRT();

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.Fonts->AddFontFromFileTTF(R"(C:\\Windows\\Fonts\\segoeui.ttf)", 20.f);
    io.FontDefault = io.Fonts->Fonts[0];

    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowPadding = {20, 20};
    s.FramePadding = {14, 10};
    s.ItemSpacing = {18, 18};
    s.WindowRounding = 14;
    s.FrameRounding = 12;
    s.ScrollbarSize = 0;
    s.WindowBorderSize = 0;
    s.GrabRounding = 12.0f;
    s.GrabMinSize  = 20.0f;

    auto& c = s.Colors;
    c[ImGuiCol_WindowBg] = {0.07f, 0.07f, 0.09f, 1.f};
    c[ImGuiCol_FrameBg] = {0.16f, 0.16f, 0.18f, 1.f};
    c[ImGuiCol_FrameBgHovered] = {0.26f, 0.26f, 0.28f, 1.f};
    c[ImGuiCol_FrameBgActive] = {0.32f, 0.32f, 0.35f, 1.f};
    c[ImGuiCol_Button] = {0.20f, 0.20f, 0.22f, 1.f};
    c[ImGuiCol_ButtonHovered] = {0.30f, 0.30f, 0.33f, 1.f};
    c[ImGuiCol_ButtonActive] = {0.38f, 0.38f, 0.42f, 1.f};
    c[ImGuiCol_Text] = {0.95f, 0.95f, 0.95f, 1.f};
    c[ImGuiCol_TextDisabled] = {0.55f, 0.55f, 0.55f, 1.f};

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(gDevice, gContext);

    bool waitingKey = false;
    MSG msg{};

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        if (!waitingKey && (GetAsyncKeyState(autoclick.toggleKey) & 1))
            autoclick.setStatus(!autoclick.getStatus());

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize(io.DisplaySize);

        ImGui::Begin("##root", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoScrollbar
        );

        constexpr float panelW = 420.f;
        constexpr float spacing = 24.f;

        ImGui::SetCursorPosX((io.DisplaySize.x - panelW) * 0.5f);
        ImGui::BeginGroup();

        ImGui::TextDisabled("TOGGLE");

        char btn[64];
        waitingKey
            ? strcpy_s(btn, "Press a key...")
            : sprintf_s(btn, "Current Key: %s", KeyName(autoclick.toggleKey));

        const float textWidth = ImGui::CalcTextSize(btn).x;
        const float buttonPadding = ImGui::GetStyle().FramePadding.x * 2.0f;
        const float buttonW = textWidth + buttonPadding;

        if (ImGui::Button(btn, {buttonW, 0}))
            waitingKey = true;

        ImGui::SameLine(0.f, spacing);

        const ImVec4 state = autoclick.getStatus()
            ? ImVec4(0.25f, 0.85f, 0.40f, 1.f)
            : ImVec4(0.90f, 0.25f, 0.25f, 1.f);

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.f);
        ImGui::SameLine();
        ImGui::TextColored(state, autoclick.getStatus() ? "ENABLED" : "DISABLED");

        if (waitingKey) {
            for (int i = 8; i < 256; ++i) {
                if (GetAsyncKeyState(i) & 1) {
                    autoclick.toggleKey = i;
                    waitingKey = false;
                    break;
                }
            }
        }

        ImGui::Dummy({0, 28});
        ImGui::TextDisabled("CPS RANGE");

        ImGui::PushItemWidth(panelW);
        ImGui::SliderInt("##min", &autoclick.minCPS, 1, 25, "Min %d");
        ImGui::SliderInt("##max", &autoclick.maxCPS, 1, 25, "Max %d");
        ImGui::PopItemWidth();

        if (autoclick.minCPS > autoclick.maxCPS)
            autoclick.minCPS = autoclick.maxCPS;

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 win_pos = ImGui::GetWindowPos();
        ImVec2 win_size = ImGui::GetWindowSize();
        const char* prefix = "Developed by ";
        const char* name = "Yourflysup";
        ImVec2 size_prefix = ImGui::CalcTextSize(prefix);
        float padding_x = 10.0f;
        float padding_y = 8.0f;
        ImVec2 pos;
        pos.x = win_pos.x + padding_x;
        pos.y = win_pos.y + win_size.y - size_prefix.y - padding_y;
        ImU32 col_gray = ImColor(0.70f, 0.70f, 0.70f, 1.0f);
        draw_list->AddText(pos, col_gray, prefix);
        ImVec2 pos_bold = ImVec2(pos.x + size_prefix.x, pos.y);
        draw_list->AddText(pos_bold, col_gray, name);
        draw_list->AddText(ImVec2(pos_bold.x + 0.5f, pos_bold.y), col_gray, name);

        ImGui::EndGroup();
        ImGui::End();

        ImGui::Render();
        constexpr float clear[4]{0.07f, 0.07f, 0.09f, 1.f};
        gContext->OMSetRenderTargets(1, &gRTV, nullptr);
        gContext->ClearRenderTargetView(gRTV, clear);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        gSwap->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupRT();
    gSwap->Release();
    gContext->Release();
    gDevice->Release();
    UnregisterClass(wc.lpszClassName, hInst);
    return 0;
}
