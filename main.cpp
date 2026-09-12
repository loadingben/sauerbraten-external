#include "globals.h"
#include "vec.h"
#include "memory.h"
#include "player.h"
#include "cheats.h"
#include "render.h"
#include "gamesutil.h"
#include <sstream>

#pragma comment(lib, "d3d9.lib")

uintptr_t gameBaseAddress = 0;

int main() {
    LogDebug("=== Diagnose-Tool gestartet ===");

    if (!InitGame()) {
        std::cerr << "[!] Sauerbraten wurde nicht gefunden! Bitte starte das Spiel zuerst." << std::endl;
        Sleep(3000);
        return -1;
    }

    ImGuiIO& io = InitOverlay();
    int lastWindowX = 0, lastWindowY = 0;

    // === REPARATUR: Synchronisiere den Startzustand, damit nichts feststeckt ===
    settings.showMenu = false;
    io.MouseDrawCursor = false;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouse;

    // Macht das Fenster beim Starten fuer Windows-Klicks absolut transparent
    LONG startStyle = GetWindowLong(hwndOverlay, GWL_EXSTYLE);
    SetWindowLong(hwndOverlay, GWL_EXSTYLE, startStyle | WS_EX_TRANSPARENT);

    while (msg.message != WM_QUIT) {
        HandleWindowMessages(msg);

        DWORD exitCode = 0;
        if (GetExitCodeProcess(hGame, &exitCode) && exitCode != STILL_ACTIVE) {
            LogDebug("Spiel wurde geschlossen. Beende Cheat...");
            break;
        }

        RECT gameRect;
        if (GetClientRect(hwndGame, &gameRect)) {
            POINT topLeft = { 0, 0 };
            ClientToScreen(hwndGame, &topLeft);
            if (topLeft.x != lastWindowX || topLeft.y != lastWindowY || gameRect.right != screenWidth || gameRect.bottom != screenHeight) {
                UpdateRenderDimensions();
                lastWindowX = topLeft.x;
                lastWindowY = topLeft.y;
            }
        }

        if (!HandleDeviceResetIfNeeded(lastWidth, lastHeight))
            break;

        io.DisplaySize = ImVec2((float)screenWidth, (float)screenHeight);

        if (IsGameMinimized()) {
            ShowWindow(hwndOverlay, SW_HIDE);
            continue;
        }
        else {
            ShowWindow(hwndOverlay, SW_SHOW);
        }

        // --- DER ORIGINALE, BOMBENSICHERE F5-TOGGLE ---
        static bool f5Pressed = false;
        if (GetAsyncKeyState(VK_F5) & 0x8000) {
            if (!f5Pressed) {
                settings.showMenu = !settings.showMenu;
                f5Pressed = true;

                LONG exStyle = GetWindowLong(hwndOverlay, GWL_EXSTYLE);
                if (settings.showMenu) {
                    // Overlay aktivieren und klickbar machen
                    SetWindowLong(hwndOverlay, GWL_EXSTYLE, exStyle & ~WS_EX_TRANSPARENT);
                    io.MouseDrawCursor = true;
                    io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;

                    SendMessage(hwndGame, WM_ACTIVATE, WA_INACTIVE, 0);
                    SetForegroundWindow(hwndOverlay);
                    SetActiveWindow(hwndOverlay);
                }
                else {
                    // Overlay komplett ausschalten, Klicks gehen wieder durch ins Spiel
                    SetWindowLong(hwndOverlay, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT);
                    io.MouseDrawCursor = false;
                    io.ConfigFlags |= ImGuiConfigFlags_NoMouse;

                    SendMessage(hwndGame, WM_ACTIVATE, WA_ACTIVE, 0);
                    SetForegroundWindow(hwndGame);
                }
            }
        }
        else {
            f5Pressed = false;
        }

        // Die Maus-Position wird nur aktualisiert, wenn das Menü offen ist, um Locks zu verhindern
        if (settings.showMenu) {
            POINT mousePos;
            GetCursorPos(&mousePos);
            ScreenToClient(hwndOverlay, &mousePos);
            io.MousePos = ImVec2((float)mousePos.x, (float)mousePos.y);
            io.MouseDown[0] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        }

        Player localPlayer = GetLocalPlayer();
        auto players = BuildPlayerList(localPlayer);

        BeginImGuiFrame(io);

        DrawFOV();
        DrawESP(players);
        Aimbot(players, localPlayer);
        DrawMenu(localPlayer);

        EndImGuiFrame();
        Sleep(5);
    }

    Cleanup();
    return 0;
}
