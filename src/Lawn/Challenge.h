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

#ifndef __CHALLENGE_H__
#define __CHALLENGE_H__

#include <cstdint>
#include "../ConstEnums.h"
#include "../GameConstants.h"
#include "../PvzpLib/FilterEffect.h"
#include "graphics/Graphics.h"
#include "Board.h"

constexpr const int BEGHOULED_MAX_GRIDSIZEX = 8;
constexpr const int BEGHOULED_MAX_GRIDSIZEY = 5;
constexpr const int ART_CHALLEGE_SIZE_X = MAX_GRID_SIZE_X;
constexpr const int MAX_PICK_GRID_SIZE = 50;

using namespace Sexy;

class LawnApp;
class Board;
class Plant;
class Zombie;
class GridItem;
class SeedPacket;
class HitResult;
struct PvzpWeightedGridArray;

enum BeghouledUpgrade : int32_t
{
	BEGHOULED_UPGRADE_REPEATER,
	BEGHOULED_UPGRADE_FUMESHROOM,
	BEGHOULED_UPGRADE_TALLNUT,
	NUM_BEGHOULED_UPGRADES
};

struct BeghouledBoardState
{
	SeedType		        mSeedType[9][6];
};

class Challenge
{
public:
	LawnApp*				mApp;
	Board*					mBoard;
	int32_t					mBeghouledMouseCapture;
	int32_t                 mBeghouledMouseDownX;
	int32_t                 mBeghouledMouseDownY;
	int32_t                mBeghouledEated[9][6];
	int32_t                mBeghouledPurcasedUpgrade[NUM_BEGHOULED_UPGRADES];
	int32_t                 mBeghouledMatchesThisMove;
	ChallengeState          mChallengeState;
	int32_t                 mChallengeStateCounter;
	int32_t                 mConveyorBeltCounter;
	int32_t                 mChallengeScore;
	int32_t                mShowBowlingLine;
	SeedType                mLastConveyorSeedType;
	int32_t                 mSurvivalStage;
	int32_t                 mSlotMachineRollCount;
	ReanimationID           mReanimChallenge;
	ReanimationID           mReanimClouds[6];
	int32_t                 mCloudsCounter[6];
	int32_t                 mChallengeGridX;
	int32_t                 mChallengeGridY;
	int32_t                 mScaryPotterPots;
	int32_t                 mRainCounter;
	int32_t                 mTreeOfWisdomTalkIndex;

public:
	Challenge();

	void                    StartLevel();
	void                    BeghouledPopulateBoard();
	void                    LoadBeghouledBoardState(BeghouledBoardState* theState);
	SeedType                BeghouledPickSeed(int theGridX, int theGridY, BeghouledBoardState* theBoardState, int theAllowMatches);
	int                    BeghouledBoardHasMatch(BeghouledBoardState* theBoardState);
	SeedType     BeghouledGetPlantAt(int theGridX, int theGridY, BeghouledBoardState* theBoardState);
	int                     BeghouledVerticalMatchLength(int theGridX, int theGridY, BeghouledBoardState* theBoardState);
	int                     BeghouledHorizontalMatchLength(int theGridX, int theGridY, BeghouledBoardState* theBoardState);
	void         BeghouledDragStart(int x, int y);
	void                    BeghouledDragUpdate(int x, int y);
	inline void             BeghouledDragCancel() { mBeghouledMouseCapture = false; }
	int                    MouseMove(int x, int y);
	int                    MouseDown(int x, int y, int theClickCount, HitResult* theHitResult);
	int                    MouseUp(int x, int y);
	void                    ClearCursor();
	void                    BeghouledRemoveHorizontalMatch(int theGridX, int theGridY, BeghouledBoardState* theBoardState);
	void                    BeghouledRemoveVerticalMatch(int theGridX, int theGridY, BeghouledBoardState* theBoardState);
	void                    BeghouledRemoveMatches(BeghouledBoardState* theBoardState);
	void                    Update();
	void                    UpdateBeghouled();
	int                    UpdateBeghouledPlant(Plant* thePlant);
	void                    BeghouledFallIntoSquare(int theGridX, int theGridY, BeghouledBoardState* theBoardState);
	void                    BeghouledMakePlantsFall(BeghouledBoardState* theBoardState);
	void                    ZombieAtePlant(/*Zombie* theZombie,*/ Plant* thePlant);
	void                    DrawBackdrop(Graphics* g);
	void                    DrawArtChallenge(Graphics* g);
	void                    CheckForCompleteArtChallenge(int theGridX, int theGridY);
	SeedType     GetArtChallengeSeed(int theGridX, int theGridY);
	void                    PlantAdded(Plant* thePlant);
	PlantingReason          CanPlantAt(int theGridX, int theGridY, SeedType theSeedType);
	void                    DrawBeghouled(Graphics* g);
	int                    BeghouledIsValidMove(int theFromX, int theFromY, int theToX, int theToY, BeghouledBoardState* theBoardState);
	int                    BeghouledCheckForPossibleMoves(BeghouledBoardState* theBoardState);
	void                    BeghouledCheckStuckState();
	void                    InitZombieWavesSurvival();
	void         InitZombieWavesFromList(ZombieType* theZombieList, int theListLength);
	void                    InitZombieWaves();
	Rect         SlotMachineGetHandleRect();
	void                    UpdateSlotMachine();
	void                    DrawSlotMachine(Graphics* g);
	int                    UpdateToolTip(int theX, int theY, const HitResult* theHitResult);
	void                    WhackAZombieSpawning();
	bool                   UpdateZombieSpawning();
	void                    BeghouledClearCrater(int theCount);
	void                    MouseDownWhackAZombie(int theX, int theY);
	void                    DrawStormNight(Graphics* g);
	void                    UpdateStormyNight();
	void                    InitLevel();
	void                    SpawnZombieWave();
	void                    GraveDangerSpawnRandomGrave();
	void                    GraveDangerSpawnGraveAt(int theGridX, int theGridY);
	void                    SpawnLevelAward(int theGridX, int theGridY);
	void                    BeghouledScore(int theGridX, int theGridY, int theNumPlants, int theIsHorizontal);
	void                    DrawStormFlash(Graphics* g, int theTime, int theMaxAmount);
	void                    UpdateRainingSeeds();
	void         PlayBossEnter();
	void                    UpdateConveyorBelt();
	void                    PortalStart();
	void                    UpdatePortalCombat();
	GridItem*               GetOtherPortal(GridItem* thePortal);
	void                    UpdatePortal(GridItem* thePortal);
	float                   PortalCombatRowSpawnWeight(int theGridY);
	int                    CanTargetZombieWithPortals(Plant* thePlant, Zombie* theZombie);
	GridItem*               GetPortalToRight(int theGridX, int theGridY);
	GridItem*               GetPortalAt(int theGridX, int theGridY);
	void                    MoveAPortal();
	int                     GetPortalDistanceToMower(int theGridY);
	GridItem*               GetPortalToLeft(int theGridX, int theGridY);
	void                    BeghouledPacketClicked(SeedPacket* theSeedPacket);
	void                    BeghouledShuffle();
	int         BeghouledCanClearCrater();
	void                    BeghouledUpdateCraters();
	Zombie*                 ZombiquariumSpawnSnorkle();
	void                    ZombiquariumPacketClicked(SeedPacket* theSeedPacket);
	void                    ZombiquariumMouseDown(int x, int y);
	void                    ZombiquariumDropBrain(int x, int y);
	void                    ZombiquariumUpdate();
	void         ShovelAddWallnuts();
	void                    ScaryPotterPlacePot(ScaryPotType theScaryPotType, ZombieType theZombieType, SeedType theSeedType, int theCount, PvzpWeightedGridArray* theGridArray, int theGridArrayCount);
	void                    ScaryPotterStart();
	void                    ScaryPotterUpdate();
	void                    ScaryPotterOpenPot(GridItem* theScaryPot);
	void                    ScaryPotterJackExplode(int thePosX, int thePosY);
	int                    ScaryPotterIsCompleted();
	void                    ScaryPotterChangePotType(GridItemState thePotType, int theCount);
	void                    ScaryPotterPopulate();
	void         ScaryPotterDontPlaceInCol(int theCol, PvzpWeightedGridArray* theGridArray, int theGridArrayCount);
	void                    ScaryPotterFillColumnWithPlant(int theCol, SeedType theSeedType, PvzpWeightedGridArray* theGridArray, int theGridArrayCount);
	void                    PuzzleNextStageClear();
	void                    ScaryPotterMalletPot(GridItem* theScaryPot);
	static ZombieType       IZombieSeedTypeToZombieType(SeedType theSeedType);
	static int  IsZombieSeedType(SeedType theSeedType);
	void                    IZombieMouseDownWithZombie(int theX, int theY, int theClickCount);
	void                    IZombieStart();
	void                    IZombiePlacePlants(SeedType theSeedType, int theCount, int theGridY = -1);
	void                    IZombieUpdate();
	void                    IZombieDrawPlant(Graphics* g, Plant* thePlant);
	void                    IZombieSetPlantFilterEffect(Plant* thePlant, FilterEffect theFilterEffect);
	int          ScaryPotterCountSunInPot(GridItem* theScaryPot);
	int                     ScaryPotterCountPots();
	void                    IZombieInitLevel();
	void                    DrawRain(Graphics* g);
	void                    DrawWeather(Graphics* g);
	void                    SquirrelUpdate();
	int          SquirrelCountUncaught();
	void                    SquirrelStart();
	void                    SquirrelFound(GridItem* theSquirrel);
	void                    SquirrelPeek(GridItem* theSquirrel);
	void                    SquirrelChew(GridItem* theSquirrel);
	void                    SquirrelUpdateOne(GridItem* theSquirrel);
	void                    IZombieSetupPlant(Plant* thePlant);
	void                    UpdateRain();
	int                    IZombieEatBrain(Zombie* theZombie);
	GridItem*               IZombieGetBrainTarget(Zombie* theZombie);
	void         IZombiePlacePlantInSquare(SeedType theSeedType, int theGridX, int theGridY = -1);
	void                    AdvanceCrazyDaveDialog();
	void                    BeghouledFlashPlant(int theFlashX, int theFlashY, int theFromX, int theFromY, int theToX, int theToY);
	void                    BeghouledFlashAMatch();
	int                    BeghouledFlashFromBoardState(BeghouledBoardState* theBoardState, int theFromX, int theFromY, int theToX, int theToY);
	void                    IZombiePlantDropRemainingSun(Plant* thePlant);
	void                    IZombieSquishBrain(GridItem* theBrain);
	void                    IZombieScoreBrain(GridItem* theBrain);
	void                    LastStandUpdate();
	void                    WhackAZombiePlaceGraves(int theGraveCount);
	int                    BeghouledTwistSquareFromMouse(int theX, int theY, int& theGridX, int& theGridY);
	int                    BeghouledTwistValidMove(int theGridX, int theGridY, BeghouledBoardState* theBoardState);
	void                    BeghouledTwistMouseDown(int x, int y);
	int                    BeghouledTwistMoveCausesMatch(int theGridX, int theGridY, BeghouledBoardState* theBoardState);
	int                    BeghouledTwistFlashMatch(BeghouledBoardState* theBoardState, int theGridX, int theGridY);
	void         BeghouledCancelMatchFlashing();
	void                    BeghouledStartFalling(ChallengeState theState);
	void                    BeghouledFillHoles(BeghouledBoardState* theBoardState, int theAllowMatches);
	void         BeghouledMakeStartBoard();
	void                    BeghouledCreatePlants(BeghouledBoardState* theOldBoardState, BeghouledBoardState* theNewBoardState);
	void                    PuzzlePhaseComplete(int theGridX, int theGridY);
	int         PuzzleIsAwardStage();
	void                    IZombiePlaceZombie(ZombieType theZombieType, int theGridX, int theGridY);
	void                    WhackAZombieUpdate();
	void                    LastStandCompletedStage();
	void                    TreeOfWisdomUpdate();
	void                    TreeOfWisdomFertilize();
	void                    TreeOfWisdomInit();
	int         TreeOfWisdomMouseOn(int theX, int theY);
	int          TreeOfWisdomGetSize();
	void                    TreeOfWisdomDraw(Graphics* g);
	void         TreeOfWisdomNextGarden();
	void         TreeOfWisdomToolUpdate(GridItem* theZenTool);
	void                    TreeOfWisdomOpenStore();
	void                    TreeOfWisdomLeave();
	void                    TreeOfWisdomGrow();
	void         TreeOfWisdomTool(int theMouseX, int theMouseY);
	int                    TreeOfWisdomHitTest(int theX, int theY, HitResult* theHitResult);
	void                    TreeOfWisdomBabble();
	void                    TreeOfWisdomGiveWisdom();
	void                    TreeOfWisdomSayRepeat();
	int                    TreeOfWisdomCanFeed();

	GridItem*               GetPortalLeftRight(int theGridX, int theGridY, int theToLeft = true);
};

extern SeedType gArtChallengeWallnut[6][9];
extern SeedType gArtChallengeSunFlower[6][9];
extern SeedType gArtChallengeStarFruit[6][9];

class ZombieAllowedLevels
{
public:
	ZombieType                      mZombieType;
	int32_t                         mAllowedOnLevel[50];
};
extern int gZombieWaves[NUM_LEVELS];
extern ZombieAllowedLevels gZombieAllowedLevels[NUM_ZOMBIE_TYPES];

#endif
