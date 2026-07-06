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

#include "Common.h"
#include "misc/MTRand.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <SDL.h>

#include "misc/PerfTimer.h"

bool Sexy::gDebug = false;
static Sexy::MTRand gMTRand;
namespace Sexy
{
	std::filesystem::path gAppDataFolder;
	std::filesystem::path gResourceFolder;
	std::string gResourceFolderStr; // Cached string to avoid repeated conversions
}

static inline char ToLowerAscii(char c)
{
	return (char)std::tolower((unsigned char)c);
}

static inline void SexyLogV(SDL_LogPriority thePriority, const char* theFormat, va_list theArgs)
{
	std::string aBuffer = Sexy::VFormat(theFormat, theArgs);
	if (aBuffer.empty())
		return;

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, thePriority, "%s", aBuffer.c_str());
}

static inline bool IsUnicodeSpace(char32_t theChar)
{
	switch (theChar)
	{
	case 0x0085: case 0x00A0: case 0x1680:
	case 0x2000: case 0x2001: case 0x2002: case 0x2003:
	case 0x2004: case 0x2005: case 0x2006: case 0x2007:
	case 0x2008: case 0x2009: case 0x200A:
	case 0x2028: case 0x2029: case 0x202F: case 0x205F:
	case 0x3000:
		return true;
	default:
		return theChar < 0x80 && std::isspace(static_cast<unsigned char>(theChar));
	}
}

void Sexy::PrintF(const char *text, ...)
{
	va_list args;
	va_start(args, text);
	SexyLogV(SDL_LOG_PRIORITY_INFO, text, args);
	va_end(args);
}

void Sexy::LogError(const char* theFormat, ...)
{
	va_list args;
	va_start(args, theFormat);
	SexyLogV(SDL_LOG_PRIORITY_ERROR, theFormat, args);
	va_end(args);
}

int Sexy::Rand()
{
	return gMTRand.Next();
}

int Sexy::Rand(int range)
{
	return gMTRand.Next((unsigned long)range);
}

float Sexy::Rand(float range)
{
	return gMTRand.Next(range);
}

void Sexy::SRand(ulong theSeed)
{
	gMTRand.SRand(theSeed);
}

std::string Sexy::GetAppDataFolder()
{
	return PathToU8(Sexy::gAppDataFolder);
}

void Sexy::SetAppDataFolder(std::string_view thePath)
{
	Sexy::gAppDataFolder = PathFromU8(thePath);
}

std::string Sexy::GetAppDataPath(std::string_view theRelativePath)
{
	return PathToU8(Sexy::gAppDataFolder / PathFromU8(theRelativePath));
}

const std::string& Sexy::GetResourceFolder()
{
	return Sexy::gResourceFolderStr;
}

void Sexy::SetResourceFolder(std::string_view thePath)
{
	Sexy::gResourceFolder = PathFromU8(thePath);
	Sexy::gResourceFolderStr = PathToU8(Sexy::gResourceFolder);
}

std::string Sexy::GetResourcePath(std::string_view theRelativePath)
{
	return PathToU8(Sexy::gResourceFolder / PathFromU8(theRelativePath));
}

std::string Sexy::StringToUpper(std::string_view theString)
{
	std::string aString(theString);
	std::transform(aString.begin(), aString.end(), aString.begin(),
		[](unsigned char c) { return (char)std::toupper(c); });
	return aString;
}

std::string Sexy::StringToLower(std::string_view theString)
{
	std::string aString(theString);
	std::transform(aString.begin(), aString.end(), aString.begin(),
		[](unsigned char c) { return (char)std::tolower(c); });
	return aString;
}

std::string Sexy::Trim(std::string_view theString)
{
	if (theString.empty())
		return std::string(theString);

	size_t start = 0;
	while (start < theString.size())
	{
		size_t aOffset = start;
		char32_t aChar = 0;
		if (!UTF8DecodeNext(theString, aOffset, aChar))
			break;
		if (!IsUnicodeSpace(aChar))
			break;
		start = aOffset;
	}

	if (start >= theString.size())
		return std::string();

	size_t end = theString.size();
	while (end > start)
	{
		size_t aCharStart = end - 1;
		while (aCharStart > start && (static_cast<unsigned char>(theString[aCharStart]) & 0xC0) == 0x80)
			--aCharStart;
		size_t aOffset = aCharStart;
		char32_t aChar = 0;
		if (!UTF8DecodeNext(theString, aOffset, aChar) || aOffset != end)
			break;
		if (!IsUnicodeSpace(aChar))
			break;
		end = aCharStart;
	}

	return std::string(theString.substr(start, end - start));
}

bool Sexy::StringToInt(const std::string& theString, int* theIntVal)
{
	if (theIntVal == nullptr)
		return false;

	*theIntVal = 0;
	if (theString.empty())
		return false;

	const char* start = theString.c_str();
	char* end = nullptr;
	long value = std::strtol(start, &end, 0);
	if (end != start + theString.size())
		return false;
	*theIntVal = (int)value;
	return true;
}

bool Sexy::StringToDouble(const std::string& theString, double* theDoubleVal)
{
	if (theDoubleVal == nullptr)
		return false;

	*theDoubleVal = 0.0;
	if (theString.empty())
		return false;

	const char* start = theString.c_str();
	char* end = nullptr;
	double value = std::strtod(start, &end);
	if (end != start + theString.size())
		return false;
	*theDoubleVal = value;
	return true;
}

// TODO: Use <locale> for localization of number output?
std::string Sexy::CommaSeperate(int theValue)
{	
	if (theValue == 0)
		return "0";

	bool isNeg = theValue < 0;
	unsigned int aCurValue = isNeg ? (unsigned int)(-theValue) : (unsigned int)theValue;

	std::string aCurString;

	int aPlace = 0;
	while (aCurValue > 0)
	{
		if ((aPlace != 0) && (aPlace % 3 == 0))
			aCurString = ',' + aCurString;
		aCurString = static_cast<char>('0' + (aCurValue % 10)) + aCurString;
		aCurValue /= 10;
		aPlace++;
	}

	if (isNeg)
		aCurString = '-' + aCurString;

	return aCurString;
}

std::string Sexy::GetCurDir()
{
	std::error_code ec;
	std::filesystem::path cur = std::filesystem::current_path(ec);
	if (ec)
		return std::string();
	return PathToU8(cur);
}

std::string Sexy::GetFullPath(std::string_view theRelPath)
{
	return GetPathFrom(theRelPath, GetCurDir());
}

std::string Sexy::GetPathFrom(std::string_view theRelPath, std::string_view theDir)
{
	std::filesystem::path relPath = PathFromU8(theRelPath);
	if (IsPathRooted(theRelPath))
		return PathToU8(relPath.lexically_normal());

	std::filesystem::path baseDir;
	if (!theDir.empty())
		baseDir = PathFromU8(theDir);
	else
	{
		std::error_code ec;
		baseDir = std::filesystem::current_path(ec);
		if (ec)
			return PathToU8(relPath.lexically_normal());
	}

	return PathToU8((baseDir / relPath).lexically_normal());
}

bool Sexy::IsPathRooted(std::string_view thePath)
{
	if (thePath.empty())
		return false;

	const std::filesystem::path aPath = PathFromU8(thePath);
	if (aPath.has_root_path())
		return true;

#if defined(__SWITCH__) || defined(__3DS__)
	const size_t aColonPos = thePath.find(':');
	if (aColonPos == std::string_view::npos || aColonPos == 0 || aColonPos + 1 >= thePath.size())
		return false;

	const char aSeparator = thePath[aColonPos + 1];
	if (aSeparator != '/' && aSeparator != '\\')
		return false;

	for (size_t i = 0; i < aColonPos; i++)
	{
		const unsigned char aChar = static_cast<unsigned char>(thePath[i]);
		if (!std::isalnum(aChar) && aChar != '_')
			return false;
	}

	return true;
#endif

	return false;
}

bool Sexy::AllowAllAccess(std::string_view theFileName)
{
	return false;
}

bool Sexy::Deltree(std::string_view thePath)
{
	std::error_code ec;
	std::filesystem::path path = PathFromU8(thePath);
	if (!std::filesystem::exists(path, ec))
		return false;

	std::filesystem::remove_all(path, ec);
	return !ec;
}

bool Sexy::FileExists(std::string_view theFileName)
{
	std::error_code ec;
	return std::filesystem::exists(PathFromU8(theFileName), ec);
}

void Sexy::MkDir(std::string_view theDir)
{
	std::error_code ec;
	std::filesystem::create_directories(PathFromU8(theDir), ec);
}

std::string Sexy::GetFileName(std::string_view thePath, bool noExtension)
{
	std::filesystem::path path = PathFromU8(thePath);
	if (!path.has_filename())
		return "";

	if (noExtension)
		return PathToU8(path.stem());

	return PathToU8(path.filename());
}

std::string Sexy::GetFileDir(std::string_view thePath, bool withSlash)
{
	std::filesystem::path path = PathFromU8(thePath);
	std::filesystem::path parent = path.parent_path();
	if (parent.empty())
		return "";

	std::string result = PathToU8(parent);
	if (withSlash && !result.empty())
		result += '/';

	return result;
}

std::string Sexy::RemoveTrailingSlash(std::string_view theDirectory)
{
	if (theDirectory.empty())
		return std::string(theDirectory);

	return PathToU8(PathFromU8(theDirectory).lexically_normal());
}
std::string Sexy::VFormat(const char* fmt, va_list argPtr) 
{
	va_list argsCopy;
	va_copy(argsCopy, argPtr);

#ifdef _WIN32
	int required = _vscprintf(fmt, argsCopy);
#else
	int required = vsnprintf(nullptr, 0, fmt, argsCopy);
#endif
	va_end(argsCopy);

	if (required <= 0)
		return std::string();

	std::string result;
	result.resize((size_t)required + 1);

	va_list argsCopy2;
	va_copy(argsCopy2, argPtr);
#ifdef _WIN32
	_vsnprintf(result.data(), (size_t)required + 1, fmt, argsCopy2);
#else
	vsnprintf(result.data(), (size_t)required + 1, fmt, argsCopy2);
#endif
	va_end(argsCopy2);

	result.resize((size_t)required);

	return result;
}

//overloaded StrFormat: should only be used by the xml strings
std::string Sexy::StrFormat(const char* fmt ...) 
{
    va_list argList;
    va_start(argList, fmt);
	std::string result = VFormat(fmt, argList);
    va_end(argList);

    return result;
}

std::string Sexy::Evaluate(std::string_view theString, const DefinesMap& theDefinesMap)
{
	std::string anEvaluatedString(theString);

	for (;;)
	{
		size_t aPercentPos = anEvaluatedString.find('%');

		if (aPercentPos == std::string::npos)
			break;
		
		size_t aSecondPercentPos = anEvaluatedString.find('%', aPercentPos + 1);
		if (aSecondPercentPos == std::string::npos)
			break;

		std::string aName = anEvaluatedString.substr(aPercentPos + 1, aSecondPercentPos - aPercentPos - 1);

		std::string aValue;
		DefinesMap::const_iterator anItr = theDefinesMap.find(aName);		
		if (anItr != theDefinesMap.end())
			aValue = anItr->second;
		else
			aValue = "";		

		anEvaluatedString.replace(aPercentPos, aSecondPercentPos - aPercentPos + 1, aValue);
	}

	return anEvaluatedString;
}

std::string Sexy::XMLDecodeString(std::string_view theString)
{
	std::string aNewString;
	aNewString.reserve(theString.size());

	if (theString.find('&') == std::string::npos)
		return std::string(theString);

	for (size_t i = 0; i < theString.length(); i++)
	{
		char c = theString[i];

		if (c == '&')
		{
			size_t aSemiPos = theString.find(';', i);

			if (aSemiPos != std::string::npos)
			{
				std::string anEntName(theString.substr(i+1, aSemiPos-i-1));
				i = aSemiPos;
											
				if (anEntName == "lt")
					c = '<';
				else if (anEntName == "amp")
					c = '&';
				else if (anEntName == "gt")
					c = '>';
				else if (anEntName == "quot")
					c = '"';
				else if (anEntName == "apos")
					c = '\'';
				else if (anEntName == "nbsp")
					c = ' ';
				else if (anEntName == "cr")
					c = '\n';
			}
		}				
		
		aNewString += c;
	}

	return aNewString;
}

std::string Sexy::XMLEncodeString(std::string_view theString)
{
	std::string aNewString;
	aNewString.reserve(theString.size() * 2);

	bool hasSpace = false;

	for (size_t i = 0; i < theString.length(); i++)
	{
		char c = theString[i];

		if (c == ' ')
		{
			if (hasSpace)
			{
				aNewString += "&nbsp;";
				continue;
			}
			
			hasSpace = true;
		}
		else
			hasSpace = false;

		switch (c)
		{
		case '<':
			aNewString += "&lt;";
			break;
		case '&':		
			aNewString += "&amp;";
			break;
		case '>':
			aNewString += "&gt;";
			break;
		case '"':
			aNewString += "&quot;";
			break;
		case '\'':
			aNewString += "&apos;";
			break;
		case '\n':
			aNewString += "&cr;";
			break;
		default:
			aNewString += c;
			break;
		}
	}

	return aNewString;
}

std::string Sexy::Upper(std::string_view _data)
{
	return StringToUpper(_data);
}

std::string Sexy::Lower(std::string_view _data)
{
	return StringToLower(_data);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int Sexy::StrFindNoCase(const char *theStr, const char *theFind)
{
	int p1, p2;
	int cp = 0;
	const int len1 = (int)strlen(theStr);
	const int len2 = (int)strlen(theFind);
	while (cp < len1)
	{
		p1 = cp;
		p2 = 0;
		while (p1 < len1 && p2 < len2)
		{
			if (ToLowerAscii(theStr[p1]) != ToLowerAscii(theFind[p2]))
				break;

			p1++; p2++;
		}
		if(p2==len2)
			return p1-len2;

		cp++;
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool Sexy::StrPrefixNoCase(const char *theStr, const char *thePrefix, int maxLength)
{
	int i;
	char c1 = 0, c2 = 0;
	for (i=0; i<maxLength; i++)
	{
		c1 = ToLowerAscii(*theStr++);
		c2 = ToLowerAscii(*thePrefix++);

		if (c1==0 || c2==0)
			break;

		if (c1!=c2)
			return false;
	}

	return c2==0 || i==maxLength;
}

void Sexy::SMemR(void*& _Src, void* _Dst, size_t _Size)
{
	memcpy(_Dst, _Src, _Size);
	_Src = (void*)((uintptr_t)_Src + _Size);
}

void Sexy::SMemRStr(void*& _Src, std::string& theString)
{
	size_t aStrLen;
	SMemR(_Src, &aStrLen, sizeof(aStrLen));
	theString.resize(aStrLen);
	if (aStrLen > 0)
		SMemR(_Src, &theString[0], aStrLen);
}

void Sexy::SMemW(void*& _Dst, const void* _Src, size_t _Size)
{
	memcpy(_Dst, _Src, _Size);
	_Dst = (void*)((uintptr_t)_Dst + _Size);
}

void Sexy::SMemWStr(void*& _Dst, std::string_view theString)
{
	size_t aStrLen = theString.size();
	SMemW(_Dst, &aStrLen, sizeof(aStrLen));
	SMemW(_Dst, theString.data(), aStrLen);
}
