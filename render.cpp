#include "render.h"
#include "globals.h"
#include "memory.h"
#include "cheats.h"
#include "imgui/backends/imgui_impl_dx9.h"
#include "imgui/backends/imgui_impl_win32.h"
#include <dwmapi.h>
#include <iostream>

#pragma comment(lib, "dwmapi.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool CreateOverlayWindow() {
    wc = {
        sizeof(WNDCLASSEX),
        CS_CLASSDC,
        WndProc,
        0L,
        0L,
        GetModuleHandle(NULL),
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        L"OverlayClass",
        nullptr
    };

    RegisterClassEx(&wc);
    HWND parentWindow = CreateWindowEx(0, L"STATIC", nullptr, WS_DISABLED, 0, 0, 0, 0, nullptr, nullptr, wc.hInstance, nullptr);

    hwndOverlay = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
        L"OverlayClass", L"Overlay",
        WS_POPUP, 100, 100, 800, 600,
        parentWindow, nullptr, wc.hInstance, nullptr
    );

    MARGINS margins = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(hwndOverlay, &margins);

    SetLayeredWindowAttributes(hwndOverlay, RGB(0, 0, 0), 255, LWA_ALPHA);
    ShowWindow(hwndOverlay, SW_SHOW);
    return true;
}

bool CreateDevice() {
    pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!pD3D) return false;

    d3dpp = {};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = hwndOverlay;
    d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
    d3dpp.BackBufferWidth = 800;
    d3dpp.BackBufferHeight = 600;

    return SUCCEEDED(pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwndOverlay, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDevice));
}

ImGuiIO& InitOverlay() {
    CreateOverlayWindow();
    CreateDevice();

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwndOverlay);
    ImGui_ImplDX9_Init(pDevice);

    return io;
}

void BeginImGuiFrame(ImGuiIO& io) {
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void EndImGuiFrame() {
    ImGui::Render();
    pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
    if (pDevice->BeginScene() >= 0) {
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        pDevice->EndScene();
    }
    pDevice->Present(NULL, NULL, NULL, NULL);
}

void Cleanup() {
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (pDevice) pDevice->Release();
    if (pD3D) pD3D->Release();

    UnregisterClass(wc.lpszClassName, wc.hInstance);
}

void DrawMenu(const Player& localPlayer) {
    // === REPARATUR: Wenn das Menü zu sein soll, breche SOFORT ab und zeichne nichts ===
    if (!settings.showMenu)
        return;

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.WindowPadding = ImVec2(15, 15);
    style.ItemSpacing = ImVec2(10, 10);

    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.09f, 0.11f, 0.95f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.22f, 0.33f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.17f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.25f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.20f, 0.29f, 0.42f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.40f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.29f, 0.42f, 1.00f);

    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(420, 350), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Sauerbraten External Framework v2.0", &settings.showMenu, ImGuiWindowFlags_NoCollapse)) {
        if (ImGui::BeginTabBar("FeaturesTabBar", ImGuiTabBarFlags_None)) {
            if (ImGui::BeginTabItem("Diagnostics")) {
                ImGui::TextColored(ImVec4(0.3f, 0.7f, 1.0f, 1.0f), "Local Player Data:");
                ImGui::Separator();
                ImGui::Text("Name:   %s", localPlayer.GetName());
                ImGui::Text("Team:   %s", localPlayer.GetTeam() == 1 ? "Evil" : "Good/Claw");
                ImGui::Text("Health: %d HP", localPlayer.GetHealth());
                ImGui::Text("Armor:  %d AP", localPlayer.GetArmor());

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Camera Angles:");
                ImGui::Text("Yaw:    %.1f Grad", localPlayer.GetYaw());
                ImGui::Text("Pitch:  %.1f Grad", localPlayer.GetPitch());
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Combat")) {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Aimbot Settings:");
                ImGui::Separator();
                ImGui::Checkbox("Enable Master Aimbot", &settings.enableAimbot);
                ImGui::Checkbox("Flick Silent Aim", &settings.enableSilentAim);

                if (settings.enableSilentAim) {
                    ImGui::SliderInt("Flick Duration (ms)", &settings.silentAimDelay, 0, 20, "%d ms");
                }
                else {
                    ImGui::Checkbox("Enable Smooth Aim", &settings.enableSmoothing);
                    if (settings.enableSmoothing) {
                        ImGui::SliderFloat("Smooth Factor", &settings.smoothFactor, 1.0f, 30.0f, "%.1f");
                    }
                }
                ImGui::Separator();
                ImGui::Checkbox("Draw FOV Target Circle", &settings.showFovCircle);
                if (settings.showFovCircle) {
                    ImGui::SliderFloat("FOV Radius", &settings.fovSize, 30.0f, 800.0f, "%.0f px");
                    ImGui::ColorEdit4("FOV Circle Color", settings.colorFov, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Visuals")) {
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Extra Sensory Perception:");
                ImGui::Separator();
                ImGui::Checkbox("Draw Enemy Boxes", &settings.showEnemies);
                ImGui::SameLine(250); ImGui::ColorEdit4("##EnemyColor", settings.colorEnemy, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                ImGui::Checkbox("Draw Friendly Boxes", &settings.showFriendlies);
                ImGui::SameLine(250); ImGui::ColorEdit4("##FriendlyColor", settings.colorFriendly, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                ImGui::Checkbox("Draw Ground Snaplines", &settings.showSnaplines);
                ImGui::SameLine(250); ImGui::ColorEdit4("##SnapColor", settings.colorSnapline, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Misc / Trainer")) {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "Memory Modification:");
                ImGui::Separator();
                ImGui::Checkbox("Infinite Health (Godmode)", &settings.godmode);
                ImGui::Checkbox("Infinite Weapon Ammo", &settings.infiniteAmmo);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void UpdateRenderDimensions() {
    RECT clientRect;
    GetClientRect(hwndGame, &clientRect);
    POINT topLeft = { 0, 0 };
    ClientToScreen(hwndGame, &topLeft);

    screenWidth = clientRect.right;
    screenHeight = clientRect.bottom;
    MoveWindow(hwndOverlay, topLeft.x, topLeft.y, screenWidth, screenHeight, TRUE);
}

bool IsGameMinimized() {
    WINDOWPLACEMENT placement = { sizeof(WINDOWPLACEMENT) };
    GetWindowPlacement(hwndGame, &placement);
    return placement.showCmd == SW_SHOWMINIMIZED;
}

void HandleWindowMessages(MSG& msg) {
    while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

bool HandleDeviceResetIfNeeded(int& lastWidth, int& lastHeight) {
    if (screenWidth == lastWidth && screenHeight == lastHeight) return true;
    if (settings.showMenu && ImGui::IsAnyMouseDown()) return true;

    lastWidth = screenWidth;
    lastHeight = screenHeight;

    ImGui_ImplDX9_InvalidateDeviceObjects();

    d3dpp.BackBufferWidth = screenWidth;
    d3dpp.BackBufferHeight = screenHeight;

    HRESULT hr = pDevice->Reset(&d3dpp);
    if (FAILED(hr)) return false;

    ImGui_ImplDX9_CreateDeviceObjects();
    return true;
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // REINER EXTERNAL-ZUSTAND: Reicht Eingaben nur an ImGui weiter, wenn das Menü offen ist
    if (settings.showMenu) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;
    }

    switch (msg) {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED) return 0;
        screenWidth = (UINT)LOWORD(lParam);
        screenHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
