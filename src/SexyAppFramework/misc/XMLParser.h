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

#ifndef __XMLPARSER_H__
#define __XMLPARSER_H__

#include "Common.h"

#include <list>

#include "PerfTimer.h"

struct PFILE;

namespace Sexy
{

class XMLParam
{
public:
	std::string				mKey;
	std::string				mValue;
};

typedef std::map<std::string, std::string>	XMLParamMap;
typedef std::list<XMLParamMap::iterator>	XMLParamMapIteratorList;

typedef std::vector<char> XMLParserBuffer;

class XMLElement
{
public:
	enum
	{
		TYPE_NONE,
		TYPE_START,
		TYPE_END,
		TYPE_ELEMENT,
		TYPE_INSTRUCTION,
		TYPE_COMMENT
	};
public:

	int						mType;
	std::string				mSection;
	std::string				mValue;
	std::string				mInstruction;
	XMLParamMap				mAttributes;
	XMLParamMapIteratorList	mAttributeIteratorList; // stores attribute iterators in their original order
};

class XMLParser
{
protected:
	std::string				mFileName;
	std::string				mErrorText;
	int						mLineNum;
	PFILE*					mFile;
	bool					mHasFailed;
	bool					mAllowComments;
	XMLParserBuffer			mBufferedText;
	std::string				mSection;
	bool					(XMLParser::*mGetCharFunc)(char* theChar, bool* error);
	bool					mForcedEncodingType;
	bool					mFirstChar;
	bool					mByteSwap;

protected:
	void					Fail(const std::string& theErrorText);
	void					Init();

	bool					AddAttribute(XMLElement* theElement, const std::string& aAttributeKey, const std::string& aAttributeValue);

	bool					GetAsciiChar(char* theChar, bool* error);
	bool					GetUTF8Char(char* theChar, bool* error);
	bool					GetUTF16Char(char* theChar, bool* error);
	bool					GetUTF16LEChar(char* theChar, bool* error);
	bool					GetUTF16BEChar(char* theChar, bool* error);

public:
	enum XMLEncodingType
	{
		ASCII,
		UTF_8,
		UTF_16,
		UTF_16_LE,
		UTF_16_BE
	};

public:
	XMLParser();
	virtual ~XMLParser();

	void					SetEncodingType(XMLEncodingType theEncoding);
	bool					OpenFile(const std::string& theFilename);
	void					SetStringSource(std::string_view theString);
	bool					NextElement(XMLElement* theElement);
	std::string				GetErrorText();
	int						GetCurrentLineNum();
	std::string				GetFileName();

	inline void				AllowComments(bool doAllow) { mAllowComments = doAllow; }

	bool					HasFailed();
	bool					EndOfFile();
};

};

#endif //__XMLPARSER_H__
