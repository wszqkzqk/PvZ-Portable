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

#include "TodDebug.h"
#include "Definition.h"
#include "TodParticle.h"
#include "EffectSystem.h"
#include "../GameConstants.h"
#include "graphics/Graphics.h"
#include "graphics/GLInterface.h"

int gParticleDefCount;                      // [0x6A9F08]
TodParticleDefinition* gParticleDefArray;   // [0x6A9F0C]
int gParticleParamArraySize;				// [0x6A9F10]
const ParticleParams* gParticleParamArray;		// [0x6A9F14]

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

// (ecx = *theParticleFileName, *theParticleDef)  //esp -= 4
bool TodParticleLoadADef(TodParticleDefinition* theParticleDef, const char* theParticleFileName)
{
	TodHesitationBracket("Load Particle %s", theParticleFileName);
	if (!DefinitionLoadXML(theParticleFileName, &gParticleDefMap, theParticleDef))
	{
		char aBuf[512];
		snprintf(aBuf, sizeof(aBuf), "Failed to load particle '%s'", theParticleFileName);
		TodErrorMessageBox(aBuf, "Error");
		return false;
	}
	else
	{
		for (int i = 0; i < theParticleDef->mEmitterDefCount; i++)
		{
			TodEmitterDefinition& aDef = theParticleDef->mEmitterDefs[i];
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

void TodParticleLoadDefinitions(const ParticleParams* theParticleParamArray, int theParticleParamArraySize)
{
	TodHesitationBracket aHesitiation("TodParticleLoadDefinitions");
	TOD_ASSERT(!gParticleParamArray && !gParticleDefArray);
	gParticleParamArraySize = theParticleParamArraySize;
	gParticleParamArray = theParticleParamArray;
	gParticleDefCount = theParticleParamArraySize;
	gParticleDefArray = new TodParticleDefinition[theParticleParamArraySize];
	// This was uninitialised before!
	// memset(gParticleDefArray, 0, theParticleParamArraySize*sizeof(TodParticleDefinition));

	for (int i = 0; i < gParticleParamArraySize; i++)
	{
		const ParticleParams& aParticleParams = gParticleParamArray[i];
		TOD_ASSERT(aParticleParams.mParticleEffect == i);
		if (!TodParticleLoadADef(&gParticleDefArray[i], aParticleParams.mParticleFileName))
		{
			char aBuf[512];
			snprintf(aBuf, sizeof(aBuf), "Failed to load particle '%s'", aParticleParams.mParticleFileName);
			TodErrorMessageBox(aBuf, "Error");
		}
		gSexyAppBase->mCompletedLoadingThreadTasks += 6;
	}
}

void TodParticleFreeDefinitions()
{
	for (int i = 0; i < gParticleDefCount; i++)
		DefinitionFreeMap(&gParticleDefMap, &gParticleDefArray[i]);
	delete[] gParticleDefArray;
	gParticleDefArray = nullptr;
	gParticleDefCount = 0;
	gParticleParamArray = nullptr;
	gParticleParamArraySize = 0;
}

TodParticleSystem::TodParticleSystem()
{
	mEffectType = ParticleEffect::PARTICLE_NONE;
	mParticleDef = nullptr;
	mParticleHolder = nullptr;
	mDead = false;
	mDontUpdate = false;
	mIsAttachment = false;
	mRenderOrder = 0;
}

TodParticleSystem::~TodParticleSystem()
{
	ParticleSystemDie();
	mEmitterList.RemoveAll();
}

// (edx = theEffectType, *theDefinition, ecx = theRenderOrder, theY, theX, *this)
void TodParticleSystem::TodParticleInitializeFromDef(float theX, float theY, int theRenderOrder, TodParticleDefinition* theDefinition, ParticleEffect theEffectType)
{
	TOD_ASSERT(mParticleHolder);
	mEmitterList.SetAllocator(&mParticleHolder->mEmitterListNodeAllocator);
	mParticleDef = theDefinition;
	mEffectType = theEffectType;
	mRenderOrder = theRenderOrder;

	for (int i = 0; i < theDefinition->mEmitterDefCount; i++)
	{
		TodEmitterDefinition& aDef = theDefinition->mEmitterDefs[i];
		if (!FloatTrackIsSet(aDef.mCrossFadeDuration))
		{
			if (TestBit(aDef.mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_DIE_IF_OVERLOADED)) && mParticleHolder->IsOverLoaded())
			{
				ParticleSystemDie();
				break;
			}
			TodParticleEmitter* aEmitter = mParticleHolder->mEmitters.DataArrayAlloc();
			aEmitter->TodEmitterInitialize(theX, theY, this, &aDef);
			mEmitterList.AddTail(static_cast<ParticleEmitterID>(mParticleHolder->mEmitters.DataArrayGetID(aEmitter)));
		}
	}
}

// (*theEmitterDef, *theSystem, theY, theX, ecx = *this)
void TodParticleEmitter::TodEmitterInitialize(float theX, float theY, TodParticleSystem* theSystem, TodEmitterDefinition* theEmitterDef)
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

void TodParticleSystem::ParticleSystemDie()
{
	for (TodListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		TodParticleEmitter* aEmitter = mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
		aEmitter->DeleteAll();
		mParticleHolder->mEmitters.DataArrayFree(aEmitter);
	}
	mEmitterList.RemoveAll();
	mDead = true;
}

TodParticle* TodParticleEmitter::SpawnParticle(int theIndex, int theSpawnCount)
{
	DataArray<TodParticle>& aDataArray = mParticleSystem->mParticleHolder->mParticles;
	if (aDataArray.mSize == aDataArray.mMaxSize)
	{
		TodTraceWithoutSpamming("Too many particles '%s'\n", mEmitterDef->mName);
		return nullptr;
	}

	TodParticle* aParticle = aDataArray.DataArrayAlloc();
	TOD_ASSERT(mEmitterDef->mParticleFields.count <= MAX_PARTICLE_FIELDS);
	for (int i = 0; i < mEmitterDef->mParticleFields.count; i++)
	{
		aParticle->mParticleFieldInterp[i][0] = Sexy::Rand(1.0f);  // 初始化每个粒子场的横向插值
		aParticle->mParticleFieldInterp[i][1] = Sexy::Rand(1.0f);  // 初始化每个粒子场的纵向插值
	}
	for (int i = 0; i < static_cast<int>(ParticleTracks::NUM_PARTICLE_TRACKS); i++)
		aParticle->mParticleInterp[i] = Sexy::Rand(1.0f);  // 初始化每条通道的插值

	float aParticleDurationInterp = Sexy::Rand(1.0f);
	float aLaunchSpeedInterp = Sexy::Rand(1.0f);
	float aEmitterOffsetXInterp = Sexy::Rand(1.0f);
	float aEmitterOffsetYInterp = Sexy::Rand(1.0f);
	aParticle->mParticleDuration = FloatTrackEvaluate(mEmitterDef->mParticleDuration, mSystemTimeValue, aParticleDurationInterp);
	aParticle->mParticleDuration = std::max(1, aParticle->mParticleDuration);  // 初始化粒子持续时间（至少为 1）
	aParticle->mParticleAge = 0;
	aParticle->mParticleEmitter = this;
	aParticle->mParticleTimeValue = -1.0f;
	aParticle->mParticleLastTimeValue = -1.0f;
	if (TestBit(mEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_RANDOM_START_TIME)))
		aParticle->mParticleAge = Sexy::Rand(aParticle->mParticleDuration);  // 对于“随机初始时间”的粒子
	float aLaunchSpeed = FloatTrackEvaluate(mEmitterDef->mLaunchSpeed, mSystemTimeValue, aLaunchSpeedInterp) * 0.01f;
	float aLaunchAngleInterp = Sexy::Rand(1.0f);

	float aLaunchAngle;
	if (mEmitterDef->mEmitterType == EmitterType::EMITTER_CIRCLE_PATH)
	{
		// 发射角度 = 根据路径定义计算的圆周上的基础角度 + 根据发射角度定义计算的额外偏移的角度
		aLaunchAngle = FloatTrackEvaluate(mEmitterDef->mEmitterPath, mSystemTimeValue, mTrackInterp[ParticleSystemTracks::TRACK_EMITTER_PATH]) * 2 * PI;
		aLaunchAngle += DEG_TO_RAD(FloatTrackEvaluate(mEmitterDef->mLaunchAngle, mSystemTimeValue, aLaunchAngleInterp));
	}
	else if (mEmitterDef->mEmitterType == EmitterType::EMITTER_CIRCLE_EVEN_SPACING)
		// 基础发射角度要使 theSpawnCount 个粒子平均布满圆周
		aLaunchAngle = 2 * PI * theIndex / theSpawnCount + DEG_TO_RAD(FloatTrackEvaluate(mEmitterDef->mLaunchAngle, mSystemTimeValue, aLaunchAngleInterp));
	else if (FloatTrackIsConstantZero(mEmitterDef->mLaunchAngle))
		// 未定义的轨道，发射角度直接取 [0, 2π] 的随机值
		aLaunchAngle = Sexy::Rand(static_cast<float>(2 * PI));
	else
		// 其他情况下，根据发射角度的定义值计算
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
		// ★ 以竖直向下的方向为 0 角度
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
		float aDistanceX = aMaxX - aMinX;  // 路径矩形的横向宽度
		float aDistanceY = aMaxY - aMinY;  // 路径矩形的纵向高度
		float aPathPos = aEmitterPathPosition * (aDistanceY + aDistanceX + aDistanceY + aDistanceX);  // 发射点位于矩形边上的位置
		// ★ 注释规定：以矩形左上的顶点开始，按逆时针方向依次将矩形的四个顶点标记为 A、B、C、D，并标记发射点为 P
		// ★           如此，aPathPos 即为 P 点与 A 点在矩形路径上的距离。注意，游戏中取横向水平向右和纵向竖直向下为正方向。
		if (aPathPos < aDistanceY)  // 发射点落在矩形 AB 边（左边）上
		{
			aPosX = aMinX;  // 横坐标 = 矩形左端横坐标
			aPosY = aMinY + aPathPos;  // 纵坐标 = 矩形底端坐标 + |PA|
		}
		else if (aPathPos < aDistanceY + aDistanceX)  // 发射点落在矩形 BC 边（顶边）上
		{
			aPosX = aMinX + (aPathPos - aDistanceY);  // 横坐标 = 矩形左端横坐标 + |PB|
			aPosY = aMaxY;
		}
		else if (aPathPos < aDistanceY + aDistanceX + aDistanceY)  // 发射点落在矩形 CD 边（右边）上
		{
			aPosX = aMaxX;
			aPosY = aMaxY - (aPathPos - aDistanceY - aDistanceX);  // 纵坐标 = 矩形顶端纵坐标 - |PC|
		}
		else  // 发射点落在矩形 AD 边（底边）上
		{
			aPosX = aMaxX - (aPathPos - aDistanceY - aDistanceX - aDistanceY);  // 横坐标 = 矩形右端横坐标 - |PD|
			aPosY = aMinY;
		}
		break;
	}
	default:
		TOD_ASSERT(false);
		break;
	}
	float aEmitterSkewXInterp = Sexy::Rand(1.0f);
	float aEmitterSkewYInterp = Sexy::Rand(1.0f);
	float aSkewX = FloatTrackEvaluate(mEmitterDef->mEmitterSkewX, mSystemTimeValue, aEmitterSkewXInterp);
	float aSkewY = FloatTrackEvaluate(mEmitterDef->mEmitterSkewY, mSystemTimeValue, aEmitterSkewYInterp);
	aParticle->mPosition.x = mSystemCenter.x + aPosX + aPosY * aSkewX;  // 横向（左右）倾斜的幅度受纵坐标影响
	aParticle->mPosition.y = mSystemCenter.y + aPosY + aPosX * aSkewY;  // 纵向（上下）倾斜的幅度受横坐标影响
	aParticle->mVelocity.x = sin(aLaunchAngle) * aLaunchSpeed;
	aParticle->mVelocity.y = cos(aLaunchAngle) * aLaunchSpeed;
	aParticle->mPosition.x += FloatTrackEvaluate(mEmitterDef->mEmitterOffsetX, mSystemTimeValue, aEmitterOffsetXInterp);  // 横坐标加上横向偏移值
	aParticle->mPosition.y += FloatTrackEvaluate(mEmitterDef->mEmitterOffsetY, mSystemTimeValue, aEmitterOffsetYInterp);  // 纵坐标加上纵向偏移值
	
	aParticle->mAnimationTimeValue = 0.0f;
	if (mEmitterDef->mAnimated || FloatTrackIsSet(mEmitterDef->mAnimationRate))
		aParticle->mImageFrame = 0;  // 如果定义了粒子动态或动画速率，则粒子的当前帧将在后续根据粒子时间值或动画循环率实时计算，此处先初始化为 0
	else
		aParticle->mImageFrame = Sexy::Rand(mEmitterDef->mImageFrames);  // 对于帧固定的粒子，在贴图的所有帧中随机取得一帧，后续一般不再变化

	if (TestBit(mEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_RANDOM_LAUNCH_SPIN)))
		aParticle->mSpinPosition = Sexy::Rand(static_cast<float>(2 * PI));  // 在 [0, 2π] 之间随机取得一个初始旋转角度
	else if (TestBit(mEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_ALIGN_LAUNCH_SPIN)))
		aParticle->mSpinPosition = aLaunchAngle;  // 粒子旋转角度对齐发射角度
	else
		aParticle->mSpinPosition = 0.0f;  // 默认无初始旋转
	aParticle->mSpinVelocity = 0.0f;
	aParticle->mCrossFadeDuration = 0;
	aParticle->mCrossFadeParticleID = ParticleID::PARTICLEID_NULL;

	ParticleID aParticleID = static_cast<ParticleID>(aDataArray.DataArrayGetID(aParticle));
	mParticleList.AddHead(aParticleID);
	mParticlesSpawned++;
	UpdateParticle(aParticle);
	return aParticle;
}

float TodParticleEmitter::ParticleTrackEvaluate(FloatParameterTrack& theTrack, TodParticle* theParticle, ParticleTracks theParticleTrack)
{
	return FloatTrackEvaluate(theTrack, theParticle->mParticleTimeValue, theParticle->mParticleInterp[theParticleTrack]);
}

void TodParticleEmitter::UpdateParticleField(TodParticle* theParticle, ParticleField* theParticleField, float theParticleTimeValue, int theFieldIndex)
{
	TOD_ASSERT(theFieldIndex < MAX_PARTICLE_FIELDS);
	float aInterpX = theParticle->mParticleFieldInterp[theFieldIndex][0];
	float aInterpY = theParticle->mParticleFieldInterp[theFieldIndex][1];
	float x = FloatTrackEvaluate(theParticleField->mX, theParticleTimeValue, aInterpX);
	float y = FloatTrackEvaluate(theParticleField->mY, theParticleTimeValue, aInterpY);

	switch (theParticleField->mFieldType)
	{
	case ParticleFieldType::FIELD_INVALID:
		break;
	case ParticleFieldType::FIELD_FRICTION:  // 摩擦力场
		theParticle->mVelocity.x *= 1 - x;
		theParticle->mVelocity.y *= 1 - y;
		break;
	case ParticleFieldType::FIELD_ACCELERATION:  // 加速度场
		theParticle->mVelocity.x += 0.01f * x;
		theParticle->mVelocity.y += 0.01f * y;
		break;
	case ParticleFieldType::FIELD_ATTRACTOR:  // 弹性力场
	{
		float aDiffX = x - (theParticle->mPosition.x - mSystemCenter.x);
		float aDiffY = y - (theParticle->mPosition.y - mSystemCenter.y);
		// 加速度的方向始终从粒子所在位置指向“标准位置”
		theParticle->mVelocity.x += 0.01f * aDiffX;
		theParticle->mVelocity.y += 0.01f * aDiffY;
		break;
	}
	case ParticleFieldType::FIELD_MAX_VELOCITY:  // 限速场
		theParticle->mVelocity.x = ClampFloat(theParticle->mVelocity.x, -x, x);
		theParticle->mVelocity.y = ClampFloat(theParticle->mVelocity.y, -y, y);
		break;
	case ParticleFieldType::FIELD_VELOCITY:  // 匀速场
		theParticle->mPosition.x += 0.01 * x;
		theParticle->mPosition.y += 0.01 * y;
		break;
	case ParticleFieldType::FIELD_POSITION:  // 定位场
	{
		float aLastX = FloatTrackEvaluateFromLastTime(theParticleField->mX, theParticle->mParticleLastTimeValue, aInterpX);
		float aLastY = FloatTrackEvaluateFromLastTime(theParticleField->mY, theParticle->mParticleLastTimeValue, aInterpY);
		theParticle->mPosition.x += x - aLastX;
		theParticle->mPosition.y += y - aLastY;
		break;
	}
	case ParticleFieldType::FIELD_GROUND_CONSTRAINT:
		if (theParticle->mPosition.y > mSystemCenter.y + y)  // 判断是否触及地面
		{
			theParticle->mPosition.y = mSystemCenter.y + y;  // 将坐标重置至地面
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
	case ParticleFieldType::FIELD_SHAKE:  // 震动
	{
		float aLastX = FloatTrackEvaluateFromLastTime(theParticleField->mX, theParticle->mParticleLastTimeValue, aInterpX);
		float aLastY = FloatTrackEvaluateFromLastTime(theParticleField->mY, theParticle->mParticleLastTimeValue, aInterpY);
		// 先恢复上一次震动效果的影响
		int aLastRandSeed = theParticle->mParticleAge - 1;
		if (aLastRandSeed == -1)
			aLastRandSeed = theParticle->mParticleDuration - 1;
		srand(aLastRandSeed * reinterpret_cast<uintptr_t>(theParticle));
		theParticle->mPosition.x -= aLastX * (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f);
		theParticle->mPosition.y -= aLastY * (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f);
		// 再随机取得当前帧的震动效果
		srand(theParticle->mParticleAge * reinterpret_cast<uintptr_t>(theParticle));
		theParticle->mPosition.x += x * (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f);
		theParticle->mPosition.y += y * (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f);
		break;
	}
	case ParticleFieldType::FIELD_CIRCLE:  // 圆周
	{
		SexyVector2 aToCenter = theParticle->mPosition - mSystemCenter;
		SexyVector2 aMotion = aToCenter.Perp().Normalize();  // 标准化的法向量
		float aRadius = aToCenter.Magnitude();
		aMotion *= 0.01 * (x + aRadius * y);
		theParticle->mPosition += aMotion;
		break;
	}
	case ParticleFieldType::FIELD_AWAY:  // 远离
	{
		SexyVector2 aToCenter = theParticle->mPosition - mSystemCenter;
		SexyVector2 aMotion = aToCenter.Normalize();  // 标准化的方向向量
		float aRadius = aToCenter.Magnitude();
		aMotion *= 0.01 * (x + aRadius * y);
		theParticle->mPosition += aMotion;
		break;
	}
	default:
		TOD_ASSERT(0);
		break;
	}
}

float TodParticleEmitter::SystemTrackEvaluate(FloatParameterTrack& theTrack, ParticleSystemTracks theSystemTrack)
{
	return FloatTrackEvaluate(theTrack, mSystemTimeValue, mTrackInterp[theSystemTrack]);
}

void TodParticleEmitter::UpdateSystemField(ParticleField* theParticleField, float theParticleTimeValue, int theFieldIndex)
{
	TOD_ASSERT(theFieldIndex < MAX_PARTICLE_FIELDS);
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
		TOD_ASSERT(0);
		break;
	}
}

bool TodParticleEmitter::CrossFadeParticleToName(TodParticle* theParticle, const char* theEmitterName)
{
	TodEmitterDefinition* aDef = mParticleSystem->FindEmitterDefByName(theEmitterName);
	if (aDef == nullptr)
	{
		TodTrace("Can't find emitter to cross fade: %s\n", theEmitterName);
		return false;
	}
	if (mParticleSystem->mParticleHolder->mEmitters.mSize == mParticleSystem->mParticleHolder->mEmitters.mMaxSize)
	{
		TodTrace("Too many emitters to cross fade\n");
		return false;
	}

	TodParticleEmitter* aEmitter = mParticleSystem->mParticleHolder->mEmitters.DataArrayAlloc();
	aEmitter->TodEmitterInitialize(mSystemCenter.x, mSystemCenter.y, mParticleSystem, aDef);
	ParticleEmitterID aEmitterID = static_cast<ParticleEmitterID>(mParticleSystem->mParticleHolder->mEmitters.DataArrayGetID(aEmitter));
	mParticleSystem->mEmitterList.AddTail(aEmitterID);
	return CrossFadeParticle(theParticle, aEmitter);
}

bool TodParticleEmitter::UpdateParticle(TodParticle* theParticle)
{
	if (theParticle->mParticleAge >= theParticle->mParticleDuration)  // 粒子的生命周期结束时
	{
		if (TestBit(mEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_PARTICLE_LOOPS)))  // 判断粒子是否循环
			theParticle->mParticleAge = 0;  // 重置粒子当前帧
		else if (theParticle->mCrossFadeDuration > 0)  // 判断粒子是否处于交叉混合过程中
			theParticle->mParticleAge = theParticle->mParticleDuration - 1;  // 将粒子滞留在最后一帧
		else if (*mEmitterDef->mOnDuration == '\0' || !CrossFadeParticleToName(theParticle, mEmitterDef->mOnDuration))  // 尝试进行交叉混合
			return false;
	}
	if (theParticle->mCrossFadeParticleID != ParticleID::PARTICLEID_NULL &&
		mParticleSystem->mParticleHolder->mParticles.DataArrayTryToGet(theParticle->mCrossFadeParticleID) == nullptr)
		return false;  // 当粒子不存在交叉混合时，可以删除粒子

	theParticle->mParticleTimeValue = theParticle->mParticleAge / (static_cast<float>(theParticle->mParticleDuration) - 1);
	for (int i = 0; i < mEmitterDef->mParticleFields.count; i++)  // 更新粒子受到每个粒子场的作用
		UpdateParticleField(theParticle, &mEmitterDef->mParticleFields.Fields[i], theParticle->mParticleTimeValue, i);
	theParticle->mPosition += theParticle->mVelocity;
	float aSpinSpeed = ParticleTrackEvaluate(mEmitterDef->mParticleSpinSpeed, theParticle, ParticleTracks::TRACK_PARTICLE_SPIN_SPEED) * 0.01;
	float aSpinAngle = ParticleTrackEvaluate(mEmitterDef->mParticleSpinAngle, theParticle, ParticleTracks::TRACK_PARTICLE_SPIN_ANGLE);
	float aLastSpinAngle = FloatTrackEvaluateFromLastTime(
		mEmitterDef->mParticleSpinAngle, theParticle->mParticleLastTimeValue, theParticle->mParticleInterp[ParticleTracks::TRACK_PARTICLE_SPIN_ANGLE]);
	theParticle->mSpinPosition += DEG_TO_RAD(aSpinSpeed + aSpinAngle - aLastSpinAngle) + theParticle->mSpinVelocity;  // 更新粒子旋转角度

	if (FloatTrackIsSet(mEmitterDef->mAnimationRate))  // 如果定义了动画速率
	{
		float aAnimTime = ParticleTrackEvaluate(mEmitterDef->mAnimationRate, theParticle, ParticleTracks::TRACK_PARTICLE_ANIMATION_RATE) * 0.01;
		theParticle->mAnimationTimeValue += aAnimTime;  // 更新动画时间值（动画循环率）
		while (theParticle->mAnimationTimeValue >= 1.0f)
			theParticle->mAnimationTimeValue -= 1.0f;
		while (theParticle->mAnimationTimeValue < 0.0f)
			theParticle->mAnimationTimeValue += 1.0f;
	}

	theParticle->mParticleAge++;
	theParticle->mParticleLastTimeValue = theParticle->mParticleTimeValue;
	return true;
}

void TodParticleEmitter::UpdateSpawning()
{
	TodParticleEmitter* aCrossFadeEmitter = mParticleSystem->mParticleHolder->mEmitters.DataArrayTryToGet(static_cast<unsigned int>(mCrossFadeEmitterID));
	TodParticleEmitter* aSpawningEmitter = !aCrossFadeEmitter ? this : aCrossFadeEmitter;  // 各项数据的计算均以此“主发射器”为准
	mSpawnAccum += aSpawningEmitter->SystemTrackEvaluate(aSpawningEmitter->mEmitterDef->mSpawnRate, ParticleSystemTracks::TRACK_SPAWN_RATE) * 0.01;
	int aSpawnCount = static_cast<int>(mSpawnAccum);
	mSpawnAccum -= aSpawnCount;

	int aSpawnMinActive = static_cast<int>(aSpawningEmitter->SystemTrackEvaluate(aSpawningEmitter->mEmitterDef->mSpawnMinActive, ParticleSystemTracks::TRACK_SPAWN_MIN_ACTIVE));
	if (aSpawnMinActive >= 0 && aSpawnCount < aSpawnMinActive - mParticleList.mSize)
		aSpawnCount = aSpawnMinActive - mParticleList.mSize;  // 至少确保将粒子数量增加至 aSpawnMinActive 个
	int aSpawnMaxActive = static_cast<int>(aSpawningEmitter->SystemTrackEvaluate(aSpawningEmitter->mEmitterDef->mSpawnMaxActive, ParticleSystemTracks::TRACK_SPAWN_MAX_ACTIVE));
	if (aSpawnMaxActive >= 0 && aSpawnCount > aSpawnMaxActive - mParticleList.mSize)
		aSpawnCount = aSpawnMaxActive - mParticleList.mSize;  // 至多保证粒子数量不会超过 aSpawnMaxActive 个
	if (FloatTrackIsSet(aSpawningEmitter->mEmitterDef->mSpawnMaxLaunched))
	{
		int aSpawnMaxLaunched = aSpawningEmitter->SystemTrackEvaluate(aSpawningEmitter->mEmitterDef->mSpawnMaxLaunched, ParticleSystemTracks::TRACK_SPAWN_MAX_LAUNCHED);
		if (aSpawnCount > aSpawnMaxLaunched - mParticlesSpawned)
			aSpawnCount = aSpawnMaxLaunched - mParticlesSpawned;  // 确保发射数量不超过发射器总共能发射的最大数量
	}

	for (int i = 0; i < aSpawnCount; i++)
	{
		TodParticle* aParticle = SpawnParticle(i, aSpawnCount);
		if (aCrossFadeEmitter != nullptr)
			CrossFadeParticle(aParticle, aCrossFadeEmitter);
	}
}

void TodParticleEmitter::DeleteNonCrossFading()
{
	for (TodListNode<ParticleID>* aNode = mParticleList.mHead; aNode != nullptr; )
	{
		TodListNode<ParticleID>* aNext = aNode->mNext;
		TodParticle* aParticle = mParticleSystem->mParticleHolder->mParticles.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
		if (aParticle->mCrossFadeDuration <= 0)  // 当粒子不处于交叉混合状态，则删除该粒子
			DeleteParticle(aParticle);
		aNode = aNext;
	}
}

void TodParticleEmitter::DeleteAll()
{
	while (mParticleList.mSize != 0)
	{
		ParticleID anId = mParticleList.RemoveHead();
		DataArray<TodParticle>& aDataArray = mParticleSystem->mParticleHolder->mParticles;
		aDataArray.DataArrayFree(aDataArray.DataArrayGet(anId));
	}
}

// GOTY @Patoke: 0x521A20
void TodParticleSystem::Update()
{
	if (!mDontUpdate)
	{
		bool aEmitterAlive = false;
		for (TodListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
		{
			TodParticleEmitter* aEmitter = mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
			aEmitter->Update();
			if ((FloatTrackIsSet(aEmitter->mEmitterDef->mCrossFadeDuration) && aEmitter->mParticleList.mSize > 0) || !aEmitter->mDead)
				aEmitterAlive = true;
		}
		if (!aEmitterAlive)
			mDead = true;
	}
}

bool TodParticleEmitter::CrossFadeParticle(TodParticle* theParticle, TodParticleEmitter* theToEmitter)
{
	if (theParticle->mCrossFadeDuration > 0)  // 粒子已处于交叉混合的过程中
	{
		TodTrace("We don't support cross fading more than one at a time\n");
		return false;
	}
	if (!FloatTrackIsSet(theToEmitter->mEmitterDef->mCrossFadeDuration))  // 目标发射器未设定交叉混合时长轨道
	{
		TodTrace("Can't cross fade to emitter that doesn't have CrossFadeDuration");
		return false;
	}
	TOD_ASSERT(theToEmitter != this);  // 不能交叉混合至自身

	TodParticle* aToParticle = theToEmitter->SpawnParticle(0, 1);
	if (aToParticle == nullptr)
		return false;
	if (mEmitterCrossFadeCountDown > 0)  // 如果源发射器正处于交叉混合过程中
		theParticle->mCrossFadeDuration = mEmitterCrossFadeCountDown;  // 源粒子的交叉混合的时长即为源发射器交叉混合的剩余时长
	else
	{
		float aCrossFadeDurationInterp = Sexy::Rand(1);
		int aCrossFadeDuration = FloatTrackEvaluate(theToEmitter->mEmitterDef->mCrossFadeDuration, mSystemTimeValue, aCrossFadeDurationInterp);
		theParticle->mCrossFadeDuration = std::max(1, aCrossFadeDuration);  // 随机取得交叉混合的时长（至少 1 帧）
	}
	if (!FloatTrackIsSet(theToEmitter->mEmitterDef->mParticleDuration))  // 如果目标发射器未定义粒子持续时间
		aToParticle->mParticleDuration = theParticle->mCrossFadeDuration;  // 目标粒子的持续时间等于交叉混合的时间
	aToParticle->mCrossFadeParticleID = static_cast<ParticleID>(mParticleSystem->mParticleHolder->mParticles.DataArrayGetID(theParticle));  // 赋值交叉混合来源的粒子编号
	return true;
}

void TodParticleEmitter::DeleteParticle(TodParticle* theParticle)
{
	TodParticle* aCrossFadeParticle = mParticleSystem->mParticleHolder->mParticles.DataArrayTryToGet(static_cast<unsigned int>(theParticle->mCrossFadeParticleID));
	if (aCrossFadeParticle != nullptr)
	{
		aCrossFadeParticle->mParticleEmitter->DeleteParticle(aCrossFadeParticle);  // 同时删除交叉混合的源粒子
		theParticle->mCrossFadeParticleID = ParticleID::PARTICLEID_NULL;
	}
	
	ParticleID aParticleID = static_cast<ParticleID>(mParticleSystem->mParticleHolder->mParticles.DataArrayGetID(theParticle));
	mParticleList.RemoveAt(mParticleList.Find(aParticleID));
	mParticleSystem->mParticleHolder->mParticles.DataArrayFree(theParticle);
}

void TodParticleEmitter::Update()
{
	if (mDead)
		return;

	mSystemAge++;
	bool aDie = false;
	if (mSystemAge >= mSystemDuration)  // 发射器的生命周期结束时
	{
		if (TestBit(mEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_SYSTEM_LOOPS)))  // 判断发射器是否循环
			mSystemAge = 0;  // 重置发射器当前帧
		else
		{
			mSystemAge = mSystemDuration - 1;  // 将发射器滞留在最后一帧
			aDie = true;
		}
	}

	if (mEmitterCrossFadeCountDown > 0)
	{
		mEmitterCrossFadeCountDown--;  // 更新发射器的交叉混合
		if (mEmitterCrossFadeCountDown == 0)
			aDie = true;
	}
	if (mCrossFadeEmitterID != ParticleEmitterID::PARTICLEEMITTERID_NULL)
	{
		TodParticleEmitter* aCrossFadeEmitter = mParticleSystem->mParticleHolder->mEmitters.DataArrayTryToGet(mCrossFadeEmitterID);
		if (aCrossFadeEmitter == nullptr || aCrossFadeEmitter->mDead)
			aDie = true;
	}

	mSystemTimeValue = mSystemAge / static_cast<float>(mSystemDuration - 1);
	for (int i = 0; i < mEmitterDef->mSystemFields.count; i++)
		UpdateSystemField(&mEmitterDef->mSystemFields.Fields[i], mSystemTimeValue, i);  // 更新发射器受到每个系统场的作用
	for (TodListNode<ParticleID>* aNode = mParticleList.mHead; aNode != nullptr; )
	{
		TodListNode<ParticleID>* aNext = aNode->mNext;
		TodParticle* aParticle = mParticleSystem->mParticleHolder->mParticles.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
		if (!UpdateParticle(aParticle))  // 更新发射器中的每个粒子
			DeleteParticle(aParticle);
		aNode = aNext;
	}
	UpdateSpawning();  // 更新粒子发射

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

bool TodParticleEmitter::GetRenderParams(TodParticle* theParticle, ParticleRenderParams* theParams)
{
	TodParticleEmitter* aEmitter = theParticle->mParticleEmitter;
	TodEmitterDefinition* aDef = aEmitter->mEmitterDef;

	// 颜色。对于每一色彩通道，当系统对应轨道、粒子对应轨道和对应覆写中任一有定义时，认为该通道已设定。
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
	// 缩放
	theParams->mParticleScaleIsSet = false;
	theParams->mParticleScaleIsSet |= FloatTrackIsSet(aDef->mParticleScale);
	theParams->mParticleScaleIsSet |= (aEmitter->mScaleOverride != 1.0f);
	// 拉伸
	theParams->mParticleStretchIsSet = FloatTrackIsSet(aDef->mParticleStretch);
	// 旋转角度。当粒子使用随机初始旋转角度或对齐发射方向角度时，也可认为旋转角度已设定。
	theParams->mSpinPositionIsSet = false;
	theParams->mSpinPositionIsSet |= FloatTrackIsSet(aDef->mParticleSpinSpeed);
	theParams->mSpinPositionIsSet |= FloatTrackIsSet(aDef->mParticleSpinAngle);
	theParams->mSpinPositionIsSet |= TestBit(aDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_RANDOM_LAUNCH_SPIN));
	theParams->mSpinPositionIsSet |= TestBit(aDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_ALIGN_LAUNCH_SPIN));
	// 位置
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
	// 实际颜色 = 粒子颜色 * 系统颜色 * 覆写颜色 * 亮度
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

	TodParticle* aCrossFadeParticle = aEmitter->mParticleSystem->mParticleHolder->mParticles.DataArrayTryToGet(static_cast<unsigned int>(theParticle->mCrossFadeParticleID));
	if (aCrossFadeParticle != nullptr)  // 当存在交叉混合的粒子时，将二者的渲染参数进行混合（从 aCrossFadeParticle 至 theParticle 的交叉混合）
	{
		ParticleRenderParams aCrossFadeParams;
		if (TodParticleEmitter::GetRenderParams(aCrossFadeParticle, &aCrossFadeParams))
		{
			float aFraction = theParticle->mParticleAge / static_cast<float>(aCrossFadeParticle->mCrossFadeDuration - 1);
			// 各项数值按照 aFraction 比例混合
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
			// 当交叉混合来源的某项已设定时，可以认为该粒子的对应项也已设定
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

void RenderParticle(Graphics* g, TodParticle* theParticle, const Color& theColor, ParticleRenderParams* theParams, TodTriangleGroup* theTriangleGroup)
{
	TodParticleEmitter* aEmitter = theParticle->mParticleEmitter;
	TodEmitterDefinition* aEmitterDef = aEmitter->mEmitterDef;
	Image* aImage = aEmitter->mImageOverride != nullptr ? aEmitter->mImageOverride : aEmitterDef->mImage;  // 优先使用覆写贴图，无覆写贴图则使用定义的贴图
	if (aImage == nullptr)
		return;  // 不存在贴图时，取消绘制

	int aCelWidth = aImage->GetCelWidth();
	int aCelHeight = aImage->GetCelHeight();
	int aFrame = aEmitter->mFrameOverride;
	if (aFrame == -1)  // 如果未定义覆写帧
	{
		if (FloatTrackIsSet(aEmitterDef->mAnimationRate))  // 如果定义了动画速率
			aFrame = ClampInt(theParticle->mAnimationTimeValue * aEmitterDef->mImageFrames, 0, aEmitterDef->mImageFrames - 1);  // 动画时间值（循环率） * 总帧数得到当前帧
		else if (aEmitterDef->mAnimated)
			aFrame = ClampInt(theParticle->mParticleTimeValue * aEmitterDef->mImageFrames, 0, aEmitterDef->mImageFrames - 1);  // 粒子时间值 * 总帧数得到当前帧
		else
			aFrame = theParticle->mImageFrame;  // 帧固定的粒子，直接取其贴图帧
	}
	aFrame += aEmitterDef->mImageCol;  // 当前帧加上定义中的图像起始列，得到当前帧在贴图中的列数
	if (aFrame >= aImage->mNumCols)
		aFrame = aImage->mNumCols - 1;

	Rect aSrcRect(aFrame * aCelWidth, std::min(aEmitterDef->mImageRow, aImage->mNumRows - 1) * aCelHeight, aCelWidth, aCelHeight);
	float aClipTop = TodParticleEmitter::ParticleTrackEvaluate(aEmitterDef->mClipTop, theParticle, ParticleTracks::TRACK_PARTICLE_CLIP_TOP);
	float aClipBottom = TodParticleEmitter::ParticleTrackEvaluate(aEmitterDef->mClipBottom, theParticle, ParticleTracks::TRACK_PARTICLE_CLIP_BOTTOM);
	float aClipLeft = TodParticleEmitter::ParticleTrackEvaluate(aEmitterDef->mClipLeft, theParticle, ParticleTracks::TRACK_PARTICLE_CLIP_LEFT);
	float aClipRight = TodParticleEmitter::ParticleTrackEvaluate(aEmitterDef->mClipRight, theParticle, ParticleTracks::TRACK_PARTICLE_CLIP_RIGHT);
	TOD_ASSERT(aClipTop >= 0.0f && aClipTop <= 1.0f);
	TOD_ASSERT(aClipBottom >= 0.0f && aClipBottom <= 1.0f);
	TOD_ASSERT(aClipLeft >= 0.0f && aClipLeft <= 1.0f);
	TOD_ASSERT(aClipRight >= 0.0f && aClipRight <= 1.0f);
	theParams->mPosX += aClipLeft * aCelWidth;
	theParams->mPosY += aClipTop * aCelHeight;
	aSrcRect.mX += FloatRoundToInt(aClipLeft * aCelWidth);
	aSrcRect.mY += FloatRoundToInt(aClipTop * aCelHeight);
	aSrcRect.mWidth -= FloatRoundToInt(aCelWidth * (aClipLeft + aClipRight));
	aSrcRect.mHeight -= FloatRoundToInt(aCelHeight * (aClipBottom + aClipTop));  // 以上根据裁剪各方向的比例调整源矩形
	TOD_ASSERT(aSrcRect.mX == aCelWidth * aFrame + FloatRoundToInt(aClipLeft * aCelWidth));
	TOD_ASSERT(aSrcRect.mY == aCelHeight * aEmitterDef->mImageRow + FloatRoundToInt(aClipTop * aCelHeight));
	TOD_ASSERT(aSrcRect.mX >= 0 && aSrcRect.mX < 10000);
	TOD_ASSERT(aSrcRect.mY >= 0 && aSrcRect.mY < 10000);

	if (TestBit(aEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_ALIGN_TO_PIXELS)))  // 坐标对齐至整数像素点
	{
		theParams->mPosX = FloatRoundToInt(theParams->mPosX);
		theParams->mPosY = FloatRoundToInt(theParams->mPosY);
	}
	int aDrawMode = g->mDrawMode;
	if (TestBit(aEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_ADDITIVE)))  // 使用叠加模式
		aDrawMode = Graphics::DRAWMODE_ADDITIVE;
	if (TestBit(aEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_FULLSCREEN)))  // 全屏模式
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
		TodScaleRotateTransformMatrix(
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

void TodParticleEmitter::DrawParticle(Graphics* g, TodParticle* theParticle, TodTriangleGroup* theTriangleGroup)
{
	if (theParticle->mCrossFadeDuration > 0)  // 交叉混合的源粒子，不绘制
		return;

	ParticleRenderParams aParams;
	if (GetRenderParams(theParticle, &aParams))
	{
		Color aColor(
			ClampInt(FloatRoundToInt(aParams.mRed), 0, 255), 
			ClampInt(FloatRoundToInt(aParams.mGreen), 0, 255), 
			ClampInt(FloatRoundToInt(aParams.mBlue), 0, 255), 
			ClampInt(FloatRoundToInt(aParams.mAlpha), 0, 255)
		);
		if (aColor.mAlpha > 0)  // 不透明度为 0 时，不绘制
		{
			aParams.mPosX += g->mTransX;
			aParams.mPosY += g->mTransY;

			TodParticle* aParticle;
			if (mImageOverride || mEmitterDef->mImage)  // 粒子有贴图时，渲染该粒子
				aParticle = theParticle;
			else  // 粒子没有贴图时，尝试渲染交叉混合来源的粒子
				aParticle = mParticleSystem->mParticleHolder->mParticles.DataArrayTryToGet(static_cast<unsigned int>(theParticle->mCrossFadeParticleID));
			if (aParticle != nullptr)
				RenderParticle(g, aParticle, aColor, &aParams, theTriangleGroup);
		}
	}
}

void TodParticleSystem::Draw(Graphics* g)
{
	for (TodListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
		mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue))->Draw(g);
}

void TodParticleEmitter::Draw(Graphics* g)
{
	bool aHardWare = gSexyAppBase->Is3DAccelerated();
	if ((TestBit(mEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_SOFTWARE_ONLY)) && aHardWare) ||
		(TestBit(mEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_HARDWARE_ONLY)) && !aHardWare))
		return;

	TodTriangleGroup aTriangleGroup;
	for (TodListNode<ParticleID>* aNode = mParticleList.mHead; aNode != nullptr; aNode = aNode->mNext)
		DrawParticle(g, mParticleSystem->mParticleHolder->mParticles.DataArrayGet(static_cast<unsigned int>(aNode->mValue)), &aTriangleGroup);
	aTriangleGroup.DrawGroup(g);
}

void TodParticleSystem::SystemMove(float theX, float theY)
{
	for (TodListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
		mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue))->SystemMove(theX, theY);
}

void TodParticleEmitter::SystemMove(float theX, float theY)
{
	float aDeltaX = theX - mSystemCenter.x;
	float aDeltaY = theY - mSystemCenter.y;
	if (FloatApproxEqual(aDeltaX, 0.0f) && FloatApproxEqual(aDeltaY, 0.0f))  // 移动前后的坐标几乎相等时，无需操作
		return;
	
	mSystemCenter.x = theX;
	mSystemCenter.y = theY;
	if (!TestBit(mEmitterDef->mParticleFlags, static_cast<int>(ParticleFlags::PARTICLE_PARTICLES_DONT_FOLLOW)))
	{
		for (TodListNode<ParticleID>* aNode = mParticleList.mHead; aNode != nullptr; aNode = aNode->mNext)
		{
			TodParticle* aParticle = mParticleSystem->mParticleHolder->mParticles.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
			aParticle->mPosition.x += aDeltaX;
			aParticle->mPosition.y += aDeltaY;
		}
	}
}

void TodParticleSystem::OverrideColor(const char* theEmitterName, const Color& theColor)
{
	for (TodListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		TodParticleEmitter* aEmitter = mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
		if (theEmitterName == nullptr || strcasecmp(theEmitterName, aEmitter->mEmitterDef->mName) == 0)
			aEmitter->mColorOverride = theColor;
	}
}

void TodParticleSystem::OverrideExtraAdditiveDraw(const char* theEmitterName, bool theEnableExtraAdditiveDraw)
{
	for (TodListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		TodParticleEmitter* aEmitter = mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
		if (theEmitterName == nullptr || strcasecmp(theEmitterName, aEmitter->mEmitterDef->mName) == 0)
			aEmitter->mExtraAdditiveDrawOverride = theEnableExtraAdditiveDraw;
	}
}

// GOTY @Patoke: 0x522CB0
void TodParticleSystem::OverrideImage(const char* theEmitterName, Image* theImage)
{
	for (TodListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		TodParticleEmitter* aEmitter = mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
		if (theEmitterName == nullptr || strcasecmp(theEmitterName, aEmitter->mEmitterDef->mName) == 0)
			aEmitter->mImageOverride = theImage;
	}
}

void TodParticleSystem::OverrideFrame(const char* theEmitterName, int theFrame)
{
	for (TodListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		TodParticleEmitter* aEmitter = mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
		if (theEmitterName == nullptr || strcasecmp(theEmitterName, aEmitter->mEmitterDef->mName) == 0)
			aEmitter->mFrameOverride = theFrame;
	}
}

void TodParticleSystem::OverrideScale(const char* theEmitterName, float theScale)
{
	for (TodListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		TodParticleEmitter* aEmitter = mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
		if (theEmitterName == nullptr || strcasecmp(theEmitterName, aEmitter->mEmitterDef->mName) == 0)
			aEmitter->mScaleOverride = theScale;
	}
}

TodParticleEmitter* TodParticleSystem::FindEmitterByName(const char* theEmitterName)
{
	for (TodListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		TodParticleEmitter* aEmitter = mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
		if (strcasecmp(theEmitterName, aEmitter->mEmitterDef->mName) == 0)
			return aEmitter;
	}
	return nullptr;
}

TodEmitterDefinition* TodParticleSystem::FindEmitterDefByName(const char* theEmitterName)
{
	for (int i = 0; i < mParticleDef->mEmitterDefCount; i++)
	{
		TodEmitterDefinition* aEmitterDef = &mParticleDef->mEmitterDefs[i];
		if (strcasecmp(theEmitterName, aEmitterDef->mName) == 0)
			return aEmitterDef;
	}
	return nullptr;
}

void TodParticleEmitter::CrossFadeEmitter(TodParticleEmitter* theToEmitter)
{
	if (mEmitterCrossFadeCountDown > 0)
	{
		TodTrace("We don't support cross fading emitters more than one at a time\n");
		return;
	}
	if (!FloatTrackIsSet(theToEmitter->mEmitterDef->mCrossFadeDuration))
	{
		TodTrace("Can't cross fade to emitter that doesn't have CrossFadeDuration");
		return;
	}
	TOD_ASSERT(theToEmitter != this);

	float aCrossFadeDurationInterp = Sexy::Rand(1.0f);
	mEmitterCrossFadeCountDown = FloatTrackEvaluate(theToEmitter->mEmitterDef->mCrossFadeDuration, mSystemTimeValue, aCrossFadeDurationInterp);
	mEmitterCrossFadeCountDown = std::max(1, mEmitterCrossFadeCountDown);
	mCrossFadeEmitterID = static_cast<ParticleEmitterID>(mParticleSystem->mParticleHolder->mEmitters.DataArrayGetID(theToEmitter));
	if (!FloatTrackIsSet(theToEmitter->mEmitterDef->mSystemDuration))
		theToEmitter->mSystemDuration = mEmitterCrossFadeCountDown;

	for (TodListNode<ParticleID>* aNode = mParticleList.mHead; aNode != nullptr; aNode = aNode->mNext)
		CrossFadeParticle(mParticleSystem->mParticleHolder->mParticles.DataArrayGet(static_cast<unsigned int>(aNode->mValue)), theToEmitter);
}

void TodParticleSystem::CrossFade(const char* theEmitterName)
{
	TodEmitterDefinition* aEmitterDef = FindEmitterDefByName(theEmitterName);
	if (aEmitterDef == nullptr)
	{
		TodTrace("Can't find cross fade emitter: %s\n", theEmitterName);
		return;
	}
	if (!FloatTrackIsSet(aEmitterDef->mCrossFadeDuration))
	{
		TodTrace("Can't cross fade without duration set: %s\n", theEmitterName);
		return;
	}
	if (mParticleHolder->mEmitters.mSize + mEmitterList.mSize > mParticleHolder->mEmitters.mMaxSize)
	{
		TodTrace("Too many emitters to cross fade\n");
		ParticleSystemDie();
		return;
	}

	for (TodListNode<ParticleEmitterID>* aNode = mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		TodParticleEmitter* aEmitter = mParticleHolder->mEmitters.DataArrayGet(static_cast<unsigned int>(aNode->mValue));
		if (aEmitter->mEmitterDef != aEmitterDef)  // 不能交叉混合至同种类的发射器
		{
			TodParticleEmitter* aCrossFadeEmitter = mParticleHolder->mEmitters.DataArrayAlloc();
			aCrossFadeEmitter->TodEmitterInitialize(aEmitter->mSystemCenter.x, aEmitter->mSystemCenter.y, this, aEmitterDef);
			ParticleEmitterID aCrossFadeEmitterID = static_cast<ParticleEmitterID>(mParticleHolder->mEmitters.DataArrayGetID(aCrossFadeEmitter));
			mEmitterList.AddTail(aCrossFadeEmitterID);
			aEmitter->CrossFadeEmitter(aCrossFadeEmitter);
		}
	}
}

TodParticleHolder::~TodParticleHolder()
{
	DisposeHolder();
}

void TodParticleHolder::InitializeHolder()
{
	mParticleSystems.DataArrayInitialize(1024U, "particle systems");
	mEmitters.DataArrayInitialize(1024U, "emitters");
	mParticles.DataArrayInitialize(1024U, "particles");
	mParticleListNodeAllocator.Initialize(1024, sizeof(TodListNode<ParticleID>));
	mEmitterListNodeAllocator.Initialize(1024, sizeof(TodListNode<ParticleEmitterID>));
}

void TodParticleHolder::DisposeHolder()
{
	mParticleSystems.DataArrayDispose();
	mEmitters.DataArrayDispose();
	mParticles.DataArrayDispose();
	mParticleListNodeAllocator.FreeAll();
	mEmitterListNodeAllocator.FreeAll();
}

bool TodParticleHolder::IsOverLoaded()
{
	return mParticleSystems.mSize > MAX_PARTICLES_SIZE || mEmitters.mSize > MAX_PARTICLES_SIZE || mParticles.mSize > MAX_PARTICLES_SIZE;
}

TodParticleSystem* TodParticleHolder::AllocParticleSystemFromDef(float theX, float theY, int theRenderOrder, TodParticleDefinition* theDefinition, ParticleEffect theParticleEffect)
{
	if (mParticleSystems.mSize == mParticleSystems.mMaxSize)
	{
		TodTrace("Too many particle systems\n");
		return nullptr;
	}
	if (theDefinition->mEmitterDefCount + mEmitters.mSize > mEmitters.mMaxSize)
	{
		TodTrace("Too many particle emitters\n");
		return nullptr;
	}

	TodParticleSystem* aTodParticle = mParticleSystems.DataArrayAlloc();
	aTodParticle->mParticleHolder = this;
	aTodParticle->TodParticleInitializeFromDef(theX, theY, theRenderOrder, theDefinition, theParticleEffect);
	return aTodParticle;
}

TodParticleSystem* TodParticleHolder::AllocParticleSystem(float theX, float theY, int theRenderOrder, ParticleEffect theParticleEffect)
{
	TOD_ASSERT(static_cast<int>(theParticleEffect) >= 0 && static_cast<int>(theParticleEffect) < gParticleDefCount);
	return AllocParticleSystemFromDef(theX, theY, theRenderOrder, &gParticleDefArray[theParticleEffect], theParticleEffect);
}
