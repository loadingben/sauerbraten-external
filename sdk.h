#pragma once
#include "globals.h"

// Dreidimensionale Struktur für Positionen (ESP/Aimbot)
struct Vector3 {
    float x, y, z;
};

// Makro für automatisiertes, fehlerfreies Speicher-Padding
#define STR_MERGE_IMPL(a, b) a##b
#define STR_MERGE(a, b) STR_MERGE_IMPL(a, b)
#define MAKE_PAD(size) char STR_MERGE(pad_, __COUNTER__)[size]

#pragma pack(push, 1)
class playerent {
public:
    // Offset 0x00
    Vector3 pos;                        // X (0x00), Y (0x04), Z (0x08)

    // Offset 0x0C bis 0x34 auffüllen
    MAKE_PAD(offsets::player::yaw - 0x0C);

    // Offset 0x34: Blickwinkel (Wichtig für Aimbot!)
    float yaw;                          // 0x34
    float pitch;                        // 0x38

    // Offset 0x3C bis 0x178 auffüllen
    MAKE_PAD(offsets::player::health - 0x3C);

    // Offset 0x178: Vitalwerte
    int health;                         // 0x178
    int maxHealth;                      // 0x17C
    int armor;                          // 0x180

    // Offset 0x184 bis 0x274 auffüllen
    MAKE_PAD(offsets::player::name - 0x184);

    // Offset 0x274: Text-Arrays
    char name[16];                      // 0x274 (Platz für den Spielernamen)

    // Offset 0x284 bis 0x378 auffüllen (0x378 - 0x284 = 0xF4)
    MAKE_PAD(offsets::player::team - (offsets::player::name + 16));

    // Offset 0x378: Teamname
    char team[16];                      // 0x378 (Platz für das Team, z.B. "evil")
};
#pragma pack(pop)
