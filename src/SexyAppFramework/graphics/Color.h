/*
 * Portions of this file are based on the PopCap Games Framework
 * Copyright (C) 2005-2009 PopCap Games, Inc.
 * 
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
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

#ifndef __COLOR_H__
#define __COLOR_H__

#include "Common.h"

namespace Sexy
{

class Color
{
public:
	int32_t mRed;
	int32_t mGreen;
	int32_t mBlue;
	int32_t mAlpha;

	static Color Black;
	static Color White;

public:
	constexpr Color() : mRed(0), mGreen(0), mBlue(0), mAlpha(255) {}
	constexpr Color(int32_t theColor) :
		mRed((theColor >> 16) & 0xFF),
		mGreen((theColor >> 8) & 0xFF),
		mBlue(theColor & 0xFF),
		mAlpha((theColor >> 24) & 0xFF)
	{
		if (mAlpha == 0)
			mAlpha = 0xff;
	}
	constexpr Color(int32_t theColor, int32_t theAlpha) :
		mRed((theColor >> 16) & 0xFF),
		mGreen((theColor >> 8) & 0xFF),
		mBlue(theColor & 0xFF),
		mAlpha(theAlpha) {}
	constexpr Color(int32_t theRed, int32_t theGreen, int32_t theBlue) :
		mRed(theRed), mGreen(theGreen), mBlue(theBlue), mAlpha(0xFF) {}
	constexpr Color(int32_t theRed, int32_t theGreen, int32_t theBlue, int32_t theAlpha) :
		mRed(theRed), mGreen(theGreen), mBlue(theBlue), mAlpha(theAlpha) {}
	constexpr Color(const uchar* theElements) :
		mRed(theElements[0]), mGreen(theElements[1]), mBlue(theElements[2]), mAlpha(0xFF) {}
	constexpr Color(const int32_t* theElements) :
		mRed(theElements[0]), mGreen(theElements[1]), mBlue(theElements[2]), mAlpha(0xFF) {}

	constexpr int32_t			GetRed() const { return mRed; }
	constexpr int32_t			GetGreen() const { return mGreen; }
	constexpr int32_t			GetBlue() const { return mBlue; }
	constexpr int32_t			GetAlpha() const { return mAlpha; }
	constexpr uint32_t			ToInt() const
	{
		return
			(static_cast<uint32_t>(mAlpha) << 24) |
			(static_cast<uint32_t>(mRed) << 16) |
			(static_cast<uint32_t>(mGreen) << 8) |
			(static_cast<uint32_t>(mBlue));
	}
	constexpr uint32_t			ToGLColor() const
	{
		uint32_t aGLColor = (static_cast<uint32_t>(mAlpha) << 24) |
							(static_cast<uint32_t>(mBlue) << 16) |
							(static_cast<uint32_t>(mGreen) << 8) |
							(static_cast<uint32_t>(mRed));
		return ToLE32(aGLColor);
	}

	int32_t&					operator[](int32_t theIdx);
	constexpr int32_t			operator[](int32_t theIdx) const
	{
		switch (theIdx)
		{
		case 0: return mRed;
		case 1: return mGreen;
		case 2: return mBlue;
		case 3: return mAlpha;
		default: return 0;
		}
	}

	constexpr bool				operator==(const Color& theColor) const = default;
};
}

#endif //__COLOR_H__
