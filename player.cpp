#include "player.h"
#include "memory.h"
#include <cmath>
#include <algorithm>

extern uintptr_t gameBaseAddress;

Player::Player(uintptr_t playerPtr) {
    if (playerPtr == 0) return;
    address = playerPtr;
}

Player::Player(uintptr_t offset, bool isOffset) {
    if (!isOffset) return;
    uintptr_t playerPtr = RPM<uintptr_t>(offset);
    if (playerPtr == 0) return;
    address = playerPtr;
}

Player::Player(Player&& other) noexcept {
    address = other.address;
    screenHead = other.screenHead;
    screenFeet = other.screenFeet;
    isEnemy = other.isEnemy;
    isVisible = other.isVisible;
    distance = other.distance;
    boxHeight = other.boxHeight;
    boxWidth = other.boxWidth;
    distanceToAim = other.distanceToAim;
}

Player& Player::operator=(Player&& other) noexcept {
    if (this != &other) {
        address = other.address;
        screenHead = other.screenHead;
        screenFeet = other.screenFeet;
        isEnemy = other.isEnemy;
        isVisible = other.isVisible;
        distance = other.distance;
        boxHeight = other.boxHeight;
        boxWidth = other.boxWidth;
        distanceToAim = other.distanceToAim;
    }
    return *this;
}

bool Player::IsValid() const { return address >= 0x100000; }

bool Player::IsAlive() const {
    if (!IsValid()) return false;
    int health = RPM<int>(address + offsets::player::health);
    return health > 0 && health <= 20000;
}

int Player::GetTeam() const {
    if (!IsValid()) return -1;
    char teamBuffer[16] = { 0 };
    RPM(address + offsets::player::team, teamBuffer, 16);
    return (strcmp(teamBuffer, "evil") == 0) ? 1 : 0;
}

bool Player::IsEnemy(const Player& localPlayer) const {
    if (!IsValid() || !localPlayer.IsValid()) return false;
    char myTeam[16] = { 0 };
    char localTeam[16] = { 0 };
    RPM(address + offsets::player::team, myTeam, 16);
    RPM(localPlayer.address + offsets::player::team, localTeam, 16);
    return (strcmp(myTeam, localTeam) != 0);
}

bool Player::IsVisible(int currentFrame) const { return true; }
int Player::GetHealth() const { return IsValid() ? RPM<int>(address + offsets::player::health) : 0; }
int Player::GetArmor() const { return IsValid() ? RPM<int>(address + offsets::player::armor) : 0; }

const char* Player::GetName() const {
    if (!IsValid()) return "";
    static char nameBuffer[16];
    RPM(address + offsets::player::name, nameBuffer, 16);
    return nameBuffer;
}

float Player::GetYaw() const { return IsValid() ? RPM<float>(address + offsets::player::yaw) : 0.f; }
float Player::GetPitch() const { return IsValid() ? RPM<float>(address + offsets::player::pitch) : 0.f; }

Vec3 Player::FeetPos() const {
    if (!IsValid()) return Vec3{ 0,0,0 };
    Vec3 basePos = RPM<Vec3>(address + offsets::player::position);
    return Vec3{ basePos.x, basePos.y, basePos.z - 14.0f };
}

Vec3 Player::HeadPos() const {
    if (!IsValid()) return Vec3{ 0,0,0 };
    Vec3 basePos = RPM<Vec3>(address + offsets::player::position);
    return Vec3{ basePos.x, basePos.y, basePos.z + 4.5f };
}

float Player::DistanceTo(const Player& other) const { return this->HeadPos().Distance(other.HeadPos()); }
WeaponEntry* Player::GetWeapon() const { return nullptr; }
void Player::SetMag(int value) const { if (address) WPM<int>(address + 0x11C, value); }

void Player::SetAmmo(int value) const {
    if (!address) return;
    for (uintptr_t offset = offsets::player::ammo_start; offset <= 0x1AC; offset += 0x4) {
        WPM<int>(address + offset, value);
    }
}

void Player::SetHealth(int value) const { if (address) WPM<int>(address + offsets::player::health, value); }
void Player::SetShield(int value) const { if (address) WPM<int>(address + offsets::player::armor, value); }

void Player::ComputeBoxDimensions() {
    boxHeight = fabsf(screenFeet.y - screenHead.y);
    boxWidth = boxHeight * 0.60f;
}

ImVec2 Player::GetBoxTopLeft() const {
    float topY = (screenHead.y < screenFeet.y) ? screenHead.y : screenFeet.y;
    return ImVec2(screenHead.x - (boxWidth / 2.0f), topY);
}

ImVec2 Player::GetBoxBottomRight() const {
    float bottomY = (screenHead.y > screenFeet.y) ? screenHead.y : screenFeet.y;
    return ImVec2(screenHead.x + boxWidth / 2.0f, bottomY);
}

void Player::DrawBox(ImDrawList* drawList, float rounding, float thickness) const {
    drawList->AddRect(GetBoxTopLeft(), GetBoxBottomRight(), GetBoxColor(), rounding, 0, thickness);
}

ImU32 Player::GetBoxColor() const {
    if (isEnemy) {
        return ImGui::ColorConvertFloat4ToU32(ImVec4(settings.colorEnemy[0], settings.colorEnemy[1], settings.colorEnemy[2], settings.colorEnemy[3]));
    }
    else {
        return ImGui::ColorConvertFloat4ToU32(ImVec4(settings.colorFriendly[0], settings.colorFriendly[1], settings.colorFriendly[2], settings.colorFriendly[3]));
    }
}

void Player::DrawHealthBar(ImDrawList* drawList) const {
    float currentBoxHeight = fabsf(screenFeet.y - screenHead.y);
    float healthPct = std::clamp(GetHealth() / 100.0f, 0.0f, 1.0f);
    float healthHeight = currentBoxHeight * healthPct;
    ImVec2 topLeft = GetBoxTopLeft();
    ImVec2 bottomRight = GetBoxBottomRight();
    ImVec2 barStart = { topLeft.x - 6, bottomRight.y - healthHeight };
    ImVec2 barEnd = { topLeft.x - 2, bottomRight.y };
    drawList->AddRectFilled(barStart, barEnd, IM_COL32(0, 255, 0, 255));
    drawList->AddRect(ImVec2(topLeft.x - 6, topLeft.y), ImVec2(topLeft.x - 2, bottomRight.y), IM_COL32(0, 0, 0, 200));
}

void Player::DrawTextAboveBox(ImDrawList* drawList, const std::string& text, float yOffset, ImU32 color) const {
    ImVec2 topLeft = GetBoxTopLeft();
    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
    ImVec2 pos = ImVec2(screenHead.x - textSize.x / 2.0f, topLeft.y + yOffset);
    drawList->AddText(pos, color, text.c_str());
}

void Player::DrawName(ImDrawList* drawList) const { DrawTextAboveBox(drawList, GetName(), -18.0f); }
void Player::DrawDistance(ImDrawList* drawList, ImU32 color) const {
    char buf[32]; snprintf(buf, sizeof(buf), "[%.1fm]", distance);
    DrawTextAboveBox(drawList, buf, -6.0f, color);
}

void Player::DrawNameAndDistance(ImDrawList* drawList) const {
    char buf[64]; snprintf(buf, sizeof(buf), "%s [%.1fm]", GetName(), distance / 10.f);
    DrawTextAboveBox(drawList, buf, -18.0f, IM_COL32(255, 255, 255, 255));
}

void Player::DrawSnapline(ImDrawList* drawList) const {
    if (isEnemy) {
        ImU32 snapColor = ImGui::ColorConvertFloat4ToU32(ImVec4(settings.colorSnapline[0], settings.colorSnapline[1], settings.colorSnapline[2], settings.colorSnapline[3]));
        drawList->AddLine(ImVec2(screenHead.x, screenFeet.y), ImVec2((float)screenWidth / 2.0f, (float)screenHeight), snapColor, 1.2f);
    }
}
