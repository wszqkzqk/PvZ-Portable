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

#include "Buffer.h"
#include <SDL_stdinc.h>
#include <array>
#define POLYNOMIAL 0x04c11db7L

using namespace Sexy;

static constexpr char WEB_ENCODE_MAP[] = ".-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

static constexpr int WEB_DECODE_MAP[256] = 
{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, 0, -1, 1, 0, -1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, -1, -1, -1, -1, -1
, -1, -1, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29
, 30, 31, 32, 33, 34, 35, 36, 37, -1, -1, -1, -1, -1, -1, 38, 39, 40, 41, 42, 43
, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

// Windows-1252 bytes 0x80-0x9F to Unicode codepoints (SDL_iconv mishandles this under Wine)
static constexpr char32_t WIN1252_TO_UNICODE[32] = {
	0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
	0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
	0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
	0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178
};

//----------------------------------------------------------------------------
// Generate the table of CRC remainders for all possible bytes.
//----------------------------------------------------------------------------
static constexpr std::array<uint32_t, 256> GenerateCRCTable()
{
	std::array<uint32_t, 256> table{};
	for (int i = 0; i < 256; i++)
	{
		uint32_t crc_accum = ((uint32_t) i << 24);
		for (int j = 0; j < 8; j++)
		{
			if (crc_accum & 0x80000000L)
				crc_accum = (crc_accum << 1) ^ POLYNOMIAL;
			else
				crc_accum = (crc_accum << 1);
		}
		table[i] = crc_accum;
	}
	return table;
}

static constexpr std::array<uint32_t, 256> crc_table = GenerateCRCTable();

//----------------------------------------------------------------------------
// Update the CRC on the data block one byte at a time.
//----------------------------------------------------------------------------
static uint32_t UpdateCRC(uint32_t crc_accum,
						const char *data_blk_ptr,
						int data_blk_size)
{
	int i, j;
	for (j = 0; j < data_blk_size; j++)
	{
		i = ((int) (crc_accum >> 24) ^ *data_blk_ptr++) & 0xff;
		crc_accum = (crc_accum << 8) ^ crc_table[i];
	}
	return crc_accum;
}

Buffer::Buffer()
{
	mDataBitSize = 0;
	mReadBitPos = 0;
	mWriteBitPos = 0;	
}

Buffer::~Buffer()
{
}

std::string Buffer::ToWebString() const
{
	std::string aString;
	int aSizeBits = mWriteBitPos;
	
	int anOldReadBitPos = mReadBitPos;
	mReadBitPos = 0;

	char aStr[256];
	snprintf(aStr, sizeof(aStr), "%08X", aSizeBits);
	aString += aStr;

	int aNumChars = (aSizeBits + 5) / 6;
	for (int aCharNum = 0; aCharNum < aNumChars; aCharNum++)
		aString += WEB_ENCODE_MAP[ReadNumBits(6, false)];
	
	mReadBitPos = anOldReadBitPos;
	
	return aString;
}

static std::string Win1252ToUTF8(const char* theData, int theLen)
{
	std::string aResult;
	aResult.reserve(theLen);
	for (int i = 0; i < theLen; ++i)
	{
		auto aByte = static_cast<unsigned char>(theData[i]);
		char32_t aCodepoint;
		if (aByte < 0x80)
			aCodepoint = aByte;
		else if (aByte <= 0x9F)
			aCodepoint = WIN1252_TO_UNICODE[aByte - 0x80];
		else
			aCodepoint = aByte; // 0xA0-0xFF map directly to U+00A0-U+00FF

		if (aCodepoint < 0x80)
			aResult += static_cast<char>(aCodepoint);
		else if (aCodepoint < 0x0800) {
			aResult += static_cast<char>(0xC0 | (aCodepoint >> 6));
			aResult += static_cast<char>(0x80 | (aCodepoint & 0x3F));
		} else {
			aResult += static_cast<char>(0xE0 | (aCodepoint >> 12));
			aResult += static_cast<char>(0x80 | ((aCodepoint >> 6) & 0x3F));
			aResult += static_cast<char>(0x80 | (aCodepoint & 0x3F));
		}
	}
	return aResult;
}

static bool IsValidUTF8(const char* theData, int theLen)
{
	auto p = reinterpret_cast<const unsigned char*>(theData);
	auto anEnd = p + theLen;
	while (p < anEnd)
	{
		unsigned char aFirst = *p;
		int aSeqLen;
		if (aFirst < 0x80) { ++p; continue; }
		else if ((aFirst & 0xE0) == 0xC0 && aFirst >= 0xC2) aSeqLen = 2;
		else if ((aFirst & 0xF0) == 0xE0) aSeqLen = 3;
		else if ((aFirst & 0xF8) == 0xF0 && aFirst <= 0xF4) aSeqLen = 4;
		else return false;

		if (p + aSeqLen > anEnd) return false;
		for (int i = 1; i < aSeqLen; ++i)
			if ((p[i] & 0xC0) != 0x80) return false;

		// Reject overlong encodings and surrogates
		if (aSeqLen == 3 && aFirst == 0xE0 && p[1] < 0xA0) return false;
		if (aSeqLen == 3 && aFirst == 0xED && p[1] > 0x9F) return false;
		if (aSeqLen == 4 && aFirst == 0xF0 && p[1] < 0x90) return false;
		if (aSeqLen == 4 && aFirst == 0xF4 && p[1] > 0x8F) return false;
		p += aSeqLen;
	}
	return true;
}

bool Buffer::ToUTF8String(std::string* theString) const
{
	const char* aData = (const char*)GetDataPtr();
	int aLen = GetDataLen();

	if (aLen >= 3 && memcmp(aData, "\xEF\xBB\xBF", 3) == 0) {
		// UTF-8 BOM: strip it
		*theString = std::string(aData + 3, aLen - 3);
		return true;
	}

	char* aStringBuffer = nullptr;
	if (aLen >= 2 && memcmp(aData, "\xFF\xFE", 2) == 0) {
		if ((aLen - 2) % 2 != 0) return false;
		aStringBuffer = SDL_iconv_string("UTF-8", "UTF-16LE", aData + 2, aLen - 2);
	} else if (aLen >= 2 && memcmp(aData, "\xFE\xFF", 2) == 0) {
		if ((aLen - 2) % 2 != 0) return false;
		aStringBuffer = SDL_iconv_string("UTF-8", "UTF-16BE", aData + 2, aLen - 2);
	} else if (IsValidUTF8(aData, aLen)) {
		*theString = std::string(aData, aLen);
		return true;
	} else {
		*theString = Win1252ToUTF8(aData, aLen);
		return true;
	}

	if (aStringBuffer) {
		*theString = std::string(aStringBuffer);
		SDL_free(aStringBuffer);
		return true;
	}
	return false;
}

void Buffer::FromWebString(const std::string& theString)
{
	Clear();

	if (theString.size() < 4)
		return;
	
	int aSizeBits = 0;

	for (int aDigitNum = 0; aDigitNum < 8; aDigitNum++)
	{
		char aChar = theString[aDigitNum];
		int aVal = 0;

		if ((aChar >= '0') && (aChar <= '9'))
			aVal = aChar - '0';
		else if ((aChar >= 'A') && (aChar <= 'F'))
			aVal = (aChar - 'A') + 10;
		else if ((aChar >= 'a') && (aChar <= 'f'))
			aVal = (aChar - 'f') + 10;

		aSizeBits += (aVal << ((7 - aDigitNum) * 4));
	}

	int aCharIdx = 8;
	int aNumBitsLeft = aSizeBits;
	while (aNumBitsLeft > 0)
	{
		uchar aChar = theString[aCharIdx++];
		int aVal = WEB_DECODE_MAP[aChar];
		int aNumBits = std::min(aNumBitsLeft, 6);
		WriteNumBits(aVal, aNumBits);
		aNumBitsLeft -= aNumBits;		
	}

	SeekFront();
}

void Buffer::SeekFront() const
{
	mReadBitPos = 0;	
}

void Buffer::Clear()
{
	mReadBitPos = 0;
	mWriteBitPos = 0;
	mDataBitSize = 0;
	mData.clear();
}

void Buffer::WriteByte(uchar theByte)
{	
	if (mWriteBitPos % 8 == 0)
		mData.push_back((char) theByte);
	else
	{		
		int anOfs = mWriteBitPos  % 8;
		mData[mWriteBitPos /8] |= theByte << anOfs;
		mData.push_back((char) (theByte >> (8-anOfs)));		
	}

	mWriteBitPos += 8;
	mDataBitSize = std::max(mDataBitSize, mWriteBitPos);
}

void Buffer::WriteNumBits(int theNum, int theBits)
{
	for (int aBitNum = 0; aBitNum < theBits; aBitNum++)
	{
		if (mWriteBitPos % 8 == 0)
			mData.push_back(0);
		if ((theNum & (1<<aBitNum)) != 0)
			mData[mWriteBitPos/8] |= 1 << (mWriteBitPos  % 8);
		mWriteBitPos++;
	}

	mDataBitSize = std::max(mDataBitSize, mWriteBitPos);
}

int Buffer::GetBitsRequired(int theNum, bool isSigned)
{
	if (theNum < 0) // two's compliment stuff
		theNum = -theNum - 1;
	
	int aNumBits = 0;
	while (theNum >= 1<<aNumBits)
		aNumBits++;
		
	if (isSigned)
		aNumBits++;
		
	return aNumBits;
}

void Buffer::WriteBoolean(bool theBool)
{
	WriteByte(theBool ? 1 : 0);
}

void Buffer::WriteShort(short theShort)
{
	WriteByte((uchar)theShort);
	WriteByte((uchar)(theShort >> 8));
}

void Buffer::WriteLong(int32_t theLong)
{
	WriteByte((uchar)theLong);
	WriteByte((uchar)(theLong >> 8));
	WriteByte((uchar)(theLong >> 16));
	WriteByte((uchar)(theLong >> 24));
}

void Buffer::WriteString(const std::string& theString)
{
	WriteShort((short) theString.length());
	for (int i = 0; i < (int)theString.length(); i++)
		WriteByte(theString[i]);
}

void Buffer::WriteLine(const std::string& theString)
{
	WriteBytes((const uchar*) (theString + "\r\n").c_str(), (int) theString.length() + 2);
}

void Buffer::WriteBuffer(const ByteVector& theBuffer)
{
	WriteLong((short) theBuffer.size());
	for (int i = 0; i < (int)theBuffer.size(); i++)
		WriteByte(theBuffer[i]);
}

void Buffer::WriteBytes(const uchar* theByte, int theCount)
{
	for (int i = 0; i < theCount; i++)
		WriteByte(theByte[i]);
}

void Buffer::SetData(const ByteVector& theBuffer)
{
	mData = theBuffer;
	mDataBitSize = mData.size() * 8;
}

void Buffer::SetData(uchar* thePtr, int theCount)
{
	mData.clear();
	mData.insert(mData.begin(), thePtr, thePtr + theCount);
	mDataBitSize = mData.size() * 8;
}

uchar Buffer::ReadByte() const
{
	if ((mReadBitPos + 7)/8 >= (int)mData.size())
	{		
		return 0; // Underflow
	}

	if (mReadBitPos % 8 == 0)
	{
		uchar b = mData[mReadBitPos/8];
		mReadBitPos += 8;
		return b;
	}
	else
	{
		int anOfs = mReadBitPos % 8;
			
		uchar b = 0;
		
		b = mData[mReadBitPos/8] >> anOfs;
		b |= mData[(mReadBitPos/8)+1] << (8 - anOfs);
		
		mReadBitPos += 8;		
		
		return b;
	}
}

int Buffer::ReadNumBits(int theBits, bool isSigned) const
{	
	int aByteLength = (int) mData.size();

	int theNum = 0;
	bool bset = false;
	for (int aBitNum = 0; aBitNum < theBits; aBitNum++)
	{
		int aBytePos = mReadBitPos/8;

		if (aBytePos >= aByteLength)
			break;

		if ((bset = (mData[aBytePos] & (1<<(mReadBitPos%8))) != 0))	
			theNum |= 1<<aBitNum;
		
		mReadBitPos++;
	}
	
	if ((isSigned) && (bset)) // sign extend
		for (int aBitNum = theBits; aBitNum < 32; aBitNum++)
			theNum |= 1<<aBitNum;
	
	return theNum;
}

bool Buffer::ReadBoolean() const
{
	return ReadByte() != 0;
}

short Buffer::ReadShort() const
{
	short aShort = ReadByte();
	aShort |= ((short) ReadByte() << 8);
	return aShort;	
}

int32_t Buffer::ReadLong() const
{
	int32_t aLong = ReadByte();
	aLong |= ((int32_t) ReadByte()) << 8;
	aLong |= ((int32_t) ReadByte()) << 16;
	aLong |= ((int32_t) ReadByte()) << 24;

	return aLong;
}

std::string	Buffer::ReadString() const
{
	std::string aString;
	int aLen = ReadShort();

	for (int i = 0; i < aLen; i++)
		aString += (char) ReadByte();

	return aString;
}

std::string Buffer::ReadLine() const
{
	std::string aString;

	for (;;)
	{
		char c = ReadByte();

		if ((c == 0) || (c == '\n'))
			break;

		if (c != '\r')
			aString += c;
	}

	return aString;
}

void Buffer::ReadBytes(uchar* theData, int theLen) const
{
	for (int i = 0; i < theLen; i++)
		theData[i] = ReadByte();
}

void Buffer::ReadBuffer(ByteVector* theByteVector) const
{
	theByteVector->clear();
	
	uint32_t aLength = ReadLong();
	theByteVector->resize(aLength);
	ReadBytes(&(*theByteVector)[0], aLength);
}

const uchar* Buffer::GetDataPtr() const
{
	if (mData.size() == 0)
		return nullptr;
	return &mData[0];
}

int Buffer::GetDataLen() const
{
	return (mDataBitSize + 7) / 8; // Round up
}

int Buffer::GetDataLenBits() const
{
	return mDataBitSize;
}

uint32_t Buffer::GetCRC32(uint32_t theSeed) const
{	
	uint32_t aCRC = theSeed;
	aCRC = UpdateCRC(aCRC, (const char*) &mData[0], (int) mData.size());	
	return aCRC;
}

bool Buffer::AtEnd() const
{ 
	//return mReadBitPos >= (int)mData.size()*8;
	return mReadBitPos >= mDataBitSize;
}

bool Buffer::PastEnd() const
{
	return mReadBitPos > mDataBitSize;
}
