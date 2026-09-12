#include "globals.h"

HWND hwndOverlay = nullptr;
HWND hwndGame = nullptr;
LPDIRECT3D9 pD3D = nullptr;
LPDIRECT3DDEVICE9 pDevice = nullptr;
HANDLE hGame = nullptr;
MSG msg = {};
WNDCLASSEX wc = {};
D3DPRESENT_PARAMETERS d3dpp = {};

int screenWidth = 0;
int screenHeight = 0;
int lastWidth = 0;
int lastHeight = 0;

bool lastKeyState = false;

CheatSettings settings;
// In die globals.cpp ganz unten einfügen:
#include <fstream>
#include <chrono>

void LogDebug(const std::string& message) {
    std::ofstream logFile("cheat_debug.log", std::ios::app);
    if (logFile.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        struct tm timeInfo;
        localtime_s(&timeInfo, &time);

        logFile << "[" << timeInfo.tm_hour << ":" << timeInfo.tm_min << ":" << timeInfo.tm_sec << "] " << message << std::endl;
        logFile.close();
    }
}
