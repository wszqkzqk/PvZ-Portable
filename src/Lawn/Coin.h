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

#ifndef __COIN_H__
#define __COIN_H__

#include <cstdint>
#include "GameObject.h"
#include "System/PlayerInfo.h"

class HitResult;
namespace Sexy
{
	class Graphics;
}
using namespace Sexy;

class Coin : public GameObject
{
public:
	float                   mPosX;
	float                   mPosY;
	float                   mVelX;
	float                   mVelY;
	float                   mScale;
	bool                    mDead;
	int32_t                 mFadeCount;
	float                   mCollectX;
	float                   mCollectY;
	int32_t                 mGroundY;
	int32_t                 mCoinAge;
	bool                    mIsBeingCollected;
	int32_t                 mDisappearCounter;
	CoinType                mType;
	CoinMotion              mCoinMotion;
	AttachmentID            mAttachmentID;
	float                   mCollectionDistance;
	SeedType                mUsableSeedType;
	PottedPlant             mPottedPlantSpec;
	bool                    mNeedsBouncyArrow;
	bool                    mHasBouncyArrow;
	bool                    mHitGround;
	int32_t                 mTimesDropped;

public:
	Coin();
	~Coin();

	void                    CoinInitialize(int theX, int theY, CoinType theCoinType, CoinMotion theCoinMotion);
	void                    MouseDown(int x, int y, int theClickCount);
	bool                    MouseHitTest(int theX, int theY, HitResult* theHitResult);
	void                    Die();
	void                    StartFade();
	void                    Update();
	void                    Draw(Graphics* g);
	void                    Collect();
	int          GetSunValue();
	static int   GetCoinValue(CoinType theCoinType);
	void                    UpdateFade();
	void                    UpdateFall();
	void                    ScoreCoin();
	void                    UpdateCollected();
	Color                   GetColor();
	bool         IsMoney();
	bool         IsSun();
	float                   GetSunScale();
	inline bool             IsOnGround() { return false; }
	SeedType                GetFinalSeedPacketType();
	bool                    IsLevelAward();
	bool                    CoinGetsBouncyArrow();
	void                    FanOutCoins(CoinType theCoinType, int theNumCoins);
	int                     GetDisappearTime();
	void                    DroppedUsableSeed();
	void                    PlayCollectSound();
	void                    TryAutoCollectAfterLevelAward();
	bool                    IsPresentWithAdvice();
	void                    PlayLaunchSound();
	void                    PlayGroundSound();

	static bool  IsMoney(CoinType theType);
};

#endif
