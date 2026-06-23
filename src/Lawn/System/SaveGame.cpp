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
#include "../../Sexy.TodLib/Trail.h"
#include "zlib.h"
#include "../../Sexy.TodLib/Attachment.h"
#include "../../Sexy.TodLib/Reanimator.h"
#include "../../Sexy.TodLib/TodParticle.h"
#include "../../Sexy.TodLib/EffectSystem.h"
#include "../../Sexy.TodLib/DataArray.h"
#include "../../Sexy.TodLib/TodList.h"
#include "DataSync.h"
#include "misc/Buffer.h"
#include <algorithm>
#include <cstdint>
#include <vector>

static constexpr const char* FILE_COMPILE_TIME_STRING = "Jul  2 201011:47:03"; // The compile time of 1.2.0.1073 GOTY
//static constexpr const char* FILE_COMPILE_TIME_STRING = "Dec 10 201014:56:46" // The compile time of 1.2.0.1096 GOTY Steam
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


template <typename TObject, typename TField>
static void SyncPodTail(PortableSaveContext& theContext, TObject& theObject, TField TObject::* theFirstField)
{
	unsigned char* aStart = reinterpret_cast<unsigned char*>(&theObject);
	unsigned char* aField = reinterpret_cast<unsigned char*>(&(theObject.*theFirstField));
	size_t aOffset = static_cast<size_t>(aField - aStart);
	if (aOffset < sizeof(TObject))
	{
		theContext.SyncBytes(aStart + aOffset, static_cast<uint32_t>(sizeof(TObject) - aOffset));
	}
}

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

static void SyncParticleDefPortable(PortableSaveContext& theContext, TodParticleDefinition*& theDefinition)
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
			TodParticleDefinition* aDef = &gParticleDefArray[i];
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

static void SyncDataIDListPortable(TodList<uint32_t>* theDataIDList, PortableSaveContext& theContext, TodAllocator* theAllocator)
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
			for (TodListNode<uint32_t>* aNode = theDataIDList->mHead; aNode != nullptr; aNode = aNode->mNext)
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

template <typename T>
static void ResetItemForRead(T& theItem)
{
	std::fill_n(reinterpret_cast<unsigned char*>(&theItem), sizeof(T), 0);
}

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

static void SyncTodSmoothArray(PortableSaveContext& theContext, TodSmoothArray& theArray)
{
	theContext.SyncInt32(theArray.mItem);
	theContext.SyncFloat(theArray.mWeight);
	theContext.SyncFloat(theArray.mLastPicked);
	theContext.SyncFloat(theArray.mSecondLastPicked);
}

static void SyncTodSmoothArrayList(PortableSaveContext& theContext, TodSmoothArray* theData, size_t theCount)
{
	for (size_t i = 0; i < theCount; i++)
		SyncTodSmoothArray(theContext, theData[i]);
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

template <typename TObject, typename TField>
static void WritePodTailField(std::vector<unsigned char>& theOut, uint32_t theFieldId, TObject& theObject, TField TObject::* theFirstField)
{
	AppendFieldWithSync(theOut, theFieldId, [&](PortableSaveContext& aContext)
	{
		SyncPodTail(aContext, theObject, theFirstField);
	});
}

template <typename TObject, typename TField>
static bool ReadPodTailField(const unsigned char* theData, size_t theSize, TObject& theObject, TField TObject::* theFirstField)
{
	return ApplyFieldWithSync(theData, theSize, [&](PortableSaveContext& aContext)
	{
		SyncPodTail(aContext, theObject, theFirstField);
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
	ReanimatorDefinition* aDefStart = gReanimatorDefArray;
	ReanimatorDefinition* aDefEnd = gReanimatorDefArray + static_cast<int>(ReanimationType::NUM_REANIMS);
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
			TodAllocator* aAllocator = FindGlobalAllocator(aCount * sizeof(ReanimatorTrackInstance));
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

static void SyncParticlePortable(TodParticle* theParticle, PortableSaveContext& theContext)
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

static void SyncParticleEmitterPortable(TodParticleSystem* theParticleSystem, TodParticleEmitter* theParticleEmitter, PortableSaveContext& theContext)
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
			reinterpret_cast<intptr_t>(theParticleSystem->mParticleDef->mEmitterDefs)) / sizeof(TodEmitterDefinition);
		theContext.SyncInt32(aEmitterDefIndex);
	}

	SyncDataIDListPortable((TodList<uint32_t>*)&theParticleEmitter->mParticleList, theContext, &theParticleSystem->mParticleHolder->mParticleListNodeAllocator);
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

	for (TodListNode<ParticleID>* aNode = theParticleEmitter->mParticleList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		TodParticle* aParticle = theParticleSystem->mParticleHolder->mParticles.DataArrayGet(static_cast<uint32_t>(aNode->mValue));
		if (theContext.mReading)
		{
			aParticle->mParticleEmitter = theParticleEmitter;
		}
		SyncParticlePortable(aParticle, theContext);
	}
}

static void SyncParticleSystemPortable(Board* theBoard, TodParticleSystem* theParticleSystem, PortableSaveContext& theContext)
{
	SyncParticleDefPortable(theContext, theParticleSystem->mParticleDef);
	if (theContext.mReading)
	{
		theParticleSystem->mParticleHolder = theBoard->mApp->mEffectSystem->mParticleHolder.get();
	}

	SyncDataIDListPortable((TodList<uint32_t>*)&theParticleSystem->mEmitterList, theContext, &theParticleSystem->mParticleHolder->mEmitterListNodeAllocator);
	for (TodListNode<ParticleEmitterID>* aNode = theParticleSystem->mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		TodParticleEmitter* aEmitter = theParticleSystem->mParticleHolder->mEmitters.DataArrayGet(static_cast<uint32_t>(aNode->mValue));
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
		theContext.SyncUInt32(theDataArray.mBlock[i].mID);
		theSyncFn(theDataArray.mBlock[i].mItem);
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
		theContext.SyncUInt32(theDataArray.mBlock[i].mID);
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
		theContext.SyncUInt32(theDataArray.mBlock[i].mID);
		if (theContext.mReading)
		{
			uint32_t aItemSize = 0;
			theContext.SyncUInt32(aItemSize);
			ResetItemForRead(theDataArray.mBlock[i].mItem);
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
				theReadFn(aFieldId, aFieldData, aFieldSize, theDataArray.mBlock[i].mItem);
			}
		}
		else
		{
			bool aActive = (theDataArray.mBlock[i].mID & DATA_ARRAY_KEY_MASK) != 0;
			uint32_t aItemSize = 0;
			std::vector<unsigned char> aItemData;
			if (aActive)
			{
				theWriteFn(aItemData, theDataArray.mBlock[i].mItem);
				aItemSize = static_cast<uint32_t>(aItemData.size());
			}
			theContext.SyncUInt32(aItemSize);
			if (aItemSize > 0)
				theContext.SyncBytes(aItemData.data(), aItemSize);
		}
	}
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
	BOARD_FIELD_CHOCOLATE_COLLECTED
};

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

			switch (aFieldId)
			{
			case BOARD_FIELD_PAUSED: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mPaused); }); break;
			case BOARD_FIELD_GRID_SQUARE_TYPE: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncEnum32Array(c, &theBoard->mGridSquareType[0][0], MAX_GRID_SIZE_X * MAX_GRID_SIZE_Y); }); break;
			case BOARD_FIELD_GRID_CEL_LOOK: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncInt32Array(c, &theBoard->mGridCelLook[0][0], MAX_GRID_SIZE_X * MAX_GRID_SIZE_Y); }); break;
			case BOARD_FIELD_GRID_CEL_OFFSET: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncInt32Array(c, &theBoard->mGridCelOffset[0][0][0], MAX_GRID_SIZE_X * MAX_GRID_SIZE_Y * 2); }); break;
			case BOARD_FIELD_GRID_CEL_FOG: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncInt32Array(c, &theBoard->mGridCelFog[0][0], MAX_GRID_SIZE_X * (MAX_GRID_SIZE_Y + 1)); }); break;
			case BOARD_FIELD_ENABLE_GRAVESTONES: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mEnableGraveStones); }); break;
			case BOARD_FIELD_SPECIAL_GRAVESTONE_X: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mSpecialGraveStoneX); }); break;
			case BOARD_FIELD_SPECIAL_GRAVESTONE_Y: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mSpecialGraveStoneY); }); break;
			case BOARD_FIELD_FOG_OFFSET: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncFloat(theBoard->mFogOffset); }); break;
			case BOARD_FIELD_FOG_BLOWN_COUNTDOWN: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mFogBlownCountDown); }); break;
			case BOARD_FIELD_PLANT_ROW: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncEnum32Array(c, &theBoard->mPlantRow[0], MAX_GRID_SIZE_Y); }); break;
			case BOARD_FIELD_WAVE_ROW_GOT_LAWN_MOWERED: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncInt32Array(c, &theBoard->mWaveRowGotLawnMowered[0], MAX_GRID_SIZE_Y); }); break;
			case BOARD_FIELD_BONUS_LAWN_MOWERS_REMAINING: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mBonusLawnMowersRemaining); }); break;
			case BOARD_FIELD_ICE_MIN_X: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncInt32Array(c, &theBoard->mIceMinX[0], MAX_GRID_SIZE_Y); }); break;
			case BOARD_FIELD_ICE_TIMER: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncInt32Array(c, &theBoard->mIceTimer[0], MAX_GRID_SIZE_Y); }); break;
			case BOARD_FIELD_ICE_PARTICLE_ID: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncEnumU32Array(c, &theBoard->mIceParticleID[0], MAX_GRID_SIZE_Y); }); break;
			case BOARD_FIELD_ROW_PICKING_ARRAY: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncTodSmoothArrayList(c, &theBoard->mRowPickingArray[0], MAX_GRID_SIZE_Y); }); break;
			case BOARD_FIELD_ZOMBIES_IN_WAVE: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncEnum32Array(c, &theBoard->mZombiesInWave[0][0], MAX_ZOMBIE_WAVES * MAX_ZOMBIES_IN_WAVE); }); break;
			case BOARD_FIELD_ZOMBIE_ALLOWED: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncBoolArray(c, &theBoard->mZombieAllowed[0], 100); }); break;
			case BOARD_FIELD_SUN_COUNTDOWN: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mSunCountDown); }); break;
			case BOARD_FIELD_NUM_SUNS_FALLEN: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mNumSunsFallen); }); break;
			case BOARD_FIELD_SHAKE_COUNTER: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mShakeCounter); }); break;
			case BOARD_FIELD_SHAKE_AMOUNT_X: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mShakeAmountX); }); break;
			case BOARD_FIELD_SHAKE_AMOUNT_Y: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mShakeAmountY); }); break;
			case BOARD_FIELD_BACKGROUND: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncEnum32(c, theBoard->mBackground); }); break;
			case BOARD_FIELD_LEVEL: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mLevel); }); break;
			case BOARD_FIELD_SOD_POSITION: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mSodPosition); }); break;
			case BOARD_FIELD_PREV_MOUSE_X: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mPrevMouseX); }); break;
			case BOARD_FIELD_PREV_MOUSE_Y: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mPrevMouseY); }); break;
			case BOARD_FIELD_SUN_MONEY: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mSunMoney); }); break;
			case BOARD_FIELD_NUM_WAVES: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mNumWaves); }); break;
			case BOARD_FIELD_MAIN_COUNTER: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mMainCounter); }); break;
			case BOARD_FIELD_EFFECT_COUNTER: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mEffectCounter); }); break;
			case BOARD_FIELD_DRAW_COUNT: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mDrawCount); }); break;
			case BOARD_FIELD_RISE_FROM_GRAVE_COUNTER: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mRiseFromGraveCounter); }); break;
			case BOARD_FIELD_OUT_OF_MONEY_COUNTER: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mOutOfMoneyCounter); }); break;
			case BOARD_FIELD_CURRENT_WAVE: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mCurrentWave); }); break;
			case BOARD_FIELD_TOTAL_SPAWNED_WAVES: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mTotalSpawnedWaves); }); break;
			case BOARD_FIELD_TUTORIAL_STATE: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncEnum32(c, theBoard->mTutorialState); }); break;
			case BOARD_FIELD_TUTORIAL_PARTICLE_ID: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncEnumU32(c, theBoard->mTutorialParticleID); }); break;
			case BOARD_FIELD_TUTORIAL_TIMER: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mTutorialTimer); }); break;
			case BOARD_FIELD_LAST_BUNGEE_WAVE: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mLastBungeeWave); }); break;
			case BOARD_FIELD_ZOMBIE_HEALTH_TO_NEXT_WAVE: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mZombieHealthToNextWave); }); break;
			case BOARD_FIELD_ZOMBIE_HEALTH_WAVE_START: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mZombieHealthWaveStart); }); break;
			case BOARD_FIELD_ZOMBIE_COUNTDOWN: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mZombieCountDown); }); break;
			case BOARD_FIELD_ZOMBIE_COUNTDOWN_START: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mZombieCountDownStart); }); break;
			case BOARD_FIELD_HUGE_WAVE_COUNTDOWN: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mHugeWaveCountDown); }); break;
			case BOARD_FIELD_HELP_DISPLAYED: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncBoolArray(c, &theBoard->mHelpDisplayed[0], NUM_ADVICE_TYPES); }); break;
			case BOARD_FIELD_HELP_INDEX: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncEnum32(c, theBoard->mHelpIndex); }); break;
			case BOARD_FIELD_FINAL_BOSS_KILLED: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mFinalBossKilled); }); break;
			case BOARD_FIELD_SHOW_SHOVEL: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mShowShovel); }); break;
			case BOARD_FIELD_COIN_BANK_FADE_COUNT: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mCoinBankFadeCount); }); break;
			case BOARD_FIELD_DEBUG_TEXT_MODE: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncEnum32(c, theBoard->mDebugTextMode); }); break;
			case BOARD_FIELD_LEVEL_COMPLETE: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mLevelComplete); }); break;
			case BOARD_FIELD_BOARD_FADE_OUT_COUNTER: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mBoardFadeOutCounter); }); break;
			case BOARD_FIELD_NEXT_SURVIVAL_STAGE_COUNTER: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mNextSurvivalStageCounter); }); break;
			case BOARD_FIELD_SCORE_NEXT_MOWER_COUNTER: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mScoreNextMowerCounter); }); break;
			case BOARD_FIELD_LEVEL_AWARD_SPAWNED: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mLevelAwardSpawned); }); break;
			case BOARD_FIELD_PROGRESS_METER_WIDTH: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mProgressMeterWidth); }); break;
			case BOARD_FIELD_FLAG_RAISE_COUNTER: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mFlagRaiseCounter); }); break;
			case BOARD_FIELD_ICE_TRAP_COUNTER: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mIceTrapCounter); }); break;
			case BOARD_FIELD_BOARD_RAND_SEED: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mBoardRandSeed); }); break;
			case BOARD_FIELD_POOL_SPARKLY_PARTICLE_ID: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncEnumU32(c, theBoard->mPoolSparklyParticleID); }); break;
			case BOARD_FIELD_FWOOSH_ID: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncEnumU32Array(c, &theBoard->mFwooshID[0][0], MAX_GRID_SIZE_Y * 12); }); break;
			case BOARD_FIELD_FWOOSH_COUNTDOWN: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mFwooshCountDown); }); break;
			case BOARD_FIELD_TIME_STOP_COUNTER: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mTimeStopCounter); }); break;
			case BOARD_FIELD_DROPPED_FIRST_COIN: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mDroppedFirstCoin); }); break;
			case BOARD_FIELD_FINAL_WAVE_SOUND_COUNTER: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mFinalWaveSoundCounter); }); break;
			case BOARD_FIELD_COB_CANNON_CURSOR_DELAY_COUNTER: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mCobCannonCursorDelayCounter); }); break;
			case BOARD_FIELD_COB_CANNON_MOUSE_X: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mCobCannonMouseX); }); break;
			case BOARD_FIELD_COB_CANNON_MOUSE_Y: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mCobCannonMouseY); }); break;
			case BOARD_FIELD_KILLED_YETI: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mKilledYeti); }); break;
			case BOARD_FIELD_MUSTACHE_MODE: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mMustacheMode); }); break;
			case BOARD_FIELD_SUPER_MOWER_MODE: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mSuperMowerMode); }); break;
			case BOARD_FIELD_FUTURE_MODE: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mFutureMode); }); break;
			case BOARD_FIELD_PINATA_MODE: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mPinataMode); }); break;
			case BOARD_FIELD_DANCE_MODE: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mDanceMode); }); break;
			case BOARD_FIELD_DAISY_MODE: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mDaisyMode); }); break;
			case BOARD_FIELD_SUKHBIR_MODE: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mSukhbirMode); }); break;
			case BOARD_FIELD_PREV_BOARD_RESULT: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncEnum32(c, theBoard->mPrevBoardResult); }); break;
			case BOARD_FIELD_TRIGGERED_LAWN_MOWERS: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mTriggeredLawnMowers); }); break;
			case BOARD_FIELD_PLAY_TIME_ACTIVE_LEVEL: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mPlayTimeActiveLevel); }); break;
			case BOARD_FIELD_PLAY_TIME_INACTIVE_LEVEL: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mPlayTimeInactiveLevel); }); break;
			case BOARD_FIELD_MAX_SUN_PLANTS: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mMaxSunPlants); }); break;
			case BOARD_FIELD_START_DRAW_TIME: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt64(theBoard->mStartDrawTime); }); break;
			case BOARD_FIELD_INTERVAL_DRAW_TIME: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt64(theBoard->mIntervalDrawTime); }); break;
			case BOARD_FIELD_INTERVAL_DRAW_COUNT_START: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mIntervalDrawCountStart); }); break;
			case BOARD_FIELD_MIN_FPS: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncFloat(theBoard->mMinFPS); }); break;
			case BOARD_FIELD_PRELOAD_TIME: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mPreloadTime); }); break;
			case BOARD_FIELD_GAME_ID: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ int64_t aGameId = static_cast<int64_t>(theBoard->mGameID); c.SyncInt64(aGameId); if (c.mReading) theBoard->mGameID = static_cast<intptr_t>(aGameId); }); break;
			case BOARD_FIELD_GRAVES_CLEARED: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mGravesCleared); }); break;
			case BOARD_FIELD_PLANTS_EATEN: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mPlantsEaten); }); break;
			case BOARD_FIELD_PLANTS_SHOVELED: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mPlantsShoveled); }); break;
			case BOARD_FIELD_PEA_SHOOTER_USED: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mPeaShooterUsed); }); break;
			case BOARD_FIELD_CATAPULT_PLANTS_USED: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mCatapultPlantsUsed); }); break;
			case BOARD_FIELD_MUSHROOM_AND_COFFEE_BEANS_ONLY: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mMushroomAndCoffeeBeansOnly); }); break;
			case BOARD_FIELD_MUSHROOMS_USED: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mMushroomsUsed); }); break;
			case BOARD_FIELD_LEVEL_COINS_COLLECTED: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mLevelCoinsCollected); }); break;
			case BOARD_FIELD_GARGANTUARS_KILLS_BY_CORN_COB: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mGargantuarsKillsByCornCob); }); break;
			case BOARD_FIELD_COINS_COLLECTED: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mCoinsCollected); }); break;
			case BOARD_FIELD_DIAMONDS_COLLECTED: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mDiamondsCollected); }); break;
			case BOARD_FIELD_POTTED_PLANTS_COLLECTED: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mPottedPlantsCollected); }); break;
			case BOARD_FIELD_CHOCOLATE_COLLECTED: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mChocolateCollected); }); break;
			default: break;
			}
		}
	}
	else
	{
		std::vector<unsigned char> aBlob;
		AppendFieldWithSync(aBlob, BOARD_FIELD_PAUSED, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mPaused); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_GRID_SQUARE_TYPE, [&](PortableSaveContext& c){ SyncEnum32Array(c, &theBoard->mGridSquareType[0][0], MAX_GRID_SIZE_X * MAX_GRID_SIZE_Y); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_GRID_CEL_LOOK, [&](PortableSaveContext& c){ SyncInt32Array(c, &theBoard->mGridCelLook[0][0], MAX_GRID_SIZE_X * MAX_GRID_SIZE_Y); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_GRID_CEL_OFFSET, [&](PortableSaveContext& c){ SyncInt32Array(c, &theBoard->mGridCelOffset[0][0][0], MAX_GRID_SIZE_X * MAX_GRID_SIZE_Y * 2); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_GRID_CEL_FOG, [&](PortableSaveContext& c){ SyncInt32Array(c, &theBoard->mGridCelFog[0][0], MAX_GRID_SIZE_X * (MAX_GRID_SIZE_Y + 1)); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_ENABLE_GRAVESTONES, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mEnableGraveStones); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_SPECIAL_GRAVESTONE_X, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mSpecialGraveStoneX); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_SPECIAL_GRAVESTONE_Y, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mSpecialGraveStoneY); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_FOG_OFFSET, [&](PortableSaveContext& c){ c.SyncFloat(theBoard->mFogOffset); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_FOG_BLOWN_COUNTDOWN, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mFogBlownCountDown); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_PLANT_ROW, [&](PortableSaveContext& c){ SyncEnum32Array(c, &theBoard->mPlantRow[0], MAX_GRID_SIZE_Y); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_WAVE_ROW_GOT_LAWN_MOWERED, [&](PortableSaveContext& c){ SyncInt32Array(c, &theBoard->mWaveRowGotLawnMowered[0], MAX_GRID_SIZE_Y); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_BONUS_LAWN_MOWERS_REMAINING, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mBonusLawnMowersRemaining); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_ICE_MIN_X, [&](PortableSaveContext& c){ SyncInt32Array(c, &theBoard->mIceMinX[0], MAX_GRID_SIZE_Y); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_ICE_TIMER, [&](PortableSaveContext& c){ SyncInt32Array(c, &theBoard->mIceTimer[0], MAX_GRID_SIZE_Y); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_ICE_PARTICLE_ID, [&](PortableSaveContext& c){ SyncEnumU32Array(c, &theBoard->mIceParticleID[0], MAX_GRID_SIZE_Y); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_ROW_PICKING_ARRAY, [&](PortableSaveContext& c){ SyncTodSmoothArrayList(c, &theBoard->mRowPickingArray[0], MAX_GRID_SIZE_Y); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_ZOMBIES_IN_WAVE, [&](PortableSaveContext& c){ SyncEnum32Array(c, &theBoard->mZombiesInWave[0][0], MAX_ZOMBIE_WAVES * MAX_ZOMBIES_IN_WAVE); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_ZOMBIE_ALLOWED, [&](PortableSaveContext& c){ SyncBoolArray(c, &theBoard->mZombieAllowed[0], 100); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_SUN_COUNTDOWN, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mSunCountDown); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_NUM_SUNS_FALLEN, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mNumSunsFallen); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_SHAKE_COUNTER, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mShakeCounter); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_SHAKE_AMOUNT_X, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mShakeAmountX); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_SHAKE_AMOUNT_Y, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mShakeAmountY); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_BACKGROUND, [&](PortableSaveContext& c){ SyncEnum32(c, theBoard->mBackground); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_LEVEL, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mLevel); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_SOD_POSITION, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mSodPosition); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_PREV_MOUSE_X, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mPrevMouseX); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_PREV_MOUSE_Y, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mPrevMouseY); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_SUN_MONEY, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mSunMoney); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_NUM_WAVES, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mNumWaves); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_MAIN_COUNTER, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mMainCounter); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_EFFECT_COUNTER, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mEffectCounter); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_DRAW_COUNT, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mDrawCount); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_RISE_FROM_GRAVE_COUNTER, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mRiseFromGraveCounter); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_OUT_OF_MONEY_COUNTER, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mOutOfMoneyCounter); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_CURRENT_WAVE, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mCurrentWave); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_TOTAL_SPAWNED_WAVES, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mTotalSpawnedWaves); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_TUTORIAL_STATE, [&](PortableSaveContext& c){ SyncEnum32(c, theBoard->mTutorialState); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_TUTORIAL_PARTICLE_ID, [&](PortableSaveContext& c){ SyncEnumU32(c, theBoard->mTutorialParticleID); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_TUTORIAL_TIMER, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mTutorialTimer); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_LAST_BUNGEE_WAVE, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mLastBungeeWave); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_ZOMBIE_HEALTH_TO_NEXT_WAVE, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mZombieHealthToNextWave); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_ZOMBIE_HEALTH_WAVE_START, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mZombieHealthWaveStart); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_ZOMBIE_COUNTDOWN, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mZombieCountDown); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_ZOMBIE_COUNTDOWN_START, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mZombieCountDownStart); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_HUGE_WAVE_COUNTDOWN, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mHugeWaveCountDown); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_HELP_DISPLAYED, [&](PortableSaveContext& c){ SyncBoolArray(c, &theBoard->mHelpDisplayed[0], NUM_ADVICE_TYPES); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_HELP_INDEX, [&](PortableSaveContext& c){ SyncEnum32(c, theBoard->mHelpIndex); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_FINAL_BOSS_KILLED, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mFinalBossKilled); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_SHOW_SHOVEL, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mShowShovel); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_COIN_BANK_FADE_COUNT, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mCoinBankFadeCount); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_DEBUG_TEXT_MODE, [&](PortableSaveContext& c){ SyncEnum32(c, theBoard->mDebugTextMode); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_LEVEL_COMPLETE, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mLevelComplete); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_BOARD_FADE_OUT_COUNTER, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mBoardFadeOutCounter); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_NEXT_SURVIVAL_STAGE_COUNTER, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mNextSurvivalStageCounter); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_SCORE_NEXT_MOWER_COUNTER, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mScoreNextMowerCounter); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_LEVEL_AWARD_SPAWNED, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mLevelAwardSpawned); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_PROGRESS_METER_WIDTH, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mProgressMeterWidth); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_FLAG_RAISE_COUNTER, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mFlagRaiseCounter); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_ICE_TRAP_COUNTER, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mIceTrapCounter); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_BOARD_RAND_SEED, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mBoardRandSeed); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_POOL_SPARKLY_PARTICLE_ID, [&](PortableSaveContext& c){ SyncEnumU32(c, theBoard->mPoolSparklyParticleID); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_FWOOSH_ID, [&](PortableSaveContext& c){ SyncEnumU32Array(c, &theBoard->mFwooshID[0][0], MAX_GRID_SIZE_Y * 12); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_FWOOSH_COUNTDOWN, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mFwooshCountDown); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_TIME_STOP_COUNTER, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mTimeStopCounter); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_DROPPED_FIRST_COIN, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mDroppedFirstCoin); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_FINAL_WAVE_SOUND_COUNTER, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mFinalWaveSoundCounter); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_COB_CANNON_CURSOR_DELAY_COUNTER, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mCobCannonCursorDelayCounter); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_COB_CANNON_MOUSE_X, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mCobCannonMouseX); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_COB_CANNON_MOUSE_Y, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mCobCannonMouseY); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_KILLED_YETI, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mKilledYeti); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_MUSTACHE_MODE, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mMustacheMode); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_SUPER_MOWER_MODE, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mSuperMowerMode); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_FUTURE_MODE, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mFutureMode); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_PINATA_MODE, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mPinataMode); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_DANCE_MODE, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mDanceMode); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_DAISY_MODE, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mDaisyMode); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_SUKHBIR_MODE, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mSukhbirMode); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_PREV_BOARD_RESULT, [&](PortableSaveContext& c){ SyncEnum32(c, theBoard->mPrevBoardResult); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_TRIGGERED_LAWN_MOWERS, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mTriggeredLawnMowers); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_PLAY_TIME_ACTIVE_LEVEL, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mPlayTimeActiveLevel); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_PLAY_TIME_INACTIVE_LEVEL, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mPlayTimeInactiveLevel); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_MAX_SUN_PLANTS, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mMaxSunPlants); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_START_DRAW_TIME, [&](PortableSaveContext& c){ c.SyncInt64(theBoard->mStartDrawTime); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_INTERVAL_DRAW_TIME, [&](PortableSaveContext& c){ c.SyncInt64(theBoard->mIntervalDrawTime); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_INTERVAL_DRAW_COUNT_START, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mIntervalDrawCountStart); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_MIN_FPS, [&](PortableSaveContext& c){ c.SyncFloat(theBoard->mMinFPS); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_PRELOAD_TIME, [&](PortableSaveContext& c){ c.SyncInt32(theBoard->mPreloadTime); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_GAME_ID, [&](PortableSaveContext& c){ int64_t aGameId = static_cast<int64_t>(theBoard->mGameID); c.SyncInt64(aGameId); if (c.mReading) theBoard->mGameID = static_cast<intptr_t>(aGameId); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_GRAVES_CLEARED, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mGravesCleared); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_PLANTS_EATEN, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mPlantsEaten); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_PLANTS_SHOVELED, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mPlantsShoveled); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_PEA_SHOOTER_USED, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mPeaShooterUsed); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_CATAPULT_PLANTS_USED, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mCatapultPlantsUsed); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_MUSHROOM_AND_COFFEE_BEANS_ONLY, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mMushroomAndCoffeeBeansOnly); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_MUSHROOMS_USED, [&](PortableSaveContext& c){ c.SyncBool(theBoard->mMushroomsUsed); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_LEVEL_COINS_COLLECTED, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mLevelCoinsCollected); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_GARGANTUARS_KILLS_BY_CORN_COB, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mGargantuarsKillsByCornCob); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_COINS_COLLECTED, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mCoinsCollected); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_DIAMONDS_COLLECTED, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mDiamondsCollected); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_POTTED_PLANTS_COLLECTED, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mPottedPlantsCollected); });
		AppendFieldWithSync(aBlob, BOARD_FIELD_CHOCOLATE_COLLECTED, [&](PortableSaveContext& c){ c.SyncUInt32(theBoard->mChocolateCollected); });
		WriteTLVBlob(theContext, aBlob);
	}
}

static void SyncZombiesPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncDataArrayPortableTLV(theContext, theBoard->mZombies,
		[&](std::vector<unsigned char>& aOut, Zombie& theZombie)
		{
			WriteGameObjectField(aOut, 1U, theZombie);
			AppendFieldWithSync(aOut, PORTABLE_FIELD_TAIL, [&](PortableSaveContext& c){ SyncZombieTailPortable(c, theZombie); });
		},
		[&](uint32_t aFieldId, const unsigned char* aData, size_t aSize, Zombie& theZombie)
		{
			switch (aFieldId)
			{
			case 1U: ReadGameObjectField(aData, aSize, theZombie); break;
			case 2U: ReadPodTailField(aData, aSize, theZombie, &Zombie::mZombieType); break; // legacy
			case PORTABLE_FIELD_TAIL: ApplyFieldWithSync(aData, aSize, [&](PortableSaveContext& c){ SyncZombieTailPortable(c, theZombie); }); break;
			default: break;
			}
		});
}

static void SyncPlantsPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncDataArrayPortableTLV(theContext, theBoard->mPlants,
		[&](std::vector<unsigned char>& aOut, Plant& thePlant)
		{
			WriteGameObjectField(aOut, 1U, thePlant);
			AppendFieldWithSync(aOut, PORTABLE_FIELD_TAIL, [&](PortableSaveContext& c){ SyncPlantTailPortable(c, thePlant); });
		},
		[&](uint32_t aFieldId, const unsigned char* aData, size_t aSize, Plant& thePlant)
		{
			switch (aFieldId)
			{
			case 1U: ReadGameObjectField(aData, aSize, thePlant); break;
			case 2U: ReadPodTailField(aData, aSize, thePlant, &Plant::mSeedType); break; // legacy
			case 3U: ApplyFieldWithSync(aData, aSize, [&](PortableSaveContext& c){ c.SyncEnum(thePlant.mSeedType); }); break; // legacy
			case 4U: ApplyFieldWithSync(aData, aSize, [&](PortableSaveContext& c){ c.SyncEnum(thePlant.mImitaterType); }); break; // legacy
			case 5U: ApplyFieldWithSync(aData, aSize, [&](PortableSaveContext& c){ c.SyncInt32(thePlant.mPottedPlantIndex); }); break; // legacy
			case PORTABLE_FIELD_TAIL: ApplyFieldWithSync(aData, aSize, [&](PortableSaveContext& c){ SyncPlantTailPortable(c, thePlant); }); break;
			default: break;
			}
		});
}

static void SyncProjectilesPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncDataArrayPortableTLV(theContext, theBoard->mProjectiles,
		[&](std::vector<unsigned char>& aOut, Projectile& theProjectile)
		{
			WriteGameObjectField(aOut, 1U, theProjectile);
			AppendFieldWithSync(aOut, PORTABLE_FIELD_TAIL, [&](PortableSaveContext& c){ SyncProjectileTailPortable(c, theProjectile); });
		},
		[&](uint32_t aFieldId, const unsigned char* aData, size_t aSize, Projectile& theProjectile)
		{
			switch (aFieldId)
			{
			case 1U: ReadGameObjectField(aData, aSize, theProjectile); break;
			case 2U: ReadPodTailField(aData, aSize, theProjectile, &Projectile::mMotionType); break; // legacy
			case 3U: ReadPodTailField(aData, aSize, theProjectile, &Projectile::mFrame); break; // legacy
			case PORTABLE_FIELD_TAIL: ApplyFieldWithSync(aData, aSize, [&](PortableSaveContext& c){ SyncProjectileTailPortable(c, theProjectile); }); break;
			default: break;
			}
		});
}

static void SyncCoinsPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncDataArrayPortableTLV(theContext, theBoard->mCoins,
		[&](std::vector<unsigned char>& aOut, Coin& theCoin)
		{
			WriteGameObjectField(aOut, 1U, theCoin);
			AppendFieldWithSync(aOut, PORTABLE_FIELD_TAIL, [&](PortableSaveContext& c){ SyncCoinTailPortable(c, theCoin); });
		},
		[&](uint32_t aFieldId, const unsigned char* aData, size_t aSize, Coin& theCoin)
		{
			switch (aFieldId)
			{
			case 1U: ReadGameObjectField(aData, aSize, theCoin); break;
			case 2U: ReadPodTailField(aData, aSize, theCoin, &Coin::mType); break; // legacy
			case 3U: ReadPodTailField(aData, aSize, theCoin, &Coin::mPosX); break; // legacy
			case PORTABLE_FIELD_TAIL: ApplyFieldWithSync(aData, aSize, [&](PortableSaveContext& c){ SyncCoinTailPortable(c, theCoin); }); break;
			default: break;
			}
		});
}

static void SyncMowersPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncDataArrayPortableTLV(theContext, theBoard->mLawnMowers,
		[&](std::vector<unsigned char>& aOut, LawnMower& theMower)
		{
			AppendFieldWithSync(aOut, PORTABLE_FIELD_TAIL, [&](PortableSaveContext& c){ SyncLawnMowerTailPortable(c, theMower); });
		},
		[&](uint32_t aFieldId, const unsigned char* aData, size_t aSize, LawnMower& theMower)
		{
			switch (aFieldId)
			{
			case 1U: ReadPodTailField(aData, aSize, theMower, &LawnMower::mPosX); break; // legacy
			case PORTABLE_FIELD_TAIL: ApplyFieldWithSync(aData, aSize, [&](PortableSaveContext& c){ SyncLawnMowerTailPortable(c, theMower); }); break;
			default: break;
			}
		});
}

static void SyncGridItemsPortable(PortableSaveContext& theContext, Board* theBoard)
{
	SyncDataArrayPortableTLV(theContext, theBoard->mGridItems,
		[&](std::vector<unsigned char>& aOut, GridItem& theItem)
		{
			AppendFieldWithSync(aOut, PORTABLE_FIELD_TAIL, [&](PortableSaveContext& c){ SyncGridItemTailPortable(c, theItem); });
		},
		[&](uint32_t aFieldId, const unsigned char* aData, size_t aSize, GridItem& theItem)
		{
			switch (aFieldId)
			{
			case 1U: ReadPodTailField(aData, aSize, theItem, &GridItem::mGridItemType); break; // legacy
			case PORTABLE_FIELD_TAIL: ApplyFieldWithSync(aData, aSize, [&](PortableSaveContext& c){ SyncGridItemTailPortable(c, theItem); }); break;
			default: break;
			}
		});
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
		[theBoard](std::vector<unsigned char>& aOut, TodParticleSystem& theSystem)
		{
			AppendFieldWithSync(aOut, 1U, [&](PortableSaveContext& aContext)
			{
				SyncParticleSystemPortable(theBoard, &theSystem, aContext);
			});
		},
		[theBoard](uint32_t aFieldId, const unsigned char* aData, size_t aSize, TodParticleSystem& theSystem)
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
	SyncDataArrayPortableTLV(theContext, theBoard->mApp->mEffectSystem->mAttachmentHolder->mAttachments,
		[&](std::vector<unsigned char>& aOut, Attachment& theAttachment)
		{
			AppendFieldWithSync(aOut, PORTABLE_FIELD_TAIL, [&](PortableSaveContext& c){ SyncAttachmentTailPortable(c, theAttachment); });
		},
		[&](uint32_t aFieldId, const unsigned char* aData, size_t aSize, Attachment& theAttachment)
		{
			switch (aFieldId)
			{
			case 1U: ReadPodTailField(aData, aSize, theAttachment, &Attachment::mEffectArray); break; // legacy
			case PORTABLE_FIELD_TAIL: ApplyFieldWithSync(aData, aSize, [&](PortableSaveContext& c){ SyncAttachmentTailPortable(c, theAttachment); }); break;
			default: break;
			}
		});
}

static void SyncCursorPortable(PortableSaveContext& theContext, Board* theBoard)
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
			switch (aFieldId)
			{
			case 1U: ReadGameObjectField(aFieldData, aFieldSize, *theBoard->mCursorObject); break;
			case 2U: ReadPodTailField(aFieldData, aFieldSize, *theBoard->mCursorObject, &CursorObject::mSeedBankIndex); break; // legacy
			case PORTABLE_FIELD_TAIL: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncCursorObjectTailPortable(c, *theBoard->mCursorObject); }); break;
			default: break;
			}
		}
	}
	else
	{
		std::vector<unsigned char> aBlob;
		WriteGameObjectField(aBlob, 1U, *theBoard->mCursorObject);
		AppendFieldWithSync(aBlob, PORTABLE_FIELD_TAIL, [&](PortableSaveContext& c){ SyncCursorObjectTailPortable(c, *theBoard->mCursorObject); });
		WriteTLVBlob(theContext, aBlob);
	}
}

static void SyncCursorPreviewPortable(PortableSaveContext& theContext, Board* theBoard)
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
			switch (aFieldId)
			{
			case 1U: ReadGameObjectField(aFieldData, aFieldSize, *theBoard->mCursorPreview); break;
			case 2U: ReadPodTailField(aFieldData, aFieldSize, *theBoard->mCursorPreview, &CursorPreview::mGridX); break; // legacy
			case PORTABLE_FIELD_TAIL: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncCursorPreviewTailPortable(c, *theBoard->mCursorPreview); }); break;
			default: break;
			}
		}
	}
	else
	{
		std::vector<unsigned char> aBlob;
		WriteGameObjectField(aBlob, 1U, *theBoard->mCursorPreview);
		AppendFieldWithSync(aBlob, PORTABLE_FIELD_TAIL, [&](PortableSaveContext& c){ SyncCursorPreviewTailPortable(c, *theBoard->mCursorPreview); });
		WriteTLVBlob(theContext, aBlob);
	}
}

static void SyncAdvicePortable(PortableSaveContext& theContext, Board* theBoard)
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
			switch (aFieldId)
			{
			case 1U: ReadPodTailField(aFieldData, aFieldSize, *theBoard->mAdvice, &MessageWidget::mLabel); break; // legacy
			case PORTABLE_FIELD_TAIL: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncMessageWidgetTailPortable(c, *theBoard->mAdvice); }); break;
			default: break;
			}
		}
	}
	else
	{
		std::vector<unsigned char> aBlob;
		AppendFieldWithSync(aBlob, PORTABLE_FIELD_TAIL, [&](PortableSaveContext& c){ SyncMessageWidgetTailPortable(c, *theBoard->mAdvice); });
		WriteTLVBlob(theContext, aBlob);
	}
}

static void SyncSeedBankPortable(PortableSaveContext& theContext, Board* theBoard)
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
			switch (aFieldId)
			{
			case 1U: ReadGameObjectField(aFieldData, aFieldSize, *theBoard->mSeedBank); break;
			case 2U: ReadPodTailField(aFieldData, aFieldSize, *theBoard->mSeedBank, &SeedBank::mNumPackets); break; // legacy
			case 3U: ReadPodTailField(aFieldData, aFieldSize, *theBoard->mSeedBank, &SeedBank::mCutSceneDarken); break; // legacy
			case 4U: ReadPodTailField(aFieldData, aFieldSize, *theBoard->mSeedBank, &SeedBank::mConveyorBeltCounter); break; // legacy
			case PORTABLE_FIELD_TAIL: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncSeedBankTailPortable(c, *theBoard->mSeedBank); }); break;
			default: break;
			}
		}
	}
	else
	{
		std::vector<unsigned char> aBlob;
		WriteGameObjectField(aBlob, 1U, *theBoard->mSeedBank);
		AppendFieldWithSync(aBlob, PORTABLE_FIELD_TAIL, [&](PortableSaveContext& c){ SyncSeedBankTailPortable(c, *theBoard->mSeedBank); });
		WriteTLVBlob(theContext, aBlob);
	}
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
				case 2U: ReadPodTailField(aFieldData, aFieldSize, theBoard->mSeedBank->mSeedPackets[i], &SeedPacket::mRefreshCounter); break; // legacy
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
			case 1U: ReadPodTailField(aFieldData, aFieldSize, *theBoard->mChallenge, &Challenge::mBeghouledMouseCapture); break; // legacy
			case PORTABLE_FIELD_TAIL: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncChallengeTailPortable(c, *theBoard->mChallenge); }); break;
			default: break;
			}
		}
	}
	else
	{
		std::vector<unsigned char> aBlob;
		AppendFieldWithSync(aBlob, PORTABLE_FIELD_TAIL, [&](PortableSaveContext& c){ SyncChallengeTailPortable(c, *theBoard->mChallenge); });
		WriteTLVBlob(theContext, aBlob);
	}
}

static void SyncMusicPortable(PortableSaveContext& theContext, Board* theBoard)
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
			switch (aFieldId)
			{
			case 1U: ReadPodTailField(aFieldData, aFieldSize, *theBoard->mApp->mMusic, &Music::mCurMusicTune); break; // legacy
			case PORTABLE_FIELD_TAIL: ApplyFieldWithSync(aFieldData, aFieldSize, [&](PortableSaveContext& c){ SyncMusicTailPortable(c, *theBoard->mApp->mMusic); }); break;
			default: break;
			}
		}
	}
	else
	{
		std::vector<unsigned char> aBlob;
		AppendFieldWithSync(aBlob, PORTABLE_FIELD_TAIL, [&](PortableSaveContext& c){ SyncMusicTailPortable(c, *theBoard->mApp->mMusic); });
		WriteTLVBlob(theContext, aBlob);
	}
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
	aChunkWriter.WriteUInt32(static_cast<uint32_t>(aFieldWriter.GetDataLen()));
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
		Plant* aPlant = nullptr;
		while (theBoard->mPlants.IterateNext(aPlant))
		{
			aPlant->mApp = theBoard->mApp;
			aPlant->mBoard = theBoard;
		}
	}
	{
		Zombie* aZombie = nullptr;
		while (theBoard->mZombies.IterateNext(aZombie))
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
		Projectile* aProjectile = nullptr;
		while (theBoard->mProjectiles.IterateNext(aProjectile))
		{
			aProjectile->mApp = theBoard->mApp;
			aProjectile->mBoard = theBoard;
		}
	}
	{
		Coin* aCoin = nullptr;
		while (theBoard->mCoins.IterateNext(aCoin))
		{
			aCoin->mApp = theBoard->mApp;
			aCoin->mBoard = theBoard;
		}
	}
	{
		LawnMower* aLawnMower = nullptr;
		while (theBoard->mLawnMowers.IterateNext(aLawnMower))
		{
			aLawnMower->mApp = theBoard->mApp;
			aLawnMower->mBoard = theBoard;
		}
	}
	{
		GridItem* aGridItem = nullptr;
		while (theBoard->mGridItems.IterateNext(aGridItem))
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
	theBoard->mApp->mMusic->mMusicInterface = theBoard->mApp->mMusicInterface;
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
	void			SyncInt(int& theInt);
	inline void		SyncUint(uint32_t& theInt) { SyncInt((signed int&)theInt); }
	void			SyncReanimationDef(ReanimatorDefinition*& theDefinition);
	void			SyncParticleDef(TodParticleDefinition*& theDefinition);
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

		aReadSize = mFailed ? 0 : mBuffer.ReadLong();
	}
	else
	{
		mBuffer.WriteLong(theReadSize);
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

void SaveGameContext::SyncInt(int& theInt)
{
	if (mReading)
	{
		if (ByteLeftToRead() < 4)
		{
			mFailed = true;
		}

		theInt = mFailed ? 0 : mBuffer.ReadLong();
	}
	else
	{
		mBuffer.WriteLong(theInt);
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

void SaveGameContext::SyncParticleDef(TodParticleDefinition*& theDefinition)
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
			TodParticleDefinition* aDef = &gParticleDefArray[i];
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

static void SyncDataIDList(TodList<uint32_t>* theDataIDList, SaveGameContext& theContext, TodAllocator* theAllocator)
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
			for (TodListNode<uint32_t>* aNode = theDataIDList->mHead; aNode != nullptr; aNode = aNode->mNext)
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

static void SyncParticleEmitter(TodParticleSystem* theParticleSystem, TodParticleEmitter* theParticleEmitter, SaveGameContext& theContext)
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
			reinterpret_cast<intptr_t>(theParticleSystem->mParticleDef->mEmitterDefs)) / sizeof(TodEmitterDefinition);
		theContext.SyncInt(aEmitterDefIndex);
	}

	theContext.SyncImage(theParticleEmitter->mImageOverride);
	SyncDataIDList((TodList<uint32_t>*)&theParticleEmitter->mParticleList, theContext, &theParticleSystem->mParticleHolder->mParticleListNodeAllocator);
	for (TodListNode<ParticleID>* aNode = theParticleEmitter->mParticleList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		TodParticle* aParticle = theParticleSystem->mParticleHolder->mParticles.DataArrayGet(static_cast<uint32_t>(aNode->mValue));
		if (theContext.mReading)
		{
			aParticle->mParticleEmitter = theParticleEmitter;
		}
	}
}

static void SyncParticleSystem(Board* theBoard, TodParticleSystem* theParticleSystem, SaveGameContext& theContext)
{
	theContext.SyncParticleDef(theParticleSystem->mParticleDef);
	if (theContext.mReading)
	{
		theParticleSystem->mParticleHolder = theBoard->mApp->mEffectSystem->mParticleHolder.get();
	}

	SyncDataIDList((TodList<uint32_t>*)&theParticleSystem->mEmitterList, theContext, &theParticleSystem->mParticleHolder->mEmitterListNodeAllocator);
	for (TodListNode<ParticleEmitterID>* aNode = theParticleSystem->mEmitterList.mHead; aNode != nullptr; aNode = aNode->mNext)
	{
		TodParticleEmitter* aEmitter = theParticleSystem->mParticleHolder->mEmitters.DataArrayGet(static_cast<uint32_t>(aNode->mValue));
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
				TOD_ASSERT(aTrackInstance.mBlendTransform.mFont == nullptr);
				TOD_ASSERT(aTrackInstance.mBlendTransform.mImage == nullptr);
			}
			else
			{
				TOD_ASSERT(aTrackInstance.mBlendTransform.mText[0] == 0);
				TOD_ASSERT(aTrackInstance.mBlendTransform.mFont == nullptr);
				TOD_ASSERT(aTrackInstance.mBlendTransform.mImage == nullptr);
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

template <typename T> inline static void SyncDataArray(SaveGameContext& theContext, DataArray<T>& theDataArray)
{
	theContext.SyncUint(theDataArray.mFreeListHead);
	theContext.SyncUint(theDataArray.mMaxUsedCount);
	theContext.SyncUint(theDataArray.mSize);
	theContext.SyncBytes(theDataArray.mBlock, theDataArray.mMaxUsedCount * sizeof(*theDataArray.mBlock));
}

static void SyncBoard(SaveGameContext& theContext, Board* theBoard)
{
	// TODO test if gives sane results
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
		TodParticleSystem* aParticle = nullptr;
		while (theBoard->mApp->mEffectSystem->mParticleHolder->mParticleSystems.IterateNext(aParticle))
		{
			SyncParticleSystem(theBoard, aParticle, theContext);
		}
	}
	{
		Reanimation* aReanimation = nullptr;
		while (theBoard->mApp->mEffectSystem->mReanimationHolder->mReanimations.IterateNext(aReanimation))
		{
			SyncReanimation(theBoard, aReanimation, theContext);
		}
	}
	{
		Trail* aTrail = nullptr;
		while (theBoard->mApp->mEffectSystem->mTrailHolder->mTrails.IterateNext(aTrail))
		{
			SyncTrail(theBoard, aTrail, theContext);
		}
	}

	theContext.SyncBytes(theBoard->mCursorObject, sizeof(CursorObject));
	theContext.SyncBytes(theBoard->mCursorPreview, sizeof(CursorPreview));
	theContext.SyncBytes(theBoard->mAdvice, sizeof(MessageWidget));
	theContext.SyncBytes(theBoard->mSeedBank, sizeof(SeedBank));
	theContext.SyncBytes(theBoard->mChallenge, sizeof(Challenge));
	theContext.SyncBytes(theBoard->mApp->mMusic, sizeof(Music));
	
	if (theContext.mReading)
	{
		if (theContext.ByteLeftToRead() < 4)
		{
			theContext.mFailed = true;
		}

		if (theContext.mFailed || static_cast<uint32_t>(theContext.mBuffer.ReadLong()) != SAVE_FILE_MAGIC_NUMBER)
		{
			theContext.mFailed = true;
		}
	}
	else
	{
		theContext.mBuffer.WriteLong(SAVE_FILE_MAGIC_NUMBER);
	}
}

// GOTY @Patoke: 0x48CBC0
bool LawnLoadGame(Board* theBoard, const std::string& theFilePath)
{
	if (LawnLoadGameV4(theBoard, theFilePath))
	{
		TodTrace("Loaded save game (v4)");
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

	TodTrace("Loaded save game (legacy)");
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
