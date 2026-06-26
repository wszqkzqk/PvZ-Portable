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

#include "Coin.h"
#include "Board.h"
#include "Plant.h"
#include "Zombie.h"
#include "GridItem.h"
#include "Cutscene.h"
#include "ZenGarden.h"
#include "LawnMower.h"
#include "Challenge.h"
#include "SeedPacket.h"
#include "Projectile.h"
#include "../LawnApp.h"
#include "../Resources.h"
#include "CursorObject.h"
#include "System/Music.h"
#include "ToolTipWidget.h"
#include "MessageWidget.h"
#include "../GameConstants.h"
#include "System/PlayerInfo.h"
#include "Widget/GameButton.h"
#include "Widget/StoreScreen.h"
#include "../Sexy.TodLib/TodDebug.h"
#include "../Sexy.TodLib/TodFoley.h"
#include "../Sexy.TodLib/TodCommon.h"
#include "graphics/Font.h"
#include "../Sexy.TodLib/Reanimator.h"
#include "misc/Debug.h"
#include "misc/MTRand.h"
#include "../Sexy.TodLib/TodParticle.h"
#include "../Sexy.TodLib/TodStringFile.h"
#include "widget/WidgetManager.h"
#include "Widget/AchievementsScreen.h"

int gZombieWaves[NUM_LEVELS] = {
	4,  6,  8,  10, 8,  10, 20, 10, 20, 20,
	10, 20, 10, 20, 10, 10, 20, 10, 20, 20,
	10, 20, 20, 30, 20, 20, 30, 20, 30, 30,
	10, 20, 10, 20, 20, 10, 20, 10, 20, 20,
	10, 20, 20, 30, 20, 20, 30, 20, 30, 30,
};

ZombieAllowedLevels gZombieAllowedLevels[NUM_ZOMBIE_TYPES] = {
	{ ZOMBIE_NORMAL,
		{
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		}
	},
	{ ZOMBIE_FLAG, 
		{
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		}
	},
	{ ZOMBIE_TRAFFIC_CONE,
		{
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
			0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		}
	},
	{ ZOMBIE_POLEVAULTER,
		{
			0, 0, 0, 0, 0, 1, 1, 0, 1, 1,
			0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
			0, 0, 0, 1, 0, 0, 0, 0, 1, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_PAIL,
		{
			0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
			0, 1, 0, 0, 1, 0, 0, 0, 0, 0,
			0, 1, 0, 1, 0, 0, 1, 0, 1, 1,
			0, 0, 0, 0, 0, 0, 1, 0, 1, 1,
			0, 1, 0, 0, 1, 0, 0, 0, 1, 1,
		}
	},
	{ ZOMBIE_NEWSPAPER,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			1, 1, 0, 0, 1, 0, 0, 0, 0, 0,
			0, 1, 0, 1, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_DOOR,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_FOOTBALL,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 1, 1, 0, 0, 1,
			0, 1, 0, 0, 1, 0, 0, 0, 0, 0,
			0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_DANCER,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_BACKUP_DANCER,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		}
	},
	{ ZOMBIE_DUCKY_TUBE, { 0 } },
	{ ZOMBIE_SNORKEL,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 1, 1, 1, 0, 1, 0, 0, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_ZAMBONI,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 1, 1, 0, 1, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_BOBSLED,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 1, 1, 0, 1, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_DOLPHIN_RIDER,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
			0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_JACK_IN_THE_BOX,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			1, 1, 0, 0, 0, 0, 1, 0, 0, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
		}
	},
	{ ZOMBIE_BALLOON,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 1, 1, 0, 0, 0, 0, 1, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_DIGGER,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 1, 1, 0, 0, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_POGO,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
			0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
		}
	},
	{ ZOMBIE_YETI, {0} },
	{ ZOMBIE_BUNGEE,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			1, 1, 0, 0, 0, 0, 1, 0, 1, 1,
		}
	},
	{ ZOMBIE_LADDER,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 1, 1, 1, 0, 1, 0, 1, 1,
		}
	},
	{ ZOMBIE_CATAPULT,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 1, 1, 0, 1, 1,
		}
	},
	{ ZOMBIE_GARGANTUAR,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
		}
	},
	{ ZOMBIE_IMP,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
		}
	},
	{ ZOMBIE_BOSS, {0} },
	{ ZOMBIE_PEA_HEAD, {0} },
	{ ZOMBIE_WALLNUT_HEAD, {0} },
	{ ZOMBIE_JALAPENO_HEAD, {0} },
	{ ZOMBIE_GATLING_HEAD, {0} },
	{ ZOMBIE_SQUASH_HEAD, {0} },
	{ ZOMBIE_TALLNUT_HEAD, {0} },
	{ ZOMBIE_REDEYE_GARGANTUAR, {0} },
};

SeedType gArtChallengeWallnut[MAX_GRID_SIZE_Y][MAX_GRID_SIZE_X] = {
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_WALLNUT,   SEED_WALLNUT,   SEED_WALLNUT,   SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_WALLNUT,   SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_WALLNUT,   SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_WALLNUT,   SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_WALLNUT,   SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_WALLNUT,   SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_WALLNUT,   SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_WALLNUT,   SEED_WALLNUT,   SEED_WALLNUT,   SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE}
};

SeedType gArtChallengeSunFlower[MAX_GRID_SIZE_Y][MAX_GRID_SIZE_X] = {
	{SEED_NONE,     SEED_NONE,      SEED_STARFRUIT, SEED_STARFRUIT, SEED_STARFRUIT, SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_STARFRUIT, SEED_WALLNUT,   SEED_WALLNUT,   SEED_WALLNUT,   SEED_STARFRUIT, SEED_NONE,      SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_STARFRUIT, SEED_STARFRUIT, SEED_STARFRUIT, SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_UMBRELLA,  SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_UMBRELLA,  SEED_UMBRELLA,  SEED_UMBRELLA,  SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE}
};

SeedType gArtChallengeStarFruit[MAX_GRID_SIZE_Y][MAX_GRID_SIZE_X] = {
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_STARFRUIT, SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_STARFRUIT, SEED_STARFRUIT, SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_STARFRUIT, SEED_STARFRUIT, SEED_STARFRUIT, SEED_STARFRUIT, SEED_STARFRUIT, SEED_STARFRUIT, SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_STARFRUIT, SEED_STARFRUIT, SEED_STARFRUIT, SEED_NONE,      SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_STARFRUIT, SEED_NONE,      SEED_NONE,      SEED_STARFRUIT, SEED_NONE,      SEED_NONE},
	{SEED_NONE,     SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE,      SEED_NONE}
};

Challenge::Challenge()
{
	mApp = (LawnApp*)gSexyAppBase;
	mBoard = mApp->mBoard;
	mBeghouledMouseCapture = false;
	mBeghouledMouseDownX = 0;
	mBeghouledMouseDownY = 0;
	mChallengeStateCounter = 0;
	mConveyorBeltCounter = 0;
	mChallengeScore = 0;
	mChallengeState = STATECHALLENGE_NORMAL;
	mShowBowlingLine = false;
	mLastConveyorSeedType = SEED_NONE;
	mSurvivalStage = 0;
	mSlotMachineRollCount = 0;
	mReanimChallenge = REANIMATIONID_NULL;
	mChallengeGridX = 0;
	mChallengeGridY = 0;
	mScaryPotterPots = 0;
	mBeghouledMatchesThisMove = 0;
	mRainCounter = 0;
	mTreeOfWisdomTalkIndex = 0;
	for (int i = 0; i < 6; i++)
		mReanimClouds[i] = REANIMATIONID_NULL;
	memset(mBeghouledEated, 0, sizeof(mBeghouledEated));
	for (int i = 0; i < static_cast<int>(BeghouledUpgrade::NUM_BEGHOULED_UPGRADES); i++)
		mBeghouledPurcasedUpgrade[i] = false;

	if (mApp->mBoard && mApp->mGameMode == GAMEMODE_CHALLENGE_SLOT_MACHINE)
	{
		Rect aHandleRect = SlotMachineGetHandleRect();
		ReanimatorEnsureDefinitionLoaded(REANIM_SLOT_MACHINE_HANDLE, true);
		Reanimation* aHandleReanim = mApp->AddReanimation(aHandleRect.mX - 377, aHandleRect.mY - 20, 0, REANIM_SLOT_MACHINE_HANDLE);
		aHandleReanim->mIsAttachment = true;
		aHandleReanim->mAnimRate = 0;
		mReanimChallenge = mApp->ReanimationGetID(aHandleReanim);
	}
}

void Challenge::LoadBeghouledBoardState(BeghouledBoardState* theBoardState)
{
	for (int i = 0; i < MAX_GRID_SIZE_X; i++)
		for (int j = 0; j < MAX_GRID_SIZE_Y; j++)
			theBoardState->mSeedType[i][j] = SEED_NONE;

	Plant* aPlant = nullptr;
	while (mBoard->IteratePlants(aPlant))
		theBoardState->mSeedType[aPlant->mPlantCol][aPlant->mRow] = aPlant->mSeedType;
}

void Challenge::InitLevel()
{
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_RAINING_SEEDS)
	{
		mChallengeStateCounter = 100;
		mApp->PlayFoley(FOLEY_RAIN);
	}
	if (mApp->IsStormyNightLevel())
	{
		mChallengeState = STATECHALLENGE_STORM_FLASH_2;
		mChallengeStateCounter = 100;
		mApp->PlayFoley(FOLEY_RAIN);
	}
	if (mApp->IsFinalBossLevel())
	{
		mBoard->mSeedBank->AddSeed(SEED_CABBAGEPULT);
		mBoard->mSeedBank->AddSeed(SEED_JALAPENO);
		mBoard->mSeedBank->AddSeed(SEED_CABBAGEPULT);
		mBoard->mSeedBank->AddSeed(SEED_ICESHROOM);
		mConveyorBeltCounter = 1000;
	}
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		mApp->mZenGarden->mGardenType = GARDEN_MAIN;
		mApp->mZenGarden->ZenGardenInitLevel();
	}
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_COLUMN)
	{
		mBoard->mSeedBank->AddSeed(SEED_POTATOMINE);
		mBoard->mSeedBank->AddSeed(SEED_TALLNUT);
		mBoard->mSeedBank->AddSeed(SEED_MELONPULT);
		mBoard->mSeedBank->AddSeed(SEED_MAGNETSHROOM);
		mBoard->mSeedBank->AddSeed(SEED_INSTANT_COFFEE);
		mBoard->mSeedBank->AddSeed(SEED_MELONPULT);
		mConveyorBeltCounter = 1000;
	}
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_INVISIGHOUL)
	{
		mBoard->mSeedBank->AddSeed(SEED_PEASHOOTER);
		mBoard->mSeedBank->AddSeed(SEED_ICESHROOM);
		mConveyorBeltCounter = 1000;
	}
	if (mApp->IsIZombieLevel())
	{
		IZombieInitLevel();
	}
	if (mApp->IsScaryPotterLevel())
	{
		ScaryPotterPopulate();
	}
	if (mApp->IsFirstTimeAdventureMode() && mBoard->mLevel == 5)
	{
		mBoard->NewPlant(5, 1, SEED_PEASHOOTER, SEED_NONE);
		mBoard->NewPlant(7, 2, SEED_PEASHOOTER, SEED_NONE);
		mBoard->NewPlant(6, 3, SEED_PEASHOOTER, SEED_NONE);
	}
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_BEGHOULED_TWIST)
	{
		mChallengeGridX = -1;
		mChallengeGridY = -1;
	}
	if (mApp->mGameMode == GAMEMODE_TREE_OF_WISDOM)
	{
		TreeOfWisdomInit();
	}
}

void Challenge::StartLevel()
{
	if (mApp->IsWhackAZombieLevel())
	{
		mBoard->mCursorObject->mCursorType = CURSOR_TYPE_HAMMER;
		mBoard->mZombieCountDown = 200;
		mBoard->mZombieCountDownStart = mBoard->mZombieCountDown;

		ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_HAMMER, true);
		mApp->RemoveReanimation(mBoard->mCursorObject->mReanimCursorID);
		Reanimation* aHammerReanim = mApp->AddReanimation(-25.0f, 16.0f, 0, ReanimationType::REANIM_HAMMER);
		aHammerReanim->mIsAttachment = true;
		aHammerReanim->PlayReanim("anim_whack_zombie", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 24.0f);
		aHammerReanim->mAnimTime = 1.0f;
		mBoard->mCursorObject->mReanimCursorID = mApp->ReanimationGetID(aHammerReanim);
	}
	if (mApp->IsStormyNightLevel())
	{
		mChallengeState = STATECHALLENGE_STORM_FLASH_1;
		mChallengeStateCounter = 400;
	}
	GameMode aGameMode = mApp->mGameMode;
	if (aGameMode == GAMEMODE_CHALLENGE_BOBSLED_BONANZA)
	{
		for (int i = 0; i < MAX_GRID_SIZE_Y; i++)
		{
			if (mBoard->mPlantRow[i] != PLANTROW_POOL)
			{
				mBoard->mIceMinX[i] = 400;
				mBoard->mIceTimer[i] = 0x7FFFFFFF;
			}
		}
	}
	if (mApp->IsWallnutBowlingLevel())
	{
		mBoard->mZombieCountDown = 200;
		mBoard->mZombieCountDownStart = mBoard->mZombieCountDown;
		mBoard->mSeedBank->AddSeed(SEED_WALLNUT);
		mConveyorBeltCounter = 400;
		mShowBowlingLine = true;
	}
	if (aGameMode == GAMEMODE_CHALLENGE_SHOVEL || aGameMode == GAMEMODE_CHALLENGE_SQUIRREL)
	{
		ShovelAddWallnuts();
	}
	if (mApp->IsScaryPotterLevel())
	{
		ScaryPotterStart();
	}
	if (mApp->IsLittleTroubleLevel() || mApp->IsStormyNightLevel() || mApp->IsBungeeBlitzLevel() || aGameMode == GAMEMODE_CHALLENGE_INVISIGHOUL)
	{
		mBoard->mZombieCountDown = 200;
		mBoard->mZombieCountDownStart = mBoard->mZombieCountDown;
		mConveyorBeltCounter = 200;
	}
	if (mApp->IsSurvivalMode() && mSurvivalStage == 0)
	{
		std::string aMessage =
			mApp->IsSurvivalNormal(aGameMode) ? TodReplaceNumberString("[ADVICE_SURVIVE_FLAGS]", "{FLAGS}", SURVIVAL_NORMAL_FLAGS) :
			mApp->IsSurvivalHard(aGameMode) ? TodReplaceNumberString("[ADVICE_SURVIVE_FLAGS]", "{FLAGS}", SURVIVAL_HARD_FLAGS) : 
			"[ADVICE_SURVIVE_ENDLESS]";
		mBoard->DisplayAdvice(aMessage, MESSAGE_STYLE_HINT_FAST, ADVICE_SURVIVE_FLAGS);
	}
	if (aGameMode == GAMEMODE_CHALLENGE_LAST_STAND && mSurvivalStage == 0)
	{
		mBoard->DisplayAdvice(TodReplaceNumberString("[ADVICE_SURVIVE_FLAGS]", "{FLAGS}", LAST_STAND_FLAGS), MESSAGE_STYLE_BIG_MIDDLE_FAST, ADVICE_SURVIVE_FLAGS);
	}
	if (aGameMode == GAMEMODE_CHALLENGE_ART_CHALLENGE_WALLNUT)
	{
		mBoard->DisplayAdvice("[ADVICE_FILL_IN_WALLNUTS]", MESSAGE_STYLE_HINT_FAST, ADVICE_NONE);
	}
	if (aGameMode == GAMEMODE_CHALLENGE_ART_CHALLENGE_SUNFLOWER)
	{
		mBoard->DisplayAdvice("[ADVICE_FILL_IN_SPACES]", MESSAGE_STYLE_HINT_FAST, ADVICE_NONE);
	}
	if (aGameMode == GAMEMODE_CHALLENGE_SEEING_STARS)
	{
		mBoard->DisplayAdvice("[ADVICE_FILL_IN_STARFRUIT]", MESSAGE_STYLE_HINT_FAST, ADVICE_NONE);
	}
	if (mApp->IsSlotMachineLevel())
	{
		mBoard->SetTutorialState(TUTORIAL_SLOT_MACHINE_PULL);
	}
	if (aGameMode == GAMEMODE_CHALLENGE_BEGHOULED || aGameMode == GAMEMODE_CHALLENGE_BEGHOULED_TWIST)
	{
		mBoard->mZombieCountDown = 200;
		mBoard->mZombieCountDownStart = mBoard->mZombieCountDown;
		mChallengeStateCounter = 1500;
		BeghouledMakeStartBoard();
		BeghouledUpdateCraters();

		if (aGameMode == GAMEMODE_CHALLENGE_BEGHOULED)
		{
			mBoard->DisplayAdvice("[ADVICE_BEGHOULED_DRAG_TO_MATCH_3]", MESSAGE_STYLE_HINT_FAST, ADVICE_NONE);
		}
		else if (aGameMode == GAMEMODE_CHALLENGE_BEGHOULED_TWIST)
		{
			mBoard->DisplayAdvice("[ADVICE_BEGHOULED_TWIST_TO_MATCH_3]", MESSAGE_STYLE_HINT_FAST, ADVICE_NONE);
		}
	}
	if (mApp->IsMiniBossLevel())
	{
		mBoard->mZombieCountDown = 100;
		mBoard->mZombieCountDownStart = mBoard->mZombieCountDown;
		mConveyorBeltCounter = 200;
	}
	if (aGameMode == GAMEMODE_CHALLENGE_PORTAL_COMBAT)
	{
		PortalStart();
	}
	if (aGameMode == GAMEMODE_CHALLENGE_COLUMN)
	{
		mBoard->mCurrentWave = 9;
		mBoard->mZombieCountDown = 2400;
	}
	if (aGameMode == GAMEMODE_CHALLENGE_AIR_RAID || aGameMode == GAMEMODE_CHALLENGE_BOBSLED_BONANZA)
	{
		mBoard->mZombieCountDown = 4500;
	}
	if (aGameMode == GAMEMODE_CHALLENGE_POGO_PARTY)
	{
		mBoard->mZombieCountDown = 5500;
	}
	if (aGameMode == GAMEMODE_CHALLENGE_ZOMBIQUARIUM)
	{
		mBoard->DisplayAdvice("[ADVICE_ZOMBIQUARIUM_CLICK_TO_FEED]", MESSAGE_STYLE_HINT_TALL_FAST, ADVICE_ZOMBIQUARIUM_CLICK_TO_FEED);
		ZombiquariumSpawnSnorkle();
		ZombiquariumSpawnSnorkle();
	}
	if (mApp->IsIZombieLevel())
	{
		IZombieStart();
	}
	/* Unused
	if (mApp->IsSquirrelLevel())
	{
		SquirrelStart();
	}*/
}

int Challenge::BeghouledTwistValidMove(int theGridX, int theGridY, BeghouledBoardState* theBoardState)
{
	if (theGridY == -1 || theGridX > 6 || theGridY > 3)
		return false;

	return 
		theBoardState->mSeedType[theGridX][theGridY] != SEED_NONE && 
		theBoardState->mSeedType[theGridX + 1][theGridY] != SEED_NONE &&
		theBoardState->mSeedType[theGridX][theGridY + 1] != SEED_NONE && 
		theBoardState->mSeedType[theGridX + 1][theGridY + 1] != SEED_NONE;
}

int Challenge::BeghouledTwistMoveCausesMatch(int theGridX, int theGridY, BeghouledBoardState* theBoardState)
{
	if (!BeghouledTwistValidMove(theGridX, theGridY, theBoardState))
		return false;

	SeedType aSeed1 = theBoardState->mSeedType[theGridX][theGridY];
	SeedType aSeed2 = theBoardState->mSeedType[theGridX + 1][theGridY];
	SeedType aSeed3 = theBoardState->mSeedType[theGridX][theGridY + 1];
	SeedType aSeed4 = theBoardState->mSeedType[theGridX + 1][theGridY + 1];

	theBoardState->mSeedType[theGridX + 1][theGridY] = aSeed1;
	theBoardState->mSeedType[theGridX + 1][theGridY + 1] = aSeed2;
	theBoardState->mSeedType[theGridX][theGridY + 1] = aSeed4;
	theBoardState->mSeedType[theGridX][theGridY] = aSeed3;

	int aHasMatch = BeghouledBoardHasMatch(theBoardState);

	theBoardState->mSeedType[theGridX][theGridY] = aSeed1;
	theBoardState->mSeedType[theGridX + 1][theGridY] = aSeed2;
	theBoardState->mSeedType[theGridX][theGridY + 1] = aSeed3;
	theBoardState->mSeedType[theGridX + 1][theGridY + 1] = aSeed4;
	
	return aHasMatch;
}

int Challenge::BeghouledTwistSquareFromMouse(int theX, int theY, int& theGridX, int& theGridY)
{
	theGridX = mBoard->PixelToGridX(theX - 40, theY - 40);
	theGridY = mBoard->PixelToGridY(theX - 40, theY - 40);
	if (theGridX == -1 || theGridY == -1 || theGridX > 6 || theGridY > 3)
	{
		theGridX = -1;
		theGridY = -1;
		return false;
	}
	return true;
}

void Challenge::BeghouledTwistMouseDown(int x, int y)
{
	if (mBoard->HasLevelAwardDropped())
		return;

	BeghouledBoardState aBoardState;
	LoadBeghouledBoardState(&aBoardState);

	int aGridX, aGridY;
	if (BeghouledTwistSquareFromMouse(x, y, aGridX, aGridY) && BeghouledTwistValidMove(aGridX, aGridY, &aBoardState))
	{
		Plant* aPlant1 = mBoard->GetTopPlantAt(aGridX, aGridY, TOPPLANT_ANY);
		Plant* aPlant2 = mBoard->GetTopPlantAt(aGridX + 1, aGridY, TOPPLANT_ANY);
		Plant* aPlant3 = mBoard->GetTopPlantAt(aGridX, aGridY + 1, TOPPLANT_ANY);
		Plant* aPlant4 = mBoard->GetTopPlantAt(aGridX + 1, aGridY + 1, TOPPLANT_ANY);
		if (!BeghouledTwistMoveCausesMatch(aGridX, aGridY, &aBoardState))
		{
			aPlant1->mX = mBoard->GridToPixelX(aPlant1->mPlantCol, aPlant1->mRow) + 20;
			aPlant2->mY = mBoard->GridToPixelY(aPlant2->mPlantCol, aPlant2->mRow) + 20;
			aPlant3->mY = mBoard->GridToPixelY(aPlant3->mPlantCol, aPlant3->mRow) - 20;
			aPlant4->mX = mBoard->GridToPixelX(aPlant4->mPlantCol, aPlant4->mRow) - 20;
			mApp->PlayFoley(FOLEY_FLOOP);
		}
		else
		{
			aPlant1->mPlantCol++;
			aPlant1->mRenderOrder = aPlant1->CalcRenderOrder();
			aPlant2->mRow++;
			aPlant2->mRenderOrder = aPlant2->CalcRenderOrder();
			aPlant3->mRow--;
			aPlant3->mRenderOrder = aPlant3->CalcRenderOrder();
			aPlant4->mPlantCol--;
			aPlant4->mRenderOrder = aPlant4->CalcRenderOrder();
			BeghouledStartFalling(STATECHALLENGE_BEGHOULED_MOVING);
		}
	}
}

void Challenge::BeghouledStartFalling(ChallengeState theState)
{
	mChallengeState = theState;
	mChallengeStateCounter = 100;
	mBoard->ClearAdvice(AdviceType::ADVICE_BEGHOULED_NO_MOVES);
}

int Challenge::BeghouledIsValidMove(int theFromX, int theFromY, int theToX, int theToY, BeghouledBoardState* theBoardState)
{
	if (theFromX < 0 || theFromX > BEGHOULED_MAX_GRIDSIZEX || theToX < 0 || theToX > BEGHOULED_MAX_GRIDSIZEX ||
		theFromY < 0 || theFromY > BEGHOULED_MAX_GRIDSIZEY || theToY < 0 || theToY > BEGHOULED_MAX_GRIDSIZEY ||
		mBeghouledEated[theFromX][theFromY] || mBeghouledEated[theToX][theToY])
		return false;

	SeedType aSeedFrom = theBoardState->mSeedType[theFromX][theFromY];
	SeedType aSeedTo = theBoardState->mSeedType[theToX][theToY];
	if (aSeedFrom == SEED_NONE)
		return false;

	theBoardState->mSeedType[theFromX][theFromY] = aSeedTo;
	theBoardState->mSeedType[theToX][theToY] = aSeedFrom;

	int aValid = BeghouledBoardHasMatch(theBoardState);

	theBoardState->mSeedType[theFromX][theFromY] = aSeedFrom;
	theBoardState->mSeedType[theToX][theToY] = aSeedTo;

	return aValid;
}

void Challenge::BeghouledDragUpdate(int x, int y)
{
	int aDeltaX = x - mBeghouledMouseDownX;
	int aDeltaY = y - mBeghouledMouseDownY;
	if (abs(aDeltaX) >= 10 || abs(aDeltaY) >= 10)
	{
		mBoard->ClearAdvice(ADVICE_BEGHOULED_DRAG_TO_MATCH_3);
		mBeghouledMouseCapture = false;

		int aGridXFrom = mBoard->PixelToGridX(mBeghouledMouseDownX, mBeghouledMouseDownY);
		int aGridYFrom = mBoard->PixelToGridY(mBeghouledMouseDownX, mBeghouledMouseDownY);
		int aGridXTo, aGridYTo;
		if (abs(aDeltaX) > abs(aDeltaY))
		{
			aGridXTo = aGridXFrom + (aDeltaX > 0 ? 1 : -1);
			aGridYTo = aGridYFrom;
		}
		else
		{
			aGridXTo = aGridXFrom;
			aGridYTo = aGridYFrom + (aDeltaY > 0 ? 1 : -1);
		}

		BeghouledBoardState aBoardState;
		LoadBeghouledBoardState(&aBoardState);
		Plant* aPlantFrom = mBoard->GetTopPlantAt(aGridXFrom, aGridYFrom, TOPPLANT_ANY);
		if (!BeghouledIsValidMove(aGridXFrom, aGridYFrom, aGridXTo, aGridYTo, &aBoardState))
		{
			if (aPlantFrom)
			{
				if (aGridXTo > aGridXFrom)			aPlantFrom->mX += 30;
				else if (aGridXTo < aGridXFrom)		aPlantFrom->mX -= 30;
				else if (aGridYTo > aGridYFrom)		aPlantFrom->mY += 30;
				else if (aGridYTo < aGridYFrom)		aPlantFrom->mY -= 30;

				mApp->PlayFoley(FOLEY_FLOOP);
			}
		}
		else
		{
			Plant* aPlantTo = mBoard->GetTopPlantAt(aGridXTo, aGridYTo, TOPPLANT_ANY);

			if (aPlantFrom)
			{
				aPlantFrom->mPlantCol = aGridXTo;
				aPlantFrom->mRow = aGridYTo;
				aPlantFrom->mRenderOrder = aPlantFrom->CalcRenderOrder();
			}

			if (aPlantTo)
			{
				aPlantTo->mPlantCol = aGridXFrom;
				aPlantTo->mRow = aGridYFrom;
				aPlantTo->mRenderOrder = aPlantTo->CalcRenderOrder();
			}

			BeghouledStartFalling(STATECHALLENGE_BEGHOULED_MOVING);
		}
	}
}

SeedType Challenge::BeghouledGetPlantAt(int theGridX, int theGridY, BeghouledBoardState* theBoardState)
{
	if (theGridX < 0 || theGridX > BEGHOULED_MAX_GRIDSIZEX || theGridY < 0 || theGridY > BEGHOULED_MAX_GRIDSIZEY)
		return SEED_NONE;

	return theBoardState->mSeedType[theGridX][theGridY];
}

void Challenge::BeghouledRemoveHorizontalMatch(int theGridX, int theGridY, BeghouledBoardState* theBoardState)
{
	SeedType aSeedType = BeghouledGetPlantAt(theGridX, theGridY, theBoardState);
	do
	{
		Plant* aPlant = mBoard->GetTopPlantAt(theGridX, theGridY, TOPPLANT_ANY);
		if (aPlant)
		{
			aPlant->Die();
		
		}
		theGridX++;
	} while (BeghouledGetPlantAt(theGridX, theGridY, theBoardState) == aSeedType);
}

void Challenge::BeghouledRemoveVerticalMatch(int theGridX, int theGridY, BeghouledBoardState* theBoardState)
{
	SeedType aSeedType = BeghouledGetPlantAt(theGridX, theGridY, theBoardState);
	do
	{
		Plant* aPlant = mBoard->GetTopPlantAt(theGridX, theGridY, TOPPLANT_ANY);
		if (aPlant)
		{
			aPlant->Die();
		}
		theGridY++;
	} while (BeghouledGetPlantAt(theGridX, theGridY, theBoardState) == aSeedType);
}

void Challenge::BeghouledFallIntoSquare(int theGridX, int theGridY, BeghouledBoardState* theBoardState)
{
	if (mBeghouledEated[theGridX][theGridY])
		return;

	for (int aGridY = theGridY - 1; aGridY >= 0; aGridY--)
	{
		Plant* aPlant = mBoard->GetTopPlantAt(theGridX, aGridY, TOPPLANT_ANY);
		if (aPlant)
		{
			aPlant->mRow = theGridY;
			aPlant->mRenderOrder = aPlant->CalcRenderOrder();
			theBoardState->mSeedType[theGridX][theGridY] = aPlant->mSeedType;
			theBoardState->mSeedType[theGridX][aGridY] = SEED_NONE;
			BeghouledStartFalling(STATECHALLENGE_BEGHOULED_FALLING);
			break;
		}
	}
}

void Challenge::BeghouledMakePlantsFall(BeghouledBoardState* theBoardState)
{
	for (int aGridY = BEGHOULED_MAX_GRIDSIZEY - 1; aGridY >= 0; aGridY--)
	{
		for (int aGridX = 0; aGridX < BEGHOULED_MAX_GRIDSIZEX; aGridX++)
		{
			if (BeghouledGetPlantAt(aGridX, aGridY, theBoardState) == SEED_NONE)
			{
				BeghouledFallIntoSquare(aGridX, aGridY, theBoardState);
			}
		}
	}
}

void Challenge::BeghouledUpdateCraters()
{
	if (mBoard->mSeedBank->mNumPackets != 5)
		return;

	SeedPacket* aSeedPacket = &mBoard->mSeedBank->mSeedPackets[4];
	TOD_ASSERT(aSeedPacket->mPacketType == SeedType::SEED_BEGHOULED_BUTTON_CRATER);
	aSeedPacket->SetActivate(BeghouledCanClearCrater());
}

void Challenge::BeghouledClearCrater(int theCount)
{
	mBoard->ClearAdvice(ADVICE_BEGHOULED_USE_CRATER_1);
	mBoard->ClearAdvice(ADVICE_BEGHOULED_USE_CRATER_2);

	for (int aGridX = 0; aGridX < MAX_GRID_SIZE_X; aGridX++)
	{
		for (int aGridY = 0; aGridY < BEGHOULED_MAX_GRIDSIZEY; aGridY++)
		{
			if (mBeghouledEated[aGridX][aGridY])
			{
				mBeghouledEated[aGridX][aGridY] = false;
				if (--theCount == 0)
				{
					BeghouledUpdateCraters();
					return;
				}
			}
		}
	}
}

void Challenge::BeghouledScore(int theGridX, int theGridY, int theNumPlants, int theIsHorizontal)
{
	mApp->PlayFoley(FOLEY_ART_CHALLENGE);

	float aPosX = mBoard->GridToPixelX(theGridX, theGridY);
	float aPosY = mBoard->GridToPixelY(theGridX, theGridY);
	if (theIsHorizontal)
	{
		aPosX += theNumPlants == 3 ? 80.0f : theNumPlants == 4 ? 120.0f : 160.0f;
	}
	else
	{
		aPosY += theNumPlants == 3 ? 80.0f : theNumPlants == 4 ? 120.0f : 160.0f;
	}

	mChallengeScore++;
	if (mBoard->mSeedBank->mNumPackets == 0)
	{
		mBoard->mSeedBank->mSeedPackets[0].SetPacketType(SEED_REPEATER, SEED_NONE);
		mBoard->mSeedBank->mSeedPackets[1].SetPacketType(SEED_FUMESHROOM, SEED_NONE);
		mBoard->mSeedBank->mSeedPackets[2].SetPacketType(SEED_TALLNUT, SEED_NONE);
		mBoard->mSeedBank->mSeedPackets[3].SetPacketType(SEED_BEGHOULED_BUTTON_SHUFFLE, SEED_NONE);
		mBoard->mSeedBank->mNumPackets = 4;

		mBoard->DisplayAdvice("[ADVICE_BEGHOULED_SAVE_SUN]", MESSAGE_STYLE_HINT_FAST, ADVICE_BEGHOULED_SAVE_SUN);

		if (BeghouledCanClearCrater())
		{
			mBoard->mSeedBank->mSeedPackets[4].SetPacketType(SEED_BEGHOULED_BUTTON_CRATER, SEED_NONE);
			mBoard->mSeedBank->mNumPackets = 5;
		}
	}
	else
	{
		if (!mBoard->mAdvice->IsBeingDisplayed())
		{
			std::string aMsg = TodReplaceNumberString("[ADVICE_BEGHOULED_MATCH_3]", "{SCORE}", BEGHOULED_WINNING_SCORE);
			mBoard->DisplayAdvice(aMsg, MESSAGE_STYLE_HINT_FAST, ADVICE_BEGHOULED_MATCH_3);
		}
		if (mChallengeScore >= BEGHOULED_WINNING_SCORE - 5)
		{
			mBoard->DisplayAdvice("[ADVICE_BEGHOULED_ALMOST_THERE]", MESSAGE_STYLE_HINT_FAST, ADVICE_ALMOST_THERE);
		}
	}

	if (mChallengeScore >= BEGHOULED_WINNING_SCORE)
	{
		mChallengeScore = BEGHOULED_WINNING_SCORE;
		SpawnLevelAward(theGridX, theGridY);
		mBoard->ClearAdvice(ADVICE_NONE);
	}
	else
	{
		int aNumSuns = theNumPlants - 2 + mBeghouledMatchesThisMove;
		if (theNumPlants >= 5)
		{
			aNumSuns += 2;
		}
		aNumSuns = ClampInt(aNumSuns, 1, 5);

		for (int i = 0; i < aNumSuns; i++)
		{
			mBoard->AddCoin(aPosX + 20 * i - 10, aPosY, COIN_SUN, COIN_MOTION_COIN);
		}
	}

	mBeghouledMatchesThisMove++;
}

void Challenge::BeghouledRemoveMatches(BeghouledBoardState* theBoardState)
{
	for (int aGridY = 0; aGridY < BEGHOULED_MAX_GRIDSIZEY; aGridY++)
	{
		for (int aGridX = 0; aGridX < BEGHOULED_MAX_GRIDSIZEX; aGridX++)
		{
			int aHorMatchLength = BeghouledHorizontalMatchLength(aGridX, aGridY, theBoardState);
			if (aHorMatchLength >= 3)
			{
				BeghouledRemoveHorizontalMatch(aGridX, aGridY, theBoardState);
				BeghouledScore(aGridX, aGridY, aHorMatchLength, true);
			}

			int aVerMatchLength = BeghouledVerticalMatchLength(aGridX, aGridY, theBoardState);
			if (aVerMatchLength >= 3)
			{
				BeghouledRemoveVerticalMatch(aGridX, aGridY, theBoardState);
				BeghouledScore(aGridX, aGridY, aVerMatchLength, false);
			}
		}
	}
}

int Challenge::BeghouledHorizontalMatchLength(int theGridX, int theGridY, BeghouledBoardState* theBoardState)
{
	SeedType aSeedType = BeghouledGetPlantAt(theGridX, theGridY, theBoardState);
	if (aSeedType == SEED_NONE || BeghouledGetPlantAt(theGridX - 1, theGridY, theBoardState) == aSeedType)
		return 0;

	int aLength = 1;
	while (BeghouledGetPlantAt(theGridX + aLength, theGridY, theBoardState) == aSeedType)
		aLength++;
	return aLength;
}

int Challenge::BeghouledVerticalMatchLength(int theGridX, int theGridY, BeghouledBoardState* theBoardState)
{
	SeedType aSeedType = BeghouledGetPlantAt(theGridX, theGridY, theBoardState);
	if (aSeedType == SEED_NONE || BeghouledGetPlantAt(theGridX, theGridY - 1, theBoardState) == aSeedType)
		return 0;

	int aLength = 1;
	while (BeghouledGetPlantAt(theGridX, theGridY + aLength, theBoardState) == aSeedType)
		aLength++;
	return aLength;
}

int Challenge::BeghouledBoardHasMatch(BeghouledBoardState* theBoardState)
{
	for (int aCol = 0; aCol < 8; aCol++)
	{
		for (int aRow = 0; aRow < 5; aRow++)
		{
			if (BeghouledHorizontalMatchLength(aCol, aRow, theBoardState) >= 3 || 
				BeghouledVerticalMatchLength(aCol, aRow, theBoardState) >= 3) 
				return true;
		}
	}
	return false;
}

SeedType Challenge::BeghouledPickSeed(int theGridX, int theGridY, BeghouledBoardState* theBoardState, int theAllowMatches)
{
	TOD_ASSERT(theBoardState->mSeedType[theGridX][theGridY] == SEED_NONE);
	// SeedType* aSeedState = &theBoardState->mSeedType[theGridX][theGridY];
	int aCount = 0;
	SeedType aPickArray[6];
	
	for (int i = 0; i < 6; i++)
	{
		SeedType aSeedType;
		switch (i)
		{
		case 0:		aSeedType = SeedType::SEED_PUFFSHROOM;		break;
		case 1:		aSeedType = SeedType::SEED_STARFRUIT;		break;
		case 2:		aSeedType = SeedType::SEED_MAGNETSHROOM;	break;
		case 3:		aSeedType = SeedType::SEED_SNOWPEA;			break;
		case 4:		aSeedType = SeedType::SEED_WALLNUT;			break;
		case 5:		aSeedType = SeedType::SEED_PEASHOOTER;		break;
		default:	TOD_ASSERT(false);								break;
		}

		if (mBeghouledPurcasedUpgrade[BeghouledUpgrade::BEGHOULED_UPGRADE_REPEATER] && aSeedType == SeedType::SEED_PEASHOOTER)
		{
			aSeedType = SeedType::SEED_REPEATER;
		}
		if (mBeghouledPurcasedUpgrade[BeghouledUpgrade::BEGHOULED_UPGRADE_FUMESHROOM] && aSeedType == SeedType::SEED_PUFFSHROOM)
		{
			aSeedType = SeedType::SEED_FUMESHROOM;
		}
		if (mBeghouledPurcasedUpgrade[BeghouledUpgrade::BEGHOULED_UPGRADE_TALLNUT] && aSeedType == SeedType::SEED_WALLNUT)
		{
			aSeedType = SeedType::SEED_TALLNUT;
		}

		theBoardState->mSeedType[theGridX][theGridY] = aSeedType;

		if (theAllowMatches || !BeghouledBoardHasMatch(theBoardState))
		{
			aPickArray[aCount] = aSeedType;
			aCount++;
		}
	}
	
	theBoardState->mSeedType[theGridX][theGridY] = SEED_NONE;
	return TodPickFromArray(aPickArray, aCount);
}

void Challenge::BeghouledFillHoles(BeghouledBoardState* theBoardState, int theAllowMatches)
{
	for (int aCol = 0; aCol < BEGHOULED_MAX_GRIDSIZEX; aCol++)
	{
		for (int aRow = 0; aRow < BEGHOULED_MAX_GRIDSIZEY; aRow++)
		{
			if (theBoardState->mSeedType[aCol][aRow] == SeedType::SEED_NONE && !mBeghouledEated[aCol][aRow])
			{
				theBoardState->mSeedType[aCol][aRow] = BeghouledPickSeed(aCol, aRow, theBoardState, theAllowMatches);
			}
		}
	}
}

void Challenge::BeghouledCreatePlants(BeghouledBoardState* theOldBoardState, BeghouledBoardState* theNewBoardState)
{
	for (int aCol = 0; aCol < BEGHOULED_MAX_GRIDSIZEX; aCol++)
	{
		int aFallY = 80;
		for (int aRow = BEGHOULED_MAX_GRIDSIZEY - 1; aRow >= 0; aRow--)
		{
			SeedType aSeedType = theNewBoardState->mSeedType[aCol][aRow];
			if (theOldBoardState->mSeedType[aCol][aRow] == SEED_NONE && aSeedType != SEED_NONE)
			{
				aFallY -= 100;
				mBoard->NewPlant(aCol, aRow, aSeedType, SEED_NONE)->mY = aFallY;
				BeghouledStartFalling(STATECHALLENGE_BEGHOULED_FALLING);
			}
		}
	}
}

void Challenge::BeghouledMakeStartBoard()
{
	BeghouledBoardState aEmptyBoardState;
	LoadBeghouledBoardState(&aEmptyBoardState);
	BeghouledBoardState aBoardState;
	LoadBeghouledBoardState(&aBoardState);

	BeghouledFillHoles(&aBoardState, false);
	TOD_ASSERT(!BeghouledBoardHasMatch(&aBoardState));

	if (!BeghouledBoardHasMatch(&aBoardState))
	{
		BeghouledCreatePlants(&aEmptyBoardState, &aBoardState);
	}
}

void Challenge::BeghouledPopulateBoard()
{
	BeghouledBoardState aEmptyBoardState, aBoardState;
	LoadBeghouledBoardState(&aEmptyBoardState);
	int aAllowGeneratedCascades = BeghouledBoardHasMatch(&aEmptyBoardState);

	for (int i = 0; i < 2; i++)
	{
		LoadBeghouledBoardState(&aBoardState);
		BeghouledFillHoles(&aBoardState, aAllowGeneratedCascades);
		// 填充后，若不存在可消除的移动，则重新填充 1 次
		if (BeghouledCheckForPossibleMoves(&aBoardState))
			break;
	}

	// 根据填充的结果开始执行创建植物
	BeghouledCreatePlants(&aEmptyBoardState, &aBoardState);
}

int Challenge::BeghouledCheckForPossibleMoves(BeghouledBoardState* theBoardState)
{
	GameMode aGameMode = mApp->mGameMode;

	for (int aRow = 0; aRow < BEGHOULED_MAX_GRIDSIZEY; aRow++)
	{
		for (int aCol = 0; aCol < BEGHOULED_MAX_GRIDSIZEX; aCol++)
		{
			if (aGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED)
			{
				if (BeghouledIsValidMove(aCol, aRow, aCol + 1, aRow, theBoardState) || BeghouledIsValidMove(aCol, aRow, aCol, aRow + 1, theBoardState))
				{
					return true;
				}
			}
			else if (aGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST)
			{
				if (BeghouledTwistMoveCausesMatch(aCol, aRow, theBoardState))
				{
					return true;
				}
			}
			else
			{
				return false;
			}
		}
	}

	return false;
}

void Challenge::BeghouledDragStart(int x, int y)
{
	if (!mBoard->HasLevelAwardDropped())
	{
		mBeghouledMouseCapture = true;
		mBeghouledMouseDownX = x;
		mBeghouledMouseDownY = y;
	}
}

int Challenge::MouseMove(int x, int y)
{
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_BEGHOULED && !mBoard->HasLevelAwardDropped())
	{
		if (mBeghouledMouseCapture)
		{
			BeghouledDragUpdate(x, y);
			return true;
		}

		HitResult aHitResult;
		mBoard->MouseHitTest(x, y, &aHitResult);
		if (aHitResult.mObjectType == OBJECT_TYPE_PLANT)
			return true;
	}

	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN && mApp->mActive)
	{
		if (mChallengeState == STATECHALLENGE_ZEN_FADING)
			mChallengeState = STATECHALLENGE_NORMAL;
		mChallengeStateCounter = 3000;
	}
	return false;
}

int Challenge::UpdateToolTip(int theX, int theY)
{
	if (!mApp->IsSlotMachineLevel())
		return false;
	
	HitResult aHitResult;
	mBoard->MouseHitTest(theX, theY, &aHitResult);
	if (aHitResult.mObjectType != GameObjectType::OBJECT_TYPE_SLOT_MACHINE_HANDLE ||
		mBoard->mCursorObject->mCursorType != CursorType::CURSOR_TYPE_NORMAL ||
		mChallengeState != ChallengeState::STATECHALLENGE_NORMAL)
		return false;
	
	if (!mBoard->CanTakeSunMoney(25))
	{
		mBoard->mToolTip->SetWarningText("[NOT_ENOUGH_SUN]");
	}

	Rect aSlotMachineHandleRect = SlotMachineGetHandleRect();
	mBoard->mToolTip->SetLabel("[SLOT_MACHINE_PULL_TOOLTIP]");
	mBoard->mToolTip->SetPosition(aSlotMachineHandleRect.mX + 15, aSlotMachineHandleRect.mY + 65);
	mBoard->mToolTip->mVisible = true;
	mBoard->mToolTip->mCenter = true;
	return true;
}

void Challenge::MouseDownWhackAZombie(int theX, int theY)
{
	mApp->ReanimationTryToGet(mBoard->mCursorObject->mReanimCursorID)->mAnimTime = 0.2f;
	mApp->PlayFoley(FOLEY_SWING);

	Zombie* aZombie = nullptr;
	Zombie* aTopZombie = nullptr;
	while (mBoard->IterateZombies(aZombie))
	{
		if (!aZombie->IsDeadOrDying())
		{
			Rect aZombieRect = aZombie->GetZombieRect();
			if (GetCircleRectOverlap(theX, theY - 20, 45, aZombieRect))
			{
				if (aTopZombie == nullptr || aZombie->mRenderOrder >= aTopZombie->mRenderOrder)
				{
					aTopZombie = aZombie;
				}
			}
		}
	}

	if (aTopZombie)
	{
		if (aTopZombie->mHelmType != HELMTYPE_NONE)
		{
			if (aTopZombie->mHelmType == HELMTYPE_PAIL)
			{
				mApp->PlayFoley(FOLEY_SHIELD_HIT);
			}
			else if (aTopZombie->mHelmType == HELMTYPE_TRAFFIC_CONE)
			{
				mApp->PlayFoley(FOLEY_PLASTIC_HIT);
			}

			aTopZombie->TakeHelmDamage(900, 0U);
		}
		else
		{
			mApp->PlayFoley(FOLEY_BONK);
			mApp->AddTodParticle(theX - 3, theY + 9, RENDER_LAYER_ABOVE_UI, PARTICLE_POW);
			aTopZombie->DieWithLoot();
			mBoard->ClearCursor();
		}
	}
}

void Challenge::AdvanceCrazyDaveDialog()
{
	if (!mBoard->IsScaryPotterDaveTalking() || mApp->mCrazyDaveMessageIndex == -1)
		return;

	if (!mApp->AdvanceCrazyDaveText())
	{
		mApp->CrazyDaveLeave();
		return;
	}

	// “来，我会给你更多花瓶的。” || “这应该他们最后一波了。”
	if (mApp->mCrazyDaveMessageIndex == 2702 || mApp->mCrazyDaveMessageIndex == 2801)
	{
		ScaryPotterPopulate();
		mApp->PlayFoley(FOLEY_PLANT);
		mBoard->PlaceRake();
	}
}

int Challenge::MouseDown(int x, int y, int theClickCount, HitResult* theHitResult)
{
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		return mApp->mZenGarden->MouseDownZenGarden(x, y, theClickCount, theHitResult);
	}

	if (mApp->mGameScene != SCENE_PLAYING)
		return false;

	if (mBoard->IsScaryPotterDaveTalking() && mApp->mCrazyDaveMessageIndex != -1)
	{
		AdvanceCrazyDaveDialog();
		return true;
	}

	if (theHitResult->mObjectType == OBJECT_TYPE_COIN && theClickCount >= 0)
		return false;

	if (mApp->mGameMode == GAMEMODE_CHALLENGE_BEGHOULED)
	{
		if (mChallengeState != STATECHALLENGE_NORMAL)
			return false;

		if (theHitResult->mObjectType == OBJECT_TYPE_PLANT)
		{
			BeghouledDragStart(x, y);
			return true;
		}
	}

	if (mApp->mGameMode == GAMEMODE_CHALLENGE_BEGHOULED_TWIST)
	{
		if (mChallengeState != STATECHALLENGE_NORMAL)
			return false;

		BeghouledTwistMouseDown(x, y);
	}

	if (mApp->IsSlotMachineLevel() && theHitResult->mObjectType == OBJECT_TYPE_SLOT_MACHINE_HANDLE &&
		mBoard->mCursorObject->mCursorType == CURSOR_TYPE_NORMAL && mChallengeState == STATECHALLENGE_NORMAL)
	{
		if (mBoard->TakeSunMoney(25))
		{
			for (int i = 0; i < 3; i++)
			{
				mBoard->mSeedBank->mSeedPackets[i].SlotMachineStart();
			}

			mApp->ReanimationGet(mReanimChallenge)->PlayReanim("anim_pull", REANIM_PLAY_ONCE_AND_HOLD, 0, 36.0f);
			mChallengeState = STATECHALLENGE_SLOT_MACHINE_ROLLING;
			mBoard->SetTutorialState(TUTORIAL_SLOT_MACHINE_COMPLETED);
			mBoard->ClearAdvice(ADVICE_NONE);

			mSlotMachineRollCount++;
			mApp->PlaySample(Sexy::SOUND_SLOT_MACHINE);
		}

		return true;
	}

	if (mApp->IsWhackAZombieLevel() && theHitResult->mObjectType == OBJECT_TYPE_NONE &&
		mBoard->mCursorObject->mCursorType == CURSOR_TYPE_HAMMER && theClickCount >= 0)
	{
		MouseDownWhackAZombie(x, y);
		return true;
	}

	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZOMBIQUARIUM){
		if (theClickCount > 0) {
			ZombiquariumMouseDown(x,y);
			return false;
		} else {
			mApp->PlaySample(Sexy::SOUND_TAPGLASS);
			return true;
		}
	}

	if (mApp->IsScaryPotterLevel() && theHitResult->mObjectType == OBJECT_TYPE_SCARY_POT)
	{
		ScaryPotterMalletPot((GridItem*)theHitResult->mObject);
		return true;
	}

	return false;
}

Rect Challenge::SlotMachineGetHandleRect()
{
	return Rect(mBoard->mSeedBank->mX + 473, mBoard->mSeedBank->mY, 55, 80);
}

int Challenge::MouseUp(int x, int y)
{
	(void)x;(void)y;
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_BEGHOULED)
	{
		if (mBeghouledMouseCapture && !mBoard->mAdvice->IsBeingDisplayed() && mChallengeScore == 0)
		{
			mBoard->DisplayAdvice("[ADVICE_BEGHOULED_DRAG_TO_MATCH_3]", MESSAGE_STYLE_HINT_FAST, ADVICE_BEGHOULED_DRAG_TO_MATCH_3);
		}

		BeghouledDragCancel();
	}

	return false;
}

void Challenge::ClearCursor()
{
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_BEGHOULED)
	{
		BeghouledDragCancel();
	}

	if (mApp->IsWhackAZombieLevel() && !mBoard->HasLevelAwardDropped())
	{
		mBoard->mCursorObject->mCursorType = CURSOR_TYPE_HAMMER;
	}
}

int Challenge::UpdateBeghouledPlant(Plant* thePlant)
{
	int aMoving = false;
	int aDiffX = mBoard->GridToPixelX(thePlant->mPlantCol, thePlant->mRow) - thePlant->mX;
	int aDiffY = mBoard->GridToPixelY(thePlant->mPlantCol, thePlant->mRow) - thePlant->mY;
	int aDelta = mChallengeState == STATECHALLENGE_BEGHOULED_MOVING ? 3 : TodAnimateCurve(90, 30, mChallengeStateCounter, 1, 20, CURVE_EASE_IN);
	int aDeltaX = 0, aDeltaY = 0;

	if (aDiffX > 0)
	{
		aDeltaX = std::min(aDelta, aDiffX);
		thePlant->mX += aDeltaX;
		aMoving = true;
	}
	else if (aDiffX < 0)
	{
		aDeltaX = -std::min(aDelta, -aDiffX);
		thePlant->mX += aDeltaX;
		aMoving = true;
	}
	if (aDiffY > 0)
	{
		aDeltaY = std::min(aDelta, aDiffY);
		thePlant->mY += aDeltaY;
		aMoving = true;
	}
	else if (aDiffY < 0)
	{
		aDeltaY = -std::min(aDelta, -aDiffY);
		thePlant->mY += aDeltaY;
		aMoving = true;
	}

	if (thePlant->mState == STATE_MAGNETSHROOM_CHARGING || thePlant->mState == STATE_MAGNETSHROOM_SUCKING)
	{
		thePlant->mMagnetItems[0].mPosX += aDeltaX;
		thePlant->mMagnetItems[0].mPosY += aDeltaY;
	}

	return aMoving;
}

void Challenge::BeghouledFlashPlant(int theFlashX, int theFlashY, int theFromX, int theFromY, int theToX, int theToY)
{
	if (theFlashX == theFromX && theFlashY == theFromY)
	{
		theFlashX = theToX;
		theFlashY = theToY;
	}
	else if (theFlashX == theToX && theFlashY == theToY)
	{
		theFlashX = theFromX;
		theFlashY = theFromY;
	}

	Plant* aFlashPlant = mBoard->GetTopPlantAt(theFlashX, theFlashY, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION);
	if (aFlashPlant && aFlashPlant->mBeghouledFlashCountdown == 0) 
		aFlashPlant->mBeghouledFlashCountdown = 300;
}

int Challenge::BeghouledTwistFlashMatch(BeghouledBoardState* theBoardState, int theGridX, int theGridY)
{
	if (!BeghouledTwistMoveCausesMatch(theGridX, theGridY, theBoardState))
		return false;

	for (int i = 0; i < 4; i++)
	{
		Plant* aPlant = mBoard->GetTopPlantAt(theGridX + (i % 2), theGridY + (i / 2), PlantPriority::TOPPLANT_ANY);
		if (aPlant && aPlant->mBeghouledFlashCountdown == 0)
		{
			aPlant->mBeghouledFlashCountdown = 300;
		}
	}
	return true;
}

int Challenge::BeghouledFlashFromBoardState(BeghouledBoardState* theBoardState, int theFromX, int theFromY, int theToX, int theToY)
{
	TOD_ASSERT(theFromX >= 0 && theFromX < BEGHOULED_MAX_GRIDSIZEX && theFromY >= 0 && theFromY < BEGHOULED_MAX_GRIDSIZEY);
	TOD_ASSERT(theToX >= 0 && theToX < BEGHOULED_MAX_GRIDSIZEX && theToY >= 0 && theToY < BEGHOULED_MAX_GRIDSIZEY);

	if (mBeghouledEated[theFromX][theFromY] || mBeghouledEated[theToX][theToY])
		return false;

	SeedType aFromSeedType = theBoardState->mSeedType[theFromX][theFromY];
	SeedType aToSeedType = theBoardState->mSeedType[theToX][theToY];
	theBoardState->mSeedType[theFromX][theFromY] = aToSeedType;
	theBoardState->mSeedType[theToX][theToY] = aFromSeedType;

	int aHasMatch = false;
	for (int aRow = 0; aRow < BEGHOULED_MAX_GRIDSIZEY; aRow++)
	{
		for (int aCol = 0; aCol < BEGHOULED_MAX_GRIDSIZEX; aCol++)
		{
			if (BeghouledHorizontalMatchLength(aCol, aRow, theBoardState) >= 3)
			{
				for (int i = 0; i < 3; i++)
				{
					BeghouledFlashPlant(aCol + i, aRow, theFromX, theFromY, theToX, theToY);
				}
			}

			else if (BeghouledVerticalMatchLength(aCol, aRow, theBoardState) >= 3)
			{
				for (int i = 0; i < 3; i++)
				{
					BeghouledFlashPlant(aCol, aRow + i, theFromX, theFromY, theToX, theToY);
				}
			}
			else continue;

			aHasMatch = true;
			break;
		}
	}

	theBoardState->mSeedType[theFromX][theFromY] = aFromSeedType;
	theBoardState->mSeedType[theToX][theToY] = aToSeedType;
	return aHasMatch;
}

void Challenge::BeghouledCancelMatchFlashing()
{
	Plant* aPlant = nullptr;
	while (mBoard->IteratePlants(aPlant))
	{
		if (aPlant->mEatenFlashCountdown >= 25)
			aPlant->mEatenFlashCountdown = 25;
	}
}

void Challenge::BeghouledFlashAMatch()
{
	BeghouledBoardState aBoardState;
	LoadBeghouledBoardState(&aBoardState);
	
	GameMode aGameMode = mApp->mGameMode;
	if (aGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED)
	{
		for (int aRow = 0; aRow <= BEGHOULED_MAX_GRIDSIZEY - 1; aRow++)
		{
			for (int aCol = 0; aCol <= BEGHOULED_MAX_GRIDSIZEX - 1; aCol++)
			{
				if ((aCol < BEGHOULED_MAX_GRIDSIZEX - 1 && BeghouledFlashFromBoardState(&aBoardState, aCol, aRow, aCol + 1, aRow)) ||
					(aRow < BEGHOULED_MAX_GRIDSIZEY - 1 && BeghouledFlashFromBoardState(&aBoardState, aCol, aRow, aCol, aRow + 1)))
					return;
			}
		}
	}
	else if (aGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST)
	{
		for (int aRow = 0; aRow <= BEGHOULED_MAX_GRIDSIZEY - 1; aRow++)
		{
			for (int aCol = 0; aCol <= BEGHOULED_MAX_GRIDSIZEX - 1; aCol++)
			{
				if (BeghouledTwistFlashMatch(&aBoardState, aCol, aRow))
					return;
			}
		}
	}
}

void Challenge::UpdateBeghouled()
{
	mBoard->mProgressMeterWidth = TodAnimateCurve(0, BEGHOULED_WINNING_SCORE, mChallengeScore, 0, PROGRESS_METER_COUNTER, CURVE_LINEAR);

	int aMovingPlant = false;
	Plant* aPlant = nullptr;
	while (mBoard->IteratePlants(aPlant))
	{
		if (UpdateBeghouledPlant(aPlant))
		{
			aMovingPlant = true;
		}
	}

	if (mBoard->mSeedBank->mNumPackets > 4 && !mBoard->mAdvice->IsBeingDisplayed() && !mBoard->mHelpDisplayed[ADVICE_BEGHOULED_USE_CRATER_2])
	{
		int aCost = mBoard->GetCurrentPlantCost(SEED_BEGHOULED_BUTTON_CRATER, SEED_NONE);
		if (mBoard->CanTakeSunMoney(aCost) && BeghouledCanClearCrater() && !mBoard->HasLevelAwardDropped())
		{
			mBoard->DisplayAdvice("[ADVICE_BEGHOULED_USE_CRATER_2]", MESSAGE_STYLE_HINT_FAST, ADVICE_BEGHOULED_USE_CRATER_2);
		}
	}

	if (mApp->mGameMode == GAMEMODE_CHALLENGE_BEGHOULED_TWIST && mChallengeState == STATECHALLENGE_NORMAL)
	{
		if (BeghouledTwistSquareFromMouse(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY, mChallengeGridX, mChallengeGridY))
		{
			BeghouledBoardState aBoardState;
			LoadBeghouledBoardState(&aBoardState);
			if (!BeghouledTwistValidMove(mChallengeGridX, mChallengeGridY, &aBoardState))
			{
				mChallengeGridX = -1;
				mChallengeGridY = -1;
			}
		}
	}
	else
	{
		mChallengeGridX = -1;
		mChallengeGridY = -1;
	}

	if (!aMovingPlant && (mChallengeState == STATECHALLENGE_BEGHOULED_FALLING || mChallengeState == STATECHALLENGE_BEGHOULED_MOVING))
	{
		mChallengeState = STATECHALLENGE_NORMAL;
		mChallengeStateCounter = 1500;

		BeghouledBoardState aBoardState;
		LoadBeghouledBoardState(&aBoardState);
		BeghouledRemoveMatches(&aBoardState);
		LoadBeghouledBoardState(&aBoardState);
		BeghouledMakePlantsFall(&aBoardState);
		BeghouledPopulateBoard();

		if (mChallengeState == STATECHALLENGE_BEGHOULED_FALLING)
			return;

		mChallengeStateCounter = 1500;
		mBeghouledMatchesThisMove = 0;
		BeghouledCheckStuckState();
	}

	if (mChallengeStateCounter)
	{
		mChallengeStateCounter--;
		if (mChallengeStateCounter <= 0 && !mBoard->HasLevelAwardDropped())
		{
			if (mChallengeState == STATECHALLENGE_NORMAL)
			{
				BeghouledFlashAMatch();
				mChallengeStateCounter = 1500;
			}
			else if (mChallengeState == STATECHALLENGE_BEGHOULED_NO_MATCHES)
			{
				mApp->AddTodParticle(BOARD_WIDTH / 2, BOARD_HEIGHT / 2, RENDER_LAYER_TOP, PARTICLE_SCREEN_FLASH);
				BeghouledShuffle();
			}
		}
	}
}

void Challenge::UpdateConveyorBelt()
{
	if (mBoard->HasLevelAwardDropped())
		return;

	mBoard->mSeedBank->UpdateConveyorBelt();
	mConveyorBeltCounter--;
	if (mConveyorBeltCounter > 0)
		return;

	float aConveyorSpeedMultiplier = 1;
	if (mApp->IsFinalBossLevel())
	{
		aConveyorSpeedMultiplier = 0.875f;
	}
	else if (mApp->IsShovelLevel() || mApp->mGameMode == GAMEMODE_CHALLENGE_PORTAL_COMBAT)
	{
		aConveyorSpeedMultiplier = 1.5f;
	}
	else if (mApp->mGameMode == GAMEMODE_CHALLENGE_INVISIGHOUL)
	{
		aConveyorSpeedMultiplier = 2.0f;
	}
	else if (mApp->mGameMode == GAMEMODE_CHALLENGE_COLUMN)
	{
		aConveyorSpeedMultiplier = 3.0f;
	}
	int aNumSeedsOnConveyor = mBoard->mSeedBank->GetNumSeedsOnConveyorBelt();
	mConveyorBeltCounter = aConveyorSpeedMultiplier * (aNumSeedsOnConveyor > 8 ? 1000 : aNumSeedsOnConveyor > 6 ? 500 : aNumSeedsOnConveyor > 4 ? 425 : 400);

	TodWeightedArray aSeedPickArray[20];
	int aSeedPickCount = 0;
	if (mBoard->mLevel == 10)
	{
		aSeedPickCount = 7;
		aSeedPickArray[0].mItem = SEED_PEASHOOTER;
		aSeedPickArray[0].mWeight = 20;
		aSeedPickArray[1].mItem = SEED_CHERRYBOMB;
		aSeedPickArray[1].mWeight = 20;
		aSeedPickArray[2].mItem = SEED_WALLNUT;
		aSeedPickArray[2].mWeight = 15;
		aSeedPickArray[3].mItem = SEED_REPEATER;
		aSeedPickArray[3].mWeight = 20;
		aSeedPickArray[4].mItem = SEED_SNOWPEA;
		aSeedPickArray[4].mWeight = 10;
		aSeedPickArray[5].mItem = SEED_CHOMPER;
		aSeedPickArray[5].mWeight = 5;
		aSeedPickArray[6].mItem = SEED_POTATOMINE;
		aSeedPickArray[6].mWeight = 10;
	}
	else if (mBoard->mLevel == 20)
	{
		aSeedPickCount = 7;
		aSeedPickArray[0].mItem = SEED_GRAVEBUSTER;
		aSeedPickArray[0].mWeight = 20;
		aSeedPickArray[1].mItem = SEED_ICESHROOM;
		aSeedPickArray[1].mWeight = 15;
		aSeedPickArray[2].mItem = SEED_DOOMSHROOM;
		aSeedPickArray[2].mWeight = 15;
		aSeedPickArray[3].mItem = SEED_HYPNOSHROOM;
		aSeedPickArray[3].mWeight = 10;
		aSeedPickArray[4].mItem = SEED_SCAREDYSHROOM;
		aSeedPickArray[4].mWeight = 15;
		aSeedPickArray[5].mItem = SEED_FUMESHROOM;
		aSeedPickArray[5].mWeight = 15;
		aSeedPickArray[6].mItem = SEED_PUFFSHROOM;
		aSeedPickArray[6].mWeight = 10;
	}
	else if (mBoard->mLevel == 30)
	{
		aSeedPickCount = 8;
		aSeedPickArray[0].mItem = SEED_LILYPAD;
		aSeedPickArray[0].mWeight = 25;
		aSeedPickArray[1].mItem = SEED_SQUASH;
		aSeedPickArray[1].mWeight = 5;
		aSeedPickArray[2].mItem = SEED_THREEPEATER;
		aSeedPickArray[2].mWeight = 25;
		aSeedPickArray[3].mItem = SEED_TANGLEKELP;
		aSeedPickArray[3].mWeight = 5;
		aSeedPickArray[4].mItem = SEED_JALAPENO;
		aSeedPickArray[4].mWeight = 10;
		aSeedPickArray[5].mItem = SEED_SPIKEWEED;
		aSeedPickArray[5].mWeight = 10;
		aSeedPickArray[6].mItem = SEED_TORCHWOOD;
		aSeedPickArray[6].mWeight = 10;
		aSeedPickArray[7].mItem = SEED_TALLNUT;
		aSeedPickArray[7].mWeight = 10;
	}
	else if (mBoard->mLevel == 40)
	{
		aSeedPickCount = 8;
		aSeedPickArray[0].mItem = SEED_LILYPAD;
		aSeedPickArray[0].mWeight = 25;
		aSeedPickArray[1].mItem = SEED_SEASHROOM;
		aSeedPickArray[1].mWeight = 10;
		aSeedPickArray[2].mItem = SEED_MAGNETSHROOM;
		aSeedPickArray[2].mWeight = 5;
		aSeedPickArray[3].mItem = SEED_BLOVER;
		aSeedPickArray[3].mWeight = 5;
		aSeedPickArray[4].mItem = SEED_CACTUS;
		aSeedPickArray[4].mWeight = 15;
		aSeedPickArray[5].mItem = SEED_STARFRUIT;
		aSeedPickArray[5].mWeight = 25;
		aSeedPickArray[6].mItem = SEED_SPLITPEA;
		aSeedPickArray[6].mWeight = 5;
		aSeedPickArray[7].mItem = SEED_PUMPKINSHELL;
		aSeedPickArray[7].mWeight = 10;
	}
	else if (mApp->IsFinalBossLevel())
	{
		aSeedPickCount = 6;
		aSeedPickArray[0].mItem = SEED_FLOWERPOT;
		aSeedPickArray[0].mWeight = 55;
		aSeedPickArray[1].mItem = SEED_MELONPULT;
		aSeedPickArray[1].mWeight = 10;
		aSeedPickArray[2].mItem = SEED_JALAPENO;
		aSeedPickArray[2].mWeight = 12;
		aSeedPickArray[3].mItem = SEED_CABBAGEPULT;
		aSeedPickArray[3].mWeight = 10;
		aSeedPickArray[4].mItem = SEED_KERNELPULT;
		aSeedPickArray[4].mWeight = 5;
		aSeedPickArray[5].mItem = SEED_ICESHROOM;
		aSeedPickArray[5].mWeight = 8;
	}
	else if (mApp->IsShovelLevel())
	{
		aSeedPickCount = 1;
		aSeedPickArray[0].mItem = SEED_PEASHOOTER;
		aSeedPickArray[0].mWeight = 100;
	}
	else if (mApp->mGameMode == GAMEMODE_CHALLENGE_WALLNUT_BOWLING_2)
	{
		aSeedPickCount = 3;
		aSeedPickArray[0].mItem = SEED_WALLNUT;
		aSeedPickArray[0].mWeight = 85;
		aSeedPickArray[1].mItem = SEED_EXPLODE_O_NUT;
		aSeedPickArray[1].mWeight = 15;
		aSeedPickArray[2].mItem = SEED_GIANT_WALLNUT;
		aSeedPickArray[2].mWeight = 15;
	}
	else if (mApp->IsWallnutBowlingLevel())
	{
		aSeedPickCount = 2;
		aSeedPickArray[0].mItem = SEED_WALLNUT;
		aSeedPickArray[0].mWeight = 85;
		aSeedPickArray[1].mItem = SEED_EXPLODE_O_NUT;
		aSeedPickArray[1].mWeight = 15;
	}
	else if (mApp->IsLittleTroubleLevel())
	{
		aSeedPickCount = 4;
		aSeedPickArray[0].mItem = SEED_LILYPAD;
		aSeedPickArray[0].mWeight = 25;
		aSeedPickArray[1].mItem = SEED_WALLNUT;
		aSeedPickArray[1].mWeight = 15;
		aSeedPickArray[2].mItem = SEED_PEASHOOTER;
		aSeedPickArray[2].mWeight = 25;
		aSeedPickArray[3].mItem = SEED_CHERRYBOMB;
		aSeedPickArray[3].mWeight = 35;
	}
	else if (mApp->IsStormyNightLevel())
	{
		aSeedPickCount = 5;
		aSeedPickArray[0].mItem = SEED_LILYPAD;
		aSeedPickArray[0].mWeight = 30;
		aSeedPickArray[1].mItem = SEED_CACTUS;
		aSeedPickArray[1].mWeight = 10;
		aSeedPickArray[2].mItem = SEED_PEASHOOTER;
		aSeedPickArray[2].mWeight = 20;
		aSeedPickArray[3].mItem = SEED_PUFFSHROOM;
		aSeedPickArray[3].mWeight = 15;
		aSeedPickArray[4].mItem = SEED_CHERRYBOMB;
		aSeedPickArray[4].mWeight = 25;
	}
	else if (mApp->IsBungeeBlitzLevel())
	{
		aSeedPickCount = 4;
		aSeedPickArray[0].mItem = SEED_FLOWERPOT;
		aSeedPickArray[0].mWeight = 50;
		aSeedPickArray[1].mItem = SEED_CHOMPER;
		aSeedPickArray[1].mWeight = 25;
		aSeedPickArray[2].mItem = SEED_PUMPKINSHELL;
		aSeedPickArray[2].mWeight = 15;
		aSeedPickArray[3].mItem = SEED_CHERRYBOMB;
		aSeedPickArray[3].mWeight = 10;
	}
	else if (mApp->mGameMode == GAMEMODE_CHALLENGE_PORTAL_COMBAT)
	{
		aSeedPickCount = 6;
		aSeedPickArray[0].mItem = SEED_PEASHOOTER;
		aSeedPickArray[0].mWeight = 25;
		aSeedPickArray[1].mItem = SEED_REPEATER;
		aSeedPickArray[1].mWeight = 20;
		aSeedPickArray[2].mItem = SEED_TORCHWOOD;
		aSeedPickArray[2].mWeight = 10;
		aSeedPickArray[3].mItem = SEED_CACTUS;
		aSeedPickArray[3].mWeight = 15;
		aSeedPickArray[4].mItem = SEED_WALLNUT;
		aSeedPickArray[4].mWeight = 15;
		aSeedPickArray[5].mItem = SEED_CHERRYBOMB;
		aSeedPickArray[5].mWeight = 15;
	}
	else if (mApp->mGameMode == GAMEMODE_CHALLENGE_COLUMN)
	{
		aSeedPickCount = 6;
		aSeedPickArray[0].mItem = SEED_FLOWERPOT;
		aSeedPickArray[0].mWeight = 155;
		aSeedPickArray[1].mItem = SEED_MELONPULT;
		aSeedPickArray[1].mWeight = 5;
		aSeedPickArray[2].mItem = SEED_CHOMPER;
		aSeedPickArray[2].mWeight = 5;
		aSeedPickArray[3].mItem = SEED_PUMPKINSHELL;
		aSeedPickArray[3].mWeight = 15;
		aSeedPickArray[4].mItem = SEED_JALAPENO;
		aSeedPickArray[4].mWeight = 10;
		aSeedPickArray[5].mItem = SEED_SQUASH;
		aSeedPickArray[5].mWeight = 10;
	}
	else if (mApp->mGameMode == GAMEMODE_CHALLENGE_INVISIGHOUL)
	{
		aSeedPickCount = 6;
		aSeedPickArray[0].mItem = SEED_PEASHOOTER;
		aSeedPickArray[0].mWeight = 25;
		aSeedPickArray[1].mItem = SEED_WALLNUT;
		aSeedPickArray[1].mWeight = 15;
		aSeedPickArray[2].mItem = SEED_KERNELPULT;
		aSeedPickArray[2].mWeight = 5;
		aSeedPickArray[3].mItem = SEED_SQUASH;
		aSeedPickArray[3].mWeight = 15;
		aSeedPickArray[4].mItem = SEED_LILYPAD;
		aSeedPickArray[4].mWeight = 30;
		aSeedPickArray[5].mItem = SEED_ICESHROOM;
		aSeedPickArray[5].mWeight = 10;
	}
	else TOD_ASSERT(false);

	for (int i = 0; i < aSeedPickCount; i++)
	{
		TodWeightedArray& aSeedPick = aSeedPickArray[i];
		SeedType aSeedType = (SeedType)aSeedPick.mItem;
		int aCountInBank = mBoard->mSeedBank->CountOfTypeOnConveyorBelt(aSeedType);
		int aTotalCount = mBoard->CountPlantByType(aSeedType) + aCountInBank;

		if (aSeedType == SEED_GRAVEBUSTER)
		{
			if (mBoard->GetGraveStonesCount() <= aTotalCount)
			{
				aSeedPick.mWeight = 0;
				continue;
			}
		}
		else if (aSeedType == SEED_LILYPAD)
		{
			aSeedPick.mWeight = TodAnimateCurve(0, 18, aTotalCount, aSeedPick.mWeight, 1, CURVE_LINEAR);
		}
		else if (aSeedType == SEED_FLOWERPOT)
		{
			aSeedPick.mWeight = TodAnimateCurve(0, mApp->mGameMode == GAMEMODE_CHALLENGE_COLUMN ? 45 : 35, aTotalCount, aSeedPick.mWeight, 1, CURVE_LINEAR);
		}

		if (mApp->IsFinalBossLevel())
		{
			if (aSeedType == SEED_MELONPULT || aSeedType == SEED_KERNELPULT || aSeedType == SEED_CABBAGEPULT)
			{
				int aEmptyPots = mBoard->CountEmptyPotsOrLilies(SEED_FLOWERPOT);
				if (aEmptyPots <= 2)
				{
					aSeedPick.mWeight /= 5;
				}
				else if (aEmptyPots <= 5)
				{
					aSeedPick.mWeight /= 3;
				}
			}

			if (aSeedType == SEED_FLOWERPOT && mBoard->GetBossZombie()->mZombiePhase == PHASE_BOSS_DROP_RV)
			{
				aSeedPick.mWeight = 500;
			}
		}

		if (aSeedPickCount > 2)
		{
			if (aCountInBank >= 4)
			{
				aSeedPick.mWeight = 1;
			}
			else if (aCountInBank >= 3)
			{
				aSeedPick.mWeight = 5;
			}
			else if (aSeedType == mLastConveyorSeedType)
			{
				aSeedPick.mWeight /= 2;
			}
		}
	}

	SeedType aSeedType = (SeedType)TodPickFromWeightedArray(aSeedPickArray, aSeedPickCount);
	mBoard->mSeedBank->AddSeed(aSeedType);
	mLastConveyorSeedType = aSeedType;
}

void Challenge::UpdateRainingSeeds()
{
	if (mBoard->HasLevelAwardDropped() || --mChallengeStateCounter != 0)
		return;

	mChallengeStateCounter = RandRangeInt(500, 999);

	Coin* aCoin = mBoard->AddCoin(RandRangeInt(100, 649), 60, COIN_USABLE_SEED_PACKET, COIN_MOTION_FROM_SKY_SLOW);

	SeedType aSeedType;
	do
		aSeedType = (SeedType)Rand(mApp->GetSeedsAvailable());
	while (
		mBoard->SeedNotRecommendedForLevel(aSeedType) || 
		!mApp->HasSeedType(aSeedType) || 
		Plant::IsUpgrade(aSeedType) || 
		aSeedType == SEED_SUNFLOWER || 
		aSeedType == SEED_TWINSUNFLOWER || 
		aSeedType == SEED_INSTANT_COFFEE || 
		aSeedType == SEED_UMBRELLA || 
		aSeedType == SEED_SUNSHROOM || 
		aSeedType == SEED_IMITATER
		);
	if (Rand(100) < TodAnimateCurve(0, 18, mBoard->CountPlantByType(SEED_LILYPAD), 30, 1, CURVE_LINEAR))
		aSeedType = SEED_LILYPAD;

	aCoin->mUsableSeedType = aSeedType;
}

void Challenge::UpdateStormyNight()
{
	if (mBoard->mPaused)
	{
		if (mChallengeStateCounter == 1)
			return;

		if (mChallengeStateCounter == 150 && mChallengeState == ChallengeState::STATECHALLENGE_STORM_FLASH_1)
		{
			mChallengeStateCounter = 1;
			return;
		}
	}

	mChallengeStateCounter--;
	if ((mChallengeStateCounter == 300 && (mChallengeState == STATECHALLENGE_STORM_FLASH_1 || mChallengeState == STATECHALLENGE_STORM_FLASH_2)) ||
		(mChallengeStateCounter == 150 && (mChallengeState == STATECHALLENGE_STORM_FLASH_1 || mChallengeState == STATECHALLENGE_STORM_FLASH_3)))
	{
		mApp->PlayFoley(FoleyType::FOLEY_THUNDER);
	}
	if (mChallengeStateCounter > 0)
		return;

	if (mApp->mGameScene == GameScenes::SCENE_ZOMBIES_WON)
	{
		mChallengeStateCounter = STORM_FLASH_TIME + RandRangeInt(-50, 50);
		mChallengeState = ChallengeState::STATECHALLENGE_STORM_FLASH_3;
		return;
	}
	else if (mApp->mGameScene != GameScenes::SCENE_PLAYING)
	{
		mChallengeState = ChallengeState::STATECHALLENGE_NORMAL;
		mChallengeStateCounter = 0;
		return;
	}
	else
	{
		//判断是否存在雪人僵尸，存在则状态倒计时从 350~450 随机
		if (mBoard->CountZombieByType(ZombieType::ZOMBIE_YETI))
		{
			mChallengeStateCounter = STORM_FLASH_TIME + RandRangeInt(200, 300);
		}
		else
		{
			int aMaxDur = Sexy::Rand(2) ? 400 : 750;
			mChallengeStateCounter = STORM_FLASH_TIME + RandRangeInt(300, aMaxDur);
		}

		mChallengeState = static_cast<ChallengeState>(RandRangeInt(static_cast<int>(STATECHALLENGE_STORM_FLASH_1), static_cast<int>(STATECHALLENGE_STORM_FLASH_3)));
	}
}

// GOTY @Patoke: 0x426320
void Challenge::UpdateSlotMachine()
{
	int aSunMoney = ClampInt(mBoard->mSunMoney, 0, 2000);
	if (aSunMoney >= SLOT_MACHINE_WINNING_SCORE - 100)
	{
		mBoard->DisplayAdvice("[ADVICE_ALMOST_THERE]", MESSAGE_STYLE_HINT_FAST, ADVICE_ALMOST_THERE);
	}
	if (aSunMoney >= 2000)
	{
		SpawnLevelAward(4, 2);
		mBoard->ClearAdvice(ADVICE_NONE);
	}
	mBoard->mProgressMeterWidth = TodAnimateCurve(0, SLOT_MACHINE_WINNING_SCORE, aSunMoney, 0, PROGRESS_METER_COUNTER, CURVE_LINEAR);

	if (!mBoard->mAdvice->IsBeingDisplayed())
	{
		std::string aMsg = TodReplaceNumberString("[ADVICE_SLOT_MACHINE_COLLECT_SUN]", "{SCORE}", SLOT_MACHINE_WINNING_SCORE);
		mBoard->DisplayAdvice(aMsg, MESSAGE_STYLE_SLOT_MACHINE, ADVICE_SLOT_MACHINE_COLLECT_SUN);
	}

	if (mChallengeState != STATECHALLENGE_SLOT_MACHINE_ROLLING)
	{
		if (!mBoard->mAdvice->IsBeingDisplayed() && !mBoard->HasLevelAwardDropped())
		{
			mBoard->DisplayAdviceAgain("[ADVICE_SLOT_MACHINE_SPIN_AGAIN]", MESSAGE_STYLE_SLOT_MACHINE, ADVICE_SLOT_MACHINE_SPIN_AGAIN);
		}
	}
	else if (mBoard->mSeedBank->mSeedPackets[0].mSlotMachineCountDown <= 0)
	{
		mApp->ReanimationGet(mReanimChallenge)->PlayReanim("anim_return", REANIM_PLAY_ONCE_AND_HOLD, 0, 24.0f);
		mChallengeState = STATECHALLENGE_NORMAL;

		SeedType aPacket1 = mBoard->mSeedBank->mSeedPackets[0].mPacketType;
		SeedType aPacket2 = mBoard->mSeedBank->mSeedPackets[1].mPacketType;
		SeedType aPacket3 = mBoard->mSeedBank->mSeedPackets[2].mPacketType;
		if (aPacket1 != aPacket2 || aPacket2 != aPacket3)
		{
			if (aPacket1 == aPacket2 || aPacket2 == aPacket3 || aPacket1 == aPacket3)
			{
				mApp->PlayFoley(FOLEY_ART_CHALLENGE);
				SeedType aSeedType = (aPacket1 == aPacket2 || aPacket1 == aPacket3) ? aPacket1 : aPacket2;
				if (aSeedType == SEED_SLOT_MACHINE_DIAMOND)
				{
					mBoard->DisplayAdvice("[ADVICE_SLOT_MACHINE_2_DIAMONDS]", MESSAGE_STYLE_SLOT_MACHINE, ADVICE_NONE);
					mBoard->AddCoin(360, 85, COIN_DIAMOND, COIN_MOTION_COIN);
				}
				else if (aSeedType == SEED_SLOT_MACHINE_SUN)
				{
					mBoard->DisplayAdvice("[ADVICE_SLOT_MACHINE_2_SUNS]", MESSAGE_STYLE_SLOT_MACHINE, ADVICE_NONE);
					for (int i = 0; i < 4; i++)
					{
						mBoard->AddCoin(320 + i * 15, 85, COIN_SUN, COIN_MOTION_COIN);
					}
				}
				else
				{
					mBoard->DisplayAdvice("[ADVICE_SLOT_MACHINE_2_OF_A_KIND]", MESSAGE_STYLE_SLOT_MACHINE, ADVICE_NONE);
					mBoard->AddCoin(360, 85, COIN_USABLE_SEED_PACKET, COIN_MOTION_COIN)->mUsableSeedType = aSeedType;
				}
			}
		}
		else
		{
			mApp->PlayFoley(FOLEY_ART_CHALLENGE);
			if (aPacket1 == SEED_SLOT_MACHINE_DIAMOND)
			{
				mBoard->DisplayAdvice("[ADVICE_SLOT_MACHINE_DIAMOND_JACKPOT]", MESSAGE_STYLE_SLOT_MACHINE, ADVICE_NONE);
				for (int i = 0; i < 5; i++)
				{
					mBoard->AddCoin(320 + i * 12, 85, COIN_DIAMOND, COIN_MOTION_COIN);
				}
			}
			else if (aPacket1 == SEED_SLOT_MACHINE_SUN)
			{
				mBoard->DisplayAdvice("[ADVICE_SLOT_MACHINE_SUN_JACKPOT]", MESSAGE_STYLE_SLOT_MACHINE, ADVICE_NONE);
				for (int i = 0; i < 20; i++)
				{
					mBoard->AddCoin(320 + i * 3, 85, COIN_SUN, COIN_MOTION_COIN);
				}
			}
			else
			{
				mBoard->DisplayAdvice("[ADVICE_SLOT_MACHINE_3_OF_A_KIND]", MESSAGE_STYLE_SLOT_MACHINE, ADVICE_NONE);
				// @Patoke: fix silly bug XD
				for (int i = 0; i < 3; i++)
				{
					mBoard->AddCoin(320 + i * 20, 85, COIN_USABLE_SEED_PACKET, COIN_MOTION_COIN)->mUsableSeedType = aPacket1;
				}
			}
		}
	}
}

void Challenge::BeghouledCheckStuckState()
{
	if (mChallengeState != STATECHALLENGE_NORMAL || mBoard->HasLevelAwardDropped())
		return;

	BeghouledBoardState aBoardState;
	LoadBeghouledBoardState(&aBoardState);
	if (!BeghouledCheckForPossibleMoves(&aBoardState))
	{
		mChallengeState = STATECHALLENGE_BEGHOULED_NO_MATCHES;
		mChallengeStateCounter = 500;
		mBoard->DisplayAdviceAgain("[ADVICE_BEGHOULED_NO_MOVES]", MESSAGE_STYLE_HINT_FAST, ADVICE_BEGHOULED_NO_MOVES);
	}
}

void Challenge::ZombieAtePlant(/*Zombie* theZombie,*/ Plant* thePlant)
{
	// theZombie 参数在原版中被优化删去，此处参考内测版将其保留。注意：传入的 theZombie 可能为 nullptr
	if (mApp->mGameMode != GAMEMODE_CHALLENGE_BEGHOULED && mApp->mGameMode != GAMEMODE_CHALLENGE_BEGHOULED_TWIST)
		return;

	mBeghouledEated[thePlant->mPlantCol][thePlant->mRow] = true;

	if (mBoard->mSeedBank->mNumPackets == 4)
	{
		mBoard->mSeedBank->mNumPackets += 1;
		mBoard->mSeedBank->mSeedPackets[4].SetPacketType(SEED_BEGHOULED_BUTTON_CRATER);
		mBoard->DisplayAdvice("[ADVICE_BEGHOULED_USE_CRATER_1]", MESSAGE_STYLE_HINT_FAST, ADVICE_BEGHOULED_USE_CRATER_1);
	}

	BeghouledCheckStuckState();
	BeghouledUpdateCraters();
}

void Challenge::Update()
{
	if (mApp->IsStormyNightLevel())
		UpdateStormyNight();

	if (mBoard->mPaused)
	{
		if (mApp->mGameMode == GAMEMODE_CHALLENGE_BEGHOULED_TWIST)
		{
			mChallengeGridX = -1;
			mChallengeGridY = -1;
		}
		return;
	}

	if (mApp->mGameMode == GAMEMODE_CHALLENGE_RAINING_SEEDS || mApp->IsStormyNightLevel())
	{
		UpdateRain();
	}

	if (mApp->mGameScene != SCENE_PLAYING && mApp->mGameMode != GAMEMODE_TREE_OF_WISDOM)
		return;

	if (mBoard->HasConveyorBeltSeedBank())
	{
		UpdateConveyorBelt();
	}
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_BEGHOULED || mApp->mGameMode == GAMEMODE_CHALLENGE_BEGHOULED_TWIST)
	{
		UpdateBeghouled();
	}
	if (mApp->IsScaryPotterLevel())
	{
		ScaryPotterUpdate();
	}
	if ((mApp->IsScaryPotterLevel() || mApp->IsWhackAZombieLevel()) && mBoard->mSeedBank->mY < 0)
	{
		if (mBoard->mSunMoney + mBoard->CountSunBeingCollected() > 0 || mBoard->mSeedBank->mY > Sexy::IMAGE_SEEDBANK->mWidth)
		{
			mBoard->mSeedBank->mY += 2;
			if (mBoard->mSeedBank->mY > 0)
			{
				mBoard->mSeedBank->mY = 0;
			}
		}
	}

	if (mApp->IsWhackAZombieLevel())
	{
		WhackAZombieUpdate();
	}
	if (mApp->IsIZombieLevel())
	{
		IZombieUpdate();
	}
	if (mApp->IsSlotMachineLevel())
	{
		UpdateSlotMachine();
	}
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_SPEED)
	{
		mBoard->UpdateGame();
	}
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_RAINING_SEEDS)
	{
		UpdateRainingSeeds();
	}
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_PORTAL_COMBAT)
	{
		UpdatePortalCombat();
	}
	/* Unused
	if (mApp->IsSquirrelLevel())
	{
		SquirrelUpdate();
	}*/
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZOMBIQUARIUM)
	{
		ZombiquariumUpdate();
	}
	if (mApp->mGameMode == GAMEMODE_TREE_OF_WISDOM)
	{
		TreeOfWisdomUpdate();
	}
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ICE && mBoard->mMainCounter == 3000)
	{
		mApp->PlayFoley(FOLEY_FLOOP);
		mApp->PlaySample(Sexy::SOUND_LOSEMUSIC);
	}
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_LAST_STAND)
	{
		LastStandUpdate();
	}
	
	Reanimation* aReanim = mApp->ReanimationTryToGet(mReanimChallenge);
	if (aReanim && aReanim->mIsAttachment)
	{
		aReanim->Update();
	}
}

SeedType Challenge::GetArtChallengeSeed(int theGridX, int theGridY)
{
	if (theGridY < 6)
	{
		TOD_ASSERT(theGridX >= 0 && theGridX < ART_CHALLEGE_SIZE_X && theGridY >= 0);

		GameMode aGameMode = mApp->mGameMode;
		if (aGameMode == GAMEMODE_CHALLENGE_ART_CHALLENGE_WALLNUT)		return gArtChallengeWallnut[theGridY][theGridX];
		if (aGameMode == GAMEMODE_CHALLENGE_ART_CHALLENGE_SUNFLOWER)	return gArtChallengeSunFlower[theGridY][theGridX];
		if (aGameMode == GAMEMODE_CHALLENGE_SEEING_STARS)				return gArtChallengeStarFruit[theGridY][theGridX];
	}
	return SEED_NONE;
}

void Challenge::SpawnLevelAward(int theGridX, int theGridY)
{
	if (mBoard->HasLevelAwardDropped())
		return;

	float aPosX = mBoard->GridToPixelX(theGridX, theGridY) + 40;
	float aPosY = mBoard->GridToPixelY(theGridX, theGridY) + 40;
	CoinType aCoinType = 
		mApp->IsFirstTimeAdventureMode() ? COIN_FINAL_SEED_PACKET : 
		mApp->IsAdventureMode() || mApp->HasBeatenChallenge(mApp->mGameMode) ? COIN_AWARD_MONEY_BAG : COIN_TROPHY;
	
	mBoard->mLevelAwardSpawned = true;
	mApp->mBoardResult = BOARDRESULT_WON;
	mApp->PlayFoley(FOLEY_SPAWN_SUN);
	Coin* aCoin = mBoard->AddCoin(aPosX, aPosY, aCoinType, COIN_MOTION_COIN);
	mApp->AddTodParticle(BOARD_WIDTH / 2, BOARD_HEIGHT / 2, RENDER_LAYER_TOP, PARTICLE_SCREEN_FLASH);

	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ZOMBIQUARIUM)
	{
		aCoin->Collect();
	}
	else if (!mApp->IsIZombieLevel())
	{
		Zombie* aZombie = nullptr;
		while (mBoard->IterateZombies(aZombie))
		{
			if (!aZombie->IsDeadOrDying())
			{
				aZombie->TakeDamage(1800, 0U);
			}
		}
	}
}

void Challenge::CheckForCompleteArtChallenge(int theGridX, int theGridY)
{
	if (mBoard->HasLevelAwardDropped())
		return;

	for (int aGridY = 0; aGridY < MAX_GRID_SIZE_Y; aGridY++)
	{
		for (int aGridX = 0; aGridX < MAX_GRID_SIZE_X; aGridX++)
		{
			SeedType aSeedType = GetArtChallengeSeed(aGridX, aGridY);
			if (aSeedType != SEED_NONE)
			{
				Plant* aPlant = mBoard->GetTopPlantAt(aGridX, aGridY, TOPPLANT_ONLY_NORMAL_POSITION);
				if (aPlant == nullptr || aPlant->mSeedType != aSeedType)
				{
					return;
				}
			}
		}
	}
	SpawnLevelAward(theGridX, theGridY);
}

void Challenge::DrawArtChallenge(Graphics* g)
{
	g->SetColorizeImages(true);
	g->SetColor(Color(255, 255, 255, 100));

	for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++)
	{
		for (int aCol = 0; aCol < MAX_GRID_SIZE_X; aCol++)
		{
			SeedType aSeedType = GetArtChallengeSeed(aCol, aRow);
			if (aSeedType != SEED_NONE && mBoard->GetTopPlantAt(aCol, aRow, TOPPLANT_ONLY_NORMAL_POSITION) == nullptr)
			{
				Plant::DrawSeedType(g, aSeedType, SEED_NONE, VARIATION_NORMAL, mBoard->GridToPixelX(aCol, aRow), mBoard->GridToPixelY(aCol, aRow));
			}
		}
	}
	
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_ART_CHALLENGE_WALLNUT)
	{
		// @Patoke: assets, removed, perhaps add back?
		//g->DrawImage(Sexy::IMAGE_GOOGLYEYE, 357, 174);
		//g->DrawImage(Sexy::IMAGE_GOOGLYEYE, 516, 174);
	}

	g->SetColorizeImages(false);
}

void Challenge::DrawBeghouled(Graphics* g)
{
	for (int aGridY = 0; aGridY < MAX_GRID_SIZE_Y; aGridY++)
	{
		for (int aGridX = 0; aGridX < MAX_GRID_SIZE_X; aGridX++)
		{
			if (mBeghouledEated[aGridX][aGridY])
			{
				g->DrawImageCel(Sexy::IMAGE_CRATER, mBoard->GridToPixelX(aGridX, aGridY) - 8, mBoard->GridToPixelY(aGridX, aGridY) + 40, 1, 0);
			}
		}
	}
			
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_BEGHOULED_TWIST)
	{
		HitResult aHitResult;
		mBoard->MouseHitTest(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY, &aHitResult);
		if (mChallengeGridX != -1 && mChallengeGridY != -1 && aHitResult.mObjectType != OBJECT_TYPE_COIN)
		{
			float aPixelX = mBoard->GridToPixelX(mChallengeGridX, mChallengeGridY) + 80;
			float aPixelY = mBoard->GridToPixelY(mChallengeGridX, mChallengeGridY) + 100;

			// Rotation speed coefficient is 2*PI*0.001, which means one full rotation every 1000 frames. Modulus is to avoid large angle float precision cliffs.
			constexpr uint32_t BEGHOULED_TWIST_ROTATION_PERIOD = 1000;
			float aRotation = -static_cast<float>(mBoard->mMainCounter % BEGHOULED_TWIST_ROTATION_PERIOD) * (2 * PI * 0.001f);
			SexyTransform2D aTransform;
			TodScaleRotateTransformMatrix(aTransform, aPixelX, aPixelY, aRotation, 1, 1);

			Image* aImageOverlay = Sexy::IMAGE_BEGHOULED_TWIST_OVERLAY;
			Rect aSrcRect = Rect(0, 0, aImageOverlay->mWidth, aImageOverlay->mHeight);
			TodBltMatrix(g, aImageOverlay, aTransform, g->mClipRect, Color(255, 255, 255, 128), g->mDrawMode, aSrcRect);
		}
	}
}

void Challenge::DrawSlotMachine(Graphics* g)
{
	if (mApp->mGameScene == SCENE_ZOMBIES_WON)
		return;

	Graphics gBoardParent = Graphics(*g);
	if (mSlotMachineRollCount < 3 && mBoard->mCursorObject->mCursorType == CURSOR_TYPE_NORMAL &&
		mChallengeState != STATECHALLENGE_SLOT_MACHINE_ROLLING && !mBoard->HasLevelAwardDropped())
	{
		gBoardParent.SetColor(GetFlashingColor(mBoard->mMainCounter, 75));
		gBoardParent.SetColorizeImages(true);
	}
	gBoardParent.mTransX = mBoard->mSeedBank->mX - mBoard->mX;
	gBoardParent.mTransY = mBoard->mSeedBank->mY - mBoard->mY;
	mApp->ReanimationGet(mReanimChallenge)->Draw(&gBoardParent);
}

void Challenge::DrawBackdrop(Graphics* g)
{
	GameMode aGameMode = mApp->mGameMode;
	if (mApp->IsArtChallenge())
	{
		DrawArtChallenge(g);
	}
	if (mApp->mGameMode == GAMEMODE_TREE_OF_WISDOM)
	{
		TreeOfWisdomDraw(g);
	}
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_BEGHOULED || mApp->mGameMode == GAMEMODE_CHALLENGE_BEGHOULED_TWIST)
	{
		DrawBeghouled(g);
	}

	if (mApp->IsWallnutBowlingLevel() && mShowBowlingLine)
	{
		g->DrawImage(Sexy::IMAGE_WALLNUT_BOWLINGSTRIPE, 268, 77);
	}
	if (mApp->mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_1 || mApp->mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_2 || mApp->mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_3 ||
		mApp->mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_4 || mApp->mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_5)
	{
		g->DrawImage(Sexy::IMAGE_WALLNUT_BOWLINGSTRIPE, 352, 73);
	}
	if (mApp->mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_6 || mApp->mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_7 || mApp->mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_8 ||
		mApp->mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_ENDLESS)
	{
		g->DrawImage(Sexy::IMAGE_WALLNUT_BOWLINGSTRIPE, 432, 73);
	}
	if (mApp->mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_9)
	{
		g->DrawImage(Sexy::IMAGE_WALLNUT_BOWLINGSTRIPE, 512, 73);
	}

	if (aGameMode == GAMEMODE_CHALLENGE_SLOT_MACHINE)
	{
		DrawSlotMachine(g);
	}
	if (aGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN)
	{
		mApp->mZenGarden->DrawBackdrop(g);
	}
}

void Challenge::PlantAdded(Plant* thePlant)
{
	if (mApp->IsArtChallenge())
	{
		SeedType anArtSeed = GetArtChallengeSeed(thePlant->mPlantCol, thePlant->mRow);
		if (anArtSeed != SEED_NONE && anArtSeed == thePlant->mSeedType)
		{
			mApp->PlayFoley(FOLEY_ART_CHALLENGE);
			mApp->AddTodParticle(thePlant->mX + 40, thePlant->mY + 40, RENDER_LAYER_TOP, PARTICLE_PRESENT_PICKUP);
			CheckForCompleteArtChallenge(thePlant->mPlantCol, thePlant->mRow);
		}
	}
}

PlantingReason Challenge::CanPlantAt(int theGridX, int theGridY, SeedType theSeedType)
{
	if (mApp->IsWallnutBowlingLevel())
	{
		return theGridX > 2 ? PLANTING_NOT_PASSED_LINE : PLANTING_OK;
	}
	else if (mApp->IsIZombieLevel())
	{
		int aLimit = 6;
		if (mApp->mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_1 || mApp->mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_2 || mApp->mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_3 ||
			mApp->mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_4 || mApp->mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_5)
		{
			aLimit = 4;
		}
		if (mApp->mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_6 || mApp->mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_7 || mApp->mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_8 ||
			mApp->mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_ENDLESS)
		{
			aLimit = 5;
		}

		if (theSeedType == SEED_ZOMBIE_BUNGEE)
		{
			return theGridX < aLimit ? PLANTING_OK : PLANTING_NOT_HERE;
		}
		else if (IsZombieSeedType(theSeedType))
		{
			return theGridX >= aLimit ? PLANTING_OK : PLANTING_NOT_HERE;
		}
	}
	else if (mApp->IsArtChallenge())
	{
		SeedType anArtSeed = GetArtChallengeSeed(theGridX, theGridY);
		if (anArtSeed != SEED_NONE && anArtSeed != theSeedType && theSeedType != SEED_LILYPAD && theSeedType != SEED_PUMPKINSHELL)
		{
			return PLANTING_NOT_ON_ART;
		}
		if (mApp->mGameMode == GAMEMODE_CHALLENGE_ART_CHALLENGE_WALLNUT)
		{
			if ((theGridX == 4 || theGridX == 6) && theGridY == 1)
			{
				return PLANTING_NOT_HERE;
			}
		}
	}
	else if (mApp->IsFinalBossLevel() && theGridX >= 8)
	{
		return PLANTING_NOT_HERE;
	}

	return PLANTING_OK;
}

void Challenge::InitZombieWavesSurvival()
{
	mBoard->mZombieAllowed[ZOMBIE_NORMAL] = true;
	MTRand aLevelRNG = MTRand(mBoard->GetLevelRandSeed());

	if (aLevelRNG.Next((unsigned long)5) == 0)
	{
		mBoard->mZombieAllowed[ZOMBIE_NEWSPAPER] = true;
	}
	else
	{
		mBoard->mZombieAllowed[ZOMBIE_TRAFFIC_CONE] = true;
	}

	int aCapacity = std::min(mSurvivalStage + 1, 9);
	while (aCapacity > 0)
	{
		ZombieType aRandZombie = (ZombieType)aLevelRNG.Next((unsigned long)NUM_ZOMBIE_TYPES);
		if (mBoard->mZombieAllowed[aRandZombie])																	continue;
		if (mBoard->IsZombieTypePoolOnly(aRandZombie) && !mBoard->StageHasPool())									continue;
		if (mBoard->StageHasRoof() && (aRandZombie == ZOMBIE_DIGGER || aRandZombie == ZOMBIE_DANCER))				continue;
		if (mBoard->StageHasGraveStones() && aRandZombie == ZOMBIE_ZAMBONI)											continue;
		if (!mBoard->StageHasRoof() && !mApp->IsSurvivalEndless(mApp->mGameMode) && aRandZombie == ZOMBIE_BUNGEE)	continue;
		if (mBoard->GetSurvivalFlagsCompleted() < 10 && aRandZombie == ZOMBIE_REDEYE_GARGANTUAR)								continue;
		if (mApp->IsSurvivalNormal(mApp->mGameMode) && aRandZombie > ZOMBIE_SNORKEL)								continue;
		if (mBoard->IsZombieTypeSpawnedOnly(aRandZombie) || Zombie::IsZombotany(aRandZombie) ||
			aRandZombie == ZOMBIE_DUCKY_TUBE || aRandZombie == ZOMBIE_YETI)											continue;

		mBoard->mZombieAllowed[aRandZombie] = true;
		aCapacity--;
	}
}

void Challenge::InitZombieWavesFromList(ZombieType* theZombieList, int theListLength)
{
	for (int i = 0; i < theListLength; i++)
	{
		mBoard->mZombieAllowed[theZombieList[i]] = true;
	}
}

void Challenge::InitZombieWaves()
{
	GameMode aGameMode = mApp->mGameMode;
	bool* aList = mBoard->mZombieAllowed;
	if (mApp->IsSurvivalMode())
	{
		if (mSurvivalStage == 0)
		{
			if (mApp->IsSurvivalNormal(mApp->mGameMode))
			{
				aList[ZOMBIE_NORMAL] = true;
				aList[ZOMBIE_TRAFFIC_CONE] = true;
			}
			else
			{
				aList[ZOMBIE_NORMAL] = true;
				aList[ZOMBIE_TRAFFIC_CONE] = true;
				aList[ZOMBIE_PAIL] = true;
			}
		}
		else InitZombieWavesSurvival();
	}
	else if (aGameMode == GAMEMODE_CHALLENGE_SPEED)
	{
		aList[ZOMBIE_NORMAL] = true;
		aList[ZOMBIE_TRAFFIC_CONE] = true;
		aList[ZOMBIE_DOLPHIN_RIDER] = true;
		aList[ZOMBIE_POLEVAULTER] = true;
	}
	else if (aGameMode == GAMEMODE_CHALLENGE_POGO_PARTY)
	{
		aList[ZOMBIE_POGO] = true;
	}
	else if (mApp->IsBungeeBlitzLevel())
	{
		aList[ZOMBIE_NORMAL] = true;
		aList[ZOMBIE_TRAFFIC_CONE] = true;
		aList[ZOMBIE_PAIL] = true;
		aList[ZOMBIE_LADDER] = true;
	}
	else if (aGameMode == GAMEMODE_CHALLENGE_SUNNY_DAY)
	{
		aList[ZOMBIE_NORMAL] = true;
		aList[ZOMBIE_PAIL] = true;
		aList[ZOMBIE_TRAFFIC_CONE] = true;
		aList[ZOMBIE_FOOTBALL] = true;
		aList[ZOMBIE_POLEVAULTER] = true;
		aList[ZOMBIE_JACK_IN_THE_BOX] = true;
	}
	else if (aGameMode == GAMEMODE_CHALLENGE_PORTAL_COMBAT)
	{
		aList[ZOMBIE_NORMAL] = true;
		aList[ZOMBIE_PAIL] = true;
		aList[ZOMBIE_FOOTBALL] = true;
		aList[ZOMBIE_BALLOON] = true;
	}
	else if (mApp->IsLittleTroubleLevel())
	{
		aList[ZOMBIE_NORMAL] = true;
		aList[ZOMBIE_TRAFFIC_CONE] = true;
		aList[ZOMBIE_FOOTBALL] = true;
		aList[ZOMBIE_SNORKEL] = true;
	}
	else if (aGameMode == GAMEMODE_CHALLENGE_BIG_TIME)
	{
		aList[ZOMBIE_NORMAL] = true;
		aList[ZOMBIE_TRAFFIC_CONE] = true;
		aList[ZOMBIE_PAIL] = true;
		aList[ZOMBIE_DOOR] = true;
		aList[ZOMBIE_FOOTBALL] = true;
		aList[ZOMBIE_JACK_IN_THE_BOX] = true;
	}
	else if (aGameMode == GAMEMODE_CHALLENGE_RAINING_SEEDS)
	{
		aList[ZOMBIE_NORMAL] = true;
		aList[ZOMBIE_TRAFFIC_CONE] = true;
		aList[ZOMBIE_PAIL] = true;
		aList[ZOMBIE_DOOR] = true;
		aList[ZOMBIE_FOOTBALL] = true;
		aList[ZOMBIE_NEWSPAPER] = true;
		aList[ZOMBIE_JACK_IN_THE_BOX] = true;
		aList[ZOMBIE_BUNGEE] = true;
	}
	else if (aGameMode == GAMEMODE_CHALLENGE_HIGH_GRAVITY)
	{
		aList[ZOMBIE_NORMAL] = true;
		aList[ZOMBIE_TRAFFIC_CONE] = true;
		aList[ZOMBIE_PAIL] = true;
		aList[ZOMBIE_DOOR] = true;
		aList[ZOMBIE_BALLOON] = true;
	}
	else if (mApp->IsWhackAZombieLevel())
	{
		aList[ZOMBIE_NORMAL] = true;
		aList[ZOMBIE_TRAFFIC_CONE] = true;
		aList[ZOMBIE_PAIL] = true;
	}
	else if (aGameMode == GAMEMODE_CHALLENGE_BOBSLED_BONANZA)
	{
		aList[ZOMBIE_BOBSLED] = true;
		aList[ZOMBIE_ZAMBONI] = true;
	}
	else if (aGameMode == GAMEMODE_CHALLENGE_AIR_RAID)
	{
		aList[ZOMBIE_BALLOON] = true;
	}
	else if (aGameMode == GAMEMODE_CHALLENGE_BEGHOULED || aGameMode == GAMEMODE_CHALLENGE_BEGHOULED_TWIST)
	{
		aList[ZOMBIE_NORMAL] = true;
		aList[ZOMBIE_TRAFFIC_CONE] = true;
		aList[ZOMBIE_PAIL] = true;
		aList[ZOMBIE_DOOR] = true;
		aList[ZOMBIE_FOOTBALL] = true;
		aList[ZOMBIE_NEWSPAPER] = true;
	}
	else if (aGameMode == GAMEMODE_CHALLENGE_LAST_STAND)
	{
		aList[ZOMBIE_NORMAL] = true;
		aList[ZOMBIE_TRAFFIC_CONE] = true;
		aList[ZOMBIE_PAIL] = true;
		aList[ZOMBIE_DOOR] = true;
		aList[ZOMBIE_FOOTBALL] = true;
		aList[ZOMBIE_NEWSPAPER] = true;
		aList[ZOMBIE_JACK_IN_THE_BOX] = true;
		aList[ZOMBIE_POLEVAULTER] = true;
		aList[ZOMBIE_DOLPHIN_RIDER] = true;
		aList[ZOMBIE_LADDER] = true;
	}
	else if (aGameMode == GAMEMODE_CHALLENGE_COLUMN)
	{
		aList[ZOMBIE_NORMAL] = true;
		aList[ZOMBIE_TRAFFIC_CONE] = true;
		aList[ZOMBIE_PAIL] = true;
		aList[ZOMBIE_FOOTBALL] = true;
	}
	else if (aGameMode == GAMEMODE_CHALLENGE_INVISIGHOUL)
	{
		aList[ZOMBIE_NORMAL] = true;
		aList[ZOMBIE_TRAFFIC_CONE] = true;
		aList[ZOMBIE_PAIL] = true;
		aList[ZOMBIE_DOLPHIN_RIDER] = true;
		aList[ZOMBIE_ZAMBONI] = true;
		aList[ZOMBIE_JACK_IN_THE_BOX] = true;
	}
	else if (aGameMode == GAMEMODE_CHALLENGE_WAR_AND_PEAS)
	{
		aList[ZOMBIE_PEA_HEAD] = true;
		aList[ZOMBIE_WALLNUT_HEAD] = true;
	}
	else if (aGameMode == GAMEMODE_CHALLENGE_WAR_AND_PEAS_2)
	{
		aList[ZOMBIE_PEA_HEAD] = true;
		aList[ZOMBIE_WALLNUT_HEAD] = true;
		aList[ZOMBIE_JALAPENO_HEAD] = true;
		aList[ZOMBIE_GATLING_HEAD] = true;
		aList[ZOMBIE_SQUASH_HEAD] = true;
		aList[ZOMBIE_TALLNUT_HEAD] = true;
	}
	else if (mApp->IsShovelLevel())
	{
		aList[ZOMBIE_NORMAL] = true;
		aList[ZOMBIE_TRAFFIC_CONE] = true;
	}
	else if (mApp->IsWallnutBowlingLevel())
	{
		if (aGameMode == GAMEMODE_CHALLENGE_WALLNUT_BOWLING || mApp->IsAdventureMode())
		{
			aList[ZOMBIE_NORMAL] = true;
			aList[ZOMBIE_TRAFFIC_CONE] = true;
			aList[ZOMBIE_PAIL] = true;
			aList[ZOMBIE_POLEVAULTER] = true;
			aList[ZOMBIE_NEWSPAPER] = true;
		}
		else if (aGameMode == GAMEMODE_CHALLENGE_WALLNUT_BOWLING_2)
		{
			aList[ZOMBIE_NORMAL] = true;
			aList[ZOMBIE_TRAFFIC_CONE] = true;
			aList[ZOMBIE_PAIL] = true;
			aList[ZOMBIE_POLEVAULTER] = true;
			aList[ZOMBIE_NEWSPAPER] = true;
			aList[ZOMBIE_DANCER] = true;
			aList[ZOMBIE_DOOR] = true;
		}
	}
	else if (mApp->IsStormyNightLevel())
	{
		aList[ZOMBIE_NORMAL] = true;
		aList[ZOMBIE_TRAFFIC_CONE] = true;
		aList[ZOMBIE_DOLPHIN_RIDER] = true;
		aList[ZOMBIE_BALLOON] = true;
	}
	else
	{
		aList[ZOMBIE_NORMAL] = true;
		aList[ZOMBIE_TRAFFIC_CONE] = true;
		aList[ZOMBIE_PAIL] = true;
	}
	
	if (mApp->CanSpawnYetis() && !mApp->IsWhackAZombieLevel() && !mApp->IsLittleTroubleLevel())
		aList[ZOMBIE_YETI] = true;
}

void Challenge::WhackAZombiePlaceGraves(int theGraveCount)
{
	int aPickCount = 0;
	TodWeightedGridArray aPicks[36];

	for (int aCol = 3; aCol < MAX_GRID_SIZE_X; aCol++)
	{
		for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++)
		{
			if (!mBoard->CanAddGraveStoneAt(aCol, aRow))
				continue;

			TodWeightedGridArray* aPick = &aPicks[aPickCount++];
			aPick->mWeight = mBoard->GetTopPlantAt(aCol, aRow, TOPPLANT_ANY) ? 1 : 100000;
			aPick->mX = aCol;
			aPick->mY = aRow;
		}
	}

	theGraveCount = std::min(theGraveCount, aPickCount);

	if (aPickCount == 0 || theGraveCount <= 0)
		return;

	for (int i = 0; i < theGraveCount; i++)
	{
		TodWeightedGridArray* aPick = TodPickFromWeightedGridArray(aPicks, aPickCount);

		Plant* aPlant = nullptr;
		while (mBoard->IteratePlants(aPlant))
		{
			if (aPlant->mPlantCol == aPick->mX && aPlant->mRow == aPick->mY)
			{
				aPlant->Die();
			}
		}

		mBoard->AddAGraveStone(aPick->mX, aPick->mY);
		aPick->mWeight = 0;
	}
}

void Challenge::WhackAZombieSpawning()
{
	if (mBoard->mCurrentWave == mBoard->mNumWaves && mBoard->mZombieCountDown == 0)
		return;

	mBoard->mZombieCountDown--;
	if (mBoard->mZombieCountDown == 100 && mBoard->mCurrentWave > 0)
	{
		int aNumGraves = 5 - mBoard->GetGraveStonesCount();
		WhackAZombiePlaceGraves(std::max(1, aNumGraves));
	}
	if (mBoard->mZombieCountDown == 5)
	{
		mBoard->NextWaveComing();
	}
	if (mBoard->mZombieCountDown == 0)
	{
		mBoard->mZombieCountDown = 2000;
		mBoard->mZombieCountDownStart = mBoard->mZombieCountDown;
		if (mBoard->mCurrentWave < mBoard->mNumWaves) mBoard->mCurrentWave++;
		mChallengeStateCounter = mBoard->mCurrentWave == mBoard->mNumWaves ? 300 : 1;
	}
	else if (mBoard->mZombieCountDown < 300)
	{
		return;
	}

	if (--mChallengeStateCounter == 0)
	{
		// 根据当前波数计算当前处于的阶段
		int aPhase = ClampInt((mBoard->mCurrentWave - 1) * 6 / 12, 0, 5);
		// 在不同阶段下出现路障、铁桶僵尸及出现二、三只僵尸的权重
		const int aDoubleChance[6] = { 0, 30, 10, 10, 15, 18 };
		const int aTripleChance[6] = { 0, 0, 0, 0, 10, 13 };
		const int aPailChance[6] = { 0, 0, 0, 10, 15, 15 };
		const int aConeChance[6] = { 0, 0, 30, 30, 30, 30 };
		// 默认的僵尸数量为 1 只
		int aZombieCount = 1;
		// 默认的僵尸类型为普通僵尸
		ZombieType aZombieType = ZOMBIE_NORMAL;
		// 分别取僵尸类型及僵尸数量的随机值
		int aNumHit = Rand(100);
		int aTypeHit = Rand(100);
		// 判断是否为最后一波
		int aIsFinalWave = mBoard->mCurrentWave == mBoard->mNumWaves;

		// 确定僵尸数量
		if (aIsFinalWave)
		{
			aZombieCount = 20;
		}
		else if (aNumHit < aTripleChance[aPhase])
		{
			aZombieCount = 3;
		}
		else if (aNumHit < aTripleChance[aPhase] + aDoubleChance[aPhase])
		{
			aZombieCount = 2;
		}

		// 确定僵尸类型
		if (aTypeHit < aPailChance[aPhase] && aZombieCount < 3)
		{
			aZombieType = ZOMBIE_PAIL;
		}
		else if (aTypeHit < aPailChance[aPhase] + aConeChance[aPhase])
		{
			aZombieType = ZOMBIE_TRAFFIC_CONE;
		}

		TodWeightedArray aGridPicks[MAX_GRAVE_STONES];
		int aGridPicksCount = 0;

		GridItem* aGridItem = nullptr;
		while(mBoard->IterateGridItems(aGridItem))
		{
			if (aGridItem->mGridItemType == GRIDITEM_GRAVESTONE)
			{
				Plant* aPlant = mBoard->GetTopPlantAt(aGridItem->mGridX, aGridItem->mGridY, TOPPLANT_ONLY_NORMAL_POSITION);
				if (aPlant == nullptr || aPlant->mSeedType != SEED_GRAVEBUSTER)
				{
					aGridPicks[aGridPicksCount].mItem = reinterpret_cast<intptr_t>(aGridItem);
					aGridPicks[aGridPicksCount].mWeight = 1;
					aGridPicksCount++;
				}
			}
		}
		float aMaxSpeed = TodAnimateCurve(1, 12, mBoard->mCurrentWave, 1, 3, CURVE_EASE_IN);
		aZombieCount = std::min(aZombieCount, aGridPicksCount);

		for (int i = 0; i < aZombieCount; i++)
		{
			TodWeightedArray* aGrid = TodPickArrayItemFromWeightedArray(aGridPicks, aGridPicksCount);
			GridItem* aGraveStone = reinterpret_cast<GridItem*>(aGrid->mItem);
			aGrid->mWeight = 0;

			if (aIsFinalWave)
			{
				aZombieType = Rand(2) == 0 ? ZOMBIE_TRAFFIC_CONE : ZOMBIE_PAIL;
				aMaxSpeed = 2;
			}

			Zombie* aZombie = mBoard->AddZombie(aZombieType, mBoard->mCurrentWave);
			if (aZombie == nullptr)
				break;

			aZombie->RiseFromGrave(aGraveStone->mGridX, aGraveStone->mGridY);
			aZombie->mPhaseCounter = 50;
			aZombie->mVelX = RandRangeFloat(0.5f, aMaxSpeed);
			aZombie->UpdateAnimSpeed();
		}

		int aStateCounterMin = TodAnimateCurve(1, 12, mBoard->mCurrentWave, 100, 30, CURVE_LINEAR);
		int aStateCounterMax = TodAnimateCurve(1, 12, mBoard->mCurrentWave, 200, 60, CURVE_LINEAR);
		mChallengeStateCounter = RandRangeInt(aStateCounterMin, aStateCounterMax);
		if (aIsFinalWave)
		{
			mBoard->mZombieCountDown = 0;
			mChallengeStateCounter = 0;
		}
	}
}

int Challenge::UpdateZombieSpawning()
{
	if (mApp->IsWhackAZombieLevel())
	{
		WhackAZombieSpawning();
		//@Minerscale: Need to return truthy value to prevent normal spawning
		return true;
	}
	else return
		mApp->IsFinalBossLevel() ||
		mApp->mGameMode == GAMEMODE_CHALLENGE_ICE ||
		mApp->mGameMode == GAMEMODE_CHALLENGE_ZEN_GARDEN ||
		mApp->mGameMode == GAMEMODE_TREE_OF_WISDOM ||
		mApp->mGameMode == GAMEMODE_CHALLENGE_ZOMBIQUARIUM ||
		mApp->IsIZombieLevel() ||
		mApp->IsSquirrelLevel() ||
		mApp->IsScaryPotterLevel() ||
		(mApp->mGameMode == GAMEMODE_CHALLENGE_LAST_STAND && mChallengeState != STATECHALLENGE_LAST_STAND_ONSLAUGHT);
}

void Challenge::GraveDangerSpawnGraveAt(int theGridX, int theGridY)
{
	Plant* aPlant = nullptr;
	while (mBoard->IteratePlants(aPlant))
	{
		if (aPlant->mPlantCol == theGridX && aPlant->mRow == theGridY)
		{
			aPlant->Die();
		}
	}

	mBoard->mEnableGraveStones = true;
	GridItem* aGraveStone = mBoard->AddAGraveStone(theGridX, theGridY);
	if (aGraveStone)
	{
		aGraveStone->AddGraveStoneParticles();
	}
}

void Challenge::GraveDangerSpawnRandomGrave()
{
	int aPickCount = 0;
	TodWeightedGridArray aPicks[30];

	for (int aCol = 4; aCol < MAX_GRID_SIZE_X; aCol++)
	{
		for (int aRow = 0; aRow < MAX_GRID_SIZE_Y; aRow++)
		{
			if (mBoard->CanAddGraveStoneAt(aCol, aRow))
			{
				// Source: https://wiki.pvz1.com/doku.php?id=%E6%94%BB%E7%95%A5:%E5%A2%93%E7%A2%91
				aPicks[aPickCount].mWeight = mBoard->GetTopPlantAt(aCol, aRow, TOPPLANT_ANY) ? 1 : 100000;
				aPicks[aPickCount].mX = aCol;
				aPicks[aPickCount].mY = aRow;
				aPickCount++;
			}
		}
	}

	if (aPickCount)
	{
		TodWeightedGridArray* aPick = TodPickFromWeightedGridArray(aPicks, aPickCount);
		GraveDangerSpawnGraveAt(aPick->mX, aPick->mY);
	}
}

void Challenge::SpawnZombieWave()
{
	if (mApp->IsContinuousChallenge() && mBoard->mCurrentWave == mBoard->mNumWaves)
	{
		mBoard->mCurrentWave = mBoard->mNumWaves - 1;
		for (int i = 0; i < MAX_ZOMBIES_IN_WAVE; i++)
		{
			ZombieType aWaveZombie = mBoard->mZombiesInWave[mBoard->mCurrentWave][i];
			if (aWaveZombie == ZOMBIE_INVALID)
			{
				break;
			}
			if (aWaveZombie == ZOMBIE_FLAG)
			{
				mBoard->mZombiesInWave[mBoard->mCurrentWave][i] = ZOMBIE_NORMAL;
			}
		}
	}

	int aIsFlagWave = mBoard->IsFlagWave(mBoard->mCurrentWave);
	if (mApp->mGameMode == GAMEMODE_CHALLENGE_GRAVE_DANGER && mBoard->mCurrentWave != mBoard->mNumWaves - 1)
	{
		if (aIsFlagWave)
		{
			mBoard->SpawnZombiesFromGraves();
		}
		else if (mBoard->mCurrentWave > 5)
		{
			GraveDangerSpawnRandomGrave();
		}
	}
	if (mApp->IsSurvivalMode() && mBoard->mBackground == BACKGROUND_2_NIGHT && mBoard->mCurrentWave == mBoard->mNumWaves - 1)
	{
		int aNumGraves = mBoard->GetGraveStonesCount();
		if ((mApp->IsSurvivalNormal(mApp->mGameMode) && aNumGraves < 8) || aNumGraves < 12)
		{
			GraveDangerSpawnRandomGrave();
		}
	}
	if (mApp->IsBungeeBlitzLevel() && aIsFlagWave)
	{
		mBoard->DisplayAdvice("[ADVICE_BUNGEES_INCOMING]", MESSAGE_STYLE_HINT_FAST, ADVICE_NONE);
	}
}

void Challenge::DrawStormFlash(Graphics* g, int theTime, int theMaxAmount)
{
	MTRand aDrawRand = MTRand(mBoard->mMainCounter / 6);
	int aDarkness = TodAnimateCurve(150, 0, theTime, 255 - theMaxAmount, 255, CURVE_LINEAR) + aDrawRand.NextNoAssert((unsigned long)64) - 32;
	// 设置暴风雨阴暗的颜色
	g->SetColor(Color(0, 0, 0, ClampInt(aDarkness, 0, 255)));
	// 绘制暴风雨阴暗的主色
	g->FillRect(-1000, -1000, 2800, 2600);

	// 设置闪电亮光的颜色
	g->SetColor(Color(255, 255, 255, TodAnimateCurve(150, 75, theTime, theMaxAmount, 0, CURVE_LINEAR)));
	// 绘制闪光
	g->FillRect(-1000, -1000, 2800, 2600);
}

void Challenge::DrawWeather(Graphics* g)
{
	if (mApp->IsStormyNightLevel() || mApp->mGameMode == GAMEMODE_CHALLENGE_RAINING_SEEDS)
		DrawRain(g);

	if (mApp->IsStormyNightLevel())
		DrawStormNight(g);
}

void Challenge::DrawRain(Graphics* g)
{
	if (mBoard->mCutScene->IsBeforePreloading() || !mApp->Is3DAccelerated())
		return;

	// Whatever is going on here is a bit yikes
	int aBoardOffsetX;
	/*
	if (aBoardOffsetX > 0)
	{
		aBoardOffsetX = (mBoard->mX + 100) / 100 * -100;
	}
	else
	{
		aBoardOffsetX = mBoard->mX / 100 * -100;
	}
	*/

	aBoardOffsetX = mBoard->mX / 100 * -100;

	const uint32_t aTime = mBoard->mEffectCounter;
	int aTimeOffsetXEst = TodAnimateCurve(0, 100, static_cast<int>(aTime % 100u), 0, -100, CURVE_LINEAR);
	int aTimeOffsetYEst = TodAnimateCurve(0, 20, static_cast<int>(aTime % 20u), -100, 0, CURVE_LINEAR);

	// 绘制远景的雨
	for (int aHorCnt = 9; aHorCnt > 0; aHorCnt--)
	{
		for (int aVerCnt = 7; aVerCnt > 0; aVerCnt--)
		{
			int aImageX = aTimeOffsetXEst + 100 * aHorCnt + aBoardOffsetX;
			int aImageY = aTimeOffsetYEst + 100 * aVerCnt;
			g->DrawImage(Sexy::IMAGE_RAIN, aImageX, aImageY);
		}
	}

	float aTimeOffsetXCls = TodAnimateCurve(0, 161, static_cast<int>(aTime % 161u), 0, -100, CURVE_LINEAR);
	float aTimeOffsetYCls = TodAnimateCurve(0, 33, static_cast<int>(aTime % 33u), -100, 0, CURVE_LINEAR);
	// 绘制近景的雨
	for (int aHorCnt = 0; aHorCnt < 9; aHorCnt++)
	{
		for (int aVerCnt = 0; aVerCnt < 7; aVerCnt++)
		{
			float aRainScaleCls = 1.5f;
			float aImageClsX = (aHorCnt * 100 + aTimeOffsetXCls) * aRainScaleCls + aBoardOffsetX;
			float aImageClsY = (aVerCnt * 100 + aTimeOffsetYCls) * aRainScaleCls;
			TodDrawImageScaledF(g, Sexy::IMAGE_RAIN, aImageClsX, aImageClsY, aRainScaleCls, aRainScaleCls);
		}
	}
}

void Challenge::DrawStormNight(Graphics* g)
{
	if (mChallengeState == STATECHALLENGE_STORM_FLASH_1 && mChallengeStateCounter < 300)
	{
		if (mChallengeStateCounter > STORM_FLASH_TIME)
		{
			DrawStormFlash(g, mChallengeStateCounter - STORM_FLASH_TIME, 255);
		}
		else
		{
			DrawStormFlash(g, mChallengeStateCounter, 92);
		}
	}
	else if (mChallengeState == STATECHALLENGE_STORM_FLASH_2 && mChallengeStateCounter < 300)
	{
		DrawStormFlash(g, mChallengeStateCounter / 2, 255);
	}
	else if (mChallengeState == STATECHALLENGE_STORM_FLASH_3 && mChallengeStateCounter < 150)
	{
		DrawStormFlash(g, mChallengeStateCounter, 255);
	}
	else
	{
		g->SetColor(Color::Black);
		g->FillRect(-1000, -1000, BOARD_WIDTH + 2000, BOARD_HEIGHT + 2000);
	}

	mBoard->DrawUIBottom(g);
	mBoard->DrawTopRightUI(g);
}

void Challenge::PlayBossEnter()
{
	mBoard->AddZombie(ZOMBIE_BOSS, 0);
}

void Challenge::PortalStart()
{
	mChallengeStateCounter = 9000;
	GridItem* aPortal;
	
	aPortal = mBoard->mGridItems.DataArrayAlloc();
	aPortal->mGridItemType = GRIDITEM_PORTAL_SQUARE;
	aPortal->mGridX = 2;
	aPortal->mGridY = 0;
	aPortal->mRenderOrder = mBoard->MakeRenderOrder(RENDER_LAYER_PARTICLE, aPortal->mGridY, 0);
	aPortal->OpenPortal();

	aPortal = mBoard->mGridItems.DataArrayAlloc();
	aPortal->mGridItemType = GRIDITEM_PORTAL_SQUARE;
	aPortal->mGridX = 9;
	aPortal->mGridY = 1;
	aPortal->mRenderOrder = mBoard->MakeRenderOrder(RENDER_LAYER_PARTICLE, aPortal->mGridY, 0);
	aPortal->OpenPortal();

	aPortal = mBoard->mGridItems.DataArrayAlloc();
	aPortal->mGridItemType = GRIDITEM_PORTAL_CIRCLE;
	aPortal->mGridX = 9;
	aPortal->mGridY = 3;
	aPortal->mRenderOrder = mBoard->MakeRenderOrder(RENDER_LAYER_PARTICLE, aPortal->mGridY, 0);
	aPortal->OpenPortal();

	aPortal = mBoard->mGridItems.DataArrayAlloc();
	aPortal->mGridItemType = GRIDITEM_PORTAL_CIRCLE;
	aPortal->mGridX = 2;
	aPortal->mGridY = 4;
	aPortal->mRenderOrder = mBoard->MakeRenderOrder(RENDER_LAYER_PARTICLE, aPortal->mGridY, 0);
	aPortal->OpenPortal();

	mBoard->mZombieCountDown = 200;
	mBoard->mZombieCountDownStart = mBoard->mZombieCountDown;
	mConveyorBeltCounter = 200;
}

void Challenge::UpdatePortal(GridItem* thePortal)
{
	GridItem* anOtherPortal = GetOtherPortal(thePortal);
	if (anOtherPortal == nullptr)
		return;

	Zombie* aZombie = nullptr;
	while (mBoard->IterateZombies(aZombie))
	{
		if (aZombie->mRow == thePortal->mGridY && aZombie->mLastPortalX != thePortal->mGridX)
		{
			Rect aZombieRect = aZombie->GetZombieRect();
			int aZombieX = aZombieRect.mX + aZombieRect.mWidth / 2;
			int aPortalX = thePortal->mGridX * 80 + 25;
			if (abs(aZombieX - aPortalX) <= 45)
			{
				int aDiffX = aZombieX - aZombie->mX;
				if (aZombie->IsWalkingBackwards()) aDiffX -= 60;
				aZombie->mX = anOtherPortal->mGridX * 80 - aDiffX;
				aZombie->mPosX = aZombie->mX;

				aZombie->SetRow(anOtherPortal->mGridY);
				aZombie->mY = aZombie->GetPosYBasedOnRow(anOtherPortal->mGridY);
				aZombie->mPosY = aZombie->mY;

				aZombie->mLastPortalX = anOtherPortal->mGridX;
			}
		}
	}

	Projectile* aProjectile = nullptr;
	while(mBoard->IterateProjectiles(aProjectile))
	{
		if (aProjectile->mMotionType == MOTION_STRAIGHT && aProjectile->mRow == thePortal->mGridY && aProjectile->mLastPortalX != thePortal->mGridX)
		{
			Rect aProjectileRect = aProjectile->GetProjectileRect();
			int aProjectileX = aProjectileRect.mX + aProjectileRect.mWidth / 2;
			int aPortalX = thePortal->mGridX * 80 + 55;
			if (abs(aProjectileX - aPortalX) <= 40)
			{
				int aDeltaY = (anOtherPortal->mGridY - thePortal->mGridY) * 100;
				aProjectile->mX += anOtherPortal->mGridX * 80 - aProjectileX + 60;
				aProjectile->mPosX = aProjectile->mX;

				aProjectile->mRow = anOtherPortal->mGridY;
				aProjectile->mY += aDeltaY;
				aProjectile->mPosY = aProjectile->mY;
				aProjectile->mShadowY += aDeltaY;

				aProjectile->mLastPortalX = anOtherPortal->mGridX;
				aProjectile->mRenderOrder = Board::MakeRenderOrder(RENDER_LAYER_PROJECTILE, aProjectile->mRow, 0);
			}
		}
	}

	LawnMower* aLawnMower = nullptr;
	while (mBoard->IterateLawnMowers(aLawnMower))
	{
		if (aLawnMower->mMowerState == MOWER_TRIGGERED && aLawnMower->mRow == thePortal->mGridY && aLawnMower->mLastPortalX != thePortal->mGridX)
		{
			int aMowerX = aLawnMower->mPosX + 45;
			int aPortalX = thePortal->mGridX * 80 + 25;
			if (abs(aMowerX - aPortalX) <= 20)
			{
				aLawnMower->mPosX = anOtherPortal->mGridX * 80 + 25;
				aLawnMower->mRow = anOtherPortal->mGridY;
				aLawnMower->mPosY = (anOtherPortal->mGridY - thePortal->mGridY) * 100;
				aLawnMower->mLastPortalX = anOtherPortal->mGridX;
				aLawnMower->mRenderOrder = Board::MakeRenderOrder(RENDER_LAYER_LAWN_MOWER, aLawnMower->mRow, 0);
				aLawnMower->Update();
			}
		}
	}
}

GridItem* Challenge::GetOtherPortal(GridItem* thePortal)
{
	GridItem* aGridItem = nullptr;
	while (mBoard->IterateGridItems(aGridItem))
	{
		if (aGridItem != thePortal && aGridItem->mGridItemType == thePortal->mGridItemType)
		{
			return aGridItem;
		}
	}
	return nullptr;
}

GridItem* Challenge::GetPortalAt(int theGridX, int theGridY)
{
	GridItem* aGridItem = nullptr;
	while (mBoard->IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridX == theGridX && aGridItem->mGridY == theGridY && aGridItem->IsOpenPortal())
		{
			return aGridItem;
		}
	}
	return nullptr;
}

void Challenge::MoveAPortal()
{
	TodWeightedArray aPickArray[MAX_PORTALS];
	int aNumpicks = 0;

	GridItem* aGridItem = nullptr;
	while(mBoard->IterateGridItems(aGridItem))
	{
		if (aGridItem->IsOpenPortal())
		{
			TOD_ASSERT(aNumpicks < MAX_PORTALS);
			aPickArray[aNumpicks].mWeight = 1;
			aPickArray[aNumpicks].mItem = reinterpret_cast<intptr_t>(aGridItem);
			aNumpicks++;
		}
	}

	TOD_ASSERT(aNumpicks);
	GridItem* aPortal = reinterpret_cast<GridItem*>(TodPickFromWeightedArray(aPickArray, aNumpicks));
	GridItem* aOtherPortal = GetOtherPortal(aPortal);
	TOD_ASSERT(aOtherPortal);

	TodWeightedGridArray aGridArray[MAX_PICK_GRID_SIZE];
	int aGridArrayCount = 0;
	for (int aGridX = 0; aGridX < 10; aGridX++)
	{
		for (int aGridY = 0; aGridY < 5; aGridY++)
		{
			if (GetPortalAt(aGridX, aGridY) == nullptr && aOtherPortal->mGridX != aGridX && aOtherPortal->mGridY != aGridY)
			{
				TOD_ASSERT(aGridArrayCount < MAX_PICK_GRID_SIZE);
				aGridArray[aGridArrayCount].mX = aGridX;
				aGridArray[aGridArrayCount].mY = aGridY;
				aGridArray[aGridArrayCount].mWeight = 1;
				aGridArrayCount++;
			}
		}
	}

	TodWeightedGridArray* aGrid = TodPickFromWeightedGridArray(aGridArray, aGridArrayCount);
	GridItem* aNewPortal = mBoard->mGridItems.DataArrayAlloc();
	aNewPortal->mGridItemType = aPortal->mGridItemType;
	aNewPortal->mGridX = aGrid->mX;
	aNewPortal->mGridY = aGrid->mY;
	aNewPortal->mRenderOrder = Board::MakeRenderOrder(RENDER_LAYER_PARTICLE, aNewPortal->mGridY, 0);
	aNewPortal->OpenPortal();
	aPortal->ClosePortal();
}

void Challenge::UpdatePortalCombat()
{
	GridItem* aGridItem = nullptr;
	while (mBoard->IterateGridItems(aGridItem))
	{
		if (aGridItem->IsOpenPortal())
		{
			UpdatePortal(aGridItem);
		}
	}

	if (mBoard->HasLevelAwardDropped())
	{
		mBoard->ClearAdvice(ADVICE_PORTAL_RELOCATING);
	}
	else
	{
		mChallengeStateCounter--;
		if (mChallengeStateCounter == 500)
		{
			mBoard->DisplayAdviceAgain("[ADVICE_PORTAL_RELOCATING]", MESSAGE_STYLE_HINT_FAST, ADVICE_PORTAL_RELOCATING);
		}
		else if (mChallengeStateCounter <= 0)
		{
			mBoard->ClearAdvice(ADVICE_PORTAL_RELOCATING);
			mChallengeStateCounter = 6000;
			MoveAPortal();
		}
	}
}

float Challenge::PortalCombatRowSpawnWeight(int theGridY)
{
	if (GetPortalDistanceToMower(theGridY) < 5)
		return 0.01f;

	GridItem* aGridItem = 0;
	while (mBoard->IterateGridItems(aGridItem))
		if (aGridItem->IsOpenPortal() && aGridItem->mGridY == theGridY)
			return 1.0f;

	return 0.2f;
}

GridItem* Challenge::GetPortalToLeft(int theGridX, int theGridY)
{
	GridItem* aGridItemRecord = nullptr;
	
	GridItem* aGridItem = 0;
	while (mBoard->IterateGridItems(aGridItem))
	{
		if (aGridItem->IsOpenPortal() && aGridItem->mGridX < theGridX && aGridItem->mGridY == theGridY)
		{
			if (aGridItemRecord == nullptr || aGridItemRecord->mGridX < aGridItem->mGridX)
			{
				aGridItemRecord = aGridItem;
			}
		}
	}

	return aGridItemRecord;
}

int Challenge::GetPortalDistanceToMower(int theGridY)
{
	int aGridX = 10;
	int aGridY = theGridY;
	int aDistance = 0;

	while (aDistance < 40)
	{
		GridItem* aPortal = GetPortalToLeft(aGridX, aGridY);
		if (aPortal == nullptr)
		{
			aDistance += aGridX;
			break;
		}

		GridItem* aOtherPortal = GetOtherPortal(aPortal);
		TOD_ASSERT(aOtherPortal);

		aDistance += aGridX - aPortal->mGridX;
		aGridX = aOtherPortal->mGridX;
		aGridY = aOtherPortal->mGridY;
	}

	return aDistance;
}

GridItem* Challenge::GetPortalToRight(int theGridX, int theGridY)
{
	GridItem* aGridItemRecord = nullptr;

	GridItem* aGridItem = nullptr;
	while (mBoard->IterateGridItems(aGridItem))
	{
		if (aGridItem->IsOpenPortal() && aGridItem->mGridX > theGridX && aGridItem->mGridY == theGridY)
		{
			if (!aGridItemRecord || aGridItemRecord->mGridX > aGridItem->mGridX)
			{
				aGridItemRecord = aGridItem;
			}
		}
	}

	return aGridItemRecord;
}

// BONUS
GridItem* Challenge::GetPortalLeftRight(int theGridX, int theGridY, int theToLeft)
{
	GridItem* aGridItemRecord = nullptr;

	GridItem* aGridItem = nullptr;
	while (mBoard->IterateGridItems(aGridItem))
	{
		int aPortalX = aGridItem->mGridX;
		if (aPortalX == theGridX)
		{
			continue;  // 排除传送门与 theGridX 在同一列的情况
		}

		int aIsDir = (aPortalX > theGridX) ^ theToLeft;
		if (aGridItem->IsOpenPortal() && aIsDir && aGridItem->mGridY == theGridY)
		{
			int aIsCls = (aGridItemRecord->mGridX > aPortalX) ^ theToLeft;
			if (!aGridItemRecord || aIsCls)
			{
				aGridItemRecord = aGridItem;
			}
		}
	}

	return aGridItemRecord;
}
// BONUS_END

int Challenge::CanTargetZombieWithPortals(Plant* thePlant, Zombie* theZombie)
{
	int aGridX = thePlant->mPlantCol;
	int aGridY = thePlant->mRow;

	for (int i = 0; i < 3; i++)
	{
		GridItem* aPortal = GetPortalToRight(aGridX, aGridY);
		if (aGridY == theZombie->mRow)
		{
			int aRangeLeft = aGridX * 80;
			int aRangeRight = aPortal ? aPortal->mGridX * 80 : 800;
			if (theZombie->mX > aRangeLeft&& theZombie->mX < aRangeRight)
				return true;
		}

		if (aPortal)
		{
			GridItem* aOtherPortal = GetOtherPortal(aPortal);
			if (aOtherPortal)
			{
				aGridX = aOtherPortal->mGridX;
				aGridY = aOtherPortal->mGridY;
				continue;
			}
		}
		break;
	}
	return false;
}

void Challenge::BeghouledPacketClicked(SeedPacket* theSeedPacket)
{
	SeedType aPacketType = theSeedPacket->mPacketType;
	int aPacketCost = mBoard->GetCurrentPlantCost(aPacketType, SEED_NONE);
	if (!mBoard->CanTakeSunMoney(aPacketCost))
		return;

	int aUpgrade = aPacketType == SEED_REPEATER ? 0 : aPacketType == SEED_FUMESHROOM ? 1 : aPacketType == SEED_TALLNUT ? 2 : -1;
	if (aPacketType == SEED_BEGHOULED_BUTTON_SHUFFLE)
	{
		if (mChallengeState == STATECHALLENGE_BEGHOULED_FALLING || mChallengeState == STATECHALLENGE_BEGHOULED_MOVING)
			return;
		BeghouledShuffle();
	}
	else if (aPacketType == SEED_BEGHOULED_BUTTON_CRATER)
	{
		if (!BeghouledCanClearCrater() || mChallengeState == STATECHALLENGE_BEGHOULED_FALLING || mChallengeState == STATECHALLENGE_BEGHOULED_MOVING)
			return;
		BeghouledClearCrater(1);
		BeghouledStartFalling(STATECHALLENGE_BEGHOULED_FALLING);
	}
	else if (aUpgrade != -1 && !mBeghouledPurcasedUpgrade[aUpgrade])
	{
		mBeghouledPurcasedUpgrade[aUpgrade] = true;
		const SeedType gUpgradableSeedTypes[3] = { SEED_PEASHOOTER, SEED_PUFFSHROOM, SEED_WALLNUT };
		SeedType aSeedPrimary = gUpgradableSeedTypes[aUpgrade];
	
		Plant* aPlant = nullptr;
		while (mBoard->IteratePlants(aPlant))
		{
			if (aPlant->mSeedType == aSeedPrimary)
			{
				aPlant->Die();
				mBoard->AddPlant(aPlant->mPlantCol, aPlant->mRow, aPacketType, SEED_NONE);
			}
		}
		theSeedPacket->SetActivate(false);
	}
	mBoard->TakeSunMoney(aPacketCost);
}

void Challenge::BeghouledShuffle()
{
	mBoard->ClearAdvice(ADVICE_NONE);

	Plant* aPlant = nullptr;
	while (mBoard->IteratePlants(aPlant))
	{
		aPlant->Die();
	}

	BeghouledStartFalling(STATECHALLENGE_BEGHOULED_FALLING);
}

int Challenge::BeghouledCanClearCrater()
{
	for (int aRow = 0; aRow < 5; aRow++)
	{
		for (int aCol = 0; aCol < 8; aCol++)
		{
			if (mBeghouledEated[aCol][aRow])
			{
				return true;
			}
		}
	}

	return false;
}

Zombie* Challenge::ZombiquariumSpawnSnorkle()
{
	Zombie* aZombie = mBoard->AddZombieInRow(ZOMBIE_SNORKEL, 0, 0);
	aZombie->mPosX = RandRangeFloat(50, 650);
	aZombie->mPosY = RandRangeFloat(100, 400);
	return aZombie;
}

void Challenge::ZombiquariumPacketClicked(SeedPacket* theSeedPacket)
{
	int aCost = mBoard->GetCurrentPlantCost(theSeedPacket->mPacketType, SEED_NONE);
	if (mBoard->CanTakeSunMoney(aCost))
	{
		if (theSeedPacket->mPacketType == SEED_ZOMBIQUARIUM_SNORKLE)
		{
			if (mBoard->CountZombiesOnScreen() > 100)
				return;

			if (mBoard->mTutorialState == TUTORIAL_ZOMBIQUARIUM_BUY_SNORKEL)
			{
				mBoard->ClearAdvice(ADVICE_ZOMBIQUARIUM_BUY_SNORKEL);
				mBoard->TutorialArrowRemove();
				mBoard->mTutorialState = TUTORIAL_ZOMBIQUARIUM_BOUGHT_SNORKEL;
			}

			Zombie* aZombie = ZombiquariumSpawnSnorkle();
			mApp->PlayFoley(FOLEY_ZOMBIESPLASH);
			mApp->AddTodParticle(aZombie->mPosX + 60.0f, aZombie->mPosY + 20.0f, RENDER_LAYER_TOP, PARTICLE_PLANTING_POOL);
		}
		else if (theSeedPacket->mPacketType == SEED_ZOMBIQUARIUM_TROPHY)
		{
			SpawnLevelAward(2, 0);
			mBoard->ClearAdvice(ADVICE_NONE);
		}

		mBoard->TakeSunMoney(aCost);
	}
}

void Challenge::ZombiquariumDropBrain(int x, int y)
{
	mBoard->ClearAdvice(ADVICE_ZOMBIQUARIUM_CLICK_TO_FEED);
	GridItem* aBrain = mBoard->mGridItems.DataArrayAlloc();
	aBrain->mGridItemType = GRIDITEM_BRAIN;
	aBrain->mRenderOrder = 400000;
	aBrain->mGridX = 0;
	aBrain->mGridY = 0;
	aBrain->mGridItemCounter = 0;
	aBrain->mPosX = x - 15.0f;
	aBrain->mPosY = y - 15.0f;
	mApp->PlaySample(Sexy::SOUND_TAP);
}

void Challenge::ZombiquariumMouseDown(int x, int y)
{
	if (x < 80 || x > 720 || y < 90 || y > 430)
		return;
	
	int aBrainsCount = 0;
	GridItem* aGridItem = nullptr;
	while (mBoard->IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType == GRIDITEM_BRAIN)
		{
			aBrainsCount++;
		}
	}
	if (aBrainsCount < 3 && mBoard->TakeSunMoney(5))
	{
		ZombiquariumDropBrain(x, y);
	}
}

void Challenge::ZombiquariumUpdate()
{
	if (mBoard->mZombies.mSize == 0 && !mBoard->HasLevelAwardDropped())
	{
		mBoard->ZombiesWon(nullptr);
		return;
	}
	if (!mBoard->mAdvice->IsBeingDisplayed() && !mBoard->mHelpDisplayed[ADVICE_ZOMBIQUARIUM_COLLECT_SUN])
	{
		std::string aMsg = TodReplaceNumberString("[ADVICE_ZOMBIQUARIUM_COLLECT_SUN]", "{SCORE}", ZOMBIQUARIUM_WINNING_SCORE);
		mBoard->DisplayAdvice(aMsg, MESSAGE_STYLE_HINT_TALL_FAST, ADVICE_ZOMBIQUARIUM_COLLECT_SUN);
	}

	int aScore = ClampInt(mBoard->mSunMoney, 0, ZOMBIQUARIUM_WINNING_SCORE);
	mBoard->mProgressMeterWidth = TodAnimateCurve(0, ZOMBIQUARIUM_WINNING_SCORE, aScore, 0, PROGRESS_METER_COUNTER, CURVE_LINEAR);
	if (aScore >= ZOMBIQUARIUM_WINNING_SCORE - 100)
	{
		mBoard->DisplayAdvice("[ADVICE_ALMOST_THERE]", MESSAGE_STYLE_HINT_TALL_FAST, ADVICE_ALMOST_THERE);
	}
	if (aScore >= 110 && mBoard->mTutorialState == TUTORIAL_OFF)
	{
		mBoard->mTutorialState = TUTORIAL_ZOMBIQUARIUM_BUY_SNORKEL;
		float aPosX = mBoard->mSeedBank->mX + mBoard->mSeedBank->mSeedPackets[0].mX;
		float aPosY = mBoard->mSeedBank->mY + mBoard->mSeedBank->mSeedPackets[0].mY;
		mBoard->TutorialArrowShow(aPosX, aPosY);
		mBoard->DisplayAdvice("[ADVICE_ZOMBIQUARIUM_BUY_SNORKEL]", MESSAGE_STYLE_HINT_TALL_FAST, ADVICE_ZOMBIQUARIUM_BUY_SNORKEL);
	}
	else if (aScore < 100 && mBoard->mTutorialState == TUTORIAL_ZOMBIQUARIUM_BUY_SNORKEL)
	{
		mBoard->TutorialArrowRemove();
		mBoard->ClearAdvice(ADVICE_ZOMBIQUARIUM_BUY_SNORKEL);
		mBoard->mTutorialState = TUTORIAL_OFF;
	}
	if (aScore >= ZOMBIQUARIUM_WINNING_SCORE && mBoard->mTutorialState == TUTORIAL_ZOMBIQUARIUM_BOUGHT_SNORKEL)
	{
		mBoard->mTutorialState = TUTORIAL_ZOMBIQUARIUM_CLICK_TROPHY;
		float aPosX = mBoard->mSeedBank->mX + mBoard->mSeedBank->mSeedPackets[1].mX;
		float aPosY = mBoard->mSeedBank->mY + mBoard->mSeedBank->mSeedPackets[1].mY;
		mBoard->TutorialArrowShow(aPosX, aPosY);
		mBoard->DisplayAdvice("[ADVICE_ZOMBIQUARIUM_CLICK_TROPHY]", MESSAGE_STYLE_HINT_TALL_FAST, ADVICE_ZOMBIQUARIUM_CLICK_TROPHY);
	}
	else if (aScore < ZOMBIQUARIUM_WINNING_SCORE && mBoard->mTutorialState == TUTORIAL_ZOMBIQUARIUM_CLICK_TROPHY)
	{
		mBoard->TutorialArrowRemove();
		mBoard->ClearAdvice(ADVICE_ZOMBIQUARIUM_CLICK_TROPHY);
		mBoard->mTutorialState = TUTORIAL_ZOMBIQUARIUM_BOUGHT_SNORKEL;
	}

	GridItem* aGridItem = nullptr;
	while(mBoard->IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType == GRIDITEM_BRAIN)
		{
			aGridItem->mGridItemCounter++;
			aGridItem->mPosY += 0.15f;
			if (aGridItem->mPosY >= 500.0f)
			{
				aGridItem->GridItemDie();
			}
		}
	}
}

void Challenge::ShovelAddWallnuts()
{
	for (int aCol = 0; aCol < MAX_GRID_SIZE_X; aCol++)
	{
		for (int aRow = 0; aRow < MAX_GRID_SIZE_Y - 1; aRow++)
		{
			mBoard->AddPlant(aCol, aRow, SEED_WALLNUT, SEED_NONE);
		}
	}
}

void Challenge::ScaryPotterDontPlaceInCol(int theCol, TodWeightedGridArray* theGridArray, int theGridArrayCount)
{
	for (int i = 0; i < theGridArrayCount; i++)
	{
		if (theGridArray[i].mX == theCol)
		{
			theGridArray[i].mWeight = 0;
		}
	}
}

void Challenge::ScaryPotterFillColumnWithPlant(int theCol, SeedType theSeedType, TodWeightedGridArray* theGridArray, int theGridArrayCount)
{
	ScaryPotterDontPlaceInCol(theCol, theGridArray, theGridArrayCount);

	for (int i = 0; i < MAX_GRID_SIZE_Y - 1; i++)
	{
		Plant* aPlant = mBoard->NewPlant(theCol, i, theSeedType, SeedType::SEED_NONE);
		if (theSeedType == SeedType::SEED_POTATOMINE)
		{
			aPlant->mStateCountdown = 10;
		}
	}
}

// GOTY @Patoke: 0x42AFA0
void Challenge::ScaryPotterPlacePot(ScaryPotType theScaryPotType, ZombieType theZombieType, SeedType theSeedType, int theCount, TodWeightedGridArray* theGridArray, int theGridArrayCount)
{
	ScaryPotType aPotType = theScaryPotType;
	while (theCount > 0)
	{
		TodWeightedGridArray* aGrid = TodPickFromWeightedGridArray(theGridArray, theGridArrayCount);

		GridItem* aScaryPot = mBoard->mGridItems.DataArrayAlloc();
		aScaryPot->mGridItemType = GRIDITEM_SCARY_POT;
		aScaryPot->mGridItemState = GRIDITEM_STATE_SCARY_POT_QUESTION;
		aScaryPot->mGridX = aGrid->mX;
		aScaryPot->mGridY = aGrid->mY;
		aGrid->mWeight = 0;
		aScaryPot->mRenderOrder = mBoard->MakeRenderOrder(RENDER_LAYER_PLANT, aGrid->mY, 0);
		aScaryPot->mZombieType = theZombieType; aScaryPot->mSeedType = theSeedType; aScaryPot->mScaryPotType = aPotType;
		if (aPotType == SCARYPOT_SUN)
		{
			aScaryPot->mSunCount = Rand(3) + 1;
		}
		--theCount;
	}
}

// GOTY @Patoke: 0x42B040
void Challenge::ScaryPotterChangePotType(GridItemState thePotType, int theCount)
{
	TodWeightedArray aPotArray[MAX_SCARY_POTS];
	int aPotCount = 0;

	GridItem* aGridItem = nullptr;
	while (mBoard->IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType == GRIDITEM_SCARY_POT)
		{
			if ((thePotType == GRIDITEM_STATE_SCARY_POT_LEAF && aGridItem->mScaryPotType == SCARYPOT_SEED) ||
				(thePotType == GRIDITEM_STATE_SCARY_POT_ZOMBIE && aGridItem->mZombieType == ZOMBIE_GARGANTUAR))
			{
				aPotArray[aPotCount].mItem = reinterpret_cast<intptr_t>(aGridItem);
				aPotArray[aPotCount].mWeight = 1;
				aPotCount++;
			}
		}
	}
	theCount = std::min(theCount, aPotCount);

	for (int i = 0; i < theCount; i++)
	{
		TodWeightedArray* aScaryPotArray = TodPickArrayItemFromWeightedArray(aPotArray, aPotCount);
		aScaryPotArray->mWeight = 0;
		((GridItem*)(aScaryPotArray->mItem))->mGridItemState = thePotType;
	}
}

// GOTY @Patoke: 0x42B0E0
void Challenge::ScaryPotterPopulate()
{
	int aGridArrayCount = 0;
	TodWeightedGridArray aGridArray[MAX_SCARY_POTS];
	for (int aGridX = 0; aGridX < MAX_GRID_SIZE_X; aGridX++)
	{
		for (int aGridY = 0; aGridY < MAX_GRID_SIZE_Y - 1; aGridY++)
		{
			aGridArray[aGridArrayCount].mX = aGridX;
			aGridArray[aGridArrayCount].mY = aGridY;
			aGridArray[aGridArrayCount].mWeight = 1;
			aGridArrayCount++;

			TOD_ASSERT(aGridArrayCount <= MAX_SCARY_POTS);
		}
	}

	if (mApp->IsAdventureMode() && mBoard->mLevel == 35)
	{
		switch (mSurvivalStage)
		{
		case 0:
			ScaryPotterDontPlaceInCol(0, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(1, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(2, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(3, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(4, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(5, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_PEASHOOTER, 5, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_SQUASH, 5, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_NORMAL, SEED_NONE, 4, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_PAIL, SEED_NONE, 1, aGridArray, aGridArrayCount);
			break;
		case 1:
			ScaryPotterDontPlaceInCol(0, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(1, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(2, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(3, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(4, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_PEASHOOTER, 4, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_SNOWPEA, 5, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_SQUASH, 4, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_NORMAL, SEED_NONE, 5, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_PAIL, SEED_NONE, 1, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_FOOTBALL, SEED_NONE, 1, aGridArray, aGridArrayCount);
			ScaryPotterChangePotType(GRIDITEM_STATE_SCARY_POT_LEAF, 2);
			break;
		case 2:
			ScaryPotterDontPlaceInCol(0, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(1, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(2, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(3, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_PEASHOOTER, 5, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_SNOWPEA, 5, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_HYPNOSHROOM, 5, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_NORMAL, SEED_NONE, 6, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_PAIL, SEED_NONE, 2, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_DANCER, SEED_NONE, 1, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_JACK_IN_THE_BOX, SEED_NONE, 1, aGridArray, aGridArrayCount);
			ScaryPotterChangePotType(GRIDITEM_STATE_SCARY_POT_LEAF, 3);
			break;
		}
	}
	else
	{
		switch (mApp->mGameMode)
		{
		case GAMEMODE_SCARY_POTTER_1:
			ScaryPotterDontPlaceInCol(0, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(1, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(2, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(3, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_PEASHOOTER, 5, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_SNOWPEA, 5, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_SQUASH, 5, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_NORMAL, SEED_NONE, 6, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_PAIL, SEED_NONE, 3, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_JACK_IN_THE_BOX, SEED_NONE, 1, aGridArray, aGridArrayCount);
			ScaryPotterChangePotType(GRIDITEM_STATE_SCARY_POT_LEAF, 2);
			break;
		case GAMEMODE_SCARY_POTTER_2:
			ScaryPotterDontPlaceInCol(0, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(1, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(2, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(8, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_LEFTPEATER, 7, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_SNOWPEA, 3, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_WALLNUT, 3, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_POTATOMINE, 2, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_NORMAL, SEED_NONE, 6, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_PAIL, SEED_NONE, 3, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_JACK_IN_THE_BOX, SEED_NONE, 1, aGridArray, aGridArrayCount);
			ScaryPotterChangePotType(GRIDITEM_STATE_SCARY_POT_LEAF, 2);
			break;
		case GAMEMODE_SCARY_POTTER_3:
			ScaryPotterDontPlaceInCol(0, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(1, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(2, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_LEFTPEATER, 6, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_SNOWPEA, 4, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_SQUASH, 2, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_HYPNOSHROOM, 3, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_WALLNUT, 3, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_NORMAL, SEED_NONE, 8, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_PAIL, SEED_NONE, 2, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_DANCER, SEED_NONE, 1, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_JACK_IN_THE_BOX, SEED_NONE, 1, aGridArray, aGridArrayCount);
			ScaryPotterChangePotType(GRIDITEM_STATE_SCARY_POT_LEAF, 2);
			break;
		case GAMEMODE_SCARY_POTTER_4:
			ScaryPotterDontPlaceInCol(0, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(1, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_PUFFSHROOM, 11, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_HYPNOSHROOM, 4, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_LEFTPEATER, 4, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_JACK_IN_THE_BOX, SEED_NONE, 8, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_NORMAL, SEED_NONE, 7, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_FOOTBALL, SEED_NONE, 1, aGridArray, aGridArrayCount);
			ScaryPotterChangePotType(GRIDITEM_STATE_SCARY_POT_LEAF, 2);
			break;
		case GAMEMODE_SCARY_POTTER_5:
			ScaryPotterDontPlaceInCol(0, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(1, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_LEFTPEATER, 6, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_PUMPKINSHELL, 3, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_SQUASH, 4, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_HYPNOSHROOM, 2, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_SNOWPEA, 2, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_MAGNETSHROOM, 3, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_NORMAL, SEED_NONE, 6, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_PAIL, SEED_NONE, 5, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_JACK_IN_THE_BOX, SEED_NONE, 1, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_FOOTBALL, SEED_NONE, 3, aGridArray, aGridArrayCount);
			ScaryPotterChangePotType(GRIDITEM_STATE_SCARY_POT_LEAF, 2);
			break;
		case GAMEMODE_SCARY_POTTER_6:
			ScaryPotterDontPlaceInCol(0, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(1, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_LEFTPEATER, 7, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_SQUASH, 2, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_TALLNUT, 5, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_THREEPEATER, 2, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_TORCHWOOD, 4, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_NORMAL, SEED_NONE, 7, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_POLEVAULTER, SEED_NONE, 5, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_FOOTBALL, SEED_NONE, 2, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_JACK_IN_THE_BOX, SEED_NONE, 1, aGridArray, aGridArrayCount);
			ScaryPotterChangePotType(GRIDITEM_STATE_SCARY_POT_LEAF, 2);
			break;
		case GAMEMODE_SCARY_POTTER_7:
			ScaryPotterDontPlaceInCol(0, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(1, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(2, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_SPIKEWEED, 13, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_WALLNUT, 3, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_SQUASH, 3, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_NORMAL, SEED_NONE, 10, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_PAIL, SEED_NONE, 1, aGridArray, aGridArrayCount);
			ScaryPotterChangePotType(GRIDITEM_STATE_SCARY_POT_LEAF, 2);
			break;
		case GAMEMODE_SCARY_POTTER_8:
			ScaryPotterDontPlaceInCol(0, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(1, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_PUFFSHROOM, 7, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_WALLNUT, 3, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_SQUASH, 5, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_LEFTPEATER, 4, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_JACK_IN_THE_BOX, SEED_NONE, 8, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_NORMAL, SEED_NONE, 4, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_POGO, SEED_NONE, 4, aGridArray, aGridArrayCount);
			ScaryPotterChangePotType(GRIDITEM_STATE_SCARY_POT_LEAF, 2);
			break;
		case GAMEMODE_SCARY_POTTER_9:
			ScaryPotterDontPlaceInCol(0, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(1, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_LEFTPEATER, 6, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_SNOWPEA, 2, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_PEASHOOTER, 2, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_THREEPEATER, 2, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_SQUASH, 5, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_POTATOMINE, 1, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_WALLNUT, 1, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_PLANTERN, 1, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_NORMAL, SEED_NONE, 8, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_PAIL, SEED_NONE, 5, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_JACK_IN_THE_BOX, SEED_NONE, 1, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_GARGANTUAR, SEED_NONE, 1, aGridArray, aGridArrayCount);
			ScaryPotterChangePotType(GRIDITEM_STATE_SCARY_POT_LEAF, 2);
			break;
		case GAMEMODE_SCARY_POTTER_ENDLESS:
		{
			int aNumExtraGargantuars = ClampInt(mSurvivalStage / 10, 0, 8);
			ScaryPotterDontPlaceInCol(0, aGridArray, aGridArrayCount);
			ScaryPotterDontPlaceInCol(1, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_LEFTPEATER, 6, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_SNOWPEA, 2, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_PEASHOOTER, 1, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_THREEPEATER, 2, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_SQUASH, 5, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_POTATOMINE, 1, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_WALLNUT, 1, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SEED, ZOMBIE_INVALID, SEED_PLANTERN, 1, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_SUN, ZOMBIE_INVALID, SEED_NONE, 1, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_NORMAL, SEED_NONE, 8 - aNumExtraGargantuars, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_PAIL, SEED_NONE, 5, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_JACK_IN_THE_BOX, SEED_NONE, 1, aGridArray, aGridArrayCount);
			ScaryPotterPlacePot(SCARYPOT_ZOMBIE, ZOMBIE_GARGANTUAR, SEED_NONE, 1 + aNumExtraGargantuars, aGridArray, aGridArrayCount);
			ScaryPotterChangePotType(GRIDITEM_STATE_SCARY_POT_LEAF, 2);

			if (mSurvivalStage == 15) // @Patoke: add achievement
				ReportAchievement::GiveAchievement(mApp, ChinaShop, true);

			break;
		}
		default:
			TOD_ASSERT(false);
			break;
		}
	}

	mScaryPotterPots = ScaryPotterCountPots();
}

void Challenge::ScaryPotterStart()
{
	if (mApp->IsAdventureMode())
	{
		mBoard->DisplayAdvice("[ADVICE_USE_SHOVEL_ON_POTS]", MESSAGE_STYLE_HINT_STAY, ADVICE_USE_SHOVEL_ON_POTS);
	}
}

int Challenge::ScaryPotterIsCompleted()
{
	GridItem* aGridItem = nullptr;
	while (mBoard->IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType == GRIDITEM_SCARY_POT)
		{
			return false;
		}
	}

	return !mBoard->AreEnemyZombiesOnScreen();
}

void Challenge::ScaryPotterUpdate()
{
	if (mChallengeState == STATECHALLENGE_SCARY_POTTER_MALLETING)
	{
		Reanimation* aMalletReanim = mApp->ReanimationGet(mReanimChallenge);
		if (aMalletReanim->mLoopCount > 0)
		{
			GridItem* aScaryPot = mBoard->GetGridItemAt(GRIDITEM_SCARY_POT, mChallengeGridX, mChallengeGridY);
			if (aScaryPot)
			{
				ScaryPotterOpenPot(aScaryPot);
			}

			mChallengeGridX = 0;
			mChallengeGridY = 0;
			aMalletReanim->ReanimationDie();
			mReanimChallenge = REANIMATIONID_NULL;
			mChallengeState = STATECHALLENGE_NORMAL;
		}
	}
}

void Challenge::ScaryPotterMalletPot(GridItem* theScaryPot)
{
	mChallengeGridX = theScaryPot->mGridX;
	mChallengeGridY = theScaryPot->mGridY;
	int aXPos = mBoard->GridToPixelX(theScaryPot->mGridX, theScaryPot->mGridY);
	int aYPos = mBoard->GridToPixelY(theScaryPot->mGridX, theScaryPot->mGridY);
	Reanimation* aMalletReanim = mApp->AddReanimation(aXPos, aYPos, RENDER_LAYER_TOP, REANIM_HAMMER);
	aMalletReanim->PlayReanim("anim_pot_open", REANIM_PLAY_ONCE_AND_HOLD, 0, 40.0f);
	mReanimChallenge = mApp->ReanimationGetID(aMalletReanim);
	mChallengeState = STATECHALLENGE_SCARY_POTTER_MALLETING;
	mApp->PlayFoley(FOLEY_SWING);
}

int Challenge::ScaryPotterCountPots()
{
	int aCount = 0;
	GridItem* aGridItem = nullptr;
	while (mBoard->IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType == GRIDITEM_SCARY_POT)
		{
			aCount++;
		}
	}
	return aCount;
}

int Challenge::PuzzleIsAwardStage()
{
	if (mApp->IsAdventureMode())
		return false;

	int aGoal = mApp->mGameMode == GAMEMODE_PUZZLE_I_ZOMBIE_ENDLESS ? 3 : mApp->mGameMode == GAMEMODE_SCARY_POTTER_ENDLESS ? 10 : 1;
	return mSurvivalStage % aGoal == 0;
}

void Challenge::PuzzlePhaseComplete(int theGridX, int theGridY)
{
	if (PuzzleIsAwardStage())
	{
		CoinType aCoinType;
		int aHit = Rand(100);
		if (aHit < 15)
		{
			aCoinType = mApp->mZenGarden->CanDropPottedPlantLoot() ? COIN_AWARD_PRESENT : COIN_AWARD_MONEY_BAG;
		}
		else if (aHit < 30)
		{
			aCoinType = mApp->mZenGarden->CanDropChocolate() ? COIN_AWARD_CHOCOLATE : COIN_AWARD_MONEY_BAG;
		}
		else
		{
			aCoinType = COIN_AWARD_BAG_DIAMOND;
		}

		int aPosX = mBoard->GridToPixelX(theGridX, theGridY) + 40;
		int aPosY = mBoard->GridToPixelY(theGridX, theGridY) + 40;
		mBoard->AddCoin(aPosX, aPosY, aCoinType, COIN_MOTION_COIN);
	}
	else
	{
		mBoard->FadeOutLevel();
	}
}

int Challenge::ScaryPotterCountSunInPot(GridItem* theScaryPot)
{
	return theScaryPot->mSunCount;
}

void Challenge::ScaryPotterOpenPot(GridItem* theScaryPot)
{
	int aXPos = mBoard->GridToPixelX(theScaryPot->mGridX, theScaryPot->mGridY);
	int aYPos = mBoard->GridToPixelY(theScaryPot->mGridX, theScaryPot->mGridY);
	switch (theScaryPot->mScaryPotType)
	{
	case SCARYPOT_SEED:
		mBoard->AddCoin(aXPos + 20, aYPos, COIN_USABLE_SEED_PACKET, COIN_MOTION_FROM_PLANT)->mUsableSeedType = theScaryPot->mSeedType;
		break;
	case SCARYPOT_ZOMBIE:
		mBoard->AddZombieInRow(theScaryPot->mZombieType, theScaryPot->mGridY, 0)->mPosX = aXPos;
		break;
	case SCARYPOT_SUN:
	{
		int aSunCount = ScaryPotterCountSunInPot(theScaryPot);
		for (int i = 0; i < aSunCount; i++)
		{
			mBoard->AddCoin(aXPos + 15 * i, aYPos, COIN_SUN, COIN_MOTION_FROM_PLANT);
		}
		break;
	}
	default:
		TOD_ASSERT(false);
		break;
	}

	theScaryPot->GridItemDie();

	if (mBoard->mHelpIndex == ADVICE_USE_SHOVEL_ON_POTS)
	{
		mBoard->DisplayAdvice("[ADVICE_DESTROY_POTS_TO_FINISH_LEVEL]", MESSAGE_STYLE_HINT_FAST, ADVICE_DESTORY_POTS_TO_FINISIH_LEVEL);
	}
	if (ScaryPotterIsCompleted())
	{
		if (mApp->IsScaryPotterLevel() && !mBoard->IsFinalScaryPotterStage())
		{
			PuzzlePhaseComplete(theScaryPot->mGridX, theScaryPot->mGridY);
		}
		else
		{
			SpawnLevelAward(theScaryPot->mGridX, theScaryPot->mGridY);
		}
	}

	mApp->PlaySample(Sexy::SOUND_BONK);
	mApp->PlayFoley(FOLEY_VASE_BREAKING);
	ParticleEffect anEffect =
		theScaryPot->mGridItemState == GRIDITEM_STATE_SCARY_POT_LEAF ? PARTICLE_VASE_SHATTER_LEAF :
		theScaryPot->mGridItemState == GRIDITEM_STATE_SCARY_POT_ZOMBIE ? PARTICLE_VASE_SHATTER_ZOMBIE :
		PARTICLE_VASE_SHATTER;
	mApp->AddTodParticle(aXPos + 20, aYPos, RENDER_LAYER_GROUND, anEffect);
}

void Challenge::ScaryPotterJackExplode(int thePosX, int thePosY)
{
	int aGridX = mBoard->PixelToGridX(thePosX, thePosY);
	int aGridY = mBoard->PixelToGridY(thePosX, thePosY);

	GridItem* aGridItem = nullptr;
	while (mBoard->IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType == GRIDITEM_SCARY_POT && GridInRange(aGridItem->mGridX, aGridItem->mGridY, aGridX, aGridY, 1, 1))
		{
			ScaryPotterOpenPot(aGridItem);
		}
	}
}

// GOTY @Patoke: 0x42C9F0
void Challenge::PuzzleNextStageClear()
{
	mApp->PlaySample(Sexy::SOUND_HUGE_WAVE);
	mBoard->mNextSurvivalStageCounter = 0;
	mBoard->mProgressMeterWidth = 0;

	{
		Zombie* aZombie = nullptr;
		while (mBoard->IterateZombies(aZombie))
		{
			if (aZombie->IsOnBoard())
			{
				aZombie->DieNoLoot();
			}
		}
	}
	{
		Plant* aPlant = nullptr;
		while (mBoard->IteratePlants(aPlant))
		{
			if (aPlant->IsOnBoard())
			{
				aPlant->Die();
			}
		}
	}
	mBoard->RefreshSeedPacketFromCursor();
	{
		Coin* aCoin = nullptr;
		while (mBoard->IterateCoins(aCoin))
		{
			if (aCoin->mType == COIN_USABLE_SEED_PACKET)
			{
				aCoin->Die();
			}
			else
			{
				aCoin->TryAutoCollectAfterLevelAward();
			}
		}
	}
	{
		GridItem* aGridItem = nullptr;
		while (mBoard->IterateGridItems(aGridItem))
		{
			aGridItem->GridItemDie();
		}
	}

	mSurvivalStage++;
	mBoard->ClearAdviceImmediately();
	mBoard->mLevelAwardSpawned = false;
	mApp->AddTodParticle(BOARD_WIDTH / 2, BOARD_HEIGHT / 2, RENDER_LAYER_TOP, PARTICLE_SCREEN_FLASH);
}

ZombieType Challenge::IZombieSeedTypeToZombieType(SeedType theSeedType)
{
	switch (theSeedType)
	{
	case SEED_ZOMBIE_NORMAL:		return ZOMBIE_NORMAL;
	case SEED_ZOMBIE_TRAFFIC_CONE:	return ZOMBIE_TRAFFIC_CONE;
	case SEED_ZOMBIE_POLEVAULTER:	return ZOMBIE_POLEVAULTER;
	case SEED_ZOMBIE_PAIL:			return ZOMBIE_PAIL;
	case SEED_ZOMBIE_LADDER:		return ZOMBIE_LADDER;
	case SEED_ZOMBIE_DIGGER:		return ZOMBIE_DIGGER;
	case SEED_ZOMBIE_BUNGEE:		return ZOMBIE_BUNGEE;
	case SEED_ZOMBIE_FOOTBALL:		return ZOMBIE_FOOTBALL;
	case SEED_ZOMBIE_BALLOON:		return ZOMBIE_BALLOON;
	case SEED_ZOMBIE_SCREEN_DOOR:	return ZOMBIE_DOOR;
	case SEED_ZOMBONI:				return ZOMBIE_ZAMBONI;
	case SEED_ZOMBIE_POGO:			return ZOMBIE_POGO;
	case SEED_ZOMBIE_DANCER:		return ZOMBIE_DANCER;
	case SEED_ZOMBIE_GARGANTUAR:	return ZOMBIE_GARGANTUAR;
	case SEED_ZOMBIE_IMP:			return ZOMBIE_IMP;
	default:						TOD_ASSERT(false);
	}

	unreachable();
}

void Challenge::IZombiePlaceZombie(ZombieType theZombieType, int theGridX, int theGridY)
{
	Zombie* aZombie = mBoard->AddZombieInRow(theZombieType, theGridY, 0);
	if (theZombieType == ZOMBIE_BUNGEE)
	{
		aZombie->mTargetCol = theGridX;
		aZombie->SetRow(theGridY);
		aZombie->mPosX = mBoard->GridToPixelX(theGridX, theGridY);
		aZombie->mPosY = aZombie->GetPosYBasedOnRow(theGridY);
		aZombie->mRenderOrder = Board::MakeRenderOrder(RENDER_LAYER_GRAVE_STONE, theGridY, 7);
	}
	else
	{
		aZombie->mPosX = mBoard->GridToPixelX(theGridX, theGridY) - 30.0f;
	}
}

void Challenge::IZombieMouseDownWithZombie(int theX, int theY, int theClickCount)
{
	if (theClickCount >= 0)
	{
		SeedType aSeedType = mBoard->mCursorObject->mType;
		int aGridX = mBoard->PlantingPixelToGridX(theX, theY, aSeedType);
		int aGridY = mBoard->PlantingPixelToGridY(theX, theY, aSeedType);
		if (aGridX != -1 && aGridY != -1 && theClickCount)
		{
			if (CanPlantAt(aGridX, aGridY, aSeedType) == PLANTING_OK)
			{
				if (mApp->mEasyPlantingCheat || mBoard->TakeSunMoney(mBoard->GetCurrentPlantCost(aSeedType, SEED_NONE)))
				{
					mBoard->ClearAdvice(ADVICE_I_ZOMBIE_LEFT_OF_LINE);
					mBoard->ClearAdvice(ADVICE_I_ZOMBIE_NOT_PASSED_LINE);
					ZombieType aZombieType = IZombieSeedTypeToZombieType(aSeedType);
					IZombiePlaceZombie(aZombieType, aGridX, aGridY);

					TOD_ASSERT(mBoard->mCursorObject->mSeedBankIndex >= 0 && mBoard->mCursorObject->mSeedBankIndex < mBoard->mSeedBank->mNumPackets);
					mBoard->mSeedBank->mSeedPackets[mBoard->mCursorObject->mSeedBankIndex].WasPlanted();
					mApp->PlayFoley(FOLEY_PLANT);
					mBoard->ClearCursor();
				}
			}
			else
			{
				mBoard->ClearAdvice(ADVICE_NONE);
				if (aSeedType == SEED_ZOMBIE_BUNGEE)
				{
					mBoard->DisplayAdvice("[ADVICE_I_ZOMBIE_LEFT_OF_LINE]", MESSAGE_STYLE_HINT_LONG, ADVICE_I_ZOMBIE_LEFT_OF_LINE);
				}
				else
				{
					mBoard->DisplayAdvice("[ADVICE_I_ZOMBIE_NOT_PASSED_LINE]", MESSAGE_STYLE_HINT_LONG, ADVICE_I_ZOMBIE_NOT_PASSED_LINE);
				}
			}

			return;
		}
	}
	mBoard->RefreshSeedPacketFromCursor();
	mApp->PlayFoley(FOLEY_DROP);
}

void Challenge::IZombieSetupPlant(Plant* thePlant)
{
	Reanimation* aBodyReanim = mApp->ReanimationTryToGet(thePlant->mBodyReanimID);
	Reanimation* aHeadReanim = mApp->ReanimationTryToGet(thePlant->mHeadReanimID);
	Reanimation* aHeadReanim2 = mApp->ReanimationTryToGet(thePlant->mHeadReanimID2);
	Reanimation* aHeadReanim3 = mApp->ReanimationTryToGet(thePlant->mHeadReanimID3);
	if (aBodyReanim)	aBodyReanim->mAnimRate = 0;
	if (aHeadReanim)	aHeadReanim->mAnimRate = 0;
	if (aHeadReanim2)	aHeadReanim2->mAnimRate = 0;
	if (aHeadReanim3)	aHeadReanim3->mAnimRate = 0;

	if (thePlant->mSeedType == SEED_POTATOMINE)
	{
		thePlant->PlayBodyReanim("anim_armed", REANIM_LOOP, 0, 0);
		thePlant->mState = STATE_POTATO_ARMED;
	}

	thePlant->mBlinkCountdown = 0;
	thePlant->UpdateReanim();
}

// GOTY @Patoke: 0x42D250
void Challenge::IZombiePlacePlantInSquare(SeedType theSeedType, int theGridX, int theGridY)
{
	if (mBoard->CanPlantAt(theGridX, theGridY, theSeedType) == PLANTING_OK)
	{
		IZombieSetupPlant(mBoard->NewPlant(theGridX, theGridY, theSeedType));
	}
}

// GOTY @Patoke: 0x42D2B0
void Challenge::IZombiePlacePlants(SeedType theSeedType, int theCount, int theGridY)
{
	// 关卡中植物（或限制红线）的列数
	int aColumns = 6;
	GameMode aGameMode = mApp->mGameMode;
	if (aGameMode >= GAMEMODE_PUZZLE_I_ZOMBIE_1 && aGameMode <= GAMEMODE_PUZZLE_I_ZOMBIE_5)
	{
		aColumns = 4;
	}
	else if (aGameMode != GAMEMODE_PUZZLE_I_ZOMBIE_9)
	{
		aColumns = 5;
	}

	// 只能生成在 aMinGridY 行与 aMaxGridY 行之间
	int aMinGridY, aMaxGridY;
	if (theGridY == -1)  // 参数 theGridY 使用默认的 -1 时，不限定生成范围
	{
		aMinGridY = 0;  // 从第 0 行开始
		aMaxGridY = 4;  // 至第 4 行结束
	}
	else
	{
		aMinGridY = theGridY;
		aMaxGridY = theGridY;  // 可种植范围从 theGridY 行至 theGridY 行，表示限定种植的行只能为 theGridY
	}

	int aGridArrayCount = 0;
	TodWeightedGridArray aGridArray[MAX_GRID_SIZE_X * MAX_GRID_SIZE_Y];
	// 将从 theGridY 行至 aMaxGridY 行的选择数据写入格子数组中
	for (int aRow = aMinGridY; aRow <= aMaxGridY; aRow++)
	{
		// 对于每一行，写入红线左侧每一列的选择数据
		for (int aCol = 0; aCol < aColumns; aCol++)
		{
			// 当且仅当格子内可以种植目标植物时
			if (mBoard->CanPlantAt(aCol, aRow, theSeedType) == PLANTING_OK)
			{
				// 坚果和火炬树桩只会出现在靠右侧的 3 列
				if ((theSeedType != SEED_WALLNUT && theSeedType != SEED_TORCHWOOD) || aColumns - aCol <= 3)
				{
					aGridArray[aGridArrayCount].mX = aCol;
					aGridArray[aGridArrayCount].mY = aRow;
					aGridArray[aGridArrayCount].mWeight = 1;
					aGridArrayCount++;
				}
			}
		}
	}

	theCount = std::min(theCount, aGridArrayCount);

	// 依次选择 theCount 个格子并在这些格子中放置植物
	for (int i = 0; i < theCount; i++)
	{
		TodWeightedGridArray* aGrid = TodPickFromWeightedGridArray(aGridArray, aGridArrayCount);
		IZombiePlacePlantInSquare(theSeedType, aGrid->mX, aGrid->mY);
		aGrid->mWeight = 0;
	}
}

// GOTY @Patoke: 0x42D440
void Challenge::IZombieInitLevel()
{
	mChallengeScore = 0;
	for (int aRow = 0; aRow < I_ZOMBIE_WINNING_SCORE; aRow++)
	{
		GridItem* aBrain = mBoard->mGridItems.DataArrayAlloc();
		aBrain->mGridItemType = GRIDITEM_IZOMBIE_BRAIN;
		aBrain->mGridX = 0;
		aBrain->mGridY = aRow;
		aBrain->mRenderOrder = Board::MakeRenderOrder(RENDER_LAYER_PLANT, aRow, 0);
		aBrain->mGridItemCounter = 70;
		aBrain->mPosX = mBoard->GridToPixelX(0, aRow) - 40;
		aBrain->mPosY = mBoard->GridToPixelY(0, aRow) + 40;
	}

	switch (mApp->mGameMode)
	{
	case GAMEMODE_PUZZLE_I_ZOMBIE_1:
		IZombiePlacePlantInSquare(SEED_SUNFLOWER, 3, 2);
		IZombiePlacePlantInSquare(SEED_SUNFLOWER, 3, 3);
		IZombiePlacePlants(SEED_SUNFLOWER, 7, -1);
		IZombiePlacePlants(SEED_SQUASH, 3, -1);
		IZombiePlacePlants(SEED_PEASHOOTER, 6, -1);
		IZombiePlacePlants(SEED_SNOWPEA, 2, -1);
		break;
	case GAMEMODE_PUZZLE_I_ZOMBIE_2:
		IZombiePlacePlantInSquare(SEED_SPIKEWEED, 3, 0);
		IZombiePlacePlantInSquare(SEED_SUNFLOWER, 2, 0);
		IZombiePlacePlantInSquare(SEED_SUNFLOWER, 3, 3);
		IZombiePlacePlants(SEED_SPIKEWEED, 1, 0);
		IZombiePlacePlants(SEED_PEASHOOTER, 1, 0);
		IZombiePlacePlants(SEED_SNOWPEA, 2, 3);
		IZombiePlacePlants(SEED_SUNFLOWER, 1, 3);
		IZombiePlacePlants(SEED_SUNFLOWER, 4, -1);
		IZombiePlacePlants(SEED_SPIKEWEED, 2, -1);
		IZombiePlacePlants(SEED_SNOWPEA, 2, -1);
		IZombiePlacePlants(SEED_PEASHOOTER, 4, -1);
		break;
	case GAMEMODE_PUZZLE_I_ZOMBIE_3:
		IZombiePlacePlantInSquare(SEED_POTATOMINE, 3, 0);
		IZombiePlacePlantInSquare(SEED_SUNFLOWER, 2, 0);
		IZombiePlacePlantInSquare(SEED_POTATOMINE, 2, 2);
		IZombiePlacePlantInSquare(SEED_SUNFLOWER, 2, 4);
		IZombiePlacePlantInSquare(SEED_TORCHWOOD, 3, 3);
		IZombiePlacePlants(SEED_TORCHWOOD, 2, -1);
		IZombiePlacePlants(SEED_SUNFLOWER, 5, -1);
		IZombiePlacePlants(SEED_PEASHOOTER, 7, -1);
		IZombiePlacePlants(SEED_SPLITPEA, 1, -1);
		break;
	case GAMEMODE_PUZZLE_I_ZOMBIE_4:
		IZombiePlacePlantInSquare(SEED_WALLNUT, 3, 0);
		IZombiePlacePlantInSquare(SEED_SUNFLOWER, 2, 0);
		IZombiePlacePlantInSquare(SEED_WALLNUT, 3, 1);
		IZombiePlacePlantInSquare(SEED_WALLNUT, 3, 2);
		IZombiePlacePlantInSquare(SEED_SUNFLOWER, 2, 2);
		IZombiePlacePlantInSquare(SEED_WALLNUT, 3, 3);
		IZombiePlacePlantInSquare(SEED_WALLNUT, 3, 4);
		IZombiePlacePlantInSquare(SEED_SUNFLOWER, 2, 4);
		IZombiePlacePlants(SEED_PEASHOOTER, 1, 0);
		IZombiePlacePlants(SEED_SNOWPEA, 1, 1);
		IZombiePlacePlants(SEED_FUMESHROOM, 2, 2);
		IZombiePlacePlants(SEED_SNOWPEA, 1, 3);
		IZombiePlacePlants(SEED_PEASHOOTER, 1, 4);
		IZombiePlacePlants(SEED_PEASHOOTER, 2, -1);
		IZombiePlacePlants(SEED_SUNFLOWER, 4, -1);
		break;
	case GAMEMODE_PUZZLE_I_ZOMBIE_5:
		IZombiePlacePlantInSquare(SEED_SUNFLOWER, 3, 2);
		IZombiePlacePlantInSquare(SEED_SUNFLOWER, 3, 3);
		IZombiePlacePlants(SEED_CACTUS, 1, 1);
		IZombiePlacePlants(SEED_CACTUS, 1, 4);
		IZombiePlacePlants(SEED_MAGNETSHROOM, 1, -1);
		IZombiePlacePlants(SEED_SUNFLOWER, 5, -1);
		IZombiePlacePlants(SEED_PEASHOOTER, 8, -1);
		IZombiePlacePlants(SEED_SNOWPEA, 2, -1);
		break;
	case GAMEMODE_PUZZLE_I_ZOMBIE_6:
		IZombiePlacePlantInSquare(SEED_GARLIC, 4, 1);
		IZombiePlacePlantInSquare(SEED_GARLIC, 4, 3);
		IZombiePlacePlants(SEED_SUNFLOWER, 3, 1);
		IZombiePlacePlants(SEED_SUNFLOWER, 3, 3);
		IZombiePlacePlants(SEED_TORCHWOOD, 2, -1);
		IZombiePlacePlants(SEED_SUNFLOWER, 2, -1);
		IZombiePlacePlants(SEED_SPIKEWEED, 3, -1);
		IZombiePlacePlants(SEED_SNOWPEA, 1, -1);
		IZombiePlacePlants(SEED_PEASHOOTER, 5, -1);
		IZombiePlacePlants(SEED_SQUASH, 2, -1);
		IZombiePlacePlants(SEED_KERNELPULT, 2, -1);
		break;
	case GAMEMODE_PUZZLE_I_ZOMBIE_7:
		IZombiePlacePlantInSquare(SEED_SUNFLOWER, 4, 2);
		IZombiePlacePlantInSquare(SEED_SUNFLOWER, 4, 4);
		IZombiePlacePlants(SEED_SUNFLOWER, 6, -1);
		IZombiePlacePlants(SEED_POTATOMINE, 9, -1);
		IZombiePlacePlants(SEED_CHOMPER, 8, -1);
		break;
	case GAMEMODE_PUZZLE_I_ZOMBIE_8:
		IZombiePlacePlants(SEED_WALLNUT, 3, -1);
		IZombiePlacePlants(SEED_MAGNETSHROOM, 2, -1);
		IZombiePlacePlants(SEED_PEASHOOTER, 8, -1);
		IZombiePlacePlants(SEED_SQUASH, 2, -1);
		IZombiePlacePlants(SEED_POTATOMINE, 2, -1);
		IZombiePlacePlants(SEED_SUNFLOWER, 8, -1);
		break;
	case GAMEMODE_PUZZLE_I_ZOMBIE_9:
		IZombiePlacePlantInSquare(SEED_TALLNUT, 5, 1);
		IZombiePlacePlantInSquare(SEED_TORCHWOOD, 5, 3);
		IZombiePlacePlants(SEED_POTATOMINE, 4, 0);
		IZombiePlacePlants(SEED_SUNFLOWER, 2, 0);
		IZombiePlacePlants(SEED_SUNFLOWER, 2, 1);
		IZombiePlacePlants(SEED_THREEPEATER, 1, 1);
		IZombiePlacePlants(SEED_SNOWPEA, 1, 1);
		IZombiePlacePlants(SEED_SPLITPEA, 1, 1);
		IZombiePlacePlants(SEED_CHOMPER, 3, 2);
		IZombiePlacePlants(SEED_SUNFLOWER, 2, 2);
		IZombiePlacePlants(SEED_SQUASH, 1, 2);
		IZombiePlacePlants(SEED_PEASHOOTER, 3, 3);
		IZombiePlacePlants(SEED_SUNFLOWER, 2, 3);
		IZombiePlacePlants(SEED_SUNFLOWER, 1, 4);
		IZombiePlacePlants(SEED_FUMESHROOM, 1, 4);
		IZombiePlacePlants(SEED_SCAREDYSHROOM, 1, 4);
		IZombiePlacePlants(SEED_STARFRUIT, 1, 4);
		IZombiePlacePlants(SEED_SPLITPEA, 1, 4);
		IZombiePlacePlants(SEED_MAGNETSHROOM, 1, 4);
		break;
	case GAMEMODE_PUZZLE_I_ZOMBIE_ENDLESS:
	{
		if (mSurvivalStage == 10) // @Patoke: add achievement
			ReportAchievement::GiveAchievement(mApp, BetterOffDead, true);

		int aFormationHit = RandRangeInt(0, 4);

		int aPuffshroomCount = RandRangeInt(ClampInt(2 + mSurvivalStage / 3, 2, 4), ClampInt(3 + mSurvivalStage / 2, 2, 6));
		if (mSurvivalStage == 0)
		{
			aPuffshroomCount = 0;
		}
		else if (mSurvivalStage == 1)
		{
			aPuffshroomCount = 1;
		}
		else if (mSurvivalStage >= 10)
		{
			aPuffshroomCount = RandRangeInt(ClampInt(2 + mSurvivalStage / 3, 2, 5), ClampInt(3 + mSurvivalStage / 2, 2, 7));
		}
		int aSunflowerCount = 8 - aPuffshroomCount;
		IZombiePlacePlants(SEED_SUNFLOWER, aSunflowerCount, -1);
		IZombiePlacePlants(SEED_PUFFSHROOM, aPuffshroomCount, -1);

		if (aFormationHit == 0 && mSurvivalStage >= 1)
		{
			switch (RandRangeInt(0, 4))
			{
			case 0:  // 输出阵
				IZombiePlacePlants(SEED_SNOWPEA, 9, -1);
				IZombiePlacePlants(SEED_SPLITPEA, 4, -1);
				IZombiePlacePlants(SEED_REPEATER, 4, -1);
				break;
			case 1:  // 爆炸阵
				IZombiePlacePlants(SEED_POTATOMINE, 9, -1);
				IZombiePlacePlants(SEED_CHOMPER, 8, -1);
				break;
			case 2:  // 倾斜阵
				IZombiePlacePlants(SEED_SPIKEWEED, 9, -1);
				IZombiePlacePlants(SEED_STARFRUIT, 8, -1);
				break;
			case 3:  // 穿刺阵
				IZombiePlacePlants(SEED_FUMESHROOM, 9, -1);
				IZombiePlacePlants(SEED_MAGNETSHROOM, 8, -1);
				break;
			default:  // 回复阵
				IZombiePlacePlants(SEED_SCAREDYSHROOM, 12, -1);
				IZombiePlacePlants(SEED_SUNFLOWER, 5, -1);
			}
		}
		else
		{
			switch (RandRangeInt(0, 5))
			{
			case 0:
			case 1:
			case 2:  // 综合阵
				IZombiePlacePlants(SEED_WALLNUT, 1, -1);
				IZombiePlacePlants(SEED_TORCHWOOD, 1, -1);
				IZombiePlacePlants(SEED_POTATOMINE, 1, -1);
				IZombiePlacePlants(SEED_CHOMPER, 2, -1);
				IZombiePlacePlants(SEED_PEASHOOTER, 1, -1);
				IZombiePlacePlants(SEED_SPLITPEA, 1, -1);
				IZombiePlacePlants(SEED_KERNELPULT, 1, -1);
				IZombiePlacePlants(SEED_THREEPEATER, 1, -1);
				IZombiePlacePlants(SEED_SNOWPEA, 1, -1);
				IZombiePlacePlants(SEED_SQUASH, 1, -1);
				IZombiePlacePlants(SEED_FUMESHROOM, 1, -1);
				IZombiePlacePlants(SEED_UMBRELLA, 1, -1);
				IZombiePlacePlants(SEED_STARFRUIT, 1, -1);
				IZombiePlacePlants(SEED_MAGNETSHROOM, 1, -1);
				IZombiePlacePlants(SEED_SPIKEWEED, 2, -1);
				break;
			case 3:
			case 4:  // 控制阵
				IZombiePlacePlants(SEED_TORCHWOOD, 1, -1);
				IZombiePlacePlants(SEED_SPLITPEA, 3, -1);
				IZombiePlacePlants(SEED_REPEATER, 1, -1);
				IZombiePlacePlants(SEED_KERNELPULT, 3, -1);
				IZombiePlacePlants(SEED_THREEPEATER, 1, -1);
				IZombiePlacePlants(SEED_SNOWPEA, 3, -1);
				IZombiePlacePlants(SEED_UMBRELLA, 1, -1);
				IZombiePlacePlants(SEED_MAGNETSHROOM, 1, -1);
				IZombiePlacePlants(SEED_SPIKEWEED, 3, -1);
				break;
			default:  // 即死阵
				IZombiePlacePlants(SEED_POTATOMINE, 4, -1);
				IZombiePlacePlants(SEED_CHOMPER, 3, -1);
				IZombiePlacePlants(SEED_SQUASH, 3, -1);
				IZombiePlacePlants(SEED_FUMESHROOM, 4, -1);
				IZombiePlacePlants(SEED_SPIKEWEED, 3, -1);
				break;
			}
		}
		break;
	}
	default:
		TOD_ASSERT(false);
	}

	mBoard->mBonusLawnMowersRemaining = 0;
}

void Challenge::IZombieStart()
{
	mBoard->DisplayAdvice("[ADVICE_I_ZOMBIE_EAT_ALL_BRAINS]", MESSAGE_STYLE_HINT_FAST, ADVICE_I_ZOMBIE_EAT_ALL_BRAINS);
}

void Challenge::IZombieUpdate()
{
	int aSunMoney = mBoard->mSunMoney + mBoard->CountSunBeingCollected();

	Zombie* aZombie = nullptr;
	while (mBoard->IterateZombies(aZombie))
	{
		if (!aZombie->IsDeadOrDying() && aZombie->mZombiePhase != PHASE_POLEVAULTER_IN_VAULT && !aZombie->mIsEating && aZombie->mJustGotShotCounter < -500)
		{
			aZombie->PickRandomSpeed();
		}
	}
	
	int anActive = false;
	Plant* aPlant = nullptr;
	while (mBoard->IteratePlants(aPlant))
	{
		PlantState aState = aPlant->mState;
		if (aState == STATE_SQUASH_FALLING || aState == STATE_SQUASH_DONE_FALLING || aState == STATE_CHOMPER_BITING || aState == STATE_CHOMPER_BITING_GOT_ONE)
		{
			anActive = true;
		}
	}
	TodParticleSystem* aParticle = nullptr;
	while (mBoard->IterateParticles(aParticle))
	{
		if (aParticle->mEffectType == PARTICLE_POTATO_MINE)
		{
			anActive = true;
		}
	}

	bool aHasAvailableSun = false;
	Coin* aCoinForSun = nullptr;
	while (mBoard->IterateCoins(aCoinForSun))
	{
		if (aCoinForSun->IsSun() && !aCoinForSun->mIsBeingCollected && !aCoinForSun->mDead)
		{
			aHasAvailableSun = true;
			break;
		}
	}

	if (mBoard->mZombies.mSize == 0 && aSunMoney < 50 && !mBoard->HasLevelAwardDropped() && !anActive && !aHasAvailableSun)
	{
		Coin* aCoin = nullptr;
		while (mBoard->IterateCoins(aCoin))
		{
			if (aCoin->IsMoney())
			{
				aCoin->Die();
			}
		}

		mBoard->ZombiesWon(nullptr);
	}
}

int Challenge::IsZombieSeedType(SeedType theSeedType)
{
	return 
		theSeedType == SEED_ZOMBIQUARIUM_SNORKLE ||
		theSeedType == SEED_ZOMBIQUARIUM_TROPHY ||
		theSeedType == SEED_ZOMBIE_NORMAL ||
		theSeedType == SEED_ZOMBIE_TRAFFIC_CONE ||
		theSeedType == SEED_ZOMBIE_POLEVAULTER ||
		theSeedType == SEED_ZOMBIE_PAIL ||
		theSeedType == SEED_ZOMBIE_LADDER ||
		theSeedType == SEED_ZOMBIE_DIGGER ||
		theSeedType == SEED_ZOMBIE_BUNGEE ||
		theSeedType == SEED_ZOMBIE_FOOTBALL ||
		theSeedType == SEED_ZOMBIE_BALLOON ||
		theSeedType == SEED_ZOMBIE_SCREEN_DOOR ||
		theSeedType == SEED_ZOMBONI ||
		theSeedType == SEED_ZOMBIE_POGO ||
		theSeedType == SEED_ZOMBIE_DANCER ||
		theSeedType == SEED_ZOMBIE_GARGANTUAR ||
		theSeedType == SEED_ZOMBIE_IMP;
}

// GOTY @Patoke: 0x42E280
void Challenge::IZombieSetPlantFilterEffect(Plant* thePlant, FilterEffect theFilterEffect)
{
	Reanimation* aBodyReanim = mApp->ReanimationTryToGet(thePlant->mBodyReanimID);
	Reanimation* aHeadReanim = mApp->ReanimationTryToGet(thePlant->mHeadReanimID);
	Reanimation* aHeadReanim2 = mApp->ReanimationTryToGet(thePlant->mHeadReanimID2);
	Reanimation* aHeadReanim3 = mApp->ReanimationTryToGet(thePlant->mHeadReanimID3);
	if (aBodyReanim)	aBodyReanim->mFilterEffect = theFilterEffect;
	if (aHeadReanim)	aHeadReanim->mFilterEffect = theFilterEffect;
	if (aHeadReanim2)	aHeadReanim2->mFilterEffect = theFilterEffect;
	if (aHeadReanim3)	aHeadReanim3->mFilterEffect = theFilterEffect;
}

// GOTY @Patoke: 0x42E3D4
void Challenge::IZombieDrawPlant(Graphics* g, Plant* thePlant)
{
	Reanimation* aReanim = mApp->ReanimationTryToGet(thePlant->mBodyReanimID);
	if (aReanim)
	{
		IZombieSetPlantFilterEffect(thePlant, FILTER_EFFECT_WHITE);
		g->SetColorizeImages(true);

		float aOffsetX = g->mTransX, aOffsetY = g->mTransY;
		g->mTransX = aOffsetX + 4;
		g->mTransY = aOffsetY + 4;
		g->SetColor(Color(122, 86, 58));
		aReanim->DrawRenderGroup(g, 0);

		g->mTransX = aOffsetX + 2;
		g->mTransY = aOffsetY + 2;
		g->SetColor(Color(171, 135, 107));
		aReanim->DrawRenderGroup(g, 0);

		g->mTransX = aOffsetX - 2;
		g->mTransY = aOffsetY - 2;
		g->SetColor(Color(171, 135, 107));
		aReanim->DrawRenderGroup(g, 0);

		g->mTransX = aOffsetX;
		g->mTransY = aOffsetY;
		g->SetColor(Color(255, 201, 160));
		IZombieSetPlantFilterEffect(thePlant, FILTER_EFFECT_NONE);
		aReanim->DrawRenderGroup(g, 0);

		IZombieSetPlantFilterEffect(thePlant, FILTER_EFFECT_NONE);
		g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
		g->SetColorizeImages(false);
	}
}

GridItem* Challenge::IZombieGetBrainTarget(Zombie* theZombie)
{
	if (theZombie->mZombieType == ZOMBIE_BUNGEE || theZombie->IsWalkingBackwards())
		return nullptr;

	Rect aZombieRect = theZombie->GetZombieAttackRect();
	if (theZombie->mZombiePhase == PHASE_POLEVAULTER_PRE_VAULT)
	{
		aZombieRect = Rect(50 + theZombie->mX, 0, 20, 115);
	}
	if (theZombie->mZombieType == ZOMBIE_BALLOON)
	{
		aZombieRect.mX += 25;
	}

	if (aZombieRect.mX > 20)
		return nullptr;

	GridItem* aBrain = mBoard->GetGridItemAt(GRIDITEM_IZOMBIE_BRAIN, 0, theZombie->mRow);
	return (aBrain && aBrain->mGridItemState != GRIDITEM_STATE_BRAIN_SQUISHED) ? aBrain : nullptr;
}

void Challenge::IZombieScoreBrain(GridItem* theBrain)
{
	mBoard->mChallenge->mChallengeScore++;
	mBoard->mProgressMeterWidth = TodAnimateCurve(0, I_ZOMBIE_WINNING_SCORE, mBoard->mChallenge->mChallengeScore, 0, PROGRESS_METER_COUNTER, CURVE_LINEAR);

	if (mBoard->mChallenge->mChallengeScore == I_ZOMBIE_WINNING_SCORE)
	{
		if (mApp->IsEndlessIZombie(mApp->mGameMode))
		{
			PuzzlePhaseComplete(0, theBrain->mGridY);
		}
		else
		{
			mBoard->mChallenge->SpawnLevelAward(0, theBrain->mGridY);
		}
	}

	if (mBoard->mChallenge->mChallengeScore != I_ZOMBIE_WINNING_SCORE || !PuzzleIsAwardStage())
	{
		mBoard->DropLootPiece(theBrain->mPosX + 40, theBrain->mPosY - 50, 12);
	}
}

int Challenge::IZombieEatBrain(Zombie* theZombie)
{
	GridItem* aBrain = IZombieGetBrainTarget(theZombie);
	if (aBrain == nullptr)
		return false;

	theZombie->StartEating();
	aBrain->mGridItemCounter--;
	if (aBrain->mGridItemCounter <= 0)
	{
		mApp->PlaySample(Sexy::SOUND_GULP);
		aBrain->GridItemDie();
		IZombieScoreBrain(aBrain);
	}
	return true;
}

void Challenge::IZombiePlantDropRemainingSun(Plant* thePlant)
{
	if (thePlant->mSeedType == SEED_SUNFLOWER)
	{
		int aSunCount = thePlant->mPlantHealth / 40 + 1;
		for (int i = 0; i < aSunCount; i++)
		{
			mBoard->AddCoin(thePlant->mX + 5 * i, thePlant->mY, COIN_SUN, COIN_MOTION_FROM_PLANT);
		}
	}
}

void Challenge::IZombieSquishBrain(GridItem* theBrain)
{
	theBrain->mRenderOrder = Board::MakeRenderOrder(RENDER_LAYER_GRAVE_STONE, theBrain->mGridY, 0);
	theBrain->mGridItemState = GRIDITEM_STATE_BRAIN_SQUISHED;
	theBrain->mGridItemCounter = 500;
	theBrain->mApp->PlayFoley(FOLEY_SQUISH);
	IZombieScoreBrain(theBrain);
}

/*
int Challenge::SquirrelCountUncaught()
{
	int aCount = 0;

	GridItem* aGridItem = nullptr;
	while (mBoard->IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType == GRIDITEM_SQUIRREL &&
			aGridItem->mGridItemState != GRIDITEM_STATE_SQUIRREL_CAUGHT &&
			aGridItem->mGridItemState != GRIDITEM_STATE_SQUIRREL_ZOMBIE)
		{
			aCount++;
		}
	}

	return aCount;
}

void Challenge::SquirrelStart()
{
	int aPicksCount = 0;
	TodWeightedGridArray aPicks[MAX_GRID_SIZE_X * (MAX_GRID_SIZE_Y - 1)];
	for (int aCol = 0; aCol < MAX_GRID_SIZE_X; aCol++)
	{
		for (int aRow = 0; aRow < MAX_GRID_SIZE_Y - 1; aRow++)
		{
			aPicks[aPicksCount].mX = aCol;
			aPicks[aPicksCount].mY = aRow;
			aPicks[aPicksCount].mWeight = 1;
			aPicksCount++;
		}
	}
	
	for (int aCount = MAX_SQUIRRELS; aCount > 0; aCount--)
	{
		TodWeightedGridArray* aGrid = TodPickFromWeightedGridArray(aPicks, 45);
		aGrid->mWeight = 0;

		GridItem* aSquirrel = mBoard->mGridItems.DataArrayAlloc();
		aSquirrel->mGridItemType = GRIDITEM_SQUIRREL;
		aSquirrel->mGridItemState = GRIDITEM_STATE_SQUIRREL_WAITING;
		aSquirrel->mGridX = aGrid->mX;
		aSquirrel->mGridY = aGrid->mY;
		aSquirrel->mGridItemCounter = RandRangeInt(100, 400);
		aSquirrel->mRenderOrder = mBoard->MakeRenderOrder(RENDER_LAYER_GRAVE_STONE, aSquirrel->mGridY, 1);
	}

	for (int i = 0; i < aPicksCount; i++)
	{
		if (aPicks[i].mX < 4)
		{
			aPicks[i].mWeight = 0;
		}
	}

	TodWeightedGridArray* aGrid = TodPickFromWeightedGridArray(aPicks, 45);
	GridItem* aSquirrel = mBoard->mGridItems.DataArrayAlloc();
	aSquirrel->mGridItemType = GRIDITEM_SQUIRREL;
	aSquirrel->mGridItemState = GRIDITEM_STATE_SQUIRREL_ZOMBIE;
	aSquirrel->mGridX = aGrid->mX;
	aSquirrel->mGridY = aGrid->mY;
	aSquirrel->mRenderOrder = Board::MakeRenderOrder(RENDER_LAYER_GRAVE_STONE, aGrid->mY, 1);
}

void Challenge::SquirrelChew(GridItem* theSquirrel)
{
	theSquirrel->mGridItemCounter = RandRangeInt(100, 400);

	Plant* aPlant = mBoard->GetTopPlantAt(theSquirrel->mGridX, theSquirrel->mGridY, TOPPLANT_EATING_ORDER);
	if (aPlant)
	{
		float aPosX = mBoard->GridToPixelX(theSquirrel->mGridX, theSquirrel->mGridY);
		float aPosY = mBoard->GridToPixelY(theSquirrel->mGridX, theSquirrel->mGridY);
		mApp->AddTodParticle(aPosX + 40, aPosY + 40, aPlant->mRenderOrder + 1, PARTICLE_WALLNUT_EAT_SMALL);

		if (aPlant->mEatenFlashCountdown <= 25)
			aPlant->mEatenFlashCountdown = 25;
	}
}

void Challenge::SquirrelPeek(GridItem* theSquirrel)
{
	theSquirrel->mGridItemCounter = 50;
	theSquirrel->mGridItemState = GRIDITEM_STATE_SQUIRREL_PEEKING;
}

void Challenge::SquirrelFound(GridItem* theSquirrel)
{
	if (theSquirrel->mGridItemState == GRIDITEM_STATE_SQUIRREL_ZOMBIE)
	{
		Zombie* aZombie = mBoard->AddZombieInRow(ZOMBIE_NORMAL, theSquirrel->mGridY, 0);
		aZombie->mPosX = mBoard->GridToPixelX(theSquirrel->mGridX, theSquirrel->mGridY);
		theSquirrel->GridItemDie();
		mBoard->DisplayAdvice("[ADVICE_SQUIRREL_ZOMBIE]", MESSAGE_STYLE_HINT_FAST, ADVICE_NONE);
	}
	else
	{
		TodWeightedGridArray aPicks[4];
		int aPickCount = 0;
		for (int i = 0; i < 4; i++)
		{
			int aGridX = theSquirrel->mGridX;
			int aGridY = theSquirrel->mGridY;
			switch (i)
			{
			case 0: aGridX--; break;
			case 1: aGridX++; break;
			case 2: aGridY--; break;
			case 3: aGridY++; break;
			}

			if (!mBoard->GetGridItemAt(GRIDITEM_SQUIRREL, aGridX, aGridY) && mBoard->GetTopPlantAt(aGridX, aGridY, TOPPLANT_EATING_ORDER))
			{
				aPicks[aPickCount].mX = aGridX;
				aPicks[aPickCount].mY = aGridY;
				aPicks[aPickCount].mWeight = 1;
				aPickCount++;
			}
		}
		
		if (aPickCount > 0)
		{
			TodWeightedGridArray* aGrid = TodPickFromWeightedGridArray(aPicks, aPickCount);

			theSquirrel->mGridItemState =
				aGrid->mX < theSquirrel->mGridX ? GRIDITEM_STATE_SQUIRREL_RUNNING_LEFT :
				aGrid->mX >= theSquirrel->mGridX ? GRIDITEM_STATE_SQUIRREL_RUNNING_RIGHT :
				aGrid->mY < theSquirrel->mGridY ? GRIDITEM_STATE_SQUIRREL_RUNNING_UP : GRIDITEM_STATE_SQUIRREL_RUNNING_DOWN;

			theSquirrel->mGridX = aGrid->mX;
			theSquirrel->mGridY = aGrid->mY;
			theSquirrel->mRenderOrder = Board::MakeRenderOrder(RENDER_LAYER_GRAVE_STONE, aGrid->mY, 1);
		}
		else
		{
			theSquirrel->mGridItemState = GRIDITEM_STATE_SQUIRREL_CAUGHT;
			theSquirrel->mGridItemCounter = 100;

			int aSquirrelsRemaining = SquirrelCountUncaught();
			if (aSquirrelsRemaining)
			{
				std::string aMessage = mApp->Pluralize(aSquirrelsRemaining, "[ADVICE_SQUIRRELS_ONE_LEFT]", "[ADVICE_SQUIRRELS_LEFT]");
				mBoard->DisplayAdvice(aMessage, MESSAGE_STYLE_HINT_FAST, ADVICE_NONE);
			}
			else
			{
				mBoard->ClearAdvice(ADVICE_NONE);
				SpawnLevelAward(theSquirrel->mGridX, theSquirrel->mGridY);
			}
		}
	}
}

void Challenge::SquirrelUpdateOne(GridItem* theSquirrel)
{
	int& aCounter = theSquirrel->mGridItemCounter;
	if (aCounter > 0)
		aCounter--;

	GridItemState& aState = theSquirrel->mGridItemState;
	if (aState == GRIDITEM_STATE_SQUIRREL_WAITING || aState == GRIDITEM_STATE_SQUIRREL_ZOMBIE)
	{
		if (!mBoard->GetTopPlantAt(theSquirrel->mGridX, theSquirrel->mGridY, TOPPLANT_EATING_ORDER))
		{
			SquirrelFound(theSquirrel);
		}
		if (aCounter == 0)
		{
			if (Rand(2) && aState != GRIDITEM_STATE_SQUIRREL_ZOMBIE)
			{
				SquirrelPeek(theSquirrel);
			}
			else
			{
				SquirrelChew(theSquirrel);
			}
		}
	}

	if (aState >= GRIDITEM_STATE_SQUIRREL_PEEKING && aState <= GRIDITEM_STATE_SQUIRREL_RUNNING_RIGHT && aCounter == 0)
	{
		aState = GRIDITEM_STATE_SQUIRREL_WAITING;
		aCounter = RandRangeInt(100, 400);
	}
	if (aState == GRIDITEM_STATE_SQUIRREL_CAUGHT && aCounter == 0)
	{
		theSquirrel->GridItemDie();
	}
}

void Challenge::SquirrelUpdate()
{
	GridItem* aGridItem;
	while (mBoard->IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType == GRIDITEM_SQUIRREL)
		{
			SquirrelUpdateOne(aGridItem);
		}
	}

	mChallengeScore = MAX_SQUIRRELS - SquirrelCountUncaught();
	mBoard->mProgressMeterWidth = TodAnimateCurve(0, MAX_SQUIRRELS, mChallengeScore, 0, PROGRESS_METER_COUNTER, CURVE_LINEAR);
}
*/

void Challenge::UpdateRain()
{
	mRainCounter--;
	if (mRainCounter < 0 && !mBoard->mCutScene->IsBeforePreloading())
	{
		float aPosX = RandRangeFloat(40.0f, 740.0f);
		float aPosY = RandRangeFloat(90.0f, 240.0f);
		Reanimation* aSplashReanim = mApp->AddReanimation(aPosX, aPosY, RENDER_LAYER_GROUND, REANIM_RAIN_SPLASH);
		aSplashReanim->mColorOverride = Color(255, 255, 255, RandRangeInt(100, 200));
		float aScale = RandRangeFloat(0.7f, 1.2f);
		aSplashReanim->OverrideScale(aScale, aScale);

		aPosX = RandRangeFloat(40.0f, 740.0f);
		aPosY = RandRangeFloat(290.0f, 410.0f);
		Reanimation* aCircleReanim = mApp->AddReanimation(aPosX, aPosY, RENDER_LAYER_GROUND, REANIM_RAIN_CIRCLE);
		(void)aCircleReanim; // unused
		aSplashReanim->mColorOverride = Color(255, 255, 255, RandRangeInt(50, 150));
		aScale = RandRangeFloat(0.7f, 1.1f);
		aSplashReanim->OverrideScale(aScale, aScale);

		aPosX = RandRangeFloat(40.0f, 740.0f);
		aPosY = RandRangeFloat(450.0f, 580.0f);
		Reanimation* aRainReanim = mApp->AddReanimation(aPosX, aPosY, RENDER_LAYER_GROUND, REANIM_RAIN_SPLASH);
		(void)aRainReanim; // unused
		aSplashReanim->mColorOverride = Color(255, 255, 255, RandRangeInt(100, 200));
		aScale = RandRangeFloat(0.7f, 1.2f);
		aSplashReanim->OverrideScale(aScale, aScale);
		
        mRainCounter = RandRangeInt(10, 20);
	}
}

void Challenge::LastStandUpdate()
{
	if (mBoard->mNextSurvivalStageCounter == 0 && mChallengeState == STATECHALLENGE_NORMAL && mBoard->mStoreButton->mBtnNoDraw)
	{
		GameButton* aButton = mBoard->mStoreButton;
		aButton->mBtnNoDraw = false;
		aButton->mDisabled = false;

		if (mSurvivalStage == 0)
		{
			aButton->SetLabel("[START_ONSLAUGHT]");
			aButton->Resize(300, 559, 210, 46);
		}
		else
		{
			aButton->SetLabel("[CONTINUE_ONSLAUGHT]");
			aButton->Resize(270, 559, 257, 46);
		}
	}

	if (mChallengeState == STATECHALLENGE_LAST_STAND_ONSLAUGHT && mApp->mGameScene == SCENE_PLAYING)
		mChallengeStateCounter++;
}

void Challenge::LastStandCompletedStage()
{
	mApp->PlaySample(Sexy::SOUND_HUGE_WAVE);
	mChallengeState = STATECHALLENGE_NORMAL;
	mBoard->mSeedBank->RefreshAllPackets();
	
	Plant* aPlant = nullptr;
	while (mBoard->IteratePlants(aPlant))
	{
		if (aPlant->mState == STATE_CHOMPER_DIGESTING || aPlant->mState == STATE_COBCANNON_ARMING || 
			aPlant->mState == STATE_MAGNETSHROOM_SUCKING || aPlant->mState == STATE_MAGNETSHROOM_CHARGING)
		{
			aPlant->mStateCountdown = std::min(aPlant->mStateCountdown, 200);
		}
	}

	std::string aFlagStr = mApp->Pluralize(mBoard->GetSurvivalFlagsCompleted(), "[ONE_FLAG]", "[COUNT_FLAGS]");
	std::string aMsg = TodReplaceString("[SUCCESSFULLY_DEFENDED]", "{FLAGS}", aFlagStr);
	mBoard->DisplayAdvice(aMsg, MESSAGE_STYLE_BIG_MIDDLE_FAST, ADVICE_NONE);

	mSurvivalStage++;
	mBoard->mLevelComplete = false;
	mBoard->InitZombieWaves();
}

void Challenge::WhackAZombieUpdate()
{
	if (mBoard->mSunMoney > 0 && mBoard->mTutorialState == TUTORIAL_OFF)
	{
		mBoard->SetTutorialState(TUTORIAL_WHACK_A_ZOMBIE_BEFORE_PICK_SEED);
		mBoard->mTutorialTimer = 1500;
	}
	if (mBoard->mTutorialState == TUTORIAL_WHACK_A_ZOMBIE_BEFORE_PICK_SEED && mBoard->mTutorialTimer == 0)
	{
		mBoard->SetTutorialState(TUTORIAL_WHACK_A_ZOMBIE_PICK_SEED);
		mBoard->mTutorialTimer = 400;
	}
	if (mBoard->mTutorialState == TUTORIAL_WHACK_A_ZOMBIE_PICK_SEED && mBoard->mTutorialTimer == 0)
	{
		mBoard->SetTutorialState(TUTORIAL_WHACK_A_ZOMBIE_COMPLETED);
	}
}

int Challenge::TreeOfWisdomMouseOn(int theX, int theY)
{
	HitResult aHitResult;
	mBoard->MouseHitTest(theX, theY, &aHitResult);
	return (aHitResult.mObjectType == OBJECT_TYPE_TREE_OF_WISDOM && mBoard->mCursorObject->mCursorType == CURSOR_TYPE_TREE_FOOD);
}

int Challenge::TreeOfWisdomGetSize()
{
	return mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()];
}

void Challenge::TreeOfWisdomDraw(Graphics* g)
{
	int aMouseOn = TreeOfWisdomMouseOn(mApp->mWidgetManager->mLastMouseX - mBoard->mX, mApp->mWidgetManager->mLastMouseY - mBoard->mY);

	Reanimation* aReanimTree = mApp->ReanimationGet(mReanimChallenge);
	aReanimTree->mEnableExtraOverlayDraw = false;
	aReanimTree->DrawRenderGroup(g, 0);  // 绘制背景
	for (int i = 0; i < 6; i++)
	{
		mApp->ReanimationGet(mReanimClouds[i])->DrawRenderGroup(g, 0);
	}

	int aHeight = TreeOfWisdomGetSize();
	if (aMouseOn)
	{
		aReanimTree->mExtraOverlayColor = Color(255, 255, 255, aHeight < 18 ? 128 : 48);
		aReanimTree->mEnableExtraOverlayDraw = true;
	}
	else
	{
		aReanimTree->mEnableExtraOverlayDraw = false;
	}
	aReanimTree->DrawRenderGroup(g, 2);  // 绘制树干

	aReanimTree->mEnableExtraOverlayDraw = false;
	aReanimTree->DrawRenderGroup(g, 3);  // 绘制土地

	if (aMouseOn)
	{
		aReanimTree->mExtraOverlayColor = Color(255, 255, 255, 32);
		aReanimTree->mEnableExtraOverlayDraw = true;
	}
	else
	{
		aReanimTree->mEnableExtraOverlayDraw = false;
	}
	aReanimTree->DrawRenderGroup(g, 4);  // 绘制根部

	if (mChallengeState == STATECHALLENGE_TREE_GIVE_WISDOM || mChallengeState == STATECHALLENGE_TREE_BABBLING)
	{
		int aPosX, aPosY;
		if (aHeight < 7)
		{
			aPosX = 400;
			aPosY = 152;
		}
		else if (aHeight < 12)
		{
			aPosX = 395;
			aPosY = 60;
		}
		else
		{
			aPosX = 390;
			aPosY = 52;
		}

		g->DrawImage(Sexy::IMAGE_STORE_SPEECHBUBBLE2, aPosX, aPosY);
		std::string aText = StrFormat("[TREE_OF_WISDOM_%d]", mTreeOfWisdomTalkIndex);
		auto aWrapEnum = static_cast<DrawStringJustification>(mApp->GetInteger("TREE_OF_WISDOM_TEXT_WRAP_ENUM", DS_ALIGN_CENTER_VERTICAL_MIDDLE));
		TodDrawStringWrapped(g, aText, Rect(aPosX + 25, aPosY + 6, 233, 144), Sexy::FONT_BRIANNETOD16, Color::Black, aWrapEnum);
	}

	int aCurSize = aHeight;
	float aScale = 1.0f;
	if (mChallengeState == STATECHALLENGE_TREE_JUST_GREW)
	{
		if (mChallengeStateCounter > 30)
		{
			aCurSize--;
		}
		aScale = TodAnimateCurveFloat(55, 20, mChallengeStateCounter, 1.0f, 1.2f, CURVE_BOUNCE);
	}
	if (aCurSize >= 50)
	{
		std::string aSizeStr = TodReplaceNumberString("[TREE_OF_WISDOM_HIEGHT]", "{HEIGHT}", aCurSize);
		float aStrWidth = Sexy::FONT_HOUSEOFTERROR16->StringWidth(aSizeStr) * aScale;
		float aStrHeight = Sexy::FONT_HOUSEOFTERROR16->mAscent * aScale;

		SexyTransform2D aMatrix;
		TodScaleTransformMatrix(aMatrix, 400.0f - aStrWidth * 0.5f, 20.0f + aStrHeight * 0.5f, aScale, aScale);
		TodDrawStringMatrix(g, Sexy::FONT_HOUSEOFTERROR16, aMatrix, aSizeStr, Color(255, 255, 255));
	}
}

void Challenge::TreeOfWisdomInit()
{
	ReanimatorEnsureDefinitionLoaded(REANIM_TREEOFWISDOM, true);
	Reanimation* aReanimTree = mApp->AddReanimation(0.5f, 0.5f, 0, REANIM_TREEOFWISDOM);
	aReanimTree->mIsAttachment = true;
	aReanimTree->AssignRenderGroupToPrefix("bg", 0);
	aReanimTree->AssignRenderGroupToPrefix("tree", 2);
	aReanimTree->AssignRenderGroupToPrefix("grass", 3);
	aReanimTree->AssignRenderGroupToPrefix("overlay", 4);
	aReanimTree->AssignRenderGroupToPrefix("leaf", 4);
	aReanimTree->AssignRenderGroupToPrefix("bunch", 4);
	aReanimTree->SetTruncateDisappearingFrames(nullptr, false);
	mReanimChallenge = mApp->ReanimationGetID(aReanimTree);

	int aTreeSize = ClampInt(TreeOfWisdomGetSize(), 1, 50);
	aReanimTree->PlayReanim(StrFormat("anim_grow%d", aTreeSize).c_str(), REANIM_PLAY_ONCE_AND_HOLD, 0, 18.0f);
	if (aTreeSize == 0 && mApp->mPlayerInfo->mPurchases[STORE_ITEM_TREE_FOOD] < PURCHASE_COUNT_OFFSET)
	{
		aReanimTree->mFrameCount += aReanimTree->mFrameStart;
		aReanimTree->mFrameStart = 0;
	}
	else
	{
		aReanimTree->mAnimTime = 0.99f;
	}

	for (int i = 0; i < 6; i++)
	{
		Reanimation* aReanimCloud = mApp->AddReanimation(0, 0, 0, REANIM_TREEOFWISDOM_CLOUDS);
		aReanimCloud->PlayReanim(StrFormat("Cloud%d", i + 1).c_str(), REANIM_PLAY_ONCE_AND_HOLD, 0, 0);
		mReanimClouds[i] = mApp->ReanimationGetID(aReanimCloud);

		int aCloudCounter = RandRangeInt(-6000, 2000);
		mCloudsCounter[i] = aCloudCounter;
		if (aCloudCounter < 0)
		{
			aReanimCloud->mAnimTime = -aCloudCounter / 6000.0f;
			aReanimCloud->mAnimRate = 0.2f;
			mCloudsCounter[i] = 0;
		}
		else
		{
			aReanimCloud->mAnimRate = 0;
		}
	}

	mChallengeState = STATECHALLENGE_TREE_WAITING_TO_BABBLE;
	mChallengeStateCounter = RandRangeInt(700, 1500);
}

// GOTY @Patoke: 0x42FF00
void Challenge::TreeOfWisdomGrow()
{
	mApp->mPlayerInfo->mChallengeRecords[mApp->GetCurrentChallengeIndex()]++;
	int aTreeSize = TreeOfWisdomGetSize();
	mApp->ReanimationGet(mReanimChallenge)->PlayReanim(StrFormat("anim_grow%d", ClampInt(aTreeSize, 1, 51)).c_str(), REANIM_PLAY_ONCE_AND_HOLD, 0, 8.0f);
	mApp->PlayFoley(FOLEY_PLANTGROW);

	if (aTreeSize > 1)
	{
		mChallengeState = STATECHALLENGE_TREE_JUST_GREW;
		mChallengeStateCounter = 120;
	}
	else
	{
		mChallengeState = STATECHALLENGE_NORMAL;
	}

	if (aTreeSize == 100) // @Patoke: add achievement
		ReportAchievement::GiveAchievement(mApp, ToweringWisdom, true);
}

void Challenge::TreeOfWisdomFertilize()
{
	GridItem* aTreeFood = mBoard->mGridItems.DataArrayAlloc();
	aTreeFood->mPosX = 340.0f;
	aTreeFood->mPosY = 300.0f;
	aTreeFood->mGridItemType = GRIDITEM_ZEN_TOOL;
	aTreeFood->mGridX = 0;
	aTreeFood->mGridY = 0;
	aTreeFood->mRenderOrder = Board::MakeRenderOrder(RENDER_LAYER_ABOVE_UI, 0, 0);
	Reanimation* aReanim = mApp->AddReanimation(340.0f, 300.0f, 0, REANIM_TREEOFWISDOM_TREEFOOD);
	aReanim->mLoopType = REANIM_PLAY_ONCE_AND_HOLD;
	aTreeFood->mGridItemReanimID = mApp->ReanimationGetID(aReanim);
	aTreeFood->mGridItemState = GRIDITEM_STATE_ZEN_TOOL_FERTILIZER;

	mApp->PlayFoley(FOLEY_FERTILIZER);
	mApp->mPlayerInfo->mPurchases[STORE_ITEM_TREE_FOOD]--;
	mChallengeState = STATECHALLENGE_NORMAL;
	mBoard->ClearCursor();
}

void Challenge::TreeOfWisdomBabble()
{
	mChallengeState = STATECHALLENGE_TREE_BABBLING;
	mChallengeStateCounter = 400;

	int aTreeSize = TreeOfWisdomGetSize();
	int aBabbleHit = RandRangeInt(0, 2);
	if (aTreeSize <= 1)
	{
		mTreeOfWisdomTalkIndex = 600;
	}
	else if (aBabbleHit == 0 && aTreeSize >= 5)
	{
		mTreeOfWisdomTalkIndex = 500;
	}
	else if (aBabbleHit == 1)
	{
		mTreeOfWisdomTalkIndex = RandRangeInt(101, 110);
	}
	else
	{
		mTreeOfWisdomTalkIndex = Rand(4) + (aTreeSize < 12 ? 201 : aTreeSize < 50 ? 301 : 401);
	}
}

void Challenge::TreeOfWisdomGiveWisdom()
{
	mChallengeState = STATECHALLENGE_TREE_GIVE_WISDOM;
	mChallengeStateCounter = 1000;

	int aTreeSize = TreeOfWisdomGetSize();
	if (aTreeSize == 100)
	{
		mTreeOfWisdomTalkIndex = 800;
	}
	else if (aTreeSize == 500)
	{
		mTreeOfWisdomTalkIndex = 900;
	}
	else if (aTreeSize == 1000)
	{
		mTreeOfWisdomTalkIndex = 1000;
	}
	else if (aTreeSize > 1000)
	{
		mTreeOfWisdomTalkIndex = 1100;
	}
	else
	{
		mTreeOfWisdomTalkIndex = ClampInt(aTreeSize - 1, 1, 49);
	}
}

void Challenge::TreeOfWisdomSayRepeat()
{
	int aTreeSize = TreeOfWisdomGetSize();
	if (aTreeSize >= 100 && Rand(47) == 0)
	{
		mTreeOfWisdomTalkIndex = 800;
	}
	else if (aTreeSize >= 500 && Rand(47) == 0)
	{
		mTreeOfWisdomTalkIndex = 900;
	}
	else if (aTreeSize >= 1000 && Rand(47) == 0)
	{
		mTreeOfWisdomTalkIndex = 1000;
	}
	else
	{
		mTreeOfWisdomTalkIndex = RandRangeInt(2, ClampInt(aTreeSize, 3, 49));
	}

	mChallengeStateCounter = 600;
}

void Challenge::TreeOfWisdomToolUpdate(GridItem* theZenTool)
{
	Reanimation* aReanim = mApp->ReanimationTryToGet(theZenTool->mGridItemReanimID);
	if (aReanim && aReanim->mLoopCount > 0)
	{
		TreeOfWisdomGrow();
		theZenTool->GridItemDie();
	}
}

void Challenge::TreeOfWisdomUpdate()
{
	GridItem* aGridItem = nullptr;
	while (mBoard->IterateGridItems(aGridItem))
	{
		TreeOfWisdomToolUpdate(aGridItem);
	}

	if (mApp->mGameScene == SCENE_PLAYING && mChallengeStateCounter > 0)
	{
		mChallengeStateCounter--;
	}
	if (mChallengeStateCounter == 0)
	{
		if (mChallengeState == STATECHALLENGE_TREE_JUST_GREW)
		{
			TreeOfWisdomGiveWisdom();
		}
		else if (mChallengeState == STATECHALLENGE_TREE_WAITING_TO_BABBLE)
		{
			TreeOfWisdomBabble();
		}
		else if (mChallengeState == STATECHALLENGE_TREE_BABBLING || mChallengeState == STATECHALLENGE_TREE_GIVE_WISDOM)
		{
			if (mTreeOfWisdomTalkIndex == 500)
			{
				TreeOfWisdomSayRepeat();
			}
			else
			{
				mChallengeState = STATECHALLENGE_TREE_WAITING_TO_BABBLE;
				mChallengeStateCounter = RandRangeInt(700, 1000);
			}
		}
	}

	for (int i = 5; i >= 0; i--) // Off by one error!
	{
		Reanimation* aReanimCloud = mApp->ReanimationGet(mReanimClouds[i]);
		if (mCloudsCounter[i] > 0)
		{
			mCloudsCounter[i]--;
			if (mCloudsCounter[i] == 0)
			{
				aReanimCloud->mLoopCount = 0;
				aReanimCloud->mAnimTime = 0.0f;
				aReanimCloud->mAnimRate = 0.2f;
			}
		}
		else if (aReanimCloud->mLoopCount > 0)
		{
			mCloudsCounter[i] = RandRangeInt(2000, 4000);
		}

		aReanimCloud->Update();
	}
}

void Challenge::TreeOfWisdomLeave()
{
	GridItem* aGridItem = nullptr;
	while (mBoard->IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType == GRIDITEM_ZEN_TOOL)
		{
			TreeOfWisdomGrow();
			aGridItem->GridItemDie();
		}
	}
}

void Challenge::TreeOfWisdomNextGarden()
{
	TreeOfWisdomLeave();
	mApp->KillBoard();
	mApp->PreNewGame(GAMEMODE_CHALLENGE_ZEN_GARDEN, false);
}

void Challenge::TreeOfWisdomOpenStore()
{
	TreeOfWisdomLeave();
	StoreScreen* aStore = mApp->ShowStoreScreen();
	aStore->mBackButton->SetLabel("[STORE_BACK_TO_GAME]");
	aStore->mPage = STORE_PAGE_ZEN2;
	aStore->WaitForResult(true);
	mApp->mMusic->MakeSureMusicIsPlaying(MUSIC_TUNE_ZEN_GARDEN);
}

void Challenge::TreeOfWisdomTool(int theMouseX, int theMouseY)
{
	if (TreeOfWisdomMouseOn(theMouseX, theMouseY))
	{
		TreeOfWisdomFertilize();
	}

	mBoard->ClearCursor();
}

int Challenge::TreeOfWisdomHitTest(int theX, int theY, HitResult* theHitResult)
{
	Rect aTreeRect;
	int aTreeSize = TreeOfWisdomGetSize();
	if (aTreeSize <= 1)			aTreeRect = Rect(310, 275, 175, 175);
	else if (aTreeSize < 7)		aTreeRect = Rect(290, 255, 205, 195);
	else if (aTreeSize < 12)	aTreeRect = Rect(290, 215, 205, 225);
	else						aTreeRect = Rect(280, 155, 225, 305);

	if (aTreeRect.Contains(theX, theY))
	{
		theHitResult->mObject = nullptr;
		theHitResult->mObjectType = OBJECT_TYPE_TREE_OF_WISDOM;
		return true;
	}
	else
	{
		theHitResult->mObject = nullptr;
		theHitResult->mObjectType = OBJECT_TYPE_NONE;
		return false;
	}
}

int Challenge::TreeOfWisdomCanFeed()
{
	if (mChallengeState == STATECHALLENGE_TREE_JUST_GREW)
		return false;
	
	GridItem* aGridItem = nullptr;
	while (mBoard->IterateGridItems(aGridItem))
	{
		if (aGridItem->mGridItemType == GRIDITEM_ZEN_TOOL)
		{
			return false;
		}
	}

	return true;
}
