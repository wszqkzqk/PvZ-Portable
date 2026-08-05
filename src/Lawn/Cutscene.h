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

#ifndef __CUTSCENE_H__
#define __CUTSCENE_H__

#include "misc/KeyCodes.h"
#include "Zombie.h"

using namespace Sexy;
class LawnApp;
class Board;
class ChallengeScreen;

class CutScene
{
public:
	LawnApp*                    mApp;
	Board*                      mBoard;
	int                         mCutsceneTime;
	int                         mSodTime;
	int                         mGraveStoneTime;
	int                         mReadySetPlantTime;
	int                         mFogTime;
	int                         mBossTime;
	int                         mCrazyDaveTime;
	int                         mLawnMowerTime;
	int                         mCrazyDaveDialogStart;
	bool                        mSeedChoosing;
	ReanimationID               mZombiesWonReanimID;
	bool                        mPreloaded;
	bool                        mPlacedZombies;
	bool                        mPlacedLawnItems;
	int                         mCrazyDaveCountDown;              //+0x38 Countdown for Crazy Dave's upsell dialog
	int                         mCrazyDaveLastTalkIndex;          //+0x3C Index of Crazy Dave's upsell dialog lines
	bool                        mUpsellHideBoard;                 //+0x40 Whether to hide the board during upsell
	ChallengeScreen*            mUpsellChallengeScreen;           //+0x44 The challenge screen inserted during upsell
	bool                        mPreUpdatingBoard;                //+0x48 Whether the board is pre-updating during scene setup
	std::vector<std::string> mLoadedResourceNames;

public:
	CutScene();
	~CutScene();

	void                        StartLevelIntro();
	void                        CancelIntro();
	void                        Update();
	void                        AnimateBoard();
	void             StartSeedChooser();
	void             EndSeedChooser();
	int              CalcPosition(int theTimeStart, int theTimeEnd, int thePositionStart, int thePositionEnd);
	void                        PlaceStreetZombies();
	void                        AddGraveStoneParticles();
	void                        PlaceAZombie(ZombieType theZombieType, int theGridX, int theGridY);
	bool                        CanZombieGoInGridSpot(ZombieType theZombieType, int theGridX, int theGridY, bool theZombieGrid[5][5]);
	bool             IsSurvivalRepick();
	bool             IsAfterSeedChooser();
	void                        AddFlowerPots();
	void                        UpdateZombiesWon();
	void                        StartZombiesWon();
	bool             ShowZombieWalking();
	bool             IsCutSceneOver();
	void                        ZombieWonClick();
	void                        MouseDown(int theX, int theY);
	void                        KeyDown(KeyCode theKey);
	void             AdvanceCrazyDaveDialog(bool theJustSkipping);
	void                        ShowShovel();
	bool                        CanGetPacketUpgrade();
	bool                        CanGetPacketUpgrade(int theIndex);
	void                        FindPlaceForStreetZombies(ZombieType theZombieType, bool theZombieGrid[5][5], int& thePosX, int& thePosY);
	void                        FindAndPlaceZombie(ZombieType theZombieType, bool theZombieGrid[5][5]);
	static bool      Is2x2Zombie(ZombieType theZombieType);
	void                        PreloadResources();
	bool             IsBeforePreloading();
	bool             IsShowingCrazyDave();
	bool                        IsNonScrollingCutscene();
	bool                        IsScrolledLeftAtStart();
	bool             IsInShovelTutorial();
	void                        PlaceLawnItems();
	bool                        CanGetSecondPacketUpgrade();
	int                         ParseDelayTimeFromMessage();
	int                         ParseTalkTimeFromMessage();
	void                        ClearUpsellBoard();
	void                        LoadIntroBoard();
	void             AddUpsellZombie(ZombieType theZombieType, int thePixelX, int theGridY);
	void                        LoadUpsellBoardPool();
	void                        LoadUpsellBoardFog();
	void                        LoadUpsellChallengeScreen();
	void                        LoadUpsellBoardRoof();
	void                        UpdateUpsell();
	void                        DrawUpsell(Graphics* g);
	void                        UpdateIntro();
	void                        DrawIntro(Graphics* g);
	bool             ShouldRunUpsellBoard();
};

#endif
