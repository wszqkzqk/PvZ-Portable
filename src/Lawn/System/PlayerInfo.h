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
    SeedType            mSeedType;                  //+0x0
    GardenType          mWhichZenGarden;            //+0x4
    int32_t             mX;                         //+0x8
    int32_t             mY;                         //+0xC
    FacingDirection     mFacing;                    //+0x10
    uint32_t            mPadding1;                  //+0x14 for explicit alignment, unused
    int64_t             mLastWateredTime;           //+0x18
    DrawVariation       mDrawVariation;             //+0x20
    PottedPlantAge      mPlantAge;                  //+0x24
    int32_t             mTimesFed;                  //+0x28
    int32_t             mFeedingsPerGrow;           //+0x2C
    PottedPlantNeed     mPlantNeed;                 //+0x30
    uint32_t            mPadding2;                  //+0x34, for explicit alignment, unused
    int64_t             mLastNeedFulfilledTime;     //+0x38
    int64_t             mLastFertilizedTime;        //+0x40
    int64_t             mLastChocolateTime;         //+0x48
    int64_t             mFutureAttribute[1];        //+0x50

public:
    void                InitializePottedPlant(SeedType theSeedType);
};

class DataSync;
class PlayerInfo
{
public:
    std::string         mName;                              //+0x0
    uint32_t            mUseSeq;                            //+0x1C
    uint32_t            mId;                                //+0x20
    int32_t             mLevel;                             //+0x24
    int32_t             mCoins;                             //+0x28
    uint32_t            mFinishedAdventure;                 //+0x2C
    uint32_t            mChallengeRecords[100];             //+0x30
    uint32_t            mPurchases[80];                     //+0x1C0
    uint32_t            mPlayTimeActivePlayer;              //+0x300
    uint32_t            mPlayTimeInactivePlayer;            //+0x304
    int32_t             mHasUsedCheatKeys;                  //+0x308
    int32_t             mHasWokenStinky;                    //+0x30C
    int32_t             mDidntPurchasePacketUpgrade;        //+0x310
    uint32_t            mLastStinkyChocolateTime;           //+0x314
    int32_t             mStinkyPosX;                        //+0x318
    int32_t             mStinkyPosY;                        //+0x31C
    int32_t             mHasUnlockedMinigames;              //+0x320
    int32_t             mHasUnlockedPuzzleMode;             //+0x324
    int32_t             mHasNewMiniGame;                    //+0x328
    int32_t             mHasNewScaryPotter;                 //+0x32C
    int32_t             mHasNewIZombie;                     //+0x330
    int32_t             mHasNewSurvival;                    //+0x334
    int32_t             mHasUnlockedSurvivalMode;           //+0x338
    int32_t             mNeedsMessageOnGameSelector;        //+0x33C
    int32_t             mNeedsMagicTacoReward;              //+0x340
    int32_t             mHasSeenStinky;                     //+0x344
    int32_t             mHasSeenUpsell;                     //+0x348
    int32_t             mPlaceHolderPlayerStats;            //+0x??????
    int32_t             mNumPottedPlants;                   //+0x350
    PottedPlant         mPottedPlant[MAX_POTTED_PLANTS];    //+0x358
    bool                mEarnedAchievements[20];            //+GOTY @Patoke: 0x24
    bool                mShownAchievements[20];             //+GOTY
    unsigned char       mZombatarAccepted;                  //+GOTY: 0x28
    uint32_t            mZombatarHeadCount;                 //+GOTY: 0x29
    std::vector<unsigned char> mZombatarData;               // raw 0x48 * count
    int32_t             mZombatarIndex;                     // selected Zombatar index, mirrored into the trailing block
    unsigned char       mZombatarTrailingUnknown[0x14];     // trailing Zombatar metadata; first uint32 stores selected index
    unsigned char       mZombatarCreatedBefore;             // created at least one Zombatar (0/1)

public:
    PlayerInfo();

    void                Reset();
    /*inline*/ void     AddCoins(int theAmount);
    void                SyncSummary(DataSync& theSync);
    void                SyncDetails(DataSync& theSync);
    void                DeleteUserFiles();
    void                LoadDetails();
    void                SaveDetails();
    inline int          GetLevel() const { return mLevel; }
    inline void         SetLevel(int theLevel) { mLevel = theLevel; }
    /*inline*/ void     ResetChallengeRecord(GameMode theGameMode);
};

#endif
