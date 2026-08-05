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

#ifndef __PAKINTERFACE_H__
#define __PAKINTERFACE_H__

#include <map>
#include <list>
#include <string>
#include <string_view>
#include <cstdint>

class PakCollection;

// a .pak file (e.g. main.pak) packs individual resource files (e.g. zombie_falling_1.ogg)

// a PakRecord describes one resource file inside a pak: name, position, size, etc.
class PakRecord
{
public:
	PakCollection*			mCollection;			//+0x0
	std::string				mFileName;				//+0x4: path inside the pak, e.g. sounds\zombie_falling_1.ogg
	int64_t				mFileTime;				//+0x20: timestamp
	int						mStartPos;				//+0x28: offset of the file data in mCollection->mDataPtr
	int						mSize;					//+0x2C: size in bytes
};

typedef std::map<std::string, PakRecord> PakRecordMap;

// a PakCollection holds one pak file's data in memory
class PakCollection
{
public:
	void*						mDataPtr;				//+0x8: raw bytes of the whole pak

	explicit PakCollection(size_t size) { mDataPtr = malloc(size); }

	~PakCollection() { free(mDataPtr); }
};

typedef std::list<PakCollection> PakCollectionList;

struct PFILE
{
	PakRecord*				mRecord;
	int						mPos;
	FILE*					mFP;
};

class PakInterfaceBase
{
public:
	virtual PFILE*			FOpen(const char* theFileName, const char* theAccess) = 0;
	virtual int				FClose(PFILE* theFile) = 0;
	virtual int				FSeek(PFILE* theFile, long theOffset, int theOrigin) = 0;
	virtual int				FTell(PFILE* theFile) = 0;
	virtual size_t			FRead(void* thePtr, int theElemSize, int theCount, PFILE* theFile) = 0;
	virtual int				FGetC(PFILE* theFile) = 0;
	virtual int				UnGetC(int theChar, PFILE* theFile) = 0;
	virtual char*			FGetS(char* thePtr, int theSize, PFILE* theFile) = 0;
	virtual int				FEof(PFILE* theFile) = 0;
};

class PakInterface : public PakInterfaceBase
{
public:
	PakCollectionList		mPakCollectionList;		//+0x4: data of every pak added via AddPakFile()
	PakRecordMap			mPakRecordMap;			//+0x10: maps every resource file name to its record

	static std::string		NormalizePakPath(std::string_view theFileName);

public:

	PakInterface();
	~PakInterface();

	bool					AddPakFile(const std::string& theFileName);
	PFILE*					FOpen(const char* theFileName, const char* theAccess) override;
	int						FClose(PFILE* theFile) override;
	int						FSeek(PFILE* theFile, long theOffset, int theOrigin) override;
	int						FTell(PFILE* theFile) override;
	size_t					FRead(void* thePtr, int theElemSize, int theCount, PFILE* theFile) override;
	int						FGetC(PFILE* theFile) override;
	int						UnGetC(int theChar, PFILE* theFile) override;
	char*					FGetS(char* thePtr, int theSize, PFILE* theFile) override;
	int						FEof(PFILE* theFile) override;

};

extern PakInterface* gPakInterface;

[[maybe_unused]]
static PFILE* p_fopen(const char* theFileName, const char* theAccess)
{
	return gPakInterface->FOpen(theFileName, theAccess);
}

[[maybe_unused]]
static int p_fclose(PFILE* theFile)
{
	return gPakInterface->FClose(theFile);
}

[[maybe_unused]]
static int p_fseek(PFILE* theFile, long theOffset, int theOrigin)
{
	return gPakInterface->FSeek(theFile, theOffset, theOrigin);
}

[[maybe_unused]]
static int p_ftell(PFILE* theFile)
{
	return gPakInterface->FTell(theFile);
}

[[maybe_unused]]
static size_t p_fread(void* thePtr, int theSize, int theCount, PFILE* theFile)
{
	return gPakInterface->FRead(thePtr, theSize, theCount, theFile);
}

[[maybe_unused]]
static size_t p_fwrite(const void* thePtr, int theSize, int theCount, PFILE* theFile)
{
	if (theFile->mFP == nullptr)
		return 0;
	return fwrite(thePtr, theSize, theCount, theFile->mFP);
}

[[maybe_unused]]
static int p_fgetc(PFILE* theFile)
{
	return gPakInterface->FGetC(theFile);
}

[[maybe_unused]]
static int p_ungetc(int theChar, PFILE* theFile)
{
	return gPakInterface->UnGetC(theChar, theFile);
}

[[maybe_unused]]
static char* p_fgets(char* thePtr, int theSize, PFILE* theFile)
{
	return gPakInterface->FGetS(thePtr, theSize, theFile);
}

[[maybe_unused]]
static int p_feof(PFILE* theFile)
{
	return gPakInterface->FEof(theFile);
}

#endif //__PAKINTERFACE_H__
