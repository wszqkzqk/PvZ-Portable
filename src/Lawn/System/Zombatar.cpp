/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * This file is part of PvZ-Portable.
 *
 * PvZ-Portable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PvZ-Portable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PvZ-Portable. If not, see <https://www.gnu.org/licenses/>.
 */

#include "Zombatar.h"

constexpr ZombatarPartLayout gClothesLayout[12] =
{
	{88, 110, 0, 0, 0}, {75, 100, 0, 0, 0}, {85, 112, 0, 0, 0}, {76, 110, 0, 0, 0},
	{89, 115, 0, 0, 0}, {93, 110, 0, 0, 0}, {78, 105, 0, 0, 0}, {88, 110, 0, 0, 0},
	{88, 102, 0, 0, 0}, {85, 110, 0, 0, 0}, {85, 110, 0, 0, 0}, {79, 112, 0, 0, 0}
};

constexpr ZombatarPartLayout gTidbitsLayout[14] =
{
	{28, 63, 18, 48, 2}, {28, 63, 0, 0, 2}, {46, 111, 0, 0, 2}, {31, 62, 0, 0, 2},
	{31, 58, 0, 0, 2}, {28, 66, 0, 0, 2}, {28, 72, 0, 0, 2}, {33, 55, 0, 0, 2},
	{21, 76, 0, 0, 2}, {36, 71, 0, 0, 2}, {36, 70, 0, 0, 2}, {86, 91, 0, 6, 2},
	{88, 50, 0, 8, 2}, {113, 115, 0, 0, 2}
};

constexpr ZombatarPartLayout gAccessoryLayout[15] =
{
	{103, 110, 0, 0, 4}, {108, 110, 0, 0, 4}, {86, 113, 0, 0, 4}, {131, 95, 0, 0, 4},
	{131, 100, 0, 0, 4}, {131, 100, 0, 0, 4}, {104, 111, 0, 0, 4}, {118, 65, 0, 0, 4},
	{61, 118, 0, 0, 4}, {43, 100, 0, 0, 4}, {135, 92, 0, 0, 4}, {78, 130, 0, 0, 4},
	{68, 145, 0, 0, 4}, {133, 70, 0, 0, 4}, {13, 40, 0, 0, 10}
};

constexpr ZombatarPartLayout gFacialHairLayout[24] =
{
	{35, 107, 1, 0, 6}, {51, 110, 0, 0, 6}, {45, 110, 0, 0, 6}, {38, 105, 3, 2, 6},
	{69, 145, 0, 0, 6}, {48, 112, 0, 0, 6}, {13, 107, 0, 0, 6}, {45, 105, 1, 1, 6},
	{41, 105, 1, 1, 6}, {44, 112, 1, 2, 6}, {43, 88, 1, 4, 6}, {28, 105, 8, 1, 6},
	{45, 110, 0, 0, 6}, {18, 103, 1, 1, 6}, {63, 145, 2, 1, 6}, {63, 140, 1, 1, 6},
	{43, 110, 0, 0, 6}, {58, 96, 1, 3, 6}, {46, 92, 0, 0, 6}, {114, 80, 0, 0, 6},
	{118, 83, 1, 1, 6}, {13, 87, 3, 4, 6}, {58, 145, 1, 1, 6}, {38, 108, 4, 2, 6}
};

constexpr ZombatarPartLayout gHairLayout[16] =
{
	{23, 0, 8, 1, 8}, {23, 25, 2, 3, 8}, {23, 30, 0, 0, 8}, {30, 15, 0, 0, 8},
	{36, 37, 0, 0, 8}, {39, 13, 0, 0, 8}, {51, 22, 0, 0, 8}, {28, 15, 0, 0, 8},
	{128, 55, 0, 0, 8}, {22, 32, 0, 0, 8}, {25, 19, 2, 2, 8}, {51, -5, 2, 2, 8},
	{33, 13, 2, 2, 8}, {9, -2, 1, 5, 8}, {45, 4, 0, -1, 8}, {26, 20, 0, 0, 8}
};

constexpr ZombatarPartLayout gEyewearLayout[16] =
{
	{28, 73, 0, 0, 10}, {31, 85, 0, -1, 10}, {28, 69, 0, 1, 10}, {28, 78, 0, -1, 10},
	{30, 75, -1, -1, 10}, {30, 78, -1, -1, 10}, {50, 90, -1, -1, 10}, {32, 70, -1, -1, 10},
	{36, 100, -1, -1, 10}, {31, 75, -1, -1, 10}, {31, 67, -1, -1, 10}, {38, 95, -1, -1, 10},
	{30, 81, 0, 0, 10}, {35, 64, 0, 0, 10}, {42, 65, 0, 0, 10}, {35, 65, 0, 0, 10}
};

constexpr ZombatarPartLayout gHatsLayout[14] =
{
	{28, 5, 2, 1, 12}, {47, 12, 0, 0, 12}, {36, 20, 15, -1, 12}, {11, 10, 0, 0, 12},
	{41, 16, 0, 0, 12}, {18, 3, 4, -2, 12}, {53, 17, 0, 15, 12}, {3, 0, 0, -2, 12},
	{38, 0, -1, -2, 12}, {13, 45, 0, 0, 12}, {63, 8, 1, 14, 12}, {43, 15, 0, 0, 12},
	{18, 0, 0, 0, 12}, {23, 5, 0, 0, 12}
};

const ZombatarPartLayout* GetPartLayout(ZombatarPage thePage, int theIndex)
{
	switch (thePage)
	{
	case ZOMBATAR_PAGE_CLOTHES: return theIndex < 12 ? &gClothesLayout[theIndex] : nullptr;
	case ZOMBATAR_PAGE_TIDBITS: return theIndex < 14 ? &gTidbitsLayout[theIndex] : nullptr;
	case ZOMBATAR_PAGE_ACCESSORY: return theIndex < 15 ? &gAccessoryLayout[theIndex] : nullptr;
	case ZOMBATAR_PAGE_FACIAL_HAIR:
	{
		int aIdx = theIndex;
		if (aIdx > 16)
			aIdx -= aIdx / 17;
		return aIdx < 24 ? &gFacialHairLayout[aIdx] : nullptr;
	}
	case ZOMBATAR_PAGE_HAIR: return theIndex < 16 ? &gHairLayout[theIndex] : nullptr;
	case ZOMBATAR_PAGE_EYEWEAR: return theIndex < 16 ? &gEyewearLayout[theIndex] : nullptr;
	case ZOMBATAR_PAGE_HATS: return theIndex < 14 ? &gHatsLayout[theIndex] : nullptr;
	default: return nullptr;
	}
}
