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

constexpr int ZombatarClampColor(int theColor)
{
	if (theColor < 0)
		return -1;
	return std::min(theColor, 47);
}

constexpr Sexy::Color gZombatarColors[] =
{
	Sexy::Color(134, 147, 122), Sexy::Color(79, 135, 94), Sexy::Color(127, 135, 94), Sexy::Color(120, 130, 50),
	Sexy::Color(156, 163, 105), Sexy::Color(96, 151, 11), Sexy::Color(147, 184, 77), Sexy::Color(82, 143, 54),
	Sexy::Color(121, 168, 99), Sexy::Color(65, 156, 74), Sexy::Color(107, 178, 114), Sexy::Color(104, 121, 90),
	Sexy::Color(151, 33, 33), Sexy::Color(199, 53, 53), Sexy::Color(220, 112, 47), Sexy::Color(251, 251, 172),
	Sexy::Color(240, 210, 87), Sexy::Color(165, 126, 65), Sexy::Color(106, 72, 32), Sexy::Color(72, 35, 5),
	Sexy::Color(50, 56, 61), Sexy::Color(0, 0, 10), Sexy::Color(197, 239, 239), Sexy::Color(63, 109, 242),
	Sexy::Color(13, 202, 151), Sexy::Color(158, 183, 19), Sexy::Color(30, 210, 64), Sexy::Color(225, 65, 230),
	Sexy::Color(128, 47, 204), Sexy::Color(255, 255, 255), Sexy::Color(238, 19, 24), Sexy::Color(247, 89, 215),
	Sexy::Color(239, 198, 253), Sexy::Color(160, 56, 241), Sexy::Color(86, 74, 241), Sexy::Color(74, 160, 241),
	Sexy::Color(199, 244, 251), Sexy::Color(49, 238, 237), Sexy::Color(16, 194, 66), Sexy::Color(112, 192, 33),
	Sexy::Color(16, 145, 52), Sexy::Color(248, 247, 41), Sexy::Color(227, 180, 20), Sexy::Color(241, 115, 25),
	Sexy::Color(248, 247, 175), Sexy::Color(103, 85, 54), Sexy::Color(159, 17, 20), Sexy::Color(255, 255, 255)
};

constexpr Sexy::Color ZombatarGetColor(int theIndex)
{
	if (theIndex < 0)
		return Sexy::Color(255, 255, 255);
	return gZombatarColors[std::min(theIndex, 47)];
}

constexpr int ZombatarRemapAccessoryForRuntime(int theIndex)
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
