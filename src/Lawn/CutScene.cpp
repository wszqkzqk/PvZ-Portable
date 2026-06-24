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

#include "Board.h"
#include "Plant.h"
#include "Zombie.h"
#include "GridItem.h"
#include "Cutscene.h"
#include "Challenge.h"
#include "LawnMower.h"
#include "SeedPacket.h"
#include "../LawnApp.h"
#include "System/Music.h"
#include "../Resources.h"
#include "MessageWidget.h"
#include "../GameConstants.h"
#include "Widget/LawnDialog.h"
#include "Widget/GameButton.h"
#include "System/PlayerInfo.h"
#include "Widget/StoreScreen.h"
#include "Widget/ChallengeScreen.h"
#include "../Sexy.TodLib/TodFoley.h"
#include "Widget/SeedChooserScreen.h"
#include "../Sexy.TodLib/TodCommon.h"
#include "../Sexy.TodLib/Attachment.h"
#include "../Sexy.TodLib/Reanimator.h"
#include "../Sexy.TodLib/TodParticle.h"
#include "../Sexy.TodLib/TodStringFile.h"
#include "misc/PerfTimer.h"
#include "widget/WidgetManager.h"

static const int	TimePanRightStart				= 1500;
static const int	TimePanRightEnd					= 3500;
static const int	TimeEarlyDaveEnterStart			= 2000;
static const int	TimeEarlyDaveEnterEnd			= 2750;
//static const int	TimeEarlyDaveLeaveStart			= 3250;
static const int	TimeEarlyDaveLeaveEnd			= 4000;
static const int	TimeSeedChoserSlideOnStart		= 4000;
static const int	TimeSeedChoserSlideOnEnd		= 4250;
static const int	TimeSeedChoserSlideOffStart		= 4500;
static const int	TimeSeedChoserSlideOffEnd		= 4750;
static const int	TimeSeedBankOnStart				= 4000;
static const int	TimeSeedBankOnEnd				= 4250;
static const int	TimePanLeftStart				= 4500;
static const int	TimePanLeftEnd					= 6000;
static const int	TimeSeedBankRightStart			= 4750;
static const int	TimeSeedBankRightEnd			= 6000;
static const int	TimeRollSodStart				= 6000;
static const int	TimeRollSodEnd					= 8000;
static const int	TimeGraveStoneStart				= 6000;
static const int	TimeGraveStoneEnd				= 7000;
static const int	TimeReadySetPlantStart			= 6000;
static const int	TimeReadySetPlantEnd			= 7830;
static const int	TimeFogRollIn					= 5950;
//static const int	TimeCrazyDaveEnterStart			= 6500;
//static const int	TimeCrazyDaveEnterEnd			= 7250;
//static const int	TimeCrazyDaveLeaveStart			= 7750;
//static const int	TimeCrazyDaveLeaveEnd			= 8500;
static const int	TimeIntroEnd					= 6000;
static const int	LostTimePanRightStart			= 1500;
static const int	LostTimePanRightEnd				= 3500;
static const int	LostTimeBrainGraphicStart		= 6000;
static const int	LostTimeBrainGraphicShake		= 7000;
static const int	LostTimeBrainGraphicCancelShake	= 8000;
static const int	LostTimeBrainGraphicEnd			= 11000;
static const int	LostTimeEnd						= 11000;
static const int	TimeIntro_PresentsFadeIn		= 1000;
static const int	TimeIntro_LogoStart				= 5500;
static const int	TimeIntro_LogoEnd				= 5900;
static const int	TimeIntro_PanRightStart			= 5890;
static const int	TimeIntro_PanRightEnd			= 11890;
static const int	TimeIntro_FadeOut				= 10890;
static const int	TimeIntro_FadeOutEnd			= 11890;
static const int	TimeIntro_End					= 13890;
static const int	TimeLawnMowerDuration			= 250;
static const int	TimeLawnMowerStart[6]			= { 6300, 6250, 6200, 6150, 6100, 6050 };

CutScene::CutScene()
{
	mApp = (LawnApp*)gSexyAppBase;
	mBoard = mApp->mBoard;
	mCutsceneTime = 0;
	mSodTime = 0;
	mFogTime = 0;
	mBossTime = 0;
	mCrazyDaveTime = 0;
	mGraveStoneTime = 0;
	mReadySetPlantTime = 0;
	mLawnMowerTime = 0;
	mCrazyDaveDialogStart = -1;
	mSeedChoosing = false;
	mZombiesWonReanimID = REANIMATIONID_NULL;
	mPreloaded = false;
	mPlacedZombies = false;
	mPlacedLawnItems = false;
	mCrazyDaveCountDown = 0;
	mCrazyDaveLastTalkIndex = -1;
	mUpsellHideBoard = false;
	mUpsellChallengeScreen = nullptr;
	mPreUpdatingBoard = false;
}

CutScene::~CutScene()
{
	if (mUpsellChallengeScreen)
	{
		delete mUpsellChallengeScreen;
	}
	mApp->mMuteSoundsForCutscene = false;

	mApp->mResourceManager->ReleaseTrackedResources(mLoadedResourceNames);
}

void CutScene::PlaceAZombie(ZombieType theZombieType, int theGridX, int theGridY)
{
	bool aPutOnDuckyTube = false;
	if (theZombieType == ZombieType::ZOMBIE_DUCKY_TUBE && mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS_2)
	{
		theZombieType = ZombieType::ZOMBIE_PEA_HEAD;
		aPutOnDuckyTube = true;
	}

	Zombie* aZombie = mBoard->AddZombieInRow(theZombieType, theGridY, -2);
	TOD_ASSERT(aZombie);
	aZombie->mPosX = theGridX * 56 + 830;
	aZombie->mPosY = theGridY * 90 + 70;
	if (theGridX % 2 == 1)
	{
		aZombie->mPosY += 30.0f;
	}

	if (aPutOnDuckyTube)
	{
		//aZombie->ReanimShowTrack("Zombie_duckytube", 0);
		mApp->ReanimationGet(aZombie->mBodyReanimID)->AssignRenderGroupToTrack("Zombie_duckytube", RENDER_GROUP_NORMAL);
	}
	if (mBoard->StageHasRoof())
	{
		aZombie->mPosY -= theGridY * 2 - theGridX * 7 + 30;  //7 * (5 - theGridX) - 2 * (5 - theGridY) + 5;
		aZombie->mPosX -= 5.0f;
	}
	if (theZombieType == ZombieType::ZOMBIE_ZAMBONI)
	{
		aZombie->mPosY -= 10.0f;
		aZombie->mPosX -= 30.0f;
	}
	else if (mApp->IsLittleTroubleLevel())
	{
		aZombie->mPosY += Rand(50) - 25;  //RandRangeInt(-25, 24);
		aZombie->mPosX += Rand(50) - 25;  //RandRangeInt(-25, 24);
	}
	else if (Is2x2Zombie(theZombieType))
	{
		aZombie->mPosX += Rand(15) - 20;  //RandRangeInt(-20, -6);
	}
	else if (theGridY == 4 && (mApp->CanShowAlmanac() || mApp->CanShowStore()))
	{
		aZombie->mPosX += Rand(15);  //RandRangeInt(0, 14);
	}
	else
	{
		aZombie->mPosY += Rand(15);  //RandRangeInt(0, 14);
		aZombie->mPosX += Rand(15);  //RandRangeInt(0, 14);
	}
	aZombie->mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_LAWN, 0, (theGridX % 2) * 2 + theGridY * 4);

	if (theZombieType == ZombieType::ZOMBIE_BUNGEE)
	{
		aZombie->mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_GROUND, 0, 0);
		aZombie->mRow = 0;
		aZombie->mPosX = theGridX * 50.0f + 950.0f;
		aZombie->mPosY = 50.0f;
	}
	else if (theZombieType == ZombieType::ZOMBIE_BOBSLED)
	{
		aZombie->mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_LAWN, 0, 1000);
		aZombie->mRow = 0;
		aZombie->mPosX = 1105.0f;
		aZombie->mPosY = 480.0f;
	}
}

bool CutScene::CanZombieGoInGridSpot(ZombieType theZombieType, int theGridX, int theGridY, bool theZombieGrid[5][5])
{
	if (theZombieGrid[theGridX][theGridY])
	{
		return false;
	}

	if (Is2x2Zombie(theZombieType))
	{
		if (theGridX == 0 || theGridY == 0)
		{
			return false;
		}

		if (theZombieGrid[theGridX - 1][theGridY] || theZombieGrid[theGridX][theGridY - 1] || theZombieGrid[theGridX - 1][theGridY - 1])
		{
			return false;
		}
	}

	if (theGridX == 4)
	{
		if (theGridY == 0)
		{
			return false;
		}
	}
	else
	{
		if (theZombieType == ZombieType::ZOMBIE_ZAMBONI)
		{
			return false;
		}

		if (theGridX == 0 && mBoard->StageHasPool())
		{
			return false;
		}
	}

	if (mBoard->StageHasRoof() && theGridX == 0 && theGridY == 0)
	{
		return false;
	}
	if (theGridX == 4 && mBoard->StageHasFog() && theZombieType == ZombieType::ZOMBIE_BALLOON)
	{
		return false;
	}

	if (Is2x2Zombie(theZombieType) || 
		theZombieType == ZombieType::ZOMBIE_ZAMBONI || 
		theZombieType == ZombieType::ZOMBIE_BOBSLED || 
		theZombieType == ZombieType::ZOMBIE_POLEVAULTER)
	{
		if (theGridX == 0)
		{
			return false;
		}
		if (theGridX == 1 && (mBoard->StageHasPool() || theGridY == 0))
		{
			return false;
		}
	}

	return true;
}

void CutScene::FindPlaceForStreetZombies(ZombieType theZombieType, bool theZombieGrid[5][5], int& thePosX, int& thePosY)
{
	if (theZombieType == ZOMBIE_BUNGEE)
	{
		thePosX = 0;
		thePosY = 0;
		return;
	}

	int aPicksCount = 0;
	TodWeightedGridArray aPicks[25];
	for (int aGridX = 0; aGridX < 5; aGridX++)
	{
		for (int aGridY = 0; aGridY < 5; aGridY++)
		{
			if (CanZombieGoInGridSpot(theZombieType, aGridX, aGridY, theZombieGrid))
			{
				aPicks[aPicksCount].mX = aGridX;
				aPicks[aPicksCount].mY = aGridY;
				aPicks[aPicksCount].mWeight = 1;
				aPicksCount++;
			}
		}
	}

	if (aPicksCount == 0)
	{
		TodTrace("No place for street zombie!!");
		thePosX = 2;
		thePosY = 2;
	}
	else
	{
		TodWeightedGridArray* aGrid = TodPickFromWeightedGridArray(aPicks, aPicksCount);
		thePosX = aGrid->mX;
		thePosY = aGrid->mY;
	}
}

void CutScene::FindAndPlaceZombie(ZombieType theZombieType, bool theZombieGrid[5][5])
{
	int aGridX, aGridY;
	FindPlaceForStreetZombies(theZombieType, theZombieGrid, aGridX, aGridY);

	if (theZombieType != ZombieType::ZOMBIE_BUNGEE)
	{
		theZombieGrid[aGridX][aGridY] = true;
	}
	if (Is2x2Zombie(theZombieType))
	{
		TOD_ASSERT(aGridX > 0 && aGridY > 0);
		theZombieGrid[aGridX - 1][aGridY] = true;
		theZombieGrid[aGridX][aGridY - 1] = true;
		theZombieGrid[aGridX - 1][aGridY - 1] = true;
	}

	PlaceAZombie(theZombieType, aGridX, aGridY);
	if (theZombieType == ZombieType::ZOMBIE_BUNGEE && mApp->IsBungeeBlitzLevel())
	{
		PlaceAZombie(ZombieType::ZOMBIE_BUNGEE, 1, aGridY);
		PlaceAZombie(ZombieType::ZOMBIE_BUNGEE, 2, aGridY);
	}
}

bool CutScene::Is2x2Zombie(ZombieType theZombieType)
{
	return theZombieType == ZombieType::ZOMBIE_GARGANTUAR || theZombieType == ZombieType::ZOMBIE_REDEYE_GARGANTUAR;
}

// GOTY @Patoke: 0x43C840
void CutScene::PreloadResources()
{
	TodHesitationTrace("pre-CutScene::PreloadResources()");

	if (mPreloaded)
	{
		return;
	}
	mPreloaded = true;

	mLoadedResourceNames.clear();

	PerfTimer aTimer;
	aTimer.Start();

	for (int aWave = 0; aWave < mBoard->mNumWaves; aWave++)
	{
		for (int aZombieIndex = 0; aZombieIndex < MAX_ZOMBIES_IN_WAVE; aZombieIndex++)
		{
			ZombieType aZombieType = mBoard->mZombiesInWave[aWave][aZombieIndex];
			if (aZombieType == ZombieType::ZOMBIE_INVALID)
			{
				break;
			}
			Zombie::PreloadZombieResources(aZombieType);
		}
	}

	for (SeedType aSeedType = SeedType::SEED_PEASHOOTER; aSeedType < SeedType::NUM_SEED_TYPES; aSeedType = static_cast<SeedType>(static_cast<int>(aSeedType) + 1))
	{
		if (mApp->HasSeedType(aSeedType))
		{
			Plant::PreloadPlantResources(aSeedType);
		}
	}

	if (mApp->IsFirstTimeAdventureMode() && mBoard->mLevel <= 50)
	{
		Plant::PreloadPlantResources(mApp->GetAwardSeedForLevel(mBoard->mLevel));
	}

	if (mCrazyDaveDialogStart != -1)
	{
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_CRAZY_DAVE, true);
	}
	if (mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_RAKE])
	{
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_RAKE, true);
	}
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		Plant::PreloadPlantResources(SeedType::SEED_SPROUT);
		Plant::PreloadPlantResources(SeedType::SEED_MARIGOLD);
	}

	if (mBoard->StageHasRoof())
	{
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_ROOF_CLEANER, true);
	}
	else
	{
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_LAWNMOWER, true);
	}
	if (mBoard->StageHasPool())
	{
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_SPLASH, true);
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_POOL_CLEANER, true);
	}

	if (mBoard->CanDropLoot())
	{
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_COIN_SILVER, true);
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_COIN_GOLD, true);
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_DIAMOND, true);
	}

	if (mSodTime > 0)
	{
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_SODROLL, true);
	}
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_PORTAL_COMBAT)
	{
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_PORTAL_CIRCLE, true);
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_PORTAL_SQUARE, true);
	}
	if (mApp->IsWhackAZombieLevel() || mApp->IsScaryPotterLevel())
	{
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_HAMMER, true);
	}
	if (mApp->IsStormyNightLevel() || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_RAINING_SEEDS)
	{
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_RAIN_CIRCLE, true);
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_RAIN_SPLASH, true);
	}

	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_ZENGARDEN_WATERINGCAN, true);
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_ZENGARDEN_FERTILIZER, true);
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_ZENGARDEN_BUGSPRAY, true);
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_ZENGARDEN_PHONOGRAPH, true);
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_STINKY, true);
	}
	if (mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM)
	{
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_ZENGARDEN_FERTILIZER, true);
	}
	if (mApp->mGameMode == GameMode::GAMEMODE_UPSELL)
	{
		mLoadedResourceNames.push_back("DelayLoad_Background3");
		mLoadedResourceNames.push_back("DelayLoad_Background4");
		mLoadedResourceNames.push_back("DelayLoad_Background5");
		mLoadedResourceNames.push_back("DelayLoad_ChallengeScreen");
		Zombie::PreloadZombieResources(ZombieType::ZOMBIE_NORMAL);
		Zombie::PreloadZombieResources(ZombieType::ZOMBIE_TRAFFIC_CONE);
		Zombie::PreloadZombieResources(ZombieType::ZOMBIE_PAIL);
		Zombie::PreloadZombieResources(ZombieType::ZOMBIE_ZAMBONI);
		Zombie::PreloadZombieResources(ZombieType::ZOMBIE_POGO);
		Zombie::PreloadZombieResources(ZombieType::ZOMBIE_BALLOON);
		Zombie::PreloadZombieResources(ZombieType::ZOMBIE_CATAPULT);
		Plant::PreloadPlantResources(SeedType::SEED_SQUASH);
		Plant::PreloadPlantResources(SeedType::SEED_THREEPEATER);
		Plant::PreloadPlantResources(SeedType::SEED_MAGNETSHROOM);
		Plant::PreloadPlantResources(SeedType::SEED_LILYPAD);
		Plant::PreloadPlantResources(SeedType::SEED_TORCHWOOD);
		Plant::PreloadPlantResources(SeedType::SEED_SPIKEWEED);
		Plant::PreloadPlantResources(SeedType::SEED_TANGLEKELP);
		Plant::PreloadPlantResources(SeedType::SEED_SUNFLOWER);
		Plant::PreloadPlantResources(SeedType::SEED_PEASHOOTER);
		Plant::PreloadPlantResources(SeedType::SEED_SUNSHROOM);
		Plant::PreloadPlantResources(SeedType::SEED_SUNSHROOM);  // 这里不知为何原版把阳光菇预加载了两次
		Plant::PreloadPlantResources(SeedType::SEED_FLOWERPOT);
		Plant::PreloadPlantResources(SeedType::SEED_PLANTERN);
		Plant::PreloadPlantResources(SeedType::SEED_FUMESHROOM);
		Plant::PreloadPlantResources(SeedType::SEED_CACTUS);
		Plant::PreloadPlantResources(SeedType::SEED_PUFFSHROOM);
		Plant::PreloadPlantResources(SeedType::SEED_SEASHROOM);
		Plant::PreloadPlantResources(SeedType::SEED_CABBAGEPULT);
		Plant::PreloadPlantResources(SeedType::SEED_WALLNUT);
		Plant::PreloadPlantResources(SeedType::SEED_CHOMPER);
	}
	if (mApp->mGameMode == GameMode::GAMEMODE_INTRO)
	{
		mLoadedResourceNames.push_back("DelayLoad_Background3");
		mLoadedResourceNames.push_back("DelayLoad_Credits");
		Zombie::PreloadZombieResources(ZombieType::ZOMBIE_NORMAL);
		Zombie::PreloadZombieResources(ZombieType::ZOMBIE_TRAFFIC_CONE);
		Zombie::PreloadZombieResources(ZombieType::ZOMBIE_PAIL);
		Zombie::PreloadZombieResources(ZombieType::ZOMBIE_ZAMBONI);
		Plant::PreloadPlantResources(SeedType::SEED_SUNFLOWER);
		Plant::PreloadPlantResources(SeedType::SEED_PEASHOOTER);
		Plant::PreloadPlantResources(SeedType::SEED_SQUASH);
		Plant::PreloadPlantResources(SeedType::SEED_THREEPEATER);
		Plant::PreloadPlantResources(SeedType::SEED_LILYPAD);
		Plant::PreloadPlantResources(SeedType::SEED_TORCHWOOD);
		Plant::PreloadPlantResources(SeedType::SEED_SPIKEWEED);
		Plant::PreloadPlantResources(SeedType::SEED_TANGLEKELP);
	}

	for (std::string& resource : mLoadedResourceNames)
		TodLoadResources(resource.c_str());

	PlaceStreetZombies();

	mBoard->mPreloadTime = std::max(aTimer.GetDuration(), 0.0);
	TodTrace("preloading: %d ms", mBoard->mPreloadTime);
	TodHesitationTrace("CutScene::PreloadResources");
}

void CutScene::PlaceStreetZombies()
{
	if (mPlacedZombies)
		return;

	mPlacedZombies = true;
	if (mApp->IsFinalBossLevel())
		return;

	// 以下统计出怪列表中各种可预览的僵尸的数量
	// int aZombieValueTotal = 0;
	int aTotalZombieCount = 0;
	int aZombieTypeCount[ZombieType::NUM_ZOMBIE_TYPES] = { 0 };
	TOD_ASSERT(mBoard->mNumWaves <= MAX_ZOMBIE_WAVES);

	for (int aWave = 0; aWave < mBoard->mNumWaves; aWave++)
	{
		for (int aZombieIndex = 0; aZombieIndex < MAX_ZOMBIES_IN_WAVE; aZombieIndex++)
		{
			ZombieType aZombieType = mBoard->mZombiesInWave[aWave][aZombieIndex];
			if (aZombieType == ZombieType::ZOMBIE_INVALID)
			{
				break;
			}

			// aZombieValueTotal += GetZombieDefinition(aZombieType).mZombieValue;
			// (void)aZombieValueTotal; // Unused

			if (aZombieType == ZombieType::ZOMBIE_FLAG)
			{
				continue;
			}
			if (aZombieType == ZombieType::ZOMBIE_YETI && !mApp->IsStormyNightLevel())
			{
				continue;
			}
			if (aZombieType == ZombieType::ZOMBIE_BOBSLED && mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_BOBSLED_BONANZA)
			{
				continue;
			}

			TOD_ASSERT(aZombieType >= 0 && aZombieType < ZombieType::NUM_ZOMBIE_TYPES);

			++aZombieTypeCount[aZombieType];
			++aTotalZombieCount;
			if (aZombieType == ZombieType::ZOMBIE_BUNGEE || aZombieType == ZombieType::ZOMBIE_BOBSLED)
			{
				aZombieTypeCount[aZombieType] = 1;  // 蹦极僵尸和雪橇僵尸至多仅允许有 1 只预览僵尸
			}
		}
	}

	// 谁笑到最后关卡，除雪人僵尸外，所有允许出怪的僵尸类型至少计入 1 只僵尸
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND)
	{
		for (int aZombieType = 0; aZombieType < static_cast<int>(ZombieType::NUM_ZOMBIE_TYPES); aZombieType++)
		{
			if (aZombieType != ZombieType::ZOMBIE_YETI && mBoard->mZombieAllowed[aZombieType])
			{
				aZombieTypeCount[aZombieType] = std::max(aZombieTypeCount[aZombieType], 1);
			}
		}
	}
	if (mBoard->StageHasPool())
	{
		aZombieTypeCount[ZombieType::ZOMBIE_DUCKY_TUBE] = 1;  // 泳池关卡，必定出现鸭子僵尸预览
	}
	
	bool aZombieGrid[5][5] = {{ false }};
	int aPreviewCapacity = 10;
	if (mApp->IsLittleTroubleLevel())
	{
		aPreviewCapacity = 15;
	}
	else if ((mApp->IsStormyNightLevel() && mApp->IsAdventureMode()) || mApp->IsMiniBossLevel())
	{
		aPreviewCapacity = 18;
	}

	// 优先放置较大体型的僵尸，然后再放置较小体型的僵尸
	for (ZombieType aZombieType = ZombieType::ZOMBIE_NORMAL; aZombieType < ZombieType::NUM_ZOMBIE_TYPES; aZombieType = static_cast<ZombieType>(static_cast<int>(aZombieType) + 1))
	{
		if (aZombieTypeCount[aZombieType] && (Is2x2Zombie(aZombieType) || aZombieType == ZombieType::ZOMBIE_ZAMBONI))
		{
			FindAndPlaceZombie(aZombieType, aZombieGrid);
		}
	}
	for (ZombieType aZombieType = ZombieType::ZOMBIE_NORMAL; aZombieType < ZombieType::NUM_ZOMBIE_TYPES; aZombieType = static_cast<ZombieType>(static_cast<int>(aZombieType) + 1))
	{
		if (aZombieTypeCount[aZombieType] && !Is2x2Zombie(aZombieType) && aZombieType != ZombieType::ZOMBIE_ZAMBONI)
		{
			int aZombieNumInWave = aZombieTypeCount[aZombieType];
			int aZombiePreviewNum = aZombieNumInWave * aPreviewCapacity / aTotalZombieCount;
			aZombiePreviewNum = ClampInt(aZombiePreviewNum, 1, aZombieNumInWave);
			for (int i = 0; i < aZombiePreviewNum; i++)
			{
				FindAndPlaceZombie(aZombieType, aZombieGrid);
			}
		}
	}
}

void CutScene::PlaceLawnItems()
{
	if (mPlacedLawnItems)
	{
		return;
	}
	mPlacedLawnItems = true;

	if (!IsSurvivalRepick())
	{
		mBoard->InitLawnMowers();
		AddFlowerPots();
	}

	if (!IsSurvivalRepick())
	{
		mBoard->PlaceRake();
	}
}

bool CutScene::IsSurvivalRepick()
{
	return (mApp->IsSurvivalMode() && mBoard->mChallenge->mSurvivalStage > 0 && mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO); 
}

bool CutScene::IsNonScrollingCutscene()
{
	return
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ICE || 
		mApp->mGameMode == GameMode::GAMEMODE_UPSELL || 
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN ||
		mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM || 
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM || 
		mApp->IsScaryPotterLevel() || 
		mApp->IsIZombieLevel() || 
		mApp->IsWhackAZombieLevel() ||
		mApp->IsShovelLevel() || 
		mApp->IsSquirrelLevel() || 
		mApp->IsWallnutBowlingLevel();
}

bool CutScene::IsScrolledLeftAtStart()
{
	if (mBoard->mChallenge->mSurvivalStage > 0 && mApp->IsSurvivalMode())
		return false;  // 非首轮的生存模式的过场，屏幕滚动从屏幕中央开始

	return !IsNonScrollingCutscene();
}

bool CutScene::CanGetPacketUpgrade()
{
	int aCost = StoreScreen::GetItemCost(StoreItem::STORE_ITEM_PACKET_UPGRADE);

	return
		mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_PACKET_UPGRADE] == 0 &&
		mApp->mPlayerInfo->mCoins >= aCost &&
		mApp->mPlayerInfo->mDidntPurchasePacketUpgrade < 2;
}

bool CutScene::CanGetSecondPacketUpgrade()
{
	int aCost = StoreScreen::GetItemCost(StoreItem::STORE_ITEM_PACKET_UPGRADE);

	return
		mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_PACKET_UPGRADE] == 1 &&
		mApp->mPlayerInfo->mCoins >= aCost &&
		mApp->mPlayerInfo->mDidntPurchasePacketUpgrade < 2;
}

bool CutScene::CanGetPacketUpgrade(int theUpgradeIndex)
{
	int aCost = StoreScreen::GetItemCost(StoreItem::STORE_ITEM_PACKET_UPGRADE);

	return 
		mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_PACKET_UPGRADE] == theUpgradeIndex &&  // theUpgradeIndex 从首次为 0 开始计算
		mApp->mPlayerInfo->mCoins >= aCost && 
		mApp->mPlayerInfo->mDidntPurchasePacketUpgrade < 2;
}

// GOTY @Patoke: 0x43DB50
void CutScene::StartLevelIntro()
{
	mCutsceneTime = 0;
	mBoard->mSeedBank->Move(SEED_BANK_OFFSET_X, -IMAGE_SEEDBANK->GetHeight());
	mBoard->mMenuButton->mBtnNoDraw = true;
	mApp->mSeedChooserScreen->mMouseVisible = false;
	mApp->mSeedChooserScreen->Move(0, SEED_CHOOSER_OFFSET_Y);
	mApp->mSeedChooserScreen->mMenuButton->mBtnNoDraw = true;
	mBoard->mShowShovel = false;
	mBoard->mSeedBank->mCutSceneDarken = 255;
	mPlacedZombies = false;
	mPreloaded = false;
	mPlacedLawnItems = false;
	mApp->mWidgetManager->SetFocus(mBoard);

	int aLevel = mBoard->mLevel;
	if (mApp->IsFirstTimeAdventureMode() && (aLevel == 1 || aLevel == 2 || aLevel == 4))
	{
		mSodTime = TimeRollSodEnd - TimeRollSodStart;
		mBoard->mSodPosition = 0;
	}
	else
	{
		mSodTime = 0;
		mBoard->mSodPosition = 1000;
	}

	mGraveStoneTime = 0;
	mBoard->mEnableGraveStones = false;
	if (mBoard->StageHasGraveStones())
	{
		if (mApp->IsAdventureMode() && mApp->IsWhackAZombieLevel())
		{
			mGraveStoneTime = 0;
		}
		else if (!IsSurvivalRepick())
		{
			mGraveStoneTime = TimeGraveStoneEnd - TimeGraveStoneStart;
		}
	}

	if (mApp->IsFirstTimeAdventureMode() && aLevel <= 2)
	{
		mReadySetPlantTime = 0;
	}
	else if (mApp->IsShovelLevel() ||
		mApp->IsSquirrelLevel() ||
		mApp->IsWallnutBowlingLevel() ||
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM ||
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND ||
		mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM ||
		mApp->IsIZombieLevel() ||
		mApp->IsWhackAZombieLevel() ||
		mApp->IsScaryPotterLevel())
	{
		mReadySetPlantTime = 0;
	}
	else
	{
		mReadySetPlantTime = TimeReadySetPlantEnd - TimeReadySetPlantStart;
	}

	mLawnMowerTime = 0;
	if (!IsSurvivalRepick())
	{
		mLawnMowerTime = 550;
	}

	bool isRestart = false;
	if (mBoard->mPrevBoardResult == BoardResult::BOARDRESULT_LOST || mBoard->mPrevBoardResult == BoardResult::BOARDRESULT_RESTART)
	{
		isRestart = true;
	}

	if (mApp->IsFirstTimeAdventureMode() && aLevel == 11)
	{
		mCrazyDaveDialogStart = 201;
	}
	else if (mApp->IsFirstTimeAdventureMode() && aLevel == 12)
	{
		mCrazyDaveDialogStart = 1401;
	}
	else if (mApp->IsFirstTimeAdventureMode() && aLevel >= 13 && aLevel <= 24 && aLevel != 15 && aLevel != 20 && aLevel != 21 && CanGetPacketUpgrade())
	{
		mCrazyDaveDialogStart = 1501;
	}
	else if (mApp->IsFirstTimeAdventureMode() && aLevel >= 16 && aLevel <= 24 && aLevel != 20 && aLevel != 21 && CanGetSecondPacketUpgrade())
	{
		mCrazyDaveDialogStart = 1551;
	}
	else if (mApp->IsWallnutBowlingLevel() && mApp->IsAdventureMode())
	{
		if (mApp->IsFirstTimeAdventureMode())
		{
			mCrazyDaveDialogStart = 2400;
		}
		else
		{
			mCrazyDaveDialogStart = 2411;
			mBoard->mChallenge->mShowBowlingLine = true;
		}
		mBoard->mShowShovel = true;
	}
	else if (mApp->IsFirstTimeAdventureMode() && aLevel == 21)
	{
		mCrazyDaveDialogStart = 501;
	}
	else if (mApp->IsWhackAZombieLevel() && mApp->IsAdventureMode())
	{
		mCrazyDaveDialogStart = 401;
	}
	else if (mApp->IsLittleTroubleLevel() && mApp->IsAdventureMode())
	{
		mCrazyDaveDialogStart = 701;
	}
	else if (mApp->IsFirstTimeAdventureMode() && aLevel == 31)
	{
		mCrazyDaveDialogStart = 801;
	}
	else if (mApp->IsScaryPotterLevel() && mApp->IsAdventureMode())
	{
		mCrazyDaveDialogStart = 2500;
	}
	else if (mApp->IsStormyNightLevel() && mApp->IsAdventureMode())
	{
		mCrazyDaveDialogStart = 1101;
	}
	else if (mApp->IsFirstTimeAdventureMode() && aLevel == 41)
	{
		mCrazyDaveDialogStart = 1201;
	}
	else if (mApp->IsBungeeBlitzLevel() && mApp->IsAdventureMode())
	{
		mCrazyDaveDialogStart = mApp->IsFirstTimeAdventureMode() ? 1301 : 1304;
	}
	else if (!mApp->IsFirstTimeAdventureMode() && aLevel == 1)
	{
		mCrazyDaveDialogStart = 1601;
	}
	else if (mApp->mGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_1)
	{
		mCrazyDaveDialogStart = 2200;
	}
	else if (mApp->mGameMode == GameMode::GAMEMODE_UPSELL)
	{
		mCrazyDaveDialogStart = 3300;
		mUpsellHideBoard = true;
		mBoard->mMenuButton->mBtnNoDraw = false;
	}
	else if (mApp->mGameMode == GameMode::GAMEMODE_SCARY_POTTER_1 && !mApp->HasBeatenChallenge(GameMode::GAMEMODE_SCARY_POTTER_1))
	{
		mCrazyDaveDialogStart = 3000;
	}
	else if (mApp->IsFinalBossLevel() && mApp->IsAdventureMode() && !isRestart)
	{
		mCrazyDaveDialogStart = 2300;
	}
	else if (mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM)
	{
		if (mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_TREE_FOOD] < PURCHASE_COUNT_OFFSET)
		{
			mCrazyDaveDialogStart = 3200;
			mBoard->mStoreButton->mBtnNoDraw = true;
		}
	}
	if (mCrazyDaveDialogStart != -1)
	{
		mCrazyDaveTime = TimeEarlyDaveLeaveEnd - TimePanRightStart;
		if (mApp->IsFinalBossLevel() && mApp->IsAdventureMode())
		{
			mCrazyDaveTime += 4000;
		}
	}

	if (mBoard->StageHasFog())
	{
		mFogTime = TimeFogRollIn - mSodTime - mLawnMowerTime - TimeReadySetPlantStart + 2000;
	}
	else
	{
		mFogTime = 0;
	}

	if (mApp->IsFinalBossLevel())
	{
		mBossTime = 4000;
	}
	else
	{
		mBossTime = 0;
	}

	if (IsScrolledLeftAtStart())
	{
		mBoard->Move(220, 0);
	}
	if (IsNonScrollingCutscene() && mCrazyDaveTime == 0)
	{
		CancelIntro();
		return;
	}

	if (mApp->IsFinalBossLevel() || mApp->IsScaryPotterLevel() || mApp->IsWallnutBowlingLevel())
	{
		PreloadResources();
		PlaceLawnItems();
	}

	std::string aHouseMessage;
	if (mCrazyDaveTime <= 0 && mApp->mGameMode != GameMode::GAMEMODE_INTRO)
	{
		if (mApp->IsSurvivalMode())
		{
			aHouseMessage = mApp->GetCurrentChallengeDef().mChallengeName;
		}
		else if (mApp->IsAdventureMode())
		{
			if (mBoard->mBackground == BackgroundType::BACKGROUND_1_DAY || mBoard->mBackground == BackgroundType::BACKGROUND_2_NIGHT)
			{
				aHouseMessage = TodStringTranslate("[PLAYERS_HOUSE]");
			}
			else if (mBoard->mBackground == BackgroundType::BACKGROUND_3_POOL || mBoard->mBackground == BackgroundType::BACKGROUND_4_FOG)
			{
				aHouseMessage = TodStringTranslate("[PLAYERS_BACKYARD]");
			}
			else if (mBoard->mBackground == BackgroundType::BACKGROUND_5_ROOF || mBoard->mBackground == BackgroundType::BACKGROUND_6_BOSS)
			{
				aHouseMessage = TodStringTranslate("[PLAYERS_ROOF]");
			}
			else
			{
				TOD_ASSERT(false);
			}
		}
		else
		{
			aHouseMessage = mApp->GetCurrentChallengeDef().mChallengeName;
		}
	}

	aHouseMessage = TodReplaceString(aHouseMessage, "{PLAYER}", mApp->mPlayerInfo->mName);
	if (!aHouseMessage.empty())
	{
		mBoard->DisplayAdvice(aHouseMessage, MessageStyle::MESSAGE_STYLE_HOUSE_NAME, AdviceType::ADVICE_NONE);
	}

	if (mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM)
	{
		mApp->mMusic->MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_ZEN_GARDEN);
	}
	else if (mApp->mGameMode == GameMode::GAMEMODE_UPSELL)
	{
		mApp->mMusic->StopAllMusic();
	}
	else if (mApp->mGameMode == GameMode::GAMEMODE_INTRO)
	{
		mApp->mMusic->MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_POOL_WATERYGRAVES);
	}
	else if (mCrazyDaveTime > 0)
	{
		mApp->mMusic->MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_TITLE_CRAZY_DAVE_MAIN_THEME);
	}
	else if (mApp->IsFinalBossLevel())
	{
		mApp->mMusic->StopAllMusic();
	}
	else
	{
		mApp->mMusic->MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_CHOOSE_YOUR_SEEDS);
	}
}

bool CutScene::IsBeforePreloading()
{
	return mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO && !mPreloaded;
}

void CutScene::CancelIntro()
{
	PreloadResources();
	PlaceStreetZombies();
	if (mCutsceneTime < mCrazyDaveTime + TimePanRightEnd)
	{
		mCutsceneTime = TimeSeedChoserSlideOnEnd + mCrazyDaveTime - 20;
		if (!IsNonScrollingCutscene())
		{
			mBoard->Move(mApp->mWidth - BOARD_IMAGE_WIDTH_OFFSET, 0);
		}
		if (mBoard->mAdvice->mMessageStyle == MessageStyle::MESSAGE_STYLE_HOUSE_NAME)
		{
			mBoard->ClearAdvice(AdviceType::ADVICE_NONE);
		}

		if (mCrazyDaveDialogStart != -1)
		{
			if (mApp->mCrazyDaveState == CrazyDaveState::CRAZY_DAVE_OFF)
			{
				mApp->CrazyDaveEnter();
			}
			mApp->mCrazyDaveMessageIndex = mCrazyDaveDialogStart;
		}
		while (mApp->mCrazyDaveMessageIndex != -1)
		{
			AdvanceCrazyDaveDialog(true);
		}

		if (mBoard->mLevel == 5)
		{
			Plant* aPlant = nullptr;
			while (mBoard->IteratePlants(aPlant))
			{
				aPlant->Die();
			}
			mBoard->mChallenge->mShowBowlingLine = true;
		}
	}
	mApp->CrazyDaveDie();

	if (mCutsceneTime > mCrazyDaveTime + TimePanLeftStart || !mBoard->ChooseSeedsOnCurrentLevel())
	{
		// 将过场时间快进至关卡引入结束时
		mCutsceneTime = TimeIntroEnd + mLawnMowerTime + mSodTime + mGraveStoneTime + mCrazyDaveTime + mFogTime + mBossTime + mReadySetPlantTime - 20;

		PlaceLawnItems();
		if (mApp->IsStormyNightLevel())
		{
			mBoard->mChallenge->mChallengeStateCounter = 0;
		}
		if (mApp->IsFinalBossLevel())
		{
			mBoard->mChallenge->PlayBossEnter();
		}
		if (!mApp->IsChallengeWithoutSeedBank())
		{
			mBoard->mSeedBank->Move(SEED_BANK_OFFSET_X_END, 0);
		}

		mBoard->mEnableGraveStones = true;
		ShowShovel();

		if (mApp->IsFinalBossLevel())
		{
			mApp->mMusic->StartGameMusic();
		}

		if (mBoard->mFogBlownCountDown > 0)
		{
			mBoard->mFogBlownCountDown = 0;
			mBoard->mFogOffset = 0;
		}

		if (mBoard->mTutorialState != TutorialState::TUTORIAL_ZEN_GARDEN_PICKUP_WATER)
		{
			mBoard->mMenuButton->mBtnNoDraw = false;
		}
		mApp->mSoundSystem->StopFoley(FoleyType::FOLEY_DIGGER);
	}
}

void CutScene::AddGraveStoneParticles()
{
	GridItem* aGridItem = nullptr;
	while (mBoard->IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType == GridItemType::GRIDITEM_GRAVESTONE)
		{
			aGridItem->AddGraveStoneParticles();
		}
	}
}

void CutScene::AddFlowerPots()
{
	int aPotColumns = 0;
	if (mBoard->mLevel == 41)
	{
		aPotColumns = 5;
	}
	else if (mBoard->mLevel == 42)
	{
		aPotColumns = 4;
	}
	else if (mBoard->mLevel >= 43 && mBoard->mLevel <= 50)
	{
		aPotColumns = 3;
	}
	else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_COLUMN)
	{
		aPotColumns = 8;
	}
	else if (mBoard->StageHasRoof())
	{
		aPotColumns = 3;
	}

	for (int x = 0; x < aPotColumns; x++)
	{
		for (int y = 0; y < MAX_GRID_SIZE_Y; y++)
		{
			if (mBoard->CanPlantAt(x, y, SeedType::SEED_FLOWERPOT) == PlantingReason::PLANTING_OK)
			{
				mBoard->NewPlant(x, y, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
			}
		}
	}
}

int CutScene::CalcPosition(int theTimeStart, int theTimeEnd, int thePositionStart, int thePositionEnd)
{
	return TodAnimateCurve(theTimeStart, theTimeEnd, mCutsceneTime, thePositionStart, thePositionEnd, TodCurves::CURVE_EASE_IN_OUT);
}

void CutScene::AnimateBoard()
{
	int aTimePanRightStart = TimePanRightStart + mCrazyDaveTime;
	int aTimePanRightEnd = TimePanRightEnd + mCrazyDaveTime;
	int aTimePanLeftStart = TimePanLeftStart + mCrazyDaveTime;
	int aTimePanLeftEnd = TimePanLeftEnd + mCrazyDaveTime;

	// ====================================================================================================
	// ▲ 疯狂戴夫动态的更新
	// ====================================================================================================
	if (mCrazyDaveTime > 0)
	{
		if (mCutsceneTime == TimeEarlyDaveEnterStart)
		{
			mApp->CrazyDaveEnter();
			if (mApp->mGameMode == GameMode::GAMEMODE_UPSELL)
			{
				Reanimation* aDaveReanim = mApp->ReanimationTryToGet(mApp->mCrazyDaveReanimID);
				aDaveReanim->PlayReanim("anim_enterup", REANIM_PLAY_ONCE_AND_HOLD, 0, 12);
				aDaveReanim->SetPosition(150, 70);
			}
		}

		if (mCutsceneTime == TimeEarlyDaveEnterEnd && mCrazyDaveDialogStart != -1 &&
			mApp->mGameMode != GameMode::GAMEMODE_UPSELL)
		{
			mApp->CrazyDaveTalkIndex(mCrazyDaveDialogStart);
			mCrazyDaveDialogStart = -1;
		}

		if (mCutsceneTime == TimeEarlyDaveLeaveEnd && IsNonScrollingCutscene())
		{
			mCutsceneTime = aTimePanLeftEnd;
		}
	}

	// ====================================================================================================
	// ▲ 关卡界面右移的更新
	// ====================================================================================================
	int aBoardOffset = IsScrolledLeftAtStart() ? BOARD_OFFSET : 0;
	if (mCutsceneTime <= aTimePanRightStart)
	{
		mBoard->Move(aBoardOffset, 0);
	}
	if (mCutsceneTime > aTimePanRightStart && mCutsceneTime <= aTimePanRightEnd)
	{
		int aPanOffset = CalcPosition(aTimePanRightStart, aTimePanRightEnd, -aBoardOffset, BOARD_IMAGE_WIDTH_OFFSET - mApp->mWidth);
		mBoard->Move(-aPanOffset, 0);
	}
	
	// ====================================================================================================
	// ▲ 选卡界面动态的更新
	// ====================================================================================================
	if (mBoard->ChooseSeedsOnCurrentLevel())
	{
		int aTimeSeedChoserSlideOnStart = TimeSeedChoserSlideOnStart + mCrazyDaveTime;
		int aTimeSeedChoserSlideOnEnd = TimeSeedChoserSlideOnEnd + mCrazyDaveTime;
		SeedChooserScreen* aSeedChoser = mApp->mSeedChooserScreen;
		// ====================================================================================================
		// △ 选卡界面滑出
		// ====================================================================================================
		if (mCutsceneTime > aTimeSeedChoserSlideOnStart && mCutsceneTime <= aTimeSeedChoserSlideOnEnd)
		{
			aSeedChoser->Move(0, CalcPosition(aTimeSeedChoserSlideOnStart, aTimeSeedChoserSlideOnEnd, SEED_CHOOSER_OFFSET_Y, 0));
			aSeedChoser->mMenuButton->mY = CalcPosition(aTimeSeedChoserSlideOnStart, aTimeSeedChoserSlideOnEnd, -50, -10);
			aSeedChoser->mMenuButton->mBtnNoDraw = false;
		}
		// ====================================================================================================
		// △ 选卡界面滑落
		// ====================================================================================================
		int aTimeSeedChoserSlideOffStart = TimeSeedChoserSlideOffStart + mCrazyDaveTime;
		int aTimeSeedChoserSlideOffEnd = TimeSeedChoserSlideOffEnd + mCrazyDaveTime;
		if (mCutsceneTime > aTimeSeedChoserSlideOffStart && mCutsceneTime <= aTimeSeedChoserSlideOffEnd)
		{
			aSeedChoser->Move(0, CalcPosition(aTimeSeedChoserSlideOffStart, aTimeSeedChoserSlideOffEnd, 0, SEED_CHOOSER_OFFSET_Y));
			aSeedChoser->mMenuButton->mDisabled = true;
		}
	}

	// ====================================================================================================
	// ▲ 关卡界面左移的更新
	// ====================================================================================================
	if (mCutsceneTime > aTimePanLeftStart)
	{
		int aPanOffset = CalcPosition(aTimePanLeftStart, aTimePanLeftEnd, BOARD_IMAGE_WIDTH_OFFSET - mApp->mWidth, 0);
		mBoard->Move(-aPanOffset, 0);
	}

	// ====================================================================================================
	// ▲ 卡槽动态的更新
	// ====================================================================================================
	int aTimePrepareEnd = 0;
	if (!mBoard->ChooseSeedsOnCurrentLevel())
	{
		aTimePrepareEnd = mBossTime + mFogTime + mGraveStoneTime + mSodTime - TimeSeedChoserSlideOnStart + TimePanLeftEnd;
	}
	int aTimeSeedBankOnStart = TimeSeedBankOnStart + aTimePrepareEnd + mCrazyDaveTime;
	int aTimeSeedBankOnEnd = TimeSeedBankOnEnd + aTimePrepareEnd + mCrazyDaveTime;
	if (!mApp->IsChallengeWithoutSeedBank() && mCutsceneTime > aTimeSeedBankOnStart && mCutsceneTime <= aTimeSeedBankOnEnd)
	{
		int aSeedBankY = CalcPosition(aTimeSeedBankOnStart, aTimeSeedBankOnEnd, -IMAGE_SEEDBANK->GetHeight(), 0);
		mBoard->mSeedBank->Move(SEED_BANK_OFFSET_X, aSeedBankY);
	}
	int aTimeSeedBankRightStart = TimeSeedBankRightStart + mCrazyDaveTime;
	int aTimeSeedBankRightEnd = TimeSeedBankRightEnd + mCrazyDaveTime;
	if (mCutsceneTime > aTimeSeedBankRightStart)
	{
		int aSeedBankX = CalcPosition(aTimeSeedBankRightStart, aTimeSeedBankRightEnd, SEED_BANK_OFFSET_X, SEED_BANK_OFFSET_X_END);
		int aDarken = TodAnimateCurve(aTimeSeedBankRightStart, aTimeSeedBankRightEnd, mCutsceneTime, 255, 128, TodCurves::CURVE_EASE_OUT);
		mBoard->mSeedBank->mCutSceneDarken = aDarken;
		mBoard->mSeedBank->Move(aSeedBankX, mBoard->mSeedBank->mY);
	}

	// ====================================================================================================
	// ▲ 冒险模式初期关卡铺草皮的更新
	// ====================================================================================================
	if (mSodTime > 0)
	{
		int aTimeRollSodStart = TimeRollSodStart + mCrazyDaveTime;
		int aTimeRollSodEnd = TimeRollSodEnd + mCrazyDaveTime;
		mBoard->mSodPosition = TodAnimateCurve(aTimeRollSodStart, aTimeRollSodEnd, mCutsceneTime, 0, 1000, TodCurves::CURVE_LINEAR);

		if (mCutsceneTime == aTimeRollSodStart)
		{
			mApp->PlayFoley(FoleyType::FOLEY_DIGGER);
			if (mBoard->mLevel == 1)
			{
				mApp->AddReanimation(0, 0, Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_TOP, 0, 0), ReanimationType::REANIM_SODROLL);
				mApp->AddTodParticle(35, 348, Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_TOP, 0, 1), ParticleEffect::PARTICLE_SOD_ROLL);
			}
			else if (mBoard->mLevel == 2)
			{
				mApp->AddReanimation(0, -102, Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_TOP, 0, 0), ReanimationType::REANIM_SODROLL);
				mApp->AddReanimation(0, 111, Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_TOP, 0, 0), ReanimationType::REANIM_SODROLL);
				mApp->AddTodParticle(35, 246, Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_TOP, 0, 1), ParticleEffect::PARTICLE_SOD_ROLL);
				mApp->AddTodParticle(35, 459, Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_TOP, 0, 1), ParticleEffect::PARTICLE_SOD_ROLL);
			}
			else if (mBoard->mLevel == 4)
			{
				mApp->AddReanimation(-3, -198, Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_TOP, 0, 0), ReanimationType::REANIM_SODROLL);
				mApp->AddReanimation(-3, 203, Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_TOP, 0, 0), ReanimationType::REANIM_SODROLL);
				mApp->AddTodParticle(32, 150, Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_TOP, 0, 1), ParticleEffect::PARTICLE_SOD_ROLL);
				mApp->AddTodParticle(32, 511, Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_TOP, 0, 1), ParticleEffect::PARTICLE_SOD_ROLL);
			}
		}

		if (mCutsceneTime == aTimeRollSodEnd)
		{
			mApp->mSoundSystem->StopFoley(FoleyType::FOLEY_DIGGER);
		}
	}

	// ====================================================================================================
	// ▲ 黑夜关卡出现墓碑的特效的更新
	// ====================================================================================================
	if (mGraveStoneTime > 0)
	{
		int aTimeGraveStoneStart = mSodTime + TimeGraveStoneStart + mCrazyDaveTime;
		if (mCutsceneTime == aTimeGraveStoneStart)
		{
			mBoard->mEnableGraveStones = true;
			AddGraveStoneParticles();
		}
	}

	// ====================================================================================================
	// ▲ 画面开始向左滚动时，创建战场物品
	// ====================================================================================================
	if (mCutsceneTime == aTimePanLeftStart)
	{
		PlaceLawnItems();
	}

	// ====================================================================================================
	// ▲ 每一行小推车的启动的更新
	// ====================================================================================================
	if (!IsSurvivalRepick())
	{
		for (int aGridY = 0; aGridY < MAX_GRID_SIZE_Y; aGridY++)
		{
			int aTimeLawnMowerStart = TimeLawnMowerStart[aGridY] + mSodTime + mGraveStoneTime + mCrazyDaveTime;
			if (mCutsceneTime > aTimeLawnMowerStart)
			{
				LawnMower* aLawnMower = mBoard->FindLawnMowerInRow(aGridY);
				if (aLawnMower)
				{
					aLawnMower->mVisible = true;
					aLawnMower->mPosX = CalcPosition(aTimeLawnMowerStart, aTimeLawnMowerStart + TimeLawnMowerDuration, -80, -21);
				}
			}
		}
	}

	// ====================================================================================================
	// ▲ 浓雾的更新
	// ====================================================================================================
	if (mBoard->mFogBlownCountDown > 0)
	{
		int aTimeFogRollIn = TimeFogRollIn + mSodTime + mGraveStoneTime + mCrazyDaveTime;
		if (mCutsceneTime > aTimeFogRollIn)
		{
			if (mBoard->mFogBlownCountDown > 200)
			{
				mBoard->mFogBlownCountDown = 200;
			}
			mBoard->mFogBlownCountDown--;
		}
	}

	// ====================================================================================================
	// ▲ 暴风雨的更新
	// ====================================================================================================
	if (mApp->IsStormyNightLevel() && (mCutsceneTime == aTimePanRightEnd - 1000 || mCutsceneTime == aTimePanLeftEnd))
	{
		mBoard->mChallenge->mChallengeState = ChallengeState::STATECHALLENGE_STORM_FLASH_2;
		mBoard->mChallenge->mChallengeStateCounter = 310;
	}
	
	// ====================================================================================================
	// ▲ 僵王博士的入场
	// ====================================================================================================
	if (mBossTime > 0)
	{
		int aTimeBossEnter = TimeReadySetPlantStart + mLawnMowerTime + mCrazyDaveTime;
		if (mCutsceneTime == aTimeBossEnter)
		{
			mBoard->mChallenge->PlayBossEnter();
		}
	}

	// ====================================================================================================
	// ▲ 僵王博士关卡背景音乐的播放
	// ====================================================================================================
	if (mApp->IsFinalBossLevel() && mCutsceneTime == aTimeSeedBankOnStart)
	{
		mApp->mMusic->StartGameMusic();
	}

	// ====================================================================================================
	// ▲ Ready Set Plant 动画的播放
	// ====================================================================================================
	int aTimeReadySetPlant = TimeReadySetPlantStart + mLawnMowerTime + mSodTime + mGraveStoneTime + mCrazyDaveTime + mFogTime + mBossTime;
	if (mReadySetPlantTime > 0 && mCutsceneTime == aTimeReadySetPlant)
	{
		mApp->AddReanimation(400, 324, Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_SCREEN_FADE, 0, 0), ReanimationType::REANIM_READYSETPLANT);
		mApp->PlaySample(SOUND_READYSETPLANT);
		if (!mApp->IsFinalBossLevel())
		{
			mApp->mMusic->FadeOut(150);
		}
	}
	if (mReadySetPlantTime == 0 && mCutsceneTime == aTimeReadySetPlant - 2000)
	{
		if (!mApp->IsFinalBossLevel())
		{
			mApp->mMusic->FadeOut(200);
		}
	}

	// ====================================================================================================
	// ▲ 将选卡界面移动至顶层显示
	// ====================================================================================================
	mApp->mSeedChooserScreen->mParent->BringToFront(mApp->mSeedChooserScreen);
}

void CutScene::ShowShovel()
{
	if (mApp->IsWhackAZombieLevel() || 
		mApp->IsWallnutBowlingLevel() || 
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED || 
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST ||
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN || 
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM || 
		mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM ||
		mApp->IsIZombieLevel())
		return;

	if (!mApp->IsFirstTimeAdventureMode() || mBoard->mLevel > 4)
	{
		mBoard->mShowShovel = true;
	}
}

bool CutScene::IsInShovelTutorial()
{
	return
		mBoard->mTutorialState == TutorialState::TUTORIAL_SHOVEL_PICKUP || 
		mBoard->mTutorialState == TutorialState::TUTORIAL_SHOVEL_DIG || 
		mBoard->mTutorialState == TutorialState::TUTORIAL_SHOVEL_KEEP_DIGGING;
}

void CutScene::StartSeedChooser()
{
	mApp->mSeedChooserScreen->mMouseVisible = true;
	mSeedChoosing = true;
	mApp->mWidgetManager->SetFocus(mApp->mSeedChooserScreen);
}

void CutScene::EndSeedChooser()
{
	mApp->mSeedChooserScreen->mMouseVisible = false;
	mSeedChoosing = false;
	mCutsceneTime = mCrazyDaveTime + TimeSeedChoserSlideOnEnd + 10;
	mApp->mWidgetManager->SetFocus(mBoard);
}

bool CutScene::IsShowingCrazyDave()
{
	return mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO && (mCrazyDaveTime > 0 && mCutsceneTime < TimePanRightEnd + mCrazyDaveTime);
}

// GOTY @Patoke: 0x43F440
void CutScene::Update()
{
	if (mPreUpdatingBoard)
		return;

	// 更新疯狂戴夫
	if (IsShowingCrazyDave() && (!mBoard->mPaused || mApp->mGameMode != GameMode::GAMEMODE_UPSELL))
	{
		mApp->UpdateCrazyDave();
	}

	if (mBoard->mPaused)
		return;

	// 僵尸进家过场的更新
	if (mApp->mGameScene == GameScenes::SCENE_ZOMBIES_WON)
	{
		mCutsceneTime += 10;
		UpdateZombiesWon();
		return;
	}

	if (mApp->mGameScene != GameScenes::SCENE_LEVEL_INTRO || mBoard->mDrawCount == 0)
		return;

	// 进行预加载
	if (!mPreloaded)
	{
		PreloadResources();
	}
	// 放置预览僵尸
	if (!mPlacedZombies)
	{
		PlaceStreetZombies();
	}
	// 放置战场物品
	if (IsNonScrollingCutscene() || !mBoard->ChooseSeedsOnCurrentLevel())
	{
		PlaceLawnItems();
	}

	// 选卡之前的更新
	bool aCutsceneTimeStop = false;
	if (mSeedChoosing || mApp->mCrazyDaveMessageIndex != -1 || IsInShovelTutorial())
	{
		aCutsceneTimeStop = true;
	}
	if (mApp->mGameMode == GameMode::GAMEMODE_UPSELL)
	{
		UpdateUpsell();
		if (mApp->mCrazyDaveState != CrazyDaveState::CRAZY_DAVE_OFF && mApp->mCrazyDaveState != CrazyDaveState::CRAZY_DAVE_ENTERING)
		{
			aCutsceneTimeStop = true;
		}
	}
	if (mApp->mGameMode == GameMode::GAMEMODE_INTRO)
	{
		mCutsceneTime += 10;
		UpdateIntro();
		return;
	}
	if (!aCutsceneTimeStop)
	{
		mCutsceneTime += 10;
		if (mCutsceneTime == TimeSeedChoserSlideOnEnd + mCrazyDaveTime && mBoard->ChooseSeedsOnCurrentLevel())
		{
			StartSeedChooser();
		}
	}

	// 过场结束的判定
	int aTimeStart = TimeIntroEnd + mLawnMowerTime + mSodTime + mGraveStoneTime + mCrazyDaveTime + mFogTime + mBossTime + mReadySetPlantTime;
	if (mCutsceneTime >= aTimeStart)
	{
		mBoard->RemoveCutsceneZombies();
		if (mBoard->mTutorialState != TutorialState::TUTORIAL_ZEN_GARDEN_PICKUP_WATER)
		{
			mBoard->mMenuButton->mBtnNoDraw = false;
		}

		ShowShovel();
		mApp->StartPlaying();
		return;
	}

	AnimateBoard();
}

void CutScene::StartZombiesWon()
{
	mCutsceneTime = 0;
	mBoard->mMenuButton->mBtnNoDraw = true;
	mBoard->mShowShovel = false;
	mApp->mMusic->StopAllMusic();
	mBoard->StopAllZombieSounds();
	mApp->PlaySample(SOUND_LOSEMUSIC);
}

void CutScene::UpdateZombiesWon()
{
	// 画面滚动
	if (mCutsceneTime > LostTimePanRightStart && mCutsceneTime <= LostTimePanRightEnd)
	{
		mBoard->Move(CalcPosition(LostTimePanRightStart, LostTimePanRightEnd, 0, BOARD_OFFSET), 0);
	}
	
	// 啃食脑子的音效
	if (mCutsceneTime == LostTimeBrainGraphicStart - 400 || mCutsceneTime == LostTimeBrainGraphicStart - 900)
	{
		mApp->PlayFoley(FoleyType::FOLEY_CHOMP);
	}

	// 食脑的动画及惨叫的音效
	if (mCutsceneTime == LostTimeBrainGraphicStart)
	{
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_ZOMBIES_WON, true);
		int aRenderPosition = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_SCREEN_FADE, 0, 0);
		Reanimation* aReanimation = mApp->AddReanimation(-BOARD_OFFSET, 0, aRenderPosition, ReanimationType::REANIM_ZOMBIES_WON);
		aReanimation->mAnimRate = 12.0f;
		aReanimation->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
		aReanimation->GetTrackInstanceByName("fullscreen")->mTrackColor = Color::Black;
		mZombiesWonReanimID = mApp->ReanimationGetID(aReanimation);
		aReanimation->SetFramesForLayer("ZombiesWon");
		mApp->PlayFoley(FoleyType::FOLEY_SCREAM);
	}

	// 食脑动画开始抖动
	if (mCutsceneTime == LostTimeBrainGraphicShake)
	{
		mApp->ReanimationGet(mZombiesWonReanimID)->SetShakeOverride("ZombiesWon", 1.0f);
	}
	// 食脑动画结束抖动
	if (mCutsceneTime == LostTimeBrainGraphicCancelShake)
	{
		mApp->ReanimationGet(mZombiesWonReanimID)->SetShakeOverride("ZombiesWon", 0.0f);
	}
	// 食脑动画结束
	if (mCutsceneTime == LostTimeBrainGraphicEnd)
	{
		mApp->ReanimationGet(mZombiesWonReanimID)->SetFramesForLayer("anim_screen");
	}

	// 过场结束，游戏失败
	if (mCutsceneTime == LostTimeEnd)
	{
		if (mApp->IsSurvivalMode())
		{
			int aFlagsCompleted = mBoard->GetSurvivalFlagsCompleted();
			std::string aFlagsStr = mApp->Pluralize(aFlagsCompleted, "[ONE_FLAG]", "[COUNT_FLAGS]");
			std::string aStr = TodReplaceString("[SURVIVAL_DEATH_MESSAGE]", "{FLAGS}", aFlagsStr);
			GameOverDialog* aDialog = new GameOverDialog(aStr, true);
			mApp->AddDialog(Dialogs::DIALOG_GAME_OVER, aDialog);
			mApp->mWidgetManager->SetFocus(aDialog);
		}
		else
		{
			GameOverDialog* aDialog = new GameOverDialog("", false);
			mApp->AddDialog(Dialogs::DIALOG_GAME_OVER, aDialog);
			mApp->mWidgetManager->SetFocus(aDialog);
		}
	}
}

bool CutScene::IsCutSceneOver()
{
	TOD_ASSERT(mApp->mGameScene == GameScenes::SCENE_ZOMBIES_WON);
	return mCutsceneTime >= LostTimeEnd;
}

void CutScene::ZombieWonClick()
{
	if (IsCutSceneOver() || mApp->mTodCheatKeys)
	{
		mApp->EndLevel();
	}
}

void CutScene::AdvanceCrazyDaveDialog(bool theJustSkipping)
{
	if (mApp->mGameMode == GameMode::GAMEMODE_UPSELL || mApp->mCrazyDaveMessageIndex == -1)
		return;

	// “拿起铲子开始挖吧”
	if (mApp->mCrazyDaveMessageIndex == 2406 && !theJustSkipping)
	{
		mBoard->SetTutorialState(TutorialState::TUTORIAL_SHOVEL_PICKUP);
		mApp->CrazyDaveLeave();
		return;
	}
	// “这是你的智慧树，我会给你一些肥料让你开始的”
	if (mApp->mCrazyDaveMessageIndex == 3200)
	{
		mApp->mPlayerInfo->mPurchases[STORE_ITEM_TREE_FOOD] = PURCHASE_COUNT_OFFSET + 5;
		mBoard->mMenuButton->mBtnNoDraw = false;
		mBoard->mStoreButton->mBtnNoDraw = false;
	}

	// 推进戴夫对话，若不存在下一句则令戴夫退出
	if (!mApp->AdvanceCrazyDaveText())
	{
		mApp->CrazyDaveLeave();
		if (mApp->IsFinalBossLevel() && mApp->IsAdventureMode())
		{
			Reanimation* aCrazyDaveReanim = mApp->ReanimationTryToGet(mApp->mCrazyDaveReanimID);
			aCrazyDaveReanim->PlayReanim("anim_grab", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 18.0f);

			mApp->mMusic->FadeOut(50);
			if (!theJustSkipping)
			{
				mApp->PlaySample(SOUND_BUNGEE_SCREAM);
			}
		}
		else if (mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM)
		{
			mApp->mMusic->MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_ZEN_GARDEN);
		}
		else
		{
			if (mBoard->ChooseSeedsOnCurrentLevel())
			{
				mApp->mMusic->MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_CHOOSE_YOUR_SEEDS);
			}
			else if (IsNonScrollingCutscene())
			{
				mApp->mMusic->FadeOut(50);
			}
		}
		return;
	}

	// 更新为最新的一句话的编号
	int aMessageIndex = mApp->mCrazyDaveMessageIndex;
	// Now_Unused
	if (aMessageIndex == 107 || aMessageIndex == 2407)
	{
		mBoard->mChallenge->ShovelAddWallnuts();
	}
	// “并且不是铁锹，是短槌” || “我们去玩保龄球！”
	if (aMessageIndex == 405 || aMessageIndex == 2411)
	{
		mBoard->mChallenge->mShowBowlingLine = true;
	}
	// （推销卡槽）“听起来怎么样”
	if ((aMessageIndex == 1503 || aMessageIndex == 1553) && !theJustSkipping)
	{
		int aCost = StoreScreen::GetItemCost(StoreItem::STORE_ITEM_PACKET_UPGRADE);
		int aNumPackets = mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_PACKET_UPGRADE];
		std::string aBodyString = TodReplaceNumberString("[UPGRADE_DIALOG_BODY]", "{SLOTS}", aNumPackets + 7);
		std::string aAmountString = mApp->GetMoneyString(aCost);
		// 创建询问是否升级卡槽格数的对话
		Dialog* aDialog = mApp->DoDialog(Dialogs::DIALOG_PURCHASE_PACKET_SLOT, true, aAmountString, aBodyString, "", Dialog::BUTTONS_YES_NO);
		aDialog->mX += 120;
		aDialog->mY += 130;
		mBoard->ShowCoinBank(100);
		// 等待返回选择的选项
		int aResult = aDialog->WaitForResult();
		if (aResult == Dialog::ID_YES)
		{
			mApp->mPlayerInfo->AddCoins(-aCost);
			mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_PACKET_UPGRADE]++;
			mApp->WriteCurrentUserConfig();
			mBoard->mSeedBank->UpdateWidth();

			if (aMessageIndex == 1503)
			{
				mApp->CrazyDaveTalkIndex(1510);
			}
			else if (aMessageIndex == 1553)
			{
				mApp->CrazyDaveTalkIndex(1560);
			}
		}
		else
		{
			mApp->mPlayerInfo->mDidntPurchasePacketUpgrade++;
			if (aMessageIndex == 1503)
			{
				mApp->CrazyDaveTalkIndex(1520);
			}
			else if (aMessageIndex == 1553)
			{
				mApp->CrazyDaveTalkIndex(1570);
			}
		}
	}
	// “当然不是我，是你！”
	if (aMessageIndex == 406)
	{
		mBoard->mEnableGraveStones = true;
		AddGraveStoneParticles();
	}
}

void CutScene::MouseDown(int theX, int theY)
{
	(void)theX;(void)theY;
	if (mApp->mTodCheatKeys && mApp->mGameMode == GameMode::GAMEMODE_UPSELL)
	{
		if (mCrazyDaveCountDown > 1)
		{
			mCrazyDaveCountDown = 1;
		}
	}
	else
	{
		if (IsShowingCrazyDave())
		{
			AdvanceCrazyDaveDialog(false);
		}
		else if (mApp->mTodCheatKeys)
		{
			CancelIntro();
		}
	}
}

void CutScene::KeyDown(KeyCode theKey)
{
	if (mApp->mGameMode == GameMode::GAMEMODE_UPSELL)
	{
		if (mApp->mTodCheatKeys && theKey == KeyCode::KEYCODE_ESCAPE)
		{
			mCrazyDaveLastTalkIndex = 3316; // “这足够把你的脑子吹到火星，再吹回来！”
			mCrazyDaveCountDown = 1;
		}
		else if (theKey == KeyCode::KEYCODE_SPACE || theKey == KeyCode::KEYCODE_RETURN || theKey == KeyCode::KEYCODE_ESCAPE)
		{
			mApp->CrazyDaveStopSound();
			mApp->PlaySample(SOUND_PAUSE);
			mApp->mMusic->GameMusicPause(true);

			int aResult = mApp->LawnMessageBox(
				Dialogs::DIALOG_MESSAGE, 
				"[UPSELL_PAUSE_HEADER]", 
				"[UPSELL_PAUSE_BODY]", 
				"[UPSELL_RESUME_BUTTON]", 
				"[MAIN_MENU_BUTTON]", 
				Dialog::BUTTONS_YES_NO
			);
			if (aResult == Dialog::ID_NO)
			{
				mApp->KillCreditScreen();
				mApp->DoBackToMain();
			}

			mApp->mMusic->GameMusicPause(false);
		}
	}
	else
	{
		if ((theKey == KeyCode::KEYCODE_SPACE || theKey == KeyCode::KEYCODE_RETURN) && IsShowingCrazyDave())
		{
			AdvanceCrazyDaveDialog(false);
		}
		else if (mApp->mTodCheatKeys && (theKey == KeyCode::KEYCODE_SPACE || theKey == KeyCode::KEYCODE_RETURN || theKey == KeyCode::KEYCODE_ESCAPE))
		{
			CancelIntro();
		}
	}
}

int CutScene::ParseDelayTimeFromMessage()
{
	std::string aCrazyDaveText = mApp->GetCrazyDaveText(mCrazyDaveLastTalkIndex);
	size_t anIndex = aCrazyDaveText.find("{DELAY_");
	if (anIndex != std::string::npos)
	{
		std::string aDelayTimeStr = aCrazyDaveText.substr(anIndex + 7, aCrazyDaveText.find("}") - anIndex - 7);
		mCrazyDaveCountDown = atoi(aDelayTimeStr.c_str());
		return mCrazyDaveCountDown;
	}
	return 100;
}

int CutScene::ParseTalkTimeFromMessage()
{
	std::string aCrazyDaveText = mApp->GetCrazyDaveText(mCrazyDaveLastTalkIndex);
	int anIndex = aCrazyDaveText.find("{TIME_");
	if (anIndex != -1)
	{
		std::string aTalkTimeStr = aCrazyDaveText.substr(anIndex + 6, aCrazyDaveText.find("}") - anIndex - 6);
		mCrazyDaveCountDown = atoi(aTalkTimeStr.c_str());
		return mCrazyDaveCountDown;
	}
	return 100;
}

void CutScene::ClearUpsellBoard()
{
	for (int i = 0; i < MAX_GRID_SIZE_Y; i++)
	{
		mBoard->mIceTimer[i] = 0;
		mBoard->mIceMinX[i] = BOARD_WIDTH;
	}
	
	mBoard->mZombies.DataArrayFreeAll();
	mBoard->mPlants.DataArrayFreeAll();
	mBoard->mCoins.DataArrayFreeAll();
	mBoard->mProjectiles.DataArrayFreeAll();
	mBoard->mGridItems.DataArrayFreeAll();
	mBoard->mLawnMowers.DataArrayFreeAll();

	TodParticleSystem* aParticle = nullptr;
	while (mBoard->IterateParticles(aParticle))
	{
		aParticle->ParticleSystemDie();
	}
	ReanimationID aDaveReanimID = mApp->mCrazyDaveReanimID;
	ReanimationID aBlinkReanimID = mApp->mCrazyDaveBlinkReanimID;
	Reanimation* aReanim = nullptr;
	while (mBoard->IterateReanimations(aReanim))
	{
		ReanimationID aReanimID = mApp->ReanimationGetID(aReanim);
		if (aReanimID != aDaveReanimID && aReanimID != aBlinkReanimID)
		{
			aReanim->ReanimationDie();
		}
	}
	mBoard->mPoolSparklyParticleID = ParticleSystemID::PARTICLESYSTEMID_NULL;

	if (mUpsellChallengeScreen)
	{
		delete mUpsellChallengeScreen;
		mUpsellChallengeScreen = nullptr;
	}
}

void CutScene::AddUpsellZombie(ZombieType theZombieType, int thePixelX, int theGridY)
{
	Zombie* aZombie = mBoard->AddZombieInRow(theZombieType, theGridY, 0);
	aZombie->mPosX = thePixelX;
	aZombie->mPosY = aZombie->GetPosYBasedOnRow(theGridY);
	aZombie->SetRow(theGridY);
	aZombie->mX = static_cast<int>(aZombie->mPosX);
	aZombie->mY = static_cast<int>(aZombie->mPosY);
}

void CutScene::LoadIntroBoard()
{
	ClearUpsellBoard();
	mApp->mMuteSoundsForCutscene = true;

	mBoard->NewPlant(0, 1, SeedType::SEED_THREEPEATER, SeedType::SEED_NONE);
	mBoard->NewPlant(0, 2, SeedType::SEED_LILYPAD, SeedType::SEED_NONE);
	mBoard->NewPlant(0, 2, SeedType::SEED_PEASHOOTER, SeedType::SEED_NONE);
	mBoard->NewPlant(0, 3, SeedType::SEED_LILYPAD, SeedType::SEED_NONE);
	mBoard->NewPlant(0, 3, SeedType::SEED_PEASHOOTER, SeedType::SEED_NONE);
	mBoard->NewPlant(0, 4, SeedType::SEED_SUNFLOWER, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 0, SeedType::SEED_THREEPEATER, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 1, SeedType::SEED_SUNFLOWER, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 2, SeedType::SEED_LILYPAD, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 2, SeedType::SEED_SUNFLOWER, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 4, SeedType::SEED_THREEPEATER, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 5, SeedType::SEED_THREEPEATER, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 0, SeedType::SEED_SUNFLOWER, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 1, SeedType::SEED_PEASHOOTER, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 3, SeedType::SEED_LILYPAD, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 3, SeedType::SEED_PEASHOOTER, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 4, SeedType::SEED_SUNFLOWER, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 5, SeedType::SEED_SUNFLOWER, SeedType::SEED_NONE);
	mBoard->NewPlant(3, 0, SeedType::SEED_TORCHWOOD, SeedType::SEED_NONE);
	mBoard->NewPlant(3, 4, SeedType::SEED_THREEPEATER, SeedType::SEED_NONE);
	mBoard->NewPlant(4, 2, SeedType::SEED_LILYPAD, SeedType::SEED_NONE);
	mBoard->NewPlant(4, 2, SeedType::SEED_TORCHWOOD, SeedType::SEED_NONE);
	mBoard->NewPlant(5, 1, SeedType::SEED_TORCHWOOD, SeedType::SEED_NONE);
	mBoard->NewPlant(5, 4, SeedType::SEED_TORCHWOOD, SeedType::SEED_NONE);
	mBoard->NewPlant(5, 5, SeedType::SEED_TORCHWOOD, SeedType::SEED_NONE);
	mBoard->NewPlant(6, 0, SeedType::SEED_SPIKEWEED, SeedType::SEED_NONE);
	mBoard->NewPlant(6, 4, SeedType::SEED_SPIKEWEED, SeedType::SEED_NONE);
	mBoard->NewPlant(7, 1, SeedType::SEED_SPIKEWEED, SeedType::SEED_NONE);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 460, 0);
	AddUpsellZombie(ZombieType::ZOMBIE_FOOTBALL, 680, 0);
	AddUpsellZombie(ZombieType::ZOMBIE_TRAFFIC_CONE, 730, 0);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 810, 0);
	AddUpsellZombie(ZombieType::ZOMBIE_TRAFFIC_CONE, 670, 1);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 740, 1);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 880, 1);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 500, 2);
	AddUpsellZombie(ZombieType::ZOMBIE_TRAFFIC_CONE, 680, 2);
	AddUpsellZombie(ZombieType::ZOMBIE_PAIL, 604, 3);
	AddUpsellZombie(ZombieType::ZOMBIE_SNORKEL, 880, 3);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 600, 4);
	AddUpsellZombie(ZombieType::ZOMBIE_PAIL, 690, 4);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 780, 4);
	AddUpsellZombie(ZombieType::ZOMBIE_CATAPULT, 730, 5);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 590, 5);

	mPreUpdatingBoard = true;
	for (int i = 0; i < 100; i++)
	{
		mBoard->Update();
	}
	mPreUpdatingBoard = false;
}

void CutScene::LoadUpsellBoardPool()
{
	ClearUpsellBoard();
	mApp->mMuteSoundsForCutscene = true;

	mBoard->NewPlant(0, 1, SeedType::SEED_THREEPEATER, SeedType::SEED_NONE);
	mBoard->NewPlant(0, 2, SeedType::SEED_LILYPAD, SeedType::SEED_NONE);
	mBoard->NewPlant(0, 2, SeedType::SEED_PEASHOOTER, SeedType::SEED_NONE);
	mBoard->NewPlant(0, 3, SeedType::SEED_LILYPAD, SeedType::SEED_NONE);
	mBoard->NewPlant(0, 3, SeedType::SEED_PEASHOOTER, SeedType::SEED_NONE);
	mBoard->NewPlant(0, 4, SeedType::SEED_SUNFLOWER, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 0, SeedType::SEED_THREEPEATER, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 1, SeedType::SEED_SUNFLOWER, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 2, SeedType::SEED_LILYPAD, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 2, SeedType::SEED_SUNFLOWER, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 4, SeedType::SEED_THREEPEATER, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 5, SeedType::SEED_THREEPEATER, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 0, SeedType::SEED_SUNFLOWER, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 1, SeedType::SEED_PEASHOOTER, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 3, SeedType::SEED_LILYPAD, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 3, SeedType::SEED_PEASHOOTER, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 4, SeedType::SEED_SUNFLOWER, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 5, SeedType::SEED_SUNFLOWER, SeedType::SEED_NONE);
	mBoard->NewPlant(3, 4, SeedType::SEED_THREEPEATER, SeedType::SEED_NONE);
	mBoard->NewPlant(4, 0, SeedType::SEED_TORCHWOOD, SeedType::SEED_NONE);
	mBoard->NewPlant(4, 2, SeedType::SEED_LILYPAD, SeedType::SEED_NONE);
	mBoard->NewPlant(4, 2, SeedType::SEED_TORCHWOOD, SeedType::SEED_NONE);
	mBoard->NewPlant(5, 1, SeedType::SEED_TORCHWOOD, SeedType::SEED_NONE);
	mBoard->NewPlant(5, 4, SeedType::SEED_TORCHWOOD, SeedType::SEED_NONE);
	mBoard->NewPlant(5, 5, SeedType::SEED_TORCHWOOD, SeedType::SEED_NONE);
	mBoard->NewPlant(6, 0, SeedType::SEED_SPIKEWEED, SeedType::SEED_NONE);
	mBoard->NewPlant(6, 3, SeedType::SEED_TANGLEKELP, SeedType::SEED_NONE);
	mBoard->NewPlant(6, 4, SeedType::SEED_SPIKEWEED, SeedType::SEED_NONE);
	mBoard->NewPlant(6, 5, SeedType::SEED_SQUASH, SeedType::SEED_NONE);
	mBoard->NewPlant(7, 1, SeedType::SEED_SPIKEWEED, SeedType::SEED_NONE);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 460, 0);
	AddUpsellZombie(ZombieType::ZOMBIE_ZAMBONI, 680, 0);
	AddUpsellZombie(ZombieType::ZOMBIE_TRAFFIC_CONE, 670, 1);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 740, 1);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 500, 2);
	AddUpsellZombie(ZombieType::ZOMBIE_TRAFFIC_CONE, 680, 2);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 604, 3);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 690, 4);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 740, 4);
	AddUpsellZombie(ZombieType::ZOMBIE_PAIL, 730, 5);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 590, 5);

	mPreUpdatingBoard = true;
	for (int i = 0; i < 100; i++)
	{
		mBoard->Update();
	}
	mPreUpdatingBoard = false;
	mApp->mMuteSoundsForCutscene = false;
}

void CutScene::LoadUpsellBoardFog()
{
	ClearUpsellBoard();
	mApp->mMuteSoundsForCutscene = true;

	mBoard->mBackground = BackgroundType::BACKGROUND_4_FOG;
	mBoard->LoadBackgroundImages();

	mBoard->NewPlant(0, 1, SeedType::SEED_SUNSHROOM, SeedType::SEED_NONE);
	mBoard->NewPlant(0, 4, SeedType::SEED_SUNSHROOM, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 0, SeedType::SEED_SUNSHROOM, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 1, SeedType::SEED_SUNSHROOM, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 2, SeedType::SEED_LILYPAD, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 2, SeedType::SEED_CACTUS, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 4, SeedType::SEED_SUNSHROOM, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 5, SeedType::SEED_SUNSHROOM, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 0, SeedType::SEED_CACTUS, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 4, SeedType::SEED_CACTUS, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 5, SeedType::SEED_FUMESHROOM, SeedType::SEED_NONE);
	mBoard->NewPlant(3, 1, SeedType::SEED_FUMESHROOM, SeedType::SEED_NONE);
	mBoard->NewPlant(3, 2, SeedType::SEED_LILYPAD, SeedType::SEED_NONE);
	mBoard->NewPlant(3, 3, SeedType::SEED_LILYPAD, SeedType::SEED_NONE);
	mBoard->NewPlant(3, 3, SeedType::SEED_CACTUS, SeedType::SEED_NONE);
	mBoard->NewPlant(3, 5, SeedType::SEED_PUFFSHROOM, SeedType::SEED_NONE);
	mBoard->NewPlant(4, 0, SeedType::SEED_PUFFSHROOM, SeedType::SEED_NONE);
	mBoard->NewPlant(4, 1, SeedType::SEED_MAGNETSHROOM, SeedType::SEED_NONE);
	mBoard->NewPlant(4, 2, SeedType::SEED_SEASHROOM, SeedType::SEED_NONE);
	mBoard->NewPlant(4, 5, SeedType::SEED_PUFFSHROOM, SeedType::SEED_NONE);
	mBoard->NewPlant(5, 1, SeedType::SEED_PUFFSHROOM, SeedType::SEED_NONE);
	mBoard->NewPlant(5, 2, SeedType::SEED_LILYPAD, SeedType::SEED_NONE);
	mBoard->NewPlant(5, 2, SeedType::SEED_PLANTERN, SeedType::SEED_NONE);
	mBoard->NewPlant(5, 3, SeedType::SEED_SEASHROOM, SeedType::SEED_NONE);
	mBoard->NewPlant(6, 2, SeedType::SEED_SEASHROOM, SeedType::SEED_NONE);
	mBoard->NewPlant(6, 3, SeedType::SEED_SEASHROOM, SeedType::SEED_NONE);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 460, 0);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 680, 0);
	AddUpsellZombie(ZombieType::ZOMBIE_BALLOON, 780, 0);
	AddUpsellZombie(ZombieType::ZOMBIE_TRAFFIC_CONE, 670, 1);
	AddUpsellZombie(ZombieType::ZOMBIE_BALLOON, 640, 1);
	AddUpsellZombie(ZombieType::ZOMBIE_PAIL, 640, 2);
	AddUpsellZombie(ZombieType::ZOMBIE_TRAFFIC_CONE, 780, 3);
	AddUpsellZombie(ZombieType::ZOMBIE_BALLOON, 704, 4);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 690, 4);
	AddUpsellZombie(ZombieType::ZOMBIE_PAIL, 590, 5);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 740, 5);

	mPreUpdatingBoard = true;
	for (int i = 0; i < 100; i++)
	{
		mBoard->Update();
	}
	mPreUpdatingBoard = false;
	mApp->mMuteSoundsForCutscene = false;
}

void CutScene::LoadUpsellChallengeScreen()
{
	ClearUpsellBoard();
	mUpsellChallengeScreen = new ChallengeScreen(mApp, ChallengePage::CHALLENGE_PAGE_CHALLENGE);
}

void CutScene::LoadUpsellBoardRoof()
{
	ClearUpsellBoard();
	mApp->mMuteSoundsForCutscene = true;

	mBoard->mBackground = BackgroundType::BACKGROUND_5_ROOF;
	mBoard->LoadBackgroundImages();
	mBoard->mPlantRow[0] = PlantRowType::PLANTROW_NORMAL;
	mBoard->mPlantRow[1] = PlantRowType::PLANTROW_NORMAL;
	mBoard->mPlantRow[2] = PlantRowType::PLANTROW_NORMAL;
	mBoard->mPlantRow[3] = PlantRowType::PLANTROW_NORMAL;
	mBoard->mPlantRow[4] = PlantRowType::PLANTROW_NORMAL;
	mBoard->mPlantRow[5] = PlantRowType::PLANTROW_DIRT;
	for (int x = 0; x < MAX_GRID_SIZE_X; x++)
	{
		for (int y = 0; y < MAX_GRID_SIZE_Y; y++)
		{
			if (mBoard->mPlantRow[y] == PlantRowType::PLANTROW_DIRT)
			{
				mBoard->mGridSquareType[x][y] = GridSquareType::GRIDSQUARE_DIRT;
			}
			else
			{
				mBoard->mGridSquareType[x][y] = GridSquareType::GRIDSQUARE_GRASS;
			}
		}
	}

	mBoard->NewPlant(0, 0, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(0, 0, SeedType::SEED_CABBAGEPULT, SeedType::SEED_NONE);
	mBoard->NewPlant(0, 1, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(0, 1, SeedType::SEED_CABBAGEPULT, SeedType::SEED_NONE);
	mBoard->NewPlant(0, 2, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(0, 2, SeedType::SEED_SUNFLOWER, SeedType::SEED_NONE);
	mBoard->NewPlant(0, 3, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(0, 3, SeedType::SEED_SUNFLOWER, SeedType::SEED_NONE);
	mBoard->NewPlant(0, 4, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(0, 4, SeedType::SEED_CABBAGEPULT, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 0, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 0, SeedType::SEED_CABBAGEPULT, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 1, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 1, SeedType::SEED_SUNFLOWER, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 2, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 2, SeedType::SEED_CABBAGEPULT, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 3, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 3, SeedType::SEED_CABBAGEPULT, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 4, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(1, 4, SeedType::SEED_SUNFLOWER, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 0, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 0, SeedType::SEED_CABBAGEPULT, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 1, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 1, SeedType::SEED_CABBAGEPULT, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 2, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 2, SeedType::SEED_CABBAGEPULT, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 3, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 3, SeedType::SEED_SUNFLOWER, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 4, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(2, 4, SeedType::SEED_CABBAGEPULT, SeedType::SEED_NONE);
	mBoard->NewPlant(3, 1, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(3, 1, SeedType::SEED_CABBAGEPULT, SeedType::SEED_NONE);
	mBoard->NewPlant(3, 2, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(3, 2, SeedType::SEED_CABBAGEPULT, SeedType::SEED_NONE);
	mBoard->NewPlant(3, 3, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(3, 3, SeedType::SEED_SUNFLOWER, SeedType::SEED_NONE);
	mBoard->NewPlant(3, 4, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(3, 4, SeedType::SEED_CABBAGEPULT, SeedType::SEED_NONE);
	mBoard->NewPlant(4, 0, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(4, 0, SeedType::SEED_CHOMPER, SeedType::SEED_NONE);
	mBoard->NewPlant(4, 1, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(4, 1, SeedType::SEED_CHOMPER, SeedType::SEED_NONE);
	mBoard->NewPlant(4, 2, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(4, 2, SeedType::SEED_REPEATER, SeedType::SEED_NONE);
	mBoard->NewPlant(4, 3, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(5, 2, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(5, 2, SeedType::SEED_WALLNUT, SeedType::SEED_NONE);
	mBoard->NewPlant(5, 3, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(5, 3, SeedType::SEED_THREEPEATER, SeedType::SEED_NONE);
	mBoard->NewPlant(5, 4, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
	mBoard->NewPlant(5, 4, SeedType::SEED_WALLNUT, SeedType::SEED_NONE);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 460, 0);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 680, 0);
	AddUpsellZombie(ZombieType::ZOMBIE_CATAPULT, 780, 1);
	AddUpsellZombie(ZombieType::ZOMBIE_TRAFFIC_CONE, 670, 1);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 580, 0);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 540, 1);
	AddUpsellZombie(ZombieType::ZOMBIE_PAIL, 500, 1);
	AddUpsellZombie(ZombieType::ZOMBIE_PAIL, 640, 2);
	AddUpsellZombie(ZombieType::ZOMBIE_TRAFFIC_CONE, 780, 3);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 380, 3);
	AddUpsellZombie(ZombieType::ZOMBIE_CATAPULT, 704, 4);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 690, 4);
	AddUpsellZombie(ZombieType::ZOMBIE_NORMAL, 590, 4);

	mPreUpdatingBoard = true;
	for (int k = 0; k < 100; k++)
	{
		mBoard->Update();
	}
	mPreUpdatingBoard = false;
	mApp->mMuteSoundsForCutscene = false;
}

void CutScene::UpdateUpsell()
{
	if (!mBoard->mMenuButton->mIsOver && !mBoard->mStoreButton->mIsOver)
	{
		mApp->SetCursor(CURSOR_POINTER);
	}
	if (mApp->mCrazyDaveState == CrazyDaveState::CRAZY_DAVE_OFF || mApp->mCrazyDaveState == CrazyDaveState::CRAZY_DAVE_ENTERING)
		return;

	if (mCrazyDaveLastTalkIndex == -1)
	{
		mApp->CrazyDaveTalkIndex(mCrazyDaveDialogStart);
		mCrazyDaveLastTalkIndex = mCrazyDaveDialogStart;
		mCrazyDaveDialogStart = -1;
		mCrazyDaveCountDown = ParseTalkTimeFromMessage();
		return;
	}

	if (mCrazyDaveCountDown > 0)
	{
		mCrazyDaveCountDown--;
	}

	// “呃，你还等什么呢？”
	if (mCrazyDaveLastTalkIndex == 3317)
	{
		if (!mCrazyDaveCountDown)
		{
			mBoard->mStoreButton->Resize(510, 420, 210, 46);
			mBoard->mMenuButton->Resize(510, 480, 210, 46);
			mBoard->mMenuButton->mBtnNoDraw = false;
			mBoard->mStoreButton->mBtnNoDraw = false;
		}
		return;
	}
	// “你想采取行动？”
	if (mCrazyDaveLastTalkIndex == 3311 && mCrazyDaveCountDown == 90)
	{
		mApp->mMusic->MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_MINIGAME_LOONBOON);
	}

	if (mCrazyDaveCountDown != 0)
		return;

	if (mApp->mCrazyDaveMessageIndex != -1)
	{
		mCrazyDaveCountDown = ParseDelayTimeFromMessage();
		mApp->CrazyDaveStopTalking();
		return;
	}

	mApp->CrazyDaveTalkIndex(mCrazyDaveLastTalkIndex + 1);
	++mCrazyDaveLastTalkIndex;
	mCrazyDaveCountDown = ParseTalkTimeFromMessage();

	Reanimation* aCrazyDaveReanim = mApp->ReanimationTryToGet(mApp->mCrazyDaveReanimID);
	switch (mCrazyDaveLastTalkIndex)
	{
	case 3305:  // “像这个！”
	{
		Reanimation* aReanimSquash = mApp->AddReanimation(0, 0, 0, ReanimationType::REANIM_SQUASH);
		aReanimSquash->PlayReanim("anim_idle", ReanimLoopType::REANIM_LOOP, 0, 15.0f);
		AttachEffect* anAttachEffect = AttachReanim(aCrazyDaveReanim->GetTrackInstanceByName("Dave_handinghand")->mAttachmentID, aReanimSquash, 92.0f, 387.0f);
		anAttachEffect->mOffset.m00 = 1.2f;
		anAttachEffect->mOffset.m11 = 1.2f;
		aCrazyDaveReanim->Update();
		break;
	}

	case 3306:  // “还有这个！”
	{
		Reanimation* aReanimThreepeater = mApp->AddReanimation(0, 0, 0, ReanimationType::REANIM_THREEPEATER);
		aReanimThreepeater->PlayReanim("anim_idle", ReanimLoopType::REANIM_LOOP, 0, 15.0f);
		for (int i = 1; i < 4; i++)
		{
			Reanimation* aReanimHead = mApp->AddReanimation(0, 0, 0, ReanimationType::REANIM_THREEPEATER);
			aReanimHead->mLoopType = ReanimLoopType::REANIM_LOOP;
			aReanimHead->mAnimRate = aReanimThreepeater->mAnimRate;
			aReanimHead->SetFramesForLayer(StrFormat("anim_head_idle%d", i).c_str());
			aReanimHead->AttachToAnotherReanimation(aReanimThreepeater, StrFormat("anim_head%d", i).c_str());
		}
		AttachEffect* anAttachEffect = AttachReanim(aCrazyDaveReanim->GetTrackInstanceByName("Dave_body1")->mAttachmentID, aReanimThreepeater, 0.0f, 0.0f);
		TodScaleRotateTransformMatrix(anAttachEffect->mOffset, -70.0f, 260.0f, 0.5f, 1.2f, 1.2f);
		aCrazyDaveReanim->Update();
		aReanimThreepeater->Update();
		break;
	}

	case 3307:  // “过会儿，我还会添加这个！”
	{
		Reanimation* aReanimMagnet = mApp->AddReanimation(0, 0, 0, ReanimationType::REANIM_MAGNETSHROOM);
		aReanimMagnet->PlayReanim("anim_idle", ReanimLoopType::REANIM_LOOP, 0, 15.0f);
		TodScaleRotateTransformMatrix(aReanimMagnet->mOverlayMatrix, 0, 0, 0.3f, 1, 1);
		AttachEffect* anAttachEffect = AttachReanim(aCrazyDaveReanim->GetTrackInstanceByName("Dave_pot")->mAttachmentID, aReanimMagnet, 25.0f, 49.0f);
		anAttachEffect->mOffset.m00 = 1.2f;
		anAttachEffect->mOffset.m11 = 1.2f;
		aCrazyDaveReanim->Update();
		break;
	}

	case 3309:  // “因为我很疯-狂-！！！！”
		aCrazyDaveReanim->FindSubReanim(ReanimationType::REANIM_THREEPEATER)->ReanimationDie();
		aCrazyDaveReanim->FindSubReanim(ReanimationType::REANIM_MAGNETSHROOM)->ReanimationDie();
		break;

	case 3312:  // “我要给你更多战斗！”
		mApp->mMusic->MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_MINIGAME_LOONBOON);
		LoadUpsellBoardPool();
		mApp->PlaySample(SOUND_FINALWAVE);
		mUpsellHideBoard = false;
		break;

	case 3313:  // “更多的25级的战斗！”
		LoadUpsellBoardFog();
		mApp->PlaySample(SOUND_HUGE_WAVE);
		mUpsellHideBoard = false;
		break;

	case 3314:  // “40个迷你游戏&谜题！”
		LoadUpsellChallengeScreen();
		mApp->PlaySample(SOUND_FINALWAVE);
		mUpsellHideBoard = false;
		break;

	case 3315:  // “大地科塔！！！”
		ClearUpsellBoard();
		mApp->PlaySample(SOUND_FINALWAVE);
		mUpsellHideBoard = true;
		mApp->AddTodParticle(592, 240, Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_SCREEN_FADE, 0, 0), ParticleEffect::PARTICLE_PERSENT_PICK_UP_ARROW);
		break;

	case 3316:  // “这足够把你的脑子吹到火星，再吹回来！”
		LoadUpsellBoardRoof();
		mApp->PlaySample(SOUND_HUGE_WAVE);
		mUpsellHideBoard = false;
		break;

	case 3317:  // “呃，你还等什么呢？”
		ClearUpsellBoard();
		mBoard->mMenuButton->mBtnNoDraw = true;
		mUpsellHideBoard = true;
		break;
	}
}

void CutScene::DrawUpsell(Graphics* g)
{
	if (mCrazyDaveLastTalkIndex == 3315)  // “大地科塔！”
	{
		Reanimation aReanim;
		aReanim.ReanimationInitializeType(565, 360, ReanimationType::REANIM_FLOWER_POT);
		aReanim.SetFramesForLayer("anim_zengarden");
		aReanim.OverrideScale(1.3f, 1.3f);
		aReanim.Draw(g);
		mBoard->mMenuButton->Draw(g);
		aReanim.ReanimationDie();
	}

	if (mUpsellChallengeScreen)
	{
		mUpsellChallengeScreen->Draw(g);
		mBoard->mMenuButton->Draw(g);
	}
}

void CutScene::UpdateIntro()
{
	mBoard->Move(-TodAnimateCurve(TimeIntro_PanRightStart, TimeIntro_PanRightEnd, mCutsceneTime, -100, 100, TodCurves::CURVE_LINEAR), 0);

	if (mCutsceneTime == 10)
	{
		LoadIntroBoard();
	}
	if (mCutsceneTime == TimeIntro_FadeOut)
	{
		mApp->mMusic->FadeOut(250);
	}
	if (mCutsceneTime == TimeIntro_LogoEnd)
	{
		int aRenderPosition = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_TOP, 0, 0);
		mApp->AddTodParticle(400, 300, aRenderPosition, ParticleEffect::PARTICLE_SCREEN_FLASH);

		mApp->mMuteSoundsForCutscene = false;
		mApp->PlaySample(SOUND_HUGE_WAVE);
		mApp->mMuteSoundsForCutscene = true;
	}
	if (mCutsceneTime == TimeIntro_FadeOut - 200)
	{
		mApp->mMuteSoundsForCutscene = false;
		mApp->PlaySample(SOUND_SIREN);
		mApp->mMuteSoundsForCutscene = true;
	}
	if (mCutsceneTime == TimeIntro_End)
	{
		mApp->PreNewGame(GameMode::GAMEMODE_ADVENTURE, false);
	}
}

void CutScene::DrawIntro(Graphics* g)
{
	if (mCutsceneTime <= TimeIntro_PanRightStart || mCutsceneTime > TimeIntro_FadeOutEnd)
	{
		g->SetColor(Color::Black);
		g->FillRect(-mBoard->mX, -mBoard->mY, BOARD_WIDTH, BOARD_HEIGHT);
	}

	// 绘制“PopCap Games 出品”字样
	int aTimePanRightStart = TimeIntro_PanRightStart - TimeIntro_PresentsFadeIn;
	if (mCutsceneTime > TimeIntro_PresentsFadeIn && mCutsceneTime <= aTimePanRightStart)
	{
		int anAlpha = mCutsceneTime < aTimePanRightStart - 600 ?
					  TodAnimateCurve(TimeIntro_PresentsFadeIn, TimeIntro_PresentsFadeIn + 300, mCutsceneTime, 0, 255, TodCurves::CURVE_LINEAR) :
					  TodAnimateCurve(aTimePanRightStart - 600, aTimePanRightStart - 300, mCutsceneTime, 255, 0, TodCurves::CURVE_LINEAR);

		TodDrawString(
			g, 
			"[INTRO_PRESENTS]", 
			BOARD_WIDTH / 2 - mBoard->mX, 
			310 - mBoard->mY, 
			FONT_BRIANNETOD32,
			Color(255, 255, 255, anAlpha), 
			DrawStringJustification::DS_ALIGN_CENTER
		);
	}

	// 绘制“Plants Vs Zombies”的 Logo
	if (mCutsceneTime > TimeIntro_LogoStart && mCutsceneTime <= TimeIntro_PanRightEnd)
	{
		float aScale = TodAnimateCurveFloat(TimeIntro_LogoStart, TimeIntro_LogoEnd, mCutsceneTime, 5, 1, TodCurves::CURVE_EASE_OUT);
		float aCenter = aScale * 0.5;
		int aOffsetX = BOARD_WIDTH / 2 - mBoard->mX, aOffsetY = BOARD_HEIGHT / 2 - mBoard->mY;
		Rect aRect(aOffsetX - BOARD_WIDTH * aCenter, aOffsetY - 75 * aScale, BOARD_WIDTH * aScale, 150 * aScale);
		g->SetColor(Color(0, 0, 0, 128));
		g->FillRect(aRect);
		Image* aImage = IMAGE_PVZ_LOGO;
		TodDrawImageScaledF(g, aImage, aOffsetX - aImage->GetWidth() * aCenter, aOffsetY - aImage->GetHeight() * aCenter, aScale, aScale);
	}

	if (mCutsceneTime > TimeIntro_FadeOut && mCutsceneTime <= TimeIntro_FadeOutEnd)
	{
		g->SetColor(Color(0, 0, 0, TodAnimateCurve(TimeIntro_FadeOut, TimeIntro_FadeOutEnd, mCutsceneTime, 0, 255, TodCurves::CURVE_LINEAR)));
		g->FillRect(-mBoard->mX, -mBoard->mY, BOARD_WIDTH, BOARD_HEIGHT);
	}
}

bool CutScene::ShouldRunUpsellBoard()
{
	return (mApp->mGameMode == GameMode::GAMEMODE_UPSELL || mApp->mGameMode == GameMode::GAMEMODE_INTRO) && !mUpsellHideBoard;
}

bool CutScene::IsAfterSeedChooser()
{
	return mCutsceneTime > TimeSeedChoserSlideOffStart + mCrazyDaveTime;
}

bool CutScene::ShowZombieWalking()
{
	return mCutsceneTime > LostTimePanRightStart;
}
