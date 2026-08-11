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

#ifndef __GRIDITEM_H__
#define __GRIDITEM_H__

#include <cstdint>
#include "../ConstEnums.h"

#define NUM_MOTION_TRAIL_FRAMES 12

class LawnApp;
class Board;
class Zombie;
namespace Sexy
{
	class Graphics;
};

class MotionTrailFrame
{
public:
	float					mPosX;
	float					mPosY;
	float					mAnimTime;
};

class GridItem
{
public:
	LawnApp*				mApp;
	Board*					mBoard;
	GridItemType			mGridItemType;
	GridItemState			mGridItemState;
	int32_t					mGridX;
	int32_t					mGridY;
	int32_t					mGridItemCounter;
	int32_t					mRenderOrder;
	bool					mDead;
	float					mPosX;
	float					mPosY;
	float					mGoalX;
	float					mGoalY;
	ReanimationID			mGridItemReanimID;
	ParticleSystemID		mGridItemParticleID;
	ZombieType				mZombieType;
	SeedType				mSeedType;
	ScaryPotType			mScaryPotType;
	bool					mHighlighted;
	int32_t					mTransparentCounter;
	int32_t					mSunCount;
	MotionTrailFrame		mMotionTrailFrames[NUM_MOTION_TRAIL_FRAMES];
	int32_t					mMotionTrailCount;

public:
	GridItem();

	void					DrawLadder(Sexy::Graphics* g);
	void					DrawCrater(Sexy::Graphics* g);
	void					DrawGraveStone(Sexy::Graphics* g);
	void					GridItemDie();
	void					AddGraveStoneParticles();
	void					DrawGridItem(Sexy::Graphics* g);
	void					DrawGridItemOverlay(Sexy::Graphics* g);
	void					OpenPortal();
	void					Update();
	void					ClosePortal();
	void					DrawScaryPot(Sexy::Graphics* g);
	void					UpdateScaryPot();
	void					UpdatePortal();
	void					UpdateRake();
	Zombie*					RakeFindZombie();
	void					DrawIZombieBrain(Sexy::Graphics* g);
	void					UpdateBrain();
	void					DrawStinky(Sexy::Graphics* g);
	bool			IsOpenPortal();
};

#endif
