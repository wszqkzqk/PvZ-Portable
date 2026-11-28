/*
 * Portions of this file are based on the PopCap Games Framework
 * Copyright (C) 2005-2009 PopCap Games, Inc.
 *
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
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

#include <algorithm>
#include <filesystem>
#include "Common.h"
#include "PakInterface.h"

enum
{
	FILEFLAGS_END = 0x80
};

PakInterface* gPakInterface = new PakInterface();

PakInterface::PakInterface()
{
}

PakInterface::~PakInterface()
{
}

// Normalize path for pak lookup: forward slashes, lexically resolved, relative to resource folder.
std::string PakInterface::NormalizePakPath(std::string_view theFileName)
{
	if (Sexy::IsPathRooted(theFileName) && !Sexy::GetResourceFolder().empty())
	{
		const auto aResourceRoot = Sexy::PathFromU8(Sexy::GetResourceFolder());
		const auto aRel = Sexy::PathFromU8(theFileName).lexically_normal().lexically_relative(aResourceRoot);
		if (!aRel.empty() && *aRel.begin() != "..")
			return Sexy::CanonicalizePath(Sexy::PathToU8(aRel));
	}

	return Sexy::CanonicalizePath(theFileName);
}

const PakRecord* PakInterface::FindRecord(std::string_view theFileName) const
{
	if (theFileName.empty())
		return nullptr;

	if (Sexy::IsCanonicalRelativePath(theFileName))
	{
		auto anItr = mPakRecordMap.find(theFileName);
		return anItr != mPakRecordMap.end() ? &anItr->second : nullptr;
	}

	auto anItr = mPakRecordMap.find(NormalizePakPath(theFileName));
	return anItr != mPakRecordMap.end() ? &anItr->second : nullptr;
}

bool PakInterface::AddPakFile(const std::string& theFileName)
{
	FILE *aFileHandle = Sexy::OpenResourceFile(theFileName, "rb");
	if (!aFileHandle)
		return false;

	fseek(aFileHandle, 0, SEEK_END);
	size_t aFileSize = ftell(aFileHandle);
	fseek(aFileHandle, 0, SEEK_SET);

#ifdef LOW_MEMORY
	mPakCollectionList.emplace_back(aFileHandle);
#else
	mPakCollectionList.emplace_back(aFileSize);
#endif
	PakCollection* aPakCollection = &mPakCollectionList.back();

#ifndef LOW_MEMORY
	if (fread(aPakCollection->mDataPtr, 1, aFileSize, aFileHandle) != aFileSize)
	{
		fclose(aFileHandle);
		return false;
	}
	fclose(aFileHandle);

	auto *aDataPtr = static_cast<uint8_t *>(aPakCollection->mDataPtr);
	for (size_t i = 0; i < aFileSize; i++)
		*aDataPtr++ ^= 0xF7;
#endif

	std::string aPakKey = NormalizePakPath(theFileName);
	auto aEmplaceResult = mPakRecordMap.emplace(aPakKey, PakRecord());
	PakRecord* aPakRecord = &aEmplaceResult.first->second;
	aPakRecord->mCollection = aPakCollection;
	aPakRecord->mFileName = aPakKey;
	aPakRecord->mStartPos = 0;
	aPakRecord->mSize = aFileSize;

	PFILE* aFP = FOpen(theFileName.c_str(), "rb");
	if (aFP == nullptr)
	{
#ifdef LOW_MEMORY
		if (aEmplaceResult.second)
		{
			mPakRecordMap.erase(aPakKey);
			mPakCollectionList.pop_back();
		}
#endif
		return false;
	}

	uint32_t aMagic = 0;
	FRead(&aMagic, sizeof(uint32_t), 1, aFP);
	aMagic = Sexy::FromLE32(aMagic);
	if (aMagic != 0xBAC04AC0)
	{
		FClose(aFP);
#ifdef LOW_MEMORY
		if (aEmplaceResult.second)
		{
			mPakRecordMap.erase(aPakKey);
			mPakCollectionList.pop_back();
		}
#endif
		return false;
	}

	uint32_t aVersion = 0;
	FRead(&aVersion, sizeof(uint32_t), 1, aFP);
	aVersion = Sexy::FromLE32(aVersion);
	if (aVersion > 0)
	{
		FClose(aFP);
#ifdef LOW_MEMORY
		if (aEmplaceResult.second)
		{
			mPakRecordMap.erase(aPakKey);
			mPakCollectionList.pop_back();
		}
#endif
		return false;
	}

	int aPos = 0;
	for (;;)
	{
		uint8_t aFlags = 0;
		int aCount = FRead(&aFlags, 1, 1, aFP);
		if ((aFlags & FILEFLAGS_END) || (aCount == 0))
			break;

		uint8_t aNameWidth = 0;
		char aName[256];
		FRead(&aNameWidth, 1, 1, aFP);
		FRead(aName, 1, aNameWidth, aFP);
		aName[aNameWidth] = 0;

		int aSrcSize = 0;
		FRead(&aSrcSize, sizeof(int), 1, aFP);
		aSrcSize = static_cast<int>(Sexy::FromLE32(static_cast<uint32_t>(aSrcSize)));
		int64_t aFileTime;
		FRead(&aFileTime, sizeof(int64_t), 1, aFP);
		aFileTime = static_cast<int64_t>(Sexy::FromLE64(static_cast<uint64_t>(aFileTime)));

		std::string aKey = NormalizePakPath(aName);
		auto aRecordItr = mPakRecordMap.emplace(aKey, PakRecord()).first;
		PakRecord* aPakRecord = &aRecordItr->second;
		aPakRecord->mCollection = aPakCollection;
		aPakRecord->mFileName = aKey;
		aPakRecord->mStartPos = aPos;
		aPakRecord->mSize = aSrcSize;
		aPakRecord->mFileTime = aFileTime;

		aPos += aSrcSize;
	}

	int anOffset = FTell(aFP);

	for (auto& [key, record] : mPakRecordMap)
	{
		if (record.mCollection == aPakCollection)
			record.mStartPos += anOffset;
	}

	FClose(aFP);
	return true;
}

PFILE* PakInterface::FOpen(const char* theFileName, const char* anAccess)
{
	if (theFileName == nullptr || anAccess == nullptr)
		return nullptr;

	if (Sexy::StrEqualsIgnoreCase(anAccess, "r") || Sexy::StrEqualsIgnoreCase(anAccess, "rb") || Sexy::StrEqualsIgnoreCase(anAccess, "rt"))
	{
		if (const PakRecord* aRecord = FindRecord(theFileName))
		{
			PFILE* aPFP = new PFILE;
			aPFP->mRecord = aRecord;
			aPFP->mPos = 0;
			aPFP->mFP = nullptr;
#ifdef LOW_MEMORY
			aPFP->mBufferStart = 0;
			aPFP->mBufferLen = 0;
#endif
			return aPFP;
		}
	}

	FILE* aFP = Sexy::OpenResourceFile(theFileName, anAccess);
	if (aFP == nullptr)
		return nullptr;

	PFILE* aPFP = new PFILE;
	aPFP->mRecord = nullptr;
	aPFP->mPos = 0;
	aPFP->mFP = aFP;
#ifdef LOW_MEMORY
	aPFP->mBufferStart = 0;
	aPFP->mBufferLen = 0;
#endif
	return aPFP;
}

int PakInterface::FClose(PFILE* theFile)
{
	if (theFile->mRecord == nullptr)
		fclose(theFile->mFP);
	delete theFile;
	return 0;
}

int PakInterface::FSeek(PFILE* theFile, long theOffset, int theOrigin)
{
	if (theFile->mRecord != nullptr)
	{
		if (theOrigin == SEEK_SET)
			theFile->mPos = theOffset;
		else if (theOrigin == SEEK_END)
			theFile->mPos = theFile->mRecord->mSize - theOffset;
		else if (theOrigin == SEEK_CUR)
			theFile->mPos += theOffset;

		theFile->mPos = std::clamp(theFile->mPos, 0, theFile->mRecord->mSize);
#ifdef LOW_MEMORY
		theFile->mBufferLen = 0;
#endif
		return 0;
	}
	else
		return fseek(theFile->mFP, theOffset, theOrigin);
}

int PakInterface::FTell(PFILE* theFile)
{
	if (theFile->mRecord != nullptr)
		return theFile->mPos;
	else
		return ftell(theFile->mFP);
}

static size_t ReadRecordBytes(PFILE* theFile, void* thePtr, int theSize)
{
#ifdef LOW_MEMORY
	if (theSize >= PFILE::BUFFER_SIZE)
	{
		int aSizeBytes = std::min(theSize, theFile->mRecord->mSize - theFile->mPos);
		size_t aRead = theFile->mRecord->mCollection->ReadAt(thePtr, theFile->mRecord->mStartPos + theFile->mPos, aSizeBytes);
		theFile->mPos += (int) aRead;
		theFile->mBufferLen = 0;
		return aRead;
	}

	auto* aDest = static_cast<uint8_t*>(thePtr);
	size_t aDone = 0;
	while (aDone < static_cast<size_t>(theSize))
	{
		int aBufOffset = theFile->mPos - theFile->mBufferStart;
		if (aBufOffset < 0 || aBufOffset >= theFile->mBufferLen)
		{
			int aFill = std::min(theFile->mRecord->mSize - theFile->mPos, PFILE::BUFFER_SIZE);
			if (aFill <= 0)
				break;
			theFile->mBufferStart = theFile->mPos;
			theFile->mBufferLen = (int) theFile->mRecord->mCollection->ReadAt(theFile->mBuffer.data(), theFile->mRecord->mStartPos + theFile->mPos, aFill);
			if (theFile->mBufferLen == 0)
				break;
			aBufOffset = 0;
		}
		int aChunk = std::min(theFile->mBufferLen - aBufOffset, (int) (theSize - aDone));
		memcpy(aDest + aDone, theFile->mBuffer.data() + aBufOffset, aChunk);
		theFile->mPos += aChunk;
		aDone += aChunk;
	}
	return aDone;
#else
	int aSizeBytes = std::min(theSize, theFile->mRecord->mSize - theFile->mPos);

	uint8_t* src = (uint8_t*) theFile->mRecord->mCollection->mDataPtr + theFile->mRecord->mStartPos + theFile->mPos;
	memcpy(thePtr, src, aSizeBytes);
	theFile->mPos += aSizeBytes;
	return aSizeBytes;
#endif
}

size_t PakInterface::FRead(void* thePtr, int theElemSize, int theCount, PFILE* theFile)
{
	if (theFile->mRecord != nullptr)
		return ReadRecordBytes(theFile, thePtr, theElemSize*theCount) / theElemSize;

	return fread(thePtr, theElemSize, theCount, theFile->mFP);
}

int PakInterface::FGetC(PFILE* theFile)
{
	if (theFile->mRecord != nullptr)
	{
		for (;;)
		{
			if (theFile->mPos >= theFile->mRecord->mSize)
				return EOF;
			char aChar;
			if (ReadRecordBytes(theFile, &aChar, 1) == 0)
				return EOF;
			if (aChar != '\r')
				return (uint8_t) aChar;
		}
	}

	return fgetc(theFile->mFP);
}

int PakInterface::UnGetC(int theChar, PFILE* theFile)
{
	if (theFile->mRecord != nullptr)
	{
		// This won't work if we're not pushing the same chars back in the stream
		theFile->mPos = std::max(theFile->mPos - 1, 0);
		return theChar;
	}

	return ungetc(theChar, theFile->mFP);
}
