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

#ifndef __DATASYNC_H__
#define __DATASYNC_H__

#include <memory>
#include <type_traits>
#include "../../SexyAppFramework/Common.h"

class DataReader
{
protected:
	FILE*					mFile;
	const uint8_t*			mData;
	uint32_t				mDataLen;
	uint32_t				mDataPos;
	std::unique_ptr<uint8_t[]>	mOwnedData;

public:
	DataReader();
	DataReader(const DataReader& theDataReader) = delete;
	DataReader& operator=(const DataReader& theDataReader) = delete;
	virtual ~DataReader();

	bool					OpenFile(const std::string& theFileName);
	void					OpenMemory(const void* theData, uint32_t theDataLen, bool takeOwnership);
	void					Close();
	void					ReadBytes(void* theMem, uint32_t theNumBytes);
	void					Rewind(uint32_t theNumBytes);
	uint64_t				ReadUInt64();
	uint32_t				ReadUInt32();
	uint16_t				ReadUInt16();
	uint8_t					ReadUInt8();
	bool					ReadBool();
	float					ReadFloat();
	double					ReadDouble();
	void					ReadString(std::string& theStr);
};
class DataReaderException : public std::exception
{
};

class DataWriter
{
protected:
	FILE*					mFile;
	std::vector<uint8_t>		mData;

public:
	DataWriter();
	DataWriter(const DataWriter& theDataWriter) = delete;
	DataWriter& operator=(const DataWriter& theDataWriter) = delete;
	virtual ~DataWriter();

	bool					OpenFile(const std::string& theFileName);
	void					OpenMemory(uint32_t theReserveAmount = 0x20);
	void					Close();
	inline bool				WriteToFile(const std::string& theFileName);
	void					WriteBytes(const void* theData, uint32_t theDataLen);
	void					WriteUInt64(uint64_t theUInt64);
	void					WriteUInt32(uint32_t theUInt32);
	void					WriteUInt16(uint16_t theUInt16);
	void					WriteUInt8(uint8_t theUInt8);
	void					WriteBool(bool theBool);
	void					WriteFloat(float theFloat);
	void					WriteDouble(double theDouble);
	void					WriteString(std::string_view theStr);
	inline uint32_t			GetPos();
	inline void*			GetDataPtr() { return mData.data(); }
	inline uint32_t			GetDataLen() { return static_cast<uint32_t>(mData.size()); }
};

class DataSync
{
protected:
	DataReader*				mReader;
	DataWriter*				mWriter;
	int						mVersion;

protected:
	void Reset();

public:
	DataSync(DataReader& theReader);
	DataSync(DataWriter& theWriter);
	virtual ~DataSync();

	inline void				SetReader(DataReader* theReader) { mReader = theReader; }
	inline void				SetWriter(DataWriter* theWriter) { mWriter = theWriter; }
	inline DataReader*		GetReader() { return mReader; }
	inline DataWriter*		GetWriter() { return mWriter; }
	void					SyncBytes(void* theData, uint32_t theDataLen);
	void					SyncUInt64(uint64_t& theNum);
	void					SyncUInt32(uint32_t& theNum);
	template <typename T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, int> = 0>
	void					SyncUInt32(T& theNum)
	{
		uint32_t aNum = static_cast<uint32_t>(static_cast<std::make_unsigned_t<T>>(theNum));
		SyncUInt32(aNum);
		if (mReader)
			theNum = static_cast<T>(aNum);
	}
	template<typename E, std::enable_if_t<std::is_enum_v<E>, int> = 0>
	void					SyncUInt32(E& theEnum) { SyncUInt32(reinterpret_cast<std::underlying_type_t<E>&>(theEnum)); }
	void					SyncUInt16(uint16_t& theNum);
	template <typename T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, int> = 0>
	void					SyncUInt16(T& theNum)
	{
		uint16_t aNum = static_cast<uint16_t>(static_cast<std::make_unsigned_t<T>>(theNum));
		SyncUInt16(aNum);
		if (mReader)
			theNum = static_cast<T>(aNum);
	}
	template<typename E, std::enable_if_t<std::is_enum_v<E>, int> = 0>
	void					SyncUInt16(E& theEnum) { SyncUInt16(reinterpret_cast<std::underlying_type_t<E>&>(theEnum)); }
	void					SyncUInt8(uint8_t& theChar);
	template <typename T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, int> = 0>
	void					SyncUInt8(T& theChar)
	{
		uint8_t aChar = static_cast<uint8_t>(static_cast<std::make_unsigned_t<T>>(theChar));
		SyncUInt8(aChar);
		if (mReader)
			theChar = static_cast<T>(aChar);
	}
	template<typename E, std::enable_if_t<std::is_enum_v<E>, int> = 0>
	void					SyncUInt8(E& theEnum) { SyncUInt8(reinterpret_cast<std::underlying_type_t<E>&>(theEnum)); }
	void					SyncBool(bool& theBool);
	void					SyncFloat(float& theFloat);
	void					SyncDouble(double& theDouble);
	void					SyncString(std::string& theStr);
	inline void				SetVersion(int theVersion) { mVersion = theVersion; }
	inline int				GetVersion() const { return mVersion; }
};

#endif
