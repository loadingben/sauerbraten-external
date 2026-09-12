#include "gamesutil.h"
#include "globals.h"
#include <iostream>
#include <psapi.h>
// Füge diese Zeile ganz oben in deine gamesutil.cpp ein:
extern uintptr_t gameBaseAddress;



DWORD GetProcessID(const wchar_t* exeName) {
    PROCESSENTRY32W entry = { sizeof(entry) };
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;

    while (Process32NextW(snapshot, &entry)) {
        if (_wcsicmp(entry.szExeFile, exeName) == 0) {
            DWORD pid = entry.th32ProcessID;
            CloseHandle(snapshot);
            return pid;
        }
    }

    CloseHandle(snapshot);
    return 0;
}

HANDLE OpenGameProcess(DWORD pid, const wchar_t* exeName) {
    if (!pid) return nullptr;
    std::wcout << L"[+] Attached to Process: " << exeName << L" (PID: " << pid << L")\n";
    return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
}

HWND FindGameWindow(DWORD pid) {
    HWND hwnd = nullptr;
    while ((hwnd = FindWindowEx(nullptr, hwnd, nullptr, nullptr))) {
        DWORD windowPID = 0;
        GetWindowThreadProcessId(hwnd, &windowPID);
        if (windowPID == pid && IsWindowVisible(hwnd)) {
            return hwnd;
        }
    }
    return nullptr;
}

bool InitGame() {
    DWORD pid = GetProcessID(sauerbraten_procname);
    if (!pid) {
        std::wcerr << L"[!] Failed to find process: " << sauerbraten_procname << std::endl;
        return false;
    }

    hGame = OpenGameProcess(pid, sauerbraten_procname);
    if (!hGame) {
        std::wcerr << L"[!] Failed to open process handle." << std::endl;
        return false;
    }

    // FIX: UNFEHLBARER 64-BIT BASISADRESSEN FINDER ÜBER ENUMPROCESSMODULES
    HMODULE hMods[1024];
    DWORD cbNeeded;
    if (EnumProcessModules(hGame, hMods, sizeof(hMods), &cbNeeded)) {
        // Das allererste Modul im Array [0] ist IMMER die Haupt-EXE (sauerbraten.exe)
        gameBaseAddress = reinterpret_cast<uintptr_t>(hMods[0]);
    }
    else {
        std::wcerr << L"[!] EnumProcessModules fehlgeschlagen. Error: " << GetLastError() << std::endl;
    }

    hwndGame = FindGameWindow(pid);
    if (!hwndGame) {
        std::wcerr << L"[!] Could not find game window." << std::endl;
        return false;
    }

    return true;
}
