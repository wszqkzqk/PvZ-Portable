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

#ifndef __PLAYERINFO_H__
#define __PLAYERINFO_H__

#define MAX_POTTED_PLANTS 200
#define PURCHASE_COUNT_OFFSET 1000
#define ZOMBATAR_RECORD_SIZE 0x48
#define MAX_ZOMBATAR_HEADS 100

#include <cstdint>
#include <ctime>
#include <vector>
#include "../../ConstEnums.h"
#include "../../SexyAppFramework/Common.h"

class PottedPlant
{
public:
    enum FacingDirection : int32_t
    {
        FACING_RIGHT,
        FACING_LEFT
    };

public:
    SeedType            mSeedType;
    GardenType          mWhichZenGarden;
    int32_t             mX;
    int32_t             mY;
    FacingDirection     mFacing;
    uint32_t            mPadding1;                  // for explicit alignment, unused
    int64_t             mLastWateredTime;
    DrawVariation       mDrawVariation;
    PottedPlantAge      mPlantAge;
    int32_t             mTimesFed;
    int32_t             mFeedingsPerGrow;
    PottedPlantNeed     mPlantNeed;
    uint32_t            mPadding2;                  // for explicit alignment, unused
    int64_t             mLastNeedFulfilledTime;
    int64_t             mLastFertilizedTime;
    int64_t             mLastChocolateTime;
    int64_t             mFutureAttribute[1];

public:
    void                InitializePottedPlant(SeedType theSeedType);
};

class DataSync;
class PlayerInfo
{
public:
    std::string         mName;
    uint32_t            mUseSeq;
    uint32_t            mId;
    int32_t             mLevel;
    int32_t             mCoins;
    uint32_t            mFinishedAdventure;
    uint32_t            mChallengeRecords[100];
    uint32_t            mPurchases[80];
    uint32_t            mPlayTimeActivePlayer;
    uint32_t            mPlayTimeInactivePlayer;
    int32_t             mHasUsedCheatKeys;
    int32_t             mHasWokenStinky;
    int32_t             mDidntPurchasePacketUpgrade;
    uint32_t            mLastStinkyChocolateTime;
    int32_t             mStinkyPosX;
    int32_t             mStinkyPosY;
    int32_t             mHasUnlockedMinigames;
    int32_t             mHasUnlockedPuzzleMode;
    int32_t             mHasNewMiniGame;
    int32_t             mHasNewScaryPotter;
    int32_t             mHasNewIZombie;
    int32_t             mHasNewSurvival;
    int32_t             mHasUnlockedSurvivalMode;
    int32_t             mNeedsMessageOnGameSelector;
    int32_t             mNeedsMagicTacoReward;
    int32_t             mHasSeenStinky;
    int32_t             mHasSeenUpsell;
    int32_t             mPlaceHolderPlayerStats;
    int32_t             mNumPottedPlants;
    PottedPlant         mPottedPlant[MAX_POTTED_PLANTS];
    bool                mEarnedAchievements[20];
    bool                mShownAchievements[20];
    unsigned char       mZombatarAccepted;
    uint32_t            mZombatarHeadCount;
    std::vector<unsigned char> mZombatarData;               // raw 0x48 * count
    // mini-game completion flags (20 bytes in the save after the Zombatar records) are derived from mChallengeRecords at save time; no runtime field
    unsigned char       mZombatarCreatedBefore;             // created at least one Zombatar (0/1)

public:
    PlayerInfo();

    void                Reset();
    void                AddCoins(int theAmount);
    void                SyncSummary(DataSync& theSync);
    void                SyncDetails(DataSync& theSync);
    void                DeleteUserFiles();
    void                LoadDetails();
    void                SaveDetails();
    inline int          GetLevel() const { return mLevel; }
    inline void         SetLevel(int theLevel) { mLevel = theLevel; }
    void                ResetChallengeRecord(GameMode theGameMode);
};

#endif
