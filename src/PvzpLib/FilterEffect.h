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

#ifndef __FILTEREFFECT_H__
#define __FILTEREFFECT_H__

#include <cstdint>
#include <map>
#include <memory>

namespace Sexy
{
	class Image;
	class MemoryImage;
}
using namespace Sexy;

enum FilterEffect : int32_t
{
	FILTER_EFFECT_NONE = -1,
	FILTER_EFFECT_WASHED_OUT,
	FILTER_EFFECT_LESS_WASHED_OUT,
	FILTER_EFFECT_WHITE,
	NUM_FILTER_EFFECTS
};

typedef std::map<Image*, std::unique_ptr<Image>> ImageFilterMap;
extern ImageFilterMap gFilterMap[FilterEffect::NUM_FILTER_EFFECTS];

void                FilterEffectInitForApp();
void                FilterEffectDisposeForApp();
void                FilterEffectDoLumSat(MemoryImage* theImage, float theLum, float theSat);
void     FilterEffectDoWashedOut(MemoryImage* theImage);
void     FilterEffectDoLessWashedOut(MemoryImage* theImage);
void                FilterEffectDoWhite(MemoryImage* theImage);
MemoryImage*        FilterEffectCreateImage(Image* theImage, FilterEffect theFilterEffect);
Image*              FilterEffectGetImage(Image* theImage, FilterEffect theFilterEffect);

#endif
