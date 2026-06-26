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

#include <bit>

#include "DataSync.h"
#include <algorithm>
#include "PlayerInfo.h"
#include "../LawnCommon.h"
#include "../Widget/ChallengeScreen.h"
#include "../../Sexy.TodLib/TodDebug.h"
#include "../../Sexy.TodLib/TodCommon.h"
#include "misc/Buffer.h"
#include "../../SexyAppFramework/SexyAppBase.h"

static int gUserVersion = 12;

// Convert PottedPlant between little-endian file format and native byte order.
// No-op on little-endian machines (entire function optimized away at compile time).
static inline void PottedPlantFromLE(PottedPlant& p)
{
	if constexpr (std::endian::native == std::endian::little)
		return;

	p.mSeedType = static_cast<SeedType>(FromLE32(static_cast<uint32_t>(p.mSeedType)));
	p.mWhichZenGarden = static_cast<GardenType>(FromLE32(static_cast<uint32_t>(p.mWhichZenGarden)));
	p.mX = static_cast<int32_t>(FromLE32(static_cast<uint32_t>(p.mX)));
	p.mY = static_cast<int32_t>(FromLE32(static_cast<uint32_t>(p.mY)));
	p.mFacing = static_cast<PottedPlant::FacingDirection>(FromLE32(static_cast<uint32_t>(p.mFacing)));
	p.mLastWateredTime = static_cast<int64_t>(FromLE64(static_cast<uint64_t>(p.mLastWateredTime)));
	p.mDrawVariation = static_cast<DrawVariation>(FromLE32(static_cast<uint32_t>(p.mDrawVariation)));
	p.mPlantAge = static_cast<PottedPlantAge>(FromLE32(static_cast<uint32_t>(p.mPlantAge)));
	p.mTimesFed = static_cast<int32_t>(FromLE32(static_cast<uint32_t>(p.mTimesFed)));
	p.mFeedingsPerGrow = static_cast<int32_t>(FromLE32(static_cast<uint32_t>(p.mFeedingsPerGrow)));
	p.mPlantNeed = static_cast<PottedPlantNeed>(FromLE32(static_cast<uint32_t>(p.mPlantNeed)));
	p.mLastNeedFulfilledTime = static_cast<int64_t>(FromLE64(static_cast<uint64_t>(p.mLastNeedFulfilledTime)));
	p.mLastFertilizedTime = static_cast<int64_t>(FromLE64(static_cast<uint64_t>(p.mLastFertilizedTime)));
	p.mLastChocolateTime = static_cast<int64_t>(FromLE64(static_cast<uint64_t>(p.mLastChocolateTime)));
	p.mFutureAttribute[0] = static_cast<int64_t>(FromLE64(static_cast<uint64_t>(p.mFutureAttribute[0])));
}

// ToLE is identical to FromLE for byte swapping (both swap on big-endian, no-op on little-endian)
static inline void PottedPlantToLE(PottedPlant& p) { PottedPlantFromLE(p); }

PlayerInfo::PlayerInfo()
{
	Reset();
}

void PlayerInfo::SyncSummary(DataSync& theSync)
{
	theSync.SyncString(mName);
	theSync.SyncUInt32(mUseSeq);
	theSync.SyncUInt32(mId);
}

void PlayerInfo::SyncDetails(DataSync& theSync)
{
	if (theSync.GetReader())
	{
		Reset();
	}

	int aVersion = gUserVersion;
	theSync.SyncUInt32(aVersion);
	theSync.SetVersion(aVersion);
	if (aVersion != gUserVersion)
	{
		return;
	}

	theSync.SyncUInt32(mLevel);
	theSync.SyncUInt32(mCoins);
	theSync.SyncUInt32(mFinishedAdventure);
	for (int i = 0; i < 100; i++)
	{
		theSync.SyncUInt32(mChallengeRecords[i]);
	}
	for (int i = 0; i < 80; i++)
	{
		theSync.SyncUInt32(mPurchases[i]);
	}
	theSync.SyncUInt32(mPlayTimeActivePlayer);
	theSync.SyncUInt32(mPlayTimeInactivePlayer);
	theSync.SyncUInt32(mHasUsedCheatKeys);
	theSync.SyncUInt32(mHasWokenStinky);
	theSync.SyncUInt32(mDidntPurchasePacketUpgrade);
	theSync.SyncUInt32(mLastStinkyChocolateTime);
	theSync.SyncUInt32(mStinkyPosX);
	theSync.SyncUInt32(mStinkyPosY);
	theSync.SyncUInt32(mHasUnlockedMinigames);
	theSync.SyncUInt32(mHasUnlockedPuzzleMode);
	theSync.SyncUInt32(mHasNewMiniGame);
	theSync.SyncUInt32(mHasNewScaryPotter);
	theSync.SyncUInt32(mHasNewIZombie);
	theSync.SyncUInt32(mHasNewSurvival);
	theSync.SyncUInt32(mHasUnlockedSurvivalMode);
	theSync.SyncUInt32(mNeedsMessageOnGameSelector);
	theSync.SyncUInt32(mNeedsMagicTacoReward);
	theSync.SyncUInt32(mHasSeenStinky);
	theSync.SyncUInt32(mHasSeenUpsell);
	theSync.SyncUInt32(mPlaceHolderPlayerStats);
	theSync.SyncUInt32(mNumPottedPlants);
	
	TOD_ASSERT(mNumPottedPlants <= MAX_POTTED_PLANTS);
	for (int i = 0; i < mNumPottedPlants; i++)
	{
		if (theSync.GetWriter())
			PottedPlantToLE(mPottedPlant[i]);
		theSync.SyncBytes(&mPottedPlant[i], sizeof(PottedPlant));
		PottedPlantFromLE(mPottedPlant[i]);
	}

	// Implemented by wszqkzqk with doc: https://plantsvszombies.fandom.com/wiki/User_file_format
	// Known that achievements are stored as 20 x 16-bit values (0/1) in the original format.
	for (int i = 0; i < 20; i++)
	{
		uint16_t aAchievementValue = mEarnedAchievements[i] ? 1 : 0;
		theSync.SyncUInt16(aAchievementValue);
		if (theSync.GetReader())
		{
			mEarnedAchievements[i] = (aAchievementValue != 0);
			mShownAchievements[i] = mEarnedAchievements[i];
		}
	}

	// Zombatar is not supported: ignore any stored data on load.
	if (theSync.GetReader())
	{
		mZombatarAccepted = 0;
		mZombatarHeadCount = 0;
		mZombatarData.clear();
		memset(mZombatarTrailingUnknown, 0, sizeof(mZombatarTrailingUnknown));
		mZombatarCreatedBefore = 0;
		return;
	}

	// Write a minimal, safe layout (no Zombatars).
	theSync.SyncUInt8(mZombatarAccepted);
	theSync.SyncUInt32(mZombatarHeadCount);
	theSync.SyncBytes(mZombatarTrailingUnknown, sizeof(mZombatarTrailingUnknown));
	theSync.SyncUInt8(mZombatarCreatedBefore);
}

void PlayerInfo::LoadDetails()
{
	try
	{
		Buffer aBuffer;
		std::string aFileName = GetAppDataPath(StrFormat("userdata/user%d.dat", mId));
		if (!gSexyAppBase->ReadBufferFromFile(aFileName, &aBuffer, false))
		{
			return;
		}

		DataReader aReader;
		aReader.OpenMemory(aBuffer.GetDataPtr(), aBuffer.GetDataLen(), false);
		DataSync aSync(aReader);
		SyncDetails(aSync);
	}
	catch (DataReaderException&)
	{
		TodTrace("Failed to player data, resetting it\n");
		Reset();
	}
}

// GOTY @Patoke: 0x46D750
void PlayerInfo::SaveDetails()
{
	DataWriter aWriter;
	aWriter.OpenMemory();
	DataSync aSync(aWriter);
	SyncDetails(aSync);

	MkDir(GetAppDataPath("userdata"));
	std::string aFileName = GetAppDataPath(StrFormat("userdata/user%d.dat", mId));
	gSexyAppBase->WriteBytesToFile(aFileName, aWriter.GetDataPtr(), aWriter.GetDataLen());
}

void PlayerInfo::DeleteUserFiles()
{
	std::string aFilename = GetAppDataPath(StrFormat("userdata/user%d.dat", mId));
	gSexyAppBase->EraseFile(aFilename);

	for (int i = 0; i < static_cast<int>(GameMode::NUM_GAME_MODES); i++)
	{
		std::string aFileName = GetSavedGameName((GameMode)i, mId);
		gSexyAppBase->EraseFile(aFileName);
		std::string aLegacyFileName = GetLegacySavedGameName((GameMode)i, mId);
		gSexyAppBase->EraseFile(aLegacyFileName);
	}
}

void PlayerInfo::Reset()
{
	mLevel = 1;
	mCoins = 0;
	mFinishedAdventure = 0;
	memset(mChallengeRecords, 0, sizeof(mChallengeRecords));
	memset(mPurchases, 0, sizeof(mPurchases));
	mPlayTimeActivePlayer = 0;
	mPlayTimeInactivePlayer = 0;
	mHasUsedCheatKeys = 0;
	mHasWokenStinky = 0;
	mDidntPurchasePacketUpgrade = 0;
	mLastStinkyChocolateTime = 0;
	mStinkyPosX = 0;
	mStinkyPosY = 0;
	mHasUnlockedMinigames = 0;
	mHasUnlockedPuzzleMode = 0;
	mHasNewMiniGame = 0;
	mHasNewScaryPotter = 0;
	mHasNewIZombie = 0;
	mHasNewSurvival = 0;
	mHasUnlockedSurvivalMode = 0;
	mNeedsMessageOnGameSelector = 0;
	mNeedsMagicTacoReward = 0;
	mHasSeenStinky = 0;
	mHasSeenUpsell = 0;
	mPlaceHolderPlayerStats = 0;
	memset(mPottedPlant, 0, sizeof(mPottedPlant));
	mNumPottedPlants = 0;
	memset(mEarnedAchievements, 0, sizeof(mEarnedAchievements));
	memset(mShownAchievements, 0, sizeof(mShownAchievements));
	mZombatarAccepted = 0;
	mZombatarHeadCount = 0;
	mZombatarData.clear();
	memset(mZombatarTrailingUnknown, 0, sizeof(mZombatarTrailingUnknown));
	mZombatarCreatedBefore = 0;
}

void PlayerInfo::AddCoins(int theAmount)
{
	mCoins += theAmount;
	mCoins = std::clamp(mCoins, 0, 99999);
}

void PlayerInfo::ResetChallengeRecord(GameMode theGameMode)
{
	int aGameMode = static_cast<int>(theGameMode) - static_cast<int>(GameMode::GAMEMODE_SURVIVAL_NORMAL_STAGE_1);
	TOD_ASSERT(aGameMode >= 0 && aGameMode <= NUM_CHALLENGE_MODES);
	mChallengeRecords[aGameMode] = 0;
}

void PottedPlant::InitializePottedPlant(SeedType theSeedType)
{
	memset(this, 0, sizeof(PottedPlant));
	mSeedType = theSeedType;
	mDrawVariation = DrawVariation::VARIATION_NORMAL;
	mLastWateredTime = 0;
	mFacing = static_cast<FacingDirection>(RandRangeInt(static_cast<int>(FacingDirection::FACING_RIGHT), static_cast<int>(FacingDirection::FACING_LEFT)));
	mPlantAge = PottedPlantAge::PLANTAGE_SPROUT;
	mTimesFed = 0;
	mWhichZenGarden = GardenType::GARDEN_MAIN;
	mFeedingsPerGrow = RandRangeInt(3, 5);
	mPlantNeed = PottedPlantNeed::PLANTNEED_NONE;
	mLastNeedFulfilledTime = 0;
	mLastFertilizedTime = 0;
	mLastChocolateTime = 0;
}
