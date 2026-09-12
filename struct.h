#pragma once
#include <basetsd.h>
#include "vec.h"

struct WeaponEntry {
    DWORD32 vftable;            // 0x00
    int id;                     // 0x04
    struct PlayerStruct* owner; // 0x08
};

struct PlayerStruct {
    // Offset 0x000: Bewegungskoordinaten (Füße)
    Vec3 positionFeet;          // X (0x00), Y (0x04), Z (0x08)

    // Offset 0x00C bis 0x034 auffüllen
    char pad1[0x034 - 0x00C];   // 0x00C → 0x034

    // Offset 0x034: Blickwinkel
    float yaw;                  // 0x034
    float pitch;                // 0x038

    // Offset 0x03C bis 0x078 auffüllen
    char pad_to_state[0x078 - 0x03C]; // 0x03C → 0x078

    // Offset 0x078: Lebens-Zustand (0 = Lebend, 1 = Tot)
    int state;                  // 0x078

    // Offset 0x07C bis 0x178 auffüllen
    char pad2[0x178 - 0x07C];   // 0x07C → 0x178

    // Offset 0x178: Vitalwerte
    int health;                 // 0x178
    int maxHealth;              // 0x17C
    int armor;                  // 0x180

    // Offset 0x184 bis 0x274 auffüllen
    char pad3[0x274 - 0x184];   // 0x184 → 0x274

    // Offset 0x274: Spielername (FIXED: Größe [16] wieder da!)
    char name[16];              // 0x274 (Größe 16 Bytes)

    // Offset 0x284 bis 0x378 auffüllen (0x378 - 0x284 = 0x0F4)
    char pad4[0x378 - 0x284];   // 0x284 → 0x378

    // Offset 0x378: Live-Team (FIXED: Größe [16] wieder da!)
    char team[16];              // 0x378

    // Offset 0x388 bis 0x3C0 auffüllen für Waffen-Pointer Dummy
    char pad_to_weapon[0x3C0 - 0x388];
    WeaponEntry* currentWeapon; // 0x3C0
};
