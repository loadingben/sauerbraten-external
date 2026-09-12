#pragma once
#include <cstdint>
#include <Windows.h>
#include <TlHelp32.h>
#include <d3d9.h>
#include "imgui/imgui.h"
#include <vector>
#include <string>
#include <iostream>
#include "vec.h"

namespace offsets
{
    inline constexpr uintptr_t local_player = 0x2A5730;
    inline constexpr uintptr_t entity_list = 0x346C90;
    inline constexpr uintptr_t player_count = 0x346C9C;
    inline constexpr uintptr_t viewmatrix = 0x32D040;

    namespace player
    {
        inline constexpr uintptr_t position = 0x0;
        inline constexpr uintptr_t yaw = 0x3C;
        inline constexpr uintptr_t pitch = 0x40;
        inline constexpr uintptr_t health = 0x178;
        inline constexpr uintptr_t armor = 0x180;
        inline constexpr uintptr_t name = 0x274;
        inline constexpr uintptr_t team = 0x378;
        inline constexpr uintptr_t ammo_start = 0x198;
    }
}

inline const wchar_t* sauerbraten_procname = L"sauerbraten.exe";

extern HWND hwndOverlay;
extern HWND hwndGame;
extern LPDIRECT3D9 pD3D;
extern LPDIRECT3DDEVICE9 pDevice;
extern HANDLE hGame;
extern MSG msg;
extern WNDCLASSEX wc;
extern D3DPRESENT_PARAMETERS d3dpp;

extern int screenWidth;
extern int screenHeight;
extern int lastWidth;
extern int lastHeight;
extern bool lastKeyState;

struct CheatSettings {
    bool showMenu = false;
    bool showSnaplines = true;
    bool showEnemies = true;
    bool showFriendlies = true;
    bool godmode = true;
    bool infiniteAmmo = true;
    bool enableAimbot = true;
    bool showFovCircle = true;
    float fovSize = 400.0f;

    float colorEnemy[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
    float colorFriendly[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    float colorFov[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
    float colorSnapline[4] = { 1.0f, 0.0f, 0.0f, 0.58f };

    bool enableSmoothing = true;
    float smoothFactor = 5.0f;
    bool enableSilentAim = false;
    int silentAimDelay = 2;
};

extern CheatSettings settings;
void LogDebug(const std::string& message);
