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

#include "../Plant.h"
#include "../Zombie.h"
#include "../../LawnApp.h"
#include "ReanimationLawn.h"
#include "../../PvzpLib/PvzpDebug.h"
#include "graphics/Color.h"
#include "../../PvzpLib/Reanimator.h"
#include "graphics/MemoryImage.h"

void ReanimatorCache::UpdateReanimationForVariation(Reanimation* theReanim, DrawVariation theDrawVariation)
{
	if (theDrawVariation >= DrawVariation::VARIATION_MARIGOLD_WHITE && theDrawVariation <= DrawVariation::VARIATION_MARIGOLD_LIGHT_GREEN)
	{
		size_t aVariationIndex = static_cast<size_t>(theDrawVariation) - static_cast<size_t>(DrawVariation::VARIATION_MARIGOLD_WHITE);
		Color MARIGOLD_VARIATIONS[] = {
			Color(255, 255, 255),
			Color(230, 30, 195),
			Color(250, 125, 5),
			Color(255, 145, 215),
			Color(160, 255, 245),
			Color(230, 30, 30),
			Color(5, 130, 255),
			Color(195, 55, 235),
			Color(235, 210, 255),
			Color(255, 245, 55),
			Color(180, 255, 105)
		};

		PVZP_ASSERT(aVariationIndex >= 0 && aVariationIndex < LENGTH(MARIGOLD_VARIATIONS));
		theReanim->GetTrackInstanceByName("Marigold_petals")->mTrackColor = MARIGOLD_VARIATIONS[aVariationIndex];
	}
	else
	{
		switch (theDrawVariation)
		{
		case DrawVariation::VARIATION_IMITATER:
			theReanim->mFilterEffect = FilterEffect::FILTER_EFFECT_WASHED_OUT;
			break;
		case DrawVariation::VARIATION_IMITATER_LESS:
			theReanim->mFilterEffect = FilterEffect::FILTER_EFFECT_LESS_WASHED_OUT;
			break;
		case DrawVariation::VARIATION_ZEN_GARDEN:
			theReanim->SetFramesForLayer("anim_zengarden");
			break;
		case DrawVariation::VARIATION_ZEN_GARDEN_WATER:
			theReanim->SetFramesForLayer("anim_waterplants");
			break;
		case DrawVariation::VARIATION_AQUARIUM:
			theReanim->SetFramesForLayer("anim_idle_aquarium");
			break;
		case DrawVariation::VARIATION_SPROUT_NO_FLOWER:
			theReanim->SetFramesForLayer("anim_idle_noflower");
			break;
		default:
			PVZP_ASSERT(false);
			break;
		}
	}
}

void ReanimatorCache::DrawReanimatorFrame(Graphics* g, float thePosX, float thePosY, ReanimationType theReanimationType, const char* theTrackName, DrawVariation theDrawVariation)
{
	Reanimation aReanim;
	aReanim.ReanimationInitializeType(thePosX, thePosY, theReanimationType);

	if (theTrackName != nullptr && aReanim.TrackExists(theTrackName))
	{
		aReanim.SetFramesForLayer(theTrackName);
	}
	if (theReanimationType == ReanimationType::REANIM_KERNELPULT)
	{
		aReanim.AssignRenderGroupToTrack("Cornpult_butter", RENDER_GROUP_HIDDEN);
	}
	else if (theReanimationType == ReanimationType::REANIM_SUNFLOWER)
	{
		aReanim.mAnimTime = 0.15f;
	}
	aReanim.AssignRenderGroupToTrack("anim_waterline", RENDER_GROUP_HIDDEN);

	if (g->GetColorizeImages())
	{
		aReanim.mColorOverride = g->GetColor();
	}
	aReanim.OverrideScale(g->mScaleX, g->mScaleY);

	if (theDrawVariation != DrawVariation::VARIATION_NORMAL)
	{
		UpdateReanimationForVariation(&aReanim, theDrawVariation);
	}

	aReanim.Draw(g);
}

MemoryImage* ReanimatorCache::MakeBlankMemoryImage(int theWidth, int theHeight)
{
	MemoryImage* aImage = new MemoryImage();

	int aBitsCount = theWidth * theHeight;
	aImage->mBits = new uint32_t[aBitsCount + 1];
	aImage->mWidth = theWidth;
	aImage->mHeight = theHeight;
	aImage->mHasTrans = true;
	aImage->mHasAlpha = true;
	memset(aImage->mBits, 0, aBitsCount * 4);
	aImage->mBits[aBitsCount] = Sexy::MEMORYCHECK_ID;
	return aImage;
}

void ReanimatorCache::GetPlantImageSize(SeedType theSeedType, int& theOffsetX, int& theOffsetY, int& theWidth, int& theHeight)
{
	theOffsetX = -20;
	theOffsetY = -20;
	theWidth = 120;
	theHeight = 120;

	if (theSeedType == SeedType::SEED_TALLNUT)
	{
		theOffsetY = -40;
		theHeight += 40;
	}
	else if (theSeedType == SeedType::SEED_MELONPULT || theSeedType == SeedType::SEED_WINTERMELON)
	{
		theOffsetX = -40;
		theWidth += 40;
	}
	else if (theSeedType == SeedType::SEED_COBCANNON)
	{
		theWidth += 80;
	}
}

MemoryImage* ReanimatorCache::MakeCachedMowerFrame(LawnMowerType theMowerType)
{
	MemoryImage* aImage = nullptr;

	switch (theMowerType)
	{
	case LawnMowerType::LAWNMOWER_LAWN:
	{
		aImage = MakeBlankMemoryImage(90, 100);
		Graphics aMemoryGraphics(aImage);
		aMemoryGraphics.SetLinearBlend(true);
		aMemoryGraphics.mScaleX = 0.85f;
		aMemoryGraphics.mScaleY = 0.85f;
		DrawReanimatorFrame(&aMemoryGraphics, 10.0f, 0.0f, ReanimationType::REANIM_LAWNMOWER, "anim_normal", DrawVariation::VARIATION_NORMAL);
		break;
	}
	case LawnMowerType::LAWNMOWER_POOL:
	{
		aImage = MakeBlankMemoryImage(90, 100);
		Graphics aMemoryGraphics(aImage);
		aMemoryGraphics.SetLinearBlend(true);
		aMemoryGraphics.mScaleX = 0.8f;
		aMemoryGraphics.mScaleY = 0.8f;
		DrawReanimatorFrame(&aMemoryGraphics, 10.0f, 25.0f, ReanimationType::REANIM_POOL_CLEANER, nullptr, DrawVariation::VARIATION_NORMAL);
		break;
	}
	case LawnMowerType::LAWNMOWER_ROOF:
	{
		aImage = MakeBlankMemoryImage(90, 100);
		Graphics aMemoryGraphics(aImage);
		aMemoryGraphics.SetLinearBlend(true);
		aMemoryGraphics.mScaleX = 0.85f;
		aMemoryGraphics.mScaleY = 0.85f;
		DrawReanimatorFrame(&aMemoryGraphics, 10.0f, 0.0f, ReanimationType::REANIM_ROOF_CLEANER, nullptr, DrawVariation::VARIATION_NORMAL);
		break;
	}
	case LawnMowerType::LAWNMOWER_SUPER_MOWER:
	{
		aImage = MakeBlankMemoryImage(90, 100);
		Graphics aMemoryGraphics(aImage);
		aMemoryGraphics.SetLinearBlend(true);
		aMemoryGraphics.mScaleX = 0.85f;
		aMemoryGraphics.mScaleY = 0.85f;
		DrawReanimatorFrame(&aMemoryGraphics, 10.0f, 0.0f, ReanimationType::REANIM_LAWNMOWER, "anim_tricked", DrawVariation::VARIATION_NORMAL);
		break;
	}
	default:
		PVZP_ASSERT(false);
		break;
	}

	return aImage;
}

MemoryImage* ReanimatorCache::MakeCachedPlantFrame(SeedType theSeedType, DrawVariation theDrawVariation)
{
	int aOffsetX, aOffsetY, aWidth, aHeight;
	GetPlantImageSize(theSeedType, aOffsetX, aOffsetY, aWidth, aHeight);
	MemoryImage* aMemoryImage = MakeBlankMemoryImage(aWidth, aHeight);
	Graphics aMemoryGraphics(aMemoryImage);
	aMemoryGraphics.SetLinearBlend(true);

	const PlantDefinition& aPlantDef = GetPlantDefinition(theSeedType);
	//PVZP_ASSERT(aPlantDef.mReanimationType != ReanimationType::REANIM_NONE);

	if (theSeedType == SeedType::SEED_POTATOMINE)
	{
		aMemoryGraphics.mScaleX = 0.85f;
		aMemoryGraphics.mScaleY = 0.85f;
		DrawReanimatorFrame(&aMemoryGraphics, -static_cast<int>(aOffsetX - 12.0f), -static_cast<int>(aOffsetY - 12.0f), aPlantDef.mReanimationType, "anim_armed", theDrawVariation);
	}
	else if (theSeedType == SeedType::SEED_INSTANT_COFFEE)
	{
		aMemoryGraphics.mScaleX = 0.8f;
		aMemoryGraphics.mScaleY = 0.8f;
		DrawReanimatorFrame(&aMemoryGraphics, -static_cast<int>(aOffsetX - 12.0f), -static_cast<int>(aOffsetY - 12.0f), aPlantDef.mReanimationType, "anim_idle", theDrawVariation);
	}
	else if (theSeedType == SeedType::SEED_EXPLODE_O_NUT)
	{
		aMemoryGraphics.SetColorizeImages(true);
		aMemoryGraphics.SetColor(Color(255, 64, 64));
		DrawReanimatorFrame(&aMemoryGraphics, -aOffsetX, -aOffsetY, aPlantDef.mReanimationType, "anim_idle", theDrawVariation);
	}
	else
	{
		DrawReanimatorFrame(&aMemoryGraphics, -aOffsetX, -aOffsetY, aPlantDef.mReanimationType, "anim_idle", theDrawVariation);

		if (theSeedType == SeedType::SEED_PEASHOOTER || theSeedType == SeedType::SEED_SNOWPEA || theSeedType == SeedType::SEED_REPEATER ||
			theSeedType == SeedType::SEED_LEFTPEATER || theSeedType == SeedType::SEED_GATLINGPEA)
		{
			DrawReanimatorFrame(&aMemoryGraphics, -aOffsetX, -aOffsetY, aPlantDef.mReanimationType, "anim_head_idle", theDrawVariation);
		}
		else if (theSeedType == SeedType::SEED_SPLITPEA)
		{
			DrawReanimatorFrame(&aMemoryGraphics, -aOffsetX, -aOffsetY, aPlantDef.mReanimationType, "anim_head_idle", theDrawVariation);
			DrawReanimatorFrame(&aMemoryGraphics, -aOffsetX, -aOffsetY, aPlantDef.mReanimationType, "anim_splitpea_idle", theDrawVariation);
		}
		else if (theSeedType == SeedType::SEED_THREEPEATER)
		{
			DrawReanimatorFrame(&aMemoryGraphics, -aOffsetX, -aOffsetY, aPlantDef.mReanimationType, "anim_head_idle1", theDrawVariation);
			DrawReanimatorFrame(&aMemoryGraphics, -aOffsetX, -aOffsetY, aPlantDef.mReanimationType, "anim_head_idle3", theDrawVariation);
			DrawReanimatorFrame(&aMemoryGraphics, -aOffsetX, -aOffsetY, aPlantDef.mReanimationType, "anim_head_idle2", theDrawVariation);
		}
	}

	return aMemoryImage;
}

MemoryImage* ReanimatorCache::MakeCachedZombieFrame(ZombieType theZombieType)
{
	MemoryImage* aMemoryImage = MakeBlankMemoryImage(200, 210);
	Graphics aMemoryGraphics(aMemoryImage);
	aMemoryGraphics.SetLinearBlend(true);

	ZombieType aUseZombieType = theZombieType;
	if (theZombieType == ZombieType::ZOMBIE_CACHED_POLEVAULTER_WITH_POLE)
	{
		aUseZombieType = ZombieType::ZOMBIE_POLEVAULTER;
	}
	const ZombieDefinition& aZombieDef = GetZombieDefinition(aUseZombieType);
	PVZP_ASSERT(aZombieDef.mReanimationType != ReanimationType::REANIM_NONE);

	float aPosX = 40.0f, aPosY = 40.0f;
	if (aZombieDef.mReanimationType == ReanimationType::REANIM_ZOMBIE)
	{
		Reanimation aReanim;
		aReanim.ReanimationInitializeType(aPosX, aPosY, aZombieDef.mReanimationType);
		aReanim.SetFramesForLayer("anim_idle");
		Zombie::SetupReanimLayers(&aReanim, aUseZombieType);

		if (theZombieType == ZombieType::ZOMBIE_DOOR)
			aReanim.AssignRenderGroupToTrack("anim_screendoor", RENDER_GROUP_NORMAL);
		else if (theZombieType == ZombieType::ZOMBIE_FLAG)
		{
			Reanimation aReanimFlag;
			aReanimFlag.ReanimationInitializeType(aPosX, aPosY, ReanimationType::REANIM_FLAG);
			aReanimFlag.SetFramesForLayer("Zombie_flag");
			aReanimFlag.Draw(&aMemoryGraphics);
		}
		aReanim.Draw(&aMemoryGraphics);
	}
	else if (aZombieDef.mReanimationType == ReanimationType::REANIM_BOSS)
	{
		Reanimation aReanim;
		aReanim.ReanimationInitializeType(-524.0f, -88.0f, aZombieDef.mReanimationType);
		aReanim.SetFramesForLayer("anim_head_idle");
		Reanimation aReanimDriver;
		aReanimDriver.ReanimationInitializeType(46.0f, 22.0f, ReanimationType::REANIM_BOSS_DRIVER);
		aReanimDriver.SetFramesForLayer("anim_idle");

		aReanim.Draw(&aMemoryGraphics);
		aReanimDriver.Draw(&aMemoryGraphics);
		aReanim.AssignRenderGroupToTrack("boss_body1", RENDER_GROUP_HIDDEN);
		aReanim.AssignRenderGroupToTrack("boss_neck", RENDER_GROUP_HIDDEN);
		aReanim.AssignRenderGroupToTrack("boss_head2", RENDER_GROUP_HIDDEN);
		aReanim.Draw(&aMemoryGraphics);
	}
	else
	{
		const char* aTrackName = "anim_idle";
		if (theZombieType == ZombieType::ZOMBIE_POGO)
		{
			aTrackName = "anim_pogo";
		}
		else if (theZombieType == ZombieType::ZOMBIE_CACHED_POLEVAULTER_WITH_POLE)
		{
			aTrackName = "anim_idle";
		}
		else if (theZombieType == ZombieType::ZOMBIE_POLEVAULTER)
		{
			aTrackName = "anim_walk";
		}
		else if (theZombieType == ZombieType::ZOMBIE_GARGANTUAR)
		{
			aPosY = 60.0f;
		}

		DrawReanimatorFrame(&aMemoryGraphics, aPosX, aPosY, aZombieDef.mReanimationType, aTrackName, DrawVariation::VARIATION_NORMAL);
	}
	return aMemoryImage;
}

ReanimatorCache::~ReanimatorCache()
{
	ReanimatorCacheDispose();
}

void ReanimatorCache::ReanimatorCacheInitialize()
{
	mApp = (LawnApp*)gSexyAppBase;
	for (int i = 0; i < SeedType::NUM_SEED_TYPES; i++)
		mPlantImages[i] = nullptr;
	for (int i = 0; i < LawnMowerType::NUM_MOWER_TYPES; i++)
		mLawnMowers[i] = nullptr;
	for (int i = 0; i < ZombieType::NUM_CACHED_ZOMBIE_TYPES; i++)
		mZombieImages[i] = nullptr;
}

void ReanimatorCache::ReanimatorCacheDispose()
{
	for (int i = 0; i < SeedType::NUM_SEED_TYPES; i++)
	{
		delete mPlantImages[i];
		mPlantImages[i] = nullptr;
	}
	while (mImageVariationList.mSize != 0)
	{
		ReanimCacheImageVariation aImageVariation = mImageVariationList.RemoveHead();
		if (aImageVariation.mImage != nullptr)
			delete aImageVariation.mImage;
	}
	for (int i = 0; i < LawnMowerType::NUM_MOWER_TYPES; i++)
	{
		delete mLawnMowers[i];
		mLawnMowers[i] = nullptr;
	}
	for (int i = 0; i < ZombieType::NUM_CACHED_ZOMBIE_TYPES; i++)
	{
		delete mZombieImages[i];
		mZombieImages[i] = nullptr;
	}
}


void ReanimatorCache::DrawCachedPlant(Graphics* g, float thePosX, float thePosY, SeedType theSeedType, DrawVariation theDrawVariation)
{
	PVZP_ASSERT(theSeedType >= 0 && theSeedType < SeedType::NUM_SEED_TYPES);

	MemoryImage* aImage = nullptr;
	if (theDrawVariation != DrawVariation::VARIATION_NORMAL)
	{
		for (PvzpListNode<ReanimCacheImageVariation>* aNode = mImageVariationList.mHead; aNode != nullptr; aNode = aNode->mNext)
		{
			ReanimCacheImageVariation& aImageVariation = aNode->mValue;
			if (aImageVariation.mSeedType == theSeedType && aImageVariation.mDrawVariation == theDrawVariation)
			{
				aImage = aImageVariation.mImage;
				break;
			}
		}

		if (aImage == nullptr)
		{
			aImage = MakeCachedPlantFrame(theSeedType, theDrawVariation);
			ReanimCacheImageVariation aNewImageVariation;
			aNewImageVariation.mSeedType = theSeedType;
			aNewImageVariation.mDrawVariation = theDrawVariation;
			aNewImageVariation.mImage = aImage;
			mImageVariationList.AddHead(aNewImageVariation);
		}
	}
	else
	{
		aImage = mPlantImages[theSeedType];
		if (aImage == nullptr)
		{
			aImage = MakeCachedPlantFrame(theSeedType, DrawVariation::VARIATION_NORMAL);
			mPlantImages[theSeedType] = aImage;
		}
	}

	int aOffsetX, aOffsetY, aWidth, aHeight;
	GetPlantImageSize(theSeedType, aOffsetX, aOffsetY, aWidth, aHeight);
	if (!mApp->Is3DAccelerated() && g->mScaleX == 1.0f && g->mScaleY == 1.0f)
		g->DrawImage(aImage, thePosX + aOffsetX, thePosY + aOffsetY);
	else
		PvzpDrawImageScaledF(g, aImage, thePosX + (aOffsetX * g->mScaleX), thePosY + (aOffsetY * g->mScaleY), g->mScaleX, g->mScaleY);
}

void ReanimatorCache::DrawCachedMower(Graphics* g, float thePosX, float thePosY, LawnMowerType theMowerType)
{
	PVZP_ASSERT(theMowerType >= 0 && theMowerType < LawnMowerType::NUM_MOWER_TYPES);

	if (mLawnMowers[theMowerType] == nullptr)
		mLawnMowers[theMowerType] = MakeCachedMowerFrame(theMowerType);
	PvzpDrawImageScaledF(g, mLawnMowers[theMowerType], thePosX - 20.0f, thePosY, g->mScaleX, g->mScaleY);
}

void ReanimatorCache::DrawCachedZombie(Graphics* g, float thePosX, float thePosY, ZombieType theZombieType)
{
	PVZP_ASSERT(theZombieType >= 0 && theZombieType < ZombieType::NUM_CACHED_ZOMBIE_TYPES);

	if (mZombieImages[theZombieType] == nullptr)
		mZombieImages[theZombieType] = MakeCachedZombieFrame(theZombieType);
	PvzpDrawImageScaledF(g, mZombieImages[theZombieType], thePosX, thePosY, g->mScaleX, g->mScaleY);
}
