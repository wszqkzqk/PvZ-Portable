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

#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include <cstdint>
#include "../ConstEnums.h"
#include "GameObject.h"

class Plant;
class Zombie;
namespace Sexy
{
	class Graphics;
};
using namespace Sexy;

class ProjectileDefinition
{
public:
	ProjectileType          mProjectileType;
	int32_t                 mImageRow;
	int32_t                 mDamage;
};
extern const ProjectileDefinition gProjectileDefinition[NUM_PROJECTILES];

class Projectile : public GameObject
{
public:
	int32_t                 mFrame;
	int32_t                 mNumFrames;
	int32_t                 mAnimCounter;
	float                   mPosX;
	float                   mPosY;
	float                   mPosZ;
	float                   mVelX;
	float                   mVelY;
	float                   mVelZ;
	float                   mAccZ;
	float                   mShadowY;
	bool                    mDead;
	int32_t                 mAnimTicksPerFrame;
	ProjectileMotion        mMotionType;
	ProjectileType          mProjectileType;
	int32_t                 mProjectileAge;
	int32_t                 mClickBackoffCounter;
	float                   mRotation;
	float                   mRotationSpeed;
	bool                    mOnHighGround;
	int32_t                 mDamageRangeFlags;
	int32_t                 mHitTorchwoodGridX;
	AttachmentID            mAttachmentID;
	float                   mCobTargetX;
	int32_t                 mCobTargetRow;
	ZombieID                mTargetZombieID;
	int32_t                 mLastPortalX;

public:
	Projectile();
	~Projectile();

	void                    ProjectileInitialize(int theX, int theY, int theRenderOrder, int theRow, ProjectileType theProjectileType);
	void                    Update();
	void                    Draw(Graphics* g);
	void                    DrawShadow(Graphics* g);
	void                    Die();
	void                    DoImpact(Zombie* theZombie);
	void                    UpdateMotion();
	void                    CheckForCollision();
	Zombie*                 FindCollisionTarget();
	void                    UpdateLobMotion();
	void                    CheckForHighGround();
	bool                    CantHitHighGround();
	void                    DoSplashDamage(Zombie* theZombie);
	const ProjectileDefinition&   GetProjectileDef();
	unsigned int            GetDamageFlags(Zombie* theZombie/* = nullptr*/);
	Rect                    GetProjectileRect();
	void                    UpdateNormalMotion();
	Plant*                  FindCollisionTargetPlant();
	void                    ConvertToFireball(int theGridX);
	void                    ConvertToPea(int theGridX);
	bool                    IsSplashDamage(Zombie* theZombie/* = nullptr*/);
	void                    PlayImpactSound(Zombie* theZombie);
	bool                    IsZombieHitBySplash(Zombie* theZombie);
	bool                    PeaAboutToHitTorchwood();

};

#endif
