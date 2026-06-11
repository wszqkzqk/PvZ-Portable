/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef __ZOMBATAR_H__
#define __ZOMBATAR_H__

#include "../../SexyAppFramework/Common.h"
#include "../../SexyAppFramework/graphics/Color.h"

#include <algorithm>
#include <climits>
#include <cstdint>
#include <cstring>

enum ZombatarRecordSlot
{
    ZOMBATAR_SLOT_SKIN_COLOR,
    ZOMBATAR_SLOT_CLOTHES,
    ZOMBATAR_SLOT_CLOTHES_COLOR,
    ZOMBATAR_SLOT_TIDBITS,
    ZOMBATAR_SLOT_TIDBITS_COLOR,
    ZOMBATAR_SLOT_ACCESSORY,
    ZOMBATAR_SLOT_ACCESSORY_COLOR,
    ZOMBATAR_SLOT_FACIAL_HAIR,
    ZOMBATAR_SLOT_FACIAL_HAIR_COLOR,
    ZOMBATAR_SLOT_HAIR,
    ZOMBATAR_SLOT_HAIR_COLOR,
    ZOMBATAR_SLOT_EYEWEAR,
    ZOMBATAR_SLOT_EYEWEAR_COLOR,
    ZOMBATAR_SLOT_HATS,
    ZOMBATAR_SLOT_HATS_COLOR,
    ZOMBATAR_SLOT_BACKGROUND,
    ZOMBATAR_SLOT_BACKGROUND_COLOR,
    ZOMBATAR_SLOT_RESERVED
};

inline uint32_t ZombatarReadRecordSlot(const unsigned char* theRecord, int theSlot)
{
    uint32_t aValue = 0;
    memcpy(&aValue, theRecord + theSlot * 4, sizeof(aValue));
    return Sexy::FromLE32(aValue);
}

inline int ZombatarReadSignedRecordSlot(const unsigned char* theRecord, int theSlot)
{
    uint32_t aValue = ZombatarReadRecordSlot(theRecord, theSlot);
    if (aValue == 0xFFFFFFFFU || aValue > static_cast<uint32_t>(INT32_MAX))
        return -1;
    return static_cast<int>(aValue);
}

inline void ZombatarWriteRecordSlot(unsigned char* theRecord, int theSlot, int theValue)
{
    uint32_t aValue = Sexy::ToLE32(static_cast<uint32_t>(theValue));
    memcpy(theRecord + theSlot * 4, &aValue, sizeof(aValue));
}

inline int ZombatarClampColor(int theColor)
{
    return std::max(0, std::min(theColor, 17));
}

inline Sexy::Color ZombatarGetColor(int theIndex)
{
    static const Sexy::Color aColors[] =
    {
        Sexy::Color(89, 187, 64), Sexy::Color(135, 207, 71), Sexy::Color(81, 155, 74), Sexy::Color(193, 226, 102),
        Sexy::Color(117, 197, 183), Sexy::Color(115, 169, 214), Sexy::Color(173, 122, 196), Sexy::Color(221, 126, 185),
        Sexy::Color(230, 112, 102), Sexy::Color(239, 162, 75), Sexy::Color(240, 218, 80), Sexy::Color(143, 105, 69),
        Sexy::Color(82, 60, 42), Sexy::Color(32, 32, 32), Sexy::Color(112, 112, 112), Sexy::Color(190, 190, 190),
        Sexy::Color(245, 232, 205), Sexy::Color(255, 255, 255)
    };

    return aColors[ZombatarClampColor(theIndex)];
}

inline int ZombatarRemapAccessoryForRuntime(int theIndex)
{
    switch (theIndex)
    {
    case 5: return 14;
    case 6: return 5;
    case 7: return 6;
    case 8: return 12;
    case 9: return 7;
    case 10: return 9;
    case 11: return 10;
    case 12: return 11;
    case 14: return 8;
    default: return theIndex;
    }
}

#endif
