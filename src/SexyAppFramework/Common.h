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

#ifndef __SEXYAPPFRAMEWORK_COMMON_H__
#define __SEXYAPPFRAMEWORK_COMMON_H__

#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <cstdlib>
#include <cstdint>
#include <cstdarg>
#include <ctime>
#include <filesystem>
#include <string_view>
#include <type_traits>
#include <bit>

#ifdef _MSC_VER
#include <intrin.h>
#endif

#ifdef _WIN32
#undef _WIN32_WINNT
#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500
#undef _UNICODE
#undef UNICODE
#define NOMINMAX 1
#include <windows.h>
#include <shellapi.h>
#include <mmsystem.h>
#else
#include <strings.h>
#endif

// fallback if NOMINMAX fails (somehow?)
#undef min
#undef max

// Define unreachable()
#ifdef _MSC_VER
#include <direct.h>
#include <io.h>
#define unreachable() __assume(0)
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define getcwd _getcwd
#define access _access
#define F_OK 0

inline int nanosleep(const struct timespec* ts, struct timespec* rem)
{
	// rem is not implemented
	HANDLE timer = CreateWaitableTimer(nullptr, TRUE, nullptr);
	if (!timer)
		return -1;

	// SetWaitableTimer() defines interval in 100ns units.
	// negative is to indicate relative time.
	time_t sec = ts->tv_sec;
	long nsec = ts->tv_nsec;
	if (sec < 0 || (sec == 0 && nsec <= 0))
	{
		CloseHandle(timer);
		return 0;
	}

	LARGE_INTEGER delay;
	delay.QuadPart = -((LONGLONG)sec * 10000000LL + (LONGLONG)nsec / 100LL);
	BOOL ok = SetWaitableTimer(timer, &delay, 0, nullptr, nullptr, FALSE) &&
		WaitForSingleObject(timer, INFINITE) == WAIT_OBJECT_0;

	CloseHandle(timer);

	if (!ok)
		return -1;

	return 0;
}
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5)) || defined(__clang__)
#define unreachable() __builtin_unreachable()
#else
#define unreachable() ((void)0)
#endif

#define LENGTH(anyarray) (sizeof(anyarray) / sizeof(anyarray[0]))

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef int64_t int64;

typedef std::map<std::string, std::string> DefinesMap;
typedef std::vector<char> CharVector;

namespace Sexy
{

const ulong SEXY_RAND_MAX = 0x7FFFFFFF;

extern bool			gDebug;

void				PrintF(const char *text, ...);
void				LogError(const char* theFormat, ...);

int					Rand();
int					Rand(int range);
float				Rand(float range);
void				SRand(ulong theSeed);
extern std::string	VFormat(const char* fmt, va_list argPtr);
extern std::string	StrFormat(const char* fmt ...);
std::string			GetAppDataFolder();
void				SetAppDataFolder(std::string_view thePath);
std::string			GetAppDataPath(std::string_view theRelativePath);
const std::string&	GetResourceFolder();
void				SetResourceFolder(std::string_view thePath);
std::string			GetResourcePath(std::string_view theRelativePath);
std::string			StringToUpper(std::string_view theString);
std::string			StringToLower(std::string_view theString);
std::string			Upper(std::string_view theData);
std::string			Lower(std::string_view theData);
std::string			Trim(std::string_view theString);
bool				StringToInt(const std::string& theString, int* theIntVal);
bool				StringToDouble(const std::string& theString, double* theDoubleVal);
int					StrFindNoCase(const char *theStr, const char *theFind);
bool				StrPrefixNoCase(const char *theStr, const char *thePrefix, int maxLength = 10000000);
std::string			CommaSeperate(int theValue);
std::string			Evaluate(std::string_view theString, const DefinesMap& theDefinesMap);
std::string			XMLDecodeString(std::string_view theString);
std::string			XMLEncodeString(std::string_view theString);

bool				Deltree(std::string_view thePath);
bool				FileExists(std::string_view theFileName);
void				MkDir(std::string_view theDir);
std::string			GetFileName(std::string_view thePath, bool noExtension = false);
std::string			GetFileDir(std::string_view thePath, bool withSlash = false);
std::string			RemoveTrailingSlash(std::string_view theDirectory);
std::string			GetCurDir();
std::string			GetFullPath(std::string_view theRelPath);
std::string			GetPathFrom(std::string_view theRelPath, std::string_view theDir);
bool				IsPathRooted(std::string_view thePath);
bool				AllowAllAccess(std::string_view theFileName);

// Read memory and then move the pointer
void				SMemR(void*& _Src, void* _Dst, size_t _Size);
void				SMemRStr(void*& _Src, std::string& theString);
// Write memory and then move the pointer
void				SMemW(void*& _Dst, const void* _Src, size_t _Size);
void				SMemWStr(void*& _Dst, std::string_view theString);

inline void			inlineLTrim(std::string &theData, std::string_view theChars = " \t\r\n")
{
    theData.erase(0, theData.find_first_not_of(theChars));
}

inline void			inlineRTrim(std::string &theData, std::string_view theChars = " \t\r\n")
{
    theData.resize(theData.find_last_not_of(theChars) + 1);
}

inline void			inlineTrim(std::string &theData, std::string_view theChars = " \t\r\n")
{
	inlineRTrim(theData, theChars);
	inlineLTrim(theData, theChars);
}

// Decode next UTF-8 codepoint, advancing theOffset. Returns false on end/error.
inline bool UTF8DecodeNext(std::string_view theString, size_t& theOffset, char32_t& theOutChar)
{
	if (theOffset >= theString.size())
		return false;

	unsigned char aFirst = static_cast<unsigned char>(theString[theOffset]);
	if (aFirst < 0x80)
	{
		theOutChar = aFirst;
		theOffset += 1;
		return true;
	}

	size_t aLength;
	if ((aFirst & 0xE0) == 0xC0) aLength = 2;
	else if ((aFirst & 0xF0) == 0xE0) aLength = 3;
	else if ((aFirst & 0xF8) == 0xF0) aLength = 4;
	else return false;

	if (theOffset + aLength > theString.size())
		return false;

	for (size_t i = 1; i < aLength; i++)
		if ((static_cast<unsigned char>(theString[theOffset + i]) & 0xC0) != 0x80)
			return false;

	char32_t aValue = 0;
	if (aLength == 2)
		aValue = ((aFirst & 0x1F) << 6) |
			(static_cast<unsigned char>(theString[theOffset + 1]) & 0x3F);
	else if (aLength == 3)
		aValue = ((aFirst & 0x0F) << 12) |
			((static_cast<unsigned char>(theString[theOffset + 1]) & 0x3F) << 6) |
			(static_cast<unsigned char>(theString[theOffset + 2]) & 0x3F);
	else
		aValue = ((aFirst & 0x07) << 18) |
			((static_cast<unsigned char>(theString[theOffset + 1]) & 0x3F) << 12) |
			((static_cast<unsigned char>(theString[theOffset + 2]) & 0x3F) << 6) |
			(static_cast<unsigned char>(theString[theOffset + 3]) & 0x3F);

	theOutChar = aValue;
	theOffset += aLength;
	return true;
}

// Opening punctuation that must not appear at end of a line
inline bool IsOpeningPunctuation(char32_t theChar)
{
	switch (theChar)
	{
	case U'〈': case U'《': case U'「': case U'『':
	case U'【': case U'〔': case U'〖': case U'〘':
	case U'〚':
	case U'（': case U'［': case U'｛':
	case U'\u2018': case U'\u201A': case U'\u201B': case U'\u201C':  // ' ‚ ‛ "
		return true;
	default:
		return false;
	}
}

// Closing punctuation that must not appear at start of a line
inline bool IsClosingPunctuation(char32_t theChar)
{
	switch (theChar)
	{
	case U'〉': case U'》': case U'」': case U'』':
	case U'】': case U'〕': case U'〗': case U'〙':
	case U'〛':
	case U'）': case U'］': case U'｝':
	case U'\u2019': case U'\u201D':  // ’ ”
	case U'、': case U'。':
	case U'，': case U'．':
	case U'！': case U'？':
	case U'：': case U'；':
		return true;
	default:
		return false;
	}
}

// Characters that allow auto line-break
inline bool IsAutoBreakChar(char32_t theChar)
{
	if (theChar < 0x80)
		return false;
	return (theChar >= 0x2018 && theChar <= 0x201D) ||  // Curly quotes
		(theChar >= 0x2600 && theChar <= 0x27BF) ||  // Misc Symbols, Dingbats
		(theChar >= 0x3000 && theChar <= 0x303F) ||  // CJK Symbols and Punctuation
		(theChar >= 0x3040 && theChar <= 0x309F) ||  // Hiragana
		(theChar >= 0x30A0 && theChar <= 0x30FF) ||  // Katakana
		(theChar >= 0x3400 && theChar <= 0x4DBF) ||  // CJK Extension A
		(theChar >= 0x4E00 && theChar <= 0x9FFF) ||  // CJK Unified Ideographs
		(theChar >= 0xAC00 && theChar <= 0xD7AF) ||  // Hangul Syllables
		(theChar >= 0xF900 && theChar <= 0xFAFF) ||  // CJK Compatibility Ideographs
		(theChar >= 0xFE30 && theChar <= 0xFE4F) ||  // CJK Compatibility Forms
		(theChar >= 0xFF01 && theChar <= 0xFF60) ||  // Fullwidth Forms
		(theChar >= 0x1F300 && theChar <= 0x1FAFF) || // Emoji Symbols & Pictographs
		(theChar >= 0x20000 && theChar <= 0x2FA1F);  // CJK Extension B-F & Supplements
}

// UTF-8 path conversion helpers for Windows Unicode path support
#ifdef _WIN32
inline std::filesystem::path PathFromU8(std::string_view s)
{
	return std::filesystem::path(std::u8string_view(reinterpret_cast<const char8_t*>(s.data()), s.size()));
}

inline std::string PathToU8(const std::filesystem::path& p)
{
	auto u8 = p.generic_u8string();
	return std::string(u8.begin(), u8.end());
}
#else
inline std::filesystem::path PathFromU8(std::string_view s) { return std::filesystem::path(s); }
inline std::string PathToU8(const std::filesystem::path& p) { return p.string(); }
#endif

// Byte swap helpers
inline constexpr uint16_t ByteSwap16(uint16_t v) noexcept
{
	if (!std::is_constant_evaluated()) {
#if defined(__has_builtin)
#	if __has_builtin(__builtin_bswap16)
		return __builtin_bswap16(v);
#	endif
#elif defined(_MSC_VER)
		return _byteswap_ushort(v);
#endif
	}
	return (v >> 8) | (v << 8);
}

inline constexpr uint32_t ByteSwap32(uint32_t v) noexcept
{
	if (!std::is_constant_evaluated()) {
#if defined(__has_builtin)
#	if __has_builtin(__builtin_bswap32)
		return __builtin_bswap32(v);
#	endif
#elif defined(_MSC_VER)
		return _byteswap_ulong(v);
#endif
	}
	return ((v & 0x000000FFu) << 24) |
			((v & 0x0000FF00u) <<  8) |
			((v & 0x00FF0000u) >>  8) |
			((v & 0xFF000000u) >> 24);
}

inline constexpr uint64_t ByteSwap64(uint64_t v) noexcept
{
	if (!std::is_constant_evaluated()) {
#if defined(__has_builtin)
#	if __has_builtin(__builtin_bswap64)
		return __builtin_bswap64(v);
#	endif
#elif defined(_MSC_VER)
		return _byteswap_uint64(v);
#endif
	}
	return ((v & 0x00000000000000FFULL) << 56) |
			((v & 0x000000000000FF00ULL) << 40) |
			((v & 0x0000000000FF0000ULL) << 24) |
			((v & 0x00000000FF000000ULL) <<  8) |
			((v & 0x000000FF00000000ULL) >>  8) |
			((v & 0x0000FF0000000000ULL) >> 24) |
			((v & 0x00FF000000000000ULL) >> 40) |
			((v & 0xFF00000000000000ULL) >> 56);
}

// Endian conversion helpers
template <typename T, std::endian TargetEndian>
inline constexpr T EndianConvert(T v) noexcept
{
	if constexpr (std::endian::native != TargetEndian)
	{
		if constexpr (sizeof(T) == 2)
			return static_cast<T>(ByteSwap16(static_cast<uint16_t>(v)));
		if constexpr (sizeof(T) == 4)
			return static_cast<T>(ByteSwap32(static_cast<uint32_t>(v)));
		if constexpr (sizeof(T) == 8)
			return static_cast<T>(ByteSwap64(static_cast<uint64_t>(v)));
	}
	return v;
}

inline constexpr uint16_t FromLE16(uint16_t v) noexcept { return EndianConvert<uint16_t, std::endian::little>(v); }
inline constexpr uint16_t ToLE16(uint16_t v) noexcept { return EndianConvert<uint16_t, std::endian::little>(v); }
inline constexpr uint32_t FromLE32(uint32_t v) noexcept { return EndianConvert<uint32_t, std::endian::little>(v); }
inline constexpr uint32_t ToLE32(uint32_t v) noexcept { return EndianConvert<uint32_t, std::endian::little>(v); }
inline constexpr uint64_t FromLE64(uint64_t v) noexcept { return EndianConvert<uint64_t, std::endian::little>(v); }
inline constexpr uint64_t ToLE64(uint64_t v) noexcept { return EndianConvert<uint64_t, std::endian::little>(v); }
inline constexpr uint16_t FromBE16(uint16_t v) noexcept { return EndianConvert<uint16_t, std::endian::big>(v); }
inline constexpr uint16_t ToBE16(uint16_t v) noexcept { return EndianConvert<uint16_t, std::endian::big>(v); }
inline constexpr uint32_t FromBE32(uint32_t v) noexcept { return EndianConvert<uint32_t, std::endian::big>(v); }
inline constexpr uint32_t ToBE32(uint32_t v) noexcept { return EndianConvert<uint32_t, std::endian::big>(v); }
inline constexpr uint64_t FromBE64(uint64_t v) noexcept { return EndianConvert<uint64_t, std::endian::big>(v); }
inline constexpr uint64_t ToBE64(uint64_t v) noexcept { return EndianConvert<uint64_t, std::endian::big>(v); }

struct StringLessNoCase { bool operator()(const std::string &s1, const std::string &s2) const { return strcasecmp(s1.c_str(),s2.c_str())<0; } };

}

#endif //__SEXYAPPFRAMEWORK_COMMON_H__
