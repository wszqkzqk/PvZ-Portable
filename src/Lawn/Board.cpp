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

#include <time.h>
#include <algorithm>
#include <SDL.h>
#include "ZenGarden.h"
#include "BoardInclude.h"
#include "System/Music.h"
#include "System/SaveGame.h"
#include "Widget/LawnDialog.h"
#include "System/PlayerInfo.h"
#include "System/PoolEffect.h"
#include "System/TypingCheck.h"
#include "Widget/StoreScreen.h"
#include "Widget/AwardScreen.h"
#include "../Sexy.TodLib/Trail.h"
#include "Widget/ChallengeScreen.h"
#include "../Sexy.TodLib/TodDebug.h"
#include "../Sexy.TodLib/TodFoley.h"
#include "Widget/SeedChooserScreen.h"
#include "../Sexy.TodLib/Attachment.h"
#include "../Sexy.TodLib/Reanimator.h"
#include "widget/Dialog.h"
#include "misc/MTRand.h"
#include "../Sexy.TodLib/TodParticle.h"
//#include "graphics/SysFont.h"
#include "../Sexy.TodLib/EffectSystem.h"
#include "../Sexy.TodLib/TodStringFile.h"
#include "graphics/ImageFont.h"
#include "sound/SoundManager.h"
#include "widget/ButtonWidget.h"
#include "widget/WidgetManager.h"
#include "sound/SoundInstance.h"

//#define SEXY_PERF_ENABLED
#include "misc/PerfTimer.h"
#include "Widget/AchievementsScreen.h"

//#define SEXY_MEMTRACE
//#include "../SexyAppFramework/memmgr.h"

bool gShownMoreSunTutorial = false;

// GOTY @Patoke: 0x40A3C0
Board::Board(LawnApp* theApp)
{
	mApp = theApp;
	mApp->mBoard = this;
	TodHesitationTrace("preboard");

	mZombies.DataArrayInitialize(1024U, "zombies");
	mPlants.DataArrayInitialize(1024U, "plants");
	mProjectiles.DataArrayInitialize(1024U, "projectiles");
	mCoins.DataArrayInitialize(1024U, "coins");
	mLawnMowers.DataArrayInitialize(32U, "lawnmowers");
	mGridItems.DataArrayInitialize(128U, "griditems");
	TodHesitationTrace("board dataarrays");

	mApp->mEffectSystem->EffectSystemFreeAll();
	mBoardRandSeed = mApp->mAppRandSeed;
	if (mApp->IsSurvivalMode())
	{
		mBoardRandSeed = Rand();
	}
	mCoinBankFadeCount = 0;
	mLevel = 0;
	mCursorObject = new CursorObject();
	mCursorPreview = new CursorPreview();
	mSeedBank = new SeedBank();
	mCutScene = new CutScene();
	mSpecialGraveStoneX = -1;
	mSpecialGraveStoneY = -1;
	for (int i = 0; i < MAX_GRID_SIZE_X; i++)
	{
		for (int j = 0; j < MAX_GRID_SIZE_Y; j++)
		{
			mGridSquareType[i][j] = GridSquareType::GRIDSQUARE_GRASS;
			mGridCelLook[i][j] = Rand(20);
			mGridCelOffset[i][j][0] = Rand(10) - 5;
			mGridCelOffset[i][j][1] = Rand(10) - 5;
		}

		for (int k = 0; k < MAX_GRID_SIZE_Y + 1; k++)
		{
			mGridCelFog[i][k] = 0;
		}
	}
	mFogOffset = 0.0f;
	mSunCountDown = 0;
	mShakeCounter = 0;
	mShakeAmountX = 0;
	mShakeAmountY = 0;
	mPaused = false;
	mLevelAwardSpawned = false;
	mFlagRaiseCounter = 0;
	mIceTrapCounter = 0;
	mLevelComplete = false;
	mBoardFadeOutCounter = -1;
	mNextSurvivalStageCounter = 0;
	mScoreNextMowerCounter = 0;
	mProgressMeterWidth = 0;
	mPoolSparklyParticleID = ParticleSystemID::PARTICLESYSTEMID_NULL;
	mFogBlownCountDown = 0;
	mFwooshCountDown = 0;
	mTimeStopCounter = 0;
	mCobCannonCursorDelayCounter = 0;
	mCobCannonMouseX = 0;
	mCobCannonMouseY = 0;
	mDroppedFirstCoin = false;
	mBonusLawnMowersRemaining = 0;
	mEnableGraveStones = false;
	mHelpIndex = AdviceType::ADVICE_NONE;
	mEffectCounter = 0;
	mDrawCount = 0;
	mRiseFromGraveCounter = 0;
	mFinalWaveSoundCounter = 0;
	mKilledYeti = false;
	mTriggeredLawnMowers = 0;
	mPlayTimeActiveLevel = 0;
	mPlayTimeInactiveLevel = 0;
	mMaxSunPlants = 0;
	mStartDrawTime = 0;
	mIntervalDrawTime = 0;
	mIntervalDrawCountStart = 0;
	mPreloadTime = 0;
	mGameID = time(0);
	mMinFPS = 1000.0f;
	mGravesCleared = 0;
	mPlantsEaten = 0;
	mPlantsShoveled = 0;
	mPeaShooterUsed = false; // @Patoke: added construct
	mCatapultPlantsUsed = false; // @Patoke: added construct
	mMushroomAndCoffeeBeansOnly = true; // @Patoke: added construct
	mMushroomsUsed = false; // @Patoke: added construct
	mLevelCoinsCollected = 0;
	mGargantuarsKillsByCornCob = 0;
	mCoinsCollected = 0;
	mDiamondsCollected = 0;
	mPottedPlantsCollected = 0;
	mChocolateCollected = 0;
	for (int y = 0; y < MAX_GRID_SIZE_Y; y++)
	{
		for (int x = 0; x < 12; x++)
		{
			mFwooshID[y][x] = ReanimationID::REANIMATIONID_NULL;
		}
	}
	mPrevMouseX = -1;
	mPrevMouseY = -1;
	mFinalBossKilled = false;
	mMustacheMode = mApp->mMustacheMode;
	mSuperMowerMode = mApp->mSuperMowerMode;
	mFutureMode = mApp->mFutureMode;
	mPinataMode = mApp->mPinataMode;
	mDanceMode = mApp->mDanceMode;
	mDaisyMode = mApp->mDaisyMode;
	mSukhbirMode = mApp->mSukhbirMode;
	mShowShovel = false;
	mToolTip = new ToolTipWidget();
	//mDebugFont = new SysFont("Arial Unicode MS", 10, true, false, false);
	mAdvice = new MessageWidget(mApp);
	mBackground = BackgroundType::BACKGROUND_1_DAY;
	mMainCounter = 0;
	mTutorialState = TutorialState::TUTORIAL_OFF;
	mTutorialTimer = -1;
	mTutorialParticleID = ParticleSystemID::PARTICLESYSTEMID_NULL;
	mChallenge = new Challenge();
	mClip = false;
	mDebugTextMode = DebugTextMode::DEBUG_TEXT_NONE;
	mMenuButton = new GameButton(0);
	mMenuButton->mDrawStoneButton = true;
	mStoreButton = nullptr;
	mIgnoreMouseUp = false;

	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN || mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM)
	{
		mMenuButton->SetLabel("[MAIN_MENU_BUTTON]");
		mMenuButton->Resize(628, -10, 163, 46);

		mStoreButton = new GameButton(1);
		mStoreButton->mButtonImage = IMAGE_ZENSHOPBUTTON;
		mStoreButton->mOverImage = IMAGE_ZENSHOPBUTTON_HIGHLIGHT;
		mStoreButton->mDownImage = IMAGE_ZENSHOPBUTTON_HIGHLIGHT;
		mStoreButton->mParentWidget = this;
		mStoreButton->Resize(678, 33, IMAGE_ZENSHOPBUTTON->mWidth, 40);
	}
	else
	{
		mMenuButton->SetLabel("[MENU_BUTTON]");
		mMenuButton->Resize(681, -10, 117, 46);
	}

	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND)
	{
		mStoreButton = new GameButton(1);
		mStoreButton->mDrawStoneButton = true;
		mStoreButton->mBtnNoDraw = true;
		mStoreButton->mDisabled = true;
	}

	if (mApp->mGameMode == GameMode::GAMEMODE_UPSELL)
	{
		mMenuButton->SetLabel("[MAIN_MENU_BUTTON]");
		mMenuButton->Resize(628, -10, 163, 46);

		mStoreButton = new GameButton(1);
		mStoreButton->mDrawStoneButton = true;
		mStoreButton->mBtnNoDraw = true;
		mStoreButton->SetLabel("[GET_FULL_VERSION_BUTTON]");
	}
}

Board::~Board()
{
	delete mAdvice;
	delete mCursorObject;
	delete mCursorPreview;
	delete mSeedBank;
	if (mMenuButton)
	{
		delete mMenuButton;
	}
	if (mStoreButton)
	{
		delete mStoreButton;
	}
	mZombies.DataArrayDispose();
	mPlants.DataArrayDispose();
	mProjectiles.DataArrayDispose();
	mCoins.DataArrayDispose();
	mLawnMowers.DataArrayDispose();
	mGridItems.DataArrayDispose();
	if (mToolTip)
	{
		delete mToolTip;
	}
	/*
	if (mDebugFont)
	{
		delete mDebugFont;
	}
	*/
	delete mCutScene;
	delete mChallenge;
}

void BoardInitForPlayer()
{
	gShownMoreSunTutorial = false;
}

// GOTY @Patoke: 0x40B320
void Board::DisposeBoard()
{
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
		mApp->mZenGarden->LeaveGarden();
	if (mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM)
		mChallenge->TreeOfWisdomLeave();

	mApp->mSoundSystem->StopFoley(FoleyType::FOLEY_RAIN);
	mApp->mZenGarden->mBoard = nullptr;
	mApp->CrazyDaveDie();
	mApp->mEffectSystem->EffectSystemFreeAll();
}

bool Board::AreEnemyZombiesOnScreen()
{
	Zombie* aZombie = nullptr;
	while (IterateZombies(aZombie))
	{
		if (aZombie->mHasHead && !aZombie->IsDeadOrDying() && !aZombie->mMindControlled)
		{
			return true;
		}
	}
	return false;
}

// GOTY @Patoke: 0x40B4A0
int Board::CountZombiesOnScreen()
{
	int aCount = 0;
	Zombie* aZombie = nullptr;
	while (IterateZombies(aZombie))
	{
		if (aZombie->mHasHead && !aZombie->IsDeadOrDying() && !aZombie->mMindControlled && aZombie->IsOnBoard())
		{
			aCount++;
		}
	}
	return aCount;
}

// GOTY @Patoke: 0x40B3B0
int Board::GetLiveGargantuarCount() {
	int aCount = 0;
	Zombie* aZombie = nullptr;
	while (IterateZombies(aZombie))
	{
		if (aZombie->mHasHead && !aZombie->IsDeadOrDying() && aZombie->IsOnBoard() && (aZombie->mZombieType == ZombieType::ZOMBIE_GARGANTUAR || aZombie->mZombieType == ZombieType::ZOMBIE_REDEYE_GARGANTUAR))
		{
			aCount++;
		}
	}
	return aCount;
}

int Board::CountUntriggerLawnMowers()
{
	int aCount = 0;
	LawnMower* aLawnMower = nullptr;
	while (IterateLawnMowers(aLawnMower))
	{
		if (aLawnMower->mMowerState != LawnMowerState::MOWER_TRIGGERED && aLawnMower->mMowerState != LawnMowerState::MOWER_SQUISHED)
		{
			aCount++;
		}
	}
	return aCount;
}

void Board::TryToSaveGame()
{
	std::string aFileName = GetSavedGameName(mApp->mGameMode, mApp->mPlayerInfo->mId);

	if (NeedSaveGame())
	{
		if (mBoardFadeOutCounter > 0)
		{
			CompleteEndLevelSequenceForSaving();
			return;
		}

		MkDir(GetAppDataPath("userdata"));
		mApp->mMusic->GameMusicPause(true);
		LawnSaveGame(this, aFileName);
		mApp->ClearUpdateBacklog();
		SurvivalSaveScore();
	}
}

bool Board::NeedSaveGame()
{
	return 
		mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_ICE && 
		mApp->mGameMode != GameMode::GAMEMODE_UPSELL && 
		mApp->mGameMode != GameMode::GAMEMODE_INTRO && 
		mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN && 
		mApp->mGameMode != GameMode::GAMEMODE_TREE_OF_WISDOM && 
		mApp->mGameScene == GameScenes::SCENE_PLAYING;
}

void Board::SaveGame(const std::string& theFileName)
{ 
	LawnSaveGame(this, theFileName);
}

// GOTY @Patoke: 0x40B739
void Board::ResetFPSStats()
{
	int64_t aTickCount = SDL_GetTicks();
	mStartDrawTime = aTickCount;
	mIntervalDrawTime = aTickCount;
	mDrawCount = 1;
	mIntervalDrawCountStart = 1;
}

// GOTY @Patoke: 0x40B710
bool Board::LoadGame(const std::string& theFileName)
{
	if (!LawnLoadGame(this, theFileName))
		return false;

	LoadBackgroundImages();
	mApp->ClearUpdateBacklog();
	ResetFPSStats();
	UpdateLayers();
	return true;
}

GridItem* Board::GetGridItemAt(GridItemType theGridItemType, int theGridX, int theGridY)
{
	GridItem* aGridItem = nullptr;
	while (IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridX == theGridX && aGridItem->mGridY == theGridY && aGridItem->mGridItemType == theGridItemType)
		{
			return aGridItem;
		}
	}
	return nullptr;
}

GridItem* Board::GetRake()
{
	GridItem* aGridItem = nullptr;
	while (IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType == GridItemType::GRIDITEM_RAKE)
		{
			return aGridItem;
		}
	}
	return nullptr;
}

GridItem* Board::GetCraterAt(int theGridX, int theGridY)
{
	return GetGridItemAt(GridItemType::GRIDITEM_CRATER, theGridX, theGridY);
}

GridItem* Board::GetGraveStoneAt(int theGridX, int theGridY)
{
	return GetGridItemAt(GridItemType::GRIDITEM_GRAVESTONE, theGridX, theGridY);
}

GridItem* Board::GetLadderAt(int theGridX, int theGridY)
{
	return GetGridItemAt(GridItemType::GRIDITEM_LADDER, theGridX, theGridY);
}

GridItem* Board::GetScaryPotAt(int theGridX, int theGridY)
{
	return GetGridItemAt(GridItemType::GRIDITEM_SCARY_POT, theGridX, theGridY);
}

/*
GridItem* Board::GetSquirrelAt(int theGridX, int theGridY)
{
	return GetGridItemAt(GridItemType::GRIDITEM_SQUIRREL, theGridX, theGridY);
}
*/

GridItem* Board::GetZenToolAt(int theGridX, int theGridY)
{
	return GetGridItemAt(GridItemType::GRIDITEM_ZEN_TOOL, theGridX, theGridY);
}

bool Board::CanAddGraveStoneAt(int theGridX, int theGridY)
{
	if (mGridSquareType[theGridX][theGridY] != GridSquareType::GRIDSQUARE_GRASS && mGridSquareType[theGridX][theGridY] != GridSquareType::GRIDSQUARE_HIGH_GROUND)
	{
		return false;
	}

	GridItem* aGridItem = nullptr;
	while (IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridX == theGridX && aGridItem->mGridY == theGridY)
		{
			if (aGridItem->mGridItemType == GridItemType::GRIDITEM_GRAVESTONE || 
				aGridItem->mGridItemType == GridItemType::GRIDITEM_CRATER || 
				aGridItem->mGridItemType == GridItemType::GRIDITEM_LADDER)
				return false;
		}
	}
	return true;
}

int Board::MakeRenderOrder(RenderLayer theRenderLayer, int theRow, int theLayerOffset)
{
	return theRow * static_cast<int>(RenderLayer::RENDER_LAYER_ROW_OFFSET) + theRenderLayer + theLayerOffset;
}

GridItem* Board::AddALadder(int theGridX, int theGridY)
{
	GridItem* aLadder = mGridItems.DataArrayAlloc();
	aLadder->mGridItemType = GridItemType::GRIDITEM_LADDER;
	aLadder->mRenderOrder = MakeRenderOrder(RenderLayer::RENDER_LAYER_PLANT, theGridY, 800);
	aLadder->mGridX = theGridX;
	aLadder->mGridY = theGridY;
	return aLadder;
}

GridItem* Board::AddACrater(int theGridX, int theGridY)
{
	GridItem* aCrater = mGridItems.DataArrayAlloc();
	aCrater->mGridItemType = GridItemType::GRIDITEM_CRATER;
	aCrater->mRenderOrder = MakeRenderOrder(RenderLayer::RENDER_LAYER_GROUND, theGridY, 1);
	aCrater->mGridX = theGridX;
	aCrater->mGridY = theGridY;
	return aCrater;
}

GridItem* Board::AddAGraveStone(int theGridX, int theGridY)
{
	GridItem* aGraveStone = mGridItems.DataArrayAlloc();
	aGraveStone->mGridItemType = GridItemType::GRIDITEM_GRAVESTONE;
	aGraveStone->mGridItemCounter = -Rand(50);
	aGraveStone->mRenderOrder = MakeRenderOrder(RenderLayer::RENDER_LAYER_GRAVE_STONE, theGridY, 3);
	aGraveStone->mGridX = theGridX;
	aGraveStone->mGridY = theGridY;
	return aGraveStone;
}

void Board::AddGraveStones(int theGridX, int theCount, MTRand& theLevelRNG)
{
	TOD_ASSERT(theCount <= MAX_GRID_SIZE_Y);

	// 这里姑且加一个原版没有的、对于本列能否生成墓碑的判断
	// 如果没有这个判断，当本列不存在足够多的格子可以放置墓碑时，游戏会卡死
	//GridItem* aGridItem = nullptr;
	//bool aAllowGraveStone[MAX_GRID_SIZE_Y] = { false };
	int aGridAllowGraveStonesCount = 0;
	for (int y = 0; y < MAX_GRID_SIZE_Y; y++)
	{
		if (CanAddGraveStoneAt(theGridX, y))
		{
			aGridAllowGraveStonesCount++;
		}
	}
	theCount = std::min(theCount, aGridAllowGraveStonesCount);

	int i = 0;
	while (i < theCount)
	{
		int aGridY = theLevelRNG.Next((unsigned long)MAX_GRID_SIZE_Y);
		//if (aAllowGraveStone[aGridY])
		//{
		//	aAllowGraveStone[aGridY] = false;
		//	GridItem* aGraveStone = AddAGraveStone(theGridX, aGridY);
		//	++i;
		//}
		// 上述写法虽然效率更高，但当 AddAGraveStone() 函数被修改后，不能保证 aAllowGraveStone 仍然有效
		// 故这里仍然采用如下的原版的写法，仅在上面对 theCount 进行修正
		if (CanAddGraveStoneAt(theGridX, aGridY))
		{
			GridItem* aGraveStone = AddAGraveStone(theGridX, aGridY);
			(void)aGraveStone; // unused
			++i;
		}
	}
}

int Board::GetNumWavesPerFlag()
{
	return (mApp->IsFirstTimeAdventureMode() && mNumWaves < 10) ? mNumWaves : 10;
}

bool Board::IsFlagWave(int theWaveNumber)
{
	if (mApp->IsFirstTimeAdventureMode() && mLevel == 1)
		return false;

	int aWavesPerFlag = GetNumWavesPerFlag();
	return theWaveNumber % aWavesPerFlag == aWavesPerFlag - 1;
}

void ZombiePickerInitForWave(ZombiePicker* theZombiePicker)
{
	theZombiePicker->mZombieCount = 0;
	theZombiePicker->mZombiePoints = 0;
	memset(theZombiePicker->mZombieTypeCount, 0, sizeof(theZombiePicker->mZombieTypeCount));
}

void ZombiePickerInit(ZombiePicker* theZombiePicker)
{
	ZombiePickerInitForWave(theZombiePicker);
	memset(theZombiePicker->mAllWavesZombieTypeCount, 0, sizeof(theZombiePicker->mAllWavesZombieTypeCount));
}

void Board::PutZombieInWave(ZombieType theZombieType, int theWaveNumber, ZombiePicker* theZombiePicker)
{
	TOD_ASSERT(theWaveNumber < MAX_ZOMBIE_WAVES && theZombiePicker->mZombieCount < MAX_ZOMBIES_IN_WAVE);
	mZombiesInWave[theWaveNumber][theZombiePicker->mZombieCount++] = theZombieType;
	if (theZombiePicker->mZombieCount < MAX_ZOMBIES_IN_WAVE)
	{
		mZombiesInWave[theWaveNumber][theZombiePicker->mZombieCount] = ZombieType::ZOMBIE_INVALID;
	}
	theZombiePicker->mZombiePoints -= GetZombieDefinition(theZombieType).mZombieValue;
	theZombiePicker->mZombieTypeCount[theZombieType]++;
	theZombiePicker->mAllWavesZombieTypeCount[theZombieType]++;
}

void Board::PutInMissingZombies(int theWaveNumber, ZombiePicker* theZombiePicker)
{
	for (ZombieType aZombieType = ZombieType::ZOMBIE_NORMAL; aZombieType < ZombieType::NUM_ZOMBIE_TYPES; aZombieType = static_cast<ZombieType>(static_cast<int>(aZombieType) + 1))
	{
		if (theZombiePicker->mZombieTypeCount[aZombieType] <= 0 && aZombieType != ZombieType::ZOMBIE_YETI && CanZombieSpawnOnLevel(aZombieType, mLevel))
		{
			PutZombieInWave(aZombieType, theWaveNumber, theZombiePicker);
		}
	}
}

void Board::PickZombieWaves()
{
	// ====================================================================================================
	// ▲ 设定关卡总波数
	// ====================================================================================================
	if (mApp->IsAdventureMode())
	{
		if (mApp->IsWhackAZombieLevel())
		{
			mNumWaves = 8;
		}
		else
		{
			mNumWaves = gZombieWaves[ClampInt(mLevel - 1, 0, 49)];
			if (!mApp->IsFirstTimeAdventureMode() && !mApp->IsMiniBossLevel())
			{
				mNumWaves = mNumWaves < 10 ? 20 : mNumWaves + 10;
			}
		}
	}
	else
	{
		GameMode aGameMode = mApp->mGameMode;
		if (mApp->IsSurvivalMode() || aGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND)
			mNumWaves = GetNumWavesPerSurvivalStage();
		else if (aGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN || aGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM || mApp->IsSquirrelLevel())
			mNumWaves = 0;
		else if (aGameMode == GameMode::GAMEMODE_CHALLENGE_WHACK_A_ZOMBIE)
			mNumWaves = 12;
		else if (aGameMode == GameMode::GAMEMODE_CHALLENGE_WALLNUT_BOWLING || aGameMode == GameMode::GAMEMODE_CHALLENGE_AIR_RAID ||
				 aGameMode == GameMode::GAMEMODE_CHALLENGE_GRAVE_DANGER || aGameMode == GameMode::GAMEMODE_CHALLENGE_HIGH_GRAVITY ||
				 aGameMode == GameMode::GAMEMODE_CHALLENGE_PORTAL_COMBAT || aGameMode == GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS ||
				 aGameMode == GameMode::GAMEMODE_CHALLENGE_INVISIGHOUL)
			mNumWaves = 20;
		else if (mApp->IsStormyNightLevel() || mApp->IsLittleTroubleLevel() || mApp->IsBungeeBlitzLevel() ||
				 aGameMode == GameMode::GAMEMODE_CHALLENGE_COLUMN || mApp->IsShovelLevel() || aGameMode == GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS_2 ||
				 aGameMode == GameMode::GAMEMODE_CHALLENGE_WALLNUT_BOWLING_2 || aGameMode == GameMode::GAMEMODE_CHALLENGE_POGO_PARTY)
			mNumWaves = 30;
		else
			mNumWaves = 40;
	}

	// ====================================================================================================
	// ▲ 一些准备工作
	// ====================================================================================================
	ZombiePicker aZombiePicker;
	ZombiePickerInit(&aZombiePicker);
	ZombieType aIntroZombieType = GetIntroducedZombieType();
	TOD_ASSERT(mNumWaves <= MAX_ZOMBIE_WAVES);

	// ====================================================================================================
	// ▲ 遍历每一波并填充每波的出怪列表
	// ====================================================================================================
	for (int aWave = 0; aWave < mNumWaves; aWave++)
	{
		ZombiePickerInitForWave(&aZombiePicker);
		mZombiesInWave[aWave][0] = ZombieType::ZOMBIE_INVALID;

		bool aIsFlagWave = IsFlagWave(aWave);
		bool aIsFinalWave = aWave == mNumWaves - 1;

		if (mApp->IsBungeeBlitzLevel() && aIsFlagWave)
		{
			// 蹦极闪电战关卡的每大波固定刷出 5 只蹦极僵尸
			for (int _i = 0; _i < 5; _i++)
				PutZombieInWave(ZombieType::ZOMBIE_BUNGEE, aWave, &aZombiePicker);

			if (!aIsFinalWave)
				continue;
		}

		// ------------------------------------------------------------------------------------------------
		// △ 计算该波的僵尸总点数
		// ------------------------------------------------------------------------------------------------
		int& aZombiePoints = aZombiePicker.mZombiePoints;
		// 根据关卡计算本波的基础僵尸点数
		if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND)
		{
			aZombiePoints = (mChallenge->mSurvivalStage * GetNumWavesPerSurvivalStage() + aWave + 10) * 2 / 5 + 1;
		}
		else if (mApp->IsSurvivalMode() && mChallenge->mSurvivalStage > 0)
		{
			aZombiePoints = (mChallenge->mSurvivalStage * GetNumWavesPerSurvivalStage() + aWave) * 2 / 5 + 1;
		}
		else if (mApp->IsAdventureMode() && mApp->HasFinishedAdventure() && mLevel != 5)
		{
			aZombiePoints = aWave * 2 / 5 + 1;
		}
		else
		{
			aZombiePoints = aWave / 3 + 1;
		}

		// 旗帜波的特殊调整
		if (aIsFlagWave)
		{
			int aPlainZombiesNum = std::min(aZombiePoints, 8);
			aZombiePoints *= 2.5f;

			if (mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS && mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS_2)
			{
				for (int _i = 0; _i < aPlainZombiesNum; _i++)
				{
					PutZombieInWave(ZombieType::ZOMBIE_NORMAL, aWave, &aZombiePicker);
				}
				PutZombieInWave(ZombieType::ZOMBIE_FLAG, aWave, &aZombiePicker);
			}
		}

		// 部分关卡的多倍出怪
		if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_COLUMN)
		{
			aZombiePoints *= 6;
		}
		else if (mApp->IsLittleTroubleLevel() || mApp->IsWallnutBowlingLevel())
		{
			aZombiePoints *= 4;
		}
		else if (mApp->IsMiniBossLevel())
		{
			aZombiePoints *= 3;
		}
		else if (mApp->IsStormyNightLevel() && mApp->IsAdventureMode())
		{
			aZombiePoints *= 3;
		}
		else if (mApp->IsShovelLevel() || mApp->IsBungeeBlitzLevel() || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_PORTAL_COMBAT || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_INVISIGHOUL)
		{
			aZombiePoints *= 2;
		}
		
		// ------------------------------------------------------------------------------------------------
		// △ 向出怪列表中加入固定刷出的僵尸
		// ------------------------------------------------------------------------------------------------
		// 部分新出现的僵尸会在特定波固定刷出
		if (aIntroZombieType != ZombieType::ZOMBIE_INVALID && aIntroZombieType != ZombieType::ZOMBIE_DUCKY_TUBE)
		{
			bool aSpawnIntro = false;
			if ((aIntroZombieType == ZombieType::ZOMBIE_DIGGER || aIntroZombieType == ZombieType::ZOMBIE_BALLOON))
			{
				if (aWave + 1== 7 || aIsFinalWave)
				{
					aSpawnIntro = true;
				}
			}
			else if (aIntroZombieType == ZombieType::ZOMBIE_YETI)
			{
				if (aWave == mNumWaves / 2 && !mApp->mSawYeti)
				{
					aSpawnIntro = true;
				}
			}
			else if (aWave == mNumWaves / 2 || aIsFinalWave)
			{
				aSpawnIntro = true;
			}

			if (aSpawnIntro)
			{
				PutZombieInWave(aIntroZombieType, aWave, &aZombiePicker);
			}
		}

		// 5-10 关卡的最后一波加入一只伽刚特尔
		if (mLevel == 50 && aIsFinalWave)
		{
			PutZombieInWave(ZombieType::ZOMBIE_GARGANTUAR, aWave, &aZombiePicker);
		}
		// 冒险模式关卡的最后一波会出现本关卡可能出现的所有僵尸
		if (mApp->IsAdventureMode() && aIsFinalWave)
		{
			PutInMissingZombies(aWave, &aZombiePicker);
		}
		// 柱子关卡的特殊出怪
		if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_COLUMN)
		{
			// 每大波的第 5 小波，固定出现 10 只扶梯僵尸
			if (aWave % 10 == 5)
			{
				for (int _i = 0; _i < 10; _i++)
				{
					PutZombieInWave(ZombieType::ZOMBIE_LADDER, aWave, &aZombiePicker);
				}
			}

			// 每大波的第 8 小波，固定出现 10 只玩偶匣僵尸
			if (aWave % 10 == 8)
			{
				for (int _i = 0; _i < 10; _i++)
				{
					PutZombieInWave(ZombieType::ZOMBIE_JACK_IN_THE_BOX, aWave, &aZombiePicker);
				}
			}

			// 第 19/29 小波，固定出现 3/5 只伽刚特尔
			if (aWave == 19)
			{
				for (int _i = 0; _i < 3; _i++)
				{
					PutZombieInWave(ZombieType::ZOMBIE_GARGANTUAR, aWave, &aZombiePicker);
				}
			}
			if (aWave == 29)
			{
				for (int _i = 0; _i < 5; _i++)
				{
					PutZombieInWave(ZombieType::ZOMBIE_GARGANTUAR, aWave, &aZombiePicker);
				}
			}
		}
		
		// ------------------------------------------------------------------------------------------------
		// △ 剩余的僵尸点数用于向列表中补充随机僵尸
		// ------------------------------------------------------------------------------------------------
		while (aZombiePoints > 0 && aZombiePicker.mZombieCount < MAX_ZOMBIES_IN_WAVE)
		{
			ZombieType aZombieType = PickZombieType(aZombiePoints, aWave, &aZombiePicker);
			PutZombieInWave(aZombieType, aWave, &aZombiePicker);
		}
	}
}

int Board::GetLevelRandSeed()
{
	int aRndSeed = mApp->mPlayerInfo->mId + mBoardRandSeed;
	if (mApp->IsAdventureMode())
	{
		aRndSeed += mApp->mPlayerInfo->mFinishedAdventure * 101 + mLevel;
	}
	else
	{
		aRndSeed += mChallenge->mSurvivalStage * 101 + mApp->mGameMode;
	}
	return aRndSeed;
}

// GOTY @Patoke: 0x40C9F0
void Board::LoadBackgroundImages()
{
	switch (mBackground)
	{
	case BackgroundType::BACKGROUND_1_DAY:
		mLoadedResourceNames.push_back("DelayLoad_Background1");
		if ((mApp->IsAdventureMode() && mLevel <= 4) || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_RESODDED)
		{
			mLoadedResourceNames.push_back("DelayLoad_BackgroundUnsodded");
		}
		break;

	case BackgroundType::BACKGROUND_2_NIGHT:
		mLoadedResourceNames.push_back("DelayLoad_Background2");
		break;

	case BackgroundType::BACKGROUND_3_POOL:
		mLoadedResourceNames.push_back("DelayLoad_Background3");
		break;

	case BackgroundType::BACKGROUND_4_FOG:
		mLoadedResourceNames.push_back("DelayLoad_Background4");
		break;

	case BackgroundType::BACKGROUND_5_ROOF:
		mLoadedResourceNames.push_back("DelayLoad_Background5");
		break;

	case BackgroundType::BACKGROUND_6_BOSS:
		mLoadedResourceNames.push_back("DelayLoad_Background6");
		break;

	case BackgroundType::BACKGROUND_GREENHOUSE:
		mLoadedResourceNames.push_back("DelayLoad_GreenHouseGarden");
		mLoadedResourceNames.push_back("DelayLoad_GreenHouseOverlay");
		break;

	case BackgroundType::BACKGROUND_TREEOFWISDOM:
		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_TREEOFWISDOM, true);
		break;

	case BackgroundType::BACKGROUND_ZOMBIQUARIUM:
		mLoadedResourceNames.push_back("DelayLoad_Zombiquarium");
		mLoadedResourceNames.push_back("DelayLoad_GreenHouseOverlay");
		break;

	case BackgroundType::BACKGROUND_MUSHROOM_GARDEN:
		mLoadedResourceNames.push_back("DelayLoad_MushroomGarden");
		break;

	default:
		TOD_ASSERT(false);
		break;
	}

	for (std::string& resource : mLoadedResourceNames)
		TodLoadResources(resource.c_str());
}

void Board::PickBackground()
{
	switch (mApp->mGameMode)
	{
	case GameMode::GAMEMODE_ADVENTURE:
		if (mLevel <= 1 * LEVELS_PER_AREA)
		{
			mBackground = BackgroundType::BACKGROUND_1_DAY;
		}
		else if (mLevel <= 2 * LEVELS_PER_AREA)
		{
			mBackground = BackgroundType::BACKGROUND_2_NIGHT;
		}
		else if (mLevel <= 3 * LEVELS_PER_AREA)
		{
			mBackground = BackgroundType::BACKGROUND_3_POOL;
		}
		else if (mApp->IsScaryPotterLevel())
		{
			mBackground = BackgroundType::BACKGROUND_2_NIGHT;
		}
		else if (mLevel <= 4 * LEVELS_PER_AREA)
		{
			mBackground = BackgroundType::BACKGROUND_4_FOG;
		}
		else if (mLevel < FINAL_LEVEL)
		{
			mBackground = BackgroundType::BACKGROUND_5_ROOF;
		}
		else if (mLevel == FINAL_LEVEL)
		{
			mBackground = BackgroundType::BACKGROUND_6_BOSS;
		}
		else
		{
			mBackground = BackgroundType::BACKGROUND_1_DAY;
		}
		break;

	case GameMode::GAMEMODE_SURVIVAL_NORMAL_STAGE_1:
	case GameMode::GAMEMODE_SURVIVAL_HARD_STAGE_1:
	case GameMode::GAMEMODE_SURVIVAL_ENDLESS_STAGE_1:
	case GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS:
	case GameMode::GAMEMODE_CHALLENGE_WALLNUT_BOWLING:
	case GameMode::GAMEMODE_CHALLENGE_SLOT_MACHINE:
	case GameMode::GAMEMODE_CHALLENGE_SEEING_STARS:
	case GameMode::GAMEMODE_CHALLENGE_WALLNUT_BOWLING_2:
	case GameMode::GAMEMODE_CHALLENGE_ART_CHALLENGE_WALLNUT:
	case GameMode::GAMEMODE_CHALLENGE_SUNNY_DAY:
	case GameMode::GAMEMODE_CHALLENGE_RESODDED:
	case GameMode::GAMEMODE_CHALLENGE_BIG_TIME:
	case GameMode::GAMEMODE_CHALLENGE_ART_CHALLENGE_SUNFLOWER:
	case GameMode::GAMEMODE_CHALLENGE_ICE:
	case GameMode::GAMEMODE_CHALLENGE_SHOVEL:
	case GameMode::GAMEMODE_CHALLENGE_SQUIRREL:
		mBackground = BackgroundType::BACKGROUND_1_DAY;
		break;

	case GameMode::GAMEMODE_SURVIVAL_NORMAL_STAGE_2:
	case GameMode::GAMEMODE_SURVIVAL_HARD_STAGE_2:
	case GameMode::GAMEMODE_SURVIVAL_ENDLESS_STAGE_2:
	case GameMode::GAMEMODE_CHALLENGE_BEGHOULED:
	case GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST:
	case GameMode::GAMEMODE_CHALLENGE_PORTAL_COMBAT:
	case GameMode::GAMEMODE_CHALLENGE_WHACK_A_ZOMBIE:
	case GameMode::GAMEMODE_CHALLENGE_GRAVE_DANGER:
	case GameMode::GAMEMODE_SCARY_POTTER_1:
	case GameMode::GAMEMODE_SCARY_POTTER_2:
	case GameMode::GAMEMODE_SCARY_POTTER_3:
	case GameMode::GAMEMODE_SCARY_POTTER_4:
	case GameMode::GAMEMODE_SCARY_POTTER_5:
	case GameMode::GAMEMODE_SCARY_POTTER_6:
	case GameMode::GAMEMODE_SCARY_POTTER_7:
	case GameMode::GAMEMODE_SCARY_POTTER_8:
	case GameMode::GAMEMODE_SCARY_POTTER_9:
	case GameMode::GAMEMODE_SCARY_POTTER_ENDLESS:
	case GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_1:
	case GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_2:
	case GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_3:
	case GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_4:
	case GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_5:
	case GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_6:
	case GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_7:
	case GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_8:
	case GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_9:
	case GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_ENDLESS:
		mBackground = BackgroundType::BACKGROUND_2_NIGHT;
		break;

	case GameMode::GAMEMODE_SURVIVAL_NORMAL_STAGE_3:
	case GameMode::GAMEMODE_SURVIVAL_HARD_STAGE_3:
	case GameMode::GAMEMODE_SURVIVAL_ENDLESS_STAGE_3:
	case GameMode::GAMEMODE_CHALLENGE_LITTLE_TROUBLE:
	case GameMode::GAMEMODE_CHALLENGE_BOBSLED_BONANZA:
	case GameMode::GAMEMODE_CHALLENGE_SPEED:
	case GameMode::GAMEMODE_CHALLENGE_LAST_STAND:
	case GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS_2:
	case GameMode::GAMEMODE_UPSELL:
	case GameMode::GAMEMODE_INTRO:
		mBackground = BackgroundType::BACKGROUND_3_POOL;
		break;

	case GameMode::GAMEMODE_SURVIVAL_NORMAL_STAGE_4:
	case GameMode::GAMEMODE_SURVIVAL_HARD_STAGE_4:
	case GameMode::GAMEMODE_SURVIVAL_ENDLESS_STAGE_4:
	case GameMode::GAMEMODE_CHALLENGE_RAINING_SEEDS:
	case GameMode::GAMEMODE_CHALLENGE_INVISIGHOUL:
	case GameMode::GAMEMODE_CHALLENGE_AIR_RAID:
	case GameMode::GAMEMODE_CHALLENGE_STORMY_NIGHT:
		mBackground = BackgroundType::BACKGROUND_4_FOG;
		break;

	case GameMode::GAMEMODE_SURVIVAL_NORMAL_STAGE_5:
	case GameMode::GAMEMODE_SURVIVAL_HARD_STAGE_5:
	case GameMode::GAMEMODE_SURVIVAL_ENDLESS_STAGE_5:
	case GameMode::GAMEMODE_CHALLENGE_COLUMN:
	case GameMode::GAMEMODE_CHALLENGE_POGO_PARTY:
	case GameMode::GAMEMODE_CHALLENGE_HIGH_GRAVITY:
	case GameMode::GAMEMODE_CHALLENGE_BUNGEE_BLITZ:
		mBackground = BackgroundType::BACKGROUND_5_ROOF;
		break;

	case GameMode::GAMEMODE_CHALLENGE_FINAL_BOSS:
		mBackground = BackgroundType::BACKGROUND_6_BOSS;
		break;

	case GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM:
		mBackground = BackgroundType::BACKGROUND_ZOMBIQUARIUM;
		break;

	case GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN:
		mBackground = BackgroundType::BACKGROUND_GREENHOUSE;
		break;

	case GameMode::GAMEMODE_TREE_OF_WISDOM:
		mBackground = BackgroundType::BACKGROUND_TREEOFWISDOM;
		break;

	default:
		TOD_ASSERT(false);
		break;
	}
	LoadBackgroundImages();

	if (mBackground == BackgroundType::BACKGROUND_1_DAY || mBackground == BackgroundType::BACKGROUND_GREENHOUSE || mBackground == BackgroundType::BACKGROUND_TREEOFWISDOM)
	{
		mPlantRow[0] = PlantRowType::PLANTROW_NORMAL;
		mPlantRow[1] = PlantRowType::PLANTROW_NORMAL;
		mPlantRow[2] = PlantRowType::PLANTROW_NORMAL;
		mPlantRow[3] = PlantRowType::PLANTROW_NORMAL;
		mPlantRow[4] = PlantRowType::PLANTROW_NORMAL;
		mPlantRow[5] = PlantRowType::PLANTROW_DIRT;

		if (mApp->IsAdventureMode() && mApp->IsFirstTimeAdventureMode())
		{
			if (mLevel == 1)
			{
				mPlantRow[0] = PlantRowType::PLANTROW_DIRT;
				mPlantRow[1] = PlantRowType::PLANTROW_DIRT;
				mPlantRow[3] = PlantRowType::PLANTROW_DIRT;
				mPlantRow[4] = PlantRowType::PLANTROW_DIRT;
			}
			else if (mLevel == 2 || mLevel == 3)
			{
				mPlantRow[0] = PlantRowType::PLANTROW_DIRT;
				mPlantRow[4] = PlantRowType::PLANTROW_DIRT;
			}
		}
		else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_RESODDED)
		{
			mPlantRow[0] = PlantRowType::PLANTROW_DIRT;
			mPlantRow[4] = PlantRowType::PLANTROW_DIRT;
		}
	}
	else if (mBackground == BackgroundType::BACKGROUND_2_NIGHT)
	{
		mPlantRow[0] = PlantRowType::PLANTROW_NORMAL;
		mPlantRow[1] = PlantRowType::PLANTROW_NORMAL;
		mPlantRow[2] = PlantRowType::PLANTROW_NORMAL;
		mPlantRow[3] = PlantRowType::PLANTROW_NORMAL;
		mPlantRow[4] = PlantRowType::PLANTROW_NORMAL;
		mPlantRow[5] = PlantRowType::PLANTROW_DIRT;
	}
	else if (mBackground == BackgroundType::BACKGROUND_3_POOL || mBackground == BackgroundType::BACKGROUND_ZOMBIQUARIUM || mBackground == BackgroundType::BACKGROUND_4_FOG)
	{
		mPlantRow[0] = PlantRowType::PLANTROW_NORMAL;
		mPlantRow[1] = PlantRowType::PLANTROW_NORMAL;
		mPlantRow[2] = PlantRowType::PLANTROW_POOL;
		mPlantRow[3] = PlantRowType::PLANTROW_POOL;
		mPlantRow[4] = PlantRowType::PLANTROW_NORMAL;
		mPlantRow[5] = PlantRowType::PLANTROW_NORMAL;
	}
	else if (mBackground == BackgroundType::BACKGROUND_5_ROOF || mBackground == BackgroundType::BACKGROUND_6_BOSS)
	{
		mPlantRow[0] = PlantRowType::PLANTROW_NORMAL;
		mPlantRow[1] = PlantRowType::PLANTROW_NORMAL;
		mPlantRow[2] = PlantRowType::PLANTROW_NORMAL;
		mPlantRow[3] = PlantRowType::PLANTROW_NORMAL;
		mPlantRow[4] = PlantRowType::PLANTROW_NORMAL;
		mPlantRow[5] = PlantRowType::PLANTROW_DIRT;
	}
	else
	{
		TOD_ASSERT(false);
	}

	for (int x = 0; x < MAX_GRID_SIZE_X; x++)
	{
		for (int y = 0; y < MAX_GRID_SIZE_Y; y++)
		{
			if (mPlantRow[y] == PlantRowType::PLANTROW_DIRT)
			{
				mGridSquareType[x][y] = GridSquareType::GRIDSQUARE_DIRT;
			}
			else if (mPlantRow[y] == PlantRowType::PLANTROW_POOL && x >= 0 && x <= 8)
			{
				mGridSquareType[x][y] = GridSquareType::GRIDSQUARE_POOL;
			}
			else if (mPlantRow[y] == PlantRowType::PLANTROW_HIGH_GROUND && x >= 4 && x <= 8)
			{
				mGridSquareType[x][y] = GridSquareType::GRIDSQUARE_HIGH_GROUND;
			}
		}
	}

	MTRand aLevelRNG(GetLevelRandSeed());
	if (StageHasGraveStones())
	{
		if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_GRAVE_DANGER)
		{
			AddGraveStones(6, RandRangeInt(1, 2), aLevelRNG);
			AddGraveStones(7, RandRangeInt(1, 3), aLevelRNG);
			AddGraveStones(8, RandRangeInt(2, 3), aLevelRNG);
		}
		else if (mApp->IsWhackAZombieLevel())
		{
			mChallenge->WhackAZombiePlaceGraves(9);
		}
		else if (mBackground == BackgroundType::BACKGROUND_2_NIGHT)
		{
			if (mApp->IsSurvivalNormal(mApp->mGameMode))
			{
				AddGraveStones(5, 1, aLevelRNG);
				AddGraveStones(6, 1, aLevelRNG);
				AddGraveStones(7, 1, aLevelRNG);
				AddGraveStones(8, 2, aLevelRNG);
			}
			else if (!mApp->IsAdventureMode())
			{
				AddGraveStones(4, 1, aLevelRNG);
				AddGraveStones(5, 1, aLevelRNG);
				AddGraveStones(6, 2, aLevelRNG);
				AddGraveStones(7, 2, aLevelRNG);
				AddGraveStones(8, 3, aLevelRNG);
			}
			else if (mLevel == 11 || mLevel == 12 || mLevel == 13)
			{
				AddGraveStones(6, 1, aLevelRNG);
				AddGraveStones(7, 1, aLevelRNG);
				AddGraveStones(8, 2, aLevelRNG);
			}
			else if (mLevel == 14 || mLevel == 16)
			{
				AddGraveStones(5, 1, aLevelRNG);
				AddGraveStones(6, 1, aLevelRNG);
				AddGraveStones(7, 2, aLevelRNG);
				AddGraveStones(8, 3, aLevelRNG);
			}
			else if (mLevel == 17 || mLevel == 18 || mLevel == 19)
			{
				AddGraveStones(4, 1, aLevelRNG);
				AddGraveStones(5, 2, aLevelRNG);
				AddGraveStones(6, 2, aLevelRNG);
				AddGraveStones(7, 3, aLevelRNG);
				AddGraveStones(8, 3, aLevelRNG);
			}
			else if (mLevel >= 20)
			{
				AddGraveStones(3, 1, aLevelRNG);
				AddGraveStones(4, 2, aLevelRNG);
				AddGraveStones(5, 2, aLevelRNG);
				AddGraveStones(6, 2, aLevelRNG);
				AddGraveStones(7, 3, aLevelRNG);
				AddGraveStones(8, 3, aLevelRNG);
			}
			else
			{
				TOD_ASSERT(false);
			}
		}
	}
	PickSpecialGraveStone();
}

void Board::InitZombieWavesForLevel(int theForLevel)
{
	if (mApp->IsWhackAZombieLevel() || (mApp->IsWallnutBowlingLevel() && !mApp->IsFirstTimeAdventureMode()))
	{
		mChallenge->InitZombieWaves();
		return;
	}

	for (int aZombieType = ZombieType::ZOMBIE_NORMAL; aZombieType < ZombieType::NUM_ZOMBIE_TYPES; aZombieType++)
	{
		mZombieAllowed[aZombieType] = CanZombieSpawnOnLevel(static_cast<ZombieType>(aZombieType), theForLevel);
	}
}

bool Board::IsZombieWaveDistributionOk()
{
	if (!mApp->IsAdventureMode())
		return true;

	int aZombieTypeCount[ZombieType::NUM_ZOMBIE_TYPES] = { 0 };
	for (int aWave = 0; aWave < mNumWaves; aWave++)
	{
		for (int aIndex = 0; aIndex < MAX_ZOMBIES_IN_WAVE; aIndex++)
		{
			ZombieType aZombieType = mZombiesInWave[aWave][aIndex];
			if (aZombieType == ZombieType::ZOMBIE_INVALID)
			{
				break;
			}

			TOD_ASSERT(aZombieType >= 0 && aZombieType < ZombieType::NUM_ZOMBIE_TYPES);
			aZombieTypeCount[aZombieType]++;
		}
	}

	for (ZombieType aZombieType = ZombieType::ZOMBIE_NORMAL; aZombieType < ZombieType::NUM_ZOMBIE_TYPES; aZombieType = static_cast<ZombieType>(static_cast<int>(aZombieType) + 1))
	{
		if (aZombieType != ZombieType::ZOMBIE_YETI && CanZombieSpawnOnLevel(aZombieType, mLevel) && aZombieTypeCount[aZombieType] == 0)
		{
			TodTraceAndLog("Didn't spawn required zombie %s, level %d", GetZombieDefinition(aZombieType).mZombieName, mLevel);
			return false;
		}
	}
	return true;
}

void Board::InitZombieWaves()
{
	memset(mZombieAllowed, false, sizeof(mZombieAllowed));
	if (mApp->IsAdventureMode())
	{
		InitZombieWavesForLevel(mLevel);
	}
	else
	{
		mChallenge->InitZombieWaves();
	}
	PickZombieWaves();
	TOD_ASSERT(IsZombieWaveDistributionOk());

	mCurrentWave = 0;
	mTotalSpawnedWaves = 0;
	mApp->mSawYeti = false;
	if (mApp->IsFirstTimeAdventureMode() && mLevel == 2)
	{
		mZombieCountDown = ZOMBIE_COUNTDOWN * 2;
	}
	else if (mApp->IsSurvivalMode() && mChallenge->mSurvivalStage > 0)
	{
		mZombieCountDown = ZOMBIE_COUNTDOWN_RANGE;
	}
	else
	{
		mZombieCountDown = ZOMBIE_COUNTDOWN_FIRST_WAVE;
	}

	mZombieHealthWaveStart = 0;
	mLastBungeeWave = 0;
	mProgressMeterWidth = 0;
	mHugeWaveCountDown = 0;
	mLevelAwardSpawned = false;
	mZombieCountDownStart = mZombieCountDown;
	mZombieHealthToNextWave = -1;
}

void Board::FreezeEffectsForCutscene(bool theFreeze)
{
	TodParticleSystem* aParticle = nullptr;
	while (IterateParticles(aParticle))
	{
		if (aParticle->mEffectType == ParticleEffect::PARTICLE_GRAVE_BUSTER)
		{
			aParticle->mDontUpdate = theFreeze;
		}
		else if (aParticle->mEffectType == ParticleEffect::PARTICLE_POOL_SPARKLY && mIceTrapCounter == 0)
		{
			aParticle->mDontUpdate = theFreeze;
		}
	}

	Reanimation* aReanim = nullptr;
	while (IterateReanimations(aReanim))
	{
		if (aReanim->mReanimationType == ReanimationType::REANIM_SLEEPING)
		{
			aReanim->mAnimRate = theFreeze ? 0.0f : RandRangeFloat(6, 8);
		}
	}
}

void Board::InitSurvivalStage()
{
	RefreshSeedPacketFromCursor();
	mApp->mSoundSystem->GamePause(true);
	FreezeEffectsForCutscene(true);
	mLevelComplete = false;
	InitZombieWaves();
	mApp->mGameScene = GameScenes::SCENE_LEVEL_INTRO;
	mApp->ShowSeedChooserScreen();
	mCutScene->StartLevelIntro();
	mSeedBank->UpdateWidth();

	for (int i = 0; i < SEEDBANK_MAX; i++)
	{
		SeedPacket* aPacket = &mSeedBank->mSeedPackets[i];
		aPacket->mX = GetSeedPacketPositionX(i);
		aPacket->mPacketType = SeedType::SEED_NONE;
	}

	if (StageHasFog())
	{
		mFogBlownCountDown = FOG_BLOW_RETURN_TIME;
	}
	for (int j = 0; j < MAX_GRID_SIZE_Y; j++)
	{
		mWaveRowGotLawnMowered[j] = -100;
	}
}

Rect Board::GetShovelButtonRect()
{
	Rect aRect(GetSeedBankExtraWidth() + 456, 0, Sexy::IMAGE_SHOVELBANK->GetWidth(), Sexy::IMAGE_SHOVELBANK->GetHeight());
	if (mApp->IsSlotMachineLevel() || mApp->IsSquirrelLevel())
	{
		aRect.mX = 600;
	}
	return aRect;
}

void Board::GetZenButtonRect(GameObjectType theObjectType, Rect& theRect)
{
	// 此函数与内测版的差异在于，内测版在此函数中通过下列语句先取得了铲子按钮矩形：
	// Rect aRect = GetShovelButtonRect();
	// 而原版需要在函数调用前先自行取得铲子按钮矩形，并将该矩形作为参数传递给此函数，
	// 原版中此函数有将 theRect 的引用作为返回值，但并无直接使用返回值的情况。
	// 此处为了防止误用返回值而出现问题，故删除其返回值，如需调用可按照如下方式：
	// Rect aButtonRect = GetShovelButtonRect();
	// GetZenButtonRect(xxx, aButtonRect);

	theRect.mX = 30;
	if (mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM)
	{
		return;
	}
	
	bool usable = true;
	for (int anObject = GameObjectType::OBJECT_TYPE_WATERING_CAN; anObject <= GameObjectType::OBJECT_TYPE_WHEELBARROW; anObject++)
	{
		if (!CanUseGameObject((GameObjectType)anObject))
		{
			usable = false;
			break;
		}
	}
	if (usable)
	{
		theRect.mX = 0;
	}

	int aShovelWidth = Sexy::IMAGE_SHOVELBANK->GetWidth();
	for (int anObject = GameObjectType::OBJECT_TYPE_WATERING_CAN; anObject < theObjectType; anObject++)
	{
		// 每存在一个序号小于目标的可用按钮，则目标按钮的横坐标增加 70
		if (CanUseGameObject((GameObjectType)anObject))
		{
			theRect.mX += aShovelWidth;
		}
	}
	//return theRect;
}

// GOTY @Patoke: 0x40D840
void Board::InitLevel()
{
	mMainCounter = 0;
	mEnableGraveStones = false;
	mSodPosition = 0;
	mPrevBoardResult = mApp->mBoardResult;
	
	GameMode aGameMode = mApp->mGameMode;
	if (aGameMode != GameMode::GAMEMODE_TREE_OF_WISDOM && aGameMode != GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		mApp->mMusic->StopAllMusic();
	}
	// 赋值当前关卡
	mLevel = mApp->IsAdventureMode() ? mApp->mPlayerInfo->mLevel : 0;
	// 设定关卡背景
	PickBackground();
	// 设定关卡出怪
	InitZombieWaves();
	// 设定关卡初始阳光数量
	if (aGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED || aGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST ||
		mApp->IsScaryPotterLevel() || mApp->IsWhackAZombieLevel())
	{
		mSunMoney = 0;
	}
	else if (aGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND)
	{
		mSunMoney = 5000;
	}
	else if (mApp->IsIZombieLevel())
	{
		mSunMoney = 150;
	}
	else if (mApp->IsFirstTimeAdventureMode() && mLevel == 1)
	{
		mSunMoney = 150;
	}
	else
	{
		mSunMoney = 50;
	}

	// 初始化行选择数组
	memset(mRowPickingArray, 0, sizeof(mRowPickingArray));
	// 初始化每行的基础数据
	for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++)
	{
		mWaveRowGotLawnMowered[aRow] = -100;
		mIceMinX[aRow] = BOARD_ICE_START;
		mIceTimer[aRow] = 0;
		mIceParticleID[aRow] = ParticleSystemID::PARTICLESYSTEMID_NULL;
		mRowPickingArray[aRow].mItem = aRow;
	}
	// 初始化阳光掉落
	mNumSunsFallen = 0;
	if (!StageIsNight())
	{
		mSunCountDown = RandRangeInt(425, 700);
	}
	// 初始化字幕播放记录
	memset(mHelpDisplayed, 0, sizeof(mHelpDisplayed));
	// 初始化卡槽及卡牌
	mSeedBank->mNumPackets = GetNumSeedsInBank();
	mSeedBank->UpdateWidth();
	for (int i = 0; i < SEEDBANK_MAX; i++)
	{
		SeedPacket* aPacket = &mSeedBank->mSeedPackets[i];
		aPacket->mIndex = i;
		aPacket->mX = GetSeedPacketPositionX(i);
		aPacket->mY = 8;
		aPacket->mPacketType = SeedType::SEED_NONE;
	}
	// 设定固定卡牌
	if (mApp->IsSlotMachineLevel())
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 3);
		mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_SUNFLOWER);
		mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_PEASHOOTER);
		mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_SNOWPEA);
	}
	else if (aGameMode == GameMode::GAMEMODE_CHALLENGE_ICE)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 6);
		mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_PEASHOOTER);
		mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_CHERRYBOMB);
		mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_WALLNUT);
		mSeedBank->mSeedPackets[3].SetPacketType(SeedType::SEED_REPEATER);
		mSeedBank->mSeedPackets[4].SetPacketType(SeedType::SEED_SNOWPEA);
		mSeedBank->mSeedPackets[5].SetPacketType(SeedType::SEED_CHOMPER);
	}
	else if (aGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_1)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 3);
		mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIE_NORMAL);
		mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIE_PAIL);
		mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_ZOMBIE_FOOTBALL);
	}
	else if (aGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_2)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 3);
		mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIE_NORMAL);
		mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIE_SCREEN_DOOR);
		mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_ZOMBIE_PAIL);
	}
	else if (aGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_3)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 3);
		mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIE_NORMAL);
		mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIE_PAIL);
		mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_ZOMBIE_DIGGER);
	}
	else if (aGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_4)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 3);
		mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIE_NORMAL);
		mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIE_PAIL);
		mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_ZOMBIE_LADDER);
	}
	else if (aGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_5)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 4);
		mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIE_NORMAL);
		mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIE_PAIL);
		mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_ZOMBIE_BUNGEE);
		mSeedBank->mSeedPackets[3].SetPacketType(SeedType::SEED_ZOMBIE_BALLOON);
	}
	else if (aGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_6)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 4);
		mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIE_NORMAL);
		mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIE_POLEVAULTER);
		mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_ZOMBIE_PAIL);
		mSeedBank->mSeedPackets[3].SetPacketType(SeedType::SEED_ZOMBIE_GARGANTUAR);
	}
	else if (aGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_7)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 4);
		mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIE_NORMAL);
		mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIE_POLEVAULTER);
		mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_ZOMBIE_PAIL);
		mSeedBank->mSeedPackets[3].SetPacketType(SeedType::SEED_ZOMBIE_DANCER);
	}
	else if (aGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_8)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 6);
		mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIE_IMP);
		mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIE_TRAFFIC_CONE);
		mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_ZOMBIE_PAIL);
		mSeedBank->mSeedPackets[3].SetPacketType(SeedType::SEED_ZOMBIE_BUNGEE);
		mSeedBank->mSeedPackets[4].SetPacketType(SeedType::SEED_ZOMBIE_DIGGER);
		mSeedBank->mSeedPackets[5].SetPacketType(SeedType::SEED_ZOMBIE_LADDER);
	}
	else if (aGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_9)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 8);
		mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIE_IMP);
		mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIE_TRAFFIC_CONE);
		mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_ZOMBIE_POLEVAULTER);
		mSeedBank->mSeedPackets[3].SetPacketType(SeedType::SEED_ZOMBIE_PAIL);
		mSeedBank->mSeedPackets[4].SetPacketType(SeedType::SEED_ZOMBIE_BUNGEE);
		mSeedBank->mSeedPackets[5].SetPacketType(SeedType::SEED_ZOMBIE_DIGGER);
		mSeedBank->mSeedPackets[6].SetPacketType(SeedType::SEED_ZOMBIE_LADDER);
		mSeedBank->mSeedPackets[7].SetPacketType(SeedType::SEED_ZOMBIE_FOOTBALL);
	}
	else if (aGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_ENDLESS)
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 9);
		mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIE_IMP);
		mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIE_TRAFFIC_CONE);
		mSeedBank->mSeedPackets[2].SetPacketType(SeedType::SEED_ZOMBIE_POLEVAULTER);
		mSeedBank->mSeedPackets[3].SetPacketType(SeedType::SEED_ZOMBIE_PAIL);
		mSeedBank->mSeedPackets[4].SetPacketType(SeedType::SEED_ZOMBIE_BUNGEE);
		mSeedBank->mSeedPackets[5].SetPacketType(SeedType::SEED_ZOMBIE_DIGGER);
		mSeedBank->mSeedPackets[6].SetPacketType(SeedType::SEED_ZOMBIE_LADDER);
		mSeedBank->mSeedPackets[7].SetPacketType(SeedType::SEED_ZOMBIE_FOOTBALL);
		mSeedBank->mSeedPackets[8].SetPacketType(SeedType::SEED_ZOMBIE_DANCER);
	}
	else if (mApp->IsScaryPotterLevel())
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 1);
		mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_CHERRYBOMB);
	}
	else if (mApp->IsWhackAZombieLevel())
	{
		TOD_ASSERT(mSeedBank->mNumPackets == 3);
		mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_POTATOMINE);
		mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_GRAVEBUSTER);
		mSeedBank->mSeedPackets[2].SetPacketType(mApp->IsAdventureMode() ? SeedType::SEED_CHERRYBOMB : SeedType::SEED_ICESHROOM);
	}
	else if (aGameMode == GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM) {
		mSeedBank->mSeedPackets[0].SetPacketType(SeedType::SEED_ZOMBIQUARIUM_SNORKLE);
		mSeedBank->mSeedPackets[1].SetPacketType(SeedType::SEED_ZOMBIQUARIUM_TROPHY);
	}
	else if (!ChooseSeedsOnCurrentLevel() && !HasConveyorBeltSeedBank())
	{
		mSeedBank->mNumPackets = GetNumSeedsInBank();
		// 卡槽错误的关卡，依次填充所有卡牌
		for (int i = 0; i < mSeedBank->mNumPackets; i++)
		{
			mSeedBank->mSeedPackets[i].SetPacketType((SeedType)i);
		}
	}
	// 将所有子控件标记为已变动
	MarkAllDirty();
	
	mPaused = false;
	mOutOfMoneyCounter = 0;
	if (StageHasFog())
	{
		mFogBlownCountDown = 200;
		mFogOffset = 1065 - LeftFogColumn() * 80;
	}
	// 关卡玩法相关的初始化
	mChallenge->InitLevel();
}

Reanimation* Board::CreateRakeReanim(float theRakeX, float theRakeY, int theRenderOrder)
{
	Reanimation* aReanim = mApp->AddReanimation(theRakeX + 20, theRakeY, theRenderOrder, REANIM_RAKE);
	aReanim->mAnimRate = 0;
	aReanim->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
	aReanim->mIsAttachment = true;
	return aReanim;
}

void Board::PlaceRake()
{
	if (!mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_RAKE])
		return;

	int aGridX = 7;
	if (mApp->IsScaryPotterLevel())
	{
		GridItem* aGridItem = nullptr;
		while (IterateGridItems(aGridItem))
		{
			if (aGridItem->mGridItemType == GridItemType::GRIDITEM_SCARY_POT && aGridItem->mGridX <= aGridX && aGridItem->mGridX > 0)
			{
				aGridX = aGridItem->mGridX - 1;
			}
		}
	}
	else
	{
		if (!StageHasZombieWalkInFromRight() || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED ||
			mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BOBSLED_BONANZA)
			return;
	}

	int aPickCount = 0;
	TodWeightedArray aPickArray[MAX_GRID_SIZE_Y];
	for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++)
	{
		if (aRow != 5 && mPlantRow[aRow] == PlantRowType::PLANTROW_NORMAL)
		{
			aPickArray[aPickCount].mWeight = 1;
			aPickArray[aPickCount].mItem = aRow;
			aPickCount++;
		}
	}
	if (aPickCount == 0)
		return;

	int aGridY = TodPickFromWeightedArray(aPickArray, aPickCount);
	mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_RAKE]--;
	GridItem* aRake = mGridItems.DataArrayAlloc();
	aRake->mGridItemType = GridItemType::GRIDITEM_RAKE;
	aRake->mGridX = aGridX;
	aRake->mGridY = aGridY;
	aRake->mPosX = GridToPixelX(aGridX, aGridY);
	aRake->mPosY = GridToPixelY(aGridX, aGridY);
	aRake->mRenderOrder = MakeRenderOrder(RenderLayer::RENDER_LAYER_GRAVE_STONE, aGridY, 9);
	aRake->mGridItemReanimID = mApp->ReanimationGetID(CreateRakeReanim(aRake->mPosX, aRake->mPosY, 0)); // Lmao gotta pass in the right coords
	aRake->mGridItemState = GridItemState::GRIDITEM_STATE_RAKE_ATTRACTING;
}

void Board::InitLawnMowers()
{
	GameMode aGameMode = mApp->mGameMode;
	// 这里优化一下原版的代码，事先列举一些不创建小推车的关卡
	if (aGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED || aGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST ||
		aGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN || aGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM ||
		aGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND || aGameMode == GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM ||
		mApp->IsSquirrelLevel() || mApp->IsIZombieLevel() || (StageHasRoof() && !mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_ROOF_CLEANER]))
		return;

	for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++)
	{
		if ((aGameMode == GameMode::GAMEMODE_CHALLENGE_RESODDED && aRow <= 4) || 
			(mApp->IsAdventureMode() && mLevel == 35) ||   // 这里原版没有对于行的判断，故冒险模式 4-5 关卡有 6 行小推车
			(!mApp->IsScaryPotterLevel() && mPlantRow[aRow] != PlantRowType::PLANTROW_DIRT))  // 除冒险模式 4-5 关卡外的破罐者模式关卡无小推车
		{
			LawnMower* aLawnMower = mLawnMowers.DataArrayAlloc();
			aLawnMower->LawnMowerInitialize(aRow);
			aLawnMower->mVisible = false;
		}
	}
}

bool Board::ChooseSeedsOnCurrentLevel()
{
	if (mApp->IsChallengeWithoutSeedBank() || HasConveyorBeltSeedBank())
		return false;

	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ICE || 
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED ||
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST || 
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM)
		return false;

	if (mApp->IsIZombieLevel() || mApp->IsSlotMachineLevel())
		return false;

	return (!mApp->IsFirstTimeAdventureMode() || mLevel > 7);
}

// GOTY @Patoke: 0x40E6A0
void Board::StartLevel()
{
	mCoinBankFadeCount = 0;
	mApp->mLastLevelStats->Reset();
	mChallenge->StartLevel();

	// @Patoke: implemented, i think it's intentional to cause an underflow here?
	unsigned int aSurvivalStage = mApp->mGameMode - GAMEMODE_SURVIVAL_ENDLESS_STAGE_1;
	if (aSurvivalStage <= 4) {
		if (GetSurvivalFlagsCompleted() >= 20) {
			// if ( !*(mApp->mPlayerInfo + 53) ) todo @Patoke: add this?
			ReportAchievement::GiveAchievement(mApp, Immortal, true);
		}
	}

	if (mApp->IsSurvivalMode() && mChallenge->mSurvivalStage > 0)
	{
		FreezeEffectsForCutscene(false);
		mApp->mSoundSystem->GamePause(false);
	}

	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ICE || 
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN || 
		mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM ||
		mApp->mGameMode == GameMode::GAMEMODE_UPSELL || 
		mApp->mGameMode == GameMode::GAMEMODE_INTRO || 
		mApp->IsFinalBossLevel())
		return;

	mApp->mMusic->StartGameMusic();
}

LawnMower* Board::GetBottomLawnMower()
{
	LawnMower* aLawnMower = nullptr;
	LawnMower* aBottomMower = nullptr;
	while (IterateLawnMowers(aLawnMower))
	{
		if (aLawnMower->mMowerState == LawnMowerState::MOWER_TRIGGERED || aLawnMower->mMowerState == LawnMowerState::MOWER_SQUISHED)
			continue;

		if (aBottomMower == nullptr || aBottomMower->mRow < aLawnMower->mRow)
		{
			aBottomMower = aLawnMower;
		}
	}
	return aBottomMower;
}

// GOTY @Patoke: 0x40E860
void Board::UpdateLevelEndSequence()
{
	if (mNextSurvivalStageCounter > 0)
	{
		if (!IsScaryPotterDaveTalking())
		{
			mNextSurvivalStageCounter--;
			if (mApp->IsAdventureMode() && mApp->IsScaryPotterLevel() && mNextSurvivalStageCounter == 300)
			{
				mApp->CrazyDaveEnter();
				mApp->CrazyDaveTalkIndex(mChallenge->mSurvivalStage == 0 ? 2700 : 2800);
				mChallenge->PuzzleNextStageClear();
				mNextSurvivalStageCounter = 100;
			}
		}

		if (mNextSurvivalStageCounter == 1 && mApp->IsSurvivalMode())
		{
			TryToSaveGame();
		}

		if (!mNextSurvivalStageCounter)
		{
			if (mApp->IsScaryPotterLevel())
			{
				if (mApp->IsAdventureMode())
					return;

				if (!IsFinalScaryPotterStage())
				{
					mChallenge->PuzzleNextStageClear();
					mChallenge->ScaryPotterPopulate();
				}
			}
			else if (LawnApp::IsEndlessIZombie(mApp->mGameMode))
			{
				mChallenge->PuzzleNextStageClear();
				mChallenge->IZombieInitLevel();
			}
			else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND)
			{
				ClearAdvice(AdviceType::ADVICE_NONE);
			}
			else
			{
				mLevelComplete = true;
				RemoveZombiesForRepick();
			}
			return;
		}
	}

	if (mBoardFadeOutCounter < 0)
		return;

	mBoardFadeOutCounter--;
	if (mBoardFadeOutCounter == 0)
	{
		mLevelComplete = true;
		return;
	}
	if (mBoardFadeOutCounter == 300)
	{
		if (!IsSurvivalStageWithRepick() && !(mLevel == 9 || mLevel == 19 || mLevel == 29 || mLevel == 39 || mLevel == 49))
		{
			mApp->PlaySample(Sexy::SOUND_LIGHTFILL);
		}
	}

	if (mScoreNextMowerCounter > 0)
	{
		mScoreNextMowerCounter--;
		if (mScoreNextMowerCounter)
		{
			return;
		}
	}

	if (CanDropLoot() && !IsSurvivalStageWithRepick())
	{
		mScoreNextMowerCounter = 40;
		LawnMower* aLawnMower = GetBottomLawnMower();
		if (aLawnMower)
		{
			AddCoin(aLawnMower->mPosX + 40, aLawnMower->mPosY + 40, CoinType::COIN_GOLD, CoinMotion::COIN_MOTION_LAWNMOWER_COIN);
			SoundInstance* aSoundInstance = mApp->mSoundManager->GetSoundInstance(Sexy::SOUND_POINTS);
			if (aSoundInstance)
			{
				aSoundInstance->Play(false, true);
				float aPitch = ClampFloat(6 - CountUntriggerLawnMowers(), 0.0f, 6.0f);
				aSoundInstance->AdjustPitch(aPitch);
			}
			aLawnMower->Die();
		}
	}
}

void Board::CompleteEndLevelSequenceForSaving()
{
	if (CanDropLoot())
	{
		LawnMower* aLawnMower = nullptr;
		while (IterateLawnMowers(aLawnMower))
		{
			if (aLawnMower->mMowerState != LawnMowerState::MOWER_TRIGGERED && aLawnMower->mMowerState != LawnMowerState::MOWER_SQUISHED)
			{
				int aCoinValue = Coin::GetCoinValue(CoinType::COIN_GOLD);
				mApp->mPlayerInfo->AddCoins(aCoinValue);
				mCoinsCollected += aCoinValue;
			}
		}
	}

	Coin* aCoin = nullptr;
	while (IterateCoins(aCoin))
	{
		if (aCoin->mIsBeingCollected)
		{
			aCoin->ScoreCoin();
		}
		else
		{
			aCoin->Die();
		}
	}
	
	mApp->UpdatePlayerProfileForFinishingLevel();
}

void Board::FadeOutLevel()
{
	if (mApp->mGameScene != GameScenes::SCENE_PLAYING)
	{
		RefreshSeedPacketFromCursor();
		mApp->mLastLevelStats->Reset();
		mLevelComplete = true;
	}

	bool aNeedSoundEffect = true;
	if (mApp->IsScaryPotterLevel() && !IsFinalScaryPotterStage())
	{
		aNeedSoundEffect = false;
	}
	else if (IsSurvivalStageWithRepick() || IsLastStandStageWithRepick() || mApp->IsEndlessIZombie(mApp->mGameMode))
	{
		aNeedSoundEffect = false;
	}
	if (aNeedSoundEffect)
	{
		mApp->mMusic->StopAllMusic();
		if (mApp->IsAdventureMode() && mLevel == 50)
		{
			mApp->PlayFoley(FoleyType::FOLEY_FINAL_FANFARE);
		}
		else if (mApp->TrophiesNeedForGoldSunflower() == 1)
		{
			mApp->PlayFoley(FoleyType::FOLEY_FINAL_FANFARE);
		}
		else
		{
			mApp->PlayFoley(FoleyType::FOLEY_WINMUSIC);
		}
	}

	if (mApp->IsScaryPotterLevel() && !IsFinalScaryPotterStage())
	{
		mNextSurvivalStageCounter = 500;
		if (mApp->IsAdventureMode())
		{
			ClearAdvice(AdviceType::ADVICE_NONE);
		}
		else
		{
			mLevelAwardSpawned = true;
			std::string aStreakStr = mApp->IsEndlessScaryPotter(mApp->mGameMode) ? "[ADVICE_MORE_SCARY_POTS]" : "[ADVICE_3_IN_A_ROW]";
			std::string aMessage = TodReplaceNumberString(aStreakStr, "{STREAK}", mChallenge->mSurvivalStage + 1);
			PuzzleSaveStreak();
			ClearAdvice(AdviceType::ADVICE_NONE);
			DisplayAdvice(aMessage, MessageStyle::MESSAGE_STYLE_BIG_MIDDLE, AdviceType::ADVICE_NONE);
		}
		return;
	}

	if (mApp->IsEndlessIZombie(mApp->mGameMode))
	{
		mNextSurvivalStageCounter = 500;
		std::string aMessage = TodReplaceNumberString("[ADVICE_MORE_IZOMBIE]", "{STREAK}", mChallenge->mSurvivalStage + 1);
		PuzzleSaveStreak();
		ClearAdvice(AdviceType::ADVICE_NONE);
		DisplayAdvice(aMessage, MessageStyle::MESSAGE_STYLE_BIG_MIDDLE, AdviceType::ADVICE_NONE);
		return;
	}

	if (IsLastStandStageWithRepick())
	{
		mNextSurvivalStageCounter = 500;
		mChallenge->LastStandCompletedStage();
		return;
	}

	if (!IsSurvivalStageWithRepick())
	{
		RefreshSeedPacketFromCursor();
		mApp->mLastLevelStats->mUnusedLawnMowers = CountUntriggerLawnMowers();

		mBoardFadeOutCounter = 600;
		if (mLevel == 9 || mLevel == 19 || mLevel == 29 || mLevel == 39 || mLevel == 49)
		{
			mBoardFadeOutCounter = 500;
		}

		if (CanDropLoot())
		{
			mScoreNextMowerCounter = 200;
		}

		Coin* aCoin = nullptr;
		while (IterateCoins(aCoin))
		{
			aCoin->TryAutoCollectAfterLevelAward();
		}
	}
	else
	{
		TOD_ASSERT(mApp->IsSurvivalMode());
		mNextSurvivalStageCounter = 500;
		DisplayAdvice("[ADVICE_MORE_ZOMBIES]", MessageStyle::MESSAGE_STYLE_BIG_MIDDLE, AdviceType::ADVICE_NONE);
		mApp->mMusic->FadeOut(500);
		mApp->PlaySample(Sexy::SOUND_HUGE_WAVE);
		for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++)
		{
			mIceTimer[aRow] = mNextSurvivalStageCounter;
		}
	}

	mApp->SetCursor(CURSOR_POINTER);
}

void Board::DisplayAdvice(const std::string& theAdvice, MessageStyle theMessageStyle, AdviceType theHelpIndex)
{
	if (theHelpIndex != AdviceType::ADVICE_NONE)
	{
		if (mHelpDisplayed[theHelpIndex])
			return;

		mHelpDisplayed[theHelpIndex] = true;
	}

	mAdvice->SetLabel(theAdvice, theMessageStyle);
	mHelpIndex = theHelpIndex;
}

void Board::DisplayAdviceAgain(const std::string& theAdvice, MessageStyle theMessageStyle, AdviceType theHelpIndex)
{
	if (theHelpIndex != AdviceType::ADVICE_NONE)
	{
		mHelpDisplayed[theHelpIndex] = false;
	}
	DisplayAdvice(theAdvice, theMessageStyle, theHelpIndex);
}

void Board::ClearAdviceImmediately()
{
	ClearAdvice(AdviceType::ADVICE_NONE);
	mAdvice->mDuration = 0;
}

void Board::ClearAdvice(AdviceType theHelpIndex)
{
	if (theHelpIndex == AdviceType::ADVICE_NONE || theHelpIndex == mHelpIndex)
	{
		mAdvice->ClearLabel();
		mHelpIndex = AdviceType::ADVICE_NONE;
	}
}

Coin* Board::AddCoin(int theX, int theY, CoinType theCoinType, CoinMotion theCoinMotion)
{
	Coin* aCoin = mCoins.DataArrayAlloc();
	aCoin->CoinInitialize(theX, theY, theCoinType, theCoinMotion);
	if (mApp->IsFirstTimeAdventureMode() && mLevel == 1)
	{
		DisplayAdvice("[ADVICE_CLICK_ON_SUN]", MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY, AdviceType::ADVICE_CLICK_ON_SUN);
	}
	return aCoin;
}

bool Board::IsPlantInCursor()
{
	return 
		mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_BANK || 
		mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_USABLE_COIN || 
		mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_GLOVE || 
		mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_DUPLICATOR || 
		mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_WHEEL_BARROW;
}

// GOTY @Patoke: 0x40F600
SeedType Board::GetSeedTypeInCursor()
{
	if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_WHEEELBARROW)
	{
		PottedPlant* aPottedPlant = mApp->mZenGarden->GetPottedPlantInWheelbarrow();
		if (aPottedPlant)
		{
			return aPottedPlant->mSeedType;
		}
	}

	if (!IsPlantInCursor())
	{
		return SeedType::SEED_NONE;
	}
	return mCursorObject->mType == SeedType::SEED_IMITATER ? mCursorObject->mImitaterType : mCursorObject->mType;
}

void Board::RefreshSeedPacketFromCursor()
{
	if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_USABLE_COIN)
	{
		mCoins.DataArrayTryToGet(mCursorObject->mCoinID)->DroppedUsableSeed();
	}
	else if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_BANK)
	{
		TOD_ASSERT(mCursorObject->mSeedBankIndex >= 0 && mCursorObject->mSeedBankIndex < mSeedBank->mNumPackets);
		mSeedBank->mSeedPackets[mCursorObject->mSeedBankIndex].Activate();
	}
	ClearCursor();
}

bool Board::IsPoolSquare(int theGridX, int theGridY)
{
	if (theGridX >= 0 && theGridY >= 0)
	{
		TOD_ASSERT(theGridX < MAX_GRID_SIZE_X && theGridY < MAX_GRID_SIZE_Y);
		return mGridSquareType[theGridX][theGridY] == GridSquareType::GRIDSQUARE_POOL;
	}
	return false;
}

Plant* Board::NewPlant(int theGridX, int theGridY, SeedType theSeedType, SeedType theImitaterType)
{
	Plant* aPlant = mPlants.DataArrayAlloc();
	aPlant->mIsOnBoard = true;
	aPlant->PlantInitialize(theGridX, theGridY, theSeedType, theImitaterType);
	return aPlant;
}

void Board::DoPlantingEffects(int theGridX, int theGridY, Plant* thePlant)
{
	int aXPos = GridToPixelX(theGridX, theGridY) + 41;
	int aYPos = GridToPixelY(theGridX, theGridY) + 74;
	if (thePlant)
	{
		if (thePlant->mSeedType == SeedType::SEED_LILYPAD)
		{
			aYPos += 15;
		}
		else if (thePlant->mSeedType == SeedType::SEED_FLOWERPOT)
		{
			aYPos += 30;
		}
	}
	
	if (mBackground == BackgroundType::BACKGROUND_GREENHOUSE)
	{
		mApp->PlayFoley(FoleyType::FOLEY_CERAMIC);
		return;
	}
	if (mBackground == BackgroundType::BACKGROUND_ZOMBIQUARIUM)
	{
		mApp->PlayFoley(FoleyType::FOLEY_PLANT_WATER);
		return;
	}
	if (Plant::IsFlying(thePlant->mSeedType))
	{
		mApp->PlayFoley(FoleyType::FOLEY_PLANT);
		return;
	}

	if (IsPoolSquare(theGridX, theGridY))
	{
		mApp->PlayFoley(FoleyType::FOLEY_PLANT_WATER);
		mApp->AddTodParticle(aXPos, aYPos, RenderLayer::RENDER_LAYER_TOP, ParticleEffect::PARTICLE_PLANTING_POOL);
	}
	else
	{
		mApp->PlayFoley(FoleyType::FOLEY_PLANT);
		mApp->AddTodParticle(aXPos, aYPos, RenderLayer::RENDER_LAYER_TOP, ParticleEffect::PARTICLE_PLANTING);
	}
}

// GOTY @Patoke: 0x40FA10
Plant* Board::AddPlant(int theGridX, int theGridY, SeedType theSeedType, SeedType theImitaterType)
{
	Plant* aPlant = NewPlant(theGridX, theGridY, theSeedType, theImitaterType);
	DoPlantingEffects(theGridX, theGridY, aPlant);
	mChallenge->PlantAdded(aPlant);

	int aSunPlantsCount = CountPlantByType(SeedType::SEED_SUNSHROOM) + CountPlantByType(SeedType::SEED_SUNFLOWER);
	if (aSunPlantsCount > mMaxSunPlants)
	{
		mMaxSunPlants = aSunPlantsCount;  //mMaxSunPlants = max(aSunPlantsCount, mMaxSunPlants);
	}

	// @Patoke: implemented
	if (theSeedType == SeedType::SEED_PEASHOOTER ||
		theSeedType == SeedType::SEED_SNOWPEA ||
		theSeedType == SeedType::SEED_REPEATER ||
		theSeedType == SeedType::SEED_THREEPEATER ||
		theSeedType == SeedType::SEED_SPLITPEA ||
		theSeedType == SeedType::SEED_GATLINGPEA)
	{
		mPeaShooterUsed = true;
	}
	if (theSeedType == SeedType::SEED_CABBAGEPULT ||
		theSeedType == SeedType::SEED_KERNELPULT ||
		theSeedType == SeedType::SEED_MELONPULT ||
		theSeedType == SeedType::SEED_WINTERMELON)
	{
		mCatapultPlantsUsed = true;
	}

	bool aIsFungi = Plant::IsFungus(theSeedType);
	if (!Plant::IsFlying(theSeedType) && !aIsFungi) {
		mMushroomAndCoffeeBeansOnly = false;
	}
	if (aIsFungi) {
		mMushroomsUsed = true;
	}

	return aPlant;
}

// GOTY @Patoke: 0x40FBA0
Plant* Board::GetPumpkinAt(int theGridX, int theGridY)
{
	Plant* aPlant = nullptr;
	while (IteratePlants(aPlant))
	{
		if (aPlant->mPlantCol == theGridX && aPlant->mRow == theGridY && !aPlant->NotOnGround() && aPlant->mSeedType == SeedType::SEED_PUMPKINSHELL)
		{
			return aPlant;
		}
	}
	return nullptr;
}

Plant* Board::GetFlowerPotAt(int theGridX, int theGridY)
{
	Plant* aPlant = nullptr;
	while (IteratePlants(aPlant))
	{
		if (aPlant->mPlantCol == theGridX && aPlant->mRow == theGridY && !aPlant->NotOnGround() && aPlant->mSeedType == SeedType::SEED_FLOWERPOT)
		{
			return aPlant;
		}
	}
	return nullptr;
}

void Board::GetPlantsOnLawn(int theGridX, int theGridY, PlantsOnLawn* thePlantOnLawn)
{
	thePlantOnLawn->mUnderPlant = nullptr;
	thePlantOnLawn->mPumpkinPlant = nullptr;
	thePlantOnLawn->mFlyingPlant = nullptr;
	thePlantOnLawn->mNormalPlant = nullptr;

	if (theGridX < 0 || theGridX >= MAX_GRID_SIZE_X || theGridY < 0 || theGridY >= MAX_GRID_SIZE_Y)
		return;

	if (mApp->IsWallnutBowlingLevel() && !mCutScene->IsInShovelTutorial())
		return;

	Plant* aPlant = nullptr;
	while (IteratePlants(aPlant))
	{
		SeedType aSeedType = aPlant->mSeedType;
		if (aSeedType == SeedType::SEED_IMITATER && aPlant->mImitaterType != SeedType::SEED_NONE)
		{
			aSeedType = aPlant->mImitaterType;
		}

		// 检测植物是否位于目标格子内
		if (aPlant->mRow != theGridY)
		{
			continue;
		}
		if (aSeedType == SeedType::SEED_COBCANNON)
		{
			if (aPlant->mPlantCol < theGridX - 1 || aPlant->mPlantCol > theGridX)
			{
				continue;
			}
		}
		else
		{
			if (aPlant->mPlantCol != theGridX)
			{
				continue;
			}
		}
		if (aPlant->NotOnGround())
		{
			continue;
		}

		// 将植物写入 thePlantOnLawn 的记录
		if (Plant::IsFlying(aSeedType))
		{
			TOD_ASSERT(!thePlantOnLawn->mFlyingPlant);
			thePlantOnLawn->mFlyingPlant = aPlant;
		}
		else if (aSeedType == SeedType::SEED_FLOWERPOT || (aSeedType == SeedType::SEED_LILYPAD && mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN))
		{
			TOD_ASSERT(!thePlantOnLawn->mUnderPlant);
			thePlantOnLawn->mUnderPlant = aPlant;
		}
		else if (aSeedType == SeedType::SEED_PUMPKINSHELL)
		{
			TOD_ASSERT(!thePlantOnLawn->mPumpkinPlant);
			thePlantOnLawn->mPumpkinPlant = aPlant;
		}
		else
		{
			TOD_ASSERT(!thePlantOnLawn->mNormalPlant);
			thePlantOnLawn->mNormalPlant = aPlant;
		}
	}
}

Plant* Board::GetTopPlantAt(int theGridX, int theGridY, PlantPriority thePriority)
{
	if (theGridX < 0 || theGridX >= MAX_GRID_SIZE_X || theGridY < 0 || theGridY >= MAX_GRID_SIZE_Y)
		return nullptr;

	if (mApp->IsWallnutBowlingLevel() && !mCutScene->IsInShovelTutorial())
		return nullptr;

	PlantsOnLawn aPlantOnLawn;
	GetPlantsOnLawn(theGridX, theGridY, &aPlantOnLawn);

	switch (thePriority)
	{
	case PlantPriority::TOPPLANT_EATING_ORDER:
		if (aPlantOnLawn.mPumpkinPlant)							return aPlantOnLawn.mPumpkinPlant;
		else if (aPlantOnLawn.mNormalPlant)						return aPlantOnLawn.mNormalPlant;
		else													return aPlantOnLawn.mUnderPlant;
	case PlantPriority::TOPPLANT_DIGGING_ORDER:
		if (aPlantOnLawn.mNormalPlant)							return aPlantOnLawn.mNormalPlant;
		else													return aPlantOnLawn.mUnderPlant;
	case PlantPriority::TOPPLANT_BUNGEE_ORDER:
	case PlantPriority::TOPPLANT_CATAPULT_ORDER:
	case PlantPriority::TOPPLANT_ANY:
		if (aPlantOnLawn.mFlyingPlant)							return aPlantOnLawn.mFlyingPlant;
		else if (aPlantOnLawn.mNormalPlant)						return aPlantOnLawn.mNormalPlant;
		else if (aPlantOnLawn.mPumpkinPlant)					return aPlantOnLawn.mPumpkinPlant;
		else													return aPlantOnLawn.mUnderPlant;
	case PlantPriority::TOPPLANT_ZEN_TOOL_ORDER:
		if (aPlantOnLawn.mFlyingPlant)							return aPlantOnLawn.mFlyingPlant;
		else if (aPlantOnLawn.mPumpkinPlant)					return aPlantOnLawn.mPumpkinPlant;
		else if (aPlantOnLawn.mNormalPlant)						return aPlantOnLawn.mNormalPlant;
		else													return aPlantOnLawn.mUnderPlant;
	case PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION:			return aPlantOnLawn.mNormalPlant;
	case PlantPriority::TOPPLANT_ONLY_FLYING:					return aPlantOnLawn.mFlyingPlant;
	case PlantPriority::TOPPLANT_ONLY_PUMPKIN:					return aPlantOnLawn.mPumpkinPlant;
	case PlantPriority::TOPPLANT_ONLY_UNDER_PLANT:				return aPlantOnLawn.mUnderPlant;
	default:													TOD_ASSERT(false);
	}
	unreachable();
}

int Board::CountSunFlowers()
{
	int aCount = 0;
	Plant* aPlant = nullptr;
	while (IteratePlants(aPlant))
	{
		if (aPlant->MakesSun())
		{
			aCount++;
		}
	}
	return aCount;
}

int Board::CountPlantByType(SeedType theSeedType)
{
	int aCount = 0;
	Plant* aPlant = nullptr;
	while (IteratePlants(aPlant))
	{
		if (aPlant->mSeedType == theSeedType)
		{
			aCount++;
		}
	}
	return aCount;
}

int Board::CountEmptyPotsOrLilies(SeedType theSeedType)
{
	int aCount = 0;
	Plant* aPlant = nullptr;
	while (IteratePlants(aPlant))
	{
		if (aPlant->mSeedType == theSeedType && !GetTopPlantAt(aPlant->mPlantCol, aPlant->mRow, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION))
		{
			aCount++;
		}
	}
	return aCount;
}

bool Board::IsValidCobCannonSpotHelper(int theGridX, int theGridY)
{
	PlantsOnLawn aPlantOnLawn;
	GetPlantsOnLawn(theGridX, theGridY, &aPlantOnLawn);
	if (aPlantOnLawn.mPumpkinPlant)
		return false;

	if (aPlantOnLawn.mNormalPlant && aPlantOnLawn.mNormalPlant->mSeedType == SeedType::SEED_KERNELPULT)
		return true;

	return mApp->mEasyPlantingCheat && CanPlantAt(theGridX, theGridY, SeedType::SEED_KERNELPULT) == PlantingReason::PLANTING_OK;
}

bool Board::IsValidCobCannonSpot(int theGridX, int theGridY)
{
	if (!IsValidCobCannonSpotHelper(theGridX, theGridY) || !IsValidCobCannonSpotHelper(theGridX + 1, theGridY))
		return false;

	return !GetFlowerPotAt(theGridX, theGridY) == !GetFlowerPotAt(theGridX + 1, theGridY);
}

bool Board::HasValidCobCannonSpot()
{
	Plant* aPlant = nullptr;
	while (IteratePlants(aPlant))
	{
		if (aPlant->mSeedType == SeedType::SEED_KERNELPULT && IsValidCobCannonSpot(aPlant->mPlantCol, aPlant->mRow))
		{
			return true;
		}
	}
	return false;
}

Projectile* Board::AddProjectile(int theX, int theY, int theRenderOrder, int theRow, ProjectileType theProjectileType)
{
	Projectile* aProjectile = mProjectiles.DataArrayAlloc();
	aProjectile->ProjectileInitialize(theX, theY, theRenderOrder, theRow, theProjectileType);
	return aProjectile;
}

bool Board::CanZombieSpawnOnLevel(ZombieType theZombieType, int theLevel)
{
	const ZombieDefinition& aZombieDef = GetZombieDefinition(theZombieType);
	if (theZombieType == ZombieType::ZOMBIE_YETI)
	{
		return gLawnApp->CanSpawnYetis();
	}

	if (theLevel < aZombieDef.mStartingLevel || aZombieDef.mPickWeight == 0)
	{
		return false;
	}

	TOD_ASSERT(gZombieAllowedLevels[theZombieType].mZombieType == theZombieType);
	return gZombieAllowedLevels[theZombieType].mAllowedOnLevel[ClampInt(theLevel - 1, 0, 49)];
}

ZombieType Board::GetIntroducedZombieType()
{
	if (!mApp->IsAdventureMode() || mLevel == 1)
	{
		return ZombieType::ZOMBIE_INVALID;
	}

	for (ZombieType aZombieType = ZombieType::ZOMBIE_NORMAL; aZombieType < ZombieType::NUM_ZOMBIE_TYPES; aZombieType = static_cast<ZombieType>(static_cast<int>(aZombieType) + 1))
	{
		const ZombieDefinition& aZombieDef = GetZombieDefinition(aZombieType);
		if ((aZombieType != ZombieType::ZOMBIE_YETI || mApp->CanSpawnYetis()) && aZombieDef.mStartingLevel == mLevel)
		{
			return aZombieType;
		}
	}
	return ZombieType::ZOMBIE_INVALID;
}

ZombieType Board::PickGraveRisingZombieType()
{
	TodWeightedArray aZombieWeightArray[ZombieType::NUM_ZOMBIE_TYPES];
	int aCount = 2;
	aZombieWeightArray[0].mItem = ZombieType::ZOMBIE_NORMAL;
	aZombieWeightArray[0].mWeight = GetZombieDefinition(ZombieType::ZOMBIE_NORMAL).mPickWeight;
	aZombieWeightArray[1].mItem = ZombieType::ZOMBIE_TRAFFIC_CONE;
	aZombieWeightArray[1].mWeight = GetZombieDefinition(ZombieType::ZOMBIE_TRAFFIC_CONE).mPickWeight;
	if (!StageHasGraveStones())
	{
		aZombieWeightArray[2].mItem = ZombieType::ZOMBIE_PAIL;
		aZombieWeightArray[2].mWeight = GetZombieDefinition(ZombieType::ZOMBIE_PAIL).mPickWeight;
		aCount++;
	}

	for (int i = 0; i < aCount; i++)
	{
		ZombieType aZombieType = static_cast<ZombieType>(aZombieWeightArray[i].mItem);
		const ZombieDefinition& aZombieDef = GetZombieDefinition(aZombieType);
		if ((mApp->IsFirstTimeAdventureMode() && mLevel < aZombieDef.mStartingLevel) || (!mZombieAllowed[aZombieType] && aZombieType != ZombieType::ZOMBIE_NORMAL))
		{
			aZombieWeightArray[i].mWeight = 0;
		}
	}

	return (ZombieType)TodPickFromWeightedArray(aZombieWeightArray, aCount);
}

ZombieType Board::PickZombieType(int theZombiePoints, int theWaveIndex, ZombiePicker* theZombiePicker)
{
	int aPickCount = 0;
	TodWeightedArray aZombieWeightArray[ZombieType::NUM_ZOMBIE_TYPES];
	for (int aZombieType = ZombieType::ZOMBIE_NORMAL; aZombieType < ZombieType::NUM_ZOMBIE_TYPES; aZombieType++)
	{
		if (!mZombieAllowed[aZombieType])
			continue;

		const ZombieDefinition& aZombieDef = GetZombieDefinition((ZombieType)aZombieType);

		// ================================================================================================
		// ▲ 将不符合出怪限制或超出剩余点数的僵尸类型排除
		// ================================================================================================
		GameMode aGameMode = mApp->mGameMode;
		// 蹦极僵尸在无尽模式中仅在旗帜波出现
		if (aZombieType == ZombieType::ZOMBIE_BUNGEE && mApp->IsSurvivalEndless(aGameMode))
		{
			if (!IsFlagWave(theWaveIndex))
			{
				continue;
			}
		}
		// 僵尸最早出现的波数的限制（出怪限制）
		else if (aGameMode != GameMode::GAMEMODE_CHALLENGE_POGO_PARTY && aGameMode != GameMode::GAMEMODE_CHALLENGE_BOBSLED_BONANZA && aGameMode != GameMode::GAMEMODE_CHALLENGE_AIR_RAID)
		{
			int aFirstAllowedWave = aZombieDef.mFirstAllowedWave;
			// 无尽模式中，僵尸最早可出现的波数逐渐前移
			if (mApp->IsSurvivalEndless(aGameMode))
			{
				int aFlags = GetSurvivalFlagsCompleted();
				int aAllowedWave = aFirstAllowedWave - TodAnimateCurve(18, 50, aFlags, 0, 15, TodCurves::CURVE_LINEAR);
				aFirstAllowedWave = std::max(aAllowedWave, 1);
			}
			if (theWaveIndex + 1 < aFirstAllowedWave || theZombiePoints < aZombieDef.mZombieValue)
			{
				continue;
			}
		}

		// ================================================================================================
		// ▲ 生存模式中，根据当前旗帜数等重新计算僵尸的权重
		// ================================================================================================
		int aPickWeight = aZombieDef.mPickWeight;
		if (mApp->IsSurvivalMode())
		{
			int aFlags = GetSurvivalFlagsCompleted();
			// 伽刚特尔和雪橇车僵尸的每波出怪上限
			if (aZombieType == ZombieType::ZOMBIE_GARGANTUAR || aZombieType == ZombieType::ZOMBIE_ZAMBONI)
			{
				if (theZombiePicker->mZombieTypeCount[aZombieType] >= TodAnimateCurve(10, 50, aFlags, 2, 50, TodCurves::CURVE_LINEAR))
				{
					continue;
				}
			}
			// 红眼的旗帜波出怪上限和非旗帜波出怪总和上限
			else if (aZombieType == ZombieType::ZOMBIE_REDEYE_GARGANTUAR)
			{
				if (IsFlagWave(theWaveIndex))
				{
					if (theZombiePicker->mZombieTypeCount[aZombieType] >= TodAnimateCurve(14, 100, aFlags, 1, 50, TodCurves::CURVE_LINEAR))
					{
						continue;
					}
				}
				else
				{
					if (theZombiePicker->mAllWavesZombieTypeCount[aZombieType] >= TodAnimateCurve(10, 110, aFlags, 1, 50, TodCurves::CURVE_LINEAR))
					{
						continue;
					}
					aPickWeight = 1000;
				}
			}
			// 普通僵尸和路障僵尸的权重衰减
			else if (aZombieType == ZombieType::ZOMBIE_NORMAL)
			{
				aPickWeight = TodAnimateCurve(10, 50, aFlags, aPickWeight, aPickWeight / 10, TodCurves::CURVE_LINEAR);
			}
			else if (aZombieType == ZombieType::ZOMBIE_TRAFFIC_CONE)
			{
				aPickWeight = TodAnimateCurve(10, 50, aFlags, aPickWeight, aPickWeight / 4, TodCurves::CURVE_LINEAR);
			}
		}
		aZombieWeightArray[aPickCount].mItem = aZombieType;
		aZombieWeightArray[aPickCount].mWeight = aPickWeight;
		aPickCount++;
	}

	// 加权随机地取得一种可能的僵尸类型并返回
	return (ZombieType)TodPickFromWeightedArray(aZombieWeightArray, aPickCount);
}

bool Board::IsZombieTypePoolOnly(ZombieType theZombieType)
{
	return (theZombieType == ZombieType::ZOMBIE_SNORKEL || theZombieType == ZombieType::ZOMBIE_DOLPHIN_RIDER);
}

bool Board::RowCanHaveZombieType(int theRow, ZombieType theZombieType)
{
	if (!RowCanHaveZombies(theRow))
	{
		return false;
	}

	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_RESODDED && mPlantRow[theRow] == PlantRowType::PLANTROW_DIRT && mCurrentWave < 5)
	{
		return false;  // 无草皮之地关卡，无草皮的行在前 5 波不刷出僵尸
	}
	if (mPlantRow[theRow] == PlantRowType::PLANTROW_POOL && !Zombie::ZombieTypeCanGoInPool(theZombieType) && theZombieType != ZombieType::ZOMBIE_BALLOON)
	{
		return false;  // 水路不会刷出不能进入泳池的僵尸
	}
	if (mPlantRow[theRow] == PlantRowType::PLANTROW_HIGH_GROUND && !Zombie::ZombieTypeCanGoOnHighGround(theZombieType))
	{
		return false;  // 高地不会刷出不能走上高地的僵尸
	}

	int aCurrentWave = mCurrentWave;
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND)
	{
		aCurrentWave += mChallenge->mSurvivalStage * GetNumWavesPerSurvivalStage();
	}
	// 非水路不能刷出水路僵尸；前 5 小波，水面仅刷出潜水僵尸或海豚骑士僵尸
	if (mPlantRow[theRow] == PlantRowType::PLANTROW_POOL)
	{
		if (aCurrentWave < 5 && !IsZombieTypePoolOnly(theZombieType))
		{
			return false;
		}
	}
	else if (IsZombieTypePoolOnly(theZombieType))
	{
		return false;
	}
	// 雪橇僵尸小队仅能在有冰道的行刷出
	if (theZombieType == ZOMBIE_BOBSLED && !mIceTimer[theRow])
	{
		return false;
	}
	// “自古一路无巨人”（生存模式除外）
	if (theRow == 0 && !mApp->IsSurvivalMode())
	{
		if (theZombieType == ZombieType::ZOMBIE_GARGANTUAR || theZombieType == ZombieType::ZOMBIE_REDEYE_GARGANTUAR)
		{
			return false;
		}
	}
	// 非舞王僵尸或当前为泳池关卡，则可以刷出该僵尸
	if (theZombieType != ZombieType::ZOMBIE_DANCER || StageHasPool())
	{
		return true;
	}
	// 舞王僵尸在非泳池关卡中，为保证能召唤伴舞僵尸，仅在中间三行刷出
	return RowCanHaveZombies(theRow - 1) && RowCanHaveZombies(theRow + 1);
}

int Board::PickRowForNewZombie(ZombieType theZombieType)
{
	// ====================================================================================================
	// ▲ 当存在正在寻找目标僵尸的钉耙，且僵尸可以出现在钉耙所在行时，优先出现在钉耙所在行
	// ====================================================================================================
	GridItem* aRake = GetRake();
	if (aRake && aRake->mGridItemState == GridItemState::GRIDITEM_STATE_RAKE_ATTRACTING && RowCanHaveZombieType(aRake->mGridY, theZombieType))
	{
		aRake->mGridItemState = GridItemState::GRIDITEM_STATE_RAKE_WAITING;
		TodUpdateSmoothArrayPick(mRowPickingArray, MAX_GRID_SIZE_Y, aRake->mGridY);
		return aRake->mGridY;
	}

	// ====================================================================================================
	// ▲ 遍历每一行，将所有能允许该僵尸出现的行及其对应权重写入挑选数组中
	// ====================================================================================================
	for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++)
	{
		// 如果本行不能出现目标僵尸，则将本行权重置零，并继续下一行
		if (!RowCanHaveZombieType(aRow, theZombieType))
		{
			mRowPickingArray[aRow].mWeight = 0;
		}
		// 保护传送门关卡中，每行的出怪概率受传送门位置影响
		else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_PORTAL_COMBAT)
		{
			mRowPickingArray[aRow].mWeight = mChallenge->PortalCombatRowSpawnWeight(aRow);
		}
		// 隐形食脑者关卡中，前 3 波第六路不出怪
		else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_INVISIGHOUL && mCurrentWave <= 3 && aRow == 5)
		{
			mRowPickingArray[aRow].mWeight = 0;
		}
		// 丢车保护
		else
		{
			int aWavesMowered = mCurrentWave - mWaveRowGotLawnMowered[aRow];
			if (mApp->IsContinuousChallenge() && mCurrentWave == mNumWaves - 1)
			{
				aWavesMowered = 100;
			}

			if (aWavesMowered <= 1)
			{
				mRowPickingArray[aRow].mWeight = 0.01f;
			}
			else if (aWavesMowered <= 2)
			{
				mRowPickingArray[aRow].mWeight = 0.5f;
			}
			else
			{
				mRowPickingArray[aRow].mWeight = 1.0f;
			}
		}
	}
	return TodPickFromSmoothArray(mRowPickingArray, MAX_GRID_SIZE_Y);
}

bool Board::CanAddBobSled()
{
	for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++)
	{
		if (mIceTimer[aRow] > 0 && mIceMinX[aRow] < 700)
		{
			return true;
		}
	}
	return false;
}

// GOTY @Patoke: 0x410700
Zombie* Board::AddZombieInRow(ZombieType theZombieType, int theRow, int theFromWave)
{
	if (mZombies.mSize >= mZombies.mMaxSize - 1)
	{
		TodTrace("Too many zombies!!");
		return nullptr;
	}

	// @Patoke: implemented
	if (theZombieType == ZombieType::ZOMBIE_YETI) {
		if (mApp->IsAdventureMode() && mLevel == 40 && theFromWave >= 0)
			ReportAchievement::GiveAchievement(mApp, Zombologist, true);
	}

	bool aVariant = !Rand(5);
	Zombie* aZombie = mZombies.DataArrayAlloc();
	aZombie->ZombieInitialize(theRow, theZombieType, aVariant, nullptr, theFromWave);
	if (theZombieType == ZombieType::ZOMBIE_BOBSLED && aZombie->IsOnBoard())
	{
		for (int _i = 0; _i < 3; _i++)
		{
			mZombies.DataArrayAlloc()->ZombieInitialize(theRow, ZombieType::ZOMBIE_BOBSLED, false, aZombie, theFromWave);
		}
	}
	return aZombie;
}

Zombie* Board::AddZombie(ZombieType theZombieType, int theFromWave)
{
	return AddZombieInRow(theZombieType, PickRowForNewZombie(theZombieType), theFromWave); 
}

void Board::RemoveAllZombies()
{
	Zombie* aZombie = nullptr;
	while (IterateZombies(aZombie))
	{
		if (!aZombie->IsDeadOrDying())
		{
			aZombie->DieNoLoot();
		}
	}
}

void Board::RemoveZombiesForRepick()
{
	Zombie* aZombie = nullptr;
	while (IterateZombies(aZombie))
	{
		if (!aZombie->IsDeadOrDying() && aZombie->mMindControlled && aZombie->mPosX > 720)
		{
			aZombie->DieNoLoot();
		}
	}
}

void Board::RemoveCutsceneZombies()
{
	Zombie* aZombie = nullptr;
	while (IterateZombies(aZombie))
	{
		if (aZombie->mFromWave == Zombie::ZOMBIE_WAVE_CUTSCENE)
		{
			aZombie->DieNoLoot();
		}
	}
}

bool Board::IsIceAt(int theGridX, int theGridY)
{
	TOD_ASSERT(theGridY >= 0 && theGridY < MAX_GRID_SIZE_Y);
	if (mIceTimer[theGridY] == 0 || mIceMinX[theGridY] > 750)
		return false;

	return theGridX >= PixelToGridXKeepOnBoard(mIceMinX[theGridY] + 12, 0);
}

PlantingReason Board::CanPlantAt(int theGridX, int theGridY, SeedType theSeedType)
{
	// 目标位置不在场地内，则返回“不能种在那里”
	if (theGridX < 0 || theGridX >= MAX_GRID_SIZE_X || theGridY < 0 || theGridY >= MAX_GRID_SIZE_Y)
	{
		return PlantingReason::PLANTING_NOT_HERE;
	}

	// 从关卡玩法上，判断能否种植
	PlantingReason aReason = mChallenge->CanPlantAt(theGridX, theGridY, theSeedType);
	if (aReason != PlantingReason::PLANTING_OK || Challenge::IsZombieSeedType(theSeedType))
	{
		return aReason;
	}

	PlantsOnLawn aPlantOnLawn;
	GetPlantsOnLawn(theGridX, theGridY, &aPlantOnLawn);
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		if (aPlantOnLawn.mUnderPlant || aPlantOnLawn.mPumpkinPlant || aPlantOnLawn.mFlyingPlant || aPlantOnLawn.mNormalPlant)
		{
			return PlantingReason::PLANTING_NOT_HERE;
		}
		if (mApp->mZenGarden->mGardenType == GARDEN_AQUARIUM && !Plant::IsAquatic(theSeedType))
		{
			return PlantingReason::PLANTING_NOT_ON_WATER;
		}

		return PlantingReason::PLANTING_OK;
	}

	// 墓碑吞噬者只能种植在墓碑上
	bool aHasGrave = GetGraveStoneAt(theGridX, theGridY);
	if (theSeedType == SeedType::SEED_GRAVEBUSTER)
	{
		if (aPlantOnLawn.mNormalPlant)
		{
			return PlantingReason::PLANTING_NOT_HERE;
		}

		return aHasGrave ? PlantingReason::PLANTING_OK : PlantingReason::PLANTING_ONLY_ON_GRAVES;
	}
	if (theSeedType == SeedType::SEED_INSTANT_COFFEE)
	{
		if (aPlantOnLawn.mFlyingPlant)
		{
			return PlantingReason::PLANTING_NOT_HERE;
		}

		if (!aPlantOnLawn.mNormalPlant || !aPlantOnLawn.mNormalPlant->mIsAsleep || aPlantOnLawn.mNormalPlant->mWakeUpCounter > 0 ||
			aPlantOnLawn.mNormalPlant->mOnBungeeState == PlantOnBungeeState::GETTING_GRABBED_BY_BUNGEE)
		{
			return PlantingReason::PLANTING_NEEDS_SLEEPING;
		}

		return PlantingReason::PLANTING_OK;
	}
	// 非墓碑吞噬者且非飞行植物，则不能种在墓碑上
	if (aHasGrave)
	{
		return Plant::IsFlying(theSeedType) ? PlantingReason::PLANTING_OK : PlantingReason::PLANTING_NOT_ON_GRAVE;
	}
	
	Plant* aUnderPlant = aPlantOnLawn.mUnderPlant;
	bool aHasLilypad, aHasFlowerPot;
	if (!aUnderPlant || aUnderPlant->mOnBungeeState == PlantOnBungeeState::GETTING_GRABBED_BY_BUNGEE)
	{
		aHasLilypad = false;
		aHasFlowerPot = false;
	}
	else
	{
		aHasLilypad = aUnderPlant->mSeedType == SeedType::SEED_LILYPAD;
		aHasFlowerPot = aUnderPlant->mSeedType == SeedType::SEED_FLOWERPOT;
	}
	// 部分情况下的格子中不能种植植物
	if (GetCraterAt(theGridX, theGridY))
	{
		return PlantingReason::PLANTING_NOT_ON_CRATER;
	}
	if (GetScaryPotAt(theGridX, theGridY) || IsIceAt(theGridX, theGridY))
	{
		return PlantingReason::PLANTING_NOT_HERE;
	}
	GridSquareType aGridSquare = mGridSquareType[theGridX][theGridY];
	if (aGridSquare == GridSquareType::GRIDSQUARE_DIRT || aGridSquare == GridSquareType::GRIDSQUARE_NONE)
	{
		return PlantingReason::PLANTING_NOT_HERE;
	}
	// 水生植物只能种在水上
	Plant* aNormalPlant = aPlantOnLawn.mNormalPlant;
	if (theSeedType == SeedType::SEED_LILYPAD || theSeedType == SeedType::SEED_TANGLEKELP || theSeedType == SeedType::SEED_SEASHROOM)
	{
		if (!IsPoolSquare(theGridX, theGridY))
		{
			return PlantingReason::PLANTING_ONLY_IN_POOL;
		}

		return (aNormalPlant || aUnderPlant) ? PlantingReason::PLANTING_NOT_HERE : PlantingReason::PLANTING_OK;
	}
	if (Plant::IsFlying(theSeedType))
	{
		return aPlantOnLawn.mFlyingPlant ? PlantingReason::PLANTING_NOT_HERE : PlantingReason::PLANTING_OK;
	}
	// 地刺/地刺王只能种在坚固的地面
	if (theSeedType == SeedType::SEED_SPIKEWEED || theSeedType == SeedType::SEED_SPIKEROCK)
	{
		if (aGridSquare == GridSquareType::GRIDSQUARE_POOL || StageHasRoof() || aUnderPlant)
		{
			return PlantingReason::PLANTING_NEEDS_GROUND;
		}
	}
	// 非水生植物不能种在水面上（南瓜头可以种在香蒲上）
	Plant* aPumpkinPlant = aPlantOnLawn.mPumpkinPlant;
	if (aGridSquare == GridSquareType::GRIDSQUARE_POOL && !aHasLilypad && theSeedType != SeedType::SEED_CATTAIL)
	{
		if (!aNormalPlant || aNormalPlant->mSeedType != SeedType::SEED_CATTAIL || theSeedType != SeedType::SEED_PUMPKINSHELL)
		{
			return PlantingReason::PLANTING_NOT_ON_WATER;
		}
	}
	// 花盆的种植条件
	if (theSeedType == SeedType::SEED_FLOWERPOT)
	{
		return (aNormalPlant || aUnderPlant || aPumpkinPlant) ? PlantingReason::PLANTING_NOT_HERE : PlantingReason::PLANTING_OK;
	}
	// 屋顶种植需要花盆
	if (StageHasRoof() && !aHasFlowerPot)
	{
		return PlantingReason::PLANTING_NEEDS_POT;
	}
	// 南瓜头的种植条件
	bool aAidPurchased = mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_FIRSTAID] > 0;
	if (theSeedType == SeedType::SEED_PUMPKINSHELL)
	{
		// 不可种植在玉米加农炮上
		if (aNormalPlant && aNormalPlant->mSeedType == SeedType::SEED_COBCANNON)
		{
			return PlantingReason::PLANTING_NOT_HERE;
		}
		// 无南瓜头时，可以种植南瓜头
		if (!aPumpkinPlant)
		{
			return PlantingReason::PLANTING_OK;
		}
		// 南瓜头的坚果包扎术
		if (aAidPurchased && aPumpkinPlant->mPlantHealth < aPumpkinPlant->mPlantMaxHealth * 2 / 3 &&
			aPumpkinPlant->mSeedType == SeedType::SEED_PUMPKINSHELL && aPumpkinPlant->mOnBungeeState != PlantOnBungeeState::GETTING_GRABBED_BY_BUNGEE)
		{
			return PlantingReason::PLANTING_OK;
		}

		return PlantingReason::PLANTING_NOT_HERE;
	}
	// 土豆地雷只能种在陆地上
	if (aHasLilypad && theSeedType == SeedType::SEED_POTATOMINE)
	{
		return PlantingReason::PLANTING_ONLY_ON_GROUND;
	}

	if (aUnderPlant)
	{
		// 香蒲对底端植物的紫卡升级
		if (theSeedType == SeedType::SEED_CATTAIL)
		{
			if (aNormalPlant)
			{
				return PlantingReason::PLANTING_NOT_HERE;
			}
			if (aUnderPlant->IsUpgradableTo(theSeedType) && aUnderPlant->mOnBungeeState != PlantOnBungeeState::GETTING_GRABBED_BY_BUNGEE)
			{
				return PlantingReason::PLANTING_OK;
			}
			if (Plant::IsUpgrade(theSeedType))
			{
				return PlantingReason::PLANTING_NEEDS_UPGRADE;
			}
		}
		else
		{
			// 模仿中的模仿者不可作为花盆或睡莲
			if (aUnderPlant->mSeedType == SeedType::SEED_IMITATER)
			{
				return PlantingReason::PLANTING_NOT_HERE;
			}
		}
	}

	// 一般紫卡植物的更迭判断
	if (aNormalPlant)
	{
		// 紫卡植物的升级
		if (aNormalPlant->IsUpgradableTo(theSeedType) && aNormalPlant->mOnBungeeState != PlantOnBungeeState::GETTING_GRABBED_BY_BUNGEE)
		{
			return PlantingReason::PLANTING_OK;
		}
		if (Plant::IsUpgrade(theSeedType))
		{
			return PlantingReason::PLANTING_NEEDS_UPGRADE;
		}

		// 坚果包扎术
		if ((theSeedType == SeedType::SEED_WALLNUT || theSeedType == SeedType::SEED_TALLNUT) && aAidPurchased)
		{
			if (aNormalPlant->mPlantHealth < aNormalPlant->mPlantMaxHealth * 2 / 3 &&
				aNormalPlant->mSeedType == theSeedType && aNormalPlant->mOnBungeeState != PlantOnBungeeState::GETTING_GRABBED_BY_BUNGEE)
			{
				return PlantingReason::PLANTING_OK;
			}
		}

		return PlantingReason::PLANTING_NOT_HERE;
	}

	// 免费种植模式下紫卡的额外判断
	if (!mApp->mEasyPlantingCheat && Plant::IsUpgrade(theSeedType))
	{
		return PlantingReason::PLANTING_NEEDS_UPGRADE;
	}
	if (theSeedType == SeedType::SEED_COBCANNON && !IsValidCobCannonSpot(theGridX, theGridY))
	{
		return PlantingReason::PLANTING_NEEDS_UPGRADE;
	}
	else if (theSeedType == SeedType::SEED_CATTAIL && aGridSquare != GridSquareType::GRIDSQUARE_POOL)
	{
		return PlantingReason::PLANTING_NOT_HERE;
	}

	return PlantingReason::PLANTING_OK;
}

void Board::UpdateCursor()
{
	int aMouseX = mApp->mWidgetManager->mLastMouseX - mX;
	int aMouseY = mApp->mWidgetManager->mLastMouseY - mY;
	bool aShowFinger = false;
	bool aShowDrag = false;
	bool aHideCursor = false;

	if (mApp->mSeedChooserScreen && mApp->mSeedChooserScreen->Contains(aMouseX + mX, aMouseY + mY))
		return;

	if (mApp->GetDialogCount() > 0)
		return;

	if (mPaused || mBoardFadeOutCounter >= 0 || mTimeStopCounter > 0 || mApp->mGameScene == GameScenes::SCENE_ZOMBIES_WON)
	{
		mApp->SetCursor(Sexy::CURSOR_POINTER);
		return;
	}

	HitResult aHitResult;
	MouseHitTest(aMouseX, aMouseY, &aHitResult);
	switch (aHitResult.mObjectType)
	{
	case GameObjectType::OBJECT_TYPE_MENU_BUTTON:
	case GameObjectType::OBJECT_TYPE_STORE_BUTTON:
	case GameObjectType::OBJECT_TYPE_SHOVEL:
	case GameObjectType::OBJECT_TYPE_WATERING_CAN:
	case GameObjectType::OBJECT_TYPE_FERTILIZER:
	case GameObjectType::OBJECT_TYPE_BUG_SPRAY:
	case GameObjectType::OBJECT_TYPE_PHONOGRAPH:
	case GameObjectType::OBJECT_TYPE_CHOCOLATE:
	case GameObjectType::OBJECT_TYPE_GLOVE:
	case GameObjectType::OBJECT_TYPE_MONEY_SIGN:
	case GameObjectType::OBJECT_TYPE_NEXT_GARDEN:
	case GameObjectType::OBJECT_TYPE_WHEELBARROW:
	case GameObjectType::OBJECT_TYPE_SLOT_MACHINE_HANDLE:
	case GameObjectType::OBJECT_TYPE_TREE_FOOD:
	case GameObjectType::OBJECT_TYPE_STINKY:
	case GameObjectType::OBJECT_TYPE_TREE_OF_WISDOM:
	case GameObjectType::OBJECT_TYPE_COIN:
	case GameObjectType::OBJECT_TYPE_PROJECTILE:
		aShowFinger = true;
		break;

	case GameObjectType::OBJECT_TYPE_SEEDPACKET:
		aShowFinger = ((SeedPacket*)aHitResult.mObject)->CanPickUp();
		break;

	case GameObjectType::OBJECT_TYPE_SCARY_POT:
		if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_NORMAL)
		{
			aShowFinger = true;
		}
		else if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_HAMMER)
		{
			aHideCursor = true;
		}
		break;

	case GameObjectType::OBJECT_TYPE_PLANT:
		if ((mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST) && !HasLevelAwardDropped())
		{
			aShowFinger = true;
		}
		if (((Plant*)aHitResult.mObject)->mState == PlantState::STATE_COBCANNON_READY)
		{
			aShowFinger = true;
		}
		break;

	default:
		if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_HAMMER)
		{
			aHideCursor = true;
		}
		break;
	}

	if (mChallenge->mBeghouledMouseCapture || aShowDrag)
	{
		mApp->SetCursor(Sexy::CURSOR_DRAGGING);
	}
	else if (aShowFinger)
	{
		mApp->SetCursor(Sexy::CURSOR_HAND);
	}
	else if (aHideCursor)
	{
		mApp->SetCursor(Sexy::CURSOR_NONE);
	}
	else
	{
		mApp->SetCursor(Sexy::CURSOR_POINTER);
	}
}

void Board::MouseMove(int x, int y)
{
	Widget::MouseMove(x, y);
	mChallenge->MouseMove(x, y);
}

void Board::MouseDrag(int x, int y)
{
	Widget::MouseDrag(x, y);
	mChallenge->MouseMove(x, y);
}

Zombie* Board::ZombieHitTest(int theMouseX, int theMouseY)
{
	Zombie* aZombie = nullptr;
	Zombie* aRecord = nullptr;
	while (IterateZombies(aZombie))
	{
		// 排除已死亡的僵尸
		if (aZombie->mDead || aZombie->IsDeadOrDying())
			continue;

		// 排除关卡引入阶段及选卡界面的植物僵尸
		if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO && Zombie::IsZombotany(aZombie->mZombieType))
			continue;

		// 范围判定
		if (aZombie->GetZombieRect().Contains(theMouseX, theMouseY))
		{
			if (aRecord == nullptr || aZombie->mY > aRecord->mY)
			{
				aRecord = aZombie;
			}
		}
	}

	return aRecord;
}

bool Board::IsPlantInGoldWateringCanRange(int theMouseX, int theMouseY, Plant* thePlant)
{
	/*
	Rect aRect = Rect(theMouseX - 70, theMouseY - 80, 160, 160);
	if (GetTopPlantAt(thePlant->mPlantCol, thePlant->mRow, PlantPriority::TOPPLANT_ZEN_TOOL_ORDER) == thePlant)
	{
		return aRect.Contains(thePlant->mX + 40, thePlant->mY + 40);
	}
	return false;
	*/

	int aMinX = theMouseX - 70;
	int aMaxX = theMouseX + 90;
	int aMinY = theMouseY - 80;
	int aMaxY = theMouseY + 80;

	if (GetTopPlantAt(thePlant->mPlantCol, thePlant->mRow, PlantPriority::TOPPLANT_ZEN_TOOL_ORDER) == thePlant)
	{
		return thePlant->mX + 40 >= aMinX && thePlant->mX + 40 < aMaxX && thePlant->mY + 40 >= aMinY && thePlant->mY + 40 < aMaxY;
	}
	return false;
}

void Board::HighlightPlantsForMouse(int theMouseX, int theMouseY)
{
	if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_WATERING_CAN && mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_GOLD_WATERINGCAN])
	{
		Plant* aPlant = nullptr;
		while (IteratePlants(aPlant))
		{
			if (IsPlantInGoldWateringCanRange(theMouseX, theMouseY, aPlant))
			{
				aPlant->mHighlighted = true;
				Plant* aFlowerPot = GetTopPlantAt(aPlant->mPlantCol, aPlant->mRow, PlantPriority::TOPPLANT_ONLY_UNDER_PLANT);
				if (aFlowerPot)
				{
					aFlowerPot->mHighlighted = true;
				}
			}
		}
	}
	else
	{
		Plant* aPlant = ToolHitTest(theMouseX, theMouseY);
		if (aPlant)
		{
			aPlant->mHighlighted = true;
			if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
			{
				Plant* aFlowerPot = GetTopPlantAt(aPlant->mPlantCol, aPlant->mRow, PlantPriority::TOPPLANT_ONLY_UNDER_PLANT);
				if (aFlowerPot)
				{
					aFlowerPot->mHighlighted = true;
				}
			}
		}
	}
}

void Board::UpdateMousePosition()
{
	UpdateCursor();
	UpdateToolTip();
	Plant* aPlant = nullptr;
	while (IteratePlants(aPlant))
	{
		aPlant->mHighlighted = false;
	}

	SeedType aCursorSeedType = GetSeedTypeInCursor();
	int aMouseX = mApp->mWidgetManager->mLastMouseX - mX;
	int aMouseY = mApp->mWidgetManager->mLastMouseY - mY;

	// 破罐者关卡中，检测并高亮鼠标悬浮的罐子
	if (mApp->IsScaryPotterLevel())
	{
		GridItem* aGridItem = nullptr;
		while (IterateGridItems(aGridItem))
		{
			if (aGridItem->mGridItemType == GridItemType::GRIDITEM_SCARY_POT)
			{
				aGridItem->mHighlighted = false;
			}
		}

		HitResult aHitResult;
		MouseHitTest(aMouseX, aMouseY, &aHitResult);
		if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_SCARY_POT)
		{
			GridItem* aScaryPot = (GridItem*)aHitResult.mObject;
			aScaryPot->mHighlighted = true;
			return;
		}
	}

	// 禅境花园，设定蜗牛的高亮与否
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		GridItem* aStinky = mApp->mZenGarden->GetStinky();
		if (aStinky)
		{
			HitResult aHitResult;
			MouseHitTest(aMouseX, aMouseY, &aHitResult);
			aStinky->mHighlighted = aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_STINKY;
		}
	}

	// 手持铲子或花园工具时，令作用的植物高亮
	if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_SHOVEL || 
		mCursorObject->mCursorType == CursorType::CURSOR_TYPE_WATERING_CAN || 
		mCursorObject->mCursorType == CursorType::CURSOR_TYPE_FERTILIZER ||
		mCursorObject->mCursorType == CursorType::CURSOR_TYPE_BUG_SPRAY || 
		mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PHONOGRAPH || 
		mCursorObject->mCursorType == CursorType::CURSOR_TYPE_CHOCOLATE ||
		mCursorObject->mCursorType == CursorType::CURSOR_TYPE_GLOVE || 
		mCursorObject->mCursorType == CursorType::CURSOR_TYPE_MONEY_SIGN ||
		(mCursorObject->mCursorType == CursorType::CURSOR_TYPE_WHEEELBARROW && !mApp->mZenGarden->GetPottedPlantInWheelbarrow()))
	{
		HighlightPlantsForMouse(aMouseX, aMouseY);
		return;
	}

	// 咖啡豆及坚果包扎术
	if (aCursorSeedType == SeedType::SEED_INSTANT_COFFEE)
	{
		int aGridX = PlantingPixelToGridX(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY, aCursorSeedType);
		int aGridY = PlantingPixelToGridY(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY, aCursorSeedType);

		Plant* aPlant = GetTopPlantAt(aGridX, aGridY, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION);
		if (aPlant && aPlant->mIsAsleep && CanPlantAt(aGridX, aGridY, SeedType::SEED_INSTANT_COFFEE) == PlantingReason::PLANTING_OK)
		{
			aPlant->mHighlighted = true;
		}
	}
	else if (aCursorSeedType == SeedType::SEED_WALLNUT || aCursorSeedType == SeedType::SEED_TALLNUT)
	{
		int aGridX = PlantingPixelToGridX(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY, aCursorSeedType);
		int aGridY = PlantingPixelToGridY(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY, aCursorSeedType);

		Plant* aPlant = GetTopPlantAt(aGridX, aGridY, PlantPriority::TOPPLANT_ONLY_PUMPKIN);
		if (aPlant && aPlant->mSeedType == aCursorSeedType && CanPlantAt(aGridX, aGridY, aCursorSeedType) == PlantingReason::PLANTING_OK)
		{
			aPlant->mHighlighted = true;
		}
	}
	else if (aCursorSeedType == SeedType::SEED_PUMPKINSHELL)
	{
		int aGridX = PlantingPixelToGridX(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY, aCursorSeedType);
		int aGridY = PlantingPixelToGridY(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY, aCursorSeedType);

		Plant* aPlant = GetTopPlantAt(aGridX, aGridY, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION);
		if (aPlant && aPlant->mSeedType == SeedType::SEED_PUMPKINSHELL && CanPlantAt(aGridX, aGridY, SeedType::SEED_PUMPKINSHELL) == PlantingReason::PLANTING_OK)
		{
			aPlant->mHighlighted = true;
		}
	}
}

void Board::UpdateToolTip()
{
	if (!mApp->mWidgetManager->mMouseIn || !mApp->mActive || mTimeStopCounter > 0 || mApp->GetDialogCount() > 0 || mApp->mGameScene == GameScenes::SCENE_ZOMBIES_WON)
	{
		mToolTip->mVisible = false;
		return;
	}

	int aMouseX = mApp->mWidgetManager->mLastMouseX - mX;
	int aMouseY = mApp->mWidgetManager->mLastMouseY - mY;

	if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO)
	{
		if (!mCutScene->mSeedChoosing)
		{
			mToolTip->mVisible = false;
			return;
		}

		if (mSeedBank->ContainsPoint(mWidgetManager->mLastMouseX, mWidgetManager->mLastMouseY) ||
			mApp->mSeedChooserScreen->mAlmanacButton->IsMouseOver() ||
			mApp->mSeedChooserScreen->mStoreButton->IsMouseOver() ||
			mApp->mSeedChooserScreen->mImitaterButton->IsMouseOver())
		{
			mToolTip->mVisible = false;
			return;
		}

		Zombie* aZombie = ZombieHitTest(aMouseX, aMouseY);
		if (aZombie == nullptr || aZombie->mFromWave != Zombie::ZOMBIE_WAVE_CUTSCENE)
		{
			mToolTip->mVisible = false;
			return;
		}

		std::string aZombieName = StrFormat("[%s]", GetZombieDefinition(aZombie->mZombieType).mZombieName);
		mToolTip->SetTitle(aZombieName);
		if (mApp->CanShowAlmanac() && aZombie->mZombieType != ZombieType::ZOMBIE_REDEYE_GARGANTUAR)
		{
			mToolTip->SetLabel("[CLICK_TO_VIEW]");
		}
		else
		{
			mToolTip->SetLabel("");
		}
		mToolTip->SetWarningText("");

		Rect aRect = aZombie->GetZombieRect();
		mToolTip->mX = aRect.mWidth / 2 + aRect.mX + 5;
		mToolTip->mY = aRect.mHeight + aRect.mY - 10;
		if (aZombie->mZombieType == ZombieType::ZOMBIE_BUNGEE)
		{
			mToolTip->mY = aZombie->mY;
		}

		mToolTip->mVisible = true;
		mToolTip->mCenter = true;

		mToolTip->mMinLeft = IMAGE_SEEDCHOOSER_BACKGROUND->GetWidth();
		if (mApp->mSeedChooserScreen->mAlmanacButton->mBtnNoDraw && mApp->mSeedChooserScreen->mStoreButton->mBtnNoDraw)
		{
			mToolTip->mMaxBottom = 600;
		}
		else
		{
			mToolTip->mMaxBottom = 570;
		}
		if (!mApp->mSeedChooserScreen->mImitaterButton->mBtnNoDraw)
		{
			mToolTip->CalculateSize();
			if (mX + mToolTip->mX - mToolTip->mWidth / 2 < 524)
			{
				mToolTip->mMaxBottom = 503;
			}
		}

		return;
	}

	if (!CanInteractWithBoardButtons())
	{
		mToolTip->mVisible = false;
		return;
	}

	mToolTip->mMinLeft = 0;
	mToolTip->mMaxBottom = BOARD_HEIGHT;
	mToolTip->SetTitle("");
	mToolTip->SetLabel("");
	mToolTip->SetWarningText("");
	mToolTip->mCenter = false;
	if (mChallenge->UpdateToolTip(aMouseX, aMouseY))
	{
		return;
	}

	HitResult aHitResult;
	MouseHitTest(aMouseX, aMouseY, &aHitResult);

	switch (aHitResult.mObjectType)
	{
	case GameObjectType::OBJECT_TYPE_SHOVEL:
	{
		mToolTip->SetLabel("[SHOVEL_TOOLTIP]");
		Rect aShovelButtonRect = GetShovelButtonRect();
		mToolTip->mX = aShovelButtonRect.mX + 35;
		mToolTip->mY = aShovelButtonRect.mY + 72;
		mToolTip->mCenter = true;
		mToolTip->mVisible = true;
		return;
	}

	case GameObjectType::OBJECT_TYPE_NEXT_GARDEN:
	{
		mToolTip->SetLabel("[NEXT_GARDEN_TOOLTIP]");
		Rect aButtonRect = GetShovelButtonRect();
		mToolTip->mX = 599;
		mToolTip->mY = aButtonRect.mY + 52;
		mToolTip->mCenter = true;
		mToolTip->mVisible = true;
		return;
	}

	case GameObjectType::OBJECT_TYPE_WATERING_CAN:
		mToolTip->SetLabel("[WATERING_CAN_TOOLTIP]");
		break;
	case GameObjectType::OBJECT_TYPE_FERTILIZER:
		mToolTip->SetLabel("[FERTILIZER_TOOLTIP]");
		break;
	case GameObjectType::OBJECT_TYPE_BUG_SPRAY:
		mToolTip->SetLabel("[BUG_SPRAY_TOOLTIP]");
		break;
	case GameObjectType::OBJECT_TYPE_PHONOGRAPH:
		mToolTip->SetLabel("[PHONOGRAPH_TOOLTIP]");
		break;
	case GameObjectType::OBJECT_TYPE_CHOCOLATE:
		mToolTip->SetLabel("[CHOCOLATE_TOOLTIP]");
		break;
	case GameObjectType::OBJECT_TYPE_GLOVE:
		mToolTip->SetLabel("[GLOVE_TOOLTIP]");
		break;
	case GameObjectType::OBJECT_TYPE_MONEY_SIGN:
		mToolTip->SetLabel("[MONEY_SIGN_TOOLTIP]");
		break;
	case GameObjectType::OBJECT_TYPE_WHEELBARROW:
		mToolTip->SetLabel("[WHEELBARROW_TOOLTIP]");
		break;
	case GameObjectType::OBJECT_TYPE_TREE_FOOD:
		mToolTip->SetLabel("[TREE_FERTILIZER_TOOLTIP]");
		break;
	case GameObjectType::OBJECT_TYPE_SEEDPACKET:
		break;
	default:
		mToolTip->mVisible = false;
		return;
	}

	if (aHitResult.mObjectType != GameObjectType::OBJECT_TYPE_SEEDPACKET)
	{
		Rect aButtonRect = GetShovelButtonRect();
		GetZenButtonRect(aHitResult.mObjectType, aButtonRect);
		this->mToolTip->mX = aButtonRect.mX + 35;
		this->mToolTip->mY = aButtonRect.mY + 72;
		this->mToolTip->mCenter = true;
		this->mToolTip->mVisible = true;
		return;
	}

	SeedPacket* aSeedPacket = (SeedPacket*)aHitResult.mObject;
	SeedType aUseSeedType = aSeedPacket->mPacketType;
	if (aSeedPacket->mPacketType == SeedType::SEED_IMITATER && aSeedPacket->mImitaterType != SeedType::SEED_NONE)
	{
		aUseSeedType = aSeedPacket->mImitaterType;
	}

	if (gLawnApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED || gLawnApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST)
	{
		if (aUseSeedType == SeedType::SEED_REPEATER)
		{
			mToolTip->SetLabel("[BEGHOULED_REPEATER_UPGRADE_TOOLTIP]");
		}
		else if (aUseSeedType == SeedType::SEED_FUMESHROOM)
		{
			mToolTip->SetLabel("[BEGHOULED_FUMESHROOM_UPGRADE_TOOLTIP]");
		}
		else if (aUseSeedType == SeedType::SEED_TALLNUT)
		{
			mToolTip->SetLabel("[BEGHOULED_TALLNUT_UPGRADE_TOOLTIP]");
		}
		else if (aUseSeedType == SeedType::SEED_BEGHOULED_BUTTON_SHUFFLE)
		{
			mToolTip->SetLabel("[BEGHOULED_SHUFFLE_TOOLTIP]");
		}
		else if (aUseSeedType == SeedType::SEED_BEGHOULED_BUTTON_CRATER)
		{
			mToolTip->SetLabel("[BEGHOULED_CRATER_TOOLTIP]");
		}
	}
	else if (aUseSeedType == SeedType::SEED_SLOT_MACHINE_SUN)
	{
		mToolTip->SetLabel("[SLOT_MACHINE_SUN_TOOLTIP]");
	}
	else if (aUseSeedType == SeedType::SEED_SLOT_MACHINE_DIAMOND)
	{
		mToolTip->SetLabel("[SLOT_MACHINE_DIAMOND_TOOLTIP]");
	}
	else if (aUseSeedType == SeedType::SEED_ZOMBIQUARIUM_SNORKLE)
	{
		mToolTip->SetLabel("[ZOMBIQUARIUM_SNORKEL_TOOLTIP]");
	}
	else if (aUseSeedType == SeedType::SEED_ZOMBIQUARIUM_TROPHY)
	{
		mToolTip->SetLabel("[ZOMBIQUARIUM_TROPHY_TOOLTIP]");
	}
	else if (aUseSeedType == SeedType::SEED_ZOMBIE_NORMAL)
	{
		mToolTip->SetLabel("[ZOMBIE]");
	}
	else if (aUseSeedType == SeedType::SEED_ZOMBIE_TRAFFIC_CONE)
	{
		mToolTip->SetLabel("[CONEHEAD_ZOMBIE]");
	}
	else if (aUseSeedType == SeedType::SEED_ZOMBIE_POLEVAULTER)
	{
		mToolTip->SetLabel("[POLE_VAULTING_ZOMBIE]");
	}
	else if (aUseSeedType == SeedType::SEED_ZOMBIE_PAIL)
	{
		mToolTip->SetLabel("[BUCKETHEAD_ZOMBIE]");
	}
	else if (aUseSeedType == SeedType::SEED_ZOMBIE_LADDER)
	{
		mToolTip->SetLabel("[LADDER_ZOMBIE]");
	}
	else if (aUseSeedType == SeedType::SEED_ZOMBIE_DIGGER)
	{
		mToolTip->SetLabel("[DIGGER_ZOMBIE]");
	}
	else if (aUseSeedType == SeedType::SEED_ZOMBIE_BUNGEE)
	{
		mToolTip->SetLabel("[BUNGEE_ZOMBIE]");
	}
	else if (aUseSeedType == SeedType::SEED_ZOMBIE_FOOTBALL)
	{
		mToolTip->SetLabel("[FOOTBALL_ZOMBIE]");
	}
	else if (aUseSeedType == SeedType::SEED_ZOMBIE_BALLOON)
	{
		mToolTip->SetLabel("[BALLOON_ZOMBIE]");
	}
	else if (aUseSeedType == SeedType::SEED_ZOMBIE_SCREEN_DOOR)
	{
		mToolTip->SetLabel("[SCREEN_DOOR_ZOMBIE]");
	}
	else if (aUseSeedType == SeedType::SEED_ZOMBONI)
	{
		mToolTip->SetLabel("[ZOMBONI]");
	}
	else if (aUseSeedType == SeedType::SEED_ZOMBIE_POGO)
	{
		mToolTip->SetLabel("[POGO_ZOMBIE]");
	}
	else if (aUseSeedType == SeedType::SEED_ZOMBIE_DANCER)
	{
		mToolTip->SetLabel("[DANCING_ZOMBIE]");
	}
	else if (aUseSeedType == SeedType::SEED_ZOMBIE_GARGANTUAR)
	{
		mToolTip->SetLabel("[GARGANTUAR]");
	}
	else if (aUseSeedType == SeedType::SEED_ZOMBIE_IMP)
	{
		mToolTip->SetLabel("[IMP]");
	}
	else
	{
		mToolTip->SetLabel(Plant::GetNameString(aSeedPacket->mPacketType, aSeedPacket->mImitaterType));
	}

	int aPlantCost = GetCurrentPlantCost(aSeedPacket->mPacketType, aSeedPacket->mImitaterType);
	if (mApp->mEasyPlantingCheat)
	{
		mToolTip->SetWarningText("FREE_PLANTING_CHEAT");
	}
	else if (!aSeedPacket->mActive && (gLawnApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED || gLawnApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST))
	{
		if (aSeedPacket->mPacketType == SeedType::SEED_BEGHOULED_BUTTON_CRATER)
		{
			mToolTip->SetWarningText("[BEGHOULED_NO_CRATERS]");
		}
		else
		{
			mToolTip->SetWarningText("[BEGHOULED_SEED_ALREADY_PURCHASED]");
		}
	}
	else if (!aSeedPacket->mActive)
	{
		mToolTip->SetWarningText("[WAITING_FOR_SEED]");
	}
	else if (!CanTakeSunMoney(aPlantCost) && !HasConveyorBeltSeedBank() && !mApp->IsSlotMachineLevel())
	{
		mToolTip->SetWarningText("[NOT_ENOUGH_SUN]");
	}
	else if (aUseSeedType == SeedType::SEED_GATLINGPEA)
	{
		if (!PlantingRequirementsMet(aUseSeedType))
		{
			mToolTip->SetWarningText("[REQUIRES_REPEATER]");
		}
	}
	else if (aUseSeedType == SeedType::SEED_WINTERMELON)
	{
		if (!PlantingRequirementsMet(aUseSeedType))
		{
			mToolTip->SetWarningText("[REQUIRES_MELONPULT]");
		}
	}
	else if (aUseSeedType == SeedType::SEED_TWINSUNFLOWER)
	{
		if (!PlantingRequirementsMet(aUseSeedType))
		{
			mToolTip->SetWarningText("[REQUIRES_SUNFLOWER]");
		}
	}
	else if (aUseSeedType == SeedType::SEED_SPIKEROCK)
	{
		if (!PlantingRequirementsMet(aUseSeedType))
		{
			mToolTip->SetWarningText("[REQUIRES_SPIKEWEED]");
		}
	}
	else if (aUseSeedType == SeedType::SEED_COBCANNON)
	{
		if (!PlantingRequirementsMet(aUseSeedType))
		{
			mToolTip->SetWarningText("[REQUIRES_KERNELPULTS]");
		}
	}
	else if (aUseSeedType == SeedType::SEED_GOLD_MAGNET)
	{
		if (!PlantingRequirementsMet(aUseSeedType))
		{
			mToolTip->SetWarningText("[REQUIRES_MAGNETSHROOM]");
		}
	}
	else if (aUseSeedType == SeedType::SEED_GLOOMSHROOM)
	{
		if (!PlantingRequirementsMet(aUseSeedType))
		{
			mToolTip->SetWarningText("[REQUIRES_FUMESHROOM]");
		}
	}
	else if (aUseSeedType == SeedType::SEED_CATTAIL)
	{
		if (!PlantingRequirementsMet(aUseSeedType))
		{
			mToolTip->SetWarningText("[REQUIRES_LILY_PAD]");
		}
	}

	mToolTip->mX = (SEED_PACKET_WIDTH - mToolTip->mWidth) / 2 + mSeedBank->mX + aSeedPacket->mOffsetX + aSeedPacket->mX;
	mToolTip->mY = mSeedBank->mY + aSeedPacket->mY + 70;
	mToolTip->mVisible = true;
}

void Board::MouseDownCobcannonFire(int x, int y, int theClickCount)
{
	if (theClickCount >= 0 && y >= 80)
	{
		if (mCobCannonCursorDelayCounter > 0 && Distance2D(x, y, mCobCannonMouseX, mCobCannonMouseY) < 100.0f)
		{
			return;  // 误点检测：点击加农炮后的 30cs 内，点击的位置和准心位置之间的距离小于 100 时，将被判定为误点
		}

		if (mCursorObject->mCursorType != CursorType::CURSOR_TYPE_PLANT_FROM_DUPLICATOR)
		{
			Plant* aCobcannon = mPlants.DataArrayTryToGet(mCursorObject->mCobCannonPlantID);
			if (aCobcannon)
			{
				aCobcannon->CobCannonFire(x, y);
			}
		}
	}
	ClearCursor();
}

// GOTY @Patoke: 0x4126F0
void Board::MouseDownWithPlant(int x, int y, int theClickCount)
{
	// 右击鼠标：放下卡牌
	if (theClickCount < 0)
	{
		RefreshSeedPacketFromCursor();
		mApp->PlayFoley(FoleyType::FOLEY_DROP);
		return;
	}

	// 我是僵尸模式中，交由 Challenge 处理
	if (mApp->IsIZombieLevel())
	{
		mChallenge->IZombieMouseDownWithZombie(x, y, theClickCount);
		return;
	}

	SeedType aPlantingSeedType = GetSeedTypeInCursor();
	int aGridX = PlantingPixelToGridX(x, y, aPlantingSeedType);
	int aGridY = PlantingPixelToGridY(x, y, aPlantingSeedType);

	// 不在场地内的点击：放下卡牌
	if (aGridX < 0 || aGridX >= MAX_GRID_SIZE_X || aGridY < 0 || aGridY > MAX_GRID_SIZE_Y)
	{
		RefreshSeedPacketFromCursor();
		mApp->PlayFoley(FoleyType::FOLEY_DROP);
		return;
	}

	PlantingReason aReason = CanPlantAt(aGridX, aGridY, aPlantingSeedType);
	if (aReason != PlantingReason::PLANTING_OK)
	{
		// 根据不同的种植原因播放相应的提示字幕
		if (aReason == PlantingReason::PLANTING_ONLY_ON_GRAVES)
		{
			DisplayAdvice("[ADVICE_GRAVEBUSTERS_ON_GRAVES]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_GRAVEBUSTERS_ON_GRAVES);
		}
		else if (aPlantingSeedType == SeedType::SEED_LILYPAD)
		{
			if (aReason == PlantingReason::PLANTING_ONLY_IN_POOL)
			{
				DisplayAdvice("[ADVICE_LILYPAD_ON_WATER]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_LILYPAD_ON_WATER);
			}
		}
		else if (aPlantingSeedType == SeedType::SEED_TANGLEKELP)
		{
			if (aReason == PlantingReason::PLANTING_ONLY_IN_POOL)
			{
				DisplayAdvice("[ADVICE_TANGLEKELP_ON_WATER]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_TANGLEKELP_ON_WATER);
			}
		}
		else if (aPlantingSeedType == SeedType::SEED_SEASHROOM)
		{
			if (aReason == PlantingReason::PLANTING_ONLY_IN_POOL)
			{
				DisplayAdvice("[ADVICE_SEASHROOM_ON_WATER]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_SEASHROOM_ON_WATER);
			}
		}
		else if (aReason == PlantingReason::PLANTING_ONLY_ON_GROUND)
		{
			DisplayAdvice("[ADVICE_POTATO_MINE_ON_LILY]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_POTATOE_MINE_ON_LILY);
		}
		else if (aReason == PlantingReason::PLANTING_NOT_PASSED_LINE)
		{
			DisplayAdvice("[ADVICE_NOT_PASSED_LINE]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_NOT_PASSED_LINE);
		}
		else if (aReason == PlantingReason::PLANTING_NEEDS_UPGRADE)
		{
			switch (aPlantingSeedType)
			{
			case SeedType::SEED_GATLINGPEA:
				DisplayAdvice("[ADVICE_ONLY_ON_REPEATERS]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_ONLY_ON_REPEATERS);
				break;

			case SeedType::SEED_TWINSUNFLOWER:
				DisplayAdvice("[ADVICE_ONLY_ON_SUNFLOWER]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_ONLY_ON_SUNFLOWER);
				break;

			case SeedType::SEED_GLOOMSHROOM:
				DisplayAdvice("[ADVICE_ONLY_ON_FUMESHROOM]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_ONLY_ON_FUMESHROOM);
				break;

			case SeedType::SEED_CATTAIL:
				DisplayAdvice("[ADVICE_ONLY_ON_LILYPAD]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_ONLY_ON_LILYPAD);
				break;

			case SeedType::SEED_WINTERMELON:
				DisplayAdvice("[ADVICE_ONLY_ON_MELONPULT]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_ONLY_ON_MELONPULT);
				break;

			case SeedType::SEED_GOLD_MAGNET:
				DisplayAdvice("[ADVICE_ONLY_ON_MAGNETSHROOM]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_ONLY_ON_MAGNETSHROOM);
				break;

			case SeedType::SEED_SPIKEROCK:
				DisplayAdvice("[ADVICE_ONLY_ON_SPIKEWEED]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_ONLY_ON_SPIKEWEED);
				break;

			case SeedType::SEED_COBCANNON:
				DisplayAdvice("[ADVICE_ONLY_ON_KERNELPULT]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_ONLY_ON_KERNELPULT);
				break;
			default:
				break;
			}
		}
		else if (aReason == PlantingReason::PLANTING_NOT_ON_ART)
		{
			std::string aSeedName = Plant::GetNameString(mChallenge->GetArtChallengeSeed(aGridX, aGridY), SeedType::SEED_NONE);
			std::string aMessage = TodReplaceString("[ADVICE_WRONG_ART_TYPE]", "{SEED}", aSeedName);
			DisplayAdvice(aMessage, MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_WRONG_ART_TYPE);
		}
		else if (aReason == PlantingReason::PLANTING_NEEDS_POT)
		{
			if (mApp->IsFirstTimeAdventureMode() && mLevel == 41)
			{
				DisplayAdvice("[ADVICE_PLANT_NEED_POT1]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_NEED_POT);
			}
			else
			{
				DisplayAdvice("[ADVICE_PLANT_NEED_POT2]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_NEED_POT);
			}
		}
		else if (aReason == PlantingReason::PLANTING_NOT_ON_GRAVE)
		{
			DisplayAdvice("[ADVICE_PLANT_NOT_ON_GRAVE]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_NOT_ON_GRAVE);
		}
		else if (aReason == PlantingReason::PLANTING_NOT_ON_CRATER)
		{
			if (IsPoolSquare(aGridX, aGridY))
			{
				DisplayAdvice("[ADVICE_CANT_PLANT_THERE]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_CANT_PLANT_THERE);
			}
			else
			{
				DisplayAdvice("[ADVICE_PLANT_NOT_ON_CRATER]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_NOT_ON_CRATER);
			}
		}
		else if (aReason == PlantingReason::PLANTING_NOT_ON_WATER)
		{
			if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN && mApp->mZenGarden->mGardenType == GardenType::GARDEN_AQUARIUM)
			{
				DisplayAdvice("[ZEN_ONLY_AQUATIC_PLANTS]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_NONE);
			}
			else if (aPlantingSeedType == SeedType::SEED_POTATOMINE)
			{
				DisplayAdvice("[ADVICE_POTATO_MINE_ON_LILY]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_POTATOE_MINE_ON_LILY);
			}
			else
			{
				DisplayAdvice("[ADVICE_PLANT_NOT_ON_WATER]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANT_NOT_ON_WATER);
			}
		}
		else if (aReason == PlantingReason::PLANTING_NEEDS_GROUND)
		{
			DisplayAdvice("[ADVICE_PLANTING_NEEDS_GROUND]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANTING_NEEDS_GROUND);
		}
		else if (aReason == PlantingReason::PLANTING_NEEDS_SLEEPING)
		{
			DisplayAdvice("[ADVICE_PLANTING_NEED_SLEEPING]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_PLANTING_NEED_SLEEPING);
		}

		// 特定情况下，放下原有手持的植物
		if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_GLOVE || mApp->IsWhackAZombieLevel())
		{
			RefreshSeedPacketFromCursor();
			mApp->PlayFoley(FoleyType::FOLEY_DROP);
		}
		// 不可种植的情况至此结束，直接跳转至返回
		return;
	}
	
	/* 以下为植物类型可以种植的情况 */
	// 清除种植相关的提示字幕
	ClearAdvice(AdviceType::ADVICE_PLANTING_NEED_SLEEPING);
	ClearAdvice(AdviceType::ADVICE_CANT_PLANT_THERE);
	ClearAdvice(AdviceType::ADVICE_PLANTING_NEEDS_GROUND);
	ClearAdvice(AdviceType::ADVICE_PLANT_NOT_ON_WATER);
	ClearAdvice(AdviceType::ADVICE_PLANT_NOT_ON_CRATER);
	ClearAdvice(AdviceType::ADVICE_PLANT_NOT_ON_GRAVE);
	ClearAdvice(AdviceType::ADVICE_PLANT_NEED_POT);
	ClearAdvice(AdviceType::ADVICE_PLANT_WRONG_ART_TYPE);
	ClearAdvice(AdviceType::ADVICE_PLANT_ONLY_ON_LILYPAD);
	ClearAdvice(AdviceType::ADVICE_PLANT_ONLY_ON_MAGNETSHROOM);
	ClearAdvice(AdviceType::ADVICE_PLANT_ONLY_ON_FUMESHROOM);
	ClearAdvice(AdviceType::ADVICE_PLANT_ONLY_ON_KERNELPULT);
	ClearAdvice(AdviceType::ADVICE_PLANT_ONLY_ON_SUNFLOWER);
	ClearAdvice(AdviceType::ADVICE_PLANT_ONLY_ON_SPIKEWEED);
	ClearAdvice(AdviceType::ADVICE_PLANT_ONLY_ON_MELONPULT);
	ClearAdvice(AdviceType::ADVICE_PLANT_ONLY_ON_REPEATERS);
	ClearAdvice(AdviceType::ADVICE_PLANT_NOT_PASSED_LINE);
	ClearAdvice(AdviceType::ADVICE_PLANT_GRAVEBUSTERS_ON_GRAVES);
	ClearAdvice(AdviceType::ADVICE_PLANT_LILYPAD_ON_WATER);
	ClearAdvice(AdviceType::ADVICE_PLANT_TANGLEKELP_ON_WATER);
	ClearAdvice(AdviceType::ADVICE_PLANT_SEASHROOM_ON_WATER);
	ClearAdvice(AdviceType::ADVICE_PLANT_POTATOE_MINE_ON_LILY);
	ClearAdvice(AdviceType::ADVICE_SURVIVE_FLAGS);

	// 无免费种植、非传送带关卡的卡槽植物，判断阳光是否充足：充足则扣除阳光，不足则退出
	if (!mApp->mEasyPlantingCheat && mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_BANK && !HasConveyorBeltSeedBank())
	{
		if (!TakeSunMoney(GetCurrentPlantCost(aPlantingSeedType, SeedType::SEED_NONE)))
		{
			return;
		}
	}
	
	// 升级种植或坚果包扎术等情况时，先将原植物销毁
	bool aIsAwake = false;
	int aWakeUpCounter = 0;
	PlantsOnLawn aPlantOnLawn;
	GetPlantsOnLawn(aGridX, aGridY, &aPlantOnLawn);
	Plant* aNormalPlant = aPlantOnLawn.mNormalPlant;
	Plant* aPumpkinPlant = aPlantOnLawn.mPumpkinPlant;
	if (aNormalPlant && aNormalPlant->IsUpgradableTo(aPlantingSeedType))
	{
		if (aPlantingSeedType == SeedType::SEED_GLOOMSHROOM)
		{
			aIsAwake = !aNormalPlant->mIsAsleep;
			aWakeUpCounter = aNormalPlant->mWakeUpCounter;
		}
		aNormalPlant->Die();
	}
	if ((aPlantingSeedType == SeedType::SEED_WALLNUT || aPlantingSeedType == SeedType::SEED_TALLNUT) && aNormalPlant)
	{
		if (aNormalPlant->mSeedType == aPlantingSeedType)
		{
			aNormalPlant->Die();
		}
	}
	if (aPlantingSeedType == SeedType::SEED_PUMPKINSHELL && aPumpkinPlant)
	{
		if (aPumpkinPlant->mSeedType == SeedType::SEED_PUMPKINSHELL)
		{
			aPumpkinPlant->Die();
		}
	}
	if (aPlantingSeedType == SeedType::SEED_COBCANNON)
	{
		Plant* aRightPlant = GetTopPlantAt(aGridX + 1, aGridY, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION);
		if (aRightPlant)
		{
			aRightPlant->Die();
		}
	}
	if (aPlantingSeedType == SeedType::SEED_CATTAIL)
	{
		if (aPlantOnLawn.mUnderPlant)
		{
			aPlantOnLawn.mUnderPlant->Die();
		}
		if (aNormalPlant)
		{
			aNormalPlant->Die();
		}
	}

	if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_GLOVE)
	{
		mApp->mZenGarden->MovePlant(mPlants.DataArrayTryToGet(mCursorObject->mGlovePlantID), aGridX, aGridY);
	}
	else if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_WHEEL_BARROW)
	{
		mApp->mZenGarden->MouseDownWithFullWheelBarrow(x, y);
	}
	else if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_USABLE_COIN)
	{
		AddPlant(aGridX, aGridY, mCursorObject->mType, mCursorObject->mImitaterType);
		Coin* aCoin = mCoins.DataArrayTryToGet(mCursorObject->mCoinID);
		mCursorObject->mCoinID = CoinID::COINID_NULL;
		aCoin->Die();
	}
	else if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_BANK)
	{
		Plant* aPlant = AddPlant(aGridX, aGridY, mCursorObject->mType, mCursorObject->mImitaterType);
		if (aIsAwake)
		{
			aPlant->SetSleeping(false);
		}
		else
		{
			aPlant->mWakeUpCounter = aWakeUpCounter;
		}

		mSeedBank->mSeedPackets[mCursorObject->mSeedBankIndex].WasPlanted();
	}
	else
	{
		TOD_ASSERT(false);
	}
	
	// 柱子关卡中，一列种植
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_COLUMN)
	{
		for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++)
		{
			if (aRow == aGridY || CanPlantAt(aGridX, aRow, aPlantingSeedType) != PlantingReason::PLANTING_OK)
				continue;

			if (aPlantingSeedType == SeedType::SEED_WALLNUT || aPlantingSeedType == SeedType::SEED_TALLNUT)
			{
				aNormalPlant = GetTopPlantAt(aGridX, aRow, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION);
				if (aNormalPlant && aNormalPlant->mSeedType == aPlantingSeedType)
				{
					aNormalPlant->Die();
				}
			}
			if (aPlantingSeedType == SeedType::SEED_PUMPKINSHELL)
			{
				aPumpkinPlant = GetTopPlantAt(aGridX, aRow, PlantPriority::TOPPLANT_ONLY_PUMPKIN);
				if (aPumpkinPlant && aPumpkinPlant->mSeedType == SeedType::SEED_PUMPKINSHELL)
				{
					aPumpkinPlant->Die();
				}
			}
			AddPlant(aGridX, aRow, mCursorObject->mType, mCursorObject->mImitaterType);
		}
	}

	// 设置教程状态相关
	if (mTutorialState == TutorialState::TUTORIAL_LEVEL_1_PLANT_PEASHOOTER)
	{
		SetTutorialState(mPlants.mSize >= 2 ? TutorialState::TUTORIAL_LEVEL_1_COMPLETED : TutorialState::TUTORIAL_LEVEL_1_REFRESH_PEASHOOTER);
	}
	else if (mTutorialState == TutorialState::TUTORIAL_LEVEL_2_PLANT_SUNFLOWER)
	{
		int aSunFlowersCount = CountSunFlowers();
		if (aPlantingSeedType == SeedType::SEED_SUNFLOWER && aSunFlowersCount == 2)
		{
			DisplayAdvice("[ADVICE_MORE_SUNFLOWERS]", MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL2, AdviceType::ADVICE_NONE);
			if (!mSeedBank->mSeedPackets[1].CanPickUp())
			{
				SetTutorialState(TutorialState::TUTORIAL_LEVEL_2_REFRESH_SUNFLOWER);
			}
			else
			{
				SetTutorialState(TutorialState::TUTORIAL_LEVEL_2_PICK_UP_SUNFLOWER);
			}
		}
		else if (aSunFlowersCount >= 3)
		{
			SetTutorialState(TutorialState::TUTORIAL_LEVEL_2_COMPLETED);
		}
		else if (!mSeedBank->mSeedPackets[1].CanPickUp())
		{
			SetTutorialState(TutorialState::TUTORIAL_LEVEL_2_REFRESH_SUNFLOWER);
		}
		else
		{
			SetTutorialState(TutorialState::TUTORIAL_LEVEL_2_PICK_UP_SUNFLOWER);
		}
	}
	else if (mTutorialState == TutorialState::TUTORIAL_MORESUN_PLANT_SUNFLOWER)
	{
		if (CountSunFlowers() >= 3)
		{
			SetTutorialState(TutorialState::TUTORIAL_MORESUN_COMPLETED);
			DisplayAdvice("[ADVICE_PLANT_SUNFLOWER5]", MessageStyle::MESSAGE_STYLE_TUTORIAL_LATER, AdviceType::ADVICE_PLANT_SUNFLOWER5);
			mTutorialTimer = -1;
		}
		else if (!mSeedBank->mSeedPackets[1].CanPickUp())
		{
			SetTutorialState(TutorialState::TUTORIAL_MORESUN_REFRESH_SUNFLOWER);
		}
		else
		{
			SetTutorialState(TutorialState::TUTORIAL_MORESUN_PICK_UP_SUNFLOWER);
		}
	}

	// 保龄球关卡，播放保龄球滚动的音效
	if (mApp->IsWallnutBowlingLevel())
	{
		mApp->PlaySample(Sexy::SOUND_BOWLING);
	}

	// 重置鼠标
	ClearCursor();
}

Plant* Board::ToolHitTestHelper(HitResult* theHitResult)
{
	theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_PLANT;
	Plant* aPlant = (Plant*)theHitResult->mObject;
	return (aPlant->mSeedType != SeedType::SEED_GRAVEBUSTER || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN) ? aPlant : nullptr;
}

Plant* Board::ToolHitTest(int theX, int theY)
{
	HitResult aHitResult;
	MouseHitTest(theX, theY, &aHitResult);
	if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_PLANT)
	{
		return ToolHitTestHelper(&aHitResult);
	}
	return nullptr;
}

void Board::TutorialArrowShow(int theX, int theY)
{
	TutorialArrowRemove();
	TodParticleSystem* aParticle = mApp->AddTodParticle(theX, theY, MakeRenderOrder(RenderLayer::RENDER_LAYER_TOP, 0, 0), ParticleEffect::PARTICLE_SEED_PACKET_PICK);
	mTutorialParticleID = mApp->ParticleGetID(aParticle);
}

void Board::TutorialArrowRemove()
{
	mApp->RemoveParticle(mTutorialParticleID);
	mTutorialParticleID = ParticleSystemID::PARTICLESYSTEMID_NULL;
}

void Board::MouseDownWithTool(int x, int y, int theClickCount, CursorType theCursorType)
{
	if (theClickCount < 0)
	{
		ClearCursor();
		mApp->PlayFoley(FoleyType::FOLEY_DROP);
		return;
	}

	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		mApp->mZenGarden->MouseDownWithTool(x, y, theCursorType);
		return;
	}
	if (mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM)
	{
		mChallenge->TreeOfWisdomTool(x, y);
		return;
	}

	Plant* aPlant = ToolHitTest(x, y);
	if (aPlant == nullptr)
	{
		mApp->PlayFoley(FoleyType::FOLEY_DROP);
	}
	else if (theCursorType == CursorType::CURSOR_TYPE_SHOVEL)
	{
		mApp->PlayFoley(FoleyType::FOLEY_USE_SHOVEL);
		mPlantsShoveled++;
		aPlant->Die();

		if (aPlant->mSeedType == SeedType::SEED_CATTAIL && GetTopPlantAt(aPlant->mPlantCol, aPlant->mRow, PlantPriority::TOPPLANT_ONLY_PUMPKIN))
		{
			NewPlant(aPlant->mPlantCol, aPlant->mRow, SeedType::SEED_LILYPAD, SeedType::SEED_NONE);
		}
		if (mTutorialState == TutorialState::TUTORIAL_SHOVEL_DIG || mTutorialState == TutorialState::TUTORIAL_SHOVEL_KEEP_DIGGING)
		{
			SetTutorialState(CountPlantByType(SeedType::SEED_PEASHOOTER) == 0 ? TutorialState::TUTORIAL_SHOVEL_COMPLETED : TutorialState::TUTORIAL_SHOVEL_KEEP_DIGGING);
		}
	}

	ClearCursor();
}

Plant* Board::SpecialPlantHitTest(int x, int y)
{
	Plant* aPlant = nullptr;
	while (IteratePlants(aPlant))
	{
		if (aPlant->mSeedType == SeedType::SEED_PUMPKINSHELL)
		{
			float aMinDist = GetTopPlantAt(aPlant->mPlantCol, aPlant->mRow, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION) ? 25 : 0;
			float aDistance = Distance2D(x, y, aPlant->mX + 40, aPlant->mY + 40);
			if (aDistance >= aMinDist && aDistance <= 50 && y > aPlant->mY + 25)
			{
				return aPlant;
			}
		}
		else if (Plant::IsFlying(aPlant->mSeedType))
		{
			if (Distance2D(x, y, aPlant->mX + 40, aPlant->mY) < 15)
			{
				return aPlant;
			}
		}
	}
	return nullptr;
}

bool Board::MouseHitTestPlant(int x, int y, HitResult* theHitResult)
{
	if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_COBCANNON_TARGET || mCursorObject->mCursorType == CursorType::CURSOR_TYPE_HAMMER)
		return false;

	Plant* aPlant;
	aPlant = SpecialPlantHitTest(x, y);
	if (aPlant)
	{
		theHitResult->mObject = aPlant;
		theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_PLANT;
		return true;
	}

	int aGridX = PixelToGridX(x, y);
	int aGridY = PixelToGridY(x, y);
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		aPlant = GetTopPlantAt(aGridX, aGridY, PlantPriority::TOPPLANT_ZEN_TOOL_ORDER);
		if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_WATERING_CAN && (!aPlant || !mApp->mZenGarden->PlantCanBeWatered(aPlant)))
		{
			Plant* aTopPlant = GetTopPlantAt(PixelToGridX(x - 30, y - 20), PixelToGridY(x - 30, y - 20), PlantPriority::TOPPLANT_ZEN_TOOL_ORDER);
			if (aTopPlant && mApp->mZenGarden->PlantCanBeWatered(aTopPlant))
			{
				aPlant = aTopPlant;
			}
		}
	}
	else
	{
		aPlant = GetTopPlantAt(aGridX, aGridY, PlantPriority::TOPPLANT_DIGGING_ORDER);
		if (aPlant && (aPlant->mSeedType == SeedType::SEED_LILYPAD || aPlant->mSeedType == SeedType::SEED_FLOWERPOT))
		{
			if (GetTopPlantAt(aGridX, aGridY, PlantPriority::TOPPLANT_ONLY_PUMPKIN))
			{
				return false;
			}
		}
	}

	// 植物不存在，或者手持巧克力但植物不需要巧克力时，返回“否”
	if (aPlant == nullptr)
	{
		return false;
	}
	if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_CHOCOLATE && !mApp->mZenGarden->PlantCanHaveChocolate(aPlant))
	{
		theHitResult->mObject = nullptr;
		theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_NONE;
		return false;
	}

	theHitResult->mObject = aPlant;
	theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_PLANT;
	return true;
}

bool Board::MouseHitTest(int x, int y, HitResult* theHitResult)
{
	if (mBoardFadeOutCounter >= 0 || IsScaryPotterDaveTalking())
	{
		theHitResult->mObject = nullptr;
		theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_NONE;
		return false;
	}

	if (mMenuButton->IsMouseOver() && CanInteractWithBoardButtons())
	{
		theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_MENU_BUTTON;
		return true;
	}
	else if (mStoreButton && mStoreButton->IsMouseOver() && CanInteractWithBoardButtons())
	{
		theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_STORE_BUTTON;
		return true;
	}

	Rect aShovelButtonRect = GetShovelButtonRect();
	if (mSeedBank->MouseHitTest(x, y, theHitResult))
	{
		if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_NORMAL || 
			mCursorObject->mCursorType == CursorType::CURSOR_TYPE_COBCANNON_TARGET || 
			mCursorObject->mCursorType == CursorType::CURSOR_TYPE_HAMMER)
			return true;
	}
	if (mShowShovel && aShovelButtonRect.Contains(x, y) && CanInteractWithBoardButtons())
	{
		theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_SHOVEL;
		return true;
	}

	if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_NORMAL || mCursorObject->mCursorType == CursorType::CURSOR_TYPE_HAMMER)
	{
		Coin* aCoin = nullptr;
		Coin* aTopCoin = nullptr;
		while (IterateCoins(aCoin))
		{
			HitResult aHitResultCoin;
			if (aCoin->MouseHitTest(x, y, &aHitResultCoin))
			{
				aCoin = (Coin*)aHitResultCoin.mObject;
				if (aTopCoin == nullptr || aCoin->mRenderOrder >= aTopCoin->mRenderOrder)
				{
					theHitResult->mObjectType = aHitResultCoin.mObjectType;
					theHitResult->mObject = aCoin;
					aTopCoin = aCoin;
				}
			}
		}
		if (aTopCoin)
		{
			return true;
		}
	}

	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		bool canClick = false;
		if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_CHOCOLATE && !mApp->mZenGarden->IsStinkyHighOnChocolate())
		{
			canClick = true;
		}
		else if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_NORMAL && mApp->mZenGarden->IsStinkySleeping())
		{
			canClick = true;
		}

		GridItem* aStinky = mApp->mZenGarden->GetStinky();
		if (canClick && aStinky)
		{
			Rect aStinkyRect(aStinky->mPosX - 6, aStinky->mPosY - 10, 84, 90);
			if (aStinkyRect.Contains(x, y))
			{
				theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_STINKY;
				return true;
			}
		}
	}
	if (mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM)
	{
		if (mCursorObject->mCursorType == CursorType::CURSOR_TYPE_TREE_FOOD && mChallenge->TreeOfWisdomHitTest(x, y, theHitResult))
		{
			return true;
		}
	}
	if ((mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN || mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM) && CanInteractWithBoardButtons())
	{
		for (int i = static_cast<int>(GameObjectType::OBJECT_TYPE_WATERING_CAN); i <= static_cast<int>(GameObjectType::OBJECT_TYPE_NEXT_GARDEN); i++)
		{
			GameObjectType aTool = static_cast<GameObjectType>(i);
			if (CanUseGameObject(aTool) && (aTool != GameObjectType::OBJECT_TYPE_TREE_FOOD || mChallenge->TreeOfWisdomCanFeed()))
			{
				Rect aButtonRect = GetShovelButtonRect();
				if (aTool == GameObjectType::OBJECT_TYPE_NEXT_GARDEN)
				{
					aButtonRect.mX = 564;
				}
				else
				{
					GetZenButtonRect(aTool, aButtonRect);
				}

				if (aButtonRect.Contains(x, y))
				{
					theHitResult->mObjectType = (GameObjectType)aTool;
					return true;
				}
			}
		}
	}

	if (MouseHitTestPlant(x, y, theHitResult))
		return true;

	if (mApp->IsScaryPotterLevel() && 
		mCursorObject->mCursorType == CursorType::CURSOR_TYPE_NORMAL &&
		mChallenge->mChallengeState != ChallengeState::STATECHALLENGE_SCARY_POTTER_MALLETING && 
		mApp->mGameScene == GameScenes::SCENE_PLAYING &&
		mApp->GetDialog(Dialogs::DIALOG_GAME_OVER) == nullptr && 
		mApp->GetDialog(Dialogs::DIALOG_CONTINUE) == nullptr)
	{
		GridItem* aScaryPot = GetGridItemAt(GridItemType::GRIDITEM_SCARY_POT, PixelToGridX(x, y), PixelToGridY(x, y));
		if (aScaryPot)
		{
			theHitResult->mObject = aScaryPot;
			theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_SCARY_POT;
			return true;
		}
	}

	if (mApp->IsSlotMachineLevel())
	{
		Rect aSlotMachineHandleRect = mChallenge->SlotMachineGetHandleRect();
		if (aSlotMachineHandleRect.Contains(x, y) && mChallenge->mChallengeState == ChallengeState::STATECHALLENGE_NORMAL && !HasLevelAwardDropped())
		{
			theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_SLOT_MACHINE_HANDLE;
			return true;
		}
	}

	theHitResult->mObject = nullptr;
	theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_NONE;
	return false;
}

void Board::PickUpTool(GameObjectType theObjectType)
{
	if (mPaused || (mApp->mGameScene != GameScenes::SCENE_PLAYING && !mCutScene->IsInShovelTutorial()))
		return;

	switch (theObjectType)
	{
	case GameObjectType::OBJECT_TYPE_SHOVEL:
		if (mTutorialState == TutorialState::TUTORIAL_SHOVEL_PICKUP)
		{
			SetTutorialState(TutorialState::TUTORIAL_SHOVEL_DIG);
		}
		mCursorObject->mCursorType = CursorType::CURSOR_TYPE_SHOVEL;
		mApp->PlayFoley(FoleyType::FOLEY_SHOVEL);
		break;

	case GameObjectType::OBJECT_TYPE_WATERING_CAN:
		if (mTutorialState == TutorialState::TUTORIAL_ZEN_GARDEN_PICKUP_WATER)
		{
			mTutorialState = TutorialState::TUTORIAL_ZEN_GARDEN_WATER_PLANT;
			DisplayAdvice("[ADVICE_ZEN_GARDEN_WATER_PLANT]", MessageStyle::MESSAGE_STYLE_ZEN_GARDEN_LONG, AdviceType::ADVICE_NONE);
			TutorialArrowRemove();
		}
		mCursorObject->mCursorType = CursorType::CURSOR_TYPE_WATERING_CAN;
		mApp->PlayFoley(FoleyType::FOLEY_DROP);
		break;

	case GameObjectType::OBJECT_TYPE_FERTILIZER:
		if (mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_FERTILIZER] > PURCHASE_COUNT_OFFSET)
		{
			mCursorObject->mCursorType = CursorType::CURSOR_TYPE_FERTILIZER;
			mApp->PlayFoley(FoleyType::FOLEY_DROP);
		}
		else
		{
			mApp->PlaySample(Sexy::SOUND_BUZZER);
		}
		break;

	case GameObjectType::OBJECT_TYPE_BUG_SPRAY:
		if (mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_BUG_SPRAY] > PURCHASE_COUNT_OFFSET)
		{
			mCursorObject->mCursorType = CursorType::CURSOR_TYPE_BUG_SPRAY;
			mApp->PlayFoley(FoleyType::FOLEY_DROP);
		}
		else
		{
			mApp->PlaySample(Sexy::SOUND_BUZZER);
		}
		break;

	case GameObjectType::OBJECT_TYPE_PHONOGRAPH:
		mCursorObject->mCursorType = CursorType::CURSOR_TYPE_PHONOGRAPH;
		mApp->PlayFoley(FoleyType::FOLEY_DROP);
		break;

	case GameObjectType::OBJECT_TYPE_CHOCOLATE:
		if (mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_CHOCOLATE] > PURCHASE_COUNT_OFFSET)
		{
			mCursorObject->mCursorType = CursorType::CURSOR_TYPE_CHOCOLATE;
			mApp->PlayFoley(FoleyType::FOLEY_DROP);
		}
		else
		{
			mApp->PlaySample(Sexy::SOUND_BUZZER);
		}
		break;

	case GameObjectType::OBJECT_TYPE_GLOVE:
		mCursorObject->mCursorType = CursorType::CURSOR_TYPE_GLOVE;
		mApp->PlayFoley(FoleyType::FOLEY_DROP);
		break;

	case GameObjectType::OBJECT_TYPE_MONEY_SIGN:
		mCursorObject->mCursorType = CursorType::CURSOR_TYPE_MONEY_SIGN;
		mApp->PlayFoley(FoleyType::FOLEY_DROP);
		break;

	case GameObjectType::OBJECT_TYPE_WHEELBARROW:
		mCursorObject->mCursorType = CursorType::CURSOR_TYPE_WHEEELBARROW;
		mApp->PlayFoley(FoleyType::FOLEY_DROP);
		break;

	case GameObjectType::OBJECT_TYPE_TREE_FOOD:
		if (mChallenge->TreeOfWisdomCanFeed())
		{
			if (mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_TREE_FOOD] > PURCHASE_COUNT_OFFSET)
			{
				mCursorObject->mCursorType = CursorType::CURSOR_TYPE_TREE_FOOD;
				mApp->PlayFoley(FoleyType::FOLEY_DROP);
			}
			else
			{
				mApp->PlaySample(Sexy::SOUND_BUZZER);
			}
		}
		break;

	default:
		TOD_ASSERT(false);
		break;
	}

	mCursorObject->mType = SeedType::SEED_NONE;
}

void Board::MouseDown(int x, int y, int theClickCount)
{
	UpdateMousePosition();
	Widget::MouseDown(x, y, theClickCount);
	mIgnoreMouseUp = !CanInteractWithBoardButtons();
	if (mTimeStopCounter > 0)
		return;

	HitResult aHitResult;
	MouseHitTest(x, y, &aHitResult);
	if (mChallenge->MouseDown(x, y, theClickCount, &aHitResult))
		return;

	if (mMenuButton->IsMouseOver() && CanInteractWithBoardButtons() && theClickCount > 0)
	{
		mApp->PlaySample(Sexy::SOUND_GRAVEBUTTON);
	}
	else if (mStoreButton && mStoreButton->IsMouseOver() && CanInteractWithBoardButtons() && theClickCount > 0)
	{
		if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN || mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM)
		{
			mApp->PlaySample(Sexy::SOUND_TAP);
		}
		else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND || mApp->mGameMode == GameMode::GAMEMODE_UPSELL)
		{
			mApp->PlaySample(Sexy::SOUND_GRAVEBUTTON);
		}
	}

	if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO && mApp->mSeedChooserScreen)
	{
		mApp->mSeedChooserScreen->CancelLawnView();
	}
	if (mApp->mGameScene == GameScenes::SCENE_ZOMBIES_WON)
	{
		mCutScene->ZombieWonClick();
		return;
	}
	if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO)
	{
		mCutScene->MouseDown(x, y);
	}
	
	if (mApp->mTodCheatKeys && !mApp->IsScaryPotterLevel() && mNextSurvivalStageCounter > 0)
	{
		mNextSurvivalStageCounter = 2;
		for (int i = 0; i < MAX_GRID_SIZE_Y; i++)
		{
			mIceTimer[i] = std::min(mIceTimer[i], 2);
		}
	}

	CursorType aCursor = mCursorObject->mCursorType;
	if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_NONE)
	{
		if (aCursor == CURSOR_TYPE_COBCANNON_TARGET)
		{
			MouseDownCobcannonFire(x, y, theClickCount);
			UpdateCursor();
			return;
		}
	}
	else if (aHitResult.mObjectType == GameObjectType::OBJECT_TYPE_COIN && theClickCount >= 0)
	{
		Coin* aCoin = (Coin*)aHitResult.mObject;
		if (aCoin->mBoard)
		{
			aCoin->MouseDown(x, y, theClickCount);
		}
		UpdateCursor();
		return;
	}

	if (aCursor == CursorType::CURSOR_TYPE_SHOVEL ||
		aCursor == CursorType::CURSOR_TYPE_WATERING_CAN ||
		aCursor == CursorType::CURSOR_TYPE_FERTILIZER ||
		aCursor == CursorType::CURSOR_TYPE_BUG_SPRAY ||
		aCursor == CursorType::CURSOR_TYPE_PHONOGRAPH ||
		aCursor == CursorType::CURSOR_TYPE_CHOCOLATE ||
		aCursor == CursorType::CURSOR_TYPE_GLOVE ||
		aCursor == CursorType::CURSOR_TYPE_MONEY_SIGN ||
		aCursor == CursorType::CURSOR_TYPE_WHEEELBARROW ||
		aCursor == CursorType::CURSOR_TYPE_TREE_FOOD)
	{
		MouseDownWithTool(x, y, theClickCount, aCursor);
	}
	else if (IsPlantInCursor())
	{
		MouseDownWithPlant(x, y, theClickCount);
	}
	else
	{
		switch (aHitResult.mObjectType)
		{
		case GameObjectType::OBJECT_TYPE_SEEDPACKET:
			if (!mPaused)
			{
				((SeedPacket*)aHitResult.mObject)->MouseDown(x, y, theClickCount);
			}
			break;
		case GameObjectType::OBJECT_TYPE_NEXT_GARDEN:
			if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
			{
				mApp->mZenGarden->GotoNextGarden();
			}
			else if (mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM)
			{
				mChallenge->TreeOfWisdomNextGarden();
			}
			mApp->PlaySample(Sexy::SOUND_TAP);
			break;
		case GameObjectType::OBJECT_TYPE_SHOVEL:
		case GameObjectType::OBJECT_TYPE_WATERING_CAN:
		case GameObjectType::OBJECT_TYPE_FERTILIZER:
		case GameObjectType::OBJECT_TYPE_BUG_SPRAY:
		case GameObjectType::OBJECT_TYPE_PHONOGRAPH:
		case GameObjectType::OBJECT_TYPE_CHOCOLATE:
		case GameObjectType::OBJECT_TYPE_GLOVE:
		case GameObjectType::OBJECT_TYPE_MONEY_SIGN:
		case GameObjectType::OBJECT_TYPE_WHEELBARROW:
		case GameObjectType::OBJECT_TYPE_TREE_FOOD:
			PickUpTool(aHitResult.mObjectType);
			break;
		case GameObjectType::OBJECT_TYPE_PLANT:
			((Plant*)aHitResult.mObject)->MouseDown(x, y, theClickCount);
			break;
		default:
			break;
		}
	}

	UpdateCursor();
}

void Board::ClearCursor()
{
	if (mAdvice->mDuration > 0)
	{
		if (mHelpIndex == AdviceType::ADVICE_PLANT_GRAVEBUSTERS_ON_GRAVES ||
			mHelpIndex == AdviceType::ADVICE_PLANT_LILYPAD_ON_WATER ||
			mHelpIndex == AdviceType::ADVICE_PLANT_TANGLEKELP_ON_WATER ||
			mHelpIndex == AdviceType::ADVICE_PLANT_SEASHROOM_ON_WATER ||
			mHelpIndex == AdviceType::ADVICE_PLANT_POTATOE_MINE_ON_LILY ||
			mHelpIndex == AdviceType::ADVICE_PLANT_WRONG_ART_TYPE ||
			mHelpIndex == AdviceType::ADVICE_PLANT_NEED_POT ||
			mHelpIndex == AdviceType::ADVICE_PLANT_NOT_PASSED_LINE ||
			mHelpIndex == AdviceType::ADVICE_PLANT_ONLY_ON_REPEATERS ||
			mHelpIndex == AdviceType::ADVICE_PLANT_ONLY_ON_MELONPULT ||
			mHelpIndex == AdviceType::ADVICE_PLANT_ONLY_ON_SUNFLOWER ||
			mHelpIndex == AdviceType::ADVICE_PLANT_ONLY_ON_SPIKEWEED ||
			mHelpIndex == AdviceType::ADVICE_PLANT_ONLY_ON_KERNELPULT)
		{
			ClearAdvice(mHelpIndex);
		}
	}

	mCursorObject->mType = SeedType::SEED_NONE;
	mCursorObject->mCursorType = CursorType::CURSOR_TYPE_NORMAL;
	mCursorObject->mSeedBankIndex = -1;
	mCursorObject->mCoinID = CoinID::COINID_NULL;
	mCursorObject->mDuplicatorPlantID = PlantID::PLANTID_NULL;
	mCursorObject->mCobCannonPlantID = PlantID::PLANTID_NULL;
	mCursorObject->mGlovePlantID = PlantID::PLANTID_NULL;
	mApp->SetCursor(CURSOR_POINTER);
	mChallenge->ClearCursor();

	if (mTutorialState == TutorialState::TUTORIAL_LEVEL_1_PLANT_PEASHOOTER)
	{
		SetTutorialState(TutorialState::TUTORIAL_LEVEL_1_PICK_UP_PEASHOOTER);
	}
	else if (mTutorialState == TutorialState::TUTORIAL_LEVEL_2_PLANT_SUNFLOWER || mTutorialState == TutorialState::TUTORIAL_LEVEL_2_REFRESH_SUNFLOWER)
	{
		if (!mSeedBank->mSeedPackets[1].CanPickUp())
		{
			SetTutorialState(TutorialState::TUTORIAL_LEVEL_2_REFRESH_SUNFLOWER);
		}
		else
		{
			SetTutorialState(TutorialState::TUTORIAL_LEVEL_2_PICK_UP_SUNFLOWER);
		}
	}
	else if (mTutorialState == TutorialState::TUTORIAL_MORESUN_PLANT_SUNFLOWER || mTutorialState == TutorialState::TUTORIAL_MORESUN_REFRESH_SUNFLOWER)
	{
		if (!mSeedBank->mSeedPackets[1].CanPickUp())
		{
			SetTutorialState(TutorialState::TUTORIAL_MORESUN_REFRESH_SUNFLOWER);
		}
		else
		{
			SetTutorialState(TutorialState::TUTORIAL_MORESUN_PICK_UP_SUNFLOWER);
		}
	}
	else if (mTutorialState == TutorialState::TUTORIAL_SHOVEL_DIG)
	{
		SetTutorialState(TutorialState::TUTORIAL_SHOVEL_PICKUP);
	}
}

bool Board::CanInteractWithBoardButtons()
{
	// during the process of obtaining a reward, should not interaction with the menu.
	if (mBoardFadeOutCounter >= 0)
		return false;

	if (mPaused || mApp->GetDialogCount() > 0)
		return false;

	if (mCursorObject->mCursorType != CursorType::CURSOR_TYPE_NORMAL && 
		mCursorObject->mCursorType != CursorType::CURSOR_TYPE_HAMMER &&
		mCursorObject->mCursorType != CursorType::CURSOR_TYPE_COBCANNON_TARGET)
		return false;

	if (mChallenge->mChallengeState == ChallengeState::STATECHALLENGE_ZEN_FADING)
		return false;

	return mApp->mGameMode == GameMode::GAMEMODE_UPSELL || mApp->mCrazyDaveState == CrazyDaveState::CRAZY_DAVE_OFF;
}

void Board::MouseUp(int x, int y, int theClickCount)
{
	Widget::MouseUp(x, y, theClickCount);
	if (mIgnoreMouseUp)
	{
		mIgnoreMouseUp = false;
		return;
	}

	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED && mChallenge->MouseUp(x, y) && theClickCount > 0)
		return;

	if (CanInteractWithBoardButtons() && theClickCount > 0)
	{
		if (mMenuButton->IsMouseOver() && !mApp->GetDialog(Dialogs::DIALOG_GAME_OVER) && !mApp->GetDialog(Dialogs::DIALOG_LEVEL_COMPLETE))
		{
			mMenuButton->mIsOver = false;
			mMenuButton->mIsDown = false;
			UpdateCursor();
			ClearCursor();
			if (mTutorialState == TutorialState::TUTORIAL_ZEN_GARDEN_COMPLETED)
			{
				mApp->FinishZenGardenToturial();
			}
			else if (mApp->mGameMode != GAMEMODE_CHALLENGE_ZEN_GARDEN && mApp->mGameMode != GAMEMODE_TREE_OF_WISDOM && mApp->mGameMode != GAMEMODE_UPSELL)
			{
				mApp->PlaySample(Sexy::SOUND_PAUSE);
				mApp->DoNewOptions(false);
			}
			else
			{
				mApp->mBoardResult = BoardResult::BOARDRESULT_QUIT;
				mApp->DoBackToMain();
			}
		}
		else if(mStoreButton && mStoreButton->IsMouseOver())
		{
			if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
			{
				ClearAdviceImmediately();
				mApp->mZenGarden->OpenStore();
			}
			else if (mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM)
			{
				mChallenge->TreeOfWisdomOpenStore();
			}
			else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND)
			{
				mChallenge->mChallengeState = ChallengeState::STATECHALLENGE_LAST_STAND_ONSLAUGHT;
				mStoreButton->mBtnNoDraw = true;
				mStoreButton->mDisabled = true;
				mZombieCountDown = 10;
				mZombieCountDownStart = 10;
			}
			else if (mApp->mGameMode == GameMode::GAMEMODE_UPSELL)
			{
				mApp->DoBackToMain();
			}
		}
	}
}

void Board::ShowCoinBank(int theDuration)
{
	mCoinBankFadeCount = theDuration;
}

void Board::Pause(bool thePause)
{
	if (mPaused == thePause)
		return;

	mPaused = thePause;
	if (thePause && mApp->mPlayerInfo->mCoins > 0)
	{
		ShowCoinBank();
	}

	if (!thePause || mApp->mGameScene != GameScenes::SCENE_LEVEL_INTRO)
	{
		mApp->mSoundSystem->GamePause(thePause);
		mApp->mMusic->GameMusicPause(thePause);
	}

	if (thePause && NeedSaveGame())
	{
		TryToSaveGame();
	}
}

int Board::GetGraveStonesCount()
{
	int aCount = 0;

	GridItem* aGridItem = nullptr;
	while (IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType == GridItemType::GRIDITEM_GRAVESTONE)
		{
			aCount++;
		}
	}

	return aCount;
}

void Board::PickSpecialGraveStone()
{
	GridItem* aGridItem = nullptr;
	GridItem* aPicks[MAX_GRAVE_STONES];
	int aPickCount = 0;
	while (IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType == GridItemType::GRIDITEM_GRAVESTONE)
		{
			TOD_ASSERT(aPickCount < MAX_GRAVE_STONES);
			aPicks[aPickCount] = aGridItem;
			aPickCount++;
		}
	}

	if (aPickCount > 0)
	{
		TodPickFromArray(aPicks, aPickCount)->mGridItemState = GridItemState::GRIDITEM_STATE_GRAVESTONE_SPECIAL;
	}
}

void Board::SpawnZombiesFromPool()
{
	if (mIceTrapCounter > 0)
		return;
	
	int aCount, aZombiePoints;
	if (mLevel == 21 || mLevel == 22 || mLevel == 31 || mLevel == 32)
	{
		aCount = 2;
		aZombiePoints = 3;
	}
	else if (mLevel == 23 || mLevel == 24 || mLevel == 25 || mLevel == 33 || mLevel == 34 || mLevel == 35)
	{
		aCount = 3;
		aZombiePoints = 5;
	}
	else
	{
		aCount = 3;
		aZombiePoints = 7;
	}
	
	int aGridArrayCount = 0;
	TodWeightedGridArray aGridArray[MAX_POOL_GRID_SIZE];
	for (int aGridX = 5; aGridX < MAX_GRID_SIZE_X; aGridX++)
	{
		for (int aGridY = 2; aGridY <= 3; aGridY++)
		{
			aGridArray[aGridArrayCount].mX = aGridX;
			aGridArray[aGridArrayCount].mY = aGridY;
			aGridArray[aGridArrayCount].mWeight = 10000;
			aGridArrayCount++;
			TOD_ASSERT(aGridArrayCount <= MAX_POOL_GRID_SIZE);
		}
	}

	aGridArrayCount = std::max(aGridArrayCount, 0);
	for (int i = 0; i < aCount; i++)
	{
		TodWeightedGridArray* aGrid = TodPickFromWeightedGridArray(aGridArray, aGridArrayCount);
		aGrid->mWeight = 0;

		ZombieType aZombieType = PickGraveRisingZombieType();
		Zombie* aZombie = AddZombieInRow(aZombieType, aGrid->mY, mCurrentWave);
		if (aZombie == nullptr)
		{
			return;
		}

		aZombie->RiseFromGrave(aGrid->mX, aGrid->mY);
		aZombiePoints -= GetZombieDefinition(aZombieType).mZombieValue;
		aZombiePoints = std::max(aZombiePoints, 1);
	}
}

void Board::SetupBungeeDrop(BungeeDropGrid* theBungeeDropGrid)
{
	theBungeeDropGrid->mGridArrayCount = 0;
	for (int aGridX = 4; aGridX < MAX_GRID_SIZE_X; aGridX++)
	{
		for (int aGridY = 0; aGridY <= 4; aGridY++)
		{
			int aCount = theBungeeDropGrid->mGridArrayCount;
			theBungeeDropGrid->mGridArray[aCount].mX = aGridX;
			theBungeeDropGrid->mGridArray[aCount].mY = aGridY;
			theBungeeDropGrid->mGridArray[aCount].mWeight = 10000;
			theBungeeDropGrid->mGridArrayCount++;
			TOD_ASSERT(static_cast<size_t>(theBungeeDropGrid->mGridArrayCount) <= LENGTH(theBungeeDropGrid->mGridArray));
		}
	}
}

void Board::BungeeDropZombie(BungeeDropGrid* theBungeeDropGrid, ZombieType theZombieType)
{
	TodWeightedGridArray* aGrid = TodPickFromWeightedGridArray(theBungeeDropGrid->mGridArray, theBungeeDropGrid->mGridArrayCount);
	aGrid->mWeight = 1;

	Zombie* aBungeeZombie = AddZombie(ZombieType::ZOMBIE_BUNGEE, mCurrentWave);
	Zombie* aZombie = AddZombie(theZombieType, mCurrentWave);
	TOD_ASSERT(aBungeeZombie && aZombie);

	aBungeeZombie->BungeeDropZombie(aZombie, aGrid->mX, aGrid->mY);
}

void Board::SpawnZombiesFromSky()
{
	if (mIceTrapCounter > 0)
		return;

	int aCount, aZombiePoints;
	if (mLevel == 41 || mLevel == 42)
	{
		aCount = 2;
		aZombiePoints = 3;
	}
	else if (mLevel == 43 || mLevel == 44 || mLevel == 45)
	{
		aCount = 3;
		aZombiePoints = 5;
	}
	else
	{
		aCount = 3;
		aZombiePoints = 7;
	}
	
	BungeeDropGrid aBungeeDropGrid;
	SetupBungeeDrop(&aBungeeDropGrid);
	aCount = std::min(aCount, aBungeeDropGrid.mGridArrayCount);

	if (aBungeeDropGrid.mGridArrayCount == 0 || aCount <= 0)
		return;

	for (int i = 0; i < aCount; i++)
	{
		ZombieType aZombieType = PickGraveRisingZombieType();
		BungeeDropZombie(&aBungeeDropGrid, aZombieType);
		aZombiePoints -= GetZombieDefinition(aZombieType).mZombieValue;
		aZombiePoints = std::max(aZombiePoints, 1);
	}
}

void Board::SpawnZombiesFromGraves()
{
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS_2)
		return;

	if (StageHasRoof())
	{
		SpawnZombiesFromSky();
	}
	else if (StageHasPool())
	{
		SpawnZombiesFromPool();
		return;
	}
	
//	int aZombiePoints = GetGraveStonesCount();
	GridItem* aGridItem = nullptr;
	while (IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType != GridItemType::GRIDITEM_GRAVESTONE || aGridItem->mGridItemCounter < 100)
		{
			continue;
		}
		if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_GRAVE_DANGER && Rand(mNumWaves) > mCurrentWave)
		{
			continue;
		}
		
		ZombieType aZombieType = PickGraveRisingZombieType();
		Zombie* aZombie = AddZombie(aZombieType, mCurrentWave);
		if (aZombie == nullptr)
		{
			return;
		}

		aZombie->RiseFromGrave(aGridItem->mGridX, aGridItem->mGridY);
		/*
		aZombiePoints -= GetZombieDefinition(aZombieType).mZombieValue;
		if (aZombieType < 1)
		{
			aZombiePoints = 1;
		}
		*/
	}
}

int Board::TotalZombiesHealthInWave(int theWaveIndex)
{
	int aTotalHealth = 0;
	Zombie* aZombie = nullptr;
	while (IterateZombies(aZombie))
	{
		if (aZombie->mFromWave == theWaveIndex && !aZombie->mMindControlled && !aZombie->IsDeadOrDying() &&
			aZombie->mZombieType != ZombieType::ZOMBIE_BUNGEE && aZombie->mRelatedZombieID == ZombieID::ZOMBIEID_NULL)
		{
			aTotalHealth += aZombie->mBodyHealth + aZombie->mHelmHealth + aZombie->mShieldHealth * 0.2f + aZombie->mFlyingHealth;
		}
	}
	return aTotalHealth;
}

void Board::SpawnZombieWave()
{
	mChallenge->SpawnZombieWave();
	if (mApp->IsBungeeBlitzLevel())
	{
		BungeeDropGrid aBungeeDropGrid;
		SetupBungeeDrop(&aBungeeDropGrid);
		for (int i = 0; i < MAX_ZOMBIES_IN_WAVE; i++)
		{
			ZombieType aZombieType = mZombiesInWave[mCurrentWave][i];
			if (aZombieType == ZombieType::ZOMBIE_INVALID)
				break;

			if (aZombieType == ZombieType::ZOMBIE_BUNGEE || aZombieType == ZombieType::ZOMBIE_ZAMBONI)
			{
				AddZombie(aZombieType, mCurrentWave);
			}
			else
			{
				BungeeDropZombie(&aBungeeDropGrid, aZombieType);
			}
		}
	}
	else
	{
		TOD_ASSERT(mCurrentWave >= 0 && mCurrentWave < MAX_ZOMBIE_WAVES && mCurrentWave < mNumWaves);
		for (int i = 0; i < MAX_ZOMBIES_IN_WAVE; i++)
		{
			ZombieType aZombieType = mZombiesInWave[mCurrentWave][i];
			if (aZombieType == ZombieType::ZOMBIE_INVALID)
				break;

			if (aZombieType == ZombieType::ZOMBIE_BOBSLED && !CanAddBobSled())
			{
				for (int i = 0; i < MAX_ZOMBIE_FOLLOWERS; i++)
				{
					AddZombie(ZombieType::ZOMBIE_NORMAL, mCurrentWave);  // 生成 4 只普通僵尸以代替雪橇僵尸小队
				}
			}
			else
			{
				AddZombie(aZombieType, mCurrentWave);
			}
		}
	}

	if (mCurrentWave == mNumWaves - 1 && !mApp->IsContinuousChallenge())
	{
		mRiseFromGraveCounter = 210;
	}
	if (IsFlagWave(mCurrentWave))
	{
		mFlagRaiseCounter = FLAG_RAISE_TIME;
	}
	mCurrentWave++;
	mTotalSpawnedWaves++;
}

void Board::UpdateGameObjects()
{
	Plant* aPlant = nullptr;
	while (IteratePlants(aPlant))
	{
		aPlant->Update();
	}

	Zombie* aZombie = nullptr;
	while (IterateZombies(aZombie))
	{
		aZombie->Update();
	}

	Projectile* aProjectile = nullptr;
	while (IterateProjectiles(aProjectile))
	{
		aProjectile->Update();
	}

	Coin* aCoin = nullptr;
	while (IterateCoins(aCoin))
	{
		aCoin->Update();
	}

	LawnMower* aLawnMower = nullptr;
	while (IterateLawnMowers(aLawnMower))
	{
		aLawnMower->Update();
	}

	mCursorPreview->Update();
	mCursorObject->Update();

	for (int i = 0; i < mSeedBank->mNumPackets; i++)
	{
		mSeedBank->mSeedPackets[i].Update();
	}
}

void Board::StopAllZombieSounds()
{
	Zombie* aZombie = nullptr;
	while (IterateZombies(aZombie))
	{
		aZombie->StopZombieSound();
	}
}

// GOTY @Patoke: 0x415BD0
int Board::GetSurvivalFlagsCompleted()
{
	int aWavesPerFlag = GetNumWavesPerFlag();
	int aFlagsCompleted = mChallenge->mSurvivalStage * GetNumWavesPerSurvivalStage() / aWavesPerFlag;
	int aCurrentWave = mCurrentWave;
	if (IsFlagWave(aCurrentWave - 1) && mBoardFadeOutCounter < 0 && !mNextSurvivalStageCounter)
	{
		aCurrentWave -= 1;
	}
	return aCurrentWave / aWavesPerFlag + aFlagsCompleted;
}

void Board::SurvivalSaveScore()
{
	if (!mApp->IsSurvivalMode())
		return;

	uint32_t aFlagsCompleted = GetSurvivalFlagsCompleted();
	uint32_t& aFlagsRecord = mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()];
	if (aFlagsCompleted > aFlagsRecord)
	{
		aFlagsRecord = aFlagsCompleted;
		mApp->WriteCurrentUserConfig();
	}
}

void Board::PuzzleSaveStreak()
{
	if (!mApp->IsEndlessIZombie(mApp->mGameMode) && !mApp->IsEndlessScaryPotter(mApp->mGameMode))
		return;

	uint32_t aStreak = mChallenge->mSurvivalStage + 1;
	uint32_t& aRecord = mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()];
	if (aStreak > aRecord)
	{
		aRecord = aStreak;
		mApp->WriteCurrentUserConfig();
	}
}

void Board::ZombiesWon(Zombie* theZombie)
{
	if (mApp->mGameScene == GameScenes::SCENE_ZOMBIES_WON)
		return;

	ClearAdvice(AdviceType::ADVICE_NONE);
	mApp->mBoardResult = BoardResult::BOARDRESULT_LOST;

	Zombie* aZombie = nullptr;
	while (IterateZombies(aZombie))
	{
		if (aZombie == theZombie)
			continue;

		if (aZombie->GetZombieRect().mX < -50 || 
			aZombie->mZombiePhase == ZombiePhase::PHASE_RISING_FROM_GRAVE || 
			aZombie->mZombiePhase == ZombiePhase::PHASE_DANCER_RISING)
		{
			if ((aZombie->mZombieType == ZombieType::ZOMBIE_GARGANTUAR || aZombie->mZombieType == ZombieType::ZOMBIE_REDEYE_GARGANTUAR) && 
				aZombie->IsDeadOrDying() && aZombie->mPosX < 140)
			{
				aZombie->DieNoLoot();
			}
		}
	}
	SurvivalSaveScore();

	std::string aGameOverMsg;
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM)
	{
		aGameOverMsg = "[ZOMBIQUARIUM_DEATH_MESSAGE]";
	}
	else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND)
	{
		std::string aFlagStr = mApp->Pluralize(GetSurvivalFlagsCompleted(), "[ONE_FLAG]", "[COUNT_FLAGS]");
		aGameOverMsg = TodReplaceString("[LAST_STAND_DEATH_MESSAGE]", "{FLAGS}", aFlagStr);
	}
	else if (mApp->IsEndlessIZombie(mApp->mGameMode) || mApp->IsEndlessScaryPotter(mApp->mGameMode))
	{
		aGameOverMsg = TodReplaceNumberString("[ENDLESS_PUZZLE_DEATH_MESSAGE]", "{STREAK}", mChallenge->mSurvivalStage);
	}
	else if (mApp->IsIZombieLevel())
	{
		aGameOverMsg = "[I_ZOMBIE_DEATH_MESSAGE]";
	}
	else
	{
		mApp->mGameScene = GameScenes::SCENE_ZOMBIES_WON;
		if (theZombie)  // 原版此处没有对 theZombie 进行空指针判断，但加上判断后便允许绕过僵尸而直接调用游戏失败
		{
			theZombie->WalkIntoHouse();
		}

		ClearAdvice(AdviceType::ADVICE_NONE);
		mCutScene->StartZombiesWon();
		FreezeEffectsForCutscene(true);
		TutorialArrowRemove();
		UpdateCursor();
		return;
	}

	GameOverDialog* aGameOverDialog = new GameOverDialog(aGameOverMsg, true);
	mApp->AddDialog(Dialogs::DIALOG_GAME_OVER, aGameOverDialog);
	mApp->mWidgetManager->SetFocus(aGameOverDialog);

	mApp->mMusic->StopAllMusic();
	StopAllZombieSounds();
	mApp->PlaySample(Sexy::SOUND_LOSEMUSIC);

	ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_ZOMBIES_WON, true);
	Reanimation* aReanim = mApp->AddReanimation(-BOARD_OFFSET, 0, MakeRenderOrder(RenderLayer::RENDER_LAYER_SCREEN_FADE, 0, 0), ReanimationType::REANIM_ZOMBIES_WON);
	aReanim->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
	aReanim->GetTrackInstanceByName("fullscreen")->mTrackColor = Color::Black;
	aReanim->SetFramesForLayer("anim_screen");
}

bool Board::IsFinalScaryPotterStage()
{
	if (!mApp->IsScaryPotterLevel())
		return false;

	if (mApp->IsAdventureMode())
	{
		return mChallenge->mSurvivalStage == 2;
	}
	
	return !mApp->IsEndlessScaryPotter(mApp->mGameMode);
}

bool Board::IsFinalSurvivalStage()
{
	if (!mApp->IsSurvivalMode())
		return false;

	int aFlags = GetNumWavesPerSurvivalStage() * (mChallenge->mSurvivalStage + 1) / GetNumWavesPerFlag();
	if (mApp->IsSurvivalNormal(mApp->mGameMode))
	{
		return aFlags >= 5;
	}
	if (mApp->IsSurvivalHard(mApp->mGameMode))
	{
		return aFlags >= 10;
	}

	return false;
}

bool Board::IsLastStandFinalStage()
{
	return mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND && mChallenge->mSurvivalStage == LAST_STAND_FLAGS - 1;
}

bool Board::IsSurvivalStageWithRepick()
{
	return mApp->IsSurvivalMode() && !IsFinalSurvivalStage();
}

bool Board::IsLastStandStageWithRepick()
{
	return mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND && !IsLastStandFinalStage();
}

bool Board::HasLevelAwardDropped()
{
	return mLevelAwardSpawned || mNextSurvivalStageCounter > 0 || mBoardFadeOutCounter >= 0;
}

void Board::UpdateSunSpawning()
{
	if (StageIsNight() || 
		HasLevelAwardDropped() || 
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_RAINING_SEEDS || 
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ICE || 
		mApp->mGameMode == GameMode::GAMEMODE_UPSELL ||
		mApp->mGameMode == GameMode::GAMEMODE_INTRO || 
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM || 
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN ||
		mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM || 
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND || 
		mApp->IsIZombieLevel() ||
		mApp->IsScaryPotterLevel() || 
		mApp->IsSquirrelLevel() || 
		HasConveyorBeltSeedBank() || 
		mTutorialState == TutorialState::TUTORIAL_SLOT_MACHINE_PULL)
		return;

	if (mTutorialState == TutorialState::TUTORIAL_LEVEL_1_PICK_UP_PEASHOOTER || mTutorialState == TutorialState::TUTORIAL_LEVEL_1_PLANT_PEASHOOTER)
	{
		if (mPlants.mSize == 0)
		{
			return;
		}
	}

	mSunCountDown--;
	if (mSunCountDown != 0)
		return;

	mNumSunsFallen++;
	mSunCountDown = std::min(SUN_COUNTDOWN_MAX, SUN_COUNTDOWN + mNumSunsFallen * 10) + Rand(SUN_COUNTDOWN_RANGE);
	CoinType aSunType = mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_SUNNY_DAY ? CoinType::COIN_LARGESUN : CoinType::COIN_SUN;
	AddCoin(RandRangeInt(100, 649), 60, aSunType, CoinMotion::COIN_MOTION_FROM_SKY);
}

void Board::NextWaveComing()
{
	if (mCurrentWave + 1 == mNumWaves)
	{
		if (!IsSurvivalStageWithRepick() && mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_LAST_STAND && !mApp->IsContinuousChallenge())
		{
			mApp->AddReanimation(0, 30, MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 0), ReanimationType::REANIM_FINAL_WAVE);
			mFinalWaveSoundCounter = 60;
		}
	}

	if (mCurrentWave == 0)
	{
		mApp->PlaySample(Sexy::SOUND_AWOOGA);
	}
	else if (mApp->IsWhackAZombieLevel() ? (mCurrentWave == mNumWaves - 1) : IsFlagWave(mCurrentWave))
	{
		mApp->PlaySample(Sexy::SOUND_SIREN);
	}
}

void Board::UpdateZombieSpawning()
{
	if (mApp->mGameMode == GameMode::GAMEMODE_UPSELL || mApp->mGameMode == GameMode::GAMEMODE_INTRO)
		return;

	if (mFinalWaveSoundCounter > 0)
	{
		mFinalWaveSoundCounter--;
		if (mFinalWaveSoundCounter == 0)
		{
			mApp->PlaySample(Sexy::SOUND_FINALWAVE);
		}
	}

	if (mTutorialState == TutorialState::TUTORIAL_LEVEL_1_PICK_UP_PEASHOOTER || 
		mTutorialState == TutorialState::TUTORIAL_LEVEL_1_PLANT_PEASHOOTER ||
		mTutorialState == TutorialState::TUTORIAL_LEVEL_1_REFRESH_PEASHOOTER || 
		mTutorialState == TutorialState::TUTORIAL_SLOT_MACHINE_PULL)
		return;

	if (HasLevelAwardDropped())
		return;

	if (mRiseFromGraveCounter > 0)
	{
		mRiseFromGraveCounter--;
		if (mRiseFromGraveCounter == 0)
		{
			SpawnZombiesFromGraves();
		}
	}

	if (mHugeWaveCountDown > 0)
	{
		mHugeWaveCountDown--;
		if (mHugeWaveCountDown == 0)
		{
			ClearAdvice(AdviceType::ADVICE_HUGE_WAVE);
			NextWaveComing();
			mZombieCountDown = 1;
		}
		else
		{
			if (mHugeWaveCountDown == 725)
			{
				mApp->PlaySample(Sexy::SOUND_HUGE_WAVE);
			}
			else
			{
				if (mApp->mMusic->mCurMusicTune == MusicTune::MUSIC_TUNE_DAY_GRASSWALK || 
					mApp->mMusic->mCurMusicTune == MusicTune::MUSIC_TUNE_POOL_WATERYGRAVES || 
					mApp->mMusic->mCurMusicTune == MusicTune::MUSIC_TUNE_FOG_RIGORMORMIST ||
					mApp->mMusic->mCurMusicTune == MusicTune::MUSIC_TUNE_ROOF_GRAZETHEROOF)
				{
					if (mHugeWaveCountDown == 400)
					{
						mApp->mMusic->StartBurst();
					}
				}
				else if (mApp->mMusic->mCurMusicTune == MusicTune::MUSIC_TUNE_NIGHT_MOONGRAINS)
				{
					if (mHugeWaveCountDown == 700)
					{
						mApp->mMusic->StartBurst();
					}
				}
				return;
			}
		}
	}

	if (mChallenge->UpdateZombieSpawning())
		return;

	if (mCurrentWave == mNumWaves)
	{
		if (IsFinalSurvivalStage())
		{
			return;
		}
		if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND)
		{
			return;
		}
		if (!mApp->IsSurvivalMode() && !mApp->IsContinuousChallenge())
		{
			return;
		}
	}

	mZombieCountDown--;
	if (mCurrentWave == mNumWaves && mApp->IsSurvivalMode())
	{
		if (mZombieCountDown == 0)
		{
			FadeOutLevel();
		}
		return;
	}

	if (mZombieCountDown > 200 && mZombieCountDownStart - mZombieCountDown > 400 && TotalZombiesHealthInWave(mCurrentWave - 1) <= mZombieHealthToNextWave)
	{
		mZombieCountDown = 200;
	}
	if (mZombieCountDown == 5)
	{
		if (IsFlagWave(mCurrentWave))
		{
			ClearAdviceImmediately();
			DisplayAdviceAgain("[ADVICE_HUGE_WAVE]", MessageStyle::MESSAGE_STYLE_HUGE_WAVE, AdviceType::ADVICE_HUGE_WAVE);
			mHugeWaveCountDown = 750;
			return;
		}
		NextWaveComing();
	}
	if (mZombieCountDown == 0)
	{
		SpawnZombieWave();
		mZombieHealthWaveStart = TotalZombiesHealthInWave(mCurrentWave - 1);

		if (mCurrentWave == mNumWaves && mApp->IsSurvivalMode())
		{
			mZombieHealthToNextWave = 0;
			mZombieCountDown = ZOMBIE_COUNTDOWN_BEFORE_REPICK + 1;
		}
		else if (IsFlagWave(mCurrentWave) && !(mApp->IsWallnutBowlingLevel() || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND))
		{
			mZombieHealthToNextWave = 0;
			mZombieCountDown = ZOMBIE_COUNTDOWN_BEFORE_FLAG;
		}
		else
		{
			mZombieHealthToNextWave = RandRangeFloat(0.5f, 0.65f) * mZombieHealthWaveStart;
			if (mApp->IsLittleTroubleLevel() || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_COLUMN || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND)
			{
				mZombieCountDown = 750;
			}
			else
			{
				mZombieCountDown = ZOMBIE_COUNTDOWN + Rand(ZOMBIE_COUNTDOWN_RANGE);
			}
		}
		mZombieCountDownStart = mZombieCountDown;
	}
}

void Board::UpdateIce()
{
	for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++)
	{
		if (mIceTimer[aRow])
		{
			mIceTimer[aRow]--;
			TodParticleSystem* aParticleIce = mApp->ParticleTryToGet(mIceParticleID[aRow]);
			if (mIceTimer[aRow] == 0)
			{
				mIceMinX[aRow] = BOARD_ICE_START;
				if (aParticleIce)
				{
					aParticleIce->ParticleSystemDie();
				}
			}
			else
			{
				float aPosX = mIceMinX[aRow];
				float aPosY = GridToPixelY(8, aRow);
				if (aParticleIce)
				{
					aParticleIce->SystemMove(aPosX, aPosY);
				}
				else
				{
					int aRenderPosition = MakeRenderOrder(RenderLayer::RENDER_LAYER_GROUND, aRow, 3);
					aParticleIce = mApp->AddTodParticle(aPosX, aPosY, aRenderPosition, ParticleEffect::PARTICLE_ICE_SPARKLE);
					mIceParticleID[aRow] = mApp->ParticleGetID(aParticleIce);
				}
			}

			int anAlpha = ClampInt(mIceTimer[aRow] / 10, 0, 255);
			aParticleIce->OverrideColor(nullptr, Color(255, 255, 255, anAlpha));
		}
	}
}

void Board::UpdateProgressMeter()
{
	if (mApp->IsFinalBossLevel())
	{
		Zombie* aBoss = GetBossZombie();
		if (aBoss && !aBoss->IsDeadOrDying())
		{
			mProgressMeterWidth = 150 * (aBoss->mBodyMaxHealth - aBoss->mBodyHealth) / aBoss->mBodyMaxHealth;
		}
		else
		{
			mProgressMeterWidth = 150;
		}
	}
	else if (mCurrentWave != 0)
	{
		// 更新旗帜升起倒计时
		if (mFlagRaiseCounter > 0)
			mFlagRaiseCounter--;

		int aTotalWidth = 150;  // 可用于平均分配给每一小波的进度条总长度
		int aNumWavesPerFlag = GetNumWavesPerFlag();  // 本关卡中每相邻两个旗帜波之前的小波数量
		bool aHasFlags = ProgressMeterHasFlags();  // 进度条标注旗帜时，旗帜波占用更长的进度条
		if (aHasFlags)
		{
			aTotalWidth -= 12 * mNumWaves / aNumWavesPerFlag;  // 从每个旗帜波分割出 12 单位的长度
		}

		int aWaveLength = aTotalWidth / (mNumWaves - 1);  // 每一小波占用的进度条长度
		int aCurrentWaveLength = (mCurrentWave - 1) * aTotalWidth / (mNumWaves - 1);  // 当前波开始时的进度条长度
		int aNextWaveLength = mCurrentWave * aTotalWidth / (mNumWaves - 1);  // 下一波开始时的进度条长度
		if (aHasFlags)
		{
			int anExtraLength = mCurrentWave / aNumWavesPerFlag * 12;  // 归还已刷新的旗帜波分割的长度
			aCurrentWaveLength += anExtraLength;
			aNextWaveLength += anExtraLength;
		}

		// 根据倒计时初步计算当前波已经过的比例
		float aFraction = (mZombieCountDownStart - mZombieCountDown) / static_cast<float>(mZombieCountDownStart);
		if (mZombieHealthToNextWave != -1)
		{
			// 取得本波僵尸的当前血量
			int aHealthCurrent = TotalZombiesHealthInWave(mCurrentWave - 1);
			// 取得（本波开始时的僵尸总血量 - 下一波刷新时的僵尸总血量），即：本波刷新需要对僵尸造成的伤害
			int aDamageTarget = mZombieHealthWaveStart - mZombieHealthToNextWave;  //开始时的血量 - 刷新时的血量
			if (aDamageTarget < 1)
			{
				aDamageTarget = 1;  // 需要的伤害至少为 1
			}
			// 再次以刷新血量计算一次当前波已经过的比例
			// 血量比例 = [目标伤害 - (当前血量 - 刷新血量)] / 目标伤害 = (目标伤害 - 仍需造成的伤害) / 目标伤害 = 当前伤害 / 目标伤害
			float aHealthFraction = (aDamageTarget - aHealthCurrent + mZombieHealthToNextWave) / static_cast<float>(aDamageTarget);
			// 最终比例取上述二者的较大值
			aFraction = std::max(aHealthFraction, aFraction);
		}

		// 计算当前应当的进度条长度，并将长度的范围限定在 [1, 150] 之间
		int aLength = ClampInt(aCurrentWaveLength + FloatRoundToInt((aNextWaveLength - aCurrentWaveLength) * aFraction), 1, 150);
		// 取得当前实际与理论的进度条长度之差
		int aDelta = aLength - mProgressMeterWidth;
		// 当差值不超过一波的长度时，每 20cs 调整一次长度；否则，每 5cs 调整一次长度
		if ((aDelta > aWaveLength && (mMainCounter % 5 == 0)) || (aDelta > 0 && (mMainCounter % 20 == 0)))
		{
			mProgressMeterWidth++;
		}
	}
}

void Board::UpdateTutorial()
{
	if (mTutorialTimer > 0)
		mTutorialTimer--;

	if (mTutorialState == TutorialState::TUTORIAL_LEVEL_1_PICK_UP_PEASHOOTER && mTutorialTimer == 0)
	{
		DisplayAdvice("[ADVICE_CLICK_PEASHOOTER]", MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY, AdviceType::ADVICE_NONE);
		TutorialArrowShow(mSeedBank->mX + mSeedBank->mSeedPackets[0].mX, mSeedBank->mY + mSeedBank->mSeedPackets[0].mY);
		mTutorialTimer = -1;
	}
	else if (mTutorialState == TutorialState::TUTORIAL_LEVEL_2_PICK_UP_SUNFLOWER || 
		mTutorialState == TutorialState::TUTORIAL_LEVEL_2_PLANT_SUNFLOWER ||
		mTutorialState == TutorialState::TUTORIAL_LEVEL_2_REFRESH_SUNFLOWER)
	{
		if (mTutorialTimer == 0)
		{
			DisplayAdvice("[ADVICE_PLANT_SUNFLOWER2]", MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL2, AdviceType::ADVICE_NONE);
			mTutorialTimer = -1;
		}
		else if (mZombieCountDown == 750 && mCurrentWave == 0)
		{
			DisplayAdvice("[ADVICE_PLANT_SUNFLOWER3]", MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL2, AdviceType::ADVICE_NONE);
		}
	}
	else if (mTutorialState == TutorialState::TUTORIAL_MORESUN_PICK_UP_SUNFLOWER || 
		mTutorialState == TutorialState::TUTORIAL_MORESUN_PLANT_SUNFLOWER ||
		mTutorialState == TutorialState::TUTORIAL_MORESUN_REFRESH_SUNFLOWER)
	{
		if (mTutorialTimer == 0)
		{
			DisplayAdvice("[ADVICE_PLANT_SUNFLOWER5]", MessageStyle::MESSAGE_STYLE_TUTORIAL_LATER, AdviceType::ADVICE_PLANT_SUNFLOWER5);
			mTutorialTimer = -1;
		}
	}

	// 冒险模式初期关卡，检测到向日葵数量小于 3 时，进入“更多向日葵”的教程
	if (mApp->IsFirstTimeAdventureMode() && mLevel >= 3 && mLevel != 5 && mLevel <= 7 && mTutorialState == TutorialState::TUTORIAL_OFF &&
		mCurrentWave >= 5 && !gShownMoreSunTutorial && mSeedBank->mSeedPackets[1].CanPickUp() && CountPlantByType(SeedType::SEED_SUNFLOWER) < 3)
	{
		TOD_ASSERT(!ChooseSeedsOnCurrentLevel());
		DisplayAdvice("[ADVICE_PLANT_SUNFLOWER4]", MessageStyle::MESSAGE_STYLE_TUTORIAL_LATER_STAY, AdviceType::ADVICE_NONE);
		gShownMoreSunTutorial = true;
		SetTutorialState(TutorialState::TUTORIAL_MORESUN_PICK_UP_SUNFLOWER);
		mTutorialTimer = 500;
	}
}

void Board::SetTutorialState(TutorialState theTutorialState)
{
	switch (theTutorialState)
	{
	case TutorialState::TUTORIAL_LEVEL_1_PICK_UP_PEASHOOTER:
		if (mPlants.mSize == 0)
		{
			float aPosX = mSeedBank->mX + mSeedBank->mSeedPackets[0].mX;
			float aPosY = mSeedBank->mY + mSeedBank->mSeedPackets[0].mY;
			TutorialArrowShow(aPosX, aPosY);
			DisplayAdvice("[ADVICE_CLICK_SEED_PACKET]", MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY, AdviceType::ADVICE_NONE);
		}
		else
		{
			DisplayAdvice("[ADVICE_ENOUGH_SUN]", MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY, AdviceType::ADVICE_NONE);
			mTutorialTimer = 400;
		}
		break;

	case TutorialState::TUTORIAL_LEVEL_1_PLANT_PEASHOOTER:
		mTutorialTimer = -1;
		TutorialArrowRemove();
		if (mPlants.mSize == 0)
		{
			DisplayAdvice("[ADVICE_CLICK_ON_GRASS]", MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY, AdviceType::ADVICE_NONE);
		}
		else
		{
			ClearAdvice(AdviceType::ADVICE_NONE);
		}
		break;

	case TutorialState::TUTORIAL_LEVEL_1_REFRESH_PEASHOOTER:
		DisplayAdvice("[ADVICE_PLANTED_PEASHOOTER]", MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY, AdviceType::ADVICE_NONE);
		mSunCountDown = 400;
		break;

	case TutorialState::TUTORIAL_LEVEL_1_COMPLETED:
		DisplayAdvice("[ADVICE_ZOMBIE_ONSLAUGHT]", MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1, AdviceType::ADVICE_NONE);
		mZombieCountDown = 99;
		mZombieCountDownStart = mZombieCountDown;
		break;

	case TutorialState::TUTORIAL_LEVEL_2_PICK_UP_SUNFLOWER:
	case TutorialState::TUTORIAL_MORESUN_PICK_UP_SUNFLOWER:
		TutorialArrowShow(mSeedBank->mX + mSeedBank->mSeedPackets[1].mX, mSeedBank->mY + mSeedBank->mSeedPackets[1].mY);
		break;

	case TutorialState::TUTORIAL_LEVEL_2_PLANT_SUNFLOWER:
	case TutorialState::TUTORIAL_LEVEL_2_REFRESH_SUNFLOWER:
	case TutorialState::TUTORIAL_MORESUN_PLANT_SUNFLOWER:
	case TutorialState::TUTORIAL_MORESUN_REFRESH_SUNFLOWER:
		TutorialArrowRemove();
		break;

	case TutorialState::TUTORIAL_LEVEL_2_COMPLETED:
		if (mCurrentWave == 0)
		{
			mZombieCountDown = 999;
			mZombieCountDownStart = mZombieCountDown;
		}
		break;

	case TutorialState::TUTORIAL_SLOT_MACHINE_PULL:
		DisplayAdvice("[ADVICE_SLOT_MACHINE_PULL]", MessageStyle::MESSAGE_STYLE_SLOT_MACHINE, AdviceType::ADVICE_SLOT_MACHINE_PULL);
		break;

	case TutorialState::TUTORIAL_SLOT_MACHINE_COMPLETED:
		ClearAdvice(AdviceType::ADVICE_SLOT_MACHINE_PULL);
		break;

	case TutorialState::TUTORIAL_SHOVEL_PICKUP:
	{
		DisplayAdvice("[ADVICE_CLICK_SHOVEL]", MessageStyle::MESSAGE_STYLE_HINT_STAY, AdviceType::ADVICE_NONE);
		Rect aShovelButtonRect = GetShovelButtonRect();
		int aPosX = aShovelButtonRect.mX + aShovelButtonRect.mWidth / 2 - 25;
		int aPosY = aShovelButtonRect.mY + aShovelButtonRect.mHeight - 65;
		TutorialArrowShow(aPosX, aPosY);
		break;
	}

	case TutorialState::TUTORIAL_SHOVEL_DIG:
		DisplayAdvice("[ADVICE_CLICK_PLANT]", MessageStyle::MESSAGE_STYLE_HINT_STAY, AdviceType::ADVICE_NONE);
		TutorialArrowRemove();
		break;

	case TutorialState::TUTORIAL_SHOVEL_KEEP_DIGGING:
		DisplayAdvice("[ADVICE_KEEP_DIGGING]", MessageStyle::MESSAGE_STYLE_HINT_STAY, AdviceType::ADVICE_NONE);
		break;

	case TutorialState::TUTORIAL_SHOVEL_COMPLETED:
		ClearAdvice(AdviceType::ADVICE_NONE);
		mCutScene->mCutsceneTime = 1500;
		mCutScene->mCrazyDaveDialogStart = 2410;
		break;

	default:
		break;
	}

	mTutorialState = theTutorialState;
}

void Board::UpdateGame()
{
	UpdateGameObjects();
	if (StageHasFog() && mFogBlownCountDown > 0)
	{
		float aMaxFogOffset = 1065.0f - LeftFogColumn() * 80.0f;
		if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO)
		{
			mFogOffset = TodAnimateCurveFloat(200, 0, mFogBlownCountDown, aMaxFogOffset, 0, TodCurves::CURVE_EASE_OUT);
		}
		else if (mFogBlownCountDown < 2000)
		{
			mFogOffset = TodAnimateCurveFloat(2000, 0, mFogBlownCountDown, aMaxFogOffset, 0, TodCurves::CURVE_EASE_OUT);
		}
		else if (mFogOffset < aMaxFogOffset)
		{
			mFogOffset = TodAnimateCurveFloat(-5, aMaxFogOffset, mFogOffset * 1.1f, 0, aMaxFogOffset, TodCurves::CURVE_LINEAR);
		}
	}

	if (mApp->mGameScene != GameScenes::SCENE_PLAYING && !mCutScene->ShouldRunUpsellBoard())
		return;

	mMainCounter++;
	UpdateSunSpawning();
	UpdateZombieSpawning();
	UpdateIce();
	if (mIceTrapCounter > 0)
	{
		mIceTrapCounter--;
		if (mIceTrapCounter == 0)
		{
			TodParticleSystem* aPoolSparklyParticle = mApp->ParticleTryToGet(mPoolSparklyParticleID);
			if (aPoolSparklyParticle)
			{
				aPoolSparklyParticle->mDontUpdate = false;
			}
		}
	}

	if (mFogBlownCountDown > 0)
	{
		mFogBlownCountDown--;
	}

	if (mMainCounter == 1 && mApp->IsFirstTimeAdventureMode())
	{
		if (mLevel == 1)
		{
			SetTutorialState(TutorialState::TUTORIAL_LEVEL_1_PICK_UP_PEASHOOTER);
		}
		else if (mLevel == 2)
		{
			SetTutorialState(TutorialState::TUTORIAL_LEVEL_2_PICK_UP_SUNFLOWER);
			DisplayAdvice("[ADVICE_PLANT_SUNFLOWER1]", MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL2, AdviceType::ADVICE_NONE);
			mTutorialTimer = 500;
		}
	}

	UpdateProgressMeter();
}

void Board::Update()
{
	TodHesitationBracket aHesitation("Board::Update");

	Widget::Update();
	MarkDirty();

	mCutScene->Update();
	UpdateMousePosition();
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		mApp->mZenGarden->ZenGardenUpdate();
	}
	if (IsScaryPotterDaveTalking())
	{
		mApp->UpdateCrazyDave();
	}

	if (mPaused)
	{
		mChallenge->Update();
		mCursorPreview->mVisible = false;
		mCursorObject->mVisible = false;
		return;
	}

	bool aDisabled = !CanInteractWithBoardButtons() || mIgnoreMouseUp;
	if (!mMenuButton->mBtnNoDraw)
	{
		mMenuButton->mDisabled = aDisabled;
	}
	mMenuButton->Update();
	if (mStoreButton)
	{
		mStoreButton->mDisabled = aDisabled;
		mStoreButton->Update();
	}

	mApp->mEffectSystem->Update();
	mAdvice->Update();
	UpdateTutorial();

	if (mCobCannonCursorDelayCounter > 0)
	{
		mCobCannonCursorDelayCounter--;
	}
	if (mOutOfMoneyCounter > 0)
	{
		mOutOfMoneyCounter--;
	}
	if (mShakeCounter > 0)
	{
		mShakeCounter--;
		if (mShakeCounter == 0)
		{
			mX = 0;
			mY = 0;
		}
		else
		{
			if (!Rand(3))
			{
				mShakeAmountX = -mShakeAmountX;
			}
			mX = TodAnimateCurve(12, 0, mShakeCounter, 0, mShakeAmountX, TodCurves::CURVE_BOUNCE);
			mY = TodAnimateCurve(12, 0, mShakeCounter, 0, mShakeAmountY, TodCurves::CURVE_BOUNCE);
		}
	}
	if (mCoinBankFadeCount > 0 && mApp->GetDialog(Dialogs::DIALOG_PURCHASE_PACKET_SLOT) == nullptr)
	{
		mCoinBankFadeCount--;
	}
	UpdateLayers();

	if (mTimeStopCounter > 0)
		return;

	mEffectCounter++;
	if (StageHasPool() && !mIceTrapCounter && mApp->mGameScene != GameScenes::SCENE_ZOMBIES_WON && !mCutScene->IsSurvivalRepick())
	{
		mApp->mPoolEffect->mPoolCounter++;
	}
	if (mBackground == BackgroundType::BACKGROUND_3_POOL && mPoolSparklyParticleID == ParticleSystemID::PARTICLESYSTEMID_NULL && mDrawCount > 0)
	{
		int aRenderPosition = MakeRenderOrder(RenderLayer::RENDER_LAYER_GROUND, 2, 0);
		TodParticleSystem* aPoolParticle = mApp->AddTodParticle(450, 295, aRenderPosition, ParticleEffect::PARTICLE_POOL_SPARKLY);
		mPoolSparklyParticleID = mApp->ParticleGetID(aPoolParticle);
	}

	UpdateGridItems();
	UpdateFwoosh();
	UpdateGame();
	UpdateFog();
	mChallenge->Update();
	UpdateLevelEndSequence();
	mPrevMouseX = mApp->mWidgetManager->mLastMouseX;
	mPrevMouseY = mApp->mWidgetManager->mLastMouseY;
}

// GOTY @Patoke: 0x418940
void Board::UpdateLayers()
{
	if (mWidgetManager)
	{
		mWidgetManager->MarkAllDirty();

		for (DialogList::iterator anIter = mApp->mDialogList.begin(); anIter != mApp->mDialogList.end(); ++anIter)
		{
			Dialog* aDialog = *anIter;
			mWidgetManager->BringToFront(aDialog);
			aDialog->MarkDirty();
		}
	}
}

bool Board::RowCanHaveZombies(int theRow)
{
	if (theRow < 0 || theRow >= MAX_GRID_SIZE_Y)
		return false;

	return (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_RESODDED && theRow <= 4) || mPlantRow[theRow] != PlantRowType::PLANTROW_DIRT;
}

int Board::GetIceZPos(int theRow)
{
	return MakeRenderOrder(RenderLayer::RENDER_LAYER_GROUND, theRow, 2);
}

void Board::DrawIce(Graphics* g, int theGridY)
{
	int aPosY = GridToPixelY(8, theGridY) + 20;
	int aHeight = Sexy::IMAGE_ICE->GetHeight();
	int aWidth = Sexy::IMAGE_ICE->GetWidth();
	int anAlpha = ClampInt(255 * mIceTimer[theGridY] / 10, 0, 255);
	if (anAlpha < 255)
	{
		g->SetColorizeImages(true);
		g->SetColor(Color(255, 255, 255, anAlpha));
	}

	int aBeginningX = mIceMinX[theGridY] + 13, aDeltaX;
	for (int aPosX = aBeginningX; aPosX < BOARD_WIDTH; aPosX += aDeltaX)
	{
		if (aPosX == aBeginningX)
		{
			aDeltaX = (BOARD_WIDTH - aBeginningX) % aWidth;
			if (!aDeltaX) aDeltaX = aWidth;
		}
		else aDeltaX = aWidth;
		Rect aRepeatSrcRect(aWidth - aDeltaX, 0, aDeltaX, aHeight);
		Rect aRepeatDstRect(aPosX, aPosY, aDeltaX, aHeight);
		g->DrawImage(Sexy::IMAGE_ICE, aRepeatDstRect, aRepeatSrcRect);
	}
	g->DrawImage(Sexy::IMAGE_ICE_CAP, mIceMinX[theGridY], aPosY);
	g->SetColorizeImages(false);
}

void Board::DrawBackdrop(Graphics* g)
{
	Image* aBgImage = nullptr;
	switch (mBackground)
	{
	case BackgroundType::BACKGROUND_1_DAY:				aBgImage = Sexy::IMAGE_BACKGROUND1;						break;
	case BackgroundType::BACKGROUND_2_NIGHT:			aBgImage = Sexy::IMAGE_BACKGROUND2;						break;
	case BackgroundType::BACKGROUND_3_POOL:				aBgImage = Sexy::IMAGE_BACKGROUND3;						break;
	case BackgroundType::BACKGROUND_4_FOG:				aBgImage = Sexy::IMAGE_BACKGROUND4;						break;
	case BackgroundType::BACKGROUND_5_ROOF:				aBgImage = Sexy::IMAGE_BACKGROUND5;						break;
	case BackgroundType::BACKGROUND_6_BOSS:				aBgImage = Sexy::IMAGE_BACKGROUND6BOSS;					break;
	case BackgroundType::BACKGROUND_MUSHROOM_GARDEN:	aBgImage = Sexy::IMAGE_BACKGROUND_MUSHROOMGARDEN;		break;
	case BackgroundType::BACKGROUND_GREENHOUSE:			aBgImage = Sexy::IMAGE_BACKGROUND_GREENHOUSE;			break;
	case BackgroundType::BACKGROUND_ZOMBIQUARIUM:		aBgImage = Sexy::IMAGE_AQUARIUM1;						break;
	case BackgroundType::BACKGROUND_TREEOFWISDOM:		aBgImage = nullptr;										break;
	default:											TOD_ASSERT(false);											break;
	}

	if (mLevel == 1 && mApp->IsFirstTimeAdventureMode())
	{
		g->DrawImage(Sexy::IMAGE_BACKGROUND1UNSODDED, -BOARD_OFFSET, 0);
		int aWidth = TodAnimateCurve(0, 1000, mSodPosition, 0, Sexy::IMAGE_SOD1ROW->GetWidth(), TodCurves::CURVE_LINEAR);
		Rect aSrcRect(0, 0, aWidth, Sexy::IMAGE_SOD1ROW->GetHeight());
		g->DrawImage(Sexy::IMAGE_SOD1ROW, 239 - BOARD_OFFSET, 265, aSrcRect);
	}
	else if (((mLevel == 2 || mLevel == 3) && mApp->IsFirstTimeAdventureMode()) || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_RESODDED)
	{
		g->DrawImage(Sexy::IMAGE_BACKGROUND1UNSODDED, -BOARD_OFFSET, 0);
		g->DrawImage(Sexy::IMAGE_SOD1ROW, 239 - BOARD_OFFSET, 265);
		int aWidth = TodAnimateCurve(0, 1000, mSodPosition, 0, Sexy::IMAGE_SOD3ROW->GetWidth(), TodCurves::CURVE_LINEAR);
		Rect aSrcRect(0, 0, aWidth, Sexy::IMAGE_SOD3ROW->GetHeight());
		g->DrawImage(Sexy::IMAGE_SOD3ROW, 235 - BOARD_OFFSET, 149, aSrcRect);
	}
	else if (mLevel == 4 && mApp->IsFirstTimeAdventureMode())
	{
		g->DrawImage(Sexy::IMAGE_BACKGROUND1UNSODDED, -BOARD_OFFSET, 0);
		g->DrawImage(Sexy::IMAGE_SOD3ROW, 235 - BOARD_OFFSET, 149);
		int aWidth = TodAnimateCurve(0, 1000, mSodPosition, 0, 773, TodCurves::CURVE_LINEAR);
		Rect aSrcRect(232, 0, aWidth, Sexy::IMAGE_BACKGROUND1->GetHeight());
		g->DrawImage(Sexy::IMAGE_BACKGROUND1, 232 - BOARD_OFFSET, 0, aSrcRect);
	}
	else if (aBgImage)
	{
		if (aBgImage == Sexy::IMAGE_BACKGROUND_MUSHROOMGARDEN || aBgImage == Sexy::IMAGE_BACKGROUND_GREENHOUSE || aBgImage == Sexy::IMAGE_AQUARIUM1)
		{
			g->DrawImage(aBgImage, 0, 0);
		}
		else
		{
			g->DrawImage(aBgImage, -BOARD_OFFSET, 0);
		}
	}

	if (mApp->mGameScene == GameScenes::SCENE_ZOMBIES_WON)
	{
		DrawHouseDoorBottom(g);
	}
	if (StageHasPool())
	{
		mApp->mPoolEffect->PoolEffectDraw(g, StageIsNight());
	}
	if (mTutorialState == TutorialState::TUTORIAL_LEVEL_1_PLANT_PEASHOOTER)
	{
		Graphics aClipG(*g);
		aClipG.SetColorizeImages(true);
		aClipG.SetColor(GetFlashingColor(mMainCounter, 75));
		aClipG.DrawImage(Sexy::IMAGE_SOD1ROW, 239 - BOARD_OFFSET, 265);
		aClipG.SetColorizeImages(false);
	}
	mChallenge->DrawBackdrop(g);
	if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO && StageHasGraveStones())
	{
		g->DrawImage(Sexy::IMAGE_NIGHT_GRAVE_GRAPHIC, 1092, 40);
	}
}

bool RenderItemSortFunc(const RenderItem& theItem1, const RenderItem& theItem2)
{
	if (theItem1.mZPos == theItem2.mZPos)
	{
		return theItem1.mGameObject < theItem2.mGameObject;
	}

	return theItem1.mZPos < theItem2.mZPos;
}

void Board::AddBossRenderItem(RenderItem* theRenderList, int& theCurRenderItem, Zombie* theBossZombie)
{
	TOD_ASSERT(theCurRenderItem < MAX_RENDER_ITEMS);
	int aBackLegRow = 1;
	int aFrontLegRow = 3;
	int aBackArmRow = 4;
	if (theBossZombie->IsDeadOrDying())
	{
		aBackArmRow = 1;
	}
	else if (theBossZombie->mZombiePhase == ZombiePhase::PHASE_BOSS_STOMPING)
	{
		Reanimation* aBossReanim = mApp->ReanimationTryToGet(theBossZombie->mBodyReanimID);
		if (aBossReanim->mAnimTime > 0.25f && aBossReanim->mAnimTime < 0.75f)
		{
			if (theBossZombie->mTargetRow == 1)
			{
				aBackLegRow = 2;
			}
			else if (theBossZombie->mTargetRow == 3)
			{
				aFrontLegRow = 4;
			}
		}
	}

	RenderItem* aItem = &theRenderList[theCurRenderItem];
	aItem->mRenderObjectType = RenderObjectType::RENDER_ITEM_BOSS_PART;
	aItem->mZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_BOSS, aBackLegRow, 2);
	aItem->mBossPart = BossPart::BOSS_PART_BACK_LEG;
	theCurRenderItem++;
	aItem = &theRenderList[theCurRenderItem];
	aItem->mRenderObjectType = RenderObjectType::RENDER_ITEM_BOSS_PART;
	aItem->mZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_BOSS, aFrontLegRow, 2);
	aItem->mBossPart = BossPart::BOSS_PART_FRONT_LEG;
	theCurRenderItem++;
	aItem = &theRenderList[theCurRenderItem];
	aItem->mRenderObjectType = RenderObjectType::RENDER_ITEM_BOSS_PART;
	aItem->mZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_BOSS, 4, 2);
	aItem->mBossPart = BossPart::BOSS_PART_MAIN;
	theCurRenderItem++;
	aItem = &theRenderList[theCurRenderItem];
	aItem->mRenderObjectType = RenderObjectType::RENDER_ITEM_BOSS_PART;
	aItem->mZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_BOSS, aBackArmRow, 3);
	aItem->mBossPart = BossPart::BOSS_PART_BACK_ARM;
	theCurRenderItem++;

	Reanimation* aBallReanim = mApp->ReanimationTryToGet(theBossZombie->mBossFireBallReanimID);
	if (aBallReanim)
	{
		RenderItem* aItem = &theRenderList[theCurRenderItem];
		aItem->mRenderObjectType = RenderObjectType::RENDER_ITEM_BOSS_PART;
		aItem->mZPos = aBallReanim->mRenderOrder;
		aItem->mBossPart = BossPart::BOSS_PART_FIREBALL;
		theCurRenderItem++;
	}
}

/*
[[maybe_unused]]
static inline void AddGameObjectRenderItem(RenderItem* theRenderList, int& theCurRenderItem, RenderObjectType theRenderObjectType, GameObject* theGameObject)
{
	TOD_ASSERT(theCurRenderItem < MAX_RENDER_ITEMS);
	RenderItem& aRenderItem = theRenderList[theCurRenderItem];
	aRenderItem.mRenderObjectType = theRenderObjectType;
	aRenderItem.mZPos = theGameObject->mRenderOrder;
	aRenderItem.mGameObject = theGameObject;
	theCurRenderItem++;
}
*/

static inline void AddGameObjectRenderItemCursorPreview(RenderItem* theRenderList, int& theCurRenderItem, RenderObjectType theRenderObjectType, GameObject* theGameObject)
{
	TOD_ASSERT(theCurRenderItem < MAX_RENDER_ITEMS);
	RenderItem& aRenderItem = theRenderList[theCurRenderItem];
	aRenderItem.mRenderObjectType = theRenderObjectType;
	aRenderItem.mZPos = theGameObject->mRenderOrder;
	aRenderItem.mGameObject = theGameObject;
	aRenderItem.mCursorPreview = (CursorPreview*)theGameObject;

	theCurRenderItem++;
}

static inline void AddGameObjectRenderItemPlant(RenderItem* theRenderList, int& theCurRenderItem, RenderObjectType theRenderObjectType, GameObject* theGameObject)
{
	TOD_ASSERT(theCurRenderItem < MAX_RENDER_ITEMS);
	RenderItem& aRenderItem = theRenderList[theCurRenderItem];
	aRenderItem.mRenderObjectType = theRenderObjectType;
	aRenderItem.mZPos = theGameObject->mRenderOrder;
	aRenderItem.mGameObject = theGameObject;
	aRenderItem.mPlant = (Plant*)theGameObject;

	theCurRenderItem++;
}

static inline void AddGameObjectRenderItemZombie(RenderItem* theRenderList, int& theCurRenderItem, RenderObjectType theRenderObjectType, GameObject* theGameObject)
{
	TOD_ASSERT(theCurRenderItem < MAX_RENDER_ITEMS);
	RenderItem& aRenderItem = theRenderList[theCurRenderItem];
	aRenderItem.mRenderObjectType = theRenderObjectType;
	aRenderItem.mZPos = theGameObject->mRenderOrder;
	aRenderItem.mGameObject = theGameObject;
	aRenderItem.mZombie = (Zombie*)theGameObject;
	theCurRenderItem++;
}

static inline void AddGameObjectRenderItemProjectile(RenderItem* theRenderList, int& theCurRenderItem, RenderObjectType theRenderObjectType, GameObject* theGameObject)
{
	TOD_ASSERT(theCurRenderItem < MAX_RENDER_ITEMS);
	RenderItem& aRenderItem = theRenderList[theCurRenderItem];
	aRenderItem.mRenderObjectType = theRenderObjectType;
	aRenderItem.mZPos = theGameObject->mRenderOrder;
	aRenderItem.mGameObject = theGameObject;
	aRenderItem.mProjectile = (Projectile*)theGameObject;
	theCurRenderItem++;
}

static inline void AddGameObjectRenderItemCoin(RenderItem* theRenderList, int& theCurRenderItem, RenderObjectType theRenderObjectType, GameObject* theGameObject)
{
	TOD_ASSERT(theCurRenderItem < MAX_RENDER_ITEMS);
	RenderItem& aRenderItem = theRenderList[theCurRenderItem];
	aRenderItem.mRenderObjectType = theRenderObjectType;
	aRenderItem.mZPos = theGameObject->mRenderOrder;
	aRenderItem.mGameObject = theGameObject;
	aRenderItem.mCoin = (Coin*)theGameObject;
	theCurRenderItem++;
}

static inline void AddUIRenderItem(RenderItem* theRenderList, int& theCurRenderItem, RenderObjectType theRenderObjectType, int thePosZ)
{
	TOD_ASSERT(theCurRenderItem < MAX_RENDER_ITEMS);
	RenderItem& aRenderItem = theRenderList[theCurRenderItem];
	aRenderItem.mRenderObjectType = theRenderObjectType;
	aRenderItem.mZPos = thePosZ;
	aRenderItem.mGameObject = nullptr;
	theCurRenderItem++;
}

void Board::DrawGameObjects(Graphics* g)
{
	TodHesitationTrace("creating render list");

	RenderItem aRenderList[MAX_RENDER_ITEMS];
	int aRenderItemCount = 0;

	{
		Plant* aPlant = nullptr;
		while (IteratePlants(aPlant))
		{
			if (aPlant->mOnBungeeState == PlantOnBungeeState::NOT_ON_BUNGEE)
			{
				AddGameObjectRenderItemPlant(aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_PLANT, aPlant);

				if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN && aPlant->mPottedPlantIndex != -1)
				{
					RenderItem& aRenderItem = aRenderList[aRenderItemCount];
					aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_PLANT_OVERLAY;
					aRenderItem.mZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_PARTICLE, 0, mY);
					aRenderItem.mPlant = aPlant;
					aRenderItemCount++;
				}

				if ((aPlant->mSeedType == SeedType::SEED_MAGNETSHROOM || aPlant->mSeedType == SeedType::SEED_GOLD_MAGNET) && aPlant->DrawMagnetItemsOnTop())
				{
					RenderItem& aRenderItem = aRenderList[aRenderItemCount];
					aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_PLANT_MAGNET_ITEMS;
					aRenderItem.mZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_TOP, 0, -1);
					aRenderItem.mPlant = aPlant;
					aRenderItemCount++;
				}
			}
		}
	}
	{
		Coin* aCoin = nullptr;
		while (IterateCoins(aCoin))
		{
			AddGameObjectRenderItemCoin(aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_COIN, aCoin);
		}
	}
	{
		Zombie* aZombie = nullptr;
		while (IterateZombies(aZombie))
		{
			if (aZombie->mZombieType == ZombieType::ZOMBIE_BOSS)
			{
				AddBossRenderItem(aRenderList, aRenderItemCount, aZombie);
			}
			else
			{
				AddGameObjectRenderItemZombie(aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_ZOMBIE, aZombie);

				if (aZombie->HasShadow())
				{
					RenderItem& aRenderItem = aRenderList[aRenderItemCount];
					aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_ZOMBIE_SHADOW;
					aRenderItem.mZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_GROUND, aZombie->mRow, 3);
					aRenderItem.mZombie = aZombie;
					aRenderItemCount++;
				}

				if (aZombie->mZombieType == ZombieType::ZOMBIE_BUNGEE)
				{
					RenderItem& aRenderItem = aRenderList[aRenderItemCount];
					aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_ZOMBIE_BUNGEE_TARGET;
					aRenderItem.mZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_PROJECTILE, aZombie->mRow, 1);
					aRenderItem.mZombie = aZombie;
					aRenderItemCount++;
				}
			}
		}
	}
	{
		Projectile* aProjectile = nullptr;
		while (IterateProjectiles(aProjectile))
		{
			AddGameObjectRenderItemProjectile(aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_PROJECTILE, aProjectile);

			RenderItem& aRenderItem = aRenderList[aRenderItemCount];
			aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_PROJECTILE_SHADOW;
			aRenderItem.mZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_GROUND, aProjectile->mRow, 3);
			aRenderItem.mProjectile = aProjectile;
			aRenderItemCount++;
		}
	}
	{
		LawnMower* aLawnMower = nullptr;
		while (IterateLawnMowers(aLawnMower))
		{
			RenderItem& aRenderItem = aRenderList[aRenderItemCount];
			aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_MOWER;
			aRenderItem.mZPos = aLawnMower->mRenderOrder;
			aRenderItem.mMower = aLawnMower;
			aRenderItemCount++;
		}
	}
	{
		TodParticleSystem* aParticle = nullptr;
		while (IterateParticles(aParticle))
		{
			if (!aParticle->mIsAttachment)
			{
				RenderItem& aRenderItem = aRenderList[aRenderItemCount];
				aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_PARTICLE;
				aRenderItem.mZPos = aParticle->mRenderOrder;
				aRenderItem.mParticleSytem = aParticle;
				aRenderItemCount++;
			}
		}
	}
	{
		Reanimation* aReanimation = nullptr;
		while (IterateReanimations(aReanimation))
		{
			if (!aReanimation->mIsAttachment)
			{
				RenderItem& aRenderItem = aRenderList[aRenderItemCount];
				aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_REANIMATION;
				aRenderItem.mZPos = aReanimation->mRenderOrder;
				aRenderItem.mReanimation = aReanimation;
				aRenderItemCount++;
			}
		}
	}
	{
		GridItem* aGridItem = nullptr;
		while (IterateGridItems(aGridItem))
		{
			RenderItem& aRenderItem = aRenderList[aRenderItemCount];
			aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_GRID_ITEM;
			aRenderItem.mZPos = aGridItem->mRenderOrder;
			aRenderItem.mGridItem = aGridItem;
			aRenderItemCount++;

			if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN && aGridItem->mGridItemType == GridItemType::GRIDITEM_STINKY)
			{
				RenderItem& aRenderItem = aRenderList[aRenderItemCount];
				aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_GRID_ITEM_OVERLAY;
				aRenderItem.mZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_PARTICLE, 0, aGridItem->mPosY - 30.0f);
				aRenderItem.mGridItem = aGridItem;
				aRenderItemCount++;
			}
		}
	}
	for (int i = 0; i < MAX_GRID_SIZE_Y; i++)
	{
		if (mIceTimer[i])
		{
			RenderItem& aRenderItem = aRenderList[aRenderItemCount];
			aRenderItem.mRenderObjectType = RenderObjectType::RENDER_ITEM_ICE;
			aRenderItem.mBoardGridY = i;
			aRenderItem.mZPos = GetIceZPos(i);
			aRenderItemCount++;
		}
	}
	{
		int aZPos;
		if (mTimeStopCounter > 0)
		{
			aZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 0);
		}
		else if (mApp->mGameScene == GameScenes::SCENE_PLAYING || mApp->mGameScene == GameScenes::SCENE_ZOMBIES_WON)
		{
			aZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_UI_BOTTOM, 0, 1);
		}
		else if (mCutScene->IsAfterSeedChooser() || mCutScene->IsInShovelTutorial() || mHelpIndex == AdviceType::ADVICE_CLICK_TO_CONTINUE)
		{
			aZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_UI_BOTTOM, 0, 1);
		}
		else
		{
			aZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 0);
		}

		AddUIRenderItem(aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_BACKDROP, MakeRenderOrder(RenderLayer::RENDER_LAYER_UI_BOTTOM, 0, 0));
		AddUIRenderItem(aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_BOTTOM_UI, aZPos);
		AddUIRenderItem(aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_COIN_BANK, MakeRenderOrder(RenderLayer::RENDER_LAYER_COIN_BANK, 0, 0));
		AddUIRenderItem(aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_TOP_UI, MakeRenderOrder(RenderLayer::RENDER_LAYER_UI_TOP, 0, 0));
		AddUIRenderItem(aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_SCREEN_FADE, MakeRenderOrder(RenderLayer::RENDER_LAYER_SCREEN_FADE, 0, 0));
	}
	if (mApp->mGameScene == GameScenes::SCENE_ZOMBIES_WON)
	{
		int aZPos;
		if (StageHasRoof())
		{
			aZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_GRAVE_STONE, 0, 4);
		}
		else
		{
			aZPos = MakeRenderOrder(RenderLayer::RENDER_LAYER_GRAVE_STONE, 3, 2);
		}
		AddUIRenderItem(aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_DOOR_MASK, aZPos);
	}
	if (StageHasFog())
	{
		AddUIRenderItem(aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_FOG, MakeRenderOrder(RenderLayer::RENDER_LAYER_FOG, 0, 0));
	}
	if (mApp->IsStormyNightLevel() || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_RAINING_SEEDS)
	{
		AddUIRenderItem(aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_STORM, MakeRenderOrder(RenderLayer::RENDER_LAYER_FOG, 0, 3));
	}
	AddGameObjectRenderItemCursorPreview(aRenderList, aRenderItemCount, RenderObjectType::RENDER_ITEM_CURSOR_PREVIEW, mCursorPreview);

	TodHesitationTrace("start sort");
	std::sort(aRenderList, aRenderList + aRenderItemCount, RenderItemSortFunc);

	TodHesitationTrace("end sort, start draw");
	for (int i = 0; i < aRenderItemCount; i++)
	{
		RenderItem& aRenderItem = aRenderList[i];
		switch (aRenderItem.mRenderObjectType)
		{
		case RenderObjectType::RENDER_ITEM_PLANT:
		{
			Plant* aPlant = aRenderItem.mPlant;
			if (aPlant->BeginDraw(g))
			{
				aPlant->Draw(g);
				aPlant->EndDraw(g);
			}
			break;
		}

		case RenderObjectType::RENDER_ITEM_PLANT_OVERLAY:
		{
			Plant* aPlant = aRenderItem.mPlant;
			if (aPlant->BeginDraw(g))
			{
				mApp->mZenGarden->DrawPlantOverlay(g, aPlant);
				aPlant->EndDraw(g);
			}
			break;
		}

		case RenderObjectType::RENDER_ITEM_PLANT_MAGNET_ITEMS:
		{
			Plant* aPlant = aRenderItem.mPlant;
			if (aPlant->BeginDraw(g))
			{
				aPlant->DrawMagnetItems(g);
				aPlant->EndDraw(g);
			}
			break;
		}

		case RenderObjectType::RENDER_ITEM_MOWER:
		{
			LawnMower* aLawnMower = aRenderItem.mMower;
			aLawnMower->Draw(g);
			break;
		}

		case RenderObjectType::RENDER_ITEM_ZOMBIE:
		{
			Zombie* aZombie = aRenderItem.mZombie;
			if (aZombie->BeginDraw(g))
			{
				aZombie->Draw(g);
				aZombie->EndDraw(g);
			}
			break;
		}

		case RenderObjectType::RENDER_ITEM_ZOMBIE_SHADOW:
		{
			Zombie* aZombie = aRenderItem.mZombie;
			if (aZombie->BeginDraw(g))
			{
				aZombie->DrawShadow(g);
				aZombie->EndDraw(g);
			}
			break;
		}

		case RenderObjectType::RENDER_ITEM_ZOMBIE_BUNGEE_TARGET:
		{
			Zombie* aZombie = aRenderItem.mZombie;
			aZombie->DrawBungeeTarget(g);
			break;
		}

		case RenderObjectType::RENDER_ITEM_BOSS_PART:
		{
			Zombie* aBossZombie = GetBossZombie();
			if (aBossZombie && aBossZombie->BeginDraw(g))
			{
				aBossZombie->DrawBossPart(g, aRenderItem.mBossPart);
				aBossZombie->EndDraw(g);
			}
			break;
		}

		case RenderObjectType::RENDER_ITEM_COIN:
		{
			Coin* aCoin = aRenderItem.mCoin;
			if (aCoin->BeginDraw(g))
			{
				aCoin->Draw(g);
				aCoin->EndDraw(g);
			}
			break;
		}

		case RenderObjectType::RENDER_ITEM_PROJECTILE:
		{
			Projectile* aProjectile = aRenderItem.mProjectile;
			if (aProjectile->BeginDraw(g))
			{
				aProjectile->Draw(g);
				aProjectile->EndDraw(g);
			}
			break;
		}

		case RenderObjectType::RENDER_ITEM_PROJECTILE_SHADOW:
		{
			Projectile* aProjectile = aRenderItem.mProjectile;
			if (aProjectile->BeginDraw(g))
			{
				aProjectile->DrawShadow(g);
				aProjectile->EndDraw(g);
			}
			break;
		}

		case RenderObjectType::RENDER_ITEM_CURSOR_PREVIEW:
		{
			CursorPreview* aCursorPreview = aRenderItem.mCursorPreview;
			if (aCursorPreview->BeginDraw(g))
			{
				aCursorPreview->Draw(g);
				aCursorPreview->EndDraw(g);
			}
			break;
		}
		
		case RenderObjectType::RENDER_ITEM_GRID_ITEM:
		{
			GridItem* aGridItem = aRenderItem.mGridItem;
			aGridItem->DrawGridItem(g);
			break;
		}

		case RenderObjectType::RENDER_ITEM_GRID_ITEM_OVERLAY:
		{
			GridItem* aGridItem = aRenderItem.mGridItem;
			aGridItem->DrawGridItemOverlay(g);
			break;
		}

		case RenderObjectType::RENDER_ITEM_ICE:
			DrawIce(g, aRenderItem.mBoardGridY);
			break;

		case RenderObjectType::RENDER_ITEM_PARTICLE:
		{
			TodParticleSystem* aParticle = aRenderItem.mParticleSytem;
			aParticle->Draw(g);
			break;
		}

		case RenderObjectType::RENDER_ITEM_REANIMATION:
		{
			Reanimation* aReanimation = aRenderItem.mReanimation;
			aReanimation->Draw(g);
			break;
		}

		case RenderObjectType::RENDER_ITEM_COIN_BANK:
			DrawUICoinBank(g);
			break;

		case RenderObjectType::RENDER_ITEM_BACKDROP:
			DrawBackdrop(g);
			break;

		case RenderObjectType::RENDER_ITEM_DOOR_MASK:
			DrawHouseDoorTop(g);
			break;

		case RenderObjectType::RENDER_ITEM_BOTTOM_UI:
			DrawUIBottom(g);
			break;
		
		case RenderObjectType::RENDER_ITEM_TOP_UI:
			DrawUITop(g);
			break;
			
		case RenderObjectType::RENDER_ITEM_FOG:
			DrawFog(g);
			break;

		case RenderObjectType::RENDER_ITEM_STORM:
			mChallenge->DrawWeather(g);
			break;
		
		case RenderObjectType::RENDER_ITEM_SCREEN_FADE:
			DrawFadeOut(g);
			break;

		default:
			TOD_ASSERT(false);
			break;
		}
	}

	TodHesitationTrace("end draw");
}

bool Board::HasProgressMeter()
{
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED || 
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST || 
		mApp->IsFinalBossLevel() || 
		mApp->IsSlotMachineLevel() || 
		mApp->IsSquirrelLevel() || 
		mApp->IsIZombieLevel())
		return true;

	if (mProgressMeterWidth == 0)
		return false;

	if (mApp->IsContinuousChallenge() || 
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN || 
		mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM || 
		mApp->IsScaryPotterLevel())
		return false;

	return true;
}

bool Board::ProgressMeterHasFlags()
{
	if (mApp->IsFirstTimeAdventureMode() && mLevel == 1)
		return false;

	if (mApp->IsWhackAZombieLevel() ||
		mApp->IsFinalBossLevel() ||
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED ||
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST ||
		mApp->IsSlotMachineLevel() ||
		mApp->IsSquirrelLevel() ||
		mApp->IsIZombieLevel())
		return false;

	return true;
}

// GOTY @Patoke: 0x419E30
void Board::DrawProgressMeter(Graphics* g)
{
	if (!HasProgressMeter())
		return;

	// ====================================================================================================
	// ▲ 绘制进度条进度部分的贴图
	// ====================================================================================================
	g->DrawImageCel(Sexy::IMAGE_FLAGMETER, 600, 575, 0);
	int aCelWidth = Sexy::IMAGE_FLAGMETER->GetCelWidth();
	int aCelHeight = Sexy::IMAGE_FLAGMETER->GetCelHeight();
	int aClipWidth = TodAnimateCurve(0, PROGRESS_METER_COUNTER, mProgressMeterWidth, 0, 143, TodCurves::CURVE_LINEAR);
	Rect aSrcRect(aCelWidth - aClipWidth - 7, aCelHeight, aClipWidth, aCelHeight);
	Rect aDstRect(aCelWidth - aClipWidth + 593, 575, aClipWidth, aCelHeight);
	g->DrawImage(Sexy::IMAGE_FLAGMETER, aDstRect, aSrcRect);
	
	// ====================================================================================================
	// ▲ 根据不同关卡，绘制进度条上的文字或旗帜
	// ====================================================================================================
	int aPosX = aCelWidth / 2 + 600;
	Color aColor(224, 187, 98);
	// @Patoke: updated these
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST)
	{
		std::string aMatchStr = StrFormat("%d/%d %s", mChallenge->mChallengeScore, 75, TodStringTranslate("[MATCHES]").c_str());
		TodDrawString(g, aMatchStr, aPosX, 589, Sexy::FONT_DWARVENTODCRAFT12, aColor, DrawStringJustification::DS_ALIGN_CENTER);
	}
	else if (mApp->IsSquirrelLevel())
	{
		std::string aMatchStr = StrFormat("%d/%d %s", mChallenge->mChallengeScore, 7, TodStringTranslate("[SQUIRRELS]").c_str());
		TodDrawString(g, aMatchStr, aPosX, 589, Sexy::FONT_DWARVENTODCRAFT12, aColor, DrawStringJustification::DS_ALIGN_CENTER);
	}
	else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_SLOT_MACHINE)
	{
		int aSunMoney = ClampInt(mSunMoney, 0, 2000);
		std::string aMatchStr = StrFormat("%d/%d %s", aSunMoney, 2000, TodStringTranslate("[SUN]").c_str());
		TodDrawString(g, aMatchStr, aPosX, 589, Sexy::FONT_DWARVENTODCRAFT12, aColor, DrawStringJustification::DS_ALIGN_CENTER);
	}
	else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM)
	{
		int aSunMoney = ClampInt(mSunMoney, 0, 1000);
		std::string aMatchStr = StrFormat("%d/%d %s", aSunMoney, 1000, TodStringTranslate("[SUN]").c_str());
		TodDrawString(g, aMatchStr, aPosX, 589, Sexy::FONT_DWARVENTODCRAFT12, aColor, DrawStringJustification::DS_ALIGN_CENTER);
	}
	else if (mApp->IsIZombieLevel())
	{
		std::string aMatchStr = StrFormat("%d/%d %s", mChallenge->mChallengeScore, 5, TodStringTranslate("[BRAINS]").c_str());
		TodDrawString(g, aMatchStr, aPosX, 589, Sexy::FONT_DWARVENTODCRAFT12, aColor, DrawStringJustification::DS_ALIGN_CENTER);
	}
	else if (ProgressMeterHasFlags())
	{
		int aNumWavesPerFlag = GetNumWavesPerFlag();
		int aNumFlagWaves = mNumWaves / aNumWavesPerFlag;
		int aFlagsPosEnd = 590 + aCelWidth;  // 旗帜区域的右界横坐标
		for (int aFlagWave = 1; aFlagWave <= aNumFlagWaves; aFlagWave++)
		{
			// 取得旗帜升起时的高度偏移
			int aHeight = 0;
			int aTotalWavesAtFlag = aFlagWave * aNumWavesPerFlag;
			if (aTotalWavesAtFlag < mCurrentWave)
			{
				aHeight = 14;
			}
			else if (aTotalWavesAtFlag == mCurrentWave)
			{
				aHeight = TodAnimateCurve(100, 0, mFlagRaiseCounter, 0, 14, TodCurves::CURVE_LINEAR);
			}
			// 计算旗帜的横坐标
			int aPosX = TodAnimateCurve(0, mNumWaves, aTotalWavesAtFlag, aFlagsPosEnd, 606, TodCurves::CURVE_LINEAR);
			// 绘制旗杆
			g->DrawImageCel(Sexy::IMAGE_FLAGMETERPARTS, aPosX, 571, 1, 0);
			// 绘制旗帜
			g->DrawImageCel(Sexy::IMAGE_FLAGMETERPARTS, aPosX, 572 - aHeight, 2, 0);
		}
	}

	// ====================================================================================================
	// ▲ 绘制进度条的额外部分
	// ====================================================================================================
	// 绘制“关卡进程”的小牌子
	g->DrawImage(Sexy::IMAGE_FLAGMETERLEVELPROGRESS, 638, 589);
	// 判断是否需要绘制进度条当前位置处的小僵尸头，不需要则直接返回
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED || 
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST ||
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM || 
		mApp->IsSquirrelLevel() || 
		mApp->IsSlotMachineLevel() ||
		mApp->IsIZombieLevel() || 
		mApp->IsFinalBossLevel())
		return;
	// 绘制僵尸头
	int aHeadProgress = TodAnimateCurve(0, 150, mProgressMeterWidth, 0, 135, CURVE_LINEAR);
	g->DrawImageCel(Sexy::IMAGE_FLAGMETERPARTS, aCelWidth - aHeadProgress + 580, 572, 0, 0);
}

void Board::DrawHouseDoorBottom(Graphics* g)
{
	switch (mBackground)
	{
	case BackgroundType::BACKGROUND_1_DAY:		g->DrawImage(Sexy::IMAGE_BACKGROUND1_GAMEOVER_INTERIOR_OVERLAY, -126, 225);		break;
	case BackgroundType::BACKGROUND_2_NIGHT:	g->DrawImage(Sexy::IMAGE_BACKGROUND2_GAMEOVER_INTERIOR_OVERLAY, -125, 196);		break;
	case BackgroundType::BACKGROUND_3_POOL:		g->DrawImage(Sexy::IMAGE_BACKGROUND3_GAMEOVER_INTERIOR_OVERLAY, -171, 241);		break;
	case BackgroundType::BACKGROUND_4_FOG:		g->DrawImage(Sexy::IMAGE_BACKGROUND4_GAMEOVER_INTERIOR_OVERLAY, -172, 246);		break;
	default:																													break;
	}
}

void Board::DrawHouseDoorTop(Graphics* g)
{
	switch (mBackground)
	{
	case BackgroundType::BACKGROUND_1_DAY:		g->DrawImage(Sexy::IMAGE_BACKGROUND1_GAMEOVER_MASK, -130, 202);		break;
	case BackgroundType::BACKGROUND_2_NIGHT:	g->DrawImage(Sexy::IMAGE_BACKGROUND2_GAMEOVER_MASK, -128, 207);		break;
	case BackgroundType::BACKGROUND_3_POOL:		g->DrawImage(Sexy::IMAGE_BACKGROUND3_GAMEOVER_MASK, -172, 234);		break;
	case BackgroundType::BACKGROUND_4_FOG:		g->DrawImage(Sexy::IMAGE_BACKGROUND4_GAMEOVER_MASK, -173, 133);		break;
	case BackgroundType::BACKGROUND_5_ROOF:		g->DrawImage(Sexy::IMAGE_BACKGROUND5_GAMEOVER_MASK, -220, 81);		break;
	case BackgroundType::BACKGROUND_6_BOSS:		g->DrawImage(Sexy::IMAGE_BACKGROUND6_GAMEOVER_MASK, -220, 81);		break;
	default:																										break;
	}
}

// GOTY @Patoke: 0x41A700
void Board::DrawLevel(Graphics* g)
{
	// ====================================================================================================
	// ▲ 获取完整的关卡名称的字符串
	// ====================================================================================================
	std::string aLevelStr;
	if (mApp->IsAdventureMode())
	{
		aLevelStr = TodStringTranslate("[LEVEL]") + " " + mApp->GetStageString(mLevel);
	}
	else
	{
		aLevelStr = mApp->GetCurrentChallengeDef().mChallengeName;
		if (mApp->IsSurvivalMode() || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND)
		{
			int aFlags = GetSurvivalFlagsCompleted();
			if (aFlags > 0)
			{
				std::string aFlagStr = mApp->Pluralize(aFlags, "[ONE_FLAG]", "[COUNT_FLAGS]");
				std::string aCompletedStr = TodReplaceString("[FLAGS_COMPLETED]", "{FLAGS}", aFlagStr);
				aLevelStr = StrFormat("%s - %s", TodStringTranslate(aLevelStr).c_str(), aCompletedStr.c_str());
			}
		}
		else if (mApp->IsEndlessIZombie(mApp->mGameMode) || mApp->IsEndlessScaryPotter(mApp->mGameMode))
		{
			int aStreak = mChallenge->mSurvivalStage;
			if (mNextSurvivalStageCounter > 0)
			{
				aStreak++;
			}
			if (aStreak > 0)
			{
				std::string aStreakStr = TodReplaceNumberString("[ENDLESS_STREAK]", "{STREAK}", aStreak);
				aLevelStr = StrFormat("%s - %s", TodStringTranslate(aLevelStr).c_str(), aStreakStr.c_str());
			}
		}
	}
	
	// ====================================================================================================
	// ▲ 正式开始绘制关卡名称字符串
	// ====================================================================================================
	int aPosX = 780;
	int aPosY = 595;
	if (HasProgressMeter())
	{
		aPosX = 593;
	}
	if (mChallenge->mChallengeState == ChallengeState::STATECHALLENGE_ZEN_FADING)
	{
		aPosY += TodAnimateCurve(50, 0, mChallenge->mChallengeStateCounter, 0, 50, TodCurves::CURVE_EASE_IN_OUT);
	}
	TodDrawString(g, aLevelStr, aPosX, aPosY, Sexy::FONT_HOUSEOFTERROR16, Color(224, 187, 98), DrawStringJustification::DS_ALIGN_RIGHT);
}

void Board::DrawZenWheelBarrowButton(Graphics* g, int theOffsetY)
{
	Rect aButtonRect = GetShovelButtonRect();
	GetZenButtonRect(GameObjectType::OBJECT_TYPE_WHEELBARROW, aButtonRect);
	PottedPlant* aPlant = mApp->mZenGarden->GetPottedPlantInWheelbarrow();
	if (aPlant && mCursorObject->mCursorType != CursorType::CURSOR_TYPE_PLANT_FROM_WHEEL_BARROW)
	{
		if (mChallenge->mChallengeState == ChallengeState::STATECHALLENGE_ZEN_FADING)
		{
			g->DrawImage(Sexy::IMAGE_ZEN_WHEELBARROW, aButtonRect.mX - 7, aButtonRect.mY + theOffsetY - 3);
		}
		else
		{
			g->DrawImage(Sexy::IMAGE_ZEN_WHEELBARROW, aButtonRect.mX - 7, aButtonRect.mY + theOffsetY + 4);
		}

		if (aPlant->mPlantAge == PottedPlantAge::PLANTAGE_SMALL)
		{
			mApp->mZenGarden->DrawPottedPlant(g, aButtonRect.mX + 23, aButtonRect.mY + theOffsetY - 8, aPlant, 0.6f, true);
		}
		else if (aPlant->mPlantAge == PottedPlantAge::PLANTAGE_MEDIUM)
		{
			mApp->mZenGarden->DrawPottedPlant(g, aButtonRect.mX + 28, aButtonRect.mY + theOffsetY + 2, aPlant, 0.5f, true);
		}
		else
		{
			mApp->mZenGarden->DrawPottedPlant(g, aButtonRect.mX + 34, aButtonRect.mY + theOffsetY + 12, aPlant, 0.4f, true);
		}
	}
	else
	{
		g->DrawImage(Sexy::IMAGE_ZEN_WHEELBARROW, aButtonRect.mX - 7, aButtonRect.mY + theOffsetY - 3);
	}
}

void Board::DrawZenButtons(Graphics* g)
{
	int aOffsetY = 0;
	if (mChallenge->mChallengeState == ChallengeState::STATECHALLENGE_ZEN_FADING)
	{
		aOffsetY = TodAnimateCurve(50, 0, mChallenge->mChallengeStateCounter, 0, -72, TodCurves::CURVE_EASE_IN_OUT);
	}

	for (GameObjectType aTool = GameObjectType::OBJECT_TYPE_WATERING_CAN; aTool <= GameObjectType::OBJECT_TYPE_NEXT_GARDEN; aTool = (GameObjectType)(aTool + 1))
	{
		if (!CanUseGameObject(aTool))
			continue;

		Rect aButtonRect = GetShovelButtonRect();
		if (aTool == GameObjectType::OBJECT_TYPE_NEXT_GARDEN)
		{
			aButtonRect.mX = 564;
			if (!mMenuButton->mBtnNoDraw)
			{
				g->DrawImage(Sexy::IMAGE_ZEN_NEXTGARDEN, aButtonRect.mX + 2, aButtonRect.mY + aOffsetY);
			}
		}
		else
		{
			GetZenButtonRect(aTool, aButtonRect);
			g->DrawImage(Sexy::IMAGE_SHOVELBANK, aButtonRect.mX, aButtonRect.mY + aOffsetY);
			if (static_cast<int>(mCursorObject->mCursorType) == static_cast<int>(CursorType::CURSOR_TYPE_WATERING_CAN) + static_cast<int>(aTool) - 6)
			{
				continue;  // 如果工具正在被手持，则跳过绘制
			}

			switch (aTool)
			{
			case GameObjectType::OBJECT_TYPE_WATERING_CAN:
				if (mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_GOLD_WATERINGCAN])
				{
					g->DrawImage(Sexy::IMAGE_WATERINGCANGOLD, aButtonRect.mX - 2, aButtonRect.mY + aOffsetY - 6);
				}
				else
				{
					g->DrawImage(Sexy::IMAGE_WATERINGCAN, aButtonRect.mX - 2, aButtonRect.mY + aOffsetY - 6);
				}
				break;
			case GameObjectType::OBJECT_TYPE_FERTILIZER:
			{
				uint32_t aPurchase = mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_FERTILIZER];
				int aCharges = aPurchase > PURCHASE_COUNT_OFFSET ? aPurchase - PURCHASE_COUNT_OFFSET : 0;
				if (aCharges == 0)
				{
					g->SetColorizeImages(true);
					g->SetColor(Color(96, 96, 96));
				}
				else if (mTutorialState == TutorialState::TUTORIAL_ZEN_GARDEN_FERTILIZE_PLANTS)
				{
					g->SetColorizeImages(true);
					g->SetColor(GetFlashingColor(mMainCounter, 75));
				}
				g->DrawImage(Sexy::IMAGE_FERTILIZER, aButtonRect.mX - 6, aButtonRect.mY + aOffsetY - 7);
				g->SetColorizeImages(false);

				std::string aChargeString = StrFormat("x%d", aCharges);
				TodDrawString(g, aChargeString, aButtonRect.mX + 64, aButtonRect.mY + aOffsetY + 65, Sexy::FONT_HOUSEOFTERROR16, Color::White, DS_ALIGN_RIGHT);
				break;
			}
			case GameObjectType::OBJECT_TYPE_BUG_SPRAY:
			{
				uint32_t aPurchase = mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_BUG_SPRAY];
				int aCharges = aPurchase > PURCHASE_COUNT_OFFSET ? aPurchase - PURCHASE_COUNT_OFFSET : 0;
				if (aCharges == 0)
				{
					g->SetColorizeImages(true);
					g->SetColor(Color(128, 128, 128));
				}
				g->DrawImage(Sexy::IMAGE_BUG_SPRAY, aButtonRect.mX, aButtonRect.mY + aOffsetY - 1);
				g->SetColorizeImages(false);

				std::string aChargeString = StrFormat("x%d", aCharges);
				TodDrawString(g, aChargeString, aButtonRect.mX + 64, aButtonRect.mY + aOffsetY + 65, Sexy::FONT_HOUSEOFTERROR16, Color::White, DS_ALIGN_RIGHT);
				break;
			}
			case GameObjectType::OBJECT_TYPE_PHONOGRAPH:
				g->DrawImage(Sexy::IMAGE_PHONOGRAPH, aButtonRect.mX + 2, aButtonRect.mY + aOffsetY + 2);
				break;
			case GameObjectType::OBJECT_TYPE_CHOCOLATE:
			{
				uint32_t aPurchase = mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_CHOCOLATE];
				int aCharges = aPurchase > PURCHASE_COUNT_OFFSET ? aPurchase - PURCHASE_COUNT_OFFSET : 0;
				if (aCharges == 0)
				{
					g->SetColorizeImages(true);
					g->SetColor(Color(128, 128, 128));
				}
				g->DrawImage(Sexy::IMAGE_CHOCOLATE, aButtonRect.mX + 6, aButtonRect.mY + aOffsetY + 4);
				g->SetColorizeImages(false);

				std::string aChargeString = StrFormat("x%d", aCharges);
				TodDrawString(g, aChargeString, aButtonRect.mX + 64, aButtonRect.mY + aOffsetY + 65, Sexy::FONT_HOUSEOFTERROR16, Color::White, DS_ALIGN_RIGHT);
				break;
			}
			case GameObjectType::OBJECT_TYPE_GLOVE:
				if (mCursorObject->mCursorType != CursorType::CURSOR_TYPE_PLANT_FROM_GLOVE && 
					mCursorObject->mCursorType != CursorType::CURSOR_TYPE_PLANT_FROM_WHEEL_BARROW)
				{
					g->DrawImage(Sexy::IMAGE_ZEN_GARDENGLOVE, aButtonRect.mX - 6, aButtonRect.mY + aOffsetY - 4);
				}
				break;
			case GameObjectType::OBJECT_TYPE_MONEY_SIGN:
				g->DrawImage(Sexy::IMAGE_ZEN_MONEYSIGN, aButtonRect.mX - 5, aButtonRect.mY + aOffsetY - 4);
				break;
			case GameObjectType::OBJECT_TYPE_WHEELBARROW:
				DrawZenWheelBarrowButton(g, aOffsetY);
				break;
			case GameObjectType::OBJECT_TYPE_TREE_FOOD:
			{
				uint32_t aPurchase = mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_TREE_FOOD];
				int aCharges = aPurchase > PURCHASE_COUNT_OFFSET ? aPurchase - PURCHASE_COUNT_OFFSET : 0;
				if (aCharges == 0)
				{
					g->SetColorizeImages(true);
					g->SetColor(Color(128, 128, 128));
				}
				if (!mChallenge->TreeOfWisdomCanFeed())
				{
					g->SetColorizeImages(true);
					g->SetColor(Color(128, 128, 128));
				}
				g->DrawImage(Sexy::IMAGE_TREEFOOD, aButtonRect.mX - 6, aButtonRect.mY + aOffsetY - 7);
				g->SetColorizeImages(false);

				std::string aChargeString = StrFormat("x%d", aCharges);
				TodDrawString(g, aChargeString, aButtonRect.mX + 64, aButtonRect.mY + aOffsetY + 65, Sexy::FONT_HOUSEOFTERROR16, Color::White, DS_ALIGN_RIGHT);
				break;
			}
			default:
				break;
			}
		}
	}
}

void Board::DrawShovel(Graphics* g)
{
	if (mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN && mApp->mGameMode != GameMode::GAMEMODE_TREE_OF_WISDOM)
	{
		if (mShowShovel)
		{
			Rect aShovelRect = GetShovelButtonRect();
			g->DrawImage(Sexy::IMAGE_SHOVELBANK, aShovelRect.mX, aShovelRect.mY);

			if (mCursorObject->mCursorType != CursorType::CURSOR_TYPE_SHOVEL)
			{
				if (mChallenge->mChallengeState == (ChallengeState)15)
				{
					g->SetColorizeImages(true);
					g->SetColor(GetFlashingColor(mMainCounter, 75));
				}
				g->DrawImage(Sexy::IMAGE_SHOVEL, aShovelRect.mX - 7, aShovelRect.mY - 3);
				g->SetColorizeImages(false);
			}
		}
	}

	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN || mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM)
	{
		DrawZenButtons(g);
	}
}

void Board::DrawDebugText(Graphics* g)
{
	std::string aText;

	switch (mDebugTextMode)
	{
	case DebugTextMode::DEBUG_TEXT_NONE:
		break;

	case DebugTextMode::DEBUG_TEXT_ZOMBIE_SPAWN:
	{
		int aTime = mZombieCountDownStart - mZombieCountDown;
		float aCountDownFraction = static_cast<float>(aTime) / static_cast<float>(mZombieCountDownStart);

		aText += StrFormat("ZOMBIE SPAWNING DEBUG\n");
		aText += StrFormat("CurrentWave: %d of %d\n", mCurrentWave, mNumWaves);
		aText += StrFormat("TimeSinseLastSpawn: %d %s\n", aTime, aTime > 400 ? "" : "(too soon)");
		aText += StrFormat("ZombieCountDown: %d/%d (%.0f%%)\n", mZombieCountDown, mZombieCountDownStart, aCountDownFraction);

		if (mZombieHealthToNextWave != -1)
		{
			int aTotalHealth = TotalZombiesHealthInWave(mCurrentWave - 1);
			int aHealthRange = std::max(mZombieHealthWaveStart - mZombieHealthToNextWave, 1);
			float aHealthFraction = static_cast<float>(mZombieHealthToNextWave - aTotalHealth + aHealthRange) / static_cast<float>(aHealthRange);
			aText += StrFormat("ZombieHealth: CurZombieHealth %d trigger %d (%.0f%%)\n", aTotalHealth, mZombieHealthToNextWave, aHealthFraction * 100);
		}
		else
		{
			aText += StrFormat("ZombieHealth: before first wave\n");
		}

		if (mHugeWaveCountDown > 0)
		{
			aText += StrFormat("HugeWaveCountDown: %d\n", mHugeWaveCountDown);
		}

		Zombie* aBossZombie = GetBossZombie();
		if (aBossZombie)
		{
			aText += StrFormat("\nSpawn: %d\n", aBossZombie->mSummonCounter);
			aText += StrFormat("Stomp: %d\n", aBossZombie->mBossStompCounter);
			aText += StrFormat("Bungee: %d\n", aBossZombie->mBossBungeeCounter);
			aText += StrFormat("Head: %d\n", aBossZombie->mBossHeadCounter);
			aText += StrFormat("Health: %d of %d\n", aBossZombie->mBodyHealth, aBossZombie->mBodyMaxHealth);
		}

		break;
	}

	case DebugTextMode::DEBUG_TEXT_MUSIC:
	{
		aText += StrFormat("MUSIC DEBUG\n");
		aText += StrFormat("CurrentWave: %d of %d\n", mCurrentWave, mNumWaves);

		if (mApp->mMusic->mCurMusicFileMain == MusicFile::MUSIC_FILE_NONE)
		{
			aText += StrFormat("No music");
		}
		else
		{
			aText += StrFormat("Music Burst: ");

			if (mApp->mMusic->mMusicBurstState == MusicBurstState::MUSIC_BURST_OFF)
			{
				aText += StrFormat("Off");
			}
			else if (mApp->mMusic->mMusicBurstState == MusicBurstState::MUSIC_BURST_STARTING)
			{
				aText += StrFormat("Starting %d/%d", mApp->mMusic->mBurstStateCounter, 400);
			}
			else if (mApp->mMusic->mMusicBurstState == MusicBurstState::MUSIC_BURST_ON)
			{
				aText += StrFormat("On at least until %d/%d", mApp->mMusic->mBurstStateCounter, 800);
			}
			else if (mApp->mMusic->mMusicBurstState == MusicBurstState::MUSIC_BURST_FINISHING)
			{
				aText += StrFormat("Finishing %d/%d", mApp->mMusic->mBurstStateCounter, 400);
			}

			if (mApp->mMusic->mMusicDrumsState == MusicDrumsState::MUSIC_DRUMS_OFF)
			{
				aText += StrFormat(", Drums off");
			}
			else if (mApp->mMusic->mMusicDrumsState == MusicDrumsState::MUSIC_DRUMS_ON_QUEUED)
			{
				aText += StrFormat(", Drums queued on");
			}
			else if (mApp->mMusic->mMusicDrumsState == MusicDrumsState::MUSIC_DRUMS_ON)
			{
				aText += StrFormat(", Drums on");
			}
			else if (mApp->mMusic->mMusicDrumsState == MusicDrumsState::MUSIC_DRUMS_OFF_QUEUED)
			{
				aText += StrFormat(", Drums queued off");
			}
			else if (mApp->mMusic->mMusicDrumsState == MusicDrumsState::MUSIC_DRUMS_FADING)
			{
				aText += StrFormat(", Drums fading off %d/%d", mApp->mMusic->mDrumsStateCounter, 50);
			}
			aText += StrFormat("\n");

			/*
			int aPackedOrderMain = mApp->mMusic->GetMusicOrder(mApp->mMusic->mCurMusicFileMain);
			int aCurrentOrder = LOWORD(aPackedOrderMain);
			aText += StrFormat("Music order %02d row %02d\n", LOWORD(aPackedOrderMain), HIWORD(aPackedOrderMain) / 4);
			if (mApp->mMusic->mCurMusicTune == MusicTune::MUSIC_TUNE_DAY_GRASSWALK ||
				mApp->mMusic->mCurMusicTune == MusicTune::MUSIC_TUNE_POOL_WATERYGRAVES ||
				mApp->mMusic->mCurMusicTune == MusicTune::MUSIC_TUNE_FOG_RIGORMORMIST ||
				mApp->mMusic->mCurMusicTune == MusicTune::MUSIC_TUNE_ROOF_GRAZETHEROOF)
			{
				int aPackedOrderHihats = mApp->mMusic->GetMusicOrder(mApp->mMusic->mCurMusicFileHihats);
				int aPackedOrderDrums = mApp->mMusic->GetMusicOrder(mApp->mMusic->mCurMusicFileDrums);
				if (aCurrentOrder == LOWORD(aPackedOrderHihats) && aCurrentOrder == LOWORD(aPackedOrderDrums))
				{
					int aDiffHihats = HIWORD(aPackedOrderHihats) - HIWORD(aPackedOrderMain);
					int aDiffDrums = HIWORD(aPackedOrderDrums) - HIWORD(aPackedOrderMain);
					if (abs(aDiffHihats) > 1 || abs(aDiffDrums) > 1)
					{
						aText += StrFormat("Music unsynced hihats %d drums %d\n", aDiffHihats, aDiffDrums);
					}
				}

				HMUSIC aMusicHandle1 = mApp->mMusic->GetBassMusicHandle(mApp->mMusic->mCurMusicFileMain);
				HMUSIC aMusicHandle2 = mApp->mMusic->GetBassMusicHandle(mApp->mMusic->mCurMusicFileHihats);
				HMUSIC aMusicHandle3 = mApp->mMusic->GetBassMusicHandle(mApp->mMusic->mCurMusicFileDrums);
				float bpm1;
				float bpm2;
				float bpm3;
				gBass->BASS_ChannelGetAttribute(aMusicHandle1, BASS_ATTRIB_MUSIC_BPM, &bpm1);
				gBass->BASS_ChannelGetAttribute(aMusicHandle2, BASS_ATTRIB_MUSIC_BPM, &bpm2);
				gBass->BASS_ChannelGetAttribute(aMusicHandle3, BASS_ATTRIB_MUSIC_BPM, &bpm3);
				aText += StrFormat("bpm1 %f bmp2 %f bpm3 %f\n", bpm1, bpm2, bpm3);
			}
			else if (mApp->mMusic->mCurMusicTune == MusicTune::MUSIC_TUNE_NIGHT_MOONGRAINS)
			{
				int aPackedOrderDrums = mApp->mMusic->GetMusicOrder(mApp->mMusic->mCurMusicFileDrums);
				aText += StrFormat("Drum order %02d row %02d\n", LOWORD(aPackedOrderDrums), HIWORD(aPackedOrderDrums) / 4);
				int aDiffDrums = HIWORD(aPackedOrderDrums) - HIWORD(aPackedOrderMain);
				if (abs(aDiffDrums) > 0 && abs(aDiffDrums) <= 128)
				{
					aText += StrFormat("Drums unsynced %d", aDiffDrums);
				}
			}
			*/
		}

		break;
	}

	case DebugTextMode::DEBUG_TEXT_MEMORY:
		aText += StrFormat("MEMORY DEBUG\n");
		aText += StrFormat("attachments %d\n", mApp->mEffectSystem->mAttachmentHolder->mAttachments.mSize);
		aText += StrFormat("emitters %d\n", mApp->mEffectSystem->mParticleHolder->mEmitters.mSize);
		aText += StrFormat("particles %d\n", mApp->mEffectSystem->mParticleHolder->mParticles.mSize);
		aText += StrFormat("particle systems %d\n", mApp->mEffectSystem->mParticleHolder->mParticleSystems.mSize);
		aText += StrFormat("trails %d\n", mApp->mEffectSystem->mTrailHolder->mTrails.mSize);
		aText += StrFormat("reanimation %d\n", mApp->mEffectSystem->mReanimationHolder->mReanimations.mSize);
		aText += StrFormat("zombies %d\n", mZombies.mSize);
		aText += StrFormat("plants %d\n", mPlants.mSize);
		aText += StrFormat("projectiles %d\n", mProjectiles.mSize);
		aText += StrFormat("coins %d\n", mCoins.mSize);
		aText += StrFormat("lawn mowers %d\n", mLawnMowers.mSize);
		aText += StrFormat("grid items %d\n", mGridItems.mSize);
		break;

	case DebugTextMode::DEBUG_TEXT_COLLISION:
		aText += StrFormat("COLLISION DEBUG\n");
		break;

	default:
		TOD_ASSERT(false);
		break;
	}

	g->SetFont(FONT_PICO129);
	g->SetColor(Color::Black);
	g->DrawStringWordWrapped(aText, 10, 89);
	g->DrawStringWordWrapped(aText, 11, 91);
	g->DrawStringWordWrapped(aText, 9, 90);
	g->DrawStringWordWrapped(aText, 11, 90);
	g->SetColor(Color(255, 255, 255));
	g->DrawStringWordWrapped(aText, 10, 90);
}

void Board::DrawDebugObjectRects(Graphics* g)
{
	if (mDebugTextMode != DebugTextMode::DEBUG_TEXT_COLLISION)
		return;

	{
		Plant* aPlant = nullptr;
		while (IteratePlants(aPlant))
		{
			Rect aRect = aPlant->GetPlantRect();
			g->SetColor(Color(0, 255, 0));
			g->DrawRect(aRect);

			Rect aAttackRect = aPlant->GetPlantAttackRect(PlantWeapon::WEAPON_PRIMARY);
			if (aAttackRect.mWidth < BOARD_WIDTH)
			{
				g->SetColor(Color(255, 0, 0));
				g->DrawRect(aAttackRect);
			}

			Rect aSecondaryRect = aPlant->GetPlantAttackRect(PlantWeapon::WEAPON_SECONDARY);
			if (aSecondaryRect.mWidth < BOARD_WIDTH)
			{
				g->SetColor(Color(255, 0, 128));
				g->DrawRect(aSecondaryRect);
			}
		}
	}
	{
		Zombie* aZombie = nullptr;
		while (IterateZombies(aZombie))
		{
			if (!aZombie->IsDeadOrDying())
			{
				Rect aRect = aZombie->GetZombieRect();
				g->SetColor(Color(0, 255, 0));
				g->DrawRect(aRect);

				Rect aAttackRect = aZombie->GetZombieAttackRect();
				g->SetColor(Color(255, 0, 0));
				g->DrawRect(aAttackRect);
			}
		}
	}
	{
		LawnMower* aLawnMower = nullptr;
		while (IterateLawnMowers(aLawnMower))
		{
			Rect aAttackRect = aLawnMower->GetLawnMowerAttackRect();
			g->SetColor(Color(255, 0, 0));
			g->DrawRect(aAttackRect);
		}
	}
	{
		Projectile* aProjectile = nullptr;
		while (IterateProjectiles(aProjectile))
		{
			g->SetColor(Color(255, 0, 0));
			Rect aDamageRect = aProjectile->GetProjectileRect();
			g->DrawRect(aDamageRect);
		}
	}
}

void Board::DrawFadeOut(Graphics* g)
{
	if (mBoardFadeOutCounter < 0 || IsSurvivalStageWithRepick())
		return;

	int anAlpha = TodAnimateCurve(200, 0, mBoardFadeOutCounter, 0, 255, TodCurves::CURVE_LINEAR);
	if (mLevel == 9 || mLevel == 19 || mLevel == 29 || mLevel == 39 || mLevel == 49)
	{
		g->SetColor(Color(0, 0, 0, anAlpha));
	}
	else
	{
		g->SetColor(Color(255, 255, 255, anAlpha));
	}
	g->FillRect(0, 0, mWidth, mHeight);
}

void Board::DrawTopRightUI(Graphics* g)
{
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		if (mChallenge->mChallengeState == STATECHALLENGE_ZEN_FADING)
		{
			mMenuButton->mY = TodAnimateCurve(50, 0, mChallenge->mChallengeStateCounter, -10, -50, TodCurves::CURVE_EASE_IN_OUT);
			mStoreButton->mX = TodAnimateCurve(50, 0, mChallenge->mChallengeStateCounter, 678, 800, TodCurves::CURVE_EASE_IN_OUT);
		}
		else
		{
			mMenuButton->mY = -10;
			mStoreButton->mX = 678;
		}
	}

	if (mTutorialState == TutorialState::TUTORIAL_ZEN_GARDEN_COMPLETED)
	{
		g->SetColorizeImages(true);
		g->SetColor(GetFlashingColor(mMainCounter, 75));
	}
	mMenuButton->Draw(g);
	g->SetColorizeImages(false);

	if (mStoreButton && mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_LAST_STAND)
	{
		if (mTutorialState == TutorialState::TUTORIAL_ZEN_GARDEN_VISIT_STORE)
		{
			g->SetColorizeImages(true);
			g->SetColor(GetFlashingColor(mMainCounter, 75));
		}
		mStoreButton->Draw(g);
		g->SetColorizeImages(false);
	}
}

void Board::DrawUIBottom(Graphics* g)
{
	if (mBackground == BackgroundType::BACKGROUND_ZOMBIQUARIUM)
	{
		int aWaveTime = std::abs(static_cast<int>((mMainCounter / 8) % 22) - 11);
		g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
		g->DrawImageCel(Sexy::IMAGE_WAVESIDE, 0, 40, aWaveTime);
		g->DrawImageCel(Sexy::IMAGE_WAVECENTER, 160, 40, aWaveTime);
		g->DrawImageCel(Sexy::IMAGE_WAVECENTER, 320, 40, aWaveTime);
		g->DrawImageCel(Sexy::IMAGE_WAVECENTER, 480, 40, aWaveTime);
		//TodDrawImageCelScaled(g, Sexy::IMAGE_WAVESIDE, 800, 40, 0, aWaveTime, -1.0f, 1.0f);
		TodDrawImageCelScaled(
			g, Sexy::IMAGE_WAVESIDE, 800, 40, aWaveTime % Sexy::IMAGE_WAVESIDE->mNumCols, 
			aWaveTime / Sexy::IMAGE_WAVESIDE->mNumCols, -1.0f, 1.0f
		);	
		g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
	}

	if (mBackground == BackgroundType::BACKGROUND_GREENHOUSE || mBackground == BackgroundType::BACKGROUND_ZOMBIQUARIUM)
	{
		g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
		g->DrawImage(
			IMAGE_BACKGROUND_GREENHOUSE_OVERLAY, 
			Rect(0, 0, BOARD_WIDTH, BOARD_HEIGHT), 
			Rect(0, 0, IMAGE_BACKGROUND_GREENHOUSE_OVERLAY->mWidth, IMAGE_BACKGROUND_GREENHOUSE_OVERLAY->mHeight)
		);
		g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
	}

	if (mApp->mGameScene != GameScenes::SCENE_ZOMBIES_WON)
	{
		if (mSeedBank->BeginDraw(g))
		{
			mSeedBank->Draw(g);
			mSeedBank->EndDraw(g);
		}

		if (mAdvice->mMessageStyle == MessageStyle::MESSAGE_STYLE_SLOT_MACHINE)
		{
			mAdvice->Draw(g);
		}
	}

	DrawShovel(g);
	if (!StageHasFog())
	{
		DrawTopRightUI(g);
	}
}

void Board::DrawUICoinBank(Graphics* g)
{
	if (mApp->mGameScene != GameScenes::SCENE_PLAYING && mApp->mCrazyDaveState == CrazyDaveState::CRAZY_DAVE_OFF)
		return;

	if (mCoinBankFadeCount <= 0)
		return;

	int aPosX = 57;
	int aPosY = 599 - Sexy::IMAGE_COINBANK->GetHeight();
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN || mApp->mCrazyDaveState != CrazyDaveState::CRAZY_DAVE_OFF)
	{
		aPosX = 450 - mX;
	}

	g->SetColorizeImages(true);
	int anAlpha = ClampInt(255 * mCoinBankFadeCount / 15, 0, 255);
	g->SetColor(Color(255, 255, 255, anAlpha));
	g->DrawImage(Sexy::IMAGE_COINBANK, aPosX, aPosY);

	g->SetColor(Color(180, 255, 90, anAlpha));
	g->SetFont(Sexy::FONT_CONTINUUMBOLD14);
	std::string aCoinLabel = mApp->GetMoneyString(mApp->mPlayerInfo->mCoins);
	g->DrawString(aCoinLabel, aPosX + 116 - Sexy::FONT_CONTINUUMBOLD14->StringWidth(aCoinLabel), aPosY + 24);
	g->SetColorizeImages(false);
}

void Board::ClearFogAroundPlant(Plant* thePlant, int theSize)
{
	//int aFogFadeOutSpeed = mFogBlownCountDown >= 2000 ? 40 : mFogBlownCountDown > 0 ? 2 : 6;
	int aFogFadeOutSpeed = 6;
	if (mFogBlownCountDown > 0 && mFogBlownCountDown < 2000)
	{
		aFogFadeOutSpeed = 2;
	}
	else if (mFogBlownCountDown > 0)
	{
		aFogFadeOutSpeed = 40;
	}

	int aLeft = LeftFogColumn();
	int aFogOffsetX = (mFogOffset + 50) / 100;
	int aStartX = thePlant->mPlantCol - theSize - aFogOffsetX;
	int aEndX = thePlant->mPlantCol + theSize - aFogOffsetX;
	aStartX = std::max(aStartX, aLeft);
	aEndX = std::min(aEndX, MAX_GRID_SIZE_X - 1);

	int aStartY = thePlant->mRow - theSize;
	int aEndY = thePlant->mRow + theSize;
	aStartY = std::max(aStartY, 0);
	aEndY = std::min(aEndY, MAX_GRID_SIZE_Y);

	for (int x = aStartX; x <= aEndX; x++)
	{
		for (int y = aStartY; y <= aEndY; y++)
		{
			int aDistX = abs(x + aFogOffsetX - thePlant->mPlantCol);
			int aDistY = abs(y - thePlant->mRow);
			if (theSize == 4)
			{
				if (aDistX > 3 || aDistY > 2)
				{
					continue;
				}
				if (aDistX + aDistY == 5)
				{
					continue;
				}
			}
			else if (aDistX + aDistY > theSize)
			{
				continue;
			}

			mGridCelFog[x][y] = std::max(mGridCelFog[x][y] - aFogFadeOutSpeed, 0);
		}
	}
}

void Board::UpdateFog()
{
	if (!StageHasFog())
		return;

	//int aFogFadeInSpeed = mFogBlownCountDown >= 2000 ? 20 : mFogBlownCountDown > 0 ? 1 : 3;
	int aFogFadeInSpeed = 3;
	if (mFogBlownCountDown > 0 && mFogBlownCountDown < 2000)
	{
		aFogFadeInSpeed = 1;
	}
	else if (mFogBlownCountDown > 0)
	{
		aFogFadeInSpeed = 20;
	}

	int aLeft = LeftFogColumn();
	for (int x = aLeft; x < MAX_GRID_SIZE_X; x++)
	{
		for (int y = 0; y < MAX_GRID_SIZE_Y + 1; y++)
		{
			int aFogMax = x == aLeft ? 200 : 255;
			mGridCelFog[x][y] = std::min(mGridCelFog[x][y] + aFogFadeInSpeed, aFogMax);
		}
	}

	Plant* aPlant = nullptr;
	while (IteratePlants(aPlant))
	{
		if (aPlant->NotOnGround())
			continue;

		if (aPlant->mSeedType == SeedType::SEED_PLANTERN)
		{
			ClearFogAroundPlant(aPlant, 4);
		}
		else if (aPlant->mSeedType == SeedType::SEED_TORCHWOOD)
		{
			ClearFogAroundPlant(aPlant, 1);
		}
	}
}

void Board::DrawFog(Graphics* g)
{
	Image* aImageFog = mApp->Is3DAccelerated() ? Sexy::IMAGE_FOG : Sexy::IMAGE_FOG_SOFTWARE;
	for (int x = 0; x < MAX_GRID_SIZE_X; x++)
	{
		for (int y = 0; y < MAX_GRID_SIZE_Y + 1; y++)
		{
			int aFadeAmount = mGridCelFog[x][y];
			if (aFadeAmount == 0)
				continue;

			// 取得格子内的雾的形状（第 6 行的雾的形状采用与第 0 行相同）
			// { sub eax,edx } 向前 [y / 6] 列，但 y 超出上限 y - 5 行，故相当于列不变，行 = y % 6；
			int aCelLook = mGridCelLook[x][y % MAX_GRID_SIZE_Y];
			int aCelCol = aCelLook % 8;
			// 本格浓雾横坐标 = 列 * 80 + 浓雾偏移 - 15，纵坐标 = 行 * 85 + 20
			float aPosX = x * 80 + mFogOffset - 15;
			float aPosY = y * 85 + 20;
			// 浓雾动画依赖 900 和 500 两个周期，取最小公倍数 4500 帧后局部取模，避免大数转 float 精度丢失。
			constexpr uint32_t FOG_ANIM_PERIOD = 4500;
			// 开始计算周期变化的颜色，aTime 为根据主计时计算的时间
			float aTime = static_cast<float>(mMainCounter % FOG_ANIM_PERIOD) * PI * 2;
			// 与行、列有关的初始相位
			float aPhaseX = 6 * PI * x / MAX_GRID_SIZE_X;
			float aPhaseY = 6 * PI * y / (MAX_GRID_SIZE_Y + 1);
			// 根据初相和时间计算当前相位
			float aMotion = 13 + 4 * sin(aTime / 900 + aPhaseY) + 8 * sin(aTime / 500 + aPhaseX);

			int aColorVariant = 255 - aCelLook * 1.5 - aMotion * 1.5;
			int aLightnessVariant = 255 - aCelLook - aMotion;
			if (!mApp->Is3DAccelerated())
			{
				aPosX += 10;
				aPosY += 3;
				aCelCol = aCelLook % Sexy::IMAGE_FOG_SOFTWARE->mNumCols;
				aColorVariant = 255;
				aLightnessVariant = 255;
			}

			g->SetColorizeImages(true);
			g->SetColor(Color(aColorVariant, aColorVariant, aLightnessVariant, aFadeAmount));
			g->DrawImageCel(aImageFog, aPosX, aPosY, aCelCol, 0);

			if (x == MAX_GRID_SIZE_X - 1)
			{
				g->DrawImageCel(aImageFog, aPosX + 80, aPosY, aCelCol, 0);
			}
			g->SetColorizeImages(false);
		}
	}
}

bool Board::IsScaryPotterDaveTalking()
{
	return mApp->IsScaryPotterLevel() && mNextSurvivalStageCounter > 0 && mApp->mCrazyDaveState != CrazyDaveState::CRAZY_DAVE_OFF;
}

void Board::DrawUITop(Graphics* g)
{
	if (StageHasFog())
	{
		DrawTopRightUI(g);
	}

	if (mTimeStopCounter > 0)
	{
		g->SetColor(Color(200, 200, 200, 210));
		g->FillRect(0, 0, BOARD_WIDTH, BOARD_HEIGHT);
	}

	if (mApp->mGameScene == GameScenes::SCENE_PLAYING || mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM)
	{
		DrawProgressMeter(g);
		DrawLevel(g);
	}
	if (mStoreButton && mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND)
	{
		mStoreButton->Draw(g);
	}

	if ((mApp->mGameMode == GameMode::GAMEMODE_UPSELL || mApp->mGameMode == GameMode::GAMEMODE_INTRO) && mCutScene->mUpsellHideBoard)
	{
		g->SetColor(Color(0, 0, 0));
		g->FillRect(0, 0, BOARD_WIDTH, BOARD_HEIGHT);
	}

	if (mApp->mGameMode == GameMode::GAMEMODE_UPSELL)
	{
		mCutScene->DrawUpsell(g);
	}
	if (mApp->mGameMode == GameMode::GAMEMODE_INTRO)
	{
		mCutScene->DrawIntro(g);
	}

	if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO || 
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN ||
		mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM || 
		IsScaryPotterDaveTalking())
	{
		Graphics aScreenSpace(*g);
		aScreenSpace.mTransX -= mX;
		aScreenSpace.mTransY -= mY;
		mApp->DrawCrazyDave(&aScreenSpace);
	}

	if (mAdvice->mMessageStyle != MessageStyle::MESSAGE_STYLE_SLOT_MACHINE)
	{
		mAdvice->Draw(g);
	}

	if (mTimeStopCounter == 0 && mCursorObject->BeginDraw(g))
	{
		mCursorObject->Draw(g);
		mCursorObject->EndDraw(g);
	}

	mToolTip->Draw(g);
	DrawDebugText(g);
	DrawDebugObjectRects(g);
}

void Board::Draw(Graphics* g)
{
	if (mApp->GetDialog(Dialogs::DIALOG_STORE) || mApp->GetDialog(Dialogs::DIALOG_ALMANAC))
		return;

	g->SetLinearBlend(true);

	if (mDrawCount && mCutScene->mPreloaded)
	{
		int64_t aTickCount = SDL_GetTicks();
		int64_t aIntervalDraws = mDrawCount - mIntervalDrawCountStart;
		int64_t aInterval = aTickCount - mIntervalDrawTime;
		if (aInterval > 10000)
		{
			float aIntervalFPS = (aIntervalDraws * 1000 + 500) / aInterval;
			if (mMinFPS > aIntervalFPS)
			{
				mMinFPS = aIntervalFPS;
			}
			mIntervalDrawCountStart = mDrawCount;
			mIntervalDrawTime = aTickCount;
		}
	}
	else
	{
		ResetFPSStats();
	}

	mDrawCount++;
	DrawGameObjects(g);
}

// GOTY @Patoke: 0x41D910
void Board::SetMustacheMode(bool theEnableMustache)
{
	mApp->PlayFoley(FoleyType::FOLEY_POLEVAULT);
	mMustacheMode = theEnableMustache;
	mApp->mMustacheMode = theEnableMustache;

	Zombie* aZombie = nullptr;
	while (IterateZombies(aZombie))
	{
		aZombie->EnableMustache(theEnableMustache);
	}
}

void Board::SetFutureMode(bool theEnableFuture)
{
	mApp->PlaySample(Sexy::SOUND_BOING);
	mFutureMode = theEnableFuture;
	mApp->mFutureMode = theEnableFuture;

	Zombie* aZombie = nullptr;
	while (IterateZombies(aZombie))
	{
		aZombie->EnableFuture(theEnableFuture);
	}
}

void Board::SetPinataMode(bool theEnablePinata)
{
	mApp->PlayFoley(FoleyType::FOLEY_JUICY);
	mPinataMode = theEnablePinata;
	mApp->mPinataMode = theEnablePinata;
}

void Board::SetDanceMode(bool theEnableDance)
{
	mApp->PlayFoley(FoleyType::FOLEY_DANCER);
	mDanceMode = theEnableDance;
	mApp->mDanceMode = theEnableDance;

	Zombie* aZombie = nullptr;
	while (IterateZombies(aZombie))
	{
		if (!aZombie->mDead)
		{
			aZombie->EnableDance();
		}
	}
}

void Board::SetSuperMowerMode(bool theEnableSuperMower)
{
	mApp->PlayFoley(FoleyType::FOLEY_ZAMBONI);
	mSuperMowerMode = theEnableSuperMower;
	mApp->mSuperMowerMode = theEnableSuperMower;

	LawnMower* aLawnMower = nullptr;
	while (IterateLawnMowers(aLawnMower))
	{
		aLawnMower->EnableSuperMower(theEnableSuperMower);
	}
}

void Board::SetDaisyMode(bool theEnableDaisy)
{
	mApp->PlaySample(SOUND_LOADINGBAR_FLOWER);
	mDaisyMode = theEnableDaisy;
	mApp->mDaisyMode = theEnableDaisy;
}

void Board::SetSukhbirMode(bool theEnableSukhbir)
{
	mApp->PlaySample(Sexy::SOUND_SUKHBIR);
	mSukhbirMode = theEnableSukhbir;
	mApp->mSukhbirMode = theEnableSukhbir;
}

void Board::DoTypingCheck(KeyCode theKey)
{
	if (mApp->mKonamiCheck->Check(theKey))
	{
		mApp->PlayFoley(FoleyType::FOLEY_DROP);
		return;
	}
	if (mApp->mMustacheCheck->Check(theKey) || mApp->mMoustacheCheck->Check(theKey))
	{
		SetMustacheMode(!mMustacheMode);
		ReportAchievement::GiveAchievement(mApp, MustacheMode, false); // @Patoke: add achievement
		return;
	}
	if (mApp->mSuperMowerCheck->Check(theKey) || mApp->mSuperMowerCheck2->Check(theKey))
	{
		SetSuperMowerMode(!mSuperMowerMode);
		return;
	}
	if (mApp->mFutureCheck->Check(theKey))
	{
		SetFutureMode(!mFutureMode);
		return;
	}
	if (mApp->mPinataCheck->Check(theKey))
	{
		if (mApp->CanDoPinataMode())
		{
			SetPinataMode(!mPinataMode);
			return;
		}
		else
		{
			if (mApp->mGameScene == GameScenes::SCENE_PLAYING)
			{
				DisplayAdvice("[CANT_USE_CODE]", MessageStyle::MESSAGE_STYLE_BIG_MIDDLE_FAST, AdviceType::ADVICE_NONE);
			}
			mApp->PlaySample(Sexy::SOUND_BUZZER);
			return;
		}
	}
	if (mApp->mDanceCheck->Check(theKey))
	{
		if (mApp->CanDoDanceMode())
		{
			SetDanceMode(!mDanceMode);
			return;
		}
		else
		{
			if (mApp->mGameScene == GameScenes::SCENE_PLAYING)
			{
				DisplayAdvice("[CANT_USE_CODE]", MessageStyle::MESSAGE_STYLE_BIG_MIDDLE_FAST, AdviceType::ADVICE_NONE);
			}
			mApp->PlaySample(Sexy::SOUND_BUZZER);
			return;
		}
	}
	if (mApp->mDaisyCheck->Check(theKey))
	{
		if (mApp->CanDoDaisyMode())
		{
			SetDaisyMode(!mDaisyMode);
			return;
		}
		else
		{
			if (mApp->mGameScene == GameScenes::SCENE_PLAYING)
			{
				DisplayAdvice("[CANT_USE_CODE]", MessageStyle::MESSAGE_STYLE_BIG_MIDDLE_FAST, AdviceType::ADVICE_NONE);
			}
			mApp->PlaySample(Sexy::SOUND_BUZZER);
			return;
		}
	}
	if (mApp->mSukhbirCheck->Check(theKey))
	{
		SetSukhbirMode(!mSukhbirMode);
		return;
	}
}

void Board::KeyDown(KeyCode theKey)
{
	DoTypingCheck(theKey);

	if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO && 
		mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN && 
		mApp->mGameMode != GameMode::GAMEMODE_TREE_OF_WISDOM)
	{
		mCutScene->KeyDown(theKey);
	}
	else if (theKey == KeyCode::KEYCODE_RETURN || theKey == KeyCode::KEYCODE_SPACE)
	{
		if (IsScaryPotterDaveTalking() && mApp->mCrazyDaveMessageIndex != -1)
		{
			mChallenge->AdvanceCrazyDaveDialog();
		}
		else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN || mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM)
		{
			mApp->mZenGarden->AdvanceCrazyDaveDialog();
		}
		else if (theKey == KeyCode::KEYCODE_SPACE && mApp->CanPauseNow())
		{
			mApp->PlaySample(Sexy::SOUND_PAUSE);
			mApp->DoPauseDialog();
		}
	}
	else if (theKey == KeyCode::KEYCODE_ESCAPE)
	{
		if (mCursorObject->mCursorType != CursorType::CURSOR_TYPE_NORMAL)
		{
			RefreshSeedPacketFromCursor();
		}
		else if (CanInteractWithBoardButtons() && mApp->mGameScene != GameScenes::SCENE_ZOMBIES_WON)
		{
			mApp->DoNewOptions(false);
		}
	}
}

static void TodCrash()
{
	TOD_ASSERT(false, "Crash%s", "!!!!");
}

void Board::KeyChar(char theChar)
{
	if (!mApp->mDebugKeysEnabled)
		return;

	TodTraceAndLog("Board cheat key '%c'", theChar);

	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		if (theChar == 'm')
		{
			if (!mApp->mZenGarden->IsZenGardenFull(true))
			{
				PottedPlant aPottedPlant;
				aPottedPlant.InitializePottedPlant(SeedType::SEED_MARIGOLD);
				aPottedPlant.mDrawVariation = static_cast<DrawVariation>(RandRangeInt(static_cast<int>(DrawVariation::VARIATION_MARIGOLD_WHITE), static_cast<int>(DrawVariation::VARIATION_MARIGOLD_LIGHT_GREEN)));
				mApp->mZenGarden->AddPottedPlant(&aPottedPlant);
			}
			return;
		}
		
		if (theChar == '+')
		{
			if (!mApp->mZenGarden->IsZenGardenFull(true))
			{
				PottedPlant aPottedPlant;
				aPottedPlant.InitializePottedPlant(mApp->mZenGarden->PickRandomSeedType());
				mApp->mZenGarden->AddPottedPlant(&aPottedPlant);
			}
			return;
		}
		
		if (theChar == 'a')
		{
			if (!mApp->mZenGarden->IsZenGardenFull(true))
			{
				PottedPlant aPottedPlant;
				aPottedPlant.InitializePottedPlant(mApp->mZenGarden->PickRandomSeedType());
				aPottedPlant.mPlantAge = PottedPlantAge::PLANTAGE_FULL;
				mApp->mZenGarden->AddPottedPlant(&aPottedPlant);
			}
			return;
		}
		
		if (theChar == 'f')
		{
			Plant* aPlant = nullptr;
			while (IteratePlants(aPlant))
			{
				if (GetZenToolAt(aPlant->mPlantCol, aPlant->mRow) == nullptr && aPlant->mPottedPlantIndex >= 0)
				{
					PottedPlant* aPottedPlant = mApp->mZenGarden->PottedPlantFromIndex(aPlant->mPottedPlantIndex);
					PottedPlantNeed aNeed = mApp->mZenGarden->GetPlantsNeed(aPottedPlant);
					if (aNeed == PottedPlantNeed::PLANTNEED_WATER)
					{
						aPlant->mHighlighted = true;
						mApp->mZenGarden->MouseDownWithFeedingTool(aPlant->mX, aPlant->mY, CursorType::CURSOR_TYPE_WATERING_CAN);
						return;
					}
					else if (aNeed == PottedPlantNeed::PLANTNEED_FERTILIZER)
					{
						aPlant->mHighlighted = true;
						if (mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_FERTILIZER] <= PURCHASE_COUNT_OFFSET)
						{
							mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_FERTILIZER] = PURCHASE_COUNT_OFFSET + 1;
						}
						mApp->mZenGarden->MouseDownWithFeedingTool(aPlant->mX, aPlant->mY, CursorType::CURSOR_TYPE_FERTILIZER);
						return;
					}
					else if (aNeed == PottedPlantNeed::PLANTNEED_BUGSPRAY)
					{
						aPlant->mHighlighted = true;
						if (mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_BUG_SPRAY] <= PURCHASE_COUNT_OFFSET)
						{
							mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_BUG_SPRAY] = PURCHASE_COUNT_OFFSET + 1;
						}
						mApp->mZenGarden->MouseDownWithFeedingTool(aPlant->mX, aPlant->mY, CursorType::CURSOR_TYPE_BUG_SPRAY);
						return;
					}
					else if (aNeed == PottedPlantNeed::PLANTNEED_PHONOGRAPH)
					{
						aPlant->mHighlighted = true;
						mApp->mZenGarden->MouseDownWithFeedingTool(aPlant->mX, aPlant->mY, CursorType::CURSOR_TYPE_PHONOGRAPH);
						return;
					}
				}
			}
			return;
		}

		if (theChar == 'r')
		{
			Plant* aPlant = nullptr;
			while (IteratePlants(aPlant))
			{
				if (aPlant->mPottedPlantIndex >= 0)
				{
					TOD_ASSERT(aPlant->mPottedPlantIndex < mApp->mPlayerInfo->mNumPottedPlants);
					PottedPlant* aPottedPlant = &mApp->mPlayerInfo->mPottedPlant[aPlant->mPottedPlantIndex];
					mApp->mZenGarden->ResetPlantTimers(aPottedPlant);
				}
			}
			return;
		}

		if (theChar == 's')
		{
			if (mApp->mZenGarden->IsStinkySleeping())
			{
				mApp->mZenGarden->WakeStinky();
			}
			else
			{
				mApp->mZenGarden->ResetStinkyTimers();
			}
			return;
		}

		if (theChar == 'c')
		{
			if (mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_CHOCOLATE] < PURCHASE_COUNT_OFFSET)
			{
				mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_CHOCOLATE] = PURCHASE_COUNT_OFFSET + 1;
			}
			else
			{
				mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_CHOCOLATE]++;
			}
			return;
		}

		if (theChar == ']')
		{
			PottedPlant* aPottedPlant = mApp->mZenGarden->GetPottedPlantInWheelbarrow();
			if (aPottedPlant)
			{
				aPottedPlant->mSeedType = static_cast<SeedType>(static_cast<int>(aPottedPlant->mSeedType) + 1);
				if (aPottedPlant->mSeedType == SeedType::SEED_GATLINGPEA)
				{
					aPottedPlant->mSeedType = SeedType::SEED_PEASHOOTER;
				}
				if (aPottedPlant->mSeedType == SeedType::SEED_FLOWERPOT)
				{
					aPottedPlant->mSeedType = SeedType::SEED_KERNELPULT;
				}
			}
			return;
		}
	}

	if (mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM)
	{
		if (theChar == 'f')
		{
			if (mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_TREE_FOOD] <= PURCHASE_COUNT_OFFSET)
			{
				mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_TREE_FOOD] = PURCHASE_COUNT_OFFSET + 1;
			}
			mChallenge->TreeOfWisdomFertilize();
		}
		else if (theChar == 'g')
		{
			mChallenge->TreeOfWisdomGrow();
		}
		else if (theChar == 'b')
		{
			mChallenge->mChallengeStateCounter = 1;
		}
		else if (theChar == '0')
		{
			mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()] = 0;
			mChallenge->TreeOfWisdomGrow();
		}
		else if (theChar == '1')
		{
			mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()] = 9;
			mChallenge->TreeOfWisdomGrow();
		}
		else if (theChar == '2')
		{
			mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()] = 19;
			mChallenge->TreeOfWisdomGrow();
		}
		else if (theChar == '3')
		{
			mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()] = 29;
			mChallenge->TreeOfWisdomGrow();
		}
		else if (theChar == '4')
		{
			mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()] = 39;
			mChallenge->TreeOfWisdomGrow();
		}
		else if (theChar == '5')
		{
			mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()] = 49;
			mChallenge->TreeOfWisdomGrow();
		}
		else if (theChar == '6')
		{
			mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()] = 98;
			mChallenge->TreeOfWisdomGrow();
		}
		else if (theChar == '7')
		{
			mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()] = 498;
			mChallenge->TreeOfWisdomGrow();
		}
		else if (theChar == '8')
		{
			mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()] = 998;
			mChallenge->TreeOfWisdomGrow();
		}

		return;
	}

	if (theChar == '<')
	{
		mApp->DoNewOptions(false);
	}
	else if (theChar == 'l')
	{
		mApp->DoCheatDialog();
	}
	else if (theChar == '#')
	{
		if (mApp->IsSurvivalMode())
		{
			if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO)
			{
				return;
			}

			mCurrentWave = mNumWaves;
			mChallenge->mSurvivalStage += 5;
			RemoveAllZombies();
			FadeOutLevel();
		}
	}
	else if (theChar == '!')
	{
		mApp->mBoardResult = BoardResult::BOARDRESULT_CHEAT;
		if (IsLastStandStageWithRepick())
		{
			if (mNextSurvivalStageCounter == 0)
			{
				mCurrentWave = mNumWaves;
				RemoveAllZombies();
				FadeOutLevel();
			}
		}
		else if ((mApp->IsScaryPotterLevel() && !IsFinalScaryPotterStage()) || mApp->IsEndlessIZombie(mApp->mGameMode))
		{
			if (mNextSurvivalStageCounter == 0)
			{
				RemoveAllZombies();
				FadeOutLevel();
			}
		}
		else if (mApp->IsSurvivalMode())
		{
			if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO)
			{
				return;
			}

			mCurrentWave = mNumWaves;
			//if (!IsSurvivalStageWithRepick())
			//{
				RemoveAllZombies();
			//}
			FadeOutLevel();
		}
		else
		{
			RemoveAllZombies();
			FadeOutLevel();
			mBoardFadeOutCounter = 200;
		}
	}
	else if (theChar == '+')
	{
		mApp->mBoardResult = BoardResult::BOARDRESULT_CHEAT;
		if (IsLastStandStageWithRepick())
		{
			if (mNextSurvivalStageCounter == 0)
			{
				mCurrentWave = mNumWaves;
				RemoveAllZombies();
				FadeOutLevel();
			}
		}
		else if ((mApp->IsScaryPotterLevel() && !IsFinalScaryPotterStage()) || mApp->IsEndlessIZombie(mApp->mGameMode))
		{
			if (mNextSurvivalStageCounter == 0)
			{
				RemoveAllZombies();
				FadeOutLevel();
			}
		}
		else if (mApp->IsSurvivalEndless(mApp->mGameMode))
		{
			if (mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO)
			{
				return;
			}

			mCurrentWave = mNumWaves;
			RemoveAllZombies();
			FadeOutLevel();
		}
		else if (mApp->IsSurvivalMode())
		{
			mChallenge->mSurvivalStage = 5;
			RemoveAllZombies();
			FadeOutLevel();
			mBoardFadeOutCounter = 200;
		}
		else
		{
			RemoveAllZombies();
			FadeOutLevel();
			mBoardFadeOutCounter = 200;
		}
	}
	else if (theChar == '8')
	{
		mApp->mEasyPlantingCheat = !mApp->mEasyPlantingCheat;
	}
	else if (theChar == '7')
	{
		mApp->ToggleSlowMo();
	}
	else if (theChar == '6')
	{
		mApp->ToggleFastMo();
	}
	else if (theChar == 'z')
	{
		mDebugTextMode = static_cast<DebugTextMode>(static_cast<int>(mDebugTextMode) + 1);
		if (mDebugTextMode > DebugTextMode::DEBUG_TEXT_COLLISION)
		{
			mDebugTextMode = DebugTextMode::DEBUG_TEXT_NONE;
		}
	}

	if (mApp->mGameScene != GameScenes::SCENE_PLAYING)
	{
		return;
	}

	Zombie* aBossZombie = GetBossZombie();
	if (aBossZombie && !aBossZombie->IsDeadOrDying())
	{
		if (theChar == 'b')
		{
			aBossZombie->mBossBungeeCounter = 0;
			return;
		}
		if (theChar == 'u')
		{
			aBossZombie->mSummonCounter = 0;
			return;
		}
		if (theChar == 's')
		{
			aBossZombie->mBossStompCounter = 0;
			return;
		}
		if (theChar == 'r')
		{
			aBossZombie->BossRVAttack();
			return;
		}
		if (theChar == 'h')
		{
			aBossZombie->mBossHeadCounter = 0;
			return;
		}
		if (theChar == 'd')
		{
			aBossZombie->TakeDamage(10000, 0U);
			return;
		}
	}

	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS_2)
	{
		if (theChar == 'w')
		{
			AddZombie(ZombieType::ZOMBIE_WALLNUT_HEAD, Zombie::ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == 't')
		{
			AddZombie(ZombieType::ZOMBIE_TALLNUT_HEAD, Zombie::ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == 'j')
		{
			AddZombie(ZombieType::ZOMBIE_JALAPENO_HEAD, Zombie::ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == 'g')
		{
			AddZombie(ZombieType::ZOMBIE_GATLING_HEAD, Zombie::ZOMBIE_WAVE_DEBUG);
			return;
		}
		if (theChar == 's')
		{
			AddZombie(ZombieType::ZOMBIE_SQUASH_HEAD, Zombie::ZOMBIE_WAVE_DEBUG);
			return;
		}
	}

	if (theChar == 'q')
	{
		if (mApp->IsSurvivalEndless(mApp->mGameMode))
		{
			mApp->mEasyPlantingCheat = true;
			for (int y = 0; y < MAX_GRID_SIZE_X; y++)
			{
				for (int x = 0; x < MAX_GRID_SIZE_Y; x++)
				{
					if (CanPlantAt(x, y, SeedType::SEED_LILYPAD) == PlantingReason::PLANTING_OK)
					{
						AddPlant(x, y, SeedType::SEED_LILYPAD, SeedType::SEED_NONE);
					}
					if (CanPlantAt(x, y, SeedType::SEED_PUMPKINSHELL) == PlantingReason::PLANTING_OK)
					{
						if (x <= 6 || IsPoolSquare(x, y))
						{
							AddPlant(x, y, SeedType::SEED_PUMPKINSHELL, SeedType::SEED_NONE);
						}
					}
					if (CanPlantAt(x, y, SeedType::SEED_GATLINGPEA) == PlantingReason::PLANTING_OK)
					{
						if (x < 5)
						{
							AddPlant(x, y, SeedType::SEED_GATLINGPEA, SeedType::SEED_NONE);
						}
						else if (x == 5)
						{
							AddPlant(x, y, SeedType::SEED_TORCHWOOD, SeedType::SEED_NONE);
						}
						else if (x == 6)
						{
							AddPlant(x, y, SeedType::SEED_SPLITPEA, SeedType::SEED_NONE);
						}
						else if (y == 2 || y == 3)
						{
							AddPlant(x, y, SeedType::SEED_GLOOMSHROOM, SeedType::SEED_NONE);
							if (CanPlantAt(x, y, SeedType::SEED_INSTANT_COFFEE) == PlantingReason::PLANTING_OK)
							{
								AddPlant(x, y, SeedType::SEED_INSTANT_COFFEE, SeedType::SEED_NONE);
							}
						}
					}
				}
			}
		}
		else if (mApp->IsIZombieLevel())
		{
			mApp->mEasyPlantingCheat = true;
			for (int i = 0; i < 5; i++)
			{
				mChallenge->IZombiePlaceZombie(ZombieType::ZOMBIE_FOOTBALL, 6, i);
			}
		}
		else
		{
			mApp->mEasyPlantingCheat = true;
			for (int y = 0; y < MAX_GRID_SIZE_Y; ++y)
			{
				for (int x = 0; x < MAX_GRID_SIZE_X; ++x)
				{
					if (StageHasRoof() && CanPlantAt(x, y, SeedType::SEED_FLOWERPOT) == PlantingReason::PLANTING_OK)
					{
						AddPlant(x, y, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
					}
					if (CanPlantAt(x, y, SeedType::SEED_LILYPAD) == PlantingReason::PLANTING_OK)
					{
						AddPlant(x, y, SeedType::SEED_LILYPAD, SeedType::SEED_NONE);
					}
					if (CanPlantAt(x, y, SeedType::SEED_THREEPEATER) == PlantingReason::PLANTING_OK)
					{
						AddPlant(x, y, SeedType::SEED_THREEPEATER, SeedType::SEED_NONE);
					}
				}
			}

			if (!mChallenge->UpdateZombieSpawning())
			{
				int aWavesRemaining = std::min(mNumWaves - mCurrentWave, 20);
				while (aWavesRemaining)
				{
					SpawnZombieWave();
					aWavesRemaining--;
				}
			}

			if (mApp->IsScaryPotterLevel())
			{
				GridItem* aGridItem = nullptr;
				while (IterateGridItems(aGridItem))
				{
					if (aGridItem->mGridItemType == GridItemType::GRIDITEM_SCARY_POT)
					{
						mChallenge->ScaryPotterOpenPot(aGridItem);
					}
				}
			}
		}

		return;
	}

	if (theChar == 'O')
	{
		mApp->mEasyPlantingCheat = true;
		for (int y = 0; y < MAX_GRID_SIZE_Y; y++)
		{
			for (int x = 0; x < 3; x++)
			{
				if (CanPlantAt(x, y, SeedType::SEED_FLOWERPOT) == PlantingReason::PLANTING_OK)
				{
					AddPlant(x, y, SeedType::SEED_FLOWERPOT, SeedType::SEED_NONE);
				}
			}
		}
		return;
	}

	if (theChar == '?' || theChar == '/')
	{
		if (mHugeWaveCountDown > 0)
		{
			mHugeWaveCountDown = 1;
		}
		else
		{
			mZombieCountDown = 6;
		}
		return;
	}

	if (theChar == 'b')
	{
		AddZombie(ZombieType::ZOMBIE_BUNGEE, Zombie::ZOMBIE_WAVE_DEBUG);
		return;
	}
	if (theChar == 'o')
	{
		AddZombie(ZombieType::ZOMBIE_FOOTBALL, Zombie::ZOMBIE_WAVE_DEBUG);
		return;
	}
	if (theChar == 's')
	{
		AddZombie(ZombieType::ZOMBIE_DOOR, Zombie::ZOMBIE_WAVE_DEBUG);
		return;
	}
	if (theChar == 'L')
	{
		AddZombie(ZombieType::ZOMBIE_LADDER, Zombie::ZOMBIE_WAVE_DEBUG);
		return;
	}
	if (theChar == 'y')
	{
		AddZombie(ZombieType::ZOMBIE_YETI, Zombie::ZOMBIE_WAVE_DEBUG);
		return;
	}
	if (theChar == 'a')
	{
		AddZombie(ZombieType::ZOMBIE_FLAG, Zombie::ZOMBIE_WAVE_DEBUG);
		return;
	}
	if (theChar == 'w')
	{
		AddZombie(ZombieType::ZOMBIE_NEWSPAPER, Zombie::ZOMBIE_WAVE_DEBUG);
		return;
	}
	if (theChar == 'F')
	{
		AddZombie(ZombieType::ZOMBIE_BALLOON, Zombie::ZOMBIE_WAVE_DEBUG);
		return;
	}
	if (theChar == 'n')
	{
		if (StageHasPool())
		{
			AddZombie(ZombieType::ZOMBIE_SNORKEL, Zombie::ZOMBIE_WAVE_DEBUG);
		}
	}
	if (theChar == 'c')
	{
		AddZombie(ZombieType::ZOMBIE_TRAFFIC_CONE, Zombie::ZOMBIE_WAVE_DEBUG);
		return;
	}
	if (theChar == 'm')
	{
		AddZombie(ZombieType::ZOMBIE_DANCER, Zombie::ZOMBIE_WAVE_DEBUG);
		return;
	}
	if (theChar == 'h')
	{
		AddZombie(ZombieType::ZOMBIE_PAIL, Zombie::ZOMBIE_WAVE_DEBUG);
		return;
	}
	//if (theChar == 'H'
	//{
	//	AddZombie(ZombieType::ZOMBIE_PAIL, Zombie::ZOMBIE_WAVE_DEBUG);
	//	AddZombie(ZombieType::ZOMBIE_PAIL, Zombie::ZOMBIE_WAVE_DEBUG);
	//	AddZombie(ZombieType::ZOMBIE_PAIL, Zombie::ZOMBIE_WAVE_DEBUG);
	//	AddZombie(ZombieType::ZOMBIE_PAIL, Zombie::ZOMBIE_WAVE_DEBUG);
	//	AddZombie(ZombieType::ZOMBIE_PAIL, Zombie::ZOMBIE_WAVE_DEBUG);
	//	return;
	//}
	if (theChar == 'D')
	{
		AddZombie(ZombieType::ZOMBIE_DIGGER, Zombie::ZOMBIE_WAVE_DEBUG);
		return;
	}
	if (theChar == 'p')
	{
		AddZombie(ZombieType::ZOMBIE_POLEVAULTER, Zombie::ZOMBIE_WAVE_DEBUG);
		return;
	}
	if (theChar == 'P')
	{
		AddZombie(ZombieType::ZOMBIE_POGO, Zombie::ZOMBIE_WAVE_DEBUG);
		return;
	}
	if (theChar == 'R')
	{
		if (StageHasPool())
		{
			AddZombie(ZombieType::ZOMBIE_DOLPHIN_RIDER, Zombie::ZOMBIE_WAVE_DEBUG);
		}
		return;
	}
	else if(theChar == 'j')
	{
		AddZombie(ZombieType::ZOMBIE_JACK_IN_THE_BOX, Zombie::ZOMBIE_WAVE_DEBUG);
		return;
	}
	if (theChar == 'g')
	{
		AddZombie(ZombieType::ZOMBIE_GARGANTUAR, Zombie::ZOMBIE_WAVE_DEBUG);
		return;
	}
	if (theChar == 'G')
	{
		AddZombie(ZombieType::ZOMBIE_REDEYE_GARGANTUAR, Zombie::ZOMBIE_WAVE_DEBUG);
		return;
	}
	if (theChar == 'i')
	{
		AddZombie(ZombieType::ZOMBIE_ZAMBONI, Zombie::ZOMBIE_WAVE_DEBUG);
		return;
	}
	if (theChar == 'C')
	{
		AddZombie(ZombieType::ZOMBIE_CATAPULT, Zombie::ZOMBIE_WAVE_DEBUG);
		return;
	}
	if (theChar == '1')
	{
		Plant* aPlant = GetTopPlantAt(0, 0, PlantPriority::TOPPLANT_ANY);
		if (aPlant)
		{
			aPlant->Die();
			mChallenge->ZombieAtePlant(aPlant);
			return;
		}
	}
	if (theChar == 'B')
	{
		mFogBlownCountDown = 2200;
		return;
	}
	if (theChar == 't')
	{
		if (!CanAddBobSled())
		{
			int aRow = Rand(5);
			int aPos = 400;
			if (StageHasPool())
			{
				aRow = Rand(2);
			}
			else if (StageHasRoof())
			{
				aPos = 500;
			}
			mIceTimer[aRow] = 3000;
			mIceMinX[aRow] = aPos;
		}

		AddZombie(ZombieType::ZOMBIE_BOBSLED, Zombie::ZOMBIE_WAVE_DEBUG);
		return;
	}
	if (theChar == 'r')
	{
		SpawnZombiesFromGraves();
		return;
	}
	if (theChar == '0')
	{
		AddSunMoney(100);
		mApp->PlaySample(SOUND_BUTTONCLICK);
		return;
	}
	if (theChar == '9')
	{
		AddSunMoney(999999);
		mApp->PlaySample(SOUND_BUTTONCLICK);
		return;
	}
	if (theChar == '$')
	{
		mApp->mPlayerInfo->AddCoins(100);
		mApp->PlaySample(SOUND_BUTTONCLICK);
		ShowCoinBank();
		return;
	}
	if (theChar == '-')
	{
		mSunMoney -= 100;
		mSunMoney = std::max(mSunMoney, 0);
		return;
	}

	if (theChar == '%')
	{
		mApp->SwitchScreenMode(mApp->mIsWindowed, !mApp->Is3DAccelerated(), false);
	}
	if (theChar == 'M')
	{
		mApp->mMusic->mBurstOverride -= 2 - (mApp->mMusic->mBurstOverride != 1);
		return;
	}

	if (theChar == '\3' && mApp->mCtrlDown && mApp->mTodCheatKeys)
	{
		TodCrash();

		if (mHugeWaveCountDown > 0)
		{
			mHugeWaveCountDown = 1;
		}
		else
		{
			mZombieCountDown = 6;
		}
	}
}

// GOTY @Patoke: 0x41E6E0
void Board::AddSunMoney(int theAmount)
{
	mSunMoney += theAmount;
	mSunMoney = std::min(mSunMoney, 9990);
	if (mSunMoney >= 8000)
		// if ( !*(mApp->mPlayerInfo + 48) ) todo @Patoke: figure this out
		ReportAchievement::GiveAchievement(mApp, SunnyDays, true);
}

int Board::CountSunBeingCollected()
{
	int aCount = 0;
	Coin* aCoin = nullptr;
	while (IterateCoins(aCoin))
	{
		if (aCoin->mIsBeingCollected && aCoin->IsSun())
		{
			aCount += aCoin->GetSunValue();
		}
	}
	return aCount;
}

int Board::CountCoinsBeingCollected()
{
	int aCount = 0;
	Coin* aCoin = nullptr;
	while (IterateCoins(aCoin))
	{
		if (aCoin->mIsBeingCollected && aCoin->IsMoney())
		{
			aCount += aCoin->GetCoinValue(aCoin->mType);
		}
	}
	return aCount;
}

bool Board::TakeSunMoney(int theAmount)
{
	if (CanTakeSunMoney(theAmount))
	{
		mSunMoney -= theAmount;
		return true;
	}

	mApp->PlaySample(Sexy::SOUND_BUZZER);
	mOutOfMoneyCounter = 70;
	return false;
}

bool Board::CanTakeSunMoney(int theAmount)
{
	return theAmount <= mSunMoney + CountSunBeingCollected();
}

void Board::ProcessDeleteQueue()
{
	{
		Plant* aPlant = nullptr;
		while (mPlants.IterateNext(aPlant))
		{
			if (aPlant->mDead)
			{
				mPlants.DataArrayFree(aPlant);
			}
		}
	}
	{
		Zombie* aZombie = nullptr;
		while (mZombies.IterateNext(aZombie))
		{
			if (aZombie->mDead)
			{
				mZombies.DataArrayFree(aZombie);
			}
		}
	}
	{
		Projectile* aProjectile = nullptr;
		while (mProjectiles.IterateNext(aProjectile))
		{
			if (aProjectile->mDead)
			{
				mProjectiles.DataArrayFree(aProjectile);
			}
		}
	}
	{
		Coin* aCoin = nullptr;
		while (mCoins.IterateNext(aCoin))
		{
			if (aCoin->mDead)
			{
				mCoins.DataArrayFree(aCoin);
			}
		}
	}
	{
		LawnMower* aLawnMower = nullptr;
		while (mLawnMowers.IterateNext(aLawnMower))
		{
			if (aLawnMower->mDead)
			{
				mLawnMowers.DataArrayFree(aLawnMower);
			}
		}
	}
	{
		GridItem* aGridItem = nullptr;
		while (mGridItems.IterateNext(aGridItem))
		{
			if (aGridItem->mDead)
			{
				mGridItems.DataArrayFree(aGridItem);
			}
		}
	}
}

// GOTY @Patoke: 0x41EC10
bool Board::HasConveyorBeltSeedBank()
{
	return
		mApp->IsFinalBossLevel() || 
		mApp->IsMiniBossLevel() || 
		mApp->IsShovelLevel() || 
		mApp->IsWallnutBowlingLevel() ||
		mApp->IsLittleTroubleLevel() || 
		mApp->IsStormyNightLevel() || 
		mApp->IsBungeeBlitzLevel() || 
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_PORTAL_COMBAT || 
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_COLUMN || 
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_INVISIGHOUL;
}

int Board::GetNumSeedsInBank()
{
	if (mApp->IsScaryPotterLevel())
	{
		return 1;
	}
	if (mApp->IsWhackAZombieLevel())
	{
		return 3;
	}
	if (mApp->IsChallengeWithoutSeedBank())
	{
		return 0;
	}
	if (HasConveyorBeltSeedBank())
	{
		return 10;
	}
	if (mApp->IsSlotMachineLevel())
	{
		return 3;
	}
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ICE)
	{
		return 6;
	}
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED || mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST)
	{
		return 0;
	}
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM)
	{
		return 2;
	}
	if (mApp->mGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_1 || mApp->mGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_2 ||
		mApp->mGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_3 || mApp->mGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_4)
	{
		return 3;
	}
	if (mApp->mGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_5 || mApp->mGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_6 ||
		mApp->mGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_7)
	{
		return 4;
	}
	if (mApp->mGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_8)
	{
		return 6;
	}
	if (mApp->mGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_9)
	{
		return 8;
	}
	if (mApp->mGameMode == GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_ENDLESS)
	{
		return 9;
	}

	int aNumSeeds = mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_PACKET_UPGRADE] + 6;
	int aSeedsAvailable = mApp->GetSeedsAvailable();
	return std::min(aNumSeeds, aSeedsAvailable);
}

bool Board::StageIsNight()
{
	return 
		mBackground == BackgroundType::BACKGROUND_2_NIGHT || 
		mBackground == BackgroundType::BACKGROUND_4_FOG || 
		mBackground == BackgroundType::BACKGROUND_6_BOSS ||
		mBackground == BackgroundType::BACKGROUND_MUSHROOM_GARDEN || 
		mBackground == BackgroundType::BACKGROUND_ZOMBIQUARIUM;
}

bool Board::StageHasGraveStones()
{
	if (mApp->IsWallnutBowlingLevel() ||
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_POGO_PARTY ||
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED ||
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST ||
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_PORTAL_COMBAT ||
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND ||
		mApp->IsIZombieLevel() ||
		mApp->IsScaryPotterLevel())
		return false;

	return mBackground == BackgroundType::BACKGROUND_2_NIGHT;
}

bool Board::StageHasRoof()
{
	return (mBackground == BackgroundType::BACKGROUND_5_ROOF || mBackground == BackgroundType::BACKGROUND_6_BOSS);
}

bool Board::StageHasPool()
{
	return (mBackground == BackgroundType::BACKGROUND_3_POOL || mBackground == BackgroundType::BACKGROUND_4_FOG);
}

bool Board::StageHas6Rows()
{
	return (mBackground == BackgroundType::BACKGROUND_3_POOL || mBackground == BackgroundType::BACKGROUND_4_FOG);
}

bool Board::StageHasZombieWalkInFromRight()
{
	if (mApp->IsWhackAZombieLevel() ||
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ICE ||
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN ||
		mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM ||
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM ||
		mApp->IsFinalBossLevel() ||
		mApp->IsIZombieLevel() ||
		mApp->IsSquirrelLevel() ||
		mApp->IsScaryPotterLevel())
		return false;

	return true;
}

bool Board::StageHasFog()
{
	return !mApp->IsStormyNightLevel() && mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_INVISIGHOUL && mBackground == BackgroundType::BACKGROUND_4_FOG;
}

// GOTY @Patoke: inlined 0x41E669
bool Board::StageIsDayWithoutPool() {
	return mBackground == BackgroundType::BACKGROUND_1_DAY;
}

// GOTY @Patoke: inlined 0x41E5E6
bool Board::StageIsDayWithPool() {
	return mBackground == BackgroundType::BACKGROUND_3_POOL;
}

int Board::LeftFogColumn()
{
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_AIR_RAID)		return 6;
	if (!mApp->IsAdventureMode())										return 5;
	if (mLevel == 31)													return 6;
	if (mLevel >= 32 && mLevel <= 36)									return 5;
	if (mLevel >= 37 && mLevel <= 40)									return 4;
	TOD_ASSERT(false);

	unreachable();
}

int Board::GetSeedPacketPositionX(int theIndex)
{
	if (mApp->IsSlotMachineLevel())			return theIndex * 59 + 247;
	if (HasConveyorBeltSeedBank())			return theIndex * 50 + 91;
	
	if (mSeedBank->mNumPackets <= 7)		return theIndex * 59 + 85;
	else if (mSeedBank->mNumPackets == 8)	return theIndex * 54 + 81;
	else if (mSeedBank->mNumPackets == 9)	return theIndex * 52 + 80;
	else									return theIndex * 51 + 79;
}

int Board::GetSeedBankExtraWidth()
{
	int aNumPackets = mSeedBank->mNumPackets;
	return aNumPackets <= 6 ? 0 : aNumPackets == 7 ? 60 : aNumPackets == 8 ? 76 : aNumPackets == 9 ? 112 : 153;
}

void Board::OffsetYForPlanting(int& theY, SeedType theSeedType)
{
	if (Plant::IsFlying(theSeedType) || theSeedType == SeedType::SEED_GRAVEBUSTER)
	{
		theY += 15;
	}
	if (theSeedType == SeedType::SEED_SPIKEWEED || theSeedType == SeedType::SEED_SPIKEROCK)
	{
		theY -= 15;
	}
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN && mBackground == BackgroundType::BACKGROUND_GREENHOUSE)
	{
		theY -= 25;
	}
}

int Board::PlantingPixelToGridX(int theX, int theY, SeedType theSeedType)
{
	OffsetYForPlanting(theY, theSeedType);
	return PixelToGridX(theX, theY);
}

int Board::PlantingPixelToGridY(int theX, int theY, SeedType theSeedType)
{
	OffsetYForPlanting(theY, theSeedType);

	int aGridY = PixelToGridY(theX, theY);
	if (theSeedType == SeedType::SEED_INSTANT_COFFEE)
	{
		int aGridX = PixelToGridX(theX, theY);
		
		Plant* aPlant = GetTopPlantAt(aGridX, aGridY, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION);
		if (aPlant && aPlant->mIsAsleep)
		{
			return aGridY;
		}
		
		int aGridYDown = PixelToGridY(theX, theY + 30);
		if (aGridYDown != aGridY)
		{
			Plant* aPlantDown = GetTopPlantAt(aGridX, aGridYDown, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION);
			if (aPlantDown && aPlantDown->mIsAsleep)
			{
				return aGridYDown;
			}
		}

		int aGridYUp = PixelToGridY(theX, theY - 50);
		if (aGridYUp != aGridY)
		{
			Plant* aPlantUp = GetTopPlantAt(aGridX, aGridYUp, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION);
			if (aPlantUp && aPlantUp->mIsAsleep)
			{
				return aGridYUp;
			}
		}
	}
	return aGridY;
}

int Board::PixelToGridX(int theX, int theY)
{
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		if (mBackground == BackgroundType::BACKGROUND_GREENHOUSE || 
			mBackground == BackgroundType::BACKGROUND_MUSHROOM_GARDEN || 
			mBackground == BackgroundType::BACKGROUND_ZOMBIQUARIUM)
		{
			return mApp->mZenGarden->PixelToGridX(theX, theY);
		}
	}

	if (theX < LAWN_XMIN)
		return -1;

	return ClampInt((theX - LAWN_XMIN) / 80, 0, MAX_GRID_SIZE_X - 1);
}

int Board::PixelToGridXKeepOnBoard(int theX, int theY)
{
	int aGridX = PixelToGridX(theX, theY);
	return std::max(aGridX, 0);
}

int Board::PixelToGridY(int theX, int theY)
{
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		if (mBackground == BackgroundType::BACKGROUND_GREENHOUSE ||
			mBackground == BackgroundType::BACKGROUND_MUSHROOM_GARDEN || 
			mBackground == BackgroundType::BACKGROUND_ZOMBIQUARIUM)
		{
			return mApp->mZenGarden->PixelToGridY(theX, theY);
		}
	}

	int aGridX = PixelToGridX(theX, theY);
	if (aGridX == -1 || theY < LAWN_YMIN)
		return -1;

	if (StageHasRoof())
	{
		if (aGridX < 5)
		{
			theY -= (4 - aGridX) * 20;
		}
		return ClampInt((theY - LAWN_YMIN) / 85, 0, MAX_GRID_SIZE_Y - 2);
	}
	else if (StageHasPool())
	{
		return ClampInt((theY - LAWN_YMIN) / 85, 0, MAX_GRID_SIZE_Y - 1);
	}
	else
	{
		return ClampInt((theY - LAWN_YMIN) / 100, 0, MAX_GRID_SIZE_Y - 2);
	}
}

int Board::PixelToGridYKeepOnBoard(int theX, int theY)
{
	int aGridY = PixelToGridY(std::max(theX, 80), theY);
	return std::max(aGridY, 0);
}

int Board::GridToPixelX(int theGridX, int theGridY)
{
	TOD_ASSERT(theGridX >= 0 && theGridX < MAX_GRID_SIZE_X);
	TOD_ASSERT(theGridY >= 0 && theGridY < MAX_GRID_SIZE_Y);
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		if (mBackground == BackgroundType::BACKGROUND_GREENHOUSE ||
			mBackground == BackgroundType::BACKGROUND_MUSHROOM_GARDEN || 
			mBackground == BackgroundType::BACKGROUND_ZOMBIQUARIUM)
		{
			return mApp->mZenGarden->GridToPixelX(theGridX, theGridY);
		}
	}

	return theGridX * 80 + LAWN_XMIN;
}

float Board::GetPosYBasedOnRow(float thePosX, int theRow)
{
	if (StageHasRoof())
	{
		float aSlopeOffset = 0.0f;
		if (thePosX < 440.0f)
		{
			aSlopeOffset = (440.0f - thePosX) * 0.25f;
		}

		return GridToPixelY(8, theRow) + aSlopeOffset;
	}
	
	return GridToPixelY(0, theRow);
}

int Board::GridToPixelY(int theGridX, int theGridY)
{
	TOD_ASSERT(theGridX >= 0 && theGridX < MAX_GRID_SIZE_X);
	TOD_ASSERT(theGridY >= 0 && theGridY < MAX_GRID_SIZE_Y);
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		if (mBackground == BackgroundType::BACKGROUND_GREENHOUSE ||
			mBackground == BackgroundType::BACKGROUND_MUSHROOM_GARDEN ||
			mBackground == BackgroundType::BACKGROUND_ZOMBIQUARIUM)
		{
			return mApp->mZenGarden->GridToPixelY(theGridX, theGridY);
		}
	}

	int aY;
	if (StageHasRoof())
	{
		int aSlopeOffset;
		if (theGridX < 5)
		{
			aSlopeOffset = (5 - theGridX) * 20;
		}
		else
		{
			aSlopeOffset = 0;
		}
		aY = theGridY * 85 + aSlopeOffset + LAWN_YMIN - 10;
	}
	else if (StageHasPool())
	{
		aY = theGridY * 85 + LAWN_YMIN;
	}
	else
	{
		aY = theGridY * 100 + LAWN_YMIN;
	}

	if (theGridX != -1 && mGridSquareType[theGridX][theGridY] == GridSquareType::GRIDSQUARE_HIGH_GROUND)
	{
		aY -= HIGH_GROUND_HEIGHT;
	}

	return aY;
}

ZombieID Board::ZombieGetID(Zombie* theZombie)
{
	return static_cast<ZombieID>(mZombies.DataArrayGetID(theZombie));
}

Zombie* Board::ZombieGet(ZombieID theZombieID)
{
	return mZombies.DataArrayGet(static_cast<unsigned int>(theZombieID));
}

Zombie* Board::ZombieTryToGet(ZombieID theZombieID)
{
	return mZombies.DataArrayTryToGet(static_cast<unsigned int>(theZombieID));
}

int GetRectOverlap(const Rect& rect1, const Rect& rect2)
{
	return std::min(rect1.mX + rect1.mWidth, rect2.mX + rect2.mWidth) -
		std::max(rect1.mX, rect2.mX);
}

bool GetCircleRectOverlap(int theCircleX, int theCircleY, int theRadius, const Rect& theRect)
{
	int aNearX = std::clamp(theCircleX, theRect.mX, theRect.mX + theRect.mWidth);
	int aNearY = std::clamp(theCircleY, theRect.mY, theRect.mY + theRect.mHeight);
	int dx = theCircleX - aNearX;
	int dy = theCircleY - aNearY;
	return dx * dx + dy * dy <= theRadius * theRadius;
}

// GOTY @Patoke: 0x41F6B0
bool Board::IterateZombies(Zombie*& theZombie)
{
	while (mZombies.IterateNext(theZombie))
	{
		if (!theZombie->mDead)
		{
			return true;
		}
	}

	theZombie = (Zombie*)-1;
	return false;
}

bool Board::IteratePlants(Plant*& thePlant)
{
	while (mPlants.IterateNext(thePlant))
	{
		if (!thePlant->mDead)
		{
			return true;
		}
	}

	thePlant = (Plant*)-1;
	return false;
}

bool Board::IterateProjectiles(Projectile*& theProjectile)
{
	while (mProjectiles.IterateNext(theProjectile))
	{
		if (!theProjectile->mDead)
		{
			return true;
		}
	}

	theProjectile = (Projectile*)-1;
	return false;
}

bool Board::IterateCoins(Coin*& theCoin) 
{
	while (mCoins.IterateNext(theCoin))
	{
		if (!theCoin->mDead)
		{
			return true;
		}
	}

	theCoin = (Coin*)-1;
	return false;
}

bool Board::IterateLawnMowers(LawnMower*& theLawnMower)
{
	while (mLawnMowers.IterateNext(theLawnMower))
	{
		if (!theLawnMower->mDead)
		{
			return true;
		}
	}

	theLawnMower = (LawnMower*)-1;
	return false;
}

bool Board::IterateGridItems(GridItem*& theGridItem)
{
	while (mGridItems.IterateNext(theGridItem))
	{
		if (!theGridItem->mDead)
		{
			return true;
		}
	}

	theGridItem = (GridItem*)-1;
	return false;
}

bool Board::IterateParticles(TodParticleSystem*& theParticle)
{
	while (mApp->mEffectSystem->mParticleHolder->mParticleSystems.IterateNext(theParticle))
	{
		if (!theParticle->mDead)
		{
			return true;
		}
	}

	theParticle = (TodParticleSystem*)-1;
	return false;
}

bool Board::IterateReanimations(Reanimation*& theReanimation)
{
	while (mApp->mEffectSystem->mReanimationHolder->mReanimations.IterateNext(theReanimation))
	{
		if (!theReanimation->mDead)
		{
			return true;
		}
	}

	theReanimation = (Reanimation*)-1;
	return false;
}

void Board::KillAllPlantsInRadius(int theX, int theY, int theRadius)
{
	Plant* aPlant = nullptr;
	while (IteratePlants(aPlant))
	{
		if (GetCircleRectOverlap(theX, theY, theRadius, aPlant->GetPlantRect()))
		{
			mPlantsEaten++;
			aPlant->Die();
		}
	}
}

unsigned int Board::SeedNotRecommendedForLevel(SeedType theSeedType)
{
	unsigned int aNotRec = 0;
	if (Plant::IsNocturnal(theSeedType) && !StageIsNight())
	{
		SetBit(aNotRec, NotRecommend::NOT_RECOMMENDED_NOCTURNAL, true);
	}
	if (theSeedType == SeedType::SEED_INSTANT_COFFEE && StageIsNight())
	{
		SetBit(aNotRec, NotRecommend::NOT_RECOMMENDED_AT_NIGHT, true);
	}
	if (theSeedType == SeedType::SEED_GRAVEBUSTER && !StageHasGraveStones())
	{
		SetBit(aNotRec, NotRecommend::NOT_RECOMMENDED_NEEDS_GRAVES, true);
	}
	if (theSeedType == SeedType::SEED_PLANTERN && !StageHasFog())
	{
		SetBit(aNotRec, NotRecommend::NOT_RECOMMENDED_NEEDS_FOG, true);
	}
	if (theSeedType == SeedType::SEED_FLOWERPOT && !StageHasRoof())
	{
		SetBit(aNotRec, NotRecommend::NOT_RECOMMENDED_NEEDS_ROOF, true);
	}
	if (StageHasRoof() && (theSeedType == SeedType::SEED_SPIKEWEED || theSeedType == SeedType::SEED_SPIKEROCK))
	{
		SetBit(aNotRec, NotRecommend::NOT_RECOMMENDED_ON_ROOF, true);
	}
	if (!StageHasPool() && Plant::IsAquatic(theSeedType))
	{
		SetBit(aNotRec, NotRecommend::NOT_RECOMMENDED_NEEDS_POOL, true);
	}
	return aNotRec;
}

int Board::CountCoinByType(CoinType theCoinType)
{
	int aCount = 0;

	Coin* aCoin = nullptr;
	while (IterateCoins(aCoin))
	{
		if (aCoin->mType == theCoinType)
		{
			aCount++;
		}
	}

	return aCount;
}

int Board::GetGraveStoneCount()
{
	int aCount = 0;

	GridItem* aGridItem = nullptr;
	while (IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType == GridItemType::GRIDITEM_GRAVESTONE)
		{
			aCount++;
		}
	}

	return aCount;
}

void Board::DropLootPiece(int thePosX, int thePosY, int theDropFactor)
{
	if (mApp->IsFirstTimeAdventureMode())
	{
		if (mLevel == 22 && mCurrentWave > 5 && !mApp->mPlayerInfo->mHasUnlockedMinigames && CountCoinByType(CoinType::COIN_PRESENT_MINIGAMES) == 0)
		{
			mApp->PlayFoley(FoleyType::FOLEY_ART_CHALLENGE);
			AddCoin(thePosX, thePosY, CoinType::COIN_PRESENT_MINIGAMES, CoinMotion::COIN_MOTION_COIN);
			return;
		}
		if (mLevel == 36 && mCurrentWave > 5 && !mApp->mPlayerInfo->mHasUnlockedPuzzleMode && CountCoinByType(CoinType::COIN_PRESENT_PUZZLE_MODE) == 0)
		{
			mApp->PlayFoley(FoleyType::FOLEY_ART_CHALLENGE);
			AddCoin(thePosX, thePosY, CoinType::COIN_PRESENT_PUZZLE_MODE, CoinMotion::COIN_MOTION_COIN);
			return;
		}
	}

	int aDropHit = Rand(30000);
	if (mApp->IsFirstTimeAdventureMode() && mLevel == 11 && !mDroppedFirstCoin && mCurrentWave > 5)
	{
		aDropHit = 1000;
	}
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_COLUMN)
	{
		aDropHit *= 5;
	}

	if (mApp->IsWhackAZombieLevel())
	{
		int aSunChanceMin = 2500;
		int aSunChanceMax = mSunMoney > 500 ? 2800 : mSunMoney > 350 ? 3100 : mSunMoney > 200 ? 3700 : 5000;
		if (aDropHit >= aSunChanceMin * theDropFactor && aDropHit <= aSunChanceMax * theDropFactor)
		{
			mApp->PlayFoley(FoleyType::FOLEY_SPAWN_SUN);
			AddCoin(thePosX - 20, thePosY, CoinType::COIN_SUN, CoinMotion::COIN_MOTION_COIN);
			AddCoin(thePosX - 40, thePosY, CoinType::COIN_SUN, CoinMotion::COIN_MOTION_COIN);
			AddCoin(thePosX - 60, thePosY, CoinType::COIN_SUN, CoinMotion::COIN_MOTION_COIN);
			return;
		}
	}

	if (mTotalSpawnedWaves > 70)
		return;

	int aPottedPlantChance;
	if (!mApp->mZenGarden->CanDropPottedPlantLoot())
	{
		aPottedPlantChance = 0;
	}
	else if (mApp->IsAdventureMode() && !mApp->IsFirstTimeAdventureMode())
	{
		aPottedPlantChance = 24;
	}
	else
	{
		aPottedPlantChance = mApp->IsSurvivalEndless(mApp->mGameMode) ? 3 : 12;
	}

	int aChocolateChance = aPottedPlantChance;
	if (mApp->mZenGarden->CanDropChocolate())
	{
		if (mApp->IsAdventureMode() && !mApp->IsFirstTimeAdventureMode())
		{
			aChocolateChance = aPottedPlantChance + 72;
		}
		else
		{
			aChocolateChance = aPottedPlantChance + (mApp->IsSurvivalEndless(mApp->mGameMode) ? 9 : 36);
		}
	}

	int aDiamondChance = aChocolateChance + 14;
	int aGoldChance = aChocolateChance + 250;
	int aSilverChance = aChocolateChance + 2500;

	CoinType aCoinType;
	if (aDropHit < aPottedPlantChance * theDropFactor)
	{
		aCoinType = CoinType::COIN_PRESENT_PLANT;
	}
	else if (aDropHit < aChocolateChance * theDropFactor)
	{
		aCoinType = CoinType::COIN_CHOCOLATE;
	}
	else if (aDropHit < aDiamondChance * theDropFactor)
	{
		aCoinType = mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_PACKET_UPGRADE] < 1 ? CoinType::COIN_GOLD : CoinType::COIN_DIAMOND;
	}
	else if (aDropHit < aGoldChance * theDropFactor)
	{
		aCoinType = CoinType::COIN_GOLD;
	}
	else if (aDropHit < aSilverChance * theDropFactor)
	{
		aCoinType = CoinType::COIN_SILVER;
	}
	else return;

	if (mApp->IsWallnutBowlingLevel() && Coin::IsMoney(aCoinType))
		return;

	if (mApp->IsFirstTimeAdventureMode() && mLevel == 11)
	{
		int aMoney = Coin::GetCoinValue(CoinType::COIN_GOLD) * mLawnMowers.mSize;
		int aCost = StoreScreen::GetItemCost(StoreItem::STORE_ITEM_PACKET_UPGRADE);
		aMoney += mApp->mPlayerInfo->mCoins + CountCoinsBeingCollected();
		if (Coin::GetCoinValue(aCoinType) + aMoney >= aCost)
		{
			return;
		}
	}

	mApp->PlayFoley(FoleyType::FOLEY_SPAWN_SUN);
	AddCoin(thePosX - 40, thePosY, aCoinType, CoinMotion::COIN_MOTION_COIN);
	mDroppedFirstCoin = true;
}

bool Board::CanDropLoot()
{
	return !mCutScene->ShouldRunUpsellBoard() && (!mApp->IsFirstTimeAdventureMode() || mLevel >= 11);
}

bool Board::BungeeIsTargetingCell(int theGridX, int theGridY)
{
	Zombie* aZombie = nullptr;
	while (IterateZombies(aZombie))
	{
		if (!aZombie->IsDeadOrDying() && aZombie->mZombieType == ZombieType::ZOMBIE_BUNGEE && aZombie->mRow == theGridY && aZombie->mTargetCol == theGridX)
		{
			return true;
		}
	}
	return false;
}

Zombie* Board::GetBossZombie()
{
	Zombie* aZombie = nullptr;
	while (IterateZombies(aZombie))
	{
		if (aZombie->mZombieType == ZombieType::ZOMBIE_BOSS)
		{
			return aZombie;
		}
	}
	return nullptr;
}

Plant* Board::FindUmbrellaPlant(int theGridX, int theGridY)
{
	Plant* aPlant = nullptr;
	while (IteratePlants(aPlant))
	{
		if (aPlant->mSeedType == SeedType::SEED_UMBRELLA && !aPlant->NotOnGround() && GridInRange(theGridX, theGridY, aPlant->mPlantCol, aPlant->mRow, 1, 1))
		{
			return aPlant;
		}
	}
	return nullptr;
}

void Board::DoFwoosh(int theRow)
{
	int aRenderOrder = MakeRenderOrder(RenderLayer::RENDER_LAYER_PARTICLE, theRow, 1);
	for (int i = 0; i < 12; i++)
	{
		Reanimation* aOriReanim = mApp->ReanimationTryToGet(mFwooshID[theRow][i]);
		if (aOriReanim)
		{
			aOriReanim->ReanimationDie();
		}

		float aPosX = 750.0f * i / 11.0f + 10.0f;
		float aPosY = GetPosYBasedOnRow(aPosX + 10.0f, theRow) - 10.0f;
		Reanimation* aFwoosh = mApp->AddReanimation(aPosX, aPosY, aRenderOrder, ReanimationType::REANIM_JALAPENO_FIRE);
		aFwoosh->SetFramesForLayer("anim_flame");
		aFwoosh->mLoopType = ReanimLoopType::REANIM_LOOP_FULL_LAST_FRAME;
		aFwoosh->mAnimRate *= RandRangeFloat(0.7f, 1.3f);

		float aScale = RandRangeFloat(0.9f, 1.1f);
		float aFlip = Rand(2) ? 1.0f : -1.0f;
		aFwoosh->OverrideScale(aScale * aFlip, 1);

		mFwooshID[theRow][i] = mApp->ReanimationGetID(aFwoosh);
	}
	mFwooshCountDown = 100;
}

void Board::UpdateFwoosh()
{
	if (mFwooshCountDown == 0)
		return;

	int aFwooshRemaining = TodAnimateCurve(50, 0, --mFwooshCountDown, 12, 0, TodCurves::CURVE_LINEAR);
	for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++)
	{
		for (int i = 0; i < 12 - aFwooshRemaining; i++)
		{
			Reanimation* aFwoosh = mApp->ReanimationTryToGet(mFwooshID[aRow][i]);
			if (aFwoosh)
			{
				aFwoosh->SetFramesForLayer("anim_done");
				aFwoosh->mAnimRate = 15;
				aFwoosh->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_FULL_LAST_FRAME;
			}
			mFwooshID[aRow][i] = ReanimationID::REANIMATIONID_NULL;
		}
	}
}

void Board::UpdateGridItems()
{
	GridItem* aGridItem = nullptr;
	while (IterateGridItems(aGridItem))
	{
		if (mEnableGraveStones && aGridItem->mGridItemType == GridItemType::GRIDITEM_GRAVESTONE && aGridItem->mGridItemCounter < 100)
		{
			aGridItem->mGridItemCounter++;
		}

		if (aGridItem->mGridItemType == GridItemType::GRIDITEM_CRATER && mApp->mGameScene == GameScenes::SCENE_PLAYING)
		{
			if (aGridItem->mGridItemCounter > 0)
			{
				aGridItem->mGridItemCounter--;
			}
			if (aGridItem->mGridItemCounter == 0)
			{
				aGridItem->GridItemDie();
			}
		}
		aGridItem->Update();
	}
}

bool Board::PlantingRequirementsMet(SeedType theSeedType)
{
	switch (theSeedType)
	{
	case SeedType::SEED_GATLINGPEA:			return CountPlantByType(SeedType::SEED_REPEATER);
	case SeedType::SEED_TWINSUNFLOWER:		return CountPlantByType(SeedType::SEED_SUNFLOWER);
	case SeedType::SEED_GLOOMSHROOM:		return CountPlantByType(SeedType::SEED_FUMESHROOM);
	case SeedType::SEED_CATTAIL:			return CountEmptyPotsOrLilies(SeedType::SEED_LILYPAD);
	case SeedType::SEED_WINTERMELON:		return CountPlantByType(SeedType::SEED_MELONPULT);
	case SeedType::SEED_GOLD_MAGNET:		return CountPlantByType(SeedType::SEED_MAGNETSHROOM);
	case SeedType::SEED_SPIKEROCK:			return CountPlantByType(SeedType::SEED_SPIKEWEED);
	case SeedType::SEED_COBCANNON:			return HasValidCobCannonSpot();
	default:								return true;
	}
}

// GOTY @Patoke: 0x420670
int Board::KillAllZombiesInRadius(int theRow, int theX, int theY, int theRadius, int theRowRange, bool theBurn, int theDamageRangeFlags)
{
	Zombie* aZombie = nullptr;
	int aKilledZombies = 0; // @Patoke: implemented this
	while (IterateZombies(aZombie))
	{
		if (aZombie->EffectedByDamage(theDamageRangeFlags))
		{
			Rect aZombieRect = aZombie->GetZombieRect();
			int aRowDist = aZombie->mRow - theRow;
			if (aZombie->mZombieType == ZombieType::ZOMBIE_BOSS)
			{
				aRowDist = 0;
			}

			if (aRowDist <= theRowRange && aRowDist >= -theRowRange && GetCircleRectOverlap(theX, theY, theRadius, aZombieRect))
			{
				if (theBurn)
				{
					aZombie->ApplyBurn();
				}
				else
				{
					aZombie->TakeDamage(1800, 18U);
				}

				aKilledZombies++;
			}
		}
	}

	int aGridX = PixelToGridXKeepOnBoard(theX, theY);
	int aGridY = PixelToGridYKeepOnBoard(theX, theY);
	GridItem* aGridItem = nullptr;
	while (IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType == GridItemType::GRIDITEM_LADDER)
		{
			if (GridInRange(aGridItem->mGridX, aGridItem->mGridY, aGridX, aGridY, theRowRange, theRowRange))
			{
				aGridItem->GridItemDie();
			}
		}
	}

	return aKilledZombies;
}

int Board::GetNumWavesPerSurvivalStage()
{
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND || mApp->IsSurvivalNormal(mApp->mGameMode))
	{
		return 10;
	}
	else if (mApp->IsSurvivalHard(mApp->mGameMode) || mApp->IsSurvivalEndless(mApp->mGameMode))
	{
		return 20;
	}

	TOD_ASSERT(false);

	unreachable();
}

void Board::RemoveParticleByType(ParticleEffect theEffectType)
{
	TodParticleSystem* aParticle = nullptr;
	while (IterateParticles(aParticle))
	{
		if (aParticle->mEffectType == theEffectType)
		{
			aParticle->ParticleSystemDie();
		}
	}
}

bool Board::PlantUsesAcceleratedPricing(SeedType theSeedType)
{
	return Plant::IsUpgrade(theSeedType) && mApp->IsSurvivalEndless(mApp->mGameMode);
}

int Board::GetCurrentPlantCost(SeedType theSeedType, SeedType theImitaterType)
{
	int aCost = Plant::GetCost(theSeedType, theImitaterType);
	if (PlantUsesAcceleratedPricing(theSeedType))
	{
		aCost += CountPlantByType(theSeedType) * 50;
	}
	return aCost;
}

bool Board::CanUseGameObject(GameObjectType theGameObject)
{
	if (mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM)
	{
		return (theGameObject == GameObjectType::OBJECT_TYPE_TREE_FOOD) || (theGameObject == GameObjectType::OBJECT_TYPE_NEXT_GARDEN);
	}
	if (mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		return false;
	}

	switch (theGameObject)
	{
	case GameObjectType::OBJECT_TYPE_WATERING_CAN:
		return true;
	case GameObjectType::OBJECT_TYPE_NEXT_GARDEN:
		return 
			mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_MUSHROOM_GARDEN] || 
			mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_AQUARIUM_GARDEN] ||
			mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_TREE_OF_WISDOM];
	case GameObjectType::OBJECT_TYPE_FERTILIZER:
		return mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_FERTILIZER] > 0;
	case GameObjectType::OBJECT_TYPE_BUG_SPRAY:
		return mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_BUG_SPRAY] > 0;
	case GameObjectType::OBJECT_TYPE_PHONOGRAPH:
		return  mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_PHONOGRAPH] > 0;
	case GameObjectType::OBJECT_TYPE_CHOCOLATE:
		return mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_CHOCOLATE] > 0;
	case GameObjectType::OBJECT_TYPE_WHEELBARROW:
		return mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_WHEEL_BARROW] > 0;
	case GameObjectType::OBJECT_TYPE_GLOVE:
		return mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_GARDENING_GLOVE] > 0;
	case GameObjectType::OBJECT_TYPE_MONEY_SIGN:
		return mApp->HasFinishedAdventure();
	case GameObjectType::OBJECT_TYPE_TREE_FOOD:
		return false;
	default:
		TOD_ASSERT(false);
		unreachable();
	}
}

void Board::ShakeBoard(int theShakeAmountX, int theShakeAmountY)
{
	mShakeCounter = 12;
	mShakeAmountX = theShakeAmountX;
	mShakeAmountY = theShakeAmountY;
}

LawnMower* Board::FindLawnMowerInRow(int theRow)
{
	LawnMower* aLawnMower = nullptr;
	while (IterateLawnMowers(aLawnMower))
	{
		if (aLawnMower->mRow == theRow)
		{
			return aLawnMower;
		}
	}
	return nullptr;
}

Zombie* Board::GetWinningZombie()
{
	Zombie* aZombie = nullptr;
	while (IterateZombies(aZombie))
	{
		if (aZombie->mFromWave == Zombie::ZOMBIE_WAVE_WINNER)
		{
			return aZombie;
		}
	}
	return nullptr;
}

int Board::CountZombieByType(ZombieType theZombieType)
{
	int aCount = 0;

	Zombie* aZombie = nullptr;
	while (IterateZombies(aZombie))
	{
		if (aZombie->mZombieType == theZombieType)
		{
			aCount++;
		}
	}

	return aCount;
}

int Board::NumberZombiesInWave(int theWaveIndex)
{
	TOD_ASSERT(theWaveIndex >= 0 && theWaveIndex < MAX_ZOMBIE_WAVES && theWaveIndex < mNumWaves);

	for (int i = 0; i < MAX_ZOMBIES_IN_WAVE; i++)
	{
		if (mZombiesInWave[theWaveIndex][i] == ZombieType::ZOMBIE_INVALID)
		{
			return i;
		}
	}

	TOD_ASSERT(false);
	return 0;
}

bool Board::IsZombieTypeSpawnedOnly(ZombieType theZombieType)
{
	return (theZombieType == ZombieType::ZOMBIE_BACKUP_DANCER || theZombieType == ZombieType::ZOMBIE_BOBSLED || theZombieType == ZombieType::ZOMBIE_IMP);
}
