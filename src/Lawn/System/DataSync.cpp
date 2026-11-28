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
#include <cstring>
#include <cstdint>
#include <algorithm>

#include "DataSync.h"

using Sexy::FromLE16;
using Sexy::FromLE32;
using Sexy::FromLE64;
using Sexy::ToLE16;
using Sexy::ToLE32;
using Sexy::ToLE64;

void DataReader::OpenMemory(const void* theData, uint32_t theDataLen, bool takeOwnership)
{
	if (takeOwnership)
		mOwnedData.reset(const_cast<uint8_t*>(static_cast<const uint8_t*>(theData)));
	else
		mOwnedData.reset();

	mData = static_cast<const uint8_t*>(theData);
	mDataLen = theDataLen;
	mDataPos = 0;
}

void DataReader::ReadBytes(void* theMem, uint32_t theNumBytes)
{
	if (!mData || theNumBytes > mDataLen - mDataPos)
		throw DataReaderException();

	std::memcpy(theMem, mData + mDataPos, theNumBytes);
	mDataPos += theNumBytes;
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
	return ReadUInt8() != 0;
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

DataSync::DataSync(DataReader& theReader)
	: mReader(&theReader)
{
}

DataSync::DataSync(DataWriter& theWriter)
	: mWriter(&theWriter)
{
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

void DataWriter::OpenMemory(uint32_t theReserveAmount)
{
	mData.clear();
	mData.reserve(std::max<uint32_t>(theReserveAmount, 32));
}

void DataWriter::WriteBytes(const void* theData, uint32_t theDataLen)
{
	const uint8_t* aData = static_cast<const uint8_t*>(theData);
	mData.insert(mData.end(), aData, aData + theDataLen);
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
	WriteUInt8(theBool ? 1 : 0);
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
