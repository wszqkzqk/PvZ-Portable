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

#ifndef __LAWNMOWER_H__
#define __LAWNMOWER_H__

#include <cstdint>
#include "../ConstEnums.h"
#include "misc/Rect.h"

class LawnApp;
class Board;
class Zombie;
namespace Sexy
{
	class Graphics;
};
using namespace Sexy;

class LawnMower
{
public:
	LawnApp*            mApp;
	Board*              mBoard;
	float               mPosX;
	float               mPosY;
	int32_t             mRenderOrder;
	int32_t             mRow;
	int32_t             mAnimTicksPerFrame;
	ReanimationID       mReanimID;
	int32_t             mChompCounter;
	int32_t             mRollingInCounter;
	int32_t             mSquishedCounter;
	LawnMowerState      mMowerState;
	bool                mDead;
	bool                mVisible;
	LawnMowerType       mMowerType;
	float               mAltitude;
	MowerHeight         mMowerHeight;
	int32_t             mLastPortalX;

public:
	void                LawnMowerInitialize(int theRow);
	void                StartMower();
	void                Update();
	void                Draw(Graphics* g);
	void                Die();
	Rect                GetLawnMowerAttackRect();
	void                UpdatePool();
	void                MowZombie(Zombie* theZombie);
	void                SquishMower();
	void     EnableSuperMower(bool theEnable);
};

#endif
