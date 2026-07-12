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

#include "DataSync.h"
#include "fcaseopen/fcaseopen.h"

using Sexy::FromLE16;
using Sexy::FromLE32;
using Sexy::FromLE64;
using Sexy::ToLE16;
using Sexy::ToLE32;
using Sexy::ToLE64;

DataReader::DataReader()
{
	mFile = nullptr;
	mData = nullptr;
	mDataLen = 0;
	mDataPos = 0;
	mOwnData = false;
}

DataReader::~DataReader()
{
	if (mFile)
	{
		fclose(mFile);
		mFile = nullptr;
	}

	if (mOwnData)
	{
		delete[] mData;
	}

	mData = nullptr;
	mDataLen = 0;
	mDataPos = 0;
	mOwnData = false;
}

bool DataReader::OpenFile(const std::string& theFileName)
{
	mFile = fcaseopen(theFileName.c_str(), "rb");
	return mFile;
}

void DataReader::OpenMemory(const void* theData, uint32_t theDataLen, bool takeOwnership)
{
	if (mFile)
	{
		fclose(mFile);
		mFile = nullptr;
	}
	if (mOwnData)
	{
		delete[] mData;
	}

	mData = (char*)theData;
	mDataLen = theDataLen;
	mOwnData = takeOwnership;
}

void DataReader::Close()
{
	if (mFile)
	{
		fclose(mFile);
		mFile = nullptr;
	}
}

void DataReader::ReadBytes(void* theMem, uint32_t theNumBytes)
{
	if (mData)
	{
		mDataPos += theNumBytes;
		if (mDataPos > mDataLen)
		{
			throw DataReaderException();
		}

		memcpy(theMem, mData, theNumBytes);
		mData += theNumBytes;
	}
	else if (!mFile || fread(theMem, sizeof(char), theNumBytes, mFile) != theNumBytes)
	{
		throw DataReaderException();
	}
}

void DataReader::Rewind(uint32_t theNumBytes)
{
	theNumBytes = std::min(theNumBytes, mDataPos);
	mDataPos -= theNumBytes;
	mData -= theNumBytes;
}

uint16_t DataReader::ReadUInt16()
{
	uint16_t aShort;
	ReadBytes(&aShort, sizeof(aShort));
	return FromLE16(aShort);
}

uint32_t DataReader::ReadUInt32()
{
	uint32_t aLong;
	ReadBytes(&aLong, sizeof(aLong));
	return FromLE32(aLong);
}

uint64_t DataReader::ReadUInt64()
{
	uint64_t aValue;
	ReadBytes(&aValue, sizeof(aValue));
	return FromLE64(aValue);
}

uint8_t DataReader::ReadUInt8()
{
	uint8_t aChar;
	ReadBytes(&aChar, sizeof(aChar));
	return aChar;
}

bool DataReader::ReadBool()
{
	bool aBool;
	ReadBytes(&aBool, sizeof(aBool));
	return aBool;
}

float DataReader::ReadFloat()
{
	uint32_t aRaw;
	ReadBytes(&aRaw, sizeof(aRaw));
	aRaw = FromLE32(aRaw);
	float aFloat;
	memcpy(&aFloat, &aRaw, sizeof(float));
	return aFloat;
}

double DataReader::ReadDouble()
{
	uint64_t aRaw;
	ReadBytes(&aRaw, sizeof(aRaw));
	aRaw = FromLE64(aRaw);
	double aDouble;
	memcpy(&aDouble, &aRaw, sizeof(double));
	return aDouble;
}

void DataReader::ReadString(std::string& theStr)
{
	uint32_t aStrLen = ReadUInt16();
	theStr.resize(aStrLen);
	ReadBytes(theStr.data(), aStrLen);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DataSync::DataSync(DataReader& theReader)
{
	Reset();
	mReader = &theReader;
}

DataSync::DataSync(DataWriter& theWriter)
{
	Reset();
	mWriter = &theWriter;
}

DataSync::~DataSync()
{
}

void DataSync::ResetPointerTable()
{
	mIntToPointerMap.clear();
	mPointerToIntMap.clear();
	mPointerSyncList.clear();
	mCurPointerIndex = 1;
	mPointerToIntMap[nullptr] = 0;
	mIntToPointerMap[0] = nullptr;
}

void DataSync::Reset()
{
	mReader = nullptr;
	mWriter = nullptr;
	ResetPointerTable();
}

void DataSync::SyncBytes(void* theData, uint32_t theDataLen)
{
	if (mReader)
	{
		mReader->ReadBytes(theData, theDataLen);
	}
	else
	{
		mWriter->WriteBytes(theData, theDataLen);
	}
}

void DataSync::SyncUInt64(uint64_t& theNum)
{
	if (mReader)
	{
		theNum = mReader->ReadUInt64();
	}
	else
	{
		mWriter->WriteUInt64(theNum);
	}
}

void DataSync::SyncUInt32(uint32_t& theNum)
{
	if (mReader)
	{
		theNum = mReader->ReadUInt32();
	}
	else
	{
		mWriter->WriteUInt32(theNum);
	}
}

void DataSync::SyncUInt32(char& theNum)
{
	uint32_t aNum = static_cast<uint32_t>(static_cast<unsigned char>(theNum));
	SyncUInt32(aNum);
	if (mReader)
		theNum = static_cast<char>(aNum);
}

void DataSync::SyncUInt32(short& theNum)
{
	uint32_t aNum = static_cast<uint32_t>(static_cast<uint16_t>(theNum));
	SyncUInt32(aNum);
	if (mReader)
		theNum = static_cast<short>(aNum);
}

void DataSync::SyncUInt32(long& theNum)
{
	uint32_t aNum = static_cast<uint32_t>(theNum);
	SyncUInt32(aNum);
	if (mReader)
		theNum = static_cast<long>(aNum);
}

void DataSync::SyncUInt32(unsigned char& theNum)
{
	uint32_t aNum = theNum;
	SyncUInt32(aNum);
	if (mReader)
		theNum = static_cast<unsigned char>(aNum);
}

void DataSync::SyncUInt32(unsigned short& theNum)
{
	uint32_t aNum = theNum;
	SyncUInt32(aNum);
	if (mReader)
		theNum = static_cast<unsigned short>(aNum);
}

void DataSync::SyncUInt32(int32_t& theNum)
{
	uint32_t aNum = static_cast<uint32_t>(theNum);
	SyncUInt32(aNum);
	if (mReader)
		theNum = static_cast<int32_t>(aNum);
}

void DataSync::SyncUInt16(uint16_t& theNum)
{
	if (mReader)
	{
		theNum = mReader->ReadUInt16();
	}
	else
	{
		mWriter->WriteUInt16(theNum);
	}
}

void DataSync::SyncUInt16(char& theNum)
{
	uint16_t aNum = static_cast<uint16_t>(static_cast<unsigned char>(theNum));
	SyncUInt16(aNum);
	if (mReader)
		theNum = static_cast<char>(aNum);
}

void DataSync::SyncUInt16(short& theNum)
{
	uint16_t aNum = static_cast<uint16_t>(theNum);
	SyncUInt16(aNum);
	if (mReader)
		theNum = static_cast<short>(aNum);
}

void DataSync::SyncUInt16(long& theNum)
{
	uint16_t aNum = static_cast<uint16_t>(theNum);
	SyncUInt16(aNum);
	if (mReader)
		theNum = static_cast<long>(aNum);
}

void DataSync::SyncUInt16(unsigned char& theNum)
{
	uint16_t aNum = theNum;
	SyncUInt16(aNum);
	if (mReader)
		theNum = static_cast<unsigned char>(aNum);
}

void DataSync::SyncUInt16(uint32_t& theNum)
{
	uint16_t aNum = static_cast<uint16_t>(theNum);
	SyncUInt16(aNum);
	if (mReader)
		theNum = aNum;
}

void DataSync::SyncUInt16(int32_t& theNum)
{
	uint16_t aNum = static_cast<uint16_t>(theNum);
	SyncUInt16(aNum);
	if (mReader)
		theNum = aNum;
}

void DataSync::SyncUInt8(uint8_t& theChar)
{
	if (mReader)
	{
		theChar = mReader->ReadUInt8();
	}
	else
	{
		mWriter->WriteUInt8(theChar);
	}
}

void DataSync::SyncUInt8(char& theChar)
{
	uint8_t aChar = static_cast<uint8_t>(theChar);
	SyncUInt8(aChar);
	if (mReader)
		theChar = static_cast<char>(aChar);
}

void DataSync::SyncUInt8(short& theChar)
{
	uint8_t aChar = static_cast<uint8_t>(theChar);
	SyncUInt8(aChar);
	if (mReader)
		theChar = static_cast<short>(aChar);
}

void DataSync::SyncUInt8(long& theChar)
{
	uint8_t aChar = static_cast<uint8_t>(theChar);
	SyncUInt8(aChar);
	if (mReader)
		theChar = static_cast<long>(aChar);
}

void DataSync::SyncUInt8(unsigned short& theChar)
{
	uint8_t aChar = static_cast<uint8_t>(theChar);
	SyncUInt8(aChar);
	if (mReader)
		theChar = static_cast<unsigned short>(aChar);
}

void DataSync::SyncUInt8(uint32_t& theChar)
{
	uint8_t aChar = static_cast<uint8_t>(theChar);
	SyncUInt8(aChar);
	if (mReader)
		theChar = aChar;
}

void DataSync::SyncUInt8(int32_t& theChar)
{
	uint8_t aChar = static_cast<uint8_t>(theChar);
	SyncUInt8(aChar);
	if (mReader)
		theChar = aChar;
}

void DataSync::SyncBool(bool& theBool)
{
	if (mReader)
	{
		theBool = mReader->ReadBool();
	}
	else
	{
		mWriter->WriteBool(theBool);
	}
}

void DataSync::SyncFloat(float& theFloat)
{
	if (mReader)
	{
		theFloat = mReader->ReadFloat();
	}
	else
	{
		mWriter->WriteFloat(theFloat);
	}
}

void DataSync::SyncDouble(double& theDouble)
{
	if (mReader)
	{
		theDouble = mReader->ReadDouble();
	}
	else
	{
		mWriter->WriteDouble(theDouble);
	}
}

void DataSync::SyncString(std::string& theStr)
{
	if (mReader)
	{
		mReader->ReadString(theStr);
	}
	else
	{
		mWriter->WriteString(theStr);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DataWriter::DataWriter()
{
	mFile = nullptr;
	mData = nullptr;
	mDataLen = 0;
	mCapacity = 0;
}

DataWriter::~DataWriter()
{
	if (mFile)
	{
		fclose(mFile);
		mFile = nullptr;
	}

	delete[] mData;
	mData = nullptr;
	mDataLen = 0;
	mCapacity = 0;
}

bool DataWriter::OpenFile(const std::string& theFileName)
{
	mFile = fcaseopen(theFileName.c_str(), "wb");
	return mFile;
}

void DataWriter::Close()
{
	if (mFile)
	{
		fclose(mFile);
		mFile = nullptr;
	}
}

void DataWriter::EnsureCapacity(uint32_t theNumBytes)
{
	if (mCapacity < theNumBytes)
	{
		// 每次将容量乘 2 直到容量达到 theNumBytes 或更多
		do { mCapacity <<= 1; } while (mCapacity < theNumBytes);

		// 申请新内存
		char* aData = new char[mCapacity];
		// 将原数据迁移至新内存区域中
		memcpy(aData, mData, mDataLen);
		// 释放旧有内存区域
		delete[] mData;
		mData = aData;
	}
}

void DataWriter::OpenMemory(uint32_t theReserveAmount)
{
	if (mFile)
	{
		fclose(mFile);
		mFile = nullptr;
	}
	delete[] mData;
	mData = 0;
	mDataLen = 0;
	mCapacity = 0;

	theReserveAmount = std::max<uint32_t>(theReserveAmount, 32);
	mData = new char[theReserveAmount];
	mCapacity = theReserveAmount;
}

void DataWriter::WriteBytes(const void* theData, uint32_t theDataLen)
{
	if (mData)
	{
		EnsureCapacity(mDataLen + theDataLen);
		memcpy(mData + mDataLen, theData, theDataLen);
		mDataLen += theDataLen;
	}
	else if (mFile)
	{
		fwrite(theData, sizeof(unsigned char), theDataLen, mFile);
	}
}

void DataWriter::WriteUInt32(uint32_t theUInt32)
{
	uint32_t aLE = ToLE32(theUInt32);
	WriteBytes(&aLE, sizeof(uint32_t));
}

void DataWriter::WriteUInt64(uint64_t theUInt64)
{
	uint64_t aLE = ToLE64(theUInt64);
	WriteBytes(&aLE, sizeof(uint64_t));
}

void DataWriter::WriteUInt16(uint16_t theUInt16)
{
	uint16_t aLE = ToLE16(theUInt16);
	WriteBytes(&aLE, sizeof(uint16_t));
}

void DataWriter::WriteUInt8(uint8_t theUInt8)
{
	WriteBytes(&theUInt8, sizeof(uint8_t));
}

void DataWriter::WriteBool(bool theBool)
{
	WriteBytes(&theBool, sizeof(bool));
}

void DataWriter::WriteFloat(float theFloat)
{
	uint32_t aRaw;
	memcpy(&aRaw, &theFloat, sizeof(float));
	aRaw = ToLE32(aRaw);
	WriteBytes(&aRaw, sizeof(uint32_t));
}

void DataWriter::WriteDouble(double theDouble)
{
	uint64_t aRaw;
	memcpy(&aRaw, &theDouble, sizeof(double));
	aRaw = ToLE64(aRaw);
	WriteBytes(&aRaw, sizeof(uint64_t));
}

void DataWriter::WriteString(std::string_view theStr)
{
	uint16_t aStrLen = static_cast<uint16_t>(theStr.length());
	WriteUInt16(aStrLen);
	WriteBytes(theStr.data(), static_cast<uint32_t>(aStrLen));
}
