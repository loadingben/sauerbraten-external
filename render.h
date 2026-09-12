#pragma once
#include <Windows.h>
#include <d3d9.h>
#include "imgui/imgui.h"
#include "player.h"

bool CreateOverlayWindow();
bool CreateDevice();
ImGuiIO& InitOverlay();
void Cleanup();

void BeginImGuiFrame(ImGuiIO& io);
void EndImGuiFrame();
void DrawMenu(const Player& localPlayer);

void UpdateRenderDimensions();
bool IsGameMinimized();
void HandleWindowMessages(MSG& msg);
bool HandleDeviceResetIfNeeded(int& lastWidth, int& lastHeight);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
