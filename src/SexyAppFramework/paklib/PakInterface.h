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

// [定义]资源包文件：包含了若干游戏资源的 .pak 文件。例如：main.pak
// [定义]资源文件：资源包文件中的一项具体资源的文件。例如：zombie_falling_1.ogg

// ====================================================================================================
// ★ 一个 PakRecord 实例对应资源包内的一个资源文件的数据，包括文件名，地址，大小等信息
// ====================================================================================================
class PakRecord
{
public:
	PakCollection*			mCollection;			// 指向该资源文件所在的资源包的 PakCollection
	std::string				mFileName;				// 资源文件的名称及路径（路径从 .pak 开始），例如 sounds\zombie_falling_1.ogg
	int64_t				mFileTime;				// 八字节型的资源文件的时间戳
	int						mStartPos;				// 该资源文件在资源包中的位置（即在 mCollection->mDataPtr 中的偏移量）
	int						mSize;					// 资源文件的大小，单位为 Byte（字节数）
};

typedef std::map<std::string, PakRecord> PakRecordMap;

// ====================================================================================================
// ★ 一个 PakCollection 实例对应一个 pak 资源包在内存中的映射文件
// ====================================================================================================
class PakCollection
{
public:
	//HANDLE					mFileHandle;
	//HANDLE					mMappingHandle;
	void*						mDataPtr;				// 资源包中的所有数据

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
	PakCollectionList		mPakCollectionList;		// 通过 AddPakFile() 添加的各个资源包的内存映射文件数据的链表
	PakRecordMap			mPakRecordMap;			// 所有已添加的资源包中的所有资源文件的、从文件名到文件数据的映射容器

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
