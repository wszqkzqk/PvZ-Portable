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
#include "Definition.h"
#include "PvzpParticle.h"
#include "EffectSystem.h"
#include "../GameConstants.h"
#include "graphics/Graphics.h"
#include "graphics/GLInterface.h"
#include <algorithm>

int gParticleDefCount;
PvzpParticleDefinition* gParticleDefArray;
int gParticleParamArraySize;
const ParticleParams* gParticleParamArray;

constinit const ParticleParams gLawnParticleArray[ParticleEffect::NUM_PARTICLES] = {
	{ .mParticleEffect = ParticleEffect::PARTICLE_MELONSPLASH, .mParticleFileName = "particles/MelonImpact.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_WINTERMELON, .mParticleFileName = "particles/WinterMelonImpact.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_FUMECLOUD, .mParticleFileName = "particles/FumeCloud.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_POPCORNSPLASH, .mParticleFileName = "particles/PopcornSplash.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_POWIE, .mParticleFileName = "particles/Powie.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_JACKEXPLODE, .mParticleFileName = "particles/JackExplode.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_HEAD, .mParticleFileName = "particles/ZombieHead.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_ARM, .mParticleFileName = "particles/ZombieArm.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_TRAFFIC_CONE, .mParticleFileName = "particles/ZombieTrafficCone.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_PAIL, .mParticleFileName = "particles/ZombiePail.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_HELMET, .mParticleFileName = "particles/ZombieHelmet.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_FLAG, .mParticleFileName = "particles/ZombieFlag.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_DOOR, .mParticleFileName = "particles/ZombieDoor.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_NEWSPAPER, .mParticleFileName = "particles/ZombieNewspaper.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_HEADLIGHT, .mParticleFileName = "particles/ZombieHeadLight.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_POW, .mParticleFileName = "particles/Pow.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_POGO, .mParticleFileName = "particles/ZombiePogo.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_NEWSPAPER_HEAD, .mParticleFileName = "particles/ZombieNewspaperHead.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_BALLOON_HEAD, .mParticleFileName = "particles/ZombieBalloonHead.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_SOD_ROLL, .mParticleFileName = "particles/SodRoll.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_GRAVE_STONE_RISE, .mParticleFileName = "particles/GraveStoneRise.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_PLANTING, .mParticleFileName = "particles/Planting.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_PLANTING_POOL, .mParticleFileName = "particles/PlantingPool.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_RISE, .mParticleFileName = "particles/ZombieRise.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_GRAVE_BUSTER, .mParticleFileName = "particles/GraveBuster.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_GRAVE_BUSTER_DIE, .mParticleFileName = "particles/GraveBusterDie.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_POOL_SPLASH, .mParticleFileName = "particles/PoolSplash.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ICE_SPARKLE, .mParticleFileName = "particles/IceSparkle.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_SEED_PACKET, .mParticleFileName = "particles/SeedPacket.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_TALL_NUT_BLOCK, .mParticleFileName = "particles/TallNutBlock.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_DOOM, .mParticleFileName = "particles/Doom.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_DIGGER_RISE, .mParticleFileName = "particles/DiggerRise.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_DIGGER_TUNNEL, .mParticleFileName = "particles/DiggerTunnel.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_DANCER_RISE, .mParticleFileName = "particles/DancerRise.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_POOL_SPARKLY, .mParticleFileName = "particles/PoolSparkly.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_WALLNUT_EAT_SMALL, .mParticleFileName = "particles/WallnutEatSmall.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_WALLNUT_EAT_LARGE, .mParticleFileName = "particles/WallnutEatLarge.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_PEA_SPLAT, .mParticleFileName = "particles/PeaSplat.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_BUTTER_SPLAT, .mParticleFileName = "particles/ButterSplat.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_CABBAGE_SPLAT, .mParticleFileName = "particles/CabbageSplat.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_PUFF_SPLAT, .mParticleFileName = "particles/PuffSplat.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_STAR_SPLAT, .mParticleFileName = "particles/StarSplat.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ICE_TRAP, .mParticleFileName = "particles/IceTrap.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_SNOWPEA_SPLAT, .mParticleFileName = "particles/SnowPeaSplat.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_SNOWPEA_PUFF, .mParticleFileName = "particles/SnowPeaPuff.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_SNOWPEA_TRAIL, .mParticleFileName = "particles/SnowPeaTrail.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_LANTERN_SHINE, .mParticleFileName = "particles/LanternShine.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_SEED_PACKET_PICKUP, .mParticleFileName = "particles/Award.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_POTATO_MINE, .mParticleFileName = "particles/PotatoMine.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_POTATO_MINE_RISE, .mParticleFileName = "particles/PotatoMineRise.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_PUFFSHROOM_TRAIL, .mParticleFileName = "particles/PuffShroomTrail.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_PUFFSHROOM_MUZZLE, .mParticleFileName = "particles/PuffShroomMuzzle.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_SEED_PACKET_FLASH, .mParticleFileName = "particles/SeedPacketFlash.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_WHACK_A_ZOMBIE_RISE, .mParticleFileName = "particles/WhackAZombieRise.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_LADDER, .mParticleFileName = "particles/ZombieLadder.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_UMBRELLA_REFLECT, .mParticleFileName = "particles/UmbrellaReflect.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_SEED_PACKET_PICK, .mParticleFileName = "particles/SeedPacketPick.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ICE_TRAP_ZOMBIE, .mParticleFileName = "particles/IceTrapZombie.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ICE_TRAP_RELEASE, .mParticleFileName = "particles/IceTrapRelease.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZAMBONI_SMOKE, .mParticleFileName = "particles/ZamboniSmoke.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_GLOOMCLOUD, .mParticleFileName = "particles/GloomCloud.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_POGO_HEAD, .mParticleFileName = "particles/ZombiePogoHead.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZAMBONI_TIRE, .mParticleFileName = "particles/ZamboniTire.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZAMBONI_EXPLOSION, .mParticleFileName = "particles/ZamboniExplosion.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZAMBONI_EXPLOSION2, .mParticleFileName = "particles/ZamboniExplosion2.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_CATAPULT_EXPLOSION, .mParticleFileName = "particles/CatapultExplosion.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_MOWER_CLOUD, .mParticleFileName = "particles/MowerCloud.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_BOSS_ICE_BALL, .mParticleFileName = "particles/BossIceBallTrail.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_BLASTMARK, .mParticleFileName = "particles/BlastMark.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_COIN_PICKUP_ARROW, .mParticleFileName = "particles/CoinPickupArrow.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_PRESENT_PICKUP, .mParticleFileName = "particles/PresentPickup.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_IMITATER_MORPH, .mParticleFileName = "particles/ImitaterMorph.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_MOWERED_ZOMBIE_HEAD, .mParticleFileName = "particles/MoweredZombieHead.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_MOWERED_ZOMBIE_ARM, .mParticleFileName = "particles/MoweredZombieArm.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_HEAD_POOL, .mParticleFileName = "particles/ZombieHeadPool.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_BOSS_FIREBALL, .mParticleFileName = "particles/Zombie_boss_fireball.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_FIREBALL_DEATH, .mParticleFileName = "particles/FireballDeath.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ICEBALL_DEATH, .mParticleFileName = "particles/IceballDeath.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ICEBALL_TRAIL, .mParticleFileName = "particles/Iceball_Trail.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_FIREBALL_TRAIL, .mParticleFileName = "particles/Fireball_Trail.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_BOSS_EXPLOSION, .mParticleFileName = "particles/BossExplosion.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_SCREEN_FLASH, .mParticleFileName = "particles/ScreenFlash.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_TROPHY_SPARKLE, .mParticleFileName = "particles/TrophySparkle.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_PORTAL_CIRCLE, .mParticleFileName = "particles/PortalCircle.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_PORTAL_SQUARE, .mParticleFileName = "particles/PortalSquare.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_POTTED_PLANT_GLOW, .mParticleFileName = "particles/PottedPlantGlow.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_POTTED_WATER_PLANT_GLOW, .mParticleFileName = "particles/PottedWaterPlantGlow.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_POTTED_ZEN_GLOW, .mParticleFileName = "particles/PottedZenGlow.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_MIND_CONTROL, .mParticleFileName = "particles/MindControl.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_VASE_SHATTER, .mParticleFileName = "particles/VaseShatter.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_VASE_SHATTER_LEAF, .mParticleFileName = "particles/VaseShatterLeaf.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_VASE_SHATTER_ZOMBIE, .mParticleFileName = "particles/VaseShatterZombie.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_AWARD_PICKUP_ARROW, .mParticleFileName = "particles/AwardPickupArrow.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_SEAWEED, .mParticleFileName = "particles/Zombie_seaweed.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_MUSTACHE, .mParticleFileName = "particles/ZombieMustache.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_SUNGLASS, .mParticleFileName = "particles/ZombieFutureGlasses.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_PINATA, .mParticleFileName = "particles/Pinata.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_DUST_SQUASH, .mParticleFileName = "particles/Dust_Squash.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_DUST_FOOT, .mParticleFileName = "particles/Dust_Foot.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_ZOMBIE_DAISIES, .mParticleFileName = "particles/Daisy.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_CREDIT_STROBE, .mParticleFileName = "particles/Credits_Strobe.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_CREDITS_RAYSWIPE, .mParticleFileName = "particles/Credits_RaysWipe.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_CREDITS_ZOMBIEHEADWIPE, .mParticleFileName = "particles/Credits_ZombieHeadWipe.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_STARBURST, .mParticleFileName = "particles/Starburst.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_CREDITS_FOG, .mParticleFileName = "particles/Credits_fog.xml" },
	{ .mParticleEffect = ParticleEffect::PARTICLE_PERSENT_PICK_UP_ARROW, .mParticleFileName = "particles/UpsellArrow.xml" },
};  // 0x6A0FF0

bool PvzpParticleLoadADef(PvzpParticleDefinition* theParticleDef, const char* theParticleFileName)
{
	PvzpHesitationBracket("Load Particle %s", theParticleFileName);
	if (!DefinitionLoadXML(theParticleFileName, &gParticleDefMap, theParticleDef))
	{
		char aBuf[512];
		snprintf(aBuf, sizeof(aBuf), "Failed to load particle '%s'", theParticleFileName);
		PvzpErrorMessageBox(aBuf, "Error");
		return false;
	}
	else
	{
		for (int i = 0; i < theParticleDef->mEmitterDefCount; i++)
		{
			PvzpEmitterDefinition& aDef = theParticleDef->mEmitterDefs[i];
			FloatTrackSetDefault(aDef.mSystemDuration, 0.0f);
			FloatTrackSetDefault(aDef.mSpawnRate, 0.0f);
			FloatTrackSetDefault(aDef.mSpawnMinActive, -1.0f);
			FloatTrackSetDefault(aDef.mSpawnMaxActive, -1.0f);
			FloatTrackSetDefault(aDef.mSpawnMaxLaunched, -1.0f);
			FloatTrackSetDefault(aDef.mEmitterRadius, 0.0f);
			FloatTrackSetDefault(aDef.mEmitterOffsetX, 0.0f);
			FloatTrackSetDefault(aDef.mEmitterOffsetY, 0.0f);
			FloatTrackSetDefault(aDef.mEmitterBoxX, 0.0f);
			FloatTrackSetDefault(aDef.mEmitterBoxY, 0.0f);
			FloatTrackSetDefault(aDef.mEmitterSkewX, 0.0f);
			FloatTrackSetDefault(aDef.mEmitterSkewY, 0.0f);
			FloatTrackSetDefault(aDef.mParticleDuration, 100.0f);
			FloatTrackSetDefault(aDef.mLaunchSpeed, 0.0f);
			FloatTrackSetDefault(aDef.mSystemRed, 1.0f);
			FloatTrackSetDefault(aDef.mSystemGreen, 1.0f);
			FloatTrackSetDefault(aDef.mSystemBlue, 1.0f);
			FloatTrackSetDefault(aDef.mSystemAlpha, 1.0f);
			FloatTrackSetDefault(aDef.mSystemBrightness, 1.0f);
			FloatTrackSetDefault(aDef.mLaunchAngle, 0.0f);
			FloatTrackSetDefault(aDef.mCrossFadeDuration, 0.0f);
			FloatTrackSetDefault(aDef.mParticleRed, 1.0f);
			FloatTrackSetDefault(aDef.mParticleGreen, 1.0f);
			FloatTrackSetDefault(aDef.mParticleBlue, 1.0f);
			FloatTrackSetDefault(aDef.mParticleAlpha, 1.0f);
			FloatTrackSetDefault(aDef.mParticleBrightness, 1.0f);
			FloatTrackSetDefault(aDef.mParticleSpinAngle, 0.0f);
			FloatTrackSetDefault(aDef.mParticleSpinSpeed, 0.0f);
			FloatTrackSetDefault(aDef.mParticleScale, 1.0f);
			FloatTrackSetDefault(aDef.mParticleStretch, 1.0f);
			FloatTrackSetDefault(aDef.mCollisionReflect, 0.0f);
			FloatTrackSetDefault(aDef.mCollisionSpin, 0.0f);
			FloatTrackSetDefault(aDef.mClipTop, 0.0f);
			FloatTrackSetDefault(aDef.mClipBottom, 0.0f);
			FloatTrackSetDefault(aDef.mClipLeft, 0.0f);
			FloatTrackSetDefault(aDef.mClipRight, 0.0f);
			FloatTrackSetDefault(aDef.mAnimationRate, 0.0f);
			if (aDef.mImage)
				reinterpret_cast<MemoryImage*>(aDef.mImage)->mRenderFlags |= RenderImageFlags::RenderImageFlag_MinimizeNumSubdivisions;
		}
		return true;
	}
}

void PvzpParticleLoadDefinitions(const ParticleParams* theParticleParamArray, int theParticleParamArraySize)
{
	PvzpHesitationBracket aHesitiation("PvzpParticleLoadDefinitions");
	PVZP_ASSERT(!gParticleParamArray && !gParticleDefArray);
	gParticleParamArraySize = theParticleParamArraySize;
	gParticleParamArray = theParticleParamArray;
	gParticleDefCount = theParticleParamArraySize;
	gParticleDefArray = new PvzpParticleDefinition[theParticleParamArraySize];
	// This was uninitialised before!
	// memset(gParticleDefArray, 0, theParticleParamArraySize*sizeof(PvzpParticleDefinition));

	for (int i = 0; i < gParticleParamArraySize; i++)
	{
		const ParticleParams& aParticleParams = gParticleParamArray[i];
		PVZP_ASSERT(aParticleParams.mParticleEffect == i);
		if (!PvzpParticleLoadADef(&gParticleDefArray[i], aParticleParams.mParticleFileName))
		{
			char aBuf[512];
			snprintf(aBuf, sizeof(aBuf), "Failed to load particle '%s'", aParticleParams.mParticleFileName);
			PvzpErrorMessageBox(aBuf, "Error");
		}
		gSexyAppBase->mCompletedLoadingThreadTasks += 6;
	}
}

void PvzpParticleFreeDefinitions()
{
	for (int i = 0; i < gParticleDefCount; i++)
		DefinitionFreeMap(&gParticleDefMap, &gParticleDefArray[i]);
	delete[] gParticleDefArray;
	gParticleDefArray = nullptr;
	gParticleDefCount = 0;
	gParticleParamArray = nullptr;
	gParticleParamArraySize = 0;
}

PvzpParticleSystem::PvzpParticleSystem()
{
	mEffectType = ParticleEffect::PARTICLE_NONE;
	mParticleDef = nullptr;
	mParticleHolder = nullptr;
	mDead = false;
	mDontUpdate = false;
	mIsAttachment = false;
	mRenderOrder = 0;
}

PvzpParticleSystem::~PvzpParticleSystem()
{
	ParticleSystemDie();
	mEmitterList.RemoveAll();
}

void PvzpParticleSystem::PvzpParticleInitializeFromDef(float theX, float theY, int theRenderOrder, PvzpParticleDefinition* theDefinition, ParticleEffect theEffectType)
{
	PVZP_ASSERT(mParticleHolder);
	mEmitterList.SetAllocator(&mParticleHolder->mEmitterListNodeAllocator);
	mParticleDef = theDefinition;
	mEffectType = theEffectType;
	mRenderOrder = theRenderOrder;

	for (int i = 0; i < theDefinition->mEmitterDefCount; i++)
	{
		PvzpEmitterDefinition& aDef = theDefinition->mEmitterDefs[i];
		if (!FloatTrackIsSet(aDef.mCrossFadeDuration))
		{
			if (TestBit(aDef.mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_DIE_IF_OVERLOADED)) && mParticleHolder->IsOverLoaded())
			{
				ParticleSystemDie();
				break;
			}
			PvzpParticleEmitter* aEmitter = mParticleHolder->mEmitters.DataArrayAlloc();
			aEmitter->PvzpEmitterInitialize(theX, theY, this, &aDef);
			mEmitterList.AddTail(static_cast<ParticleEmitterID>(mParticleHolder->mEmitters.DataArrayGetID(aEmitter)));
		}
	}
}

void PvzpParticleEmitter::PvzpEmitterInitialize(float theX, float theY, PvzpParticleSystem* theSystem, PvzpEmitterDefinition* theEmitterDef)
{
	mSpawnAccum = 0.0f;
	mParticlesSpawned = 0;
	mSystemTimeValue = -1.0f;
	mSystemLastTimeValue = -1.0f;
	mSystemAge = -1;
	mDead = false;
	mColorOverride = Sexy::Color::White;
	mSystemCenter.x = theX;
	mSystemCenter.y = theY;
	mFrameOverride = -1;
	mParticleSystem = theSystem;
	mScaleOverride = 1.0f;
	mExtraAdditiveDrawOverride = false;
	mImageOverride = nullptr;
	mSystemDuration = 0;
	mEmitterDef = theEmitterDef;
	mParticleList.SetAllocator(&theSystem->mParticleHolder->mEmitterListNodeAllocator);

	if (FloatTrackIsSet(mEmitterDef->mSystemDuration))
		mSystemDuration = FloatTrackEvaluate(mEmitterDef->mSystemDuration, 0.0f, Sexy::Rand(1.0f));
	else
		mSystemDuration = FloatTrackEvaluate(mEmitterDef->mParticleDuration, 0.0f, 1.0f);
	mSystemDuration = std::max(1, mSystemDuration);

	for (int i = 0; i < mEmitterDef->mSystemFields.count; i++)
	{
		mSystemFieldInterp[i][0] = Sexy::Rand(1.0f);
		mSystemFieldInterp[i][1] = Sexy::Rand(1.0f);
	}
	for (int j = 0; j < 10; j++)
		mTrackInterp[j] = Sexy::Rand(1.0f);

	Update();
}

void PvzpParticleSystem::ParticleSystemDie()
{
	for (PvzpListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		PvzpParticleEmitter* aEmitter = mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
		aEmitter->DeleteAll();
		mParticleHolder->mEmitters.DataArrayFree(aEmitter);
	}
	mEmitterList.RemoveAll();
	mDead = true;
}

PvzpParticle* PvzpParticleEmitter::SpawnParticle(int theIndex, int theSpawnCount)
{
	DataArray<PvzpParticle>& aDataArray = mParticleSystem->mParticleHolder->mParticles;
	if (aDataArray.mSize == aDataArray.mMaxSize)
	{
		PvzpTraceWithoutSpamming("Too many particles '%s'\n", mEmitterDef->mName);
		return nullptr;
	}

	PvzpParticle* aParticle = aDataArray.DataArrayAlloc();
	PVZP_ASSERT(mEmitterDef->mParticleFields.count <= MAX_PARTICLE_FIELDS);
	for (int i = 0; i < mEmitterDef->mParticleFields.count; i++)
	{
		aParticle->mParticleFieldInterp[i][0] = Sexy::Rand(1.0f);  // random X interp for each particle field
		aParticle->mParticleFieldInterp[i][1] = Sexy::Rand(1.0f);  // random Y interp for each particle field
	}
	for (int i = 0; i < static_cast<int>(ParticleTracks::NUM_PARTICLE_TRACKS); i++)
		aParticle->mParticleInterp[i] = Sexy::Rand(1.0f);  // random interp for each track

	float aParticleDurationInterp = Sexy::Rand(1.0f);
	float aLaunchSpeedInterp = Sexy::Rand(1.0f);
	float aEmitterOffsetXInterp = Sexy::Rand(1.0f);
	float aEmitterOffsetYInterp = Sexy::Rand(1.0f);
	aParticle->mParticleDuration = FloatTrackEvaluate(mEmitterDef->mParticleDuration, mSystemTimeValue, aParticleDurationInterp);
	aParticle->mParticleDuration = std::max(1, aParticle->mParticleDuration);  // duration is at least 1
	aParticle->mParticleAge = 0;
	aParticle->mParticleEmitter = this;
	aParticle->mParticleTimeValue = -1.0f;
	aParticle->mParticleLastTimeValue = -1.0f;
	if (TestBit(mEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_RANDOM_START_TIME)))
		aParticle->mParticleAge = Sexy::Rand(aParticle->mParticleDuration);  // start at a random age
	float aLaunchSpeed = FloatTrackEvaluate(mEmitterDef->mLaunchSpeed, mSystemTimeValue, aLaunchSpeedInterp) * 0.01f;
	float aLaunchAngleInterp = Sexy::Rand(1.0f);

	float aLaunchAngle;
	if (mEmitterDef->mEmitterType == EmitterType::EMITTER_CIRCLE_PATH)
	{
		// launch angle = base angle on the circle from the path definition + offset from the launch angle definition
		aLaunchAngle = FloatTrackEvaluate(mEmitterDef->mEmitterPath, mSystemTimeValue, mTrackInterp[ParticleSystemTracks::TRACK_EMITTER_PATH]) * 2 * PI;
		aLaunchAngle += DEG_TO_RAD(FloatTrackEvaluate(mEmitterDef->mLaunchAngle, mSystemTimeValue, aLaunchAngleInterp));
	}
	else if (mEmitterDef->mEmitterType == EmitterType::EMITTER_CIRCLE_EVEN_SPACING)
		// base angle spreads theSpawnCount particles evenly around the circle
		aLaunchAngle = 2 * PI * theIndex / theSpawnCount + DEG_TO_RAD(FloatTrackEvaluate(mEmitterDef->mLaunchAngle, mSystemTimeValue, aLaunchAngleInterp));
	else if (FloatTrackIsConstantZero(mEmitterDef->mLaunchAngle))
		// no track defined: use a random launch angle in [0, 2π]
		aLaunchAngle = Sexy::Rand(static_cast<float>(2 * PI));
	else
		aLaunchAngle = DEG_TO_RAD(FloatTrackEvaluate(mEmitterDef->mLaunchAngle, mSystemTimeValue, aLaunchAngleInterp));

	float aPosX, aPosY;
	switch (mEmitterDef->mEmitterType)
	{
	case EmitterType::EMITTER_CIRCLE:
	case EmitterType::EMITTER_CIRCLE_PATH:
	case EmitterType::EMITTER_CIRCLE_EVEN_SPACING:
	{
		float aEmitterRadiusInterp = Sexy::Rand(1.0f);
		float aRadius = FloatTrackEvaluate(mEmitterDef->mEmitterRadius, mSystemTimeValue, aEmitterRadiusInterp);
		// angle 0 points straight down
		aPosX = sin(aLaunchAngle) * aRadius;
		aPosY = cos(aLaunchAngle) * aRadius;
		break;
	}
	case EmitterType::EMITTER_BOX:
	{
		float aEmitterBoxXInterp = Sexy::Rand(1.0f);
		float aEmitterBoxYInterp = Sexy::Rand(1.0f);
		aPosX = FloatTrackEvaluate(mEmitterDef->mEmitterBoxX, mSystemTimeValue, aEmitterBoxXInterp);
		aPosY = FloatTrackEvaluate(mEmitterDef->mEmitterBoxY, mSystemTimeValue, aEmitterBoxYInterp);
		break;
	}
	case EmitterType::EMITTER_BOX_PATH:
	{
		float aEmitterPathPosition = FloatTrackEvaluate(mEmitterDef->mEmitterPath, mSystemTimeValue, mTrackInterp[ParticleSystemTracks::TRACK_EMITTER_PATH]);
		float aMinX = FloatTrackEvaluate(mEmitterDef->mEmitterBoxX, mSystemTimeValue, 0.0f);
		float aMaxX = FloatTrackEvaluate(mEmitterDef->mEmitterBoxX, mSystemTimeValue, 1.0f);
		float aMinY = FloatTrackEvaluate(mEmitterDef->mEmitterBoxY, mSystemTimeValue, 0.0f);
		float aMaxY = FloatTrackEvaluate(mEmitterDef->mEmitterBoxY, mSystemTimeValue, 1.0f);
		float aDistanceX = aMaxX - aMinX;  // width of the path rectangle
		float aDistanceY = aMaxY - aMinY;  // height of the path rectangle
		float aPathPos = aEmitterPathPosition * (aDistanceY + aDistanceX + aDistanceY + aDistanceX);  // spawn position along the rectangle's edges
		// Label the vertices A, B, C, D counter-clockwise starting from the top-left corner, and the spawn point P;
		// aPathPos is the distance from A to P along the path. X points right and Y points down.
		if (aPathPos < aDistanceY)  // spawn point on edge AB
		{
			aPosX = aMinX;
			aPosY = aMinY + aPathPos;
		}
		else if (aPathPos < aDistanceY + aDistanceX)  // spawn point on edge BC
		{
			aPosX = aMinX + (aPathPos - aDistanceY);
			aPosY = aMaxY;
		}
		else if (aPathPos < aDistanceY + aDistanceX + aDistanceY)  // spawn point on edge CD
		{
			aPosX = aMaxX;
			aPosY = aMaxY - (aPathPos - aDistanceY - aDistanceX);
		}
		else  // spawn point on edge AD
		{
			aPosX = aMaxX - (aPathPos - aDistanceY - aDistanceX - aDistanceY);
			aPosY = aMinY;
		}
		break;
	}
	default:
		PVZP_ASSERT(false);
		break;
	}
	float aEmitterSkewXInterp = Sexy::Rand(1.0f);
	float aEmitterSkewYInterp = Sexy::Rand(1.0f);
	float aSkewX = FloatTrackEvaluate(mEmitterDef->mEmitterSkewX, mSystemTimeValue, aEmitterSkewXInterp);
	float aSkewY = FloatTrackEvaluate(mEmitterDef->mEmitterSkewY, mSystemTimeValue, aEmitterSkewYInterp);
	aParticle->mPosition.x = mSystemCenter.x + aPosX + aPosY * aSkewX;  // X skew scales with the Y coordinate
	aParticle->mPosition.y = mSystemCenter.y + aPosY + aPosX * aSkewY;  // Y skew scales with the X coordinate
	aParticle->mVelocity.x = sin(aLaunchAngle) * aLaunchSpeed;
	aParticle->mVelocity.y = cos(aLaunchAngle) * aLaunchSpeed;
	aParticle->mPosition.x += FloatTrackEvaluate(mEmitterDef->mEmitterOffsetX, mSystemTimeValue, aEmitterOffsetXInterp);
	aParticle->mPosition.y += FloatTrackEvaluate(mEmitterDef->mEmitterOffsetY, mSystemTimeValue, aEmitterOffsetYInterp);

	aParticle->mAnimationTimeValue = 0.0f;
	if (mEmitterDef->mAnimated || FloatTrackIsSet(mEmitterDef->mAnimationRate))
		aParticle->mImageFrame = 0;  // frame computed later from the time value; init to 0 for now
	else
		aParticle->mImageFrame = Sexy::Rand(mEmitterDef->mImageFrames);  // fixed-frame particle: pick a random frame once

	if (TestBit(mEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_RANDOM_LAUNCH_SPIN)))
		aParticle->mSpinPosition = Sexy::Rand(static_cast<float>(2 * PI));  // random initial spin in [0, 2π]
	else if (TestBit(mEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_ALIGN_LAUNCH_SPIN)))
		aParticle->mSpinPosition = aLaunchAngle;  // spin aligns with the launch angle
	else
		aParticle->mSpinPosition = 0.0f;  // no initial spin
	aParticle->mSpinVelocity = 0.0f;
	aParticle->mCrossFadeDuration = 0;
	aParticle->mCrossFadeParticleID = ParticleID::PARTICLEID_NULL;

	ParticleID aParticleID = static_cast<ParticleID>(aDataArray.DataArrayGetID(aParticle));
	mParticleList.AddHead(aParticleID);
	mParticlesSpawned++;
	UpdateParticle(aParticle);
	return aParticle;
}

float PvzpParticleEmitter::ParticleTrackEvaluate(FloatParameterTrack& theTrack, PvzpParticle* theParticle, ParticleTracks theParticleTrack)
{
	return FloatTrackEvaluate(theTrack, theParticle->mParticleTimeValue, theParticle->mParticleInterp[theParticleTrack]);
}

void PvzpParticleEmitter::UpdateParticleField(PvzpParticle* theParticle, ParticleField* theParticleField, float theParticleTimeValue, int theFieldIndex)
{
	PVZP_ASSERT(theFieldIndex < MAX_PARTICLE_FIELDS);
	float aInterpX = theParticle->mParticleFieldInterp[theFieldIndex][0];
	float aInterpY = theParticle->mParticleFieldInterp[theFieldIndex][1];
	float x = FloatTrackEvaluate(theParticleField->mX, theParticleTimeValue, aInterpX);
	float y = FloatTrackEvaluate(theParticleField->mY, theParticleTimeValue, aInterpY);

	switch (theParticleField->mFieldType)
	{
	case ParticleFieldType::FIELD_INVALID:
		break;
	case ParticleFieldType::FIELD_FRICTION:
		theParticle->mVelocity.x *= 1 - x;
		theParticle->mVelocity.y *= 1 - y;
		break;
	case ParticleFieldType::FIELD_ACCELERATION:
		theParticle->mVelocity.x += 0.01f * x;
		theParticle->mVelocity.y += 0.01f * y;
		break;
	case ParticleFieldType::FIELD_ATTRACTOR:
	{
		float aDiffX = x - (theParticle->mPosition.x - mSystemCenter.x);
		float aDiffY = y - (theParticle->mPosition.y - mSystemCenter.y);
		// acceleration points from the particle toward the target position
		theParticle->mVelocity.x += 0.01f * aDiffX;
		theParticle->mVelocity.y += 0.01f * aDiffY;
		break;
	}
	case ParticleFieldType::FIELD_MAX_VELOCITY:
		theParticle->mVelocity.x = std::clamp(theParticle->mVelocity.x, -x, x);
		theParticle->mVelocity.y = std::clamp(theParticle->mVelocity.y, -y, y);
		break;
	case ParticleFieldType::FIELD_VELOCITY:
		theParticle->mPosition.x += 0.01 * x;
		theParticle->mPosition.y += 0.01 * y;
		break;
	case ParticleFieldType::FIELD_POSITION:
	{
		float aLastX = FloatTrackEvaluateFromLastTime(theParticleField->mX, theParticle->mParticleLastTimeValue, aInterpX);
		float aLastY = FloatTrackEvaluateFromLastTime(theParticleField->mY, theParticle->mParticleLastTimeValue, aInterpY);
		theParticle->mPosition.x += x - aLastX;
		theParticle->mPosition.y += y - aLastY;
		break;
	}
	case ParticleFieldType::FIELD_GROUND_CONSTRAINT:
		if (theParticle->mPosition.y > mSystemCenter.y + y)  // check for ground contact
		{
			theParticle->mPosition.y = mSystemCenter.y + y;  // reset the position to the ground
			float aCollisionReflect = FloatTrackEvaluate(
				mEmitterDef->mCollisionReflect, theParticleTimeValue, theParticle->mParticleInterp[ParticleTracks::TRACK_PARTICLE_COLLISION_REFLECT]
			);
			float aCollisionSpin = FloatTrackEvaluate(
				mEmitterDef->mCollisionSpin, theParticleTimeValue, theParticle->mParticleInterp[ParticleTracks::TRACK_PARTICLE_COLLISION_SPIN]
			) / 1000.0f;
			theParticle->mSpinVelocity = theParticle->mVelocity.y * aCollisionSpin;
			theParticle->mVelocity.x *= aCollisionReflect;
			theParticle->mVelocity.y *= -aCollisionReflect;
		}
		break;
	case ParticleFieldType::FIELD_SHAKE:
	{
		float aLastX = FloatTrackEvaluateFromLastTime(theParticleField->mX, theParticle->mParticleLastTimeValue, aInterpX);
		float aLastY = FloatTrackEvaluateFromLastTime(theParticleField->mY, theParticle->mParticleLastTimeValue, aInterpY);
		// undo the previous frame's shake offset
		int aLastRandSeed = theParticle->mParticleAge - 1;
		if (aLastRandSeed == -1)
			aLastRandSeed = theParticle->mParticleDuration - 1;
		srand(aLastRandSeed * reinterpret_cast<uintptr_t>(theParticle));
		theParticle->mPosition.x -= aLastX * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f);
		theParticle->mPosition.y -= aLastY * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f);
		// apply this frame's random shake offset
		srand(theParticle->mParticleAge * reinterpret_cast<uintptr_t>(theParticle));
		theParticle->mPosition.x += x * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f);
		theParticle->mPosition.y += y * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f);
		break;
	}
	case ParticleFieldType::FIELD_CIRCLE:
	{
		SexyVector2 aToCenter = theParticle->mPosition - mSystemCenter;
		SexyVector2 aMotion = aToCenter.Perp().Normalize();
		float aRadius = aToCenter.Magnitude();
		aMotion *= 0.01 * (x + aRadius * y);
		theParticle->mPosition += aMotion;
		break;
	}
	case ParticleFieldType::FIELD_AWAY:
	{
		SexyVector2 aToCenter = theParticle->mPosition - mSystemCenter;
		SexyVector2 aMotion = aToCenter.Normalize();
		float aRadius = aToCenter.Magnitude();
		aMotion *= 0.01 * (x + aRadius * y);
		theParticle->mPosition += aMotion;
		break;
	}
	default:
		PVZP_ASSERT(0);
		break;
	}
}

float PvzpParticleEmitter::SystemTrackEvaluate(FloatParameterTrack& theTrack, ParticleSystemTracks theSystemTrack)
{
	return FloatTrackEvaluate(theTrack, mSystemTimeValue, mTrackInterp[theSystemTrack]);
}

void PvzpParticleEmitter::UpdateSystemField(ParticleField* theParticleField, float theParticleTimeValue, int theFieldIndex)
{
	PVZP_ASSERT(theFieldIndex < MAX_PARTICLE_FIELDS);
	float aInterpX = mSystemFieldInterp[theFieldIndex][0];
	float aInterpY = mSystemFieldInterp[theFieldIndex][1];
	float x = FloatTrackEvaluate(theParticleField->mX, theParticleTimeValue, aInterpX);
	float y = FloatTrackEvaluate(theParticleField->mY, theParticleTimeValue, aInterpY);

	switch (theParticleField->mFieldType)
	{
	case ParticleFieldType::FIELD_SYSTEM_POSITION:
	{
		float aLastX = FloatTrackEvaluateFromLastTime(theParticleField->mX, mSystemLastTimeValue, aInterpX);
		float aLastY = FloatTrackEvaluateFromLastTime(theParticleField->mY, mSystemLastTimeValue, aInterpY);
		mSystemCenter.x += x - aLastX;
		mSystemCenter.y += y - aLastY;
		break;
	}
	default:
		PVZP_ASSERT(0);
		break;
	}
}

bool PvzpParticleEmitter::CrossFadeParticleToName(PvzpParticle* theParticle, const char* theEmitterName)
{
	PvzpEmitterDefinition* aDef = mParticleSystem->FindEmitterDefByName(theEmitterName);
	if (aDef == nullptr)
	{
		PvzpTrace("Can't find emitter to cross fade: %s\n", theEmitterName);
		return false;
	}
	if (mParticleSystem->mParticleHolder->mEmitters.mSize == mParticleSystem->mParticleHolder->mEmitters.mMaxSize)
	{
		PvzpTrace("Too many emitters to cross fade\n");
		return false;
	}

	PvzpParticleEmitter* aEmitter = mParticleSystem->mParticleHolder->mEmitters.DataArrayAlloc();
	aEmitter->PvzpEmitterInitialize(mSystemCenter.x, mSystemCenter.y, mParticleSystem, aDef);
	ParticleEmitterID aEmitterID = static_cast<ParticleEmitterID>(mParticleSystem->mParticleHolder->mEmitters.DataArrayGetID(aEmitter));
	mParticleSystem->mEmitterList.AddTail(aEmitterID);
	return CrossFadeParticle(theParticle, aEmitter);
}

bool PvzpParticleEmitter::UpdateParticle(PvzpParticle* theParticle)
{
	if (theParticle->mParticleAge >= theParticle->mParticleDuration)  // particle reached the end of its lifetime
	{
		if (TestBit(mEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_PARTICLE_LOOPS)))
			theParticle->mParticleAge = 0;
		else if (theParticle->mCrossFadeDuration > 0)
			theParticle->mParticleAge = theParticle->mParticleDuration - 1;  // hold the particle on its last frame
		else if (*mEmitterDef->mOnDuration == '\0' || !CrossFadeParticleToName(theParticle, mEmitterDef->mOnDuration))
			return false;
	}
	if (theParticle->mCrossFadeParticleID != ParticleID::PARTICLEID_NULL &&
		mParticleSystem->mParticleHolder->mParticles.DataArrayTryToGet(theParticle->mCrossFadeParticleID) == nullptr)
		return false;  // the cross-fade source is gone; the particle can be deleted

	theParticle->mParticleTimeValue = theParticle->mParticleAge / (static_cast<float>(theParticle->mParticleDuration) - 1);
	for (int i = 0; i < mEmitterDef->mParticleFields.count; i++)
		UpdateParticleField(theParticle, &mEmitterDef->mParticleFields.Fields[i], theParticle->mParticleTimeValue, i);
	theParticle->mPosition += theParticle->mVelocity;
	float aSpinSpeed = ParticleTrackEvaluate(mEmitterDef->mParticleSpinSpeed, theParticle, ParticleTracks::TRACK_PARTICLE_SPIN_SPEED) * 0.01;
	float aSpinAngle = ParticleTrackEvaluate(mEmitterDef->mParticleSpinAngle, theParticle, ParticleTracks::TRACK_PARTICLE_SPIN_ANGLE);
	float aLastSpinAngle = FloatTrackEvaluateFromLastTime(
		mEmitterDef->mParticleSpinAngle, theParticle->mParticleLastTimeValue, theParticle->mParticleInterp[ParticleTracks::TRACK_PARTICLE_SPIN_ANGLE]);
	theParticle->mSpinPosition += DEG_TO_RAD(aSpinSpeed + aSpinAngle - aLastSpinAngle) + theParticle->mSpinVelocity;

	if (FloatTrackIsSet(mEmitterDef->mAnimationRate))
	{
		float aAnimTime = ParticleTrackEvaluate(mEmitterDef->mAnimationRate, theParticle, ParticleTracks::TRACK_PARTICLE_ANIMATION_RATE) * 0.01;
		theParticle->mAnimationTimeValue += aAnimTime;
		while (theParticle->mAnimationTimeValue >= 1.0f)
			theParticle->mAnimationTimeValue -= 1.0f;
		while (theParticle->mAnimationTimeValue < 0.0f)
			theParticle->mAnimationTimeValue += 1.0f;
	}

	theParticle->mParticleAge++;
	theParticle->mParticleLastTimeValue = theParticle->mParticleTimeValue;
	return true;
}

void PvzpParticleEmitter::UpdateSpawning()
{
	PvzpParticleEmitter* aCrossFadeEmitter = mParticleSystem->mParticleHolder->mEmitters.DataArrayTryToGet(static_cast<unsigned int>(mCrossFadeEmitterID));
	PvzpParticleEmitter* aSpawningEmitter = !aCrossFadeEmitter ? this : aCrossFadeEmitter;  // all spawn data is taken from this "primary" emitter
	mSpawnAccum += aSpawningEmitter->SystemTrackEvaluate(aSpawningEmitter->mEmitterDef->mSpawnRate, ParticleSystemTracks::TRACK_SPAWN_RATE) * 0.01;
	int aSpawnCount = static_cast<int>(mSpawnAccum);
	mSpawnAccum -= aSpawnCount;

	int aSpawnMinActive = static_cast<int>(aSpawningEmitter->SystemTrackEvaluate(aSpawningEmitter->mEmitterDef->mSpawnMinActive, ParticleSystemTracks::TRACK_SPAWN_MIN_ACTIVE));
	if (aSpawnMinActive >= 0 && aSpawnCount < aSpawnMinActive - mParticleList.mSize)
		aSpawnCount = aSpawnMinActive - mParticleList.mSize;  // spawn at least enough to reach aSpawnMinActive
	int aSpawnMaxActive = static_cast<int>(aSpawningEmitter->SystemTrackEvaluate(aSpawningEmitter->mEmitterDef->mSpawnMaxActive, ParticleSystemTracks::TRACK_SPAWN_MAX_ACTIVE));
	if (aSpawnMaxActive >= 0 && aSpawnCount > aSpawnMaxActive - mParticleList.mSize)
		aSpawnCount = aSpawnMaxActive - mParticleList.mSize;  // cap the active count at aSpawnMaxActive
	if (FloatTrackIsSet(aSpawningEmitter->mEmitterDef->mSpawnMaxLaunched))
	{
		int aSpawnMaxLaunched = aSpawningEmitter->SystemTrackEvaluate(aSpawningEmitter->mEmitterDef->mSpawnMaxLaunched, ParticleSystemTracks::TRACK_SPAWN_MAX_LAUNCHED);
		if (aSpawnCount > aSpawnMaxLaunched - mParticlesSpawned)
			aSpawnCount = aSpawnMaxLaunched - mParticlesSpawned;  // cap at the emitter's total launch limit
	}

	for (int i = 0; i < aSpawnCount; i++)
	{
		PvzpParticle* aParticle = SpawnParticle(i, aSpawnCount);
		if (aCrossFadeEmitter != nullptr)
			CrossFadeParticle(aParticle, aCrossFadeEmitter);
	}
}

void PvzpParticleEmitter::DeleteNonCrossFading()
{
	for (PvzpListNode<ParticleID>* aNode = mParticleList.mHead; aNode != nullptr; )
	{
		PvzpListNode<ParticleID>* aNext = aNode->mNext;
		PvzpParticle* aParticle = mParticleSystem->mParticleHolder->mParticles.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
		if (aParticle->mCrossFadeDuration <= 0)
			DeleteParticle(aParticle);
		aNode = aNext;
	}
}

void PvzpParticleEmitter::DeleteAll()
{
	while (mParticleList.mSize != 0)
	{
		ParticleID anId = mParticleList.RemoveHead();
		DataArray<PvzpParticle>& aDataArray = mParticleSystem->mParticleHolder->mParticles;
		aDataArray.DataArrayFree(aDataArray.DataArrayGet(anId));
	}
}

void PvzpParticleSystem::Update()
{
	if (!mDontUpdate)
	{
		bool aEmitterAlive = false;
		for (PvzpListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
		{
			PvzpParticleEmitter* aEmitter = mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
			aEmitter->Update();
			if ((FloatTrackIsSet(aEmitter->mEmitterDef->mCrossFadeDuration) && aEmitter->mParticleList.mSize > 0) || !aEmitter->mDead)
				aEmitterAlive = true;
		}
		if (!aEmitterAlive)
			mDead = true;
	}
}

bool PvzpParticleEmitter::CrossFadeParticle(PvzpParticle* theParticle, PvzpParticleEmitter* theToEmitter)
{
	if (theParticle->mCrossFadeDuration > 0)
	{
		PvzpTrace("We don't support cross fading more than one at a time\n");
		return false;
	}
	if (!FloatTrackIsSet(theToEmitter->mEmitterDef->mCrossFadeDuration))
	{
		PvzpTrace("Can't cross fade to emitter that doesn't have CrossFadeDuration");
		return false;
	}
	PVZP_ASSERT(theToEmitter != this);

	PvzpParticle* aToParticle = theToEmitter->SpawnParticle(0, 1);
	if (aToParticle == nullptr)
		return false;
	if (mEmitterCrossFadeCountDown > 0)
		theParticle->mCrossFadeDuration = mEmitterCrossFadeCountDown;  // inherit the source emitter's remaining cross-fade time
	else
	{
		float aCrossFadeDurationInterp = Sexy::Rand(1);
		int aCrossFadeDuration = FloatTrackEvaluate(theToEmitter->mEmitterDef->mCrossFadeDuration, mSystemTimeValue, aCrossFadeDurationInterp);
		theParticle->mCrossFadeDuration = std::max(1, aCrossFadeDuration);  // random cross-fade duration, at least 1 frame
	}
	if (!FloatTrackIsSet(theToEmitter->mEmitterDef->mParticleDuration))
		aToParticle->mParticleDuration = theParticle->mCrossFadeDuration;
	aToParticle->mCrossFadeParticleID = static_cast<ParticleID>(mParticleSystem->mParticleHolder->mParticles.DataArrayGetID(theParticle));
	return true;
}

void PvzpParticleEmitter::DeleteParticle(PvzpParticle* theParticle)
{
	PvzpParticle* aCrossFadeParticle = mParticleSystem->mParticleHolder->mParticles.DataArrayTryToGet(static_cast<unsigned int>(theParticle->mCrossFadeParticleID));
	if (aCrossFadeParticle != nullptr)
	{
		aCrossFadeParticle->mParticleEmitter->DeleteParticle(aCrossFadeParticle);  // also delete the cross-fade source particle
		theParticle->mCrossFadeParticleID = ParticleID::PARTICLEID_NULL;
	}

	ParticleID aParticleID = static_cast<ParticleID>(mParticleSystem->mParticleHolder->mParticles.DataArrayGetID(theParticle));
	mParticleList.RemoveAt(mParticleList.Find(aParticleID));
	mParticleSystem->mParticleHolder->mParticles.DataArrayFree(theParticle);
}

void PvzpParticleEmitter::Update()
{
	if (mDead)
		return;

	mSystemAge++;
	bool aDie = false;
	if (mSystemAge >= mSystemDuration)  // emitter reached the end of its lifetime
	{
		if (TestBit(mEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_SYSTEM_LOOPS)))
			mSystemAge = 0;
		else
		{
			mSystemAge = mSystemDuration - 1;  // hold the emitter on its last frame
			aDie = true;
		}
	}

	if (mEmitterCrossFadeCountDown > 0)
	{
		mEmitterCrossFadeCountDown--;
		if (mEmitterCrossFadeCountDown == 0)
			aDie = true;
	}
	if (mCrossFadeEmitterID != ParticleEmitterID::PARTICLEEMITTERID_NULL)
	{
		PvzpParticleEmitter* aCrossFadeEmitter = mParticleSystem->mParticleHolder->mEmitters.DataArrayTryToGet(mCrossFadeEmitterID);
		if (aCrossFadeEmitter == nullptr || aCrossFadeEmitter->mDead)
			aDie = true;
	}

	mSystemTimeValue = mSystemAge / static_cast<float>(mSystemDuration - 1);
	for (int i = 0; i < mEmitterDef->mSystemFields.count; i++)
		UpdateSystemField(&mEmitterDef->mSystemFields.Fields[i], mSystemTimeValue, i);
	for (PvzpListNode<ParticleID>* aNode = mParticleList.mHead; aNode != nullptr; )
	{
		PvzpListNode<ParticleID>* aNext = aNode->mNext;
		PvzpParticle* aParticle = mParticleSystem->mParticleHolder->mParticles.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
		if (!UpdateParticle(aParticle))
			DeleteParticle(aParticle);
		aNode = aNext;
	}
	UpdateSpawning();

	if (aDie)
	{
		DeleteNonCrossFading();
		if (mParticleList.mSize == 0)
		{
			mDead = true;
			return;
		}
	}
	mSystemLastTimeValue = mSystemTimeValue;
}

float CrossFadeLerp(float theFrom, float theTo, bool theFromIsSet, bool theToIsSet, float theFraction)
{
	if (!theFromIsSet)
		return theTo;
	if (!theToIsSet)
		return theFrom;
	return theFrom + (theTo - theFrom) * theFraction;
}

bool PvzpParticleEmitter::GetRenderParams(PvzpParticle* theParticle, ParticleRenderParams* theParams)
{
	PvzpParticleEmitter* aEmitter = theParticle->mParticleEmitter;
	PvzpEmitterDefinition* aDef = aEmitter->mEmitterDef;

	// Color: a channel counts as set when its system track, particle track, or override is defined
	theParams->mRedIsSet = false;
	theParams->mRedIsSet |= FloatTrackIsSet(aDef->mSystemRed);
	theParams->mRedIsSet |= FloatTrackIsSet(aDef->mParticleRed);
	theParams->mRedIsSet |= aEmitter->mColorOverride.mRed != 1.0f;
	theParams->mGreenIsSet = false;
	theParams->mGreenIsSet |= FloatTrackIsSet(aDef->mSystemGreen);
	theParams->mGreenIsSet |= FloatTrackIsSet(aDef->mParticleGreen);
	theParams->mGreenIsSet |= aEmitter->mColorOverride.mGreen != 1.0f;
	theParams->mBlueIsSet = false;
	theParams->mBlueIsSet |= FloatTrackIsSet(aDef->mSystemBlue);
	theParams->mBlueIsSet |= FloatTrackIsSet(aDef->mParticleBlue);
	theParams->mBlueIsSet |= aEmitter->mColorOverride.mBlue != 1.0f;
	theParams->mAlphaIsSet = false;
	theParams->mAlphaIsSet |= FloatTrackIsSet(aDef->mSystemAlpha);
	theParams->mAlphaIsSet |= FloatTrackIsSet(aDef->mParticleAlpha);
	theParams->mAlphaIsSet |= aEmitter->mColorOverride.mAlpha != 1.0f;
	theParams->mParticleScaleIsSet = false;
	theParams->mParticleScaleIsSet |= FloatTrackIsSet(aDef->mParticleScale);
	theParams->mParticleScaleIsSet |= (aEmitter->mScaleOverride != 1.0f);
	theParams->mParticleStretchIsSet = FloatTrackIsSet(aDef->mParticleStretch);
	// Spin: also counts as set when using a random or launch-aligned initial spin
	theParams->mSpinPositionIsSet = false;
	theParams->mSpinPositionIsSet |= FloatTrackIsSet(aDef->mParticleSpinSpeed);
	theParams->mSpinPositionIsSet |= FloatTrackIsSet(aDef->mParticleSpinAngle);
	theParams->mSpinPositionIsSet |= TestBit(aDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_RANDOM_LAUNCH_SPIN));
	theParams->mSpinPositionIsSet |= TestBit(aDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_ALIGN_LAUNCH_SPIN));
	theParams->mPositionIsSet = false;
	theParams->mPositionIsSet |= (aDef->mParticleFields.count > 0.0f);
	theParams->mPositionIsSet |= FloatTrackIsSet(aDef->mEmitterRadius);
	theParams->mPositionIsSet |= FloatTrackIsSet(aDef->mEmitterOffsetX);
	theParams->mPositionIsSet |= FloatTrackIsSet(aDef->mEmitterOffsetY);
	theParams->mPositionIsSet |= FloatTrackIsSet(aDef->mEmitterBoxX);
	theParams->mPositionIsSet |= FloatTrackIsSet(aDef->mEmitterBoxY);

	float aSystemRed = aEmitter->SystemTrackEvaluate(aDef->mSystemRed, ParticleSystemTracks::TRACK_SYSTEM_RED);
	float aSystemGreen = aEmitter->SystemTrackEvaluate(aDef->mSystemGreen, ParticleSystemTracks::TRACK_SYSTEM_GREEN);
	float aSystemBlue = aEmitter->SystemTrackEvaluate(aDef->mSystemBlue, ParticleSystemTracks::TRACK_SYSTEM_BLUE);
	float aSystemAlpha = aEmitter->SystemTrackEvaluate(aDef->mSystemAlpha, ParticleSystemTracks::TRACK_SYSTEM_ALPHA);
	float aSystemBrightness = aEmitter->SystemTrackEvaluate(aDef->mSystemBrightness, ParticleSystemTracks::TRACK_SYSTEM_BRIGHTNESS);
	float aParticleRed = aEmitter->ParticleTrackEvaluate(aDef->mParticleRed, theParticle, ParticleTracks::TRACK_PARTICLE_RED);
	float aParticleGreen = aEmitter->ParticleTrackEvaluate(aDef->mParticleGreen, theParticle, ParticleTracks::TRACK_PARTICLE_GREEN);
	float aParticleBlue = aEmitter->ParticleTrackEvaluate(aDef->mParticleBlue, theParticle, ParticleTracks::TRACK_PARTICLE_BLUE);
	float aParticleAlpha = aEmitter->ParticleTrackEvaluate(aDef->mParticleAlpha, theParticle, ParticleTracks::TRACK_PARTICLE_ALPHA);
	float aParticleBrightness = aEmitter->ParticleTrackEvaluate(aDef->mParticleBrightness, theParticle, ParticleTracks::TRACK_PARTICLE_BRIGHTNESS);
	float aBrightness = aParticleBrightness * aSystemBrightness;
	// final color = particle color * system color * override color * brightness
	theParams->mRed = aParticleRed * aSystemRed * aEmitter->mColorOverride.mRed * aBrightness;
	theParams->mGreen = aParticleGreen * aSystemGreen * aEmitter->mColorOverride.mGreen * aBrightness;
	theParams->mBlue = aParticleBlue * aSystemBlue * aEmitter->mColorOverride.mBlue * aBrightness;
	theParams->mAlpha = aParticleAlpha * aSystemAlpha * aEmitter->mColorOverride.mAlpha * aBrightness;
	theParams->mPosX = theParticle->mPosition.x;
	theParams->mPosY = theParticle->mPosition.y;
	float aParticleScale = aEmitter->ParticleTrackEvaluate(aDef->mParticleScale, theParticle, ParticleTracks::TRACK_PARTICLE_SCALE);
	theParams->mParticleStretch = aEmitter->ParticleTrackEvaluate(aDef->mParticleStretch, theParticle, ParticleTracks::TRACK_PARTICLE_STRETCH);
	theParams->mParticleScale = aParticleScale * aEmitter->mScaleOverride;
	theParams->mSpinPosition = theParticle->mSpinPosition;

	PvzpParticle* aCrossFadeParticle = aEmitter->mParticleSystem->mParticleHolder->mParticles.DataArrayTryToGet(static_cast<unsigned int>(theParticle->mCrossFadeParticleID));
	if (aCrossFadeParticle != nullptr)  // blend render params with the cross-fade source (from aCrossFadeParticle to theParticle)
	{
		ParticleRenderParams aCrossFadeParams;
		if (PvzpParticleEmitter::GetRenderParams(aCrossFadeParticle, &aCrossFadeParams))
		{
			float aFraction = theParticle->mParticleAge / static_cast<float>(aCrossFadeParticle->mCrossFadeDuration - 1);
			theParams->mRed = CrossFadeLerp(aCrossFadeParams.mRed, theParams->mRed, aCrossFadeParams.mRedIsSet, theParams->mRedIsSet, aFraction);
			theParams->mGreen = CrossFadeLerp(aCrossFadeParams.mGreen, theParams->mGreen, aCrossFadeParams.mGreenIsSet, theParams->mGreenIsSet, aFraction);
			theParams->mBlue = CrossFadeLerp(aCrossFadeParams.mBlue, theParams->mBlue, aCrossFadeParams.mBlueIsSet, theParams->mBlueIsSet, aFraction);
			theParams->mAlpha = CrossFadeLerp(aCrossFadeParams.mAlpha, theParams->mAlpha, aCrossFadeParams.mAlphaIsSet, theParams->mAlphaIsSet, aFraction);
			theParams->mParticleScale = CrossFadeLerp(
				aCrossFadeParams.mParticleScale, theParams->mParticleScale, aCrossFadeParams.mParticleScaleIsSet, theParams->mParticleScaleIsSet, aFraction);
			theParams->mParticleStretch = CrossFadeLerp(
				aCrossFadeParams.mParticleStretch, theParams->mParticleStretch, aCrossFadeParams.mParticleStretchIsSet, theParams->mParticleStretchIsSet, aFraction);
			theParams->mSpinPosition = CrossFadeLerp(
				aCrossFadeParams.mSpinPosition, theParams->mSpinPosition, aCrossFadeParams.mSpinPositionIsSet, theParams->mSpinPositionIsSet, aFraction);
			theParams->mPosX = CrossFadeLerp(aCrossFadeParams.mPosX, theParams->mPosX, aCrossFadeParams.mPositionIsSet, theParams->mPositionIsSet, aFraction);
			theParams->mPosY = CrossFadeLerp(aCrossFadeParams.mPosY, theParams->mPosY, aCrossFadeParams.mPositionIsSet, theParams->mPositionIsSet, aFraction);
			// a field set on the source also counts as set on this particle
			theParams->mRedIsSet |= aCrossFadeParams.mRedIsSet;
			theParams->mGreenIsSet |= aCrossFadeParams.mGreenIsSet;
			theParams->mBlueIsSet |= aCrossFadeParams.mBlueIsSet;
			theParams->mAlphaIsSet |= aCrossFadeParams.mAlphaIsSet;
			theParams->mParticleScaleIsSet |= aCrossFadeParams.mParticleScaleIsSet;
			theParams->mParticleStretchIsSet |= aCrossFadeParams.mParticleStretchIsSet;
			theParams->mSpinPositionIsSet |= aCrossFadeParams.mSpinPositionIsSet;
			theParams->mPositionIsSet |= aCrossFadeParams.mPositionIsSet;
		}
	}
	return true;
}

void RenderParticle(Graphics* g, PvzpParticle* theParticle, const Color& theColor, ParticleRenderParams* theParams, PvzpTriangleGroup* theTriangleGroup)
{
	PvzpParticleEmitter* aEmitter = theParticle->mParticleEmitter;
	PvzpEmitterDefinition* aEmitterDef = aEmitter->mEmitterDef;
	Image* aImage = aEmitter->mImageOverride != nullptr ? aEmitter->mImageOverride : aEmitterDef->mImage;
	if (aImage == nullptr)
		return;

	int aCelWidth = aImage->GetCelWidth();
	int aCelHeight = aImage->GetCelHeight();
	int aFrame = aEmitter->mFrameOverride;
	if (aFrame == -1)
	{
		if (FloatTrackIsSet(aEmitterDef->mAnimationRate))
			aFrame = std::clamp(static_cast<int>(theParticle->mAnimationTimeValue * aEmitterDef->mImageFrames), 0, aEmitterDef->mImageFrames - 1);
		else if (aEmitterDef->mAnimated)
			aFrame = std::clamp(static_cast<int>(theParticle->mParticleTimeValue * aEmitterDef->mImageFrames), 0, aEmitterDef->mImageFrames - 1);
		else
			aFrame = theParticle->mImageFrame;
	}
	aFrame += aEmitterDef->mImageCol;
	if (aFrame >= aImage->mNumCols)
		aFrame = aImage->mNumCols - 1;

	Rect aSrcRect(aFrame * aCelWidth, std::min(aEmitterDef->mImageRow, aImage->mNumRows - 1) * aCelHeight, aCelWidth, aCelHeight);
	float aClipTop = PvzpParticleEmitter::ParticleTrackEvaluate(aEmitterDef->mClipTop, theParticle, ParticleTracks::TRACK_PARTICLE_CLIP_TOP);
	float aClipBottom = PvzpParticleEmitter::ParticleTrackEvaluate(aEmitterDef->mClipBottom, theParticle, ParticleTracks::TRACK_PARTICLE_CLIP_BOTTOM);
	float aClipLeft = PvzpParticleEmitter::ParticleTrackEvaluate(aEmitterDef->mClipLeft, theParticle, ParticleTracks::TRACK_PARTICLE_CLIP_LEFT);
	float aClipRight = PvzpParticleEmitter::ParticleTrackEvaluate(aEmitterDef->mClipRight, theParticle, ParticleTracks::TRACK_PARTICLE_CLIP_RIGHT);
	PVZP_ASSERT(aClipTop >= 0.0f && aClipTop <= 1.0f);
	PVZP_ASSERT(aClipBottom >= 0.0f && aClipBottom <= 1.0f);
	PVZP_ASSERT(aClipLeft >= 0.0f && aClipLeft <= 1.0f);
	PVZP_ASSERT(aClipRight >= 0.0f && aClipRight <= 1.0f);
	theParams->mPosX += aClipLeft * aCelWidth;
	theParams->mPosY += aClipTop * aCelHeight;
	aSrcRect.mX += FloatRoundToInt(aClipLeft * aCelWidth);
	aSrcRect.mY += FloatRoundToInt(aClipTop * aCelHeight);
	aSrcRect.mWidth -= FloatRoundToInt(aCelWidth * (aClipLeft + aClipRight));
	aSrcRect.mHeight -= FloatRoundToInt(aCelHeight * (aClipBottom + aClipTop));  // adjust the source rect by the clip ratio of each side
	PVZP_ASSERT(aSrcRect.mX == aCelWidth * aFrame + FloatRoundToInt(aClipLeft * aCelWidth));
	PVZP_ASSERT(aSrcRect.mY == aCelHeight * aEmitterDef->mImageRow + FloatRoundToInt(aClipTop * aCelHeight));
	PVZP_ASSERT(aSrcRect.mX >= 0 && aSrcRect.mX < 10000);
	PVZP_ASSERT(aSrcRect.mY >= 0 && aSrcRect.mY < 10000);

	if (TestBit(aEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_ALIGN_TO_PIXELS)))
	{
		theParams->mPosX = FloatRoundToInt(theParams->mPosX);
		theParams->mPosY = FloatRoundToInt(theParams->mPosY);
	}
	int aDrawMode = g->mDrawMode;
	if (TestBit(aEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_ADDITIVE)))
		aDrawMode = Graphics::DRAWMODE_ADDITIVE;
	if (TestBit(aEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_FULLSCREEN)))
	{
		theTriangleGroup->DrawGroup(g);
		Color anOldColor = g->GetColor();
		int anOldDrawMode = g->GetDrawMode();
		g->SetColor(theColor);
		g->FillRect(-g->mTransX, -g->mTransY, BOARD_WIDTH, BOARD_HEIGHT);
		g->SetColor(anOldColor);
		g->SetDrawMode(anOldDrawMode);
	}
	else
	{
		SexyMatrix3 aTransform;
		PvzpScaleRotateTransformMatrix(
			aTransform,
			theParams->mPosX,
			theParams->mPosY,
			theParams->mSpinPosition,
			theParams->mParticleScale,
			theParams->mParticleStretch * theParams->mParticleScale
		);
		theTriangleGroup->AddTriangle(g, aImage, aTransform, g->mClipRect, theColor, aDrawMode, aSrcRect);
		if (aEmitter->mExtraAdditiveDrawOverride)
			theTriangleGroup->AddTriangle(g, aImage, aTransform, g->mClipRect, theColor, Graphics::DRAWMODE_ADDITIVE, aSrcRect);
	}
}

void PvzpParticleEmitter::DrawParticle(Graphics* g, PvzpParticle* theParticle, PvzpTriangleGroup* theTriangleGroup)
{
	if (theParticle->mCrossFadeDuration > 0)  // cross-fade source particles are not drawn
		return;

	ParticleRenderParams aParams;
	if (GetRenderParams(theParticle, &aParams))
	{
		Color aColor(
			std::clamp(FloatRoundToInt(aParams.mRed), 0, 255),
			std::clamp(FloatRoundToInt(aParams.mGreen), 0, 255),
			std::clamp(FloatRoundToInt(aParams.mBlue), 0, 255),
			std::clamp(FloatRoundToInt(aParams.mAlpha), 0, 255)
		);
		if (aColor.mAlpha > 0)
		{
			aParams.mPosX += g->mTransX;
			aParams.mPosY += g->mTransY;

			PvzpParticle* aParticle;
			if (mImageOverride || mEmitterDef->mImage)
				aParticle = theParticle;
			else  // no image of its own: try the cross-fade source particle
				aParticle = mParticleSystem->mParticleHolder->mParticles.DataArrayTryToGet(static_cast<unsigned int>(theParticle->mCrossFadeParticleID));
			if (aParticle != nullptr)
				RenderParticle(g, aParticle, aColor, &aParams, theTriangleGroup);
		}
	}
}

void PvzpParticleSystem::Draw(Graphics* g)
{
	for (PvzpListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
		mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue))->Draw(g);
}

void PvzpParticleEmitter::Draw(Graphics* g)
{
	bool aHardWare = gSexyAppBase->Is3DAccelerated();
	if ((TestBit(mEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_SOFTWARE_ONLY)) && aHardWare) ||
		(TestBit(mEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_HARDWARE_ONLY)) && !aHardWare))
		return;

	PvzpTriangleGroup aTriangleGroup;
	for (PvzpListNode<ParticleID>* aNode = mParticleList.mHead; aNode != nullptr; aNode = aNode->mNext)
		DrawParticle(g, mParticleSystem->mParticleHolder->mParticles.DataArrayGet(static_cast<unsigned int>(aNode->mValue)), &aTriangleGroup);
	aTriangleGroup.DrawGroup(g);
}

void PvzpParticleSystem::SystemMove(float theX, float theY)
{
	for (PvzpListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
		mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue))->SystemMove(theX, theY);
}

void PvzpParticleEmitter::SystemMove(float theX, float theY)
{
	float aDeltaX = theX - mSystemCenter.x;
	float aDeltaY = theY - mSystemCenter.y;
	if (FloatApproxEqual(aDeltaX, 0.0f) && FloatApproxEqual(aDeltaY, 0.0f))
		return;

	mSystemCenter.x = theX;
	mSystemCenter.y = theY;
	if (!TestBit(mEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_PARTICLES_DONT_FOLLOW)))
	{
		for (PvzpListNode<ParticleID>* aNode = mParticleList.mHead; aNode != nullptr; aNode = aNode->mNext)
		{
			PvzpParticle* aParticle = mParticleSystem->mParticleHolder->mParticles.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
			aParticle->mPosition.x += aDeltaX;
			aParticle->mPosition.y += aDeltaY;
		}
	}
}

void PvzpParticleSystem::OverrideColor(const char* theEmitterName, const Color& theColor)
{
	for (PvzpListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		PvzpParticleEmitter* aEmitter = mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
		if (theEmitterName == nullptr || strcasecmp(theEmitterName, aEmitter->mEmitterDef->mName) == 0)
			aEmitter->mColorOverride = theColor;
	}
}

void PvzpParticleSystem::OverrideExtraAdditiveDraw(const char* theEmitterName, bool theEnableExtraAdditiveDraw)
{
	for (PvzpListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		PvzpParticleEmitter* aEmitter = mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
		if (theEmitterName == nullptr || strcasecmp(theEmitterName, aEmitter->mEmitterDef->mName) == 0)
			aEmitter->mExtraAdditiveDrawOverride = theEnableExtraAdditiveDraw;
	}
}

void PvzpParticleSystem::OverrideImage(const char* theEmitterName, Image* theImage)
{
	for (PvzpListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		PvzpParticleEmitter* aEmitter = mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
		if (theEmitterName == nullptr || strcasecmp(theEmitterName, aEmitter->mEmitterDef->mName) == 0)
			aEmitter->mImageOverride = theImage;
	}
}

void PvzpParticleSystem::OverrideFrame(const char* theEmitterName, int theFrame)
{
	for (PvzpListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		PvzpParticleEmitter* aEmitter = mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
		if (theEmitterName == nullptr || strcasecmp(theEmitterName, aEmitter->mEmitterDef->mName) == 0)
			aEmitter->mFrameOverride = theFrame;
	}
}

void PvzpParticleSystem::OverrideScale(const char* theEmitterName, float theScale)
{
	for (PvzpListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		PvzpParticleEmitter* aEmitter = mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
		if (theEmitterName == nullptr || strcasecmp(theEmitterName, aEmitter->mEmitterDef->mName) == 0)
			aEmitter->mScaleOverride = theScale;
	}
}

PvzpParticleEmitter* PvzpParticleSystem::FindEmitterByName(const char* theEmitterName)
{
	for (PvzpListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		PvzpParticleEmitter* aEmitter = mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
		if (strcasecmp(theEmitterName, aEmitter->mEmitterDef->mName) == 0)
			return aEmitter;
	}
	return nullptr;
}

PvzpEmitterDefinition* PvzpParticleSystem::FindEmitterDefByName(const char* theEmitterName)
{
	for (int i = 0; i < mParticleDef->mEmitterDefCount; i++)
	{
		PvzpEmitterDefinition* aEmitterDef = &mParticleDef->mEmitterDefs[i];
		if (strcasecmp(theEmitterName, aEmitterDef->mName) == 0)
			return aEmitterDef;
	}
	return nullptr;
}

void PvzpParticleEmitter::CrossFadeEmitter(PvzpParticleEmitter* theToEmitter)
{
	if (mEmitterCrossFadeCountDown > 0)
	{
		PvzpTrace("We don't support cross fading emitters more than one at a time\n");
		return;
	}
	if (!FloatTrackIsSet(theToEmitter->mEmitterDef->mCrossFadeDuration))
	{
		PvzpTrace("Can't cross fade to emitter that doesn't have CrossFadeDuration");
		return;
	}
	PVZP_ASSERT(theToEmitter != this);

	float aCrossFadeDurationInterp = Sexy::Rand(1.0f);
	mEmitterCrossFadeCountDown = FloatTrackEvaluate(theToEmitter->mEmitterDef->mCrossFadeDuration, mSystemTimeValue, aCrossFadeDurationInterp);
	mEmitterCrossFadeCountDown = std::max(1, mEmitterCrossFadeCountDown);
	mCrossFadeEmitterID = static_cast<ParticleEmitterID>(mParticleSystem->mParticleHolder->mEmitters.DataArrayGetID(theToEmitter));
	if (!FloatTrackIsSet(theToEmitter->mEmitterDef->mSystemDuration))
		theToEmitter->mSystemDuration = mEmitterCrossFadeCountDown;

	for (PvzpListNode<ParticleID>* aNode = mParticleList.mHead; aNode != nullptr; aNode = aNode->mNext)
		CrossFadeParticle(mParticleSystem->mParticleHolder->mParticles.DataArrayGet(static_cast<unsigned int>(aNode->mValue)), theToEmitter);
}

void PvzpParticleSystem::CrossFade(const char* theEmitterName)
{
	PvzpEmitterDefinition* aEmitterDef = FindEmitterDefByName(theEmitterName);
	if (aEmitterDef == nullptr)
	{
		PvzpTrace("Can't find cross fade emitter: %s\n", theEmitterName);
		return;
	}
	if (!FloatTrackIsSet(aEmitterDef->mCrossFadeDuration))
	{
		PvzpTrace("Can't cross fade without duration set: %s\n", theEmitterName);
		return;
	}
	if (mParticleHolder->mEmitters.mSize + mEmitterList.mSize > mParticleHolder->mEmitters.mMaxSize)
	{
		PvzpTrace("Too many emitters to cross fade\n");
		ParticleSystemDie();
		return;
	}

	for (PvzpListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		PvzpParticleEmitter* aEmitter = mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
		if (aEmitter->mEmitterDef != aEmitterDef)  // don't cross fade between emitters of the same kind
		{
			PvzpParticleEmitter* aCrossFadeEmitter = mParticleHolder->mEmitters.DataArrayAlloc();
			aCrossFadeEmitter->PvzpEmitterInitialize(aEmitter->mSystemCenter.x, aEmitter->mSystemCenter.y, this, aEmitterDef);
			ParticleEmitterID aCrossFadeEmitterID = static_cast<ParticleEmitterID>(mParticleHolder->mEmitters.DataArrayGetID(aCrossFadeEmitter));
			mEmitterList.AddTail(aCrossFadeEmitterID);
			aEmitter->CrossFadeEmitter(aCrossFadeEmitter);
		}
	}
}

PvzpParticleHolder::~PvzpParticleHolder()
{
	DisposeHolder();
}

void PvzpParticleHolder::InitializeHolder()
{
	mParticleSystems.DataArrayInitialize(1024U, "particle systems");
	mEmitters.DataArrayInitialize(1024U, "emitters");
	mParticles.DataArrayInitialize(1024U, "particles");
	mParticleListNodeAllocator.Initialize(1024, sizeof(PvzpListNode<ParticleID>));
	mEmitterListNodeAllocator.Initialize(1024, sizeof(PvzpListNode<ParticleEmitterID>));
}

void PvzpParticleHolder::DisposeHolder()
{
	mParticleSystems.DataArrayDispose();
	mEmitters.DataArrayDispose();
	mParticles.DataArrayDispose();
	mParticleListNodeAllocator.FreeAll();
	mEmitterListNodeAllocator.FreeAll();
}

bool PvzpParticleHolder::IsOverLoaded()
{
	return mParticleSystems.mSize > MAX_PARTICLES_SIZE || mEmitters.mSize > MAX_PARTICLES_SIZE || mParticles.mSize > MAX_PARTICLES_SIZE;
}

PvzpParticleSystem* PvzpParticleHolder::AllocParticleSystemFromDef(float theX, float theY, int theRenderOrder, PvzpParticleDefinition* theDefinition, ParticleEffect theParticleEffect)
{
	if (mParticleSystems.mSize == mParticleSystems.mMaxSize)
	{
		PvzpTrace("Too many particle systems\n");
		return nullptr;
	}
	if (theDefinition->mEmitterDefCount + mEmitters.mSize > mEmitters.mMaxSize)
	{
		PvzpTrace("Too many particle emitters\n");
		return nullptr;
	}

	PvzpParticleSystem* aPvzpParticle = mParticleSystems.DataArrayAlloc();
	aPvzpParticle->mParticleHolder = this;
	aPvzpParticle->PvzpParticleInitializeFromDef(theX, theY, theRenderOrder, theDefinition, theParticleEffect);
	return aPvzpParticle;
}

PvzpParticleSystem* PvzpParticleHolder::AllocParticleSystem(float theX, float theY, int theRenderOrder, ParticleEffect theParticleEffect)
{
	PVZP_ASSERT(static_cast<int>(theParticleEffect) >= 0 && static_cast<int>(theParticleEffect) < gParticleDefCount);
	return AllocParticleSystemFromDef(theX, theY, theRenderOrder, &gParticleDefArray[theParticleEffect], theParticleEffect);
}
