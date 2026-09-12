#include "cheats.h"
#include "globals.h"
#include "memory.h"
#include "vec.h"
#include <algorithm>
#include <cmath>

extern uintptr_t gameBaseAddress;

bool WorldToScreen(const Vec3& world, Vec2& screen, float* m, int width, int height) {
    float clipX = world.x * m[0] + world.y * m[4] + world.z * m[8] + m[12];
    float clipY = world.x * m[1] + world.y * m[5] + world.z * m[9] + m[13];
    float clipZ = world.x * m[2] + world.y * m[6] + world.z * m[10] + m[14];
    float clipW = world.x * m[3] + world.y * m[7] + world.z * m[11] + m[15];

    if (clipW < 0.1f) return false;

    float ndcX = clipX / clipW;
    float ndcY = clipY / clipW;

    screen.x = ((float)width / 2.0f) + (ndcX * (float)width / 2.0f);
    screen.y = ((float)height / 2.0f) - (ndcY * (float)height / 2.0f);

    return true;
}

Player GetLocalPlayer() {
    uintptr_t localPlayerPtr = RPM<uintptr_t>(gameBaseAddress + offsets::local_player);
    Player localPlayer(localPlayerPtr);

    if (!localPlayer.IsValid()) return Player();

    if (settings.infiniteAmmo) {
        localPlayer.SetAmmo(999);
    }
    if (settings.godmode) {
        localPlayer.SetHealth(9999);
        localPlayer.SetShield(9999);
    }
    return localPlayer;
}

std::vector<Player> BuildPlayerList(const Player& localPlayer) {
    std::vector<Player> players;
    float viewMatrix[16] = { 0 };
    if (!ReadMatrix(gameBaseAddress + offsets::viewmatrix, viewMatrix)) return players;

    int outTotalPlayers = RPM<int>(gameBaseAddress + offsets::player_count);
    uintptr_t playerListPtr = RPM<uintptr_t>(gameBaseAddress + offsets::entity_list);

    if (playerListPtr == 0 || outTotalPlayers <= 1 || outTotalPlayers > 64)
        return players;

    for (int i = 1; i < outTotalPlayers; ++i) {
        uintptr_t entryAddress = playerListPtr + (i * 0x8);
        uintptr_t playerPtr = RPM<uintptr_t>(entryAddress);
        if (!playerPtr) continue;

        Player p(playerPtr);
        if (!p.IsValid() || !p.IsAlive()) continue;

        if (!WorldToScreen(p.HeadPos(), p.screenHead, viewMatrix, screenWidth, screenHeight)) continue;
        if (!WorldToScreen(p.FeetPos(), p.screenFeet, viewMatrix, screenWidth, screenHeight)) continue;

        p.isEnemy = p.IsEnemy(localPlayer);
        p.isVisible = true;
        p.distance = p.DistanceTo(localPlayer);
        p.ComputeBoxDimensions();

        p.distanceToAim = Vec2{ (float)screenWidth / 2, (float)screenHeight / 2 }.DistanceTo(p.screenHead);
        players.push_back(std::move(p));
    }
    return players;
}

void DrawFOV() {
    if (!ImGui::GetCurrentContext()) return;
    auto* drawList = ImGui::GetBackgroundDrawList();
    if (!drawList) return;

    if (settings.showFovCircle) {
        ImVec2 center = ImVec2((float)screenWidth / 2.f, (float)screenHeight / 2.f);
        ImU32 fovColor = ImGui::ColorConvertFloat4ToU32(ImVec4(settings.colorFov[0], settings.colorFov[1], settings.colorFov[2], settings.colorFov[3]));
        drawList->AddCircle(center, settings.fovSize, fovColor, 64, 2.0f);
    }
}

void DrawESP(const std::vector<Player>& players) {
    if (!ImGui::GetCurrentContext()) return;
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    if (!drawList) return;

    for (const auto& player : players) {
        if (settings.showEnemies && player.isEnemy) {
            player.DrawBox(drawList, 2.0f);
            player.DrawHealthBar(drawList);
            player.DrawNameAndDistance(drawList);
            if (settings.showSnaplines) player.DrawSnapline(drawList);
        }
        if (settings.showFriendlies && !player.isEnemy) {
            player.DrawBox(drawList, 2.0f);
            player.DrawHealthBar(drawList);
            player.DrawNameAndDistance(drawList);
            if (settings.showSnaplines) player.DrawSnapline(drawList);
        }
    }
}

void Aimbot(const std::vector<Player>& players, const Player& localPlayer) {
    if (!settings.enableAimbot) return;
    if (!(GetAsyncKeyState(VK_RBUTTON) & 0x8000)) return;

    const Player* closestPlayer = nullptr;
    for (const auto& player : players) {
        if (!player.isEnemy || !player.IsAlive()) continue;
        if (player.distanceToAim > settings.fovSize) continue;
        if (!closestPlayer || player.distanceToAim < closestPlayer->distanceToAim)
            closestPlayer = &player;
    }

    if (closestPlayer && localPlayer.address) {
        Vec3 localHead = localPlayer.HeadPos();
        Vec3 targetHead = closestPlayer->HeadPos();

        Vec2 targetAngles = CalcAimAngles(localHead, targetHead);
        targetAngles.y += 180.0f;

        if (targetAngles.y < 0.0f) targetAngles.y += 360.0f;
        if (targetAngles.y > 360.0f) targetAngles.y = fmodf(targetAngles.y, 360.0f);

        if (settings.enableSilentAim) {
            if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
                float originalYaw = localPlayer.GetYaw();
                float originalPitch = localPlayer.GetPitch();

                WPM<float>(localPlayer.address + offsets::player::yaw, targetAngles.y);
                WPM<float>(localPlayer.address + offsets::player::pitch, targetAngles.x);

                if (settings.silentAimDelay > 0) Sleep(settings.silentAimDelay);

                WPM<float>(localPlayer.address + offsets::player::yaw, originalYaw);
                WPM<float>(localPlayer.address + offsets::player::pitch, originalPitch);
            }
        }
        else {
            if (settings.enableSmoothing && settings.smoothFactor > 0.1f) {
                float currentYaw = localPlayer.GetYaw();
                float currentPitch = localPlayer.GetPitch();

                float yawDiff = targetAngles.y - currentYaw;
                if (yawDiff > 180.0f) yawDiff -= 360.0f;
                if (yawDiff < -180.0f) yawDiff += 360.0f;

                float pitchDiff = targetAngles.x - currentPitch;
                targetAngles.y = currentYaw + (yawDiff / settings.smoothFactor);
                targetAngles.x = currentPitch + (pitchDiff / settings.smoothFactor);

                if (targetAngles.y < 0.0f) targetAngles.y += 360.0f;
                if (targetAngles.y > 360.0f) targetAngles.y = fmodf(targetAngles.y, 360.0f);
            }
            WPM<float>(localPlayer.address + offsets::player::yaw, targetAngles.y);
            WPM<float>(localPlayer.address + offsets::player::pitch, targetAngles.x);
        }
    }
}
