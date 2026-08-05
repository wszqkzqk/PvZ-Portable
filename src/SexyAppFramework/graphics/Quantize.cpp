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

#include "Quantize.h"
#include <assert.h>
#include <math.h>

using namespace Sexy;

bool Sexy::Quantize8Bit(const uint32_t* theSrcBits, int theWidth, int theHeight, uchar* theDestColorIndices, uint32_t* theDestColorTable)
{
	int aSize = theWidth*theHeight;

	int aColorTableSize = 0;

	uint32_t aSearchTable[256];
	uchar aTranslationTable[256]; // From search table to color table

	if (aSize > 0)
	{
		aSearchTable[0] = theSrcBits[0];
		theDestColorTable[0] = theSrcBits[0];
		aTranslationTable[0] = 0;
		theDestColorIndices[0] = 0;
		aColorTableSize++;
	}

	for (int anIdx = 1; anIdx < aSize; anIdx++)
	{
		uint32_t aColor = theSrcBits[anIdx];

		int aLeftPos = 0;
		int aRightPos = aColorTableSize-1;
		int aMiddlePos = (aLeftPos+aRightPos)/2;

		for (;;)
		{
			uint32_t aCheckColor = aSearchTable[aMiddlePos];

			if (aColor < aCheckColor)
				aRightPos = aMiddlePos - 1;
			else if (aColor > aCheckColor)
				aLeftPos = aMiddlePos + 1;
			else
			{
				theDestColorIndices[anIdx] = aTranslationTable[aMiddlePos];
				break;
			}

			if (aLeftPos > aRightPos)
			{
				if (aColorTableSize >= 256)
					return false;

				int anInsertPos = aLeftPos;
				if ((anInsertPos < aColorTableSize) && (aColor > aSearchTable[anInsertPos]))
					anInsertPos++;

				// Insert color into the table
				memmove(aSearchTable+anInsertPos+1, aSearchTable+anInsertPos, (aColorTableSize-anInsertPos) * sizeof(uint32_t));
				aSearchTable[anInsertPos] = aColor;

				memmove(aTranslationTable+anInsertPos+1, aTranslationTable+anInsertPos, (aColorTableSize-anInsertPos) * sizeof(uchar));
				aTranslationTable[anInsertPos] = aColorTableSize;

				theDestColorTable[aColorTableSize] = aColor;

				theDestColorIndices[anIdx] = aColorTableSize;

				aColorTableSize++;

				break;
			}

			aMiddlePos = (aLeftPos+aRightPos)/2;
		}
	}

	return true;
}
