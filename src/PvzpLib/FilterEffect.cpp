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

#include "PvzpDebug.h"
#include "PvzpCommon.h"
#include "FilterEffect.h"
#include "graphics/Graphics.h"
#include "graphics/MemoryImage.h"
#include <algorithm>

void RGB_to_HSL(float r, float g, float b, float& h, float& s, float& l)
{
	float maxval = std::max(r, g);
	maxval = std::max(maxval, b);
	float minval = std::min(r, g);
	minval = std::min(minval, b);

	l = (minval + maxval) / 2;  //luminosity
	h = 0.0f;
	s = 0.0f;
	if (l <= 0.0f)
		return;

	float delta = maxval - minval;
	s = delta;
	if (s <= 0.0f)
		return;
	s /= ((l <= 0.5f) ? (minval + maxval) : (2.0f - minval - maxval));  //saturation

	float r2 = (maxval - r) / delta;
	float g2 = (maxval - g) / delta;
	float b2 = (maxval - b) / delta;
	if (maxval == r)
		h = ((g == minval) ? (5.0f + b2) : (1.0f - g2));
	else if (maxval == g)
		h = ((b == minval) ? (1.0f + r2) : (3.0f - b2));
	else
		h = ((r == minval) ? (3.0f + g2) : (5.0f - r2));
	h /= 6.0f;  //hue
}

void HSL_to_RGB(float h, float sl, float l, float& r, float& g, float& b)
{
	float v = (l <= 0.5f) ? (l * (1.0f + sl)) : (l + sl - l * sl);
	if (v <= 0.0f)
	{
		r = 0.0f;
		g = 0.0f;
		b = 0.0f;
		return;
	}

	float y = 2 * l - v;
	float sv = (v - y) / v;
	h *= 6.0f;
	int sextant = std::clamp(static_cast<int>(h), 0, 5);
	float vsf = v * sv * (h - sextant);
	float x = y + vsf;
	float z = v - vsf;

	switch (sextant)
	{
	case 0:	r = v;	g = x;	b = y;	break;
	case 1:	r = z;	g = v;	b = y;	break;
	case 2:	r = y;	g = v;	b = x;	break;
	case 3:	r = y;	g = z;	b = v;	break;
	case 4:	r = x;	g = y;	b = v;	break;
	case 5:	r = v;	g = y;	b = z;	break;
	}
}

ImageFilterMap gFilterMap[FilterEffect::NUM_FILTER_EFFECTS];

void FilterEffectInitForApp()
{
}

void FilterEffectDisposeForApp()
{
	for (int i = 0; i < static_cast<int>(FilterEffect::NUM_FILTER_EFFECTS); i++)
	{
		ImageFilterMap& aFilterMap = gFilterMap[i];

		aFilterMap.clear();
	}
}

void FilterEffectDoLumSat(MemoryImage* theImage, float theLum, float theSat)
{
	uint32_t* ptr = theImage->mBits.get();
	for (int y = 0; y < theImage->mHeight; y++)
	{
		for (int x = 0; x < theImage->mWidth; x++)
		{
			float b = static_cast<float>(*ptr & 255) / 255;
			float g = static_cast<float>(*ptr >> 8 & 255) / 255;
			float r = static_cast<float>(*ptr >> 16 & 255) / 255;
			int a = *ptr >> 24 & 255;

			float h, s, l;
			RGB_to_HSL(r, g, b, h, s, l);
			s *= theSat;
			l *= theLum;
			HSL_to_RGB(h, s, l, r, g, b);

			*ptr = a << 24 | std::clamp(static_cast<int>(r * 255), 0, 255) << 16 | std::clamp(static_cast<int>(g * 255), 0, 255) << 8 | std::clamp(static_cast<int>(b * 255), 0, 255);
			ptr++;
		}
	}
}

void FilterEffectDoWashedOut(MemoryImage* theImage)
{
	FilterEffectDoLumSat(theImage, 1.8f, 0.2f);
}

void FilterEffectDoLessWashedOut(MemoryImage* theImage)
{
	FilterEffectDoLumSat(theImage, 1.2f, 0.3f);
}

void FilterEffectDoWhite(MemoryImage* theImage)
{
	uint32_t* ptr = theImage->mBits.get();
	for (int y = 0; y < theImage->mHeight; y++)
		for (int x = 0; x < theImage->mWidth; x++)
			*ptr++ |= 0x00FFFFFF;
}

MemoryImage* FilterEffectCreateImage(Image* theImage, FilterEffect theFilterEffect)
{
	MemoryImage* aImage = new MemoryImage();
	aImage->mWidth = theImage->mWidth;
	aImage->mHeight = theImage->mHeight;
	int aNumBits = theImage->mWidth * theImage->mHeight;
	aImage->mBits = std::make_unique<uint32_t[]>(aNumBits + 1);
	aImage->mHasTrans = true;
	aImage->mHasAlpha = true;
	memset(aImage->mBits.get(), 0, aNumBits * 4);
	aImage->mBits[aNumBits] = Sexy::MEMORYCHECK_ID;

	Graphics aMemoryGraphics(aImage);
	aMemoryGraphics.DrawImage(theImage, 0, 0);
	FixPixelsOnAlphaEdgeForBlending(aImage);

	switch (theFilterEffect)
	{
	case FilterEffect::FILTER_EFFECT_WASHED_OUT:		FilterEffectDoWashedOut(aImage);		break;
	case FilterEffect::FILTER_EFFECT_LESS_WASHED_OUT:	FilterEffectDoLessWashedOut(aImage);	break;
	case FilterEffect::FILTER_EFFECT_WHITE:				FilterEffectDoWhite(aImage);			break;
	case FilterEffect::NUM_FILTER_EFFECTS:
	case FilterEffect::FILTER_EFFECT_NONE:
		break;
	}

	aImage->mBitsChangedCount++;
	aImage->mNumCols = theImage->mNumCols;
	aImage->mNumRows = theImage->mNumRows;
	return aImage;
}

Image* FilterEffectGetImage(Image* theImage, FilterEffect theFilterEffect)
{
	PVZP_ASSERT(theFilterEffect >= 0 && theFilterEffect < FilterEffect::NUM_FILTER_EFFECTS);

	ImageFilterMap& aFilterMap = gFilterMap[theFilterEffect];
	ImageFilterMap::iterator it = aFilterMap.find(theImage);
	if (it != aFilterMap.end())
		return it->second.get();

	MemoryImage* aImage = FilterEffectCreateImage(theImage, theFilterEffect);
	aFilterMap.insert(ImageFilterMap::value_type(theImage, aImage));
	return aImage;
}
