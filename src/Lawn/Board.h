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

#ifndef __BOARD_H__
#define __BOARD_H__

#include <cstdint>
#include <memory>

#include "../ConstEnums.h"
#include "../PvzpLib/DataArray.h"
#include "widget/Widget.h"
#include "widget/ButtonListener.h"

#include "Plant.h"
#include "Zombie.h"
#include "Projectile.h"
#include "Coin.h"
#include "LawnMower.h"
#include "GridItem.h"

using namespace Sexy;

constexpr const int MAX_GRID_SIZE_X = 9;
constexpr const int MAX_GRID_SIZE_Y = 6;
constexpr const int MAX_ZOMBIES_IN_WAVE = 50;
constexpr const int MAX_ZOMBIE_WAVES = 100;
constexpr const int MAX_GRAVE_STONES = MAX_GRID_SIZE_X * MAX_GRID_SIZE_Y;
constexpr const int MAX_POOL_GRID_SIZE = 10;
constexpr const int MAX_RENDER_ITEMS = 2048;
constexpr const int PROGRESS_METER_COUNTER = 150;

class LawnApp;
class CursorObject;
class CursorPreview;
class GameButton;
class MessageWidget;
class SeedBank;
class ToolTipWidget;
class CutScene;
class Challenge;
class Reanimation;
class DataSync;
class PvzpParticleSystem;
namespace Sexy
{
	class Graphics;
	class ButtonWidget;
	class WidgetManager;
	class Image;
	class MTRand;
}

class HitResult
{
public:
	void*							mObject;
	GameObjectType					mObjectType;
};

class RenderItem
{
public:
	RenderObjectType				mRenderObjectType;
	int								mZPos;
	union
	{
		GameObject*					mGameObject;
		Plant*						mPlant;
		Zombie*						mZombie;
		Coin*						mCoin;
		Projectile*					mProjectile;
		CursorPreview*				mCursorPreview;
		PvzpParticleSystem*			mParticleSytem;
		Reanimation*				mReanimation;
		GridItem*					mGridItem;
		LawnMower*					mMower;
		BossPart					mBossPart;
		int							mBoardGridY;
	};
};
bool RenderItemSortFunc(const RenderItem& theItem1, const RenderItem& theItem2);

struct ZombiePicker
{
	int								mZombieCount;
	int								mZombiePoints;
	int								mZombieTypeCount[NUM_ZOMBIE_TYPES];
	int								mAllWavesZombieTypeCount[NUM_ZOMBIE_TYPES];
};

void						ZombiePickerInitForWave(ZombiePicker* theZombiePicker);
void						ZombiePickerInit(ZombiePicker* theZombiePicker);

struct PlantsOnLawn
{
	Plant*							mUnderPlant;
	Plant*							mPumpkinPlant;
	Plant*							mFlyingPlant;
	Plant*							mNormalPlant;
};

struct BungeeDropGrid
{
	PvzpWeightedGridArray			mGridArray[MAX_GRID_SIZE_X * MAX_GRID_SIZE_Y];
	int								mGridArrayCount;
};

class Board : public Widget, public ButtonListener
{
public:
	LawnApp*						mApp;
	DataArray<Zombie>				mZombies;
	DataArray<Plant>				mPlants;
	DataArray<Projectile>			mProjectiles;
	DataArray<Coin>					mCoins;
	DataArray<LawnMower>			mLawnMowers;
	DataArray<GridItem>				mGridItems;
	std::unique_ptr<CursorObject>		mCursorObject;
	std::unique_ptr<CursorPreview>		mCursorPreview;
	std::unique_ptr<MessageWidget>		mAdvice;
	std::unique_ptr<SeedBank>			mSeedBank;
	std::unique_ptr<GameButton>			mMenuButton;
	std::unique_ptr<GameButton>			mStoreButton;
	bool							mIgnoreMouseUp;
	std::unique_ptr<ToolTipWidget>		mToolTip;
	std::unique_ptr<CutScene>			mCutScene;
	std::unique_ptr<Challenge>			mChallenge;
	bool							mPaused;
	GridSquareType					mGridSquareType[MAX_GRID_SIZE_X][MAX_GRID_SIZE_Y];
	int32_t							mGridCelLook[MAX_GRID_SIZE_X][MAX_GRID_SIZE_Y];
	int32_t							mGridCelOffset[MAX_GRID_SIZE_X][MAX_GRID_SIZE_Y][2];
	int32_t							mGridCelFog[MAX_GRID_SIZE_X][MAX_GRID_SIZE_Y + 1];
	bool							mEnableGraveStones;
	int32_t							mSpecialGraveStoneX;
	int32_t							mSpecialGraveStoneY;
	float							mFogOffset;
	int32_t							mFogBlownCountDown;
	PlantRowType					mPlantRow[MAX_GRID_SIZE_Y];
	int32_t							mWaveRowGotLawnMowered[MAX_GRID_SIZE_Y];
	int32_t							mBonusLawnMowersRemaining;
	int32_t							mIceMinX[MAX_GRID_SIZE_Y];
	int32_t							mIceTimer[MAX_GRID_SIZE_Y];
	ParticleSystemID				mIceParticleID[MAX_GRID_SIZE_Y];
	PvzpSmoothArray					mRowPickingArray[MAX_GRID_SIZE_Y];
	ZombieType						mZombiesInWave[MAX_ZOMBIE_WAVES][MAX_ZOMBIES_IN_WAVE];
	bool							mZombieAllowed[100];
	int32_t							mSunCountDown;
	int32_t							mNumSunsFallen;
	int32_t							mShakeCounter;
	int32_t							mShakeAmountX;
	int32_t							mShakeAmountY;
	BackgroundType					mBackground;
	int32_t							mLevel;
	int32_t							mSodPosition;
	int32_t							mPrevMouseX;
	int32_t							mPrevMouseY;
	int32_t							mSunMoney;
	int32_t							mNumWaves;
	uint32_t						mMainCounter;
	uint32_t						mEffectCounter;
	uint32_t						mBoardUpdateCounter; // ticks since level start; unlike mMainCounter it also advances outside SCENE_PLAYING
	uint32_t						mDrawCount;
	int32_t							mRiseFromGraveCounter;
	int32_t							mOutOfMoneyCounter;
	int32_t							mCurrentWave;
	int32_t							mTotalSpawnedWaves;
	TutorialState					mTutorialState;
	ParticleSystemID				mTutorialParticleID;
	int32_t							mTutorialTimer;
	int32_t							mLastBungeeWave;
	int32_t							mZombieHealthToNextWave;
	int32_t							mZombieHealthWaveStart;
	int32_t							mZombieCountDown;
	int32_t							mZombieCountDownStart;
	int32_t							mHugeWaveCountDown;
	bool							mHelpDisplayed[NUM_ADVICE_TYPES];
	AdviceType						mHelpIndex;
	bool							mFinalBossKilled;
	bool							mShowShovel;
	int32_t							mCoinBankFadeCount;
	DebugTextMode					mDebugTextMode;
	bool							mLevelComplete;
	int32_t							mBoardFadeOutCounter;
	int32_t							mNextSurvivalStageCounter;
	int32_t							mScoreNextMowerCounter;
	bool							mLevelAwardSpawned;
	int32_t							mProgressMeterWidth;
	int32_t							mFlagRaiseCounter;
	int32_t							mIceTrapCounter;
	int32_t							mBoardRandSeed;
	ParticleSystemID				mPoolSparklyParticleID;
	ReanimationID					mFwooshID[MAX_GRID_SIZE_Y][12];
	int32_t							mFwooshCountDown;
	int32_t							mTimeStopCounter;
	bool							mDroppedFirstCoin;
	int32_t							mFinalWaveSoundCounter;
	int32_t							mCobCannonCursorDelayCounter;
	int32_t							mCobCannonMouseX;
	int32_t							mCobCannonMouseY;
	bool							mKilledYeti;
	bool							mMustacheMode;
	bool							mSuperMowerMode;
	bool							mFutureMode;
	bool							mPinataMode;
	bool							mDanceMode;
	bool							mDaisyMode;
	bool							mSukhbirMode;
	BoardResult						mPrevBoardResult;
	int32_t							mTriggeredLawnMowers;
	uint32_t						mPlayTimeActiveLevel;
	uint32_t						mPlayTimeInactiveLevel;
	int32_t							mMaxSunPlants;
	int64_t							mStartDrawTime;
	int64_t							mIntervalDrawTime;
	uint32_t						mIntervalDrawCountStart;
	float							mMinFPS;
	int32_t							mPreloadTime;
	intptr_t						mGameID;
	uint32_t						mGravesCleared;
	uint32_t						mPlantsEaten;
	uint32_t						mPlantsShoveled;
	bool							mPeaShooterUsed;										//+0x5784
	bool							mCatapultPlantsUsed;									//+0x5785
	bool							mMushroomAndCoffeeBeansOnly;							//+0x5790
	bool							mMushroomsUsed;											//+0x5791
	uint32_t						mLevelCoinsCollected;									//+0x5788
	uint32_t						mGargantuarsKillsByCornCob;								//+0x578C
	uint32_t						mCoinsCollected;										//+0x57C8
	uint32_t						mDiamondsCollected;										//+0x57CC
	uint32_t						mPottedPlantsCollected;
	uint32_t						mChocolateCollected;

public:
	Board(LawnApp* theApp);
	~Board() override;

	void							DisposeBoard();
	int								CountSunBeingCollected();
	void							DrawGameObjects(Graphics* g);
	void							ClearCursor();
	bool					AreEnemyZombiesOnScreen();
	LawnMower*						FindLawnMowerInRow(int theRow);
//  inline bool						SyncState(DataSync& theDataSync) { /* not found */return true; }
	void					SaveGame(const std::string& theFileName);
	bool							LoadGame(const std::string& theFileName);
	void							InitLevel();
	void							DisplayAdvice(std::string_view theAdvice, MessageStyle theMessageStyle, AdviceType theHelpIndex);
	void							StartLevel();
	Plant*							AddPlant(int theGridX, int theGridY, SeedType theSeedType, SeedType theImitaterType = SeedType::SEED_NONE);
	Projectile*						AddProjectile(int theX, int theY, int theRenderOrder, int theRow, ProjectileType theProjectileType);
	Coin*							AddCoin(int theX, int theY, CoinType theCoinType, CoinMotion theCoinMotion);
	void							RefreshSeedPacketFromCursor();
	ZombieType						PickGraveRisingZombieType();
	ZombieType						PickZombieType(int theZombiePoints, int theWaveIndex, ZombiePicker* theZombiePicker);
	int								PickRowForNewZombie(ZombieType theZombieType);
	Zombie*				AddZombie(ZombieType theZombieType, int theFromWave);
	void							SpawnZombieWave();
	void							RemoveAllZombies();
	void							RemoveCutsceneZombies();
	void							SpawnZombiesFromGraves();
	PlantingReason					CanPlantAt(int theGridX, int theGridY, SeedType theSeedType);
	void							MouseMove(int x, int y) override;
	void							MouseDrag(int x, int y) override;
	void							MouseDown(int x, int y, int theClickCount) override;
	void							MouseUp(int x, int y, int theClickCount) override;
	void							KeyChar(char theChar) override;
	void							KeyUp(KeyCode) override {}
	void							KeyDown(KeyCode theKey) override;
	void							Update() override;
	void							UpdateLayers();
	void							Draw(Graphics* g) override;
	void							DrawBackdrop(Graphics* g);
	void							ButtonPress  	(int) override{}
	void							ButtonDepress	(int) override{}
	void							ButtonDownTick	(int) override{}
	void							ButtonMouseEnter(int) override{}
	void							ButtonMouseLeave(int) override{}
	void							ButtonMouseMove(int, int, int) override{}
	void					AddSunMoney(int theAmount);
	bool							TakeSunMoney(int theAmount);
	bool					CanTakeSunMoney(int theAmount);
	void					Pause(bool thePause);
	inline bool						MakeEasyZombieType() { /* not found */return false; }
	void							TryToSaveGame();
	bool					NeedSaveGame();
	bool					RowCanHaveZombies(int theRow);
	void							ProcessDeleteQueue();
	bool							ChooseSeedsOnCurrentLevel();
	int								GetNumSeedsInBank();
	bool					StageIsNight();
	bool					StageHasPool();
	bool					StageHas6Rows();
	bool					StageHasFog();
	bool					StageIsDayWithoutPool();
	bool					StageIsDayWithPool();
	bool							StageHasGraveStones();
	int								PixelToGridX(int theX, int theY);
	int								PixelToGridY(int theX, int theY);
	int					GridToPixelX(int theGridX, int theGridY);
	int								GridToPixelY(int theGridX, int theGridY);
	int					PixelToGridXKeepOnBoard(int theX, int theY);
	int					PixelToGridYKeepOnBoard(int theX, int theY);
	void							UpdateGameObjects();
	bool							MouseHitTest(int x, int y, HitResult* theHitResult);
	void							MouseDownWithPlant(int x, int y, int theClickCount);
	void							MouseDownWithTool(int x, int y, int theClickCount, CursorType theCursorType);
//	inline void						MouseDownNormal(int x, int y, int theClickCount) { /* not found */; }
	bool							CanInteractWithBoardButtons();
	void							DrawProgressMeter(Graphics* g);
	void							UpdateToolTip(const HitResult* theHitResult = nullptr);
	Plant*							GetTopPlantAt(int theGridX, int theGridY, PlantPriority thePriority);
	void							GetPlantsOnLawn(int theGridX, int theGridY, PlantsOnLawn* thePlantOnLawn);
	int					CountSunFlowers();
	int								GetSeedPacketPositionX(int theIndex);
	void							AddGraveStones(int theGridX, int theCount, MTRand& theLevelRNG);
	int								GetGraveStoneCount();
	void							ZombiesWon(Zombie* theZombie = nullptr);
	void							DrawLevel(Graphics* g);
	void							DrawShovel(Graphics* g);
	void							UpdateZombieSpawning();
	void							UpdateSunSpawning();
	void					ClearAdvice(AdviceType theHelpIndex);
	bool							RowCanHaveZombieType(int theRow, ZombieType theZombieType);
	int					NumberZombiesInWave(int theWaveIndex);
	int								TotalZombiesHealthInWave(int theWaveIndex);
	void							DrawDebugText(Graphics* g);
	void							DrawUICoinBank(Graphics* g);
	void					ShowCoinBank(int theDuration = 1000);
	void							FadeOutLevel();
	void							DrawFadeOut(Graphics* g);
	void							DrawIce(Graphics* g, int theGridY);
	bool							IsIceAt(int theGridX, int theGridY);
	ZombieID				ZombieGetID(Zombie* theZombie);
	Zombie*				ZombieGet(ZombieID theZombieID);
	Zombie*				ZombieTryToGet(ZombieID theZombieID);
	void							DrawDebugObjectRects(Graphics* g);
	void							UpdateIce();
	int					GetIceZPos(int theRow);
	bool					CanAddBobSled();
	void					ShakeBoard(int theShakeAmountX, int theShakeAmountY);
	int								CountUntriggerLawnMowers();
	Zombie*				AddZombieInRow(ZombieType theZombieType, int theRow, int theFromWave);
	bool					IsPoolSquare(int theGridX, int theGridY);
	void							PickZombieWaves();
	void							StopAllZombieSounds();
	bool					HasLevelAwardDropped();
	void							UpdateProgressMeter();
	void							DrawUIBottom(Graphics* g);
	void							DrawUITop(Graphics* g);
	Zombie*							ZombieHitTest(int theMouseX, int theMouseY);
	void							KillAllPlantsInRadius(int theX, int theY, int theRadius);
	Plant*							GetPumpkinAt(int theGridX, int theGridY);
	Plant*							GetFlowerPotAt(int theGridX, int theGridY);
	static bool						CanZombieSpawnOnLevel(ZombieType theZombieType, int theLevel);
	bool							IsZombieWaveDistributionOk();
	void							PickBackground();
	void							InitZombieWaves();
	void							InitSurvivalStage();
	static int			MakeRenderOrder(RenderLayer theRenderLayer, int theRow, int theLayerOffset);
	void							UpdateGame();
	void							InitZombieWavesForLevel(int theForLevel);
	unsigned int					SeedNotRecommendedForLevel(SeedType theSeedType);
	void							DrawTopRightUI(Graphics* g);
	void							DrawFog(Graphics* g);
	void							UpdateFog();
	int					LeftFogColumn();
	static bool			IsZombieTypePoolOnly(ZombieType theZombieType);
	void							DropLootPiece(int thePosX, int thePosY, int theDropFactor);
	void							UpdateLevelEndSequence();
	LawnMower*						GetBottomLawnMower();
	bool							CanDropLoot();
	ZombieType						GetIntroducedZombieType();
	void							PickSpecialGraveStone();
	float							GetPosYBasedOnRow(float thePosX, int theRow);
	void							NextWaveComing();
	bool							BungeeIsTargetingCell(int theGridX, int theGridY);
	int					PlantingPixelToGridX(int theX, int theY, SeedType theSeedType);
	int					PlantingPixelToGridY(int theX, int theY, SeedType theSeedType);
	Plant*							FindUmbrellaPlant(int theGridX, int theGridY);
	void							SetTutorialState(TutorialState theTutorialState);
	void							DoFwoosh(int theRow);
	void							UpdateFwoosh();
	Plant*							SpecialPlantHitTest(int x, int y);
	void							UpdateMousePosition();
	Plant*				ToolHitTestHelper(const HitResult* theHitResult);
	Plant*				ToolHitTest(int theX, int theY);
	bool							CanAddGraveStoneAt(int theGridX, int theGridY);
	void							UpdateGridItems();
	GridItem*			AddAGraveStone(int theGridX, int theGridY);
	int								GetSurvivalFlagsCompleted();
	bool							HasProgressMeter();
	void							UpdateCursor(const HitResult* theHitResult = nullptr);
	void							UpdateTutorial();
	SeedType						GetSeedTypeInCursor();
	int					CountPlantByType(SeedType theSeedType);
	bool							PlantingRequirementsMet(SeedType theSeedType);
	bool							HasValidCobCannonSpot();
	bool							IsValidCobCannonSpot(int theGridX, int theGridY);
	bool							IsValidCobCannonSpotHelper(int theGridX, int theGridY);
	void							MouseDownCobcannonFire(int x, int y, int theClickCount);
	int								KillAllZombiesInRadius(int theRow, int theX, int theY, int theRadius, int theRowRange, bool theBurn, int theDamageRangeFlags);
	int					GetSeedBankExtraWidth();
	bool							IsFlagWave(int theWaveNumber);
	void							DrawHouseDoorTop(Graphics* g);
	void							DrawHouseDoorBottom(Graphics* g);
	Zombie*							GetBossZombie();
	bool							HasConveyorBeltSeedBank();
	bool					StageHasRoof();
	void							SpawnZombiesFromPool();
	void							SpawnZombiesFromSky();
	void							PickUpTool(GameObjectType theObjectType);
	void							TutorialArrowShow(int theX, int theY);
	void							TutorialArrowRemove();
	int								CountCoinsBeingCollected();
	void							BungeeDropZombie(BungeeDropGrid* theBungeeDropGrid, ZombieType theZombieType);
	void							SetupBungeeDrop(BungeeDropGrid* theBungeeDropGrid);
	void					PutZombieInWave(ZombieType theZombieType, int theWaveNumber, ZombiePicker* theZombiePicker);
	void					PutInMissingZombies(int theWaveNumber, ZombiePicker* theZombiePicker);
	Rect							GetShovelButtonRect();
	void							GetZenButtonRect(GameObjectType theObjectType, Rect& theRect);
	Plant*							NewPlant(int theGridX, int theGridY, SeedType theSeedType, SeedType theImitaterType = SeedType::SEED_NONE);
	void							DoPlantingEffects(int theGridX, int theGridY, Plant* thePlant);
	bool							IsFinalSurvivalStage();
	void							SurvivalSaveScore();
	int								CountZombiesOnScreen();
	int								GetLiveGargantuarCount();
	int					GetNumWavesPerSurvivalStage();
	int								GetLevelRandSeed();
	void							AddBossRenderItem(RenderItem* theRenderList, int& theCurRenderItem, Zombie* theBossZombie);
	GridItem*			GetCraterAt(int theGridX, int theGridY);
	GridItem*			GetGraveStoneAt(int theGridX, int theGridY);
	GridItem*			GetLadderAt(int theGridX, int theGridY);
	GridItem*			AddALadder(int theGridX, int theGridY);
	GridItem*			AddACrater(int theGridX, int theGridY);
	void							InitLawnMowers();
	bool					IsPlantInCursor();
	void							HighlightPlantsForMouse(int theMouseX, int theMouseY, const HitResult* theHitResult);
	void							ClearFogAroundPlant(Plant* thePlant, int theSize);
	void					RemoveParticleByType(ParticleEffect theEffectType);
	GridItem*			GetScaryPotAt(int theGridX, int theGridY);
	void							PuzzleSaveStreak();
	void					ClearAdviceImmediately();
	bool					IsFinalScaryPotterStage();
	void					DisplayAdviceAgain(std::string_view theAdvice, MessageStyle theMessageStyle, AdviceType theHelpIndex);
	GridItem*						GetZenToolAt(int theGridX, int theGridY);
	bool							IsPlantInGoldWateringCanRange(int theMouseX, int theMouseY, Plant* thePlant);
	bool							StageHasZombieWalkInFromRight();
	void							PlaceRake();
	GridItem*						GetRake();
	bool					IsScaryPotterDaveTalking();
	Zombie*				GetWinningZombie();
	void					ResetFPSStats();
	int								CountEmptyPotsOrLilies(SeedType theSeedType);
	GridItem*						GetGridItemAt(GridItemType theGridItemType, int theGridX, int theGridY);
	bool							ProgressMeterHasFlags();
	bool					IsLastStandFinalStage();
	int					GetNumWavesPerFlag();
	int								GetCurrentPlantCost(SeedType theSeedType, SeedType theImitaterType);
	bool					PlantUsesAcceleratedPricing(SeedType theSeedType);
	void							FreezeEffectsForCutscene(bool theFreeze);
	void							LoadBackgroundImages();
	bool							CanUseGameObject(GameObjectType theGameObject);
	void							SetMustacheMode(bool theEnableMustache);
	int								CountCoinByType(CoinType theCoinType);
	void							SetSuperMowerMode(bool theEnableSuperMower);
	void							DrawZenWheelBarrowButton(Graphics* g, int theOffsetY);
	void							DrawZenButtons(Graphics* g);
	void					OffsetYForPlanting(int& theY, SeedType theSeedType);
	void							SetDanceMode(bool theEnableDance);
	void							SetFutureMode(bool theEnableFuture);
	void							SetPinataMode(bool theEnablePinata);
	void							SetDaisyMode(bool theEnableDaisy);
	void							SetSukhbirMode(bool theEnableSukhbir);
	bool							MouseHitTestPlant(int x, int y, HitResult* theHitResult);

	Reanimation*			CreateRakeReanim(float theRakeX, float theRakeY, int theRenderOrder);
	void							CompleteEndLevelSequenceForSaving();
	void							RemoveZombiesForRepick();
	int								GetGraveStonesCount();
	bool					IsSurvivalStageWithRepick();
	bool					IsLastStandStageWithRepick();
	void							DoTypingCheck(KeyCode theKey);
	int								CountZombieByType(ZombieType theZombieType);
	static bool			IsZombieTypeSpawnedOnly(ZombieType theZombieType);
};
extern bool gShownMoreSunTutorial;

int									GetRectOverlap(const Rect& rect1, const Rect& rect2);
bool								GetCircleRectOverlap(int theCircleX, int theCircleY, int theRadius, const Rect& theRect);
void						BoardInitForPlayer();

#endif // __BOARD_H__
