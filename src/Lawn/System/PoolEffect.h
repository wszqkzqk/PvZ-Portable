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

#ifndef __POOLEFFECT_H__
#define __POOLEFFECT_H__

#include <memory>
#include <vector>
#include "../../ConstEnums.h"

namespace Sexy
{
	class MemoryImage;
	class Graphics;
};

class LawnApp;
class PoolEffect
{
public:
	std::vector<unsigned char>			mCausticGrayscaleImage;
	std::unique_ptr<Sexy::MemoryImage>	mCausticImage;
	LawnApp*			mApp;
	unsigned int		mPoolCounter;

public:
	void				PoolEffectInitialize();
	void				PoolEffectDraw(Sexy::Graphics* g, bool theIsNight);
	void				UpdateWaterEffect();
	unsigned int		BilinearLookupFixedPoint(unsigned int u, unsigned int v);
	//unsigned int		BilinearLookup(float u, float v);
	void				PoolEffectUpdate();
};

#endif
