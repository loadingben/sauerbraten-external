#ifndef MEMORY_H
#define MEMORY_H

#include <Windows.h>

extern HANDLE hGame;

template<typename T>
inline T RPM(uintptr_t address) {
    T buffer{};
    if (address < 0x100000) return buffer;
    ReadProcessMemory(hGame, reinterpret_cast<LPCVOID>(address), &buffer, sizeof(T), nullptr);
    return buffer;
}

template<typename T>
inline void WPM(uintptr_t address, const T& value) {
    if (address < 0x100000) return;
    WriteProcessMemory(hGame, reinterpret_cast<LPVOID>(address), &value, sizeof(T), nullptr);
}

inline void RPM(uintptr_t address, void* outBuffer, size_t size) {
    if (address < 0x100000 || !outBuffer) return;
    ReadProcessMemory(hGame, reinterpret_cast<LPCVOID>(address), outBuffer, size, nullptr);
}

inline bool ReadMatrix(uintptr_t address, float* outMatrix) {
    if (address < 0x100000 || !outMatrix) return false;
    SIZE_T bytesRead = 0;
    return ReadProcessMemory(hGame, reinterpret_cast<LPCVOID>(address), outMatrix, sizeof(float) * 16, &bytesRead)
        && bytesRead == (sizeof(float) * 16);
}

#endif // MEMORY_H
