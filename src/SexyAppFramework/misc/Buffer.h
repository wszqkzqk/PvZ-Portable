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

#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <string>
#include "Common.h"

namespace Sexy
{

typedef std::vector<uchar> ByteVector;

class Buffer
{
public:
	ByteVector				mData;
	int						mDataBitSize;
	mutable int				mReadBitPos;
	mutable int				mWriteBitPos;	

public:
	Buffer();
	virtual ~Buffer();
			
	void					SeekFront() const;
	void					Clear();

	void					FromWebString(std::string_view theString);
	void					WriteByte(uchar theByte);
	void					WriteNumBits(int theNum, int theBits);
	static int				GetBitsRequired(int theNum, bool isSigned);
	void					WriteBoolean(bool theBool);
	void					WriteShort(short theShort);
	void					WriteUInt32(uint32_t theValue);
	void					WriteInt32(int32_t theValue);
	void					WriteString(const std::string& theString);
	void					WriteLine(const std::string& theString);	
	void					WriteBuffer(const ByteVector& theBuffer);
	void					WriteBytes(const uchar* theByte, int theCount);
	void					SetData(const ByteVector& theBuffer);
	void					SetData(uchar* thePtr, int theCount);

	std::string				ToWebString() const;
	bool					ToUTF8String(std::string* theString) const;
	uchar					ReadByte() const;
	int						ReadNumBits(int theBits, bool isSigned) const;
	bool					ReadBoolean() const;
	short					ReadShort() const;
	uint32_t				ReadUInt32() const;
	int32_t					ReadInt32() const;
	std::string				ReadString() const;	
	std::string				ReadLine() const;
	void					ReadBytes(uchar* theData, int theLen) const;
	void					ReadBuffer(ByteVector* theByteVector) const;

	const uchar*			GetDataPtr() const;
	int						GetDataLen() const;	
	int						GetDataLenBits() const;
	uint32_t					GetCRC32(uint32_t theSeed = 0) const;

	bool					AtEnd() const;
	bool					PastEnd() const;
};

}

#endif //__BUFFER_H__
