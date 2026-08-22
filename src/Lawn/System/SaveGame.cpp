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

#include "Music.h"
#include "SaveGame.h"
#include "../Board.h"
#include "../Challenge.h"
#include "../SeedPacket.h"
#include "../../LawnApp.h"
#include "../CursorObject.h"
#include "../../Resources.h"
#include "../../ConstEnums.h"
#include "../MessageWidget.h"
#include "../../PvzpLib/Trail.h"
#include "zlib.h"
#include "../../PvzpLib/Attachment.h"
#include "../../PvzpLib/Reanimator.h"
#include "../../PvzpLib/PvzpParticle.h"
#include "../../PvzpLib/EffectSystem.h"
#include "../../PvzpLib/DataArray.h"
#include "../../PvzpLib/PvzpList.h"
#include "DataSync.h"
#include "misc/Buffer.h"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>

static constexpr const char* FILE_COMPILE_TIME_STRING = "Jul  2 201011:47:03"; // save files are tied to this exact timestamp string
static constexpr const uint32_t SAVE_FILE_MAGIC_NUMBER = 0xFEEDDEAD;
static constexpr const uint32_t SAVE_FILE_VERSION = 2U;
static const uint32_t SAVE_FILE_DATE = crc32(0, (Bytef*)FILE_COMPILE_TIME_STRING, strlen(FILE_COMPILE_TIME_STRING));

static constexpr const char SAVE_FILE_MAGIC_V4[12] = "PVZP_SAVE4";
static constexpr const uint32_t SAVE_FILE_V4_VERSION = 1U;

struct SaveFileHeaderV4
{
	char		mMagic[12];
	uint32_t	mVersion;
	uint32_t	mPayloadSize;
	uint32_t	mPayloadCrc;
};

struct SaveFileHeader
{
	uint32_t	mMagicNumber;
	uint32_t	mBuildVersion;
	uint32_t	mBuildDate;
};

enum SaveChunkTypeV4
{
	SAVE4_CHUNK_BOARD_BASE = 1,
	SAVE4_CHUNK_ZOMBIES = 2,
	SAVE4_CHUNK_PLANTS = 3,
	SAVE4_CHUNK_PROJECTILES = 4,
	SAVE4_CHUNK_COINS = 5,
	SAVE4_CHUNK_MOWERS = 6,
	SAVE4_CHUNK_GRIDITEMS = 7,
	SAVE4_CHUNK_PARTICLE_EMITTERS = 8,
	SAVE4_CHUNK_PARTICLE_PARTICLES = 9,
	SAVE4_CHUNK_PARTICLE_SYSTEMS = 10,
	SAVE4_CHUNK_REANIMATIONS = 11,
	SAVE4_CHUNK_TRAILS = 12,
	SAVE4_CHUNK_ATTACHMENTS = 13,
	SAVE4_CHUNK_CURSOR = 14,
	SAVE4_CHUNK_CURSOR_PREVIEW = 15,
	SAVE4_CHUNK_ADVICE = 16,
	SAVE4_CHUNK_SEEDBANK = 17,
	SAVE4_CHUNK_SEEDPACKETS = 18,
	SAVE4_CHUNK_CHALLENGE = 19,
	SAVE4_CHUNK_MUSIC = 20
};

static constexpr const uint32_t SAVE4_CHUNK_VERSION = 1U;

static void AppendU32LE(std::vector<unsigned char>& theOut, uint32_t theValue)
{
	unsigned char aBytes[4];
	aBytes[0] = static_cast<unsigned char>(theValue & 0xFF);
	aBytes[1] = static_cast<unsigned char>((theValue >> 8) & 0xFF);
	aBytes[2] = static_cast<unsigned char>((theValue >> 16) & 0xFF);
	aBytes[3] = static_cast<unsigned char>((theValue >> 24) & 0xFF);
	theOut.insert(theOut.end(), aBytes, aBytes + 4);
}

static void AppendBytes(std::vector<unsigned char>& theOut, const void* theData, size_t theLen)
{
	const unsigned char* aBytes = reinterpret_cast<const unsigned char*>(theData);
	theOut.insert(theOut.end(), aBytes, aBytes + theLen);
}

static void AppendChunk(std::vector<unsigned char>& theOut, uint32_t theChunkType, const std::vector<unsigned char>& theChunkData)
{
	AppendU32LE(theOut, theChunkType);
	AppendU32LE(theOut, static_cast<uint32_t>(theChunkData.size()));
	AppendBytes(theOut, theChunkData.data(), theChunkData.size());
}

class TLVReader
{
public:
	const unsigned char* mData;
	size_t mSize;
	size_t mPos;
	bool mOk;

public:
	TLVReader(const unsigned char* theData, size_t theSize)
		: mData(theData), mSize(theSize), mPos(0), mOk(true)
	{
	}

	bool ReadU32(uint32_t& theValue)
	{
		if (mPos + 4 > mSize)
		{
			mOk = false;
			theValue = 0;
			return false;
		}
		theValue = static_cast<uint32_t>(mData[mPos]) |
			(static_cast<uint32_t>(mData[mPos + 1]) << 8) |
			(static_cast<uint32_t>(mData[mPos + 2]) << 16) |
			(static_cast<uint32_t>(mData[mPos + 3]) << 24);
		mPos += 4;
		return true;
	}

	bool ReadBytes(const unsigned char*& thePtr, size_t theLen)
	{
		if (mPos + theLen > mSize)
		{
			mOk = false;
			thePtr = nullptr;
			return false;
		}
		thePtr = mData + mPos;
		mPos += theLen;
		return true;
	}
};

class PortableSaveContext
{
public:
	bool		mReading = false;
	bool		mFailed = false;
	DataReader*	mReader = nullptr;
	DataWriter*	mWriter = nullptr;

public:
	explicit PortableSaveContext(DataReader& theReader)
	{
		mReading = true;
		mReader = &theReader;
	}

	explicit PortableSaveContext(DataWriter& theWriter)
	{
		mReading = false;
		mWriter = &theWriter;
	}

	void SyncBytes(void* theData, uint32_t theDataLen)
	{
		try
		{
			if (mReading)
			{
				mReader->ReadBytes(theData, theDataLen);
			}
			else
			{
				mWriter->WriteBytes(theData, theDataLen);
			}
		}
		catch (DataReaderException&)
		{
			mFailed = true;
			if (mReading)
				memset(theData, 0, theDataLen);
		}
	}

	void SyncBytes(const void* theData, uint32_t theDataLen)
	{
		if (mReading)
		{
			mFailed = true;
			return;
		}
		mWriter->WriteBytes(theData, theDataLen);
	}

	void SyncBool(bool& theBool)
	{
		if (mReading)
		{
			try
			{
				theBool = mReader->ReadBool();
			}
			catch (DataReaderException&)
			{
				mFailed = true;
				theBool = false;
			}
		}
		else
		{
			mWriter->WriteBool(theBool);
		}
	}

	void SyncUInt32(uint32_t& theValue)
	{
		if (mReading)
		{
			try
			{
				theValue = mReader->ReadUInt32();
			}
			catch (DataReaderException&)
			{
				mFailed = true;
				theValue = 0;
			}
		}
		else
		{
			mWriter->WriteUInt32(theValue);
		}
	}

	void SyncInt32(int32_t& theValue)
	{
		if (mReading)
		{
			try
			{
				theValue = static_cast<int32_t>(mReader->ReadUInt32());
			}
			catch (DataReaderException&)
			{
				mFailed = true;
				theValue = 0;
			}
		}
		else
		{
			mWriter->WriteUInt32(static_cast<uint32_t>(theValue));
		}
	}

	void SyncFloat(float& theValue)
	{
		if (mReading)
		{
			try
			{
				theValue = mReader->ReadFloat();
			}
			catch (DataReaderException&)
			{
				mFailed = true;
				theValue = 0.0f;
			}
		}
		else
		{
			mWriter->WriteFloat(theValue);
		}
	}

	void SyncUInt64(uint64_t& theValue)
	{
		uint32_t aLow = static_cast<uint32_t>(theValue & 0xFFFFFFFFULL);
		uint32_t aHigh = static_cast<uint32_t>((theValue >> 32) & 0xFFFFFFFFULL);
		SyncUInt32(aLow);
		SyncUInt32(aHigh);
		if (mReading)
			theValue = (static_cast<uint64_t>(aHigh) << 32) | aLow;
	}

	void SyncInt64(int64_t& theValue)
	{
		uint64_t aValue = static_cast<uint64_t>(theValue);
		SyncUInt64(aValue);
		if (mReading)
			theValue = static_cast<int64_t>(aValue);
	}

	template <typename TEnum>
	void SyncEnum(TEnum& theEnum)
	{
		int32_t aValue = static_cast<int32_t>(theEnum);
		SyncInt32(aValue);
		if (mReading)
			theEnum = static_cast<TEnum>(aValue);
	}
};

static void SyncColorPortable(PortableSaveContext& theContext, Color& theColor)
{
	theContext.SyncInt32(theColor.mRed);
	theContext.SyncInt32(theColor.mGreen);
	theContext.SyncInt32(theColor.mBlue);
	theContext.SyncInt32(theColor.mAlpha);
}

static void SyncVector2Portable(PortableSaveContext& theContext, SexyVector2& theVector)
{
	theContext.SyncFloat(theVector.x);
	theContext.SyncFloat(theVector.y);
}

static void SyncMatrixPortable(PortableSaveContext& theContext, SexyMatrix3& theMatrix)
{
	theContext.SyncFloat(theMatrix.m00);
	theContext.SyncFloat(theMatrix.m01);
	theContext.SyncFloat(theMatrix.m02);
	theContext.SyncFloat(theMatrix.m10);
	theContext.SyncFloat(theMatrix.m11);
	theContext.SyncFloat(theMatrix.m12);
	theContext.SyncFloat(theMatrix.m20);
	theContext.SyncFloat(theMatrix.m21);
	theContext.SyncFloat(theMatrix.m22);
}

static void SyncRectPortable(PortableSaveContext& theContext, Rect& theRect)
{
	theContext.SyncInt32(theRect.mX);
	theContext.SyncInt32(theRect.mY);
	theContext.SyncInt32(theRect.mWidth);
	theContext.SyncInt32(theRect.mHeight);
}

static void SyncReanimationDefPortable(PortableSaveContext& theContext, ReanimatorDefinition*& theDefinition)
{
	if (theContext.mReading)
	{
		int aReanimType = 0;
		theContext.SyncInt32(aReanimType);
		if (aReanimType == static_cast<int>(ReanimationType::REANIM_NONE))
		{
			theDefinition = nullptr;
		}
		else if (aReanimType >= 0 && aReanimType < static_cast<int>(ReanimationType::NUM_REANIMS))
		{
			ReanimatorEnsureDefinitionLoaded(static_cast<ReanimationType>(aReanimType), true);
			theDefinition = &gReanimatorDefArray[aReanimType];
		}
		else
		{
			theContext.mFailed = true;
		}
	}
	else
	{
		int aReanimType = static_cast<int>(ReanimationType::REANIM_NONE);
		for (int i = 0; i < static_cast<int>(ReanimationType::NUM_REANIMS); i++)
		{
			ReanimatorDefinition* aDef = &gReanimatorDefArray[i];
			if (theDefinition == aDef)
			{
				aReanimType = i;
				break;
			}
		}
		theContext.SyncInt32(aReanimType);
	}
}

static void SyncParticleDefPortable(PortableSaveContext& theContext, PvzpParticleDefinition*& theDefinition)
{
	if (theContext.mReading)
	{
		int aParticleType = 0;
		theContext.SyncInt32(aParticleType);
		if (aParticleType == static_cast<int>(ParticleEffect::PARTICLE_NONE))
		{
			theDefinition = nullptr;
		}
		else if (aParticleType >= 0 && aParticleType < static_cast<int>(ParticleEffect::NUM_PARTICLES))
		{
			theDefinition = &gParticleDefArray[aParticleType];
		}
		else
		{
			theContext.mFailed = true;
		}
	}
	else
	{
		int aParticleType = static_cast<int>(ParticleEffect::PARTICLE_NONE);
		for (int i = 0; i < static_cast<int>(ParticleEffect::NUM_PARTICLES); i++)
		{
			PvzpParticleDefinition* aDef = &gParticleDefArray[i];
			if (theDefinition == aDef)
			{
				aParticleType = i;
				break;
			}
		}
		theContext.SyncInt32(aParticleType);
	}
}

static void SyncTrailDefPortable(PortableSaveContext& theContext, TrailDefinition*& theDefinition)
{
	if (theContext.mReading)
	{
		int aTrailType = 0;
		theContext.SyncInt32(aTrailType);
		if (aTrailType == TrailType::TRAIL_NONE)
		{
			theDefinition = nullptr;
		}
		else if (aTrailType >= 0 && aTrailType < TrailType::NUM_TRAILS)
		{
			theDefinition = &gTrailDefArray[aTrailType];
		}
		else
		{
			theContext.mFailed = true;
		}
	}
	else
	{
		int aTrailType = TrailType::TRAIL_NONE;
		for (int i = 0; i < TrailType::NUM_TRAILS; i++)
		{
			TrailDefinition* aDef = &gTrailDefArray[i];
			if (theDefinition == aDef)
			{
				aTrailType = i;
				break;
			}
		}
		theContext.SyncInt32(aTrailType);
	}
}

static void SyncImagePortable(PortableSaveContext& theContext, Image*& theImage)
{
	if (theContext.mReading)
	{
		ResourceId aResID;
		theContext.SyncInt32(reinterpret_cast<int32_t&>(aResID));
		if (aResID == Sexy::ResourceId::RESOURCE_ID_MAX)
		{
			theImage = nullptr;
		}
		else
		{
			theImage = GetImageById(aResID);
		}
	}
	else
	{
		ResourceId aResID;
		if (theImage != nullptr)
		{
			aResID = GetIdByImage(theImage);
		}
		else
		{
			aResID = Sexy::ResourceId::RESOURCE_ID_MAX;
		}
		theContext.SyncInt32(reinterpret_cast<int32_t&>(aResID));
	}
}

static void SyncDataIDListPortable(PvzpList<uint32_t>* theDataIDList, PortableSaveContext& theContext, PvzpAllocator* theAllocator)
{
	try
	{
		if (theContext.mReading)
		{
			if (theDataIDList)
			{
				theDataIDList->mHead = nullptr;
				theDataIDList->mTail = nullptr;
				theDataIDList->mSize = 0;
				theDataIDList->SetAllocator(theAllocator);
			}

			int aCount = 0;
			theContext.SyncInt32(aCount);
			for (int i = 0; i < aCount; i++)
			{
				uint32_t aDataID = 0;
				theContext.SyncUInt32(aDataID);
				theDataIDList->AddTail(aDataID);
			}
		}
		else
		{
			int aCount = theDataIDList->mSize;
			theContext.SyncInt32(aCount);
			for (PvzpListNode<uint32_t>* aNode = theDataIDList->mHead; aNode != nullptr; aNode = aNode->mNext)
			{
				uint32_t aDataID = aNode->mValue;
				theContext.SyncUInt32(aDataID);
			}
		}
	}
	catch (std::exception&)
	{
		return;
	}
}

static void SyncGameObjectPortable(PortableSaveContext& theContext, GameObject& theObject)
{
	theContext.SyncInt32(theObject.mX);
	theContext.SyncInt32(theObject.mY);
	theContext.SyncInt32(theObject.mWidth);
	theContext.SyncInt32(theObject.mHeight);
	theContext.SyncBool(theObject.mVisible);
	theContext.SyncInt32(theObject.mRow);
	theContext.SyncInt32(theObject.mRenderOrder);
}

static constexpr const uint32_t PORTABLE_FIELD_TAIL = 100U;

template <typename TEnum>
static void SyncEnum32(PortableSaveContext& theContext, TEnum& theValue)
{
	int32_t aValue = static_cast<int32_t>(theValue);
	theContext.SyncInt32(aValue);
	if (theContext.mReading)
		theValue = static_cast<TEnum>(aValue);
}

template <typename TEnum>
static void SyncEnumU32(PortableSaveContext& theContext, TEnum& theValue)
{
	uint32_t aValue = static_cast<uint32_t>(theValue);
	theContext.SyncUInt32(aValue);
	if (theContext.mReading)
		theValue = static_cast<TEnum>(aValue);
}

template <typename TEnum>
static void SyncEnum32Array(PortableSaveContext& theContext, TEnum* theData, size_t theCount)
{
	for (size_t i = 0; i < theCount; i++)
		SyncEnum32(theContext, theData[i]);
}

template <typename TEnum>
static void SyncEnumU32Array(PortableSaveContext& theContext, TEnum* theData, size_t theCount)
{
	for (size_t i = 0; i < theCount; i++)
		SyncEnumU32(theContext, theData[i]);
}

static void SyncInt32Array(PortableSaveContext& theContext, int32_t* theData, size_t theCount)
{
	for (size_t i = 0; i < theCount; i++)
		theContext.SyncInt32(theData[i]);
}

static void SyncBoolArray(PortableSaveContext& theContext, bool* theData, size_t theCount)
{
	for (size_t i = 0; i < theCount; i++)
		theContext.SyncBool(theData[i]);
}

static void SyncPvzpSmoothArray(PortableSaveContext& theContext, PvzpSmoothArray& theArray)
{
	theContext.SyncInt32(theArray.mItem);
	theContext.SyncFloat(theArray.mWeight);
	theContext.SyncFloat(theArray.mLastPicked);
	theContext.SyncFloat(theArray.mSecondLastPicked);
}

static void SyncPvzpSmoothArrayList(PortableSaveContext& theContext, PvzpSmoothArray* theData, size_t theCount)
{
	for (size_t i = 0; i < theCount; i++)
		SyncPvzpSmoothArray(theContext, theData[i]);
}

static void SyncPottedPlantPortable(PortableSaveContext& theContext, PottedPlant& thePlant)
{
	SyncEnum32(theContext, thePlant.mSeedType);
	SyncEnum32(theContext, thePlant.mWhichZenGarden);
	theContext.SyncInt32(thePlant.mX);
	theContext.SyncInt32(thePlant.mY);
	SyncEnum32(theContext, thePlant.mFacing);
	theContext.SyncInt64(thePlant.mLastWateredTime);
	SyncEnum32(theContext, thePlant.mDrawVariation);
	SyncEnum32(theContext, thePlant.mPlantAge);
	theContext.SyncInt32(thePlant.mTimesFed);
	theContext.SyncInt32(thePlant.mFeedingsPerGrow);
	SyncEnum32(theContext, thePlant.mPlantNeed);
	theContext.SyncInt64(thePlant.mLastNeedFulfilledTime);
	theContext.SyncInt64(thePlant.mLastFertilizedTime);
	theContext.SyncInt64(thePlant.mLastChocolateTime);
	theContext.SyncInt64(thePlant.mFutureAttribute[0]);
}

static void SyncMotionTrailFramePortable(PortableSaveContext& theContext, MotionTrailFrame& theFrame)
{
	theContext.SyncFloat(theFrame.mPosX);
	theContext.SyncFloat(theFrame.mPosY);
	theContext.SyncFloat(theFrame.mAnimTime);
}

static void SyncMagnetItemPortable(PortableSaveContext& theContext, MagnetItem& theItem)
{
	theContext.SyncFloat(theItem.mPosX);
	theContext.SyncFloat(theItem.mPosY);
	theContext.SyncFloat(theItem.mDestOffsetX);
	theContext.SyncFloat(theItem.mDestOffsetY);
	SyncEnum32(theContext, theItem.mItemType);
}

static void SyncAttachEffectPortable(PortableSaveContext& theContext, AttachEffect& theEffect)
{
	theContext.SyncUInt32(theEffect.mEffectID);
	SyncEnum32(theContext, theEffect.mEffectType);
	SyncMatrixPortable(theContext, theEffect.mOffset);
	theContext.SyncBool(theEffect.mDontDrawIfParentHidden);
	theContext.SyncBool(theEffect.mDontPropogateColor);
}

static void SyncAttachmentTailPortable(PortableSaveContext& theContext, Attachment& theAttachment)
{
	for (int i = 0; i < MAX_EFFECTS_PER_ATTACHMENT; i++)
		SyncAttachEffectPortable(theContext, theAttachment.mEffectArray[i]);
	theContext.SyncInt32(theAttachment.mNumEffects);
	theContext.SyncBool(theAttachment.mDead);
}

static void SyncCursorObjectTailPortable(PortableSaveContext& theContext, CursorObject& theObject)
{
	theContext.SyncInt32(theObject.mSeedBankIndex);
	SyncEnum32(theContext, theObject.mType);
	SyncEnum32(theContext, theObject.mImitaterType);
	SyncEnum32(theContext, theObject.mCursorType);
	SyncEnumU32(theContext, theObject.mCoinID);
	SyncEnumU32(theContext, theObject.mGlovePlantID);
	SyncEnumU32(theContext, theObject.mDuplicatorPlantID);
	SyncEnumU32(theContext, theObject.mCobCannonPlantID);
	theContext.SyncInt32(theObject.mHammerDownCounter);
	SyncEnumU32(theContext, theObject.mReanimCursorID);
}

static void SyncCursorPreviewTailPortable(PortableSaveContext& theContext, CursorPreview& thePreview)
{
	theContext.SyncInt32(thePreview.mGridX);
	theContext.SyncInt32(thePreview.mGridY);
}

static void SyncMessageWidgetTailPortable(PortableSaveContext& theContext, MessageWidget& theWidget)
{
	theContext.SyncBytes(theWidget.mLabel, sizeof(theWidget.mLabel));
	theContext.SyncInt32(theWidget.mDisplayTime);
	theContext.SyncInt32(theWidget.mDuration);
	SyncEnum32(theContext, theWidget.mMessageStyle);
	SyncEnumU32Array(theContext, &theWidget.mTextReanimID[0], MAX_MESSAGE_LENGTH);
	SyncEnum32(theContext, theWidget.mReanimType);
	theContext.SyncInt32(theWidget.mSlideOffTime);
	theContext.SyncBytes(theWidget.mLabelNext, sizeof(theWidget.mLabelNext));
	SyncEnum32(theContext, theWidget.mMessageStyleNext);
}

static void SyncSeedBankTailPortable(PortableSaveContext& theContext, SeedBank& theSeedBank)
{
	theContext.SyncInt32(theSeedBank.mNumPackets);
	theContext.SyncInt32(theSeedBank.mCutSceneDarken);
	theContext.SyncInt32(theSeedBank.mConveyorBeltCounter);
}

static void SyncSeedPacketTailPortable(PortableSaveContext& theContext, SeedPacket& thePacket)
{
	theContext.SyncInt32(thePacket.mRefreshCounter);
	theContext.SyncInt32(thePacket.mRefreshTime);
	theContext.SyncInt32(thePacket.mIndex);
	theContext.SyncInt32(thePacket.mOffsetX);
	SyncEnum32(theContext, thePacket.mPacketType);
	SyncEnum32(theContext, thePacket.mImitaterType);
	theContext.SyncInt32(thePacket.mSlotMachineCountDown);
	SyncEnum32(theContext, thePacket.mSlotMachiningNextSeed);
	theContext.SyncFloat(thePacket.mSlotMachiningPosition);
	theContext.SyncBool(thePacket.mActive);
	theContext.SyncBool(thePacket.mRefreshing);
	theContext.SyncInt32(thePacket.mTimesUsed);
}

static void SyncChallengeTailPortable(PortableSaveContext& theContext, Challenge& theChallenge)
{
	theContext.SyncInt32(theChallenge.mBeghouledMouseCapture);
	theContext.SyncInt32(theChallenge.mBeghouledMouseDownX);
	theContext.SyncInt32(theChallenge.mBeghouledMouseDownY);
	SyncInt32Array(theContext, &theChallenge.mBeghouledEated[0][0], 9 * 6);
	SyncInt32Array(theContext, &theChallenge.mBeghouledPurcasedUpgrade[0], NUM_BEGHOULED_UPGRADES);
	theContext.SyncInt32(theChallenge.mBeghouledMatchesThisMove);
	SyncEnum32(theContext, theChallenge.mChallengeState);
	theContext.SyncInt32(theChallenge.mChallengeStateCounter);
	theContext.SyncInt32(theChallenge.mConveyorBeltCounter);
	theContext.SyncInt32(theChallenge.mChallengeScore);
	theContext.SyncInt32(theChallenge.mShowBowlingLine);
	SyncEnum32(theContext, theChallenge.mLastConveyorSeedType);
	theContext.SyncInt32(theChallenge.mSurvivalStage);
	theContext.SyncInt32(theChallenge.mSlotMachineRollCount);
	SyncEnumU32(theContext, theChallenge.mReanimChallenge);
	SyncEnumU32Array(theContext, &theChallenge.mReanimClouds[0], 6);
	SyncInt32Array(theContext, &theChallenge.mCloudsCounter[0], 6);
	theContext.SyncInt32(theChallenge.mChallengeGridX);
	theContext.SyncInt32(theChallenge.mChallengeGridY);
	theContext.SyncInt32(theChallenge.mScaryPotterPots);
	theContext.SyncInt32(theChallenge.mRainCounter);
	theContext.SyncInt32(theChallenge.mTreeOfWisdomTalkIndex);
}

static void SyncMusicTailPortable(PortableSaveContext& theContext, Music& theMusic)
{
	SyncEnum32(theContext, theMusic.mCurMusicTune);
	SyncEnum32(theContext, theMusic.mCurMusicFileMain);
	SyncEnum32(theContext, theMusic.mCurMusicFileDrums);
	SyncEnum32(theContext, theMusic.mCurMusicFileHihats);
	theContext.SyncInt32(theMusic.mBurstOverride);
	theContext.SyncFloat(theMusic.mBaseBPM);
	theContext.SyncFloat(theMusic.mBaseModSpeed);
	SyncEnum32(theContext, theMusic.mMusicBurstState);
	theContext.SyncInt32(theMusic.mBurstStateCounter);
	SyncEnum32(theContext, theMusic.mMusicDrumsState);
	theContext.SyncInt32(theMusic.mQueuedDrumTrackPackedOrder);
	theContext.SyncInt32(theMusic.mDrumsStateCounter);
	theContext.SyncInt32(theMusic.mPauseOffset);
	theContext.SyncInt32(theMusic.mPauseOffsetDrums);
	theContext.SyncBool(theMusic.mPaused);
	// When loading, do not override a runtime music-disable flag that may have been set
	// because this platform don't have audio support; keep it if already true.
	if (theContext.mReading)
	{
		bool aSavedMusicDisabled = false;
		theContext.SyncBool(aSavedMusicDisabled); // Just read and discard
		// Completely ignore the saved value. mMusicDisabled is a runtime capability flag
		// (set when audio assets fail to load). It should never be transferred from a save.
	}
	else
	{
		theContext.SyncBool(theMusic.mMusicDisabled);
	}
	theContext.SyncInt32(theMusic.mFadeOutCounter);
	theContext.SyncInt32(theMusic.mFadeOutDuration);
}

static void SyncZombieTailPortable(PortableSaveContext& theContext, Zombie& theZombie)
{
	SyncEnum32(theContext, theZombie.mZombieType);
	SyncEnum32(theContext, theZombie.mZombiePhase);
	theContext.SyncFloat(theZombie.mPosX);
	theContext.SyncFloat(theZombie.mPosY);
	theContext.SyncFloat(theZombie.mVelX);
	theContext.SyncInt32(theZombie.mAnimCounter);
	theContext.SyncInt32(theZombie.mGroanCounter);
	theContext.SyncInt32(theZombie.mAnimTicksPerFrame);
	theContext.SyncInt32(theZombie.mAnimFrames);
	theContext.SyncInt32(theZombie.mFrame);
	theContext.SyncInt32(theZombie.mPrevFrame);
	theContext.SyncBool(theZombie.mVariant);
	theContext.SyncBool(theZombie.mIsEating);
	theContext.SyncInt32(theZombie.mJustGotShotCounter);
	theContext.SyncInt32(theZombie.mShieldJustGotShotCounter);
	theContext.SyncInt32(theZombie.mShieldRecoilCounter);
	theContext.SyncInt32(theZombie.mZombieAge);
	SyncEnum32(theContext, theZombie.mZombieHeight);
	theContext.SyncInt32(theZombie.mPhaseCounter);
	theContext.SyncInt32(theZombie.mFromWave);
	theContext.SyncBool(theZombie.mDroppedLoot);
	theContext.SyncInt32(theZombie.mZombieFade);
	theContext.SyncBool(theZombie.mFlatTires);
	theContext.SyncInt32(theZombie.mUseLadderCol);
	theContext.SyncInt32(theZombie.mTargetCol);
	theContext.SyncFloat(theZombie.mAltitude);
	theContext.SyncBool(theZombie.mHitUmbrella);
	SyncRectPortable(theContext, theZombie.mZombieRect);
	SyncRectPortable(theContext, theZombie.mZombieAttackRect);
	theContext.SyncInt32(theZombie.mChilledCounter);
	theContext.SyncInt32(theZombie.mButteredCounter);
	theContext.SyncInt32(theZombie.mIceTrapCounter);
	theContext.SyncBool(theZombie.mMindControlled);
	theContext.SyncBool(theZombie.mBlowingAway);
	theContext.SyncBool(theZombie.mHasHead);
	theContext.SyncBool(theZombie.mHasArm);
	theContext.SyncBool(theZombie.mHasObject);
	theContext.SyncBool(theZombie.mInPool);
	theContext.SyncBool(theZombie.mOnHighGround);
	theContext.SyncBool(theZombie.mYuckyFace);
	theContext.SyncInt32(theZombie.mYuckyFaceCounter);
	SyncEnum32(theContext, theZombie.mHelmType);
	theContext.SyncInt32(theZombie.mBodyHealth);
	theContext.SyncInt32(theZombie.mBodyMaxHealth);
	theContext.SyncInt32(theZombie.mHelmHealth);
	theContext.SyncInt32(theZombie.mHelmMaxHealth);
	SyncEnum32(theContext, theZombie.mShieldType);
	theContext.SyncInt32(theZombie.mShieldHealth);
	theContext.SyncInt32(theZombie.mShieldMaxHealth);
	theContext.SyncInt32(theZombie.mFlyingHealth);
	theContext.SyncInt32(theZombie.mFlyingMaxHealth);
	theContext.SyncBool(theZombie.mDead);
	SyncEnumU32(theContext, theZombie.mRelatedZombieID);
	SyncEnumU32Array(theContext, &theZombie.mFollowerZombieID[0], MAX_ZOMBIE_FOLLOWERS);
	theContext.SyncBool(theZombie.mPlayingSong);
	theContext.SyncInt32(theZombie.mParticleOffsetX);
	theContext.SyncInt32(theZombie.mParticleOffsetY);
	SyncEnum32(theContext, theZombie.mAttachmentID);
	theContext.SyncInt32(theZombie.mSummonCounter);
	SyncEnumU32(theContext, theZombie.mBodyReanimID);
	theContext.SyncFloat(theZombie.mScaleZombie);
	theContext.SyncFloat(theZombie.mVelZ);
	theContext.SyncFloat(theZombie.mOriginalAnimRate);
	SyncEnumU32(theContext, theZombie.mTargetPlantID);
	theContext.SyncInt32(theZombie.mBossMode);
	theContext.SyncInt32(theZombie.mTargetRow);
	theContext.SyncInt32(theZombie.mBossBungeeCounter);
	theContext.SyncInt32(theZombie.mBossStompCounter);
	theContext.SyncInt32(theZombie.mBossHeadCounter);
	SyncEnumU32(theContext, theZombie.mBossFireBallReanimID);
	SyncEnumU32(theContext, theZombie.mSpecialHeadReanimID);
	theContext.SyncInt32(theZombie.mFireballRow);
	theContext.SyncBool(theZombie.mIsFireBall);
	SyncEnumU32(theContext, theZombie.mMoweredReanimID);
	theContext.SyncInt32(theZombie.mLastPortalX);
	SyncEnumU32(theContext, theZombie.mZombatarHeadReanimID);
}

static void SyncPlantTailPortable(PortableSaveContext& theContext, Plant& thePlant)
{
	SyncEnum32(theContext, thePlant.mSeedType);
	theContext.SyncInt32(thePlant.mPlantCol);
	theContext.SyncInt32(thePlant.mAnimCounter);
	theContext.SyncInt32(thePlant.mFrame);
	theContext.SyncInt32(thePlant.mFrameLength);
	theContext.SyncInt32(thePlant.mNumFrames);
	SyncEnum32(theContext, thePlant.mState);
	theContext.SyncInt32(thePlant.mPlantHealth);
	theContext.SyncInt32(thePlant.mPlantMaxHealth);
	theContext.SyncInt32(thePlant.mSubclass);
	theContext.SyncInt32(thePlant.mDisappearCountdown);
	theContext.SyncInt32(thePlant.mDoSpecialCountdown);
	theContext.SyncInt32(thePlant.mStateCountdown);
	theContext.SyncInt32(thePlant.mLaunchCounter);
	theContext.SyncInt32(thePlant.mLaunchRate);
	SyncRectPortable(theContext, thePlant.mPlantRect);
	SyncRectPortable(theContext, thePlant.mPlantAttackRect);
	theContext.SyncInt32(thePlant.mTargetX);
	theContext.SyncInt32(thePlant.mTargetY);
	theContext.SyncInt32(thePlant.mStartRow);
	SyncEnumU32(theContext, thePlant.mParticleID);
	theContext.SyncInt32(thePlant.mShootingCounter);
	SyncEnumU32(theContext, thePlant.mBodyReanimID);
	SyncEnumU32(theContext, thePlant.mHeadReanimID);
	SyncEnumU32(theContext, thePlant.mHeadReanimID2);
	SyncEnumU32(theContext, thePlant.mHeadReanimID3);
	SyncEnumU32(theContext, thePlant.mBlinkReanimID);
	SyncEnumU32(theContext, thePlant.mLightReanimID);
	SyncEnumU32(theContext, thePlant.mSleepingReanimID);
	theContext.SyncInt32(thePlant.mBlinkCountdown);
	theContext.SyncInt32(thePlant.mRecentlyEatenCountdown);
	theContext.SyncInt32(thePlant.mEatenFlashCountdown);
	theContext.SyncInt32(thePlant.mBeghouledFlashCountdown);
	theContext.SyncFloat(thePlant.mShakeOffsetX);
	theContext.SyncFloat(thePlant.mShakeOffsetY);
	for (int i = 0; i < MAX_MAGNET_ITEMS; i++)
		SyncMagnetItemPortable(theContext, thePlant.mMagnetItems[i]);
	SyncEnumU32(theContext, thePlant.mTargetZombieID);
	theContext.SyncInt32(thePlant.mWakeUpCounter);
	SyncEnum32(theContext, thePlant.mOnBungeeState);
	SyncEnum32(theContext, thePlant.mImitaterType);
	theContext.SyncInt32(thePlant.mPottedPlantIndex);
	theContext.SyncBool(thePlant.mAnimPing);
	theContext.SyncBool(thePlant.mDead);
	theContext.SyncBool(thePlant.mSquished);
	theContext.SyncBool(thePlant.mIsAsleep);
	theContext.SyncBool(thePlant.mIsOnBoard);
	theContext.SyncBool(thePlant.mHighlighted);
}

static void SyncProjectileTailPortable(PortableSaveContext& theContext, Projectile& theProjectile)
{
	theContext.SyncInt32(theProjectile.mFrame);
	theContext.SyncInt32(theProjectile.mNumFrames);
	theContext.SyncInt32(theProjectile.mAnimCounter);
	theContext.SyncFloat(theProjectile.mPosX);
	theContext.SyncFloat(theProjectile.mPosY);
	theContext.SyncFloat(theProjectile.mPosZ);
	theContext.SyncFloat(theProjectile.mVelX);
	theContext.SyncFloat(theProjectile.mVelY);
	theContext.SyncFloat(theProjectile.mVelZ);
	theContext.SyncFloat(theProjectile.mAccZ);
	theContext.SyncFloat(theProjectile.mShadowY);
	theContext.SyncBool(theProjectile.mDead);
	theContext.SyncInt32(theProjectile.mAnimTicksPerFrame);
	SyncEnum32(theContext, theProjectile.mMotionType);
	SyncEnum32(theContext, theProjectile.mProjectileType);
	theContext.SyncInt32(theProjectile.mProjectileAge);
	theContext.SyncInt32(theProjectile.mClickBackoffCounter);
	theContext.SyncFloat(theProjectile.mRotation);
	theContext.SyncFloat(theProjectile.mRotationSpeed);
	theContext.SyncBool(theProjectile.mOnHighGround);
	theContext.SyncInt32(theProjectile.mDamageRangeFlags);
	theContext.SyncInt32(theProjectile.mHitTorchwoodGridX);
	SyncEnum32(theContext, theProjectile.mAttachmentID);
	theContext.SyncFloat(theProjectile.mCobTargetX);
	theContext.SyncInt32(theProjectile.mCobTargetRow);
	SyncEnumU32(theContext, theProjectile.mTargetZombieID);
	theContext.SyncInt32(theProjectile.mLastPortalX);
}

static void SyncCoinTailPortable(PortableSaveContext& theContext, Coin& theCoin)
{
	theContext.SyncFloat(theCoin.mPosX);
	theContext.SyncFloat(theCoin.mPosY);
	theContext.SyncFloat(theCoin.mVelX);
	theContext.SyncFloat(theCoin.mVelY);
	theContext.SyncFloat(theCoin.mScale);
	theContext.SyncBool(theCoin.mDead);
	theContext.SyncInt32(theCoin.mFadeCount);
	theContext.SyncFloat(theCoin.mCollectX);
	theContext.SyncFloat(theCoin.mCollectY);
	theContext.SyncInt32(theCoin.mGroundY);
	theContext.SyncInt32(theCoin.mCoinAge);
	theContext.SyncBool(theCoin.mIsBeingCollected);
	theContext.SyncInt32(theCoin.mDisappearCounter);
	SyncEnum32(theContext, theCoin.mType);
	SyncEnum32(theContext, theCoin.mCoinMotion);
	SyncEnum32(theContext, theCoin.mAttachmentID);
	theContext.SyncFloat(theCoin.mCollectionDistance);
	SyncEnum32(theContext, theCoin.mUsableSeedType);
	SyncPottedPlantPortable(theContext, theCoin.mPottedPlantSpec);
	theContext.SyncBool(theCoin.mNeedsBouncyArrow);
	theContext.SyncBool(theCoin.mHasBouncyArrow);
	theContext.SyncBool(theCoin.mHitGround);
	theContext.SyncInt32(theCoin.mTimesDropped);
}

static void SyncLawnMowerTailPortable(PortableSaveContext& theContext, LawnMower& theMower)
{
	theContext.SyncFloat(theMower.mPosX);
	theContext.SyncFloat(theMower.mPosY);
	theContext.SyncInt32(theMower.mRenderOrder);
	theContext.SyncInt32(theMower.mRow);
	theContext.SyncInt32(theMower.mAnimTicksPerFrame);
	SyncEnumU32(theContext, theMower.mReanimID);
	theContext.SyncInt32(theMower.mChompCounter);
	theContext.SyncInt32(theMower.mRollingInCounter);
	theContext.SyncInt32(theMower.mSquishedCounter);
	SyncEnum32(theContext, theMower.mMowerState);
	theContext.SyncBool(theMower.mDead);
	theContext.SyncBool(theMower.mVisible);
	SyncEnum32(theContext, theMower.mMowerType);
	theContext.SyncFloat(theMower.mAltitude);
	SyncEnum32(theContext, theMower.mMowerHeight);
	theContext.SyncInt32(theMower.mLastPortalX);
}

static void SyncGridItemTailPortable(PortableSaveContext& theContext, GridItem& theItem)
{
	SyncEnum32(theContext, theItem.mGridItemType);
	SyncEnum32(theContext, theItem.mGridItemState);
	theContext.SyncInt32(theItem.mGridX);
	theContext.SyncInt32(theItem.mGridY);
	theContext.SyncInt32(theItem.mGridItemCounter);
	theContext.SyncInt32(theItem.mRenderOrder);
	theContext.SyncBool(theItem.mDead);
	theContext.SyncFloat(theItem.mPosX);
	theContext.SyncFloat(theItem.mPosY);
	theContext.SyncFloat(theItem.mGoalX);
	theContext.SyncFloat(theItem.mGoalY);
	SyncEnumU32(theContext, theItem.mGridItemReanimID);
	SyncEnumU32(theContext, theItem.mGridItemParticleID);
	SyncEnum32(theContext, theItem.mZombieType);
	SyncEnum32(theContext, theItem.mSeedType);
	SyncEnum32(theContext, theItem.mScaryPotType);
	theContext.SyncBool(theItem.mHighlighted);
	theContext.SyncInt32(theItem.mTransparentCounter);
	theContext.SyncInt32(theItem.mSunCount);
	for (int i = 0; i < NUM_MOTION_TRAIL_FRAMES; i++)
		SyncMotionTrailFramePortable(theContext, theItem.mMotionTrailFrames[i]);
	theContext.SyncInt32(theItem.mMotionTrailCount);
}

template <typename TWriterFn>
static void AppendFieldWithSync(std::vector<unsigned char>& theOut, uint32_t theFieldId, TWriterFn theWriterFn)
{
	DataWriter aWriter;
	aWriter.OpenMemory(0x100);
	PortableSaveContext aContext(aWriter);
	theWriterFn(aContext);
	if (aContext.mFailed)
		return;

	std::vector<unsigned char> aFieldData;
	aFieldData.resize(aWriter.GetDataLen());
	memcpy(aFieldData.data(), aWriter.GetDataPtr(), aWriter.GetDataLen());
	AppendU32LE(theOut, theFieldId);
	AppendU32LE(theOut, static_cast<uint32_t>(aFieldData.size()));
	AppendBytes(theOut, aFieldData.data(), aFieldData.size());
}

template <typename TReaderFn>
static bool ApplyFieldWithSync(const unsigned char* theData, size_t theSize, TReaderFn theReaderFn)
{
	DataReader aReader;
	aReader.OpenMemory(theData, static_cast<uint32_t>(theSize), false);
	PortableSaveContext aContext(aReader);
	theReaderFn(aContext);
	return !aContext.mFailed;
}

static void WriteGameObjectField(std::vector<unsigned char>& theOut, uint32_t theFieldId, GameObject& theObject)
{
	AppendFieldWithSync(theOut, theFieldId, [&](PortableSaveContext& aContext)
	{
		SyncGameObjectPortable(aContext, theObject);
	});
}

static bool ReadGameObjectField(const unsigned char* theData, size_t theSize, GameObject& theObject)
{
	return ApplyFieldWithSync(theData, theSize, [&](PortableSaveContext& aContext)
	{
		SyncGameObjectPortable(aContext, theObject);
	});
}

static void WriteTLVBlob(PortableSaveContext& theContext, const std::vector<unsigned char>& theBlob)
{
	uint32_t aSize = static_cast<uint32_t>(theBlob.size());
	theContext.SyncUInt32(aSize);
	if (aSize > 0)
		theContext.SyncBytes(theBlob.data(), aSize);
}

static bool ReadTLVBlob(PortableSaveContext& theContext, std::vector<unsigned char>& theBlob)
{
	uint32_t aSize = 0;
	theContext.SyncUInt32(aSize);
	if (theContext.mFailed)
		return false;
	theBlob.resize(aSize);
	if (aSize > 0)
		theContext.SyncBytes(theBlob.data(), aSize);
	return !theContext.mFailed;
}

// Syncs a single object as a TLV blob: GameObject field (1U) only when TObject derives from GameObject, plus the tail field.
template <typename TObject, typename TTailSync>
static void SyncSingleObjectTLV(PortableSaveContext& theContext, TObject& theObject, TTailSync theTailSync)
{
	static constexpr bool HAS_GAME_OBJECT_FIELD = std::is_base_of_v<GameObject, TObject>;
	if (theContext.mReading)
	{
		std::vector<unsigned char> aBlob;
		if (!ReadTLVBlob(theContext, aBlob))
			return;
		TLVReader aReader(aBlob.data(), aBlob.size());
		while (aReader.mOk && aReader.mPos < aReader.mSize)
		{
			uint32_t aFieldId = 0;
			uint32_t aFieldSize = 0;
			if (!aReader.ReadU32(aFieldId) || !aReader.ReadU32(aFieldSize))
				break;
			const unsigned char* aFieldData = nullptr;
			if (!aReader.ReadBytes(aFieldData, aFieldSize))
				break;
			switch (aFieldId)
			{
			case 1U:
				if constexpr (HAS_GAME_OBJECT_FIELD)
					ReadGameObjectField(aFieldData, aFieldSize, theObject);
				break;
			case PORTABLE_FIELD_TAIL:
				ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ theTailSync(c, theObject); });
				break;
			default: break;
			}
		}
	}
	else
	{
		std::vector<unsigned char> aBlob;
		if constexpr (HAS_GAME_OBJECT_FIELD)
			WriteGameObjectField(aBlob, 1U, theObject);
		AppendFieldWithSync(aBlob, PORTABLE_FIELD_TAIL, [&](PortableSaveContext& c){ theTailSync(c, theObject); });
		WriteTLVBlob(theContext, aBlob);
	}
}

static void SyncReanimTransformPortable(PortableSaveContext& theContext, ReanimatorTransform& theTransform)
{
	theContext.SyncFloat(theTransform.mTransX);
	theContext.SyncFloat(theTransform.mTransY);
	theContext.SyncFloat(theTransform.mSkewX);
	theContext.SyncFloat(theTransform.mSkewY);
	theContext.SyncFloat(theTransform.mScaleX);
	theContext.SyncFloat(theTransform.mScaleY);
	theContext.SyncFloat(theTransform.mFrame);
	theContext.SyncFloat(theTransform.mAlpha);
	if (theContext.mReading)
	{
		theTransform.mImage = nullptr;
		theTransform.mFont = nullptr;
		theTransform.mText = "";
	}
}

static void SyncReanimTrackInstancePortable(PortableSaveContext& theContext, ReanimatorTrackInstance& theTrackInstance)
{
	theContext.SyncInt32(theTrackInstance.mBlendCounter);
	theContext.SyncInt32(theTrackInstance.mBlendTime);
	SyncReanimTransformPortable(theContext, theTrackInstance.mBlendTransform);
	theContext.SyncFloat(theTrackInstance.mShakeOverride);
	theContext.SyncFloat(theTrackInstance.mShakeX);
	theContext.SyncFloat(theTrackInstance.mShakeY);
	theContext.SyncInt32(reinterpret_cast<int32_t&>(theTrackInstance.mAttachmentID));
	SyncImagePortable(theContext, theTrackInstance.mImageOverride);
	theContext.SyncInt32(theTrackInstance.mRenderGroup);
	SyncColorPortable(theContext, theTrackInstance.mTrackColor);
	theContext.SyncBool(theTrackInstance.mIgnoreClipRect);
	theContext.SyncBool(theTrackInstance.mTruncateDisappearingFrames);
	theContext.SyncBool(theTrackInstance.mIgnoreColorOverride);
	theContext.SyncBool(theTrackInstance.mIgnoreExtraAdditiveColor);
}

static void SyncReanimationPortable(Board* theBoard, Reanimation* theReanimation, PortableSaveContext& theContext)
{
	SyncReanimationDefPortable(theContext, theReanimation->mDefinition);
	if (theContext.mReading)
	{
		theReanimation->mReanimationHolder = theBoard->mApp->mEffectSystem->mReanimationHolder.get();
	}

	ReanimatorDefinition* aDef = theReanimation->mDefinition;
	ReanimatorDefinition* aDefStart = gReanimatorDefArray.get();
	ReanimatorDefinition* aDefEnd = gReanimatorDefArray.get() + static_cast<int>(ReanimationType::NUM_REANIMS);
	if (aDef == nullptr || aDef < aDefStart || aDef >= aDefEnd)
	{
		int aType = static_cast<int>(theReanimation->mReanimationType);
		if (aType >= 0 && aType < static_cast<int>(ReanimationType::NUM_REANIMS))
		{
			ReanimatorEnsureDefinitionLoaded(static_cast<ReanimationType>(aType), true);
			aDef = &gReanimatorDefArray[aType];
			if (theContext.mReading)
				theReanimation->mDefinition = aDef;
		}
		else
		{
			aDef = nullptr;
		}
	}

	theContext.SyncEnum(theReanimation->mReanimationType);
	theContext.SyncFloat(theReanimation->mAnimTime);
	theContext.SyncFloat(theReanimation->mAnimRate);
	theContext.SyncEnum(theReanimation->mLoopType);
	theContext.SyncBool(theReanimation->mDead);
	theContext.SyncInt32(theReanimation->mFrameStart);
	theContext.SyncInt32(theReanimation->mFrameCount);
	theContext.SyncInt32(theReanimation->mFrameBasePose);
	SyncMatrixPortable(theContext, theReanimation->mOverlayMatrix);
	SyncColorPortable(theContext, theReanimation->mColorOverride);
	theContext.SyncInt32(theReanimation->mLoopCount);
	theContext.SyncBool(theReanimation->mIsAttachment);
	theContext.SyncInt32(theReanimation->mRenderOrder);
	SyncColorPortable(theContext, theReanimation->mExtraAdditiveColor);
	theContext.SyncBool(theReanimation->mEnableExtraAdditiveDraw);
	SyncColorPortable(theContext, theReanimation->mExtraOverlayColor);
	theContext.SyncBool(theReanimation->mEnableExtraOverlayDraw);
	theContext.SyncFloat(theReanimation->mLastFrameTime);
	theContext.SyncEnum(theReanimation->mFilterEffect);

	if (aDef && aDef->mTracks.count != 0)
	{
		int aCount = aDef->mTracks.count;
		bool aUseTemp = (theReanimation->mTrackInstances == nullptr);
		if (theContext.mReading)
		{
			theReanimation->mTrackInstances = reinterpret_cast<ReanimatorTrackInstance*>(
				FindGlobalAllocator(aCount * sizeof(ReanimatorTrackInstance))->Calloc(aCount * sizeof(ReanimatorTrackInstance)));
			if (theReanimation->mTrackInstances == nullptr)
			{
				aUseTemp = true;
			}
			else
			{
				aUseTemp = false;
			}
		}
		else
		{
			PvzpAllocator* aAllocator = FindGlobalAllocator(aCount * sizeof(ReanimatorTrackInstance));
			if (aAllocator == nullptr || !aAllocator->IsPointerFromAllocator(theReanimation->mTrackInstances) || aAllocator->IsPointerOnFreeList(theReanimation->mTrackInstances))
			{
				aUseTemp = true;
			}
		}
		if (aUseTemp)
		{
			std::vector<ReanimatorTrackInstance> aTemp;
			aTemp.resize(aCount);
			memset(aTemp.data(), 0, sizeof(ReanimatorTrackInstance) * aCount);
			for (int aTrackIndex = 0; aTrackIndex < aCount; aTrackIndex++)
			{
				SyncReanimTrackInstancePortable(theContext, aTemp[aTrackIndex]);
			}
		}
		else
		{
			for (int aTrackIndex = 0; aTrackIndex < aCount; aTrackIndex++)
			{
				SyncReanimTrackInstancePortable(theContext, theReanimation->mTrackInstances[aTrackIndex]);
			}
		}
	}
}

static void SyncParticlePortable(PvzpParticle* theParticle, PortableSaveContext& theContext)
{
	theContext.SyncInt32(theParticle->mParticleDuration);
	theContext.SyncInt32(theParticle->mParticleAge);
	theContext.SyncFloat(theParticle->mParticleTimeValue);
	theContext.SyncFloat(theParticle->mParticleLastTimeValue);
	theContext.SyncFloat(theParticle->mAnimationTimeValue);
	SyncVector2Portable(theContext, theParticle->mVelocity);
	SyncVector2Portable(theContext, theParticle->mPosition);
	theContext.SyncInt32(theParticle->mImageFrame);
	theContext.SyncFloat(theParticle->mSpinPosition);
	theContext.SyncFloat(theParticle->mSpinVelocity);
	theContext.SyncInt32(reinterpret_cast<int32_t&>(theParticle->mCrossFadeParticleID));
	theContext.SyncInt32(theParticle->mCrossFadeDuration);
	for (int i = 0; i < ParticleTracks::NUM_PARTICLE_TRACKS; i++)
		theContext.SyncFloat(theParticle->mParticleInterp[i]);
	for (int i = 0; i < MAX_PARTICLE_FIELDS; i++)
	{
		for (int j = 0; j < 2; j++)
			theContext.SyncFloat(theParticle->mParticleFieldInterp[i][j]);
	}
}

static void SyncParticleEmitterPortable(PvzpParticleSystem* theParticleSystem, PvzpParticleEmitter* theParticleEmitter, PortableSaveContext& theContext)
{
	int aEmitterDefIndex = 0;
	if (theContext.mReading)
	{
		theContext.SyncInt32(aEmitterDefIndex);
		theParticleEmitter->mParticleSystem = theParticleSystem;
		theParticleEmitter->mEmitterDef = &theParticleSystem->mParticleDef->mEmitterDefs[aEmitterDefIndex];
	}
	else
	{
		aEmitterDefIndex = (reinterpret_cast<intptr_t>(theParticleEmitter->mEmitterDef) -
			reinterpret_cast<intptr_t>(theParticleSystem->mParticleDef->mEmitterDefs)) / sizeof(PvzpEmitterDefinition);
		theContext.SyncInt32(aEmitterDefIndex);
	}

	SyncDataIDListPortable((PvzpList<uint32_t>*)&theParticleEmitter->mParticleList, theContext, &theParticleSystem->mParticleHolder->mParticleListNodeAllocator);
	SyncVector2Portable(theContext, theParticleEmitter->mSystemCenter);
	SyncColorPortable(theContext, theParticleEmitter->mColorOverride);
	SyncImagePortable(theContext, theParticleEmitter->mImageOverride);
	theContext.SyncFloat(theParticleEmitter->mSpawnAccum);
	theContext.SyncInt32(theParticleEmitter->mParticlesSpawned);
	theContext.SyncInt32(theParticleEmitter->mSystemAge);
	theContext.SyncInt32(theParticleEmitter->mSystemDuration);
	theContext.SyncFloat(theParticleEmitter->mSystemTimeValue);
	theContext.SyncFloat(theParticleEmitter->mSystemLastTimeValue);
	theContext.SyncBool(theParticleEmitter->mDead);
	theContext.SyncBool(theParticleEmitter->mExtraAdditiveDrawOverride);
	theContext.SyncFloat(theParticleEmitter->mScaleOverride);
	theContext.SyncInt32(reinterpret_cast<int32_t&>(theParticleEmitter->mCrossFadeEmitterID));
	theContext.SyncInt32(theParticleEmitter->mEmitterCrossFadeCountDown);
	theContext.SyncInt32(theParticleEmitter->mFrameOverride);
	for (int i = 0; i < ParticleSystemTracks::NUM_SYSTEM_TRACKS; i++)
		theContext.SyncFloat(theParticleEmitter->mTrackInterp[i]);
	for (int i = 0; i < MAX_PARTICLE_FIELDS; i++)
	{
		for (int j = 0; j < 2; j++)
			theContext.SyncFloat(theParticleEmitter->mSystemFieldInterp[i][j]);
	}

	for (PvzpListNode<ParticleID>* aNode = theParticleEmitter->mParticleList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		PvzpParticle* aParticle = theParticleSystem->mParticleHolder->mParticles.DataArrayGet(static_cast<uint32_t>(aNode->mValue));
		if (theContext.mReading)
		{
			aParticle->mParticleEmitter = theParticleEmitter;
		}
		SyncParticlePortable(aParticle, theContext);
	}
}

static void SyncParticleSystemPortable(Board* theBoard, PvzpParticleSystem* theParticleSystem, PortableSaveContext& theContext)
{
	SyncParticleDefPortable(theContext, theParticleSystem->mParticleDef);
	if (theContext.mReading)
	{
		theParticleSystem->mParticleHolder = theBoard->mApp->mEffectSystem->mParticleHolder.get();
	}

	SyncDataIDListPortable((PvzpList<uint32_t>*)&theParticleSystem->mEmitterList, theContext, &theParticleSystem->mParticleHolder->mEmitterListNodeAllocator);
	for (PvzpListNode<ParticleEmitterID>* aNode = theParticleSystem->mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		PvzpParticleEmitter* aEmitter = theParticleSystem->mParticleHolder->mEmitters.DataArrayGet(static_cast<uint32_t>(aNode->mValue));
		SyncParticleEmitterPortable(theParticleSystem, aEmitter, theContext);
	}

	theContext.SyncEnum(theParticleSystem->mEffectType);
	theContext.SyncBool(theParticleSystem->mDead);
	theContext.SyncBool(theParticleSystem->mIsAttachment);
	theContext.SyncInt32(theParticleSystem->mRenderOrder);
	theContext.SyncBool(theParticleSystem->mDontUpdate);
}

static void SyncTrailPortable(Board* theBoard, Trail* theTrail, PortableSaveContext& theContext)
{
	SyncTrailDefPortable(theContext, theTrail->mDefinition);
	if (theContext.mReading)
	{
		theTrail->mTrailHolder = theBoard->mApp->mEffectSystem->mTrailHolder.get();
	}

	for (int i = 0; i < 20; i++)
		SyncVector2Portable(theContext, theTrail->mTrailPoints[i].aPos);
	theContext.SyncInt32(theTrail->mNumTrailPoints);
	theContext.SyncBool(theTrail->mDead);
	theContext.SyncInt32(theTrail->mRenderOrder);
	theContext.SyncInt32(theTrail->mTrailAge);
	theContext.SyncInt32(theTrail->mTrailDuration);
	for (int i = 0; i < 4; i++)
		theContext.SyncFloat(theTrail->mTrailInterp[i]);
	SyncVector2Portable(theContext, theTrail->mTrailCenter);
	theContext.SyncBool(theTrail->mIsAttachment);
	SyncColorPortable(theContext, theTrail->mColorOverride);
}

template <typename T, typename TSyncFn>
static void SyncDataArrayPortable(PortableSaveContext& theContext, DataArray<T>& theDataArray, TSyncFn theSyncFn)
{
	theContext.SyncUInt32(theDataArray.mFreeListHead);
	theContext.SyncUInt32(theDataArray.mMaxUsedCount);
	theContext.SyncUInt32(theDataArray.mSize);
	theContext.SyncUInt32(theDataArray.mNextKey);
	uint32_t aMaxSize = theDataArray.mMaxSize;
	theContext.SyncUInt32(aMaxSize);
	if (theContext.mReading && aMaxSize != theDataArray.mMaxSize)
	{
		theContext.mFailed = true;
	}

	for (uint32_t i = 0; i < theDataArray.mMaxUsedCount; i++)
	{
		theContext.SyncUInt32(theDataArray.DataArrayGetIDAt(i));
		theSyncFn(theDataArray.DataArrayGetItemAt(i));
	}
}

template <typename T>
static void SyncDataArrayIdsOnlyPortable(PortableSaveContext& theContext, DataArray<T>& theDataArray)
{
	theContext.SyncUInt32(theDataArray.mFreeListHead);
	theContext.SyncUInt32(theDataArray.mMaxUsedCount);
	theContext.SyncUInt32(theDataArray.mSize);
	theContext.SyncUInt32(theDataArray.mNextKey);
	uint32_t aMaxSize = theDataArray.mMaxSize;
	theContext.SyncUInt32(aMaxSize);
	if (theContext.mReading && aMaxSize != theDataArray.mMaxSize)
	{
		theContext.mFailed = true;
	}

	for (uint32_t i = 0; i < theDataArray.mMaxUsedCount; i++)
	{
		theContext.SyncUInt32(theDataArray.DataArrayGetIDAt(i));
	}
}

template <typename T, typename TWriteFn, typename TReadFn>
static void SyncDataArrayPortableTLV(PortableSaveContext& theContext, DataArray<T>& theDataArray, TWriteFn theWriteFn, TReadFn theReadFn)
{
	theContext.SyncUInt32(theDataArray.mFreeListHead);
	theContext.SyncUInt32(theDataArray.mMaxUsedCount);
	theContext.SyncUInt32(theDataArray.mSize);
	theContext.SyncUInt32(theDataArray.mNextKey);
	uint32_t aMaxSize = theDataArray.mMaxSize;
	theContext.SyncUInt32(aMaxSize);
	if (theContext.mReading && aMaxSize != theDataArray.mMaxSize)
	{
		theContext.mFailed = true;
	}

	for (uint32_t i = 0; i < theDataArray.mMaxUsedCount; i++)
	{
		theContext.SyncUInt32(theDataArray.DataArrayGetIDAt(i));
		if (theContext.mReading)
		{
			uint32_t aItemSize = 0;
			theContext.SyncUInt32(aItemSize);
			T& anItem = theDataArray.DataArrayResetItemAt(i);
			std::vector<unsigned char> aItemData;
			aItemData.resize(aItemSize);
			if (aItemSize > 0)
				theContext.SyncBytes(aItemData.data(), aItemSize);
			TLVReader aReader(aItemData.data(), aItemSize);
			while (aReader.mOk && aReader.mPos < aReader.mSize)
			{
				uint32_t aFieldId = 0;
				uint32_t aFieldSize = 0;
				if (!aReader.ReadU32(aFieldId) || !aReader.ReadU32(aFieldSize))
					break;
				const unsigned char* aFieldData = nullptr;
				if (!aReader.ReadBytes(aFieldData, aFieldSize))
					break;
				theReadFn(aFieldId, aFieldData, aFieldSize, anItem);
			}
		}
		else
		{
			bool aActive = (theDataArray.DataArrayGetIDAt(i) & DATA_ARRAY_KEY_MASK) != 0;
			uint32_t aItemSize = 0;
			std::vector<unsigned char> aItemData;
			if (aActive)
			{
				theWriteFn(aItemData, theDataArray.DataArrayGetItemAt(i));
				aItemSize = static_cast<uint32_t>(aItemData.size());
			}
			theContext.SyncUInt32(aItemSize);
			if (aItemSize > 0)
				theContext.SyncBytes(aItemData.data(), aItemSize);
		}
	}
}

// Syncs a DataArray of entities: GameObject field (1U) only when T derives from GameObject, plus the tail field.
template <typename T, typename TTailSync>
static void SyncDataArrayObjectsTLV(PortableSaveContext& theContext, DataArray<T>& theDataArray, TTailSync theTailSync)
{
	SyncDataArrayPortableTLV(theContext, theDataArray,
		[&](std::vector<unsigned char>& aOut, T& anItem)
		{
			if constexpr (std::is_base_of_v<GameObject, T>)
				WriteGameObjectField(aOut, 1U, anItem);
			AppendFieldWithSync(aOut, PORTABLE_FIELD_TAIL, [&](PortableSaveContext& c){ theTailSync(c, anItem); });
		},
		[&](uint32_t aFieldId, const unsigned char* aData, size_t aSize, T& anItem)
		{
			switch (aFieldId)
			{
			case 1U:
				if constexpr (std::is_base_of_v<GameObject, T>)
					ReadGameObjectField(aData, aSize, anItem);
				break;
			case PORTABLE_FIELD_TAIL:
				ApplyFieldWithSync(aData, aSize, [&](PortableSaveContext& c){ theTailSync(c, anItem); });
				break;
			default: break;
			}
		});
}

// Field IDs for Board base: These IDs are part of the on-disk format and must NOT be renumbered.
enum BoardBaseFieldId : uint32_t
{
	BOARD_FIELD_PAUSED = 1,
	BOARD_FIELD_GRID_SQUARE_TYPE,
	BOARD_FIELD_GRID_CEL_LOOK,
	BOARD_FIELD_GRID_CEL_OFFSET,
	BOARD_FIELD_GRID_CEL_FOG,
	BOARD_FIELD_ENABLE_GRAVESTONES,
	BOARD_FIELD_SPECIAL_GRAVESTONE_X,
	BOARD_FIELD_SPECIAL_GRAVESTONE_Y,
	BOARD_FIELD_FOG_OFFSET,
	BOARD_FIELD_FOG_BLOWN_COUNTDOWN,
	BOARD_FIELD_PLANT_ROW,
	BOARD_FIELD_WAVE_ROW_GOT_LAWN_MOWERED,
	BOARD_FIELD_BONUS_LAWN_MOWERS_REMAINING,
	BOARD_FIELD_ICE_MIN_X,
	BOARD_FIELD_ICE_TIMER,
	BOARD_FIELD_ICE_PARTICLE_ID,
	BOARD_FIELD_ROW_PICKING_ARRAY,
	BOARD_FIELD_ZOMBIES_IN_WAVE,
	BOARD_FIELD_ZOMBIE_ALLOWED,
	BOARD_FIELD_SUN_COUNTDOWN,
	BOARD_FIELD_NUM_SUNS_FALLEN,
	BOARD_FIELD_SHAKE_COUNTER,
	BOARD_FIELD_SHAKE_AMOUNT_X,
	BOARD_FIELD_SHAKE_AMOUNT_Y,
	BOARD_FIELD_BACKGROUND,
	BOARD_FIELD_LEVEL,
	BOARD_FIELD_SOD_POSITION,
	BOARD_FIELD_PREV_MOUSE_X,
	BOARD_FIELD_PREV_MOUSE_Y,
	BOARD_FIELD_SUN_MONEY,
	BOARD_FIELD_NUM_WAVES,
	BOARD_FIELD_MAIN_COUNTER,
	BOARD_FIELD_EFFECT_COUNTER,
	BOARD_FIELD_DRAW_COUNT,
	BOARD_FIELD_RISE_FROM_GRAVE_COUNTER,
	BOARD_FIELD_OUT_OF_MONEY_COUNTER,
	BOARD_FIELD_CURRENT_WAVE,
	BOARD_FIELD_TOTAL_SPAWNED_WAVES,
	BOARD_FIELD_TUTORIAL_STATE,
	BOARD_FIELD_TUTORIAL_PARTICLE_ID,
	BOARD_FIELD_TUTORIAL_TIMER,
	BOARD_FIELD_LAST_BUNGEE_WAVE,
	BOARD_FIELD_ZOMBIE_HEALTH_TO_NEXT_WAVE,
	BOARD_FIELD_ZOMBIE_HEALTH_WAVE_START,
	BOARD_FIELD_ZOMBIE_COUNTDOWN,
	BOARD_FIELD_ZOMBIE_COUNTDOWN_START,
	BOARD_FIELD_HUGE_WAVE_COUNTDOWN,
	BOARD_FIELD_HELP_DISPLAYED,
	BOARD_FIELD_HELP_INDEX,
	BOARD_FIELD_FINAL_BOSS_KILLED,
	BOARD_FIELD_SHOW_SHOVEL,
	BOARD_FIELD_COIN_BANK_FADE_COUNT,
	BOARD_FIELD_DEBUG_TEXT_MODE,
	BOARD_FIELD_LEVEL_COMPLETE,
	BOARD_FIELD_BOARD_FADE_OUT_COUNTER,
	BOARD_FIELD_NEXT_SURVIVAL_STAGE_COUNTER,
	BOARD_FIELD_SCORE_NEXT_MOWER_COUNTER,
	BOARD_FIELD_LEVEL_AWARD_SPAWNED,
	BOARD_FIELD_PROGRESS_METER_WIDTH,
	BOARD_FIELD_FLAG_RAISE_COUNTER,
	BOARD_FIELD_ICE_TRAP_COUNTER,
	BOARD_FIELD_BOARD_RAND_SEED,
	BOARD_FIELD_POOL_SPARKLY_PARTICLE_ID,
	BOARD_FIELD_FWOOSH_ID,
	BOARD_FIELD_FWOOSH_COUNTDOWN,
	BOARD_FIELD_TIME_STOP_COUNTER,
	BOARD_FIELD_DROPPED_FIRST_COIN,
	BOARD_FIELD_FINAL_WAVE_SOUND_COUNTER,
	BOARD_FIELD_COB_CANNON_CURSOR_DELAY_COUNTER,
	BOARD_FIELD_COB_CANNON_MOUSE_X,
	BOARD_FIELD_COB_CANNON_MOUSE_Y,
	BOARD_FIELD_KILLED_YETI,
	BOARD_FIELD_MUSTACHE_MODE,
	BOARD_FIELD_SUPER_MOWER_MODE,
	BOARD_FIELD_FUTURE_MODE,
	BOARD_FIELD_PINATA_MODE,
	BOARD_FIELD_DANCE_MODE,
	BOARD_FIELD_DAISY_MODE,
	BOARD_FIELD_SUKHBIR_MODE,
	BOARD_FIELD_PREV_BOARD_RESULT,
	BOARD_FIELD_TRIGGERED_LAWN_MOWERS,
	BOARD_FIELD_PLAY_TIME_ACTIVE_LEVEL,
	BOARD_FIELD_PLAY_TIME_INACTIVE_LEVEL,
	BOARD_FIELD_MAX_SUN_PLANTS,
	BOARD_FIELD_START_DRAW_TIME,
	BOARD_FIELD_INTERVAL_DRAW_TIME,
	BOARD_FIELD_INTERVAL_DRAW_COUNT_START,
	BOARD_FIELD_MIN_FPS,
	BOARD_FIELD_PRELOAD_TIME,
	BOARD_FIELD_GAME_ID,
	BOARD_FIELD_GRAVES_CLEARED,
	BOARD_FIELD_PLANTS_EATEN,
	BOARD_FIELD_PLANTS_SHOVELED,
	BOARD_FIELD_PEA_SHOOTER_USED,
	BOARD_FIELD_CATAPULT_PLANTS_USED,
	BOARD_FIELD_MUSHROOM_AND_COFFEE_BEANS_ONLY,
	BOARD_FIELD_MUSHROOMS_USED,
	BOARD_FIELD_LEVEL_COINS_COLLECTED,
	BOARD_FIELD_GARGANTUARS_KILLS_BY_CORN_COB,
	BOARD_FIELD_COINS_COLLECTED,
	BOARD_FIELD_DIAMONDS_COLLECTED,
	BOARD_FIELD_POTTED_PLANTS_COLLECTED,
	BOARD_FIELD_CHOCOLATE_COLLECTED,
	BOARD_FIELD_COUNT
};

struct BoardBaseFieldEntry
{
	uint32_t	mFieldId;
	void		(*mSync)(PortableSaveContext&, Board*);
};

// Single source of truth for Board base fields: field order is the write order.
static constexpr BoardBaseFieldEntry gBoardBaseFields[] = {
	{ BOARD_FIELD_PAUSED, [](PortableSaveContext& c, Board* theBoard){ c.SyncBool(theBoard->mPaused); } },
	{ BOARD_FIELD_GRID_SQUARE_TYPE, [](PortableSaveContext& c, Board* theBoard){ SyncEnum32Array(c, &theBoard->mGridSquareType[0][0], MAX_GRID_SIZE_X * MAX_GRID_SIZE_Y); } },
	{ BOARD_FIELD_GRID_CEL_LOOK, [](PortableSaveContext& c, Board* theBoard){ SyncInt32Array(c, &theBoard->mGridCelLook[0][0], MAX_GRID_SIZE_X * MAX_GRID_SIZE_Y); } },
	{ BOARD_FIELD_GRID_CEL_OFFSET, [](PortableSaveContext& c, Board* theBoard){ SyncInt32Array(c, &theBoard->mGridCelOffset[0][0][0], MAX_GRID_SIZE_X * MAX_GRID_SIZE_Y * 2); } },
	{ BOARD_FIELD_GRID_CEL_FOG, [](PortableSaveContext& c, Board* theBoard){ SyncInt32Array(c, &theBoard->mGridCelFog[0][0], MAX_GRID_SIZE_X * (MAX_GRID_SIZE_Y + 1)); } },
	{ BOARD_FIELD_ENABLE_GRAVESTONES, [](PortableSaveContext& c, Board* theBoard){ c.SyncBool(theBoard->mEnableGraveStones); } },
	{ BOARD_FIELD_SPECIAL_GRAVESTONE_X, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mSpecialGraveStoneX); } },
	{ BOARD_FIELD_SPECIAL_GRAVESTONE_Y, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mSpecialGraveStoneY); } },
	{ BOARD_FIELD_FOG_OFFSET, [](PortableSaveContext& c, Board* theBoard){ c.SyncFloat(theBoard->mFogOffset); } },
	{ BOARD_FIELD_FOG_BLOWN_COUNTDOWN, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mFogBlownCountDown); } },
	{ BOARD_FIELD_PLANT_ROW, [](PortableSaveContext& c, Board* theBoard){ SyncEnum32Array(c, &theBoard->mPlantRow[0], MAX_GRID_SIZE_Y); } },
	{ BOARD_FIELD_WAVE_ROW_GOT_LAWN_MOWERED, [](PortableSaveContext& c, Board* theBoard){ SyncInt32Array(c, &theBoard->mWaveRowGotLawnMowered[0], MAX_GRID_SIZE_Y); } },
	{ BOARD_FIELD_BONUS_LAWN_MOWERS_REMAINING, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mBonusLawnMowersRemaining); } },
	{ BOARD_FIELD_ICE_MIN_X, [](PortableSaveContext& c, Board* theBoard){ SyncInt32Array(c, &theBoard->mIceMinX[0], MAX_GRID_SIZE_Y); } },
	{ BOARD_FIELD_ICE_TIMER, [](PortableSaveContext& c, Board* theBoard){ SyncInt32Array(c, &theBoard->mIceTimer[0], MAX_GRID_SIZE_Y); } },
	{ BOARD_FIELD_ICE_PARTICLE_ID, [](PortableSaveContext& c, Board* theBoard){ SyncEnumU32Array(c, &theBoard->mIceParticleID[0], MAX_GRID_SIZE_Y); } },
	{ BOARD_FIELD_ROW_PICKING_ARRAY, [](PortableSaveContext& c, Board* theBoard){ SyncPvzpSmoothArrayList(c, &theBoard->mRowPickingArray[0], MAX_GRID_SIZE_Y); } },
	{ BOARD_FIELD_ZOMBIES_IN_WAVE, [](PortableSaveContext& c, Board* theBoard){ SyncEnum32Array(c, &theBoard->mZombiesInWave[0][0], MAX_ZOMBIE_WAVES * MAX_ZOMBIES_IN_WAVE); } },
	{ BOARD_FIELD_ZOMBIE_ALLOWED, [](PortableSaveContext& c, Board* theBoard){ SyncBoolArray(c, &theBoard->mZombieAllowed[0], 100); } },
	{ BOARD_FIELD_SUN_COUNTDOWN, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mSunCountDown); } },
	{ BOARD_FIELD_NUM_SUNS_FALLEN, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mNumSunsFallen); } },
	{ BOARD_FIELD_SHAKE_COUNTER, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mShakeCounter); } },
	{ BOARD_FIELD_SHAKE_AMOUNT_X, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mShakeAmountX); } },
	{ BOARD_FIELD_SHAKE_AMOUNT_Y, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mShakeAmountY); } },
	{ BOARD_FIELD_BACKGROUND, [](PortableSaveContext& c, Board* theBoard){ SyncEnum32(c, theBoard->mBackground); } },
	{ BOARD_FIELD_LEVEL, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mLevel); } },
	{ BOARD_FIELD_SOD_POSITION, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mSodPosition); } },
	{ BOARD_FIELD_PREV_MOUSE_X, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mPrevMouseX); } },
	{ BOARD_FIELD_PREV_MOUSE_Y, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mPrevMouseY); } },
	{ BOARD_FIELD_SUN_MONEY, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mSunMoney); } },
	{ BOARD_FIELD_NUM_WAVES, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mNumWaves); } },
	{ BOARD_FIELD_MAIN_COUNTER, [](PortableSaveContext& c, Board* theBoard){ c.SyncUInt32(theBoard->mMainCounter); } },
	{ BOARD_FIELD_EFFECT_COUNTER, [](PortableSaveContext& c, Board* theBoard){ c.SyncUInt32(theBoard->mEffectCounter); } },
	{ BOARD_FIELD_DRAW_COUNT, [](PortableSaveContext& c, Board* theBoard){ c.SyncUInt32(theBoard->mDrawCount); } },
	{ BOARD_FIELD_RISE_FROM_GRAVE_COUNTER, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mRiseFromGraveCounter); } },
	{ BOARD_FIELD_OUT_OF_MONEY_COUNTER, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mOutOfMoneyCounter); } },
	{ BOARD_FIELD_CURRENT_WAVE, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mCurrentWave); } },
	{ BOARD_FIELD_TOTAL_SPAWNED_WAVES, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mTotalSpawnedWaves); } },
	{ BOARD_FIELD_TUTORIAL_STATE, [](PortableSaveContext& c, Board* theBoard){ SyncEnum32(c, theBoard->mTutorialState); } },
	{ BOARD_FIELD_TUTORIAL_PARTICLE_ID, [](PortableSaveContext& c, Board* theBoard){ SyncEnumU32(c, theBoard->mTutorialParticleID); } },
	{ BOARD_FIELD_TUTORIAL_TIMER, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mTutorialTimer); } },
	{ BOARD_FIELD_LAST_BUNGEE_WAVE, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mLastBungeeWave); } },
	{ BOARD_FIELD_ZOMBIE_HEALTH_TO_NEXT_WAVE, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mZombieHealthToNextWave); } },
	{ BOARD_FIELD_ZOMBIE_HEALTH_WAVE_START, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mZombieHealthWaveStart); } },
	{ BOARD_FIELD_ZOMBIE_COUNTDOWN, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mZombieCountDown); } },
	{ BOARD_FIELD_ZOMBIE_COUNTDOWN_START, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mZombieCountDownStart); } },
	{ BOARD_FIELD_HUGE_WAVE_COUNTDOWN, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mHugeWaveCountDown); } },
	{ BOARD_FIELD_HELP_DISPLAYED, [](PortableSaveContext& c, Board* theBoard){ SyncBoolArray(c, &theBoard->mHelpDisplayed[0], NUM_ADVICE_TYPES); } },
	{ BOARD_FIELD_HELP_INDEX, [](PortableSaveContext& c, Board* theBoard){ SyncEnum32(c, theBoard->mHelpIndex); } },
	{ BOARD_FIELD_FINAL_BOSS_KILLED, [](PortableSaveContext& c, Board* theBoard){ c.SyncBool(theBoard->mFinalBossKilled); } },
	{ BOARD_FIELD_SHOW_SHOVEL, [](PortableSaveContext& c, Board* theBoard){ c.SyncBool(theBoard->mShowShovel); } },
	{ BOARD_FIELD_COIN_BANK_FADE_COUNT, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mCoinBankFadeCount); } },
	{ BOARD_FIELD_DEBUG_TEXT_MODE, [](PortableSaveContext& c, Board* theBoard){ SyncEnum32(c, theBoard->mDebugTextMode); } },
	{ BOARD_FIELD_LEVEL_COMPLETE, [](PortableSaveContext& c, Board* theBoard){ c.SyncBool(theBoard->mLevelComplete); } },
	{ BOARD_FIELD_BOARD_FADE_OUT_COUNTER, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mBoardFadeOutCounter); } },
	{ BOARD_FIELD_NEXT_SURVIVAL_STAGE_COUNTER, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mNextSurvivalStageCounter); } },
	{ BOARD_FIELD_SCORE_NEXT_MOWER_COUNTER, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mScoreNextMowerCounter); } },
	{ BOARD_FIELD_LEVEL_AWARD_SPAWNED, [](PortableSaveContext& c, Board* theBoard){ c.SyncBool(theBoard->mLevelAwardSpawned); } },
	{ BOARD_FIELD_PROGRESS_METER_WIDTH, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mProgressMeterWidth); } },
	{ BOARD_FIELD_FLAG_RAISE_COUNTER, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mFlagRaiseCounter); } },
	{ BOARD_FIELD_ICE_TRAP_COUNTER, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mIceTrapCounter); } },
	{ BOARD_FIELD_BOARD_RAND_SEED, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mBoardRandSeed); } },
	{ BOARD_FIELD_POOL_SPARKLY_PARTICLE_ID, [](PortableSaveContext& c, Board* theBoard){ SyncEnumU32(c, theBoard->mPoolSparklyParticleID); } },
	{ BOARD_FIELD_FWOOSH_ID, [](PortableSaveContext& c, Board* theBoard){ SyncEnumU32Array(c, &theBoard->mFwooshID[0][0], MAX_GRID_SIZE_Y * 12); } },
	{ BOARD_FIELD_FWOOSH_COUNTDOWN, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mFwooshCountDown); } },
	{ BOARD_FIELD_TIME_STOP_COUNTER, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mTimeStopCounter); } },
	{ BOARD_FIELD_DROPPED_FIRST_COIN, [](PortableSaveContext& c, Board* theBoard){ c.SyncBool(theBoard->mDroppedFirstCoin); } },
	{ BOARD_FIELD_FINAL_WAVE_SOUND_COUNTER, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mFinalWaveSoundCounter); } },
	{ BOARD_FIELD_COB_CANNON_CURSOR_DELAY_COUNTER, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mCobCannonCursorDelayCounter); } },
	{ BOARD_FIELD_COB_CANNON_MOUSE_X, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mCobCannonMouseX); } },
	{ BOARD_FIELD_COB_CANNON_MOUSE_Y, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mCobCannonMouseY); } },
	{ BOARD_FIELD_KILLED_YETI, [](PortableSaveContext& c, Board* theBoard){ c.SyncBool(theBoard->mKilledYeti); } },
	{ BOARD_FIELD_MUSTACHE_MODE, [](PortableSaveContext& c, Board* theBoard){ c.SyncBool(theBoard->mMustacheMode); } },
	{ BOARD_FIELD_SUPER_MOWER_MODE, [](PortableSaveContext& c, Board* theBoard){ c.SyncBool(theBoard->mSuperMowerMode); } },
	{ BOARD_FIELD_FUTURE_MODE, [](PortableSaveContext& c, Board* theBoard){ c.SyncBool(theBoard->mFutureMode); } },
	{ BOARD_FIELD_PINATA_MODE, [](PortableSaveContext& c, Board* theBoard){ c.SyncBool(theBoard->mPinataMode); } },
	{ BOARD_FIELD_DANCE_MODE, [](PortableSaveContext& c, Board* theBoard){ c.SyncBool(theBoard->mDanceMode); } },
	{ BOARD_FIELD_DAISY_MODE, [](PortableSaveContext& c, Board* theBoard){ c.SyncBool(theBoard->mDaisyMode); } },
	{ BOARD_FIELD_SUKHBIR_MODE, [](PortableSaveContext& c, Board* theBoard){ c.SyncBool(theBoard->mSukhbirMode); } },
	{ BOARD_FIELD_PREV_BOARD_RESULT, [](PortableSaveContext& c, Board* theBoard){ SyncEnum32(c, theBoard->mPrevBoardResult); } },
	{ BOARD_FIELD_TRIGGERED_LAWN_MOWERS, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mTriggeredLawnMowers); } },
	{ BOARD_FIELD_PLAY_TIME_ACTIVE_LEVEL, [](PortableSaveContext& c, Board* theBoard){ c.SyncUInt32(theBoard->mPlayTimeActiveLevel); } },
	{ BOARD_FIELD_PLAY_TIME_INACTIVE_LEVEL, [](PortableSaveContext& c, Board* theBoard){ c.SyncUInt32(theBoard->mPlayTimeInactiveLevel); } },
	{ BOARD_FIELD_MAX_SUN_PLANTS, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mMaxSunPlants); } },
	{ BOARD_FIELD_START_DRAW_TIME, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt64(theBoard->mStartDrawTime); } },
	{ BOARD_FIELD_INTERVAL_DRAW_TIME, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt64(theBoard->mIntervalDrawTime); } },
	{ BOARD_FIELD_INTERVAL_DRAW_COUNT_START, [](PortableSaveContext& c, Board* theBoard){ c.SyncUInt32(theBoard->mIntervalDrawCountStart); } },
	{ BOARD_FIELD_MIN_FPS, [](PortableSaveContext& c, Board* theBoard){ c.SyncFloat(theBoard->mMinFPS); } },
	{ BOARD_FIELD_PRELOAD_TIME, [](PortableSaveContext& c, Board* theBoard){ c.SyncInt32(theBoard->mPreloadTime); } },
	{ BOARD_FIELD_GAME_ID, [](PortableSaveContext& c, Board* theBoard){ int64_t aGameId = static_cast<int64_t>(theBoard->mGameID); c.SyncInt64(aGameId); if (c.mReading) theBoard->mGameID = static_cast<intptr_t>(aGameId); } },
	{ BOARD_FIELD_GRAVES_CLEARED, [](PortableSaveContext& c, Board* theBoard){ c.SyncUInt32(theBoard->mGravesCleared); } },
	{ BOARD_FIELD_PLANTS_EATEN, [](PortableSaveContext& c, Board* theBoard){ c.SyncUInt32(theBoard->mPlantsEaten); } },
	{ BOARD_FIELD_PLANTS_SHOVELED, [](PortableSaveContext& c, Board* theBoard){ c.SyncUInt32(theBoard->mPlantsShoveled); } },
	{ BOARD_FIELD_PEA_SHOOTER_USED, [](PortableSaveContext& c, Board* theBoard){ c.SyncBool(theBoard->mPeaShooterUsed); } },
	{ BOARD_FIELD_CATAPULT_PLANTS_USED, [](PortableSaveContext& c, Board* theBoard){ c.SyncBool(theBoard->mCatapultPlantsUsed); } },
	{ BOARD_FIELD_MUSHROOM_AND_COFFEE_BEANS_ONLY, [](PortableSaveContext& c, Board* theBoard){ c.SyncBool(theBoard->mMushroomAndCoffeeBeansOnly); } },
	{ BOARD_FIELD_MUSHROOMS_USED, [](PortableSaveContext& c, Board* theBoard){ c.SyncBool(theBoard->mMushroomsUsed); } },
	{ BOARD_FIELD_LEVEL_COINS_COLLECTED, [](PortableSaveContext& c, Board* theBoard){ c.SyncUInt32(theBoard->mLevelCoinsCollected); } },
	{ BOARD_FIELD_GARGANTUARS_KILLS_BY_CORN_COB, [](PortableSaveContext& c, Board* theBoard){ c.SyncUInt32(theBoard->mGargantuarsKillsByCornCob); } },
	{ BOARD_FIELD_COINS_COLLECTED, [](PortableSaveContext& c, Board* theBoard){ c.SyncUInt32(theBoard->mCoinsCollected); } },
	{ BOARD_FIELD_DIAMONDS_COLLECTED, [](PortableSaveContext& c, Board* theBoard){ c.SyncUInt32(theBoard->mDiamondsCollected); } },
	{ BOARD_FIELD_POTTED_PLANTS_COLLECTED, [](PortableSaveContext& c, Board* theBoard){ c.SyncUInt32(theBoard->mPottedPlantsCollected); } },
	{ BOARD_FIELD_CHOCOLATE_COLLECTED, [](PortableSaveContext& c, Board* theBoard){ c.SyncUInt32(theBoard->mChocolateCollected); } },
};

// The enum is contiguous starting at 1: the table must cover every id, in id order, so readers can index it directly.
static_assert([]{
	if (sizeof(gBoardBaseFields) / sizeof(gBoardBaseFields[0]) != BOARD_FIELD_COUNT - 1)
		return false;
	for (uint32_t i = 0; i < sizeof(gBoardBaseFields) / sizeof(gBoardBaseFields[0]); i++)
		if (gBoardBaseFields[i].mFieldId != i + 1)
			return false;
	return true;
}(), "gBoardBaseFields must cover every BoardBaseFieldId in id order");

static void SyncBoardBasePortable(PortableSaveContext& theContext, Board* theBoard)
{
	if (theContext.mReading)
	{
		std::vector<unsigned char> aBlob;
		if (!ReadTLVBlob(theContext, aBlob))
			return;
		TLVReader aReader(aBlob.data(), aBlob.size());
		while (aReader.mOk && aReader.mPos < aReader.mSize)
		{
			uint32_t aFieldId = 0;
			uint32_t aFieldSize = 0;
			if (!aReader.ReadU32(aFieldId) || !aReader.ReadU32(aFieldSize))
				break;
			const unsigned char* aFieldData = nullptr;
			if (!aReader.ReadBytes(aFieldData, aFieldSize))
				break;
			if (aFieldId >= 1 && aFieldId <= sizeof(gBoardBaseFields) / sizeof(gBoardBaseFields[0]))
			{
				const BoardBaseFieldEntry& aField = gBoardBaseFields[aFieldId - 1];
				ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ aField.mSync(c, theBoard); });
			}
		}
	}
	else
	{
		std::vector<unsigned char> aBlob;
		for (const BoardBaseFieldEntry& aField : gBoardBaseFields)
			AppendFieldWithSync(aBlob, aField.mFieldId, [&](PortableSaveContext& c){ aField.mSync(c, theBoard); });
		WriteTLVBlob(theContext, aBlob);
	}
}

static void SyncZombiesPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncDataArrayObjectsTLV(theContext, theBoard->mZombies, SyncZombieTailPortable);
}

static void SyncPlantsPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncDataArrayObjectsTLV(theContext, theBoard->mPlants, SyncPlantTailPortable);
}

static void SyncProjectilesPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncDataArrayObjectsTLV(theContext, theBoard->mProjectiles, SyncProjectileTailPortable);
}

static void SyncCoinsPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncDataArrayObjectsTLV(theContext, theBoard->mCoins, SyncCoinTailPortable);
}

static void SyncMowersPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncDataArrayObjectsTLV(theContext, theBoard->mLawnMowers, SyncLawnMowerTailPortable);
}

static void SyncGridItemsPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncDataArrayObjectsTLV(theContext, theBoard->mGridItems, SyncGridItemTailPortable);
}

static void SyncParticleEmittersPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncDataArrayIdsOnlyPortable(theContext, theBoard->mApp->mEffectSystem->mParticleHolder->mEmitters);
}

static void SyncParticlesPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncDataArrayIdsOnlyPortable(theContext, theBoard->mApp->mEffectSystem->mParticleHolder->mParticles);
}

static void SyncParticleSystemsPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncDataArrayPortableTLV(theContext, theBoard->mApp->mEffectSystem->mParticleHolder->mParticleSystems,
		[theBoard](std::vector<unsigned char>& aOut, PvzpParticleSystem& theSystem)
		{
			AppendFieldWithSync(aOut, 1U, [&](PortableSaveContext& aContext)
			{
				SyncParticleSystemPortable(theBoard, &theSystem, aContext);
			});
		},
		[theBoard](uint32_t aFieldId, const unsigned char* aData, size_t aSize, PvzpParticleSystem& theSystem)
		{
			if (aFieldId == 1U)
			{
				ApplyFieldWithSync(aData, aSize, [&](PortableSaveContext& aContext)
				{
					SyncParticleSystemPortable(theBoard, &theSystem, aContext);
				});
			}
		});
}

static void SyncReanimationsPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncDataArrayPortableTLV(theContext, theBoard->mApp->mEffectSystem->mReanimationHolder->mReanimations,
		[theBoard](std::vector<unsigned char>& aOut, Reanimation& theReanimation)
		{
			AppendFieldWithSync(aOut, 1U, [&](PortableSaveContext& aContext)
			{
				SyncReanimationPortable(theBoard, &theReanimation, aContext);
			});
		},
		[theBoard](uint32_t aFieldId, const unsigned char* aData, size_t aSize, Reanimation& theReanimation)
		{
			if (aFieldId == 1U)
			{
				ApplyFieldWithSync(aData, aSize, [&](PortableSaveContext& aContext)
				{
					SyncReanimationPortable(theBoard, &theReanimation, aContext);
				});
			}
		});
}

static void SyncTrailsPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncDataArrayPortableTLV(theContext, theBoard->mApp->mEffectSystem->mTrailHolder->mTrails,
		[theBoard](std::vector<unsigned char>& aOut, Trail& theTrail)
		{
			AppendFieldWithSync(aOut, 1U, [&](PortableSaveContext& aContext)
			{
				SyncTrailPortable(theBoard, &theTrail, aContext);
			});
		},
		[theBoard](uint32_t aFieldId, const unsigned char* aData, size_t aSize, Trail& theTrail)
		{
			if (aFieldId == 1U)
			{
				ApplyFieldWithSync(aData, aSize, [&](PortableSaveContext& aContext)
				{
					SyncTrailPortable(theBoard, &theTrail, aContext);
				});
			}
		});
}

static void SyncAttachmentsPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncDataArrayObjectsTLV(theContext, theBoard->mApp->mEffectSystem->mAttachmentHolder->mAttachments, SyncAttachmentTailPortable);
}

static void SyncCursorPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncSingleObjectTLV(theContext, *theBoard->mCursorObject, SyncCursorObjectTailPortable);
}

static void SyncCursorPreviewPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncSingleObjectTLV(theContext, *theBoard->mCursorPreview, SyncCursorPreviewTailPortable);
}

static void SyncAdvicePortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncSingleObjectTLV(theContext, *theBoard->mAdvice, SyncMessageWidgetTailPortable);
}

static void SyncSeedBankPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncSingleObjectTLV(theContext, *theBoard->mSeedBank, SyncSeedBankTailPortable);
}

static void SyncSeedPacketsPortable(PortableSaveContext& theContext, Board* theBoard)
{
	int aCount = SEEDBANK_MAX;
	theContext.SyncInt32(aCount);
	for (int i = 0; i < aCount && i < SEEDBANK_MAX; i++)
	{
		if (theContext.mReading)
		{
			uint32_t aItemSize = 0;
			theContext.SyncUInt32(aItemSize);
			std::vector<unsigned char> aItemData;
			aItemData.resize(aItemSize);
			if (aItemSize > 0)
				theContext.SyncBytes(aItemData.data(), aItemSize);
			TLVReader aReader(aItemData.data(), aItemSize);
			while (aReader.mOk && aReader.mPos < aReader.mSize)
			{
				uint32_t aFieldId = 0;
				uint32_t aFieldSize = 0;
				if (!aReader.ReadU32(aFieldId) || !aReader.ReadU32(aFieldSize))
					break;
				const unsigned char* aFieldData = nullptr;
				if (!aReader.ReadBytes(aFieldData, aFieldSize))
					break;
				switch (aFieldId)
				{
				case 1U: ReadGameObjectField(aFieldData, aFieldSize, theBoard->mSeedBank->mSeedPackets[i]); break;
				case PORTABLE_FIELD_TAIL: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncSeedPacketTailPortable(c, theBoard->mSeedBank->mSeedPackets[i]); }); break;
				default: break;
				}
			}
		}
		else
		{
			std::vector<unsigned char> aItemData;
			WriteGameObjectField(aItemData, 1U, theBoard->mSeedBank->mSeedPackets[i]);
			AppendFieldWithSync(aItemData, PORTABLE_FIELD_TAIL, [&](PortableSaveContext& c){ SyncSeedPacketTailPortable(c, theBoard->mSeedBank->mSeedPackets[i]); });
			uint32_t aItemSize = static_cast<uint32_t>(aItemData.size());
			theContext.SyncUInt32(aItemSize);
			if (aItemSize > 0)
				theContext.SyncBytes(aItemData.data(), aItemSize);
		}
	}
}

static void SyncChallengePortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncSingleObjectTLV(theContext, *theBoard->mChallenge, SyncChallengeTailPortable);
}

static void SyncMusicPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncSingleObjectTLV(theContext, *theBoard->mApp->mMusic, SyncMusicTailPortable);
}

static void SyncBoardPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncBoardBasePortable(theContext, theBoard);
	SyncZombiesPortable(theContext, theBoard);
	SyncPlantsPortable(theContext, theBoard);
	SyncProjectilesPortable(theContext, theBoard);
	SyncCoinsPortable(theContext, theBoard);
	SyncMowersPortable(theContext, theBoard);
	SyncGridItemsPortable(theContext, theBoard);
	SyncParticleEmittersPortable(theContext, theBoard);
	SyncParticlesPortable(theContext, theBoard);
	SyncParticleSystemsPortable(theContext, theBoard);
	SyncReanimationsPortable(theContext, theBoard);
	SyncTrailsPortable(theContext, theBoard);
	SyncAttachmentsPortable(theContext, theBoard);
	SyncCursorPortable(theContext, theBoard);
	SyncCursorPreviewPortable(theContext, theBoard);
	SyncAdvicePortable(theContext, theBoard);
	SyncSeedBankPortable(theContext, theBoard);
	SyncSeedPacketsPortable(theContext, theBoard);
	SyncChallengePortable(theContext, theBoard);
	SyncMusicPortable(theContext, theBoard);
}


typedef void (*ChunkSyncFn)(PortableSaveContext&, Board*);

static ChunkSyncFn GetChunkSyncFn(uint32_t theChunkType)
{
	switch (theChunkType)
	{
	case SAVE4_CHUNK_BOARD_BASE:
		return SyncBoardBasePortable;
	case SAVE4_CHUNK_ZOMBIES:
		return SyncZombiesPortable;
	case SAVE4_CHUNK_PLANTS:
		return SyncPlantsPortable;
	case SAVE4_CHUNK_PROJECTILES:
		return SyncProjectilesPortable;
	case SAVE4_CHUNK_COINS:
		return SyncCoinsPortable;
	case SAVE4_CHUNK_MOWERS:
		return SyncMowersPortable;
	case SAVE4_CHUNK_GRIDITEMS:
		return SyncGridItemsPortable;
	case SAVE4_CHUNK_PARTICLE_EMITTERS:
		return SyncParticleEmittersPortable;
	case SAVE4_CHUNK_PARTICLE_PARTICLES:
		return SyncParticlesPortable;
	case SAVE4_CHUNK_PARTICLE_SYSTEMS:
		return SyncParticleSystemsPortable;
	case SAVE4_CHUNK_REANIMATIONS:
		return SyncReanimationsPortable;
	case SAVE4_CHUNK_TRAILS:
		return SyncTrailsPortable;
	case SAVE4_CHUNK_ATTACHMENTS:
		return SyncAttachmentsPortable;
	case SAVE4_CHUNK_CURSOR:
		return SyncCursorPortable;
	case SAVE4_CHUNK_CURSOR_PREVIEW:
		return SyncCursorPreviewPortable;
	case SAVE4_CHUNK_ADVICE:
		return SyncAdvicePortable;
	case SAVE4_CHUNK_SEEDBANK:
		return SyncSeedBankPortable;
	case SAVE4_CHUNK_SEEDPACKETS:
		return SyncSeedPacketsPortable;
	case SAVE4_CHUNK_CHALLENGE:
		return SyncChallengePortable;
	case SAVE4_CHUNK_MUSIC:
		return SyncMusicPortable;
	default:
		return nullptr;
	}
}

static bool WriteChunkV4(std::vector<unsigned char>& thePayload, uint32_t theChunkType, Board* theBoard)
{
	ChunkSyncFn aSyncFn = GetChunkSyncFn(theChunkType);
	if (!aSyncFn)
		return true;

	DataWriter aFieldWriter;
	aFieldWriter.OpenMemory(0x4000);
	PortableSaveContext aFieldContext(aFieldWriter);
	aSyncFn(aFieldContext, theBoard);
	if (aFieldContext.mFailed)
		return false;

	DataWriter aChunkWriter;
	aChunkWriter.OpenMemory(0x200);
	aChunkWriter.WriteUInt32(SAVE4_CHUNK_VERSION);
	aChunkWriter.WriteUInt32(1U);
	aChunkWriter.WriteUInt32(aFieldWriter.GetDataLen());
	aChunkWriter.WriteBytes(aFieldWriter.GetDataPtr(), aFieldWriter.GetDataLen());

	std::vector<unsigned char> aChunk;
	aChunk.resize(aChunkWriter.GetDataLen());
	memcpy(aChunk.data(), aChunkWriter.GetDataPtr(), aChunkWriter.GetDataLen());
	AppendChunk(thePayload, theChunkType, aChunk);
	return true;
}

static bool ReadChunkV4(uint32_t theChunkType, const unsigned char* theData, size_t theSize, Board* theBoard)
{
	ChunkSyncFn aSyncFn = GetChunkSyncFn(theChunkType);
	if (!aSyncFn)
		return true;
	if (theSize < 4)
		return false;

	TLVReader aReader(theData, theSize);
	uint32_t aChunkVersion = 0;
	if (!aReader.ReadU32(aChunkVersion))
		return false;
	if (aChunkVersion != SAVE4_CHUNK_VERSION)
		return false;

	bool aApplied = false;
	while (aReader.mOk && aReader.mPos < aReader.mSize)
	{
		uint32_t aFieldId = 0;
		uint32_t aFieldSize = 0;
		if (!aReader.ReadU32(aFieldId) || !aReader.ReadU32(aFieldSize))
			break;
		const unsigned char* aFieldData = nullptr;
		if (!aReader.ReadBytes(aFieldData, aFieldSize))
			break;

		if (aFieldId == 1U)
		{
			DataReader aFieldReader;
			aFieldReader.OpenMemory(aFieldData, static_cast<uint32_t>(aFieldSize), false);
			PortableSaveContext aContext(aFieldReader);
			aSyncFn(aContext, theBoard);
			if (aContext.mFailed)
				return false;
			aApplied = true;
		}
	}

	return aApplied;
}

static void FixBoardAfterLoad(Board* theBoard)
{
	{
		for (Plant* aPlant : theBoard->mPlants)
		{
			aPlant->mApp = theBoard->mApp;
			aPlant->mBoard = theBoard;
		}
	}
	{
		for (Zombie* aZombie : theBoard->mZombies)
		{
			aZombie->mApp = theBoard->mApp;
			aZombie->mBoard = theBoard;

			switch (aZombie->mZombieType)
			{
			case ZombieType::ZOMBIE_GARGANTUAR:
			case ZombieType::ZOMBIE_REDEYE_GARGANTUAR:
			{
				Reanimation* aBodyReanim = theBoard->mApp->ReanimationGet(aZombie->mBodyReanimID);
				if (aBodyReanim)
				{
					int aDamageIndex = aZombie->GetBodyDamageIndex();
					if (aDamageIndex >= 1)
					{
						aBodyReanim->SetImageOverride("Zombie_gargantua_body1", IMAGE_REANIM_ZOMBIE_GARGANTUAR_BODY1_2);
						aBodyReanim->SetImageOverride("Zombie_gargantuar_outerarm_lower", IMAGE_REANIM_ZOMBIE_GARGANTUAR_OUTERARM_LOWER2);
					}
					if (aDamageIndex >= 2)
					{
						aBodyReanim->SetImageOverride("Zombie_gargantua_body1", IMAGE_REANIM_ZOMBIE_GARGANTUAR_BODY1_3);
						aBodyReanim->SetImageOverride("Zombie_gargantuar_outerleg_foot", IMAGE_REANIM_ZOMBIE_GARGANTUAR_FOOT2);
					}

					if (aZombie->mZombieType == ZombieType::ZOMBIE_REDEYE_GARGANTUAR)
					{
						if (aDamageIndex >= 2)
							aBodyReanim->SetImageOverride("anim_head1", IMAGE_REANIM_ZOMBIE_GARGANTUAR_HEAD2_REDEYE);
						else
							aBodyReanim->SetImageOverride("anim_head1", IMAGE_REANIM_ZOMBIE_GARGANTUAR_HEAD_REDEYE);
					}
					else if (aDamageIndex >= 2)
					{
						aBodyReanim->SetImageOverride("anim_head1", IMAGE_REANIM_ZOMBIE_GARGANTUAR_HEAD2);
					}
				}
				break;
			}

			case ZombieType::ZOMBIE_ZAMBONI:
			{
				Reanimation* aBodyReanim = theBoard->mApp->ReanimationGet(aZombie->mBodyReanimID);
				if (aBodyReanim)
				{
					int aDamageIndex = aZombie->GetBodyDamageIndex();
					if (aDamageIndex >= 1)
					{
						aBodyReanim->SetImageOverride("Zombie_zamboni_1", IMAGE_REANIM_ZOMBIE_ZAMBONI_1_DAMAGE1);
						aBodyReanim->SetImageOverride("Zombie_zamboni_2", IMAGE_REANIM_ZOMBIE_ZAMBONI_2_DAMAGE1);
					}
					if (aDamageIndex >= 2)
					{
						aBodyReanim->SetImageOverride("Zombie_zamboni_1", IMAGE_REANIM_ZOMBIE_ZAMBONI_1_DAMAGE2);
						aBodyReanim->SetImageOverride("Zombie_zamboni_2", IMAGE_REANIM_ZOMBIE_ZAMBONI_2_DAMAGE2);
					}
				}
				break;
			}

			case ZombieType::ZOMBIE_CATAPULT:
			{
				Reanimation* aBodyReanim = theBoard->mApp->ReanimationGet(aZombie->mBodyReanimID);
				if (aBodyReanim)
				{
					int aDamageIndex = aZombie->GetBodyDamageIndex();
					if (aDamageIndex >= 1)
					{
						aBodyReanim->SetImageOverride("Zombie_catapult_siding", IMAGE_REANIM_ZOMBIE_CATAPULT_SIDING_DAMAGE);
					}
				}
				break;
			}

			case ZombieType::ZOMBIE_BOSS:
			{
				Reanimation* aBodyReanim = theBoard->mApp->ReanimationGet(aZombie->mBodyReanimID);
				if (aBodyReanim)
				{
					int aDamageIndex = aZombie->GetBodyDamageIndex();
					if (aDamageIndex >= 1)
					{
						aBodyReanim->SetImageOverride("Boss_head", IMAGE_REANIM_ZOMBIE_BOSS_HEAD_DAMAGE1);
						aBodyReanim->SetImageOverride("Boss_jaw", IMAGE_REANIM_ZOMBIE_BOSS_JAW_DAMAGE1);
						aBodyReanim->SetImageOverride("Boss_outerarm_hand", IMAGE_REANIM_ZOMBIE_BOSS_OUTERARM_HAND_DAMAGE1);
						aBodyReanim->SetImageOverride("Boss_outerarm_thumb2", IMAGE_REANIM_ZOMBIE_BOSS_OUTERARM_THUMB_DAMAGE1);
						aBodyReanim->SetImageOverride("Boss_innerleg_foot", IMAGE_REANIM_ZOMBIE_BOSS_FOOT_DAMAGE1);
					}
					if (aDamageIndex >= 2)
					{
						aBodyReanim->SetImageOverride("Boss_head", IMAGE_REANIM_ZOMBIE_BOSS_HEAD_DAMAGE2);
						aBodyReanim->SetImageOverride("Boss_jaw", IMAGE_REANIM_ZOMBIE_BOSS_JAW_DAMAGE2);
						aBodyReanim->SetImageOverride("Boss_outerarm_hand", IMAGE_REANIM_ZOMBIE_BOSS_OUTERARM_HAND_DAMAGE2);
						aBodyReanim->SetImageOverride("Boss_outerarm_thumb2", IMAGE_REANIM_ZOMBIE_BOSS_OUTERARM_THUMB_DAMAGE2);
						aBodyReanim->SetImageOverride("Boss_outerleg_foot", IMAGE_REANIM_ZOMBIE_BOSS_FOOT_DAMAGE2);
					}
				}
				break;
			}

			default:
				break;
			}
		}
	}
	{
		for (Projectile* aProjectile : theBoard->mProjectiles)
		{
			aProjectile->mApp = theBoard->mApp;
			aProjectile->mBoard = theBoard;
		}
	}
	{
		for (Coin* aCoin : theBoard->mCoins)
		{
			aCoin->mApp = theBoard->mApp;
			aCoin->mBoard = theBoard;
		}
	}
	{
		for (LawnMower* aLawnMower : theBoard->mLawnMowers)
		{
			aLawnMower->mApp = theBoard->mApp;
			aLawnMower->mBoard = theBoard;
		}
	}
	{
		for (GridItem* aGridItem : theBoard->mGridItems)
		{
			aGridItem->mApp = theBoard->mApp;
			aGridItem->mBoard = theBoard;
		}
	}

	theBoard->mAdvice->mApp = theBoard->mApp;
	theBoard->mCursorObject->mApp = theBoard->mApp;
	theBoard->mCursorObject->mBoard = theBoard;
	theBoard->mCursorPreview->mApp = theBoard->mApp;
	theBoard->mCursorPreview->mBoard = theBoard;
	theBoard->mSeedBank->mApp = theBoard->mApp;
	theBoard->mSeedBank->mBoard = theBoard;
	for (int i = 0; i < SEEDBANK_MAX; i++)
	{
		theBoard->mSeedBank->mSeedPackets[i].mApp = theBoard->mApp;
		theBoard->mSeedBank->mSeedPackets[i].mBoard = theBoard;
	}
	theBoard->mChallenge->mApp = theBoard->mApp;
	theBoard->mChallenge->mBoard = theBoard;
	theBoard->mApp->mMusic->mApp = theBoard->mApp;
	theBoard->mApp->mMusic->mMusicInterface = theBoard->mApp->mMusicInterface.get();
}

static bool LawnLoadGameV4(Board* theBoard, const std::string& theFilePath)
{
	Buffer aBuffer;
	if (!gSexyAppBase->ReadBufferFromFile(theFilePath, &aBuffer, false))
		return false;
	if (static_cast<uint32_t>(aBuffer.GetDataLen()) < sizeof(SaveFileHeaderV4))
		return false;

	SaveFileHeaderV4 aHeader;
	memcpy(&aHeader, aBuffer.GetDataPtr(), sizeof(aHeader));
	aHeader.mVersion = FromLE32(aHeader.mVersion);
	aHeader.mPayloadSize = FromLE32(aHeader.mPayloadSize);
	aHeader.mPayloadCrc = FromLE32(aHeader.mPayloadCrc);
	if (memcmp(aHeader.mMagic, SAVE_FILE_MAGIC_V4, sizeof(aHeader.mMagic)) != 0)
		return false;
	if (aHeader.mVersion != SAVE_FILE_V4_VERSION)
		return false;
	if (aHeader.mPayloadSize > static_cast<uint32_t>(aBuffer.GetDataLen()) - sizeof(SaveFileHeaderV4))
		return false;

	unsigned char* aPayload = (unsigned char*)aBuffer.GetDataPtr() + sizeof(SaveFileHeaderV4);
	uint32_t aCrc = crc32(0, (Bytef*)aPayload, aHeader.mPayloadSize);
	if (aCrc != aHeader.mPayloadCrc)
		return false;

	TLVReader aReader(aPayload, aHeader.mPayloadSize);
	bool aBaseLoaded = false;
	while (aReader.mOk && aReader.mPos < aReader.mSize)
	{
		uint32_t aChunkType = 0;
		uint32_t aChunkSize = 0;
		if (!aReader.ReadU32(aChunkType) || !aReader.ReadU32(aChunkSize))
			break;
		const unsigned char* aChunkData = nullptr;
		if (!aReader.ReadBytes(aChunkData, aChunkSize))
			break;

		if (!ReadChunkV4(aChunkType, aChunkData, aChunkSize, theBoard))
			return false;
		if (aChunkType == SAVE4_CHUNK_BOARD_BASE)
			aBaseLoaded = true;
	}

	if (!aBaseLoaded)
		return false;

	FixBoardAfterLoad(theBoard);
	theBoard->mApp->mGameScene = GameScenes::SCENE_PLAYING;
	return true;
}

// Legacy mid-level save support
class SaveGameContext
{
public:
	Sexy::Buffer	mBuffer;
	bool			mFailed;
	bool			mReading;

public:
	inline int		ByteLeftToRead() { return (mBuffer.mDataBitSize - mBuffer.mReadBitPos + 7) / 8; }
	void			SyncBytes(void* theDest, int theReadSize);
	void			SyncInt32(int32_t& theInt32);
	void			SyncUInt32(uint32_t& theUInt32);
	inline void		SyncInt(int& theInt)
	{
		int32_t aValue = theInt;
		SyncInt32(aValue);
		if (mReading)
			theInt = aValue;
	}
	void			SyncReanimationDef(ReanimatorDefinition*& theDefinition);
	void			SyncParticleDef(PvzpParticleDefinition*& theDefinition);
	void			SyncTrailDef(TrailDefinition*& theDefinition);
	void			SyncImage(Image*& theImage);
};

void SaveGameContext::SyncBytes(void* theDest, int theReadSize)
{
	int aReadSize = theReadSize;
	if (mReading)
	{
		if (ByteLeftToRead() < 4)
		{
			mFailed = true;
		}

		aReadSize = mFailed ? 0 : mBuffer.ReadInt32();
	}
	else
	{
		mBuffer.WriteInt32(theReadSize);
	}

	if (mReading)
	{
		if (aReadSize != theReadSize || ByteLeftToRead() < theReadSize)
		{
			mFailed = true;
		}

		if (mFailed)
		{
			memset(theDest, 0, theReadSize);
		}
		else
		{
			mBuffer.ReadBytes((uchar*)theDest, theReadSize);
		}
	}
	else
	{
		mBuffer.WriteBytes((uchar*)theDest, theReadSize);
	}
}

void SaveGameContext::SyncInt32(int32_t& theInt32)
{
	if (mReading)
	{
		if (ByteLeftToRead() < 4)
		{
			mFailed = true;
		}

		theInt32 = mFailed ? 0 : mBuffer.ReadInt32();
	}
	else
	{
		mBuffer.WriteInt32(theInt32);
	}
}

void SaveGameContext::SyncUInt32(uint32_t& theUInt32)
{
	if (mReading)
	{
		if (ByteLeftToRead() < 4)
		{
			mFailed = true;
		}

		theUInt32 = mFailed ? 0 : mBuffer.ReadUInt32();
	}
	else
	{
		mBuffer.WriteUInt32(theUInt32);
	}
}

void SaveGameContext::SyncReanimationDef(ReanimatorDefinition*& theDefinition)
{
	if (mReading)
	{
		int aReanimType;
		SyncInt(aReanimType);
		if (aReanimType == static_cast<int>(ReanimationType::REANIM_NONE))
		{
			theDefinition = nullptr;
		}
		else if (aReanimType >= 0 && aReanimType < static_cast<int>(ReanimationType::NUM_REANIMS))
		{
			ReanimatorEnsureDefinitionLoaded(static_cast<ReanimationType>(aReanimType), true);
			theDefinition = &gReanimatorDefArray[aReanimType];
		}
		else
		{
			mFailed = true;
		}
	}
	else
	{
		int aReanimType = static_cast<int>(ReanimationType::REANIM_NONE);
		for (int i = 0; i < static_cast<int>(ReanimationType::NUM_REANIMS); i++)
		{
			ReanimatorDefinition* aDef = &gReanimatorDefArray[i];
			if (theDefinition == aDef)
			{
				aReanimType = i;
				break;
			}
		}
		SyncInt(aReanimType);
	}
}

void SaveGameContext::SyncParticleDef(PvzpParticleDefinition*& theDefinition)
{
	if (mReading)
	{
		int aParticleType;
		SyncInt(aParticleType);
		if (aParticleType == static_cast<int>(ParticleEffect::PARTICLE_NONE))
		{
			theDefinition = nullptr;
		}
		else if (aParticleType >= 0 && aParticleType < static_cast<int>(ParticleEffect::NUM_PARTICLES))
		{
			theDefinition = &gParticleDefArray[aParticleType];
		}
		else
		{
			mFailed = true;
		}
	}
	else
	{
		int aParticleType = static_cast<int>(ParticleEffect::PARTICLE_NONE);
		for (int i = 0; i < static_cast<int>(ParticleEffect::NUM_PARTICLES); i++)
		{
			PvzpParticleDefinition* aDef = &gParticleDefArray[i];
			if (theDefinition == aDef)
			{
				aParticleType = i;
				break;
			}
		}
		SyncInt(aParticleType);
	}
}

void SaveGameContext::SyncTrailDef(TrailDefinition*& theDefinition)
{
	if (mReading)
	{
		int aTrailType;
		SyncInt(aTrailType);
		if (aTrailType == TrailType::TRAIL_NONE)
		{
			theDefinition = nullptr;
		}
		else if (aTrailType >= 0 && aTrailType < TrailType::NUM_TRAILS)
		{
			theDefinition = &gTrailDefArray[aTrailType];
		}
		else
		{
			mFailed = true;
		}
	}
	else
	{
		int aTrailType = TrailType::TRAIL_NONE;
		for (int i = 0; i < TrailType::NUM_TRAILS; i++)
		{
			TrailDefinition* aDef = &gTrailDefArray[i];
			if (theDefinition == aDef)
			{
				aTrailType = i;
				break;
			}
		}
		SyncInt(aTrailType);
	}
}

void SaveGameContext::SyncImage(Image*& theImage)
{
	if (mReading)
	{
		ResourceId aResID;
		SyncInt((int&)aResID);
		if (aResID == Sexy::ResourceId::RESOURCE_ID_MAX)
		{
			theImage = nullptr;
		}
		else
		{
			theImage = GetImageById(aResID);
		}
	}
	else
	{
		ResourceId aResID;
		if (theImage != nullptr)
		{
			aResID = GetIdByImage(theImage);
		}
		else
		{
			aResID = Sexy::ResourceId::RESOURCE_ID_MAX;
		}
		SyncInt((int&)aResID);
	}
}

static void SyncDataIDList(PvzpList<uint32_t>* theDataIDList, SaveGameContext& theContext, PvzpAllocator* theAllocator)
{
	try
	{
		if (theContext.mReading)
		{
			if (theDataIDList)
			{
				theDataIDList->mHead = nullptr;
				theDataIDList->mTail = nullptr;
				theDataIDList->mSize = 0;
				theDataIDList->SetAllocator(theAllocator);
			}

			int aCount;
			theContext.SyncInt(aCount);
			for (int i = 0; i < aCount; i++)
			{
				uint32_t aDataID;
				theContext.SyncBytes(&aDataID, sizeof(aDataID));
				theDataIDList->AddTail(aDataID);
			}
		}
		else
		{
			int aCount = theDataIDList->mSize;
			theContext.SyncInt(aCount);
			for (PvzpListNode<uint32_t>* aNode = theDataIDList->mHead; aNode != nullptr; aNode = aNode->mNext)
			{
				uint32_t aDataID = aNode->mValue;
				theContext.SyncBytes(&aDataID, sizeof(aDataID));
			}
		}
	}
	catch (std::exception&)
	{
		return;
	}
}

static void SyncParticleEmitter(PvzpParticleSystem* theParticleSystem, PvzpParticleEmitter* theParticleEmitter, SaveGameContext& theContext)
{
	int aEmitterDefIndex = 0;
	if (theContext.mReading)
	{
		theContext.SyncInt(aEmitterDefIndex);
		theParticleEmitter->mParticleSystem = theParticleSystem;
		theParticleEmitter->mEmitterDef = &theParticleSystem->mParticleDef->mEmitterDefs[aEmitterDefIndex];
	}
	else
	{
		aEmitterDefIndex = (reinterpret_cast<intptr_t>(theParticleEmitter->mEmitterDef) -
			reinterpret_cast<intptr_t>(theParticleSystem->mParticleDef->mEmitterDefs)) / sizeof(PvzpEmitterDefinition);
		theContext.SyncInt(aEmitterDefIndex);
	}

	theContext.SyncImage(theParticleEmitter->mImageOverride);
	SyncDataIDList((PvzpList<uint32_t>*)&theParticleEmitter->mParticleList, theContext, &theParticleSystem->mParticleHolder->mParticleListNodeAllocator);
	for (PvzpListNode<ParticleID>* aNode = theParticleEmitter->mParticleList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		PvzpParticle* aParticle = theParticleSystem->mParticleHolder->mParticles.DataArrayGet(static_cast<uint32_t>(aNode->mValue));
		if (theContext.mReading)
		{
			aParticle->mParticleEmitter = theParticleEmitter;
		}
	}
}

static void SyncParticleSystem(Board* theBoard, PvzpParticleSystem* theParticleSystem, SaveGameContext& theContext)
{
	theContext.SyncParticleDef(theParticleSystem->mParticleDef);
	if (theContext.mReading)
	{
		theParticleSystem->mParticleHolder = theBoard->mApp->mEffectSystem->mParticleHolder.get();
	}

	SyncDataIDList((PvzpList<uint32_t>*)&theParticleSystem->mEmitterList, theContext, &theParticleSystem->mParticleHolder->mEmitterListNodeAllocator);
	for (PvzpListNode<ParticleEmitterID>* aNode = theParticleSystem->mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		PvzpParticleEmitter* aEmitter = theParticleSystem->mParticleHolder->mEmitters.DataArrayGet(static_cast<uint32_t>(aNode->mValue));
		SyncParticleEmitter(theParticleSystem, aEmitter, theContext);
	}
}

static void SyncReanimation(Board* theBoard, Reanimation* theReanimation, SaveGameContext& theContext)
{
	theContext.SyncReanimationDef(theReanimation->mDefinition);
	if (theContext.mReading)
	{
		theReanimation->mReanimationHolder = theBoard->mApp->mEffectSystem->mReanimationHolder.get();
	}

	if (theReanimation->mDefinition->mTracks.count != 0)
	{
		int aSize = theReanimation->mDefinition->mTracks.count * sizeof(ReanimatorTrackInstance);
		if (theContext.mReading)
		{
			theReanimation->mTrackInstances = (ReanimatorTrackInstance*)FindGlobalAllocator(aSize)->Calloc(aSize);
		}
		theContext.SyncBytes(theReanimation->mTrackInstances, aSize);

		for (int aTrackIndex = 0; aTrackIndex < theReanimation->mDefinition->mTracks.count; aTrackIndex++)
		{
			ReanimatorTrackInstance& aTrackInstance = theReanimation->mTrackInstances[aTrackIndex];
			theContext.SyncImage(aTrackInstance.mImageOverride);

			if (theContext.mReading)
			{
				aTrackInstance.mBlendTransform.mText = "";
				PVZP_ASSERT(aTrackInstance.mBlendTransform.mFont == nullptr);
				PVZP_ASSERT(aTrackInstance.mBlendTransform.mImage == nullptr);
			}
			else
			{
				PVZP_ASSERT(aTrackInstance.mBlendTransform.mText[0] == 0);
				PVZP_ASSERT(aTrackInstance.mBlendTransform.mFont == nullptr);
				PVZP_ASSERT(aTrackInstance.mBlendTransform.mImage == nullptr);
			}
		}
	}
}

static void SyncTrail(Board* theBoard, Trail* theTrail, SaveGameContext& theContext)
{
	theContext.SyncTrailDef(theTrail->mDefinition);
	if (theContext.mReading)
	{
		theTrail->mTrailHolder = theBoard->mApp->mEffectSystem->mTrailHolder.get();
	}
}

template <typename T>
struct LegacyDataArrayItem
{
	alignas(T) unsigned char mItem[sizeof(T)];
	unsigned int mID;
};

template <typename T> inline static void SyncDataArray(SaveGameContext& theContext, DataArray<T>& theDataArray)
{
	theContext.SyncUInt32(theDataArray.mFreeListHead);
	theContext.SyncUInt32(theDataArray.mMaxUsedCount);
	theContext.SyncUInt32(theDataArray.mSize);
	auto aBlock = std::make_unique<LegacyDataArrayItem<T>[]>(theDataArray.mMaxUsedCount);
	if (!theContext.mReading)
	{
		for (uint32_t i = 0; i < theDataArray.mMaxUsedCount; i++)
		{
			auto& aSlot = aBlock[i];
			std::copy_n(reinterpret_cast<unsigned char*>(&theDataArray.DataArrayGetItemAt(i)), sizeof(T), aSlot.mItem);
			aSlot.mID = theDataArray.DataArrayGetIDAt(i);
		}
	}
	theContext.SyncBytes(aBlock.get(), theDataArray.mMaxUsedCount * sizeof(aBlock[0]));
	if (!theContext.mReading)
		return;

	for (uint32_t i = 0; i < theDataArray.mMaxUsedCount; i++)
	{
		auto& aSlot = aBlock[i];
		theDataArray.DataArrayGetIDAt(i) = aSlot.mID;
		if (aSlot.mID & DATA_ARRAY_KEY_MASK)
			std::copy_n(aSlot.mItem, sizeof(T), reinterpret_cast<unsigned char*>(&theDataArray.DataArrayGetItemAt(i)));
	}
}

static void SyncBoard(SaveGameContext& theContext, Board* theBoard)
{
	size_t offset = size_t(&theBoard->mPaused) - size_t(theBoard);
	theContext.SyncBytes(&theBoard->mPaused, sizeof(Board) - offset);

	SyncDataArray(theContext, theBoard->mZombies);
	SyncDataArray(theContext, theBoard->mPlants);
	SyncDataArray(theContext, theBoard->mProjectiles);
	SyncDataArray(theContext, theBoard->mCoins);
	SyncDataArray(theContext, theBoard->mLawnMowers);
	SyncDataArray(theContext, theBoard->mGridItems);
	SyncDataArray(theContext, theBoard->mApp->mEffectSystem->mParticleHolder->mParticleSystems);
	SyncDataArray(theContext, theBoard->mApp->mEffectSystem->mParticleHolder->mEmitters);
	SyncDataArray(theContext, theBoard->mApp->mEffectSystem->mParticleHolder->mParticles);
	SyncDataArray(theContext, theBoard->mApp->mEffectSystem->mReanimationHolder->mReanimations);
	SyncDataArray(theContext, theBoard->mApp->mEffectSystem->mTrailHolder->mTrails);
	SyncDataArray(theContext, theBoard->mApp->mEffectSystem->mAttachmentHolder->mAttachments);

	{
		for (PvzpParticleSystem* aParticle : theBoard->mApp->mEffectSystem->mParticleHolder->mParticleSystems)
		{
			SyncParticleSystem(theBoard, aParticle, theContext);
		}
	}
	{
		for (Reanimation* aReanimation : theBoard->mApp->mEffectSystem->mReanimationHolder->mReanimations)
		{
			SyncReanimation(theBoard, aReanimation, theContext);
		}
	}
	{
		for (Trail* aTrail : theBoard->mApp->mEffectSystem->mTrailHolder->mTrails)
		{
			SyncTrail(theBoard, aTrail, theContext);
		}
	}

	theContext.SyncBytes(theBoard->mCursorObject.get(), sizeof(CursorObject));
	theContext.SyncBytes(theBoard->mCursorPreview.get(), sizeof(CursorPreview));
	theContext.SyncBytes(theBoard->mAdvice.get(), sizeof(MessageWidget));
	theContext.SyncBytes(theBoard->mSeedBank.get(), sizeof(SeedBank));
	theContext.SyncBytes(theBoard->mChallenge.get(), sizeof(Challenge));
	theContext.SyncBytes(theBoard->mApp->mMusic, sizeof(Music));

	if (theContext.mReading)
	{
		if (theContext.ByteLeftToRead() < 4)
		{
			theContext.mFailed = true;
		}

		if (theContext.mFailed || theContext.mBuffer.ReadUInt32() != SAVE_FILE_MAGIC_NUMBER)
		{
			theContext.mFailed = true;
		}
	}
	else
	{
		theContext.mBuffer.WriteUInt32(SAVE_FILE_MAGIC_NUMBER);
	}
}

bool LawnLoadGame(Board* theBoard, const std::string& theFilePath)
{
	if (LawnLoadGameV4(theBoard, theFilePath))
	{
		PvzpTrace("Loaded save game (v4)");
		return true;
	}

	SaveGameContext aContext;
	aContext.mFailed = false;
	aContext.mReading = true;
	if (!gSexyAppBase->ReadBufferFromFile(theFilePath, &aContext.mBuffer, false))
	{
		return false;
	}

	SaveFileHeader aHeader;
	aContext.SyncBytes(&aHeader, sizeof(aHeader));
	if (aHeader.mMagicNumber != SAVE_FILE_MAGIC_NUMBER || aHeader.mBuildVersion != SAVE_FILE_VERSION || aHeader.mBuildDate != SAVE_FILE_DATE)
	{
		return false;
	}

	SyncBoard(aContext, theBoard);
	if (aContext.mFailed)
	{
		return false;
	}

	PvzpTrace("Loaded save game (legacy)");
	FixBoardAfterLoad(theBoard);
	theBoard->mApp->mGameScene = GameScenes::SCENE_PLAYING;
	return true;
}

bool LawnSaveGame(Board* theBoard, const std::string& theFilePath)
{
	std::vector<unsigned char> aPayload;
	if (!WriteChunkV4(aPayload, SAVE4_CHUNK_BOARD_BASE, theBoard)) return false;
	if (!WriteChunkV4(aPayload, SAVE4_CHUNK_ZOMBIES, theBoard)) return false;
	if (!WriteChunkV4(aPayload, SAVE4_CHUNK_PLANTS, theBoard)) return false;
	if (!WriteChunkV4(aPayload, SAVE4_CHUNK_PROJECTILES, theBoard)) return false;
	if (!WriteChunkV4(aPayload, SAVE4_CHUNK_COINS, theBoard)) return false;
	if (!WriteChunkV4(aPayload, SAVE4_CHUNK_MOWERS, theBoard)) return false;
	if (!WriteChunkV4(aPayload, SAVE4_CHUNK_GRIDITEMS, theBoard)) return false;
	if (!WriteChunkV4(aPayload, SAVE4_CHUNK_PARTICLE_EMITTERS, theBoard)) return false;
	if (!WriteChunkV4(aPayload, SAVE4_CHUNK_PARTICLE_PARTICLES, theBoard)) return false;
	if (!WriteChunkV4(aPayload, SAVE4_CHUNK_PARTICLE_SYSTEMS, theBoard)) return false;
	if (!WriteChunkV4(aPayload, SAVE4_CHUNK_REANIMATIONS, theBoard)) return false;
	if (!WriteChunkV4(aPayload, SAVE4_CHUNK_TRAILS, theBoard)) return false;
	if (!WriteChunkV4(aPayload, SAVE4_CHUNK_ATTACHMENTS, theBoard)) return false;
	if (!WriteChunkV4(aPayload, SAVE4_CHUNK_CURSOR, theBoard)) return false;
	if (!WriteChunkV4(aPayload, SAVE4_CHUNK_CURSOR_PREVIEW, theBoard)) return false;
	if (!WriteChunkV4(aPayload, SAVE4_CHUNK_ADVICE, theBoard)) return false;
	if (!WriteChunkV4(aPayload, SAVE4_CHUNK_SEEDBANK, theBoard)) return false;
	if (!WriteChunkV4(aPayload, SAVE4_CHUNK_SEEDPACKETS, theBoard)) return false;
	if (!WriteChunkV4(aPayload, SAVE4_CHUNK_CHALLENGE, theBoard)) return false;
	if (!WriteChunkV4(aPayload, SAVE4_CHUNK_MUSIC, theBoard)) return false;

	SaveFileHeaderV4 aHeader{};
	memcpy(aHeader.mMagic, SAVE_FILE_MAGIC_V4, sizeof(aHeader.mMagic));
	aHeader.mVersion = ToLE32(SAVE_FILE_V4_VERSION);
	aHeader.mPayloadSize = ToLE32(static_cast<uint32_t>(aPayload.size()));
	aHeader.mPayloadCrc = ToLE32(crc32(0, reinterpret_cast<Bytef*>(aPayload.data()), static_cast<uint32_t>(aPayload.size())));

	std::vector<unsigned char> aOutBuffer;
	aOutBuffer.resize(sizeof(aHeader) + aPayload.size());
	memcpy(aOutBuffer.data(), &aHeader, sizeof(aHeader));
	memcpy(aOutBuffer.data() + sizeof(aHeader), aPayload.data(), aPayload.size());

	return gSexyAppBase->WriteBytesToFile(theFilePath, aOutBuffer.data(), static_cast<int>(aOutBuffer.size()));
}
