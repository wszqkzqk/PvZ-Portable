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
#include "fcaseopen/fcaseopen.h"

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

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

// Normalize path for pak lookup.
std::string PakInterface::NormalizePakPath(std::string_view theFileName)
{
	std::filesystem::path aFilePath = Sexy::PathFromU8(theFileName);

	// Make rooted paths relative to resource folder.
	if (Sexy::IsPathRooted(theFileName))
	{
		const std::string& aResourceFolder = Sexy::GetResourceFolder();
		if (!aResourceFolder.empty())
		{
			std::filesystem::path aResPath = Sexy::PathFromU8(aResourceFolder);
			auto [aResEnd, aFileIt] = std::mismatch(aResPath.begin(), aResPath.end(),
			                                       aFilePath.begin(), aFilePath.end());
			if (aResEnd == aResPath.end())
			{
				std::filesystem::path aRelativePath;
				for (; aFileIt != aFilePath.end(); ++aFileIt)
					aRelativePath /= *aFileIt;
				aFilePath = aRelativePath;
			}
		}
	}

	std::string aResult = Sexy::PathToU8(aFilePath.lexically_normal());

	if (aResult.size() >= 2 && aResult[0] == '.' && aResult[1] == '/')
		aResult = aResult.substr(2);
	
	std::transform(aResult.begin(), aResult.end(), aResult.begin(),
	               [](unsigned char c) { return std::toupper(c); });
	
	return aResult;
}

bool PakInterface::AddPakFile(const std::string& theFileName)
{
	FILE *aFileHandle = fcaseopen(theFileName.c_str(), "rb");
	if (!aFileHandle)
		return false;

	fseek(aFileHandle, 0, SEEK_END);
	size_t aFileSize = ftell(aFileHandle);
	fseek(aFileHandle, 0, SEEK_SET);

	mPakCollectionList.emplace_back(aFileSize);
	PakCollection* aPakCollection = &mPakCollectionList.back();

	if (fread(aPakCollection->mDataPtr, 1, aFileSize, aFileHandle) != aFileSize)
	{
		fclose(aFileHandle);
		return false;
	}
	fclose(aFileHandle);

	auto *aDataPtr = static_cast<uint8_t *>(aPakCollection->mDataPtr);
	for (size_t i = 0; i < aFileSize; i++)
		*aDataPtr++ ^= 0xF7;

	std::string aPakKey = NormalizePakPath(theFileName);
	auto aRecordItr = mPakRecordMap.emplace(aPakKey, PakRecord()).first;
	PakRecord* aPakRecord = &aRecordItr->second;
	aPakRecord->mCollection = aPakCollection;
	aPakRecord->mFileName = aPakKey;
	aPakRecord->mStartPos = 0;
	aPakRecord->mSize = aFileSize;

	PFILE* aFP = FOpen(theFileName.c_str(), "rb");
	if (aFP == nullptr)
		return false;

	uint32_t aMagic = 0;
	FRead(&aMagic, sizeof(uint32_t), 1, aFP);
	aMagic = Sexy::FromLE32(aMagic);
	if (aMagic != 0xBAC04AC0)
	{
		FClose(aFP);
		return false;
	}

	uint32_t aVersion = 0;
	FRead(&aVersion, sizeof(uint32_t), 1, aFP);
	aVersion = Sexy::FromLE32(aVersion);
	if (aVersion > 0)
	{
		FClose(aFP);
		return false;
	}

	int aPos = 0;
	for (;;)
	{
		uchar aFlags = 0;
		int aCount = FRead(&aFlags, 1, 1, aFP);
		if ((aFlags & FILEFLAGS_END) || (aCount == 0))
			break;

		uchar aNameWidth = 0;
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

		for (int i = 0; i < aNameWidth; i++)
		{
			if (aName[i] == '\\')
				aName[i] = '/';
		}

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
	if ((strcasecmp(anAccess, "r") == 0) || (strcasecmp(anAccess, "rb") == 0) || (strcasecmp(anAccess, "rt") == 0))
	{
		std::string aKey = NormalizePakPath(theFileName);
		auto anItr = mPakRecordMap.find(aKey);
		if (anItr != mPakRecordMap.end())
		{
			PFILE* aPFP = new PFILE;
			aPFP->mRecord = &anItr->second;
			aPFP->mPos = 0;
			aPFP->mFP = nullptr;
			return aPFP;
		}
	}

	const std::string& aResourceBase = Sexy::GetResourceFolder();
	FILE* aFP = nullptr;
	if (!aResourceBase.empty() && !Sexy::IsPathRooted(theFileName))
	{
		aFP = fcaseopenat(aResourceBase.c_str(), theFileName, anAccess);
	}
	else
	{
		aFP = fcaseopen(theFileName, anAccess);
	}

	if (aFP == nullptr)
		return nullptr;

	PFILE* aPFP = new PFILE;
	aPFP->mRecord = nullptr;
	aPFP->mPos = 0;
	aPFP->mFP = aFP;
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

		// 当前指针位置不能超过整个文件的大小，且不能小于 0
		theFile->mPos = std::clamp(theFile->mPos, 0, theFile->mRecord->mSize);
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

size_t PakInterface::FRead(void* thePtr, int theElemSize, int theCount, PFILE* theFile)
{
	if (theFile->mRecord != nullptr)
	{
		// 实际读取的字节数不能超过当前资源文件剩余可读取的字节数
		int aSizeBytes = std::min(theElemSize*theCount, theFile->mRecord->mSize - theFile->mPos);

		// 取得在整个 pak 中开始读取的位置的指针
		uchar* src = (uchar*) theFile->mRecord->mCollection->mDataPtr + theFile->mRecord->mStartPos + theFile->mPos;
		uchar* dest = (uchar*) thePtr;
		memcpy(dest, src, aSizeBytes);
		theFile->mPos += aSizeBytes;  // 读取完成后，移动当前读取位置的指针
		return aSizeBytes / theElemSize;  // 返回实际读取的项数
	}
	
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
			char aChar = *((char*) theFile->mRecord->mCollection->mDataPtr + theFile->mRecord->mStartPos + theFile->mPos++);
			if (aChar != '\r')
				return (uchar) aChar;
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

char* PakInterface::FGetS(char* thePtr, int theSize, PFILE* theFile)
{
	if (theFile->mRecord != nullptr)
	{
		int anIdx = 0;
		while (anIdx < theSize)
		{
			if (theFile->mPos >= theFile->mRecord->mSize)
			{
				if (anIdx == 0)
					return nullptr;
				break;
			}
			char aChar = *((char*) theFile->mRecord->mCollection->mDataPtr + theFile->mRecord->mStartPos + theFile->mPos++);
			if (aChar != '\r')
				thePtr[anIdx++] = aChar;
			if (aChar == '\n')
				break;
		}
		thePtr[anIdx] = 0;
		return thePtr;
	}

	return fgets(thePtr, theSize, theFile->mFP);
}

int PakInterface::FEof(PFILE* theFile)
{
	if (theFile->mRecord != nullptr)
		return theFile->mPos >= theFile->mRecord->mSize;
	else
		return feof(theFile->mFP);
}
