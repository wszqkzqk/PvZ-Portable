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

#ifndef __REANIMATORCACHE_H__
#define __REANIMATORCACHE_H__

#include "../../ConstEnums.h"
#include "../../PvzpLib/PvzpList.h"
namespace Sexy
{
	class Graphics;
	class MemoryImage;
};
using namespace Sexy;

class LawnApp;
class ReanimCacheImageVariation
{
public:
	SeedType                mSeedType;
	DrawVariation           mDrawVariation;
	MemoryImage*            mImage;
};
typedef PvzpList<ReanimCacheImageVariation> ImageVariationList;

class Reanimation;
class ReanimatorCache
{
public:
	MemoryImage*			mPlantImages[SeedType::NUM_SEED_TYPES];
	ImageVariationList      mImageVariationList;
	MemoryImage*            mLawnMowers[LawnMowerType::NUM_MOWER_TYPES];
	MemoryImage*            mZombieImages[ZombieType::NUM_CACHED_ZOMBIE_TYPES];
	LawnApp*                mApp;

public:
	~ReanimatorCache();

	void                    ReanimatorCacheInitialize();
	void                    ReanimatorCacheDispose();
	void                    DrawCachedPlant(Graphics* g, float thePosX, float thePosY, SeedType theSeedType, DrawVariation theDrawVariation);
	void                    DrawCachedMower(Graphics* g, float thePosX, float thePosY, LawnMowerType theMowerType);
	void                    DrawCachedZombie(Graphics* g, float thePosX, float thePosY, ZombieType theZombieType);
	MemoryImage*            MakeBlankMemoryImage(int theWidth, int theHeight);
	MemoryImage*            MakeCachedPlantFrame(SeedType theSeedType, DrawVariation theDrawVariation);
	MemoryImage*            MakeCachedMowerFrame(LawnMowerType theMowerType);
	MemoryImage*            MakeCachedZombieFrame(ZombieType theZombieType);
	void         GetPlantImageSize(SeedType theSeedType, int& theOffsetX, int& theOffsetY, int& theWidth, int& theHeight);
	void                    DrawReanimatorFrame(Graphics* g, float thePosX, float thePosY, ReanimationType theReanimationType, const char* theTrackName, DrawVariation theDrawVariation);
	void                    UpdateReanimationForVariation(Reanimation* theReanim, DrawVariation theDrawVariation);
};

#endif
