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

#ifndef __PVZPSTRINGFILE_H__
#define __PVZPSTRINGFILE_H__

#include "graphics/Graphics.h"
#include "../ConstEnums.h"
using namespace Sexy;

//enum DrawStringJustification;
enum PvzpStringFormatFlag
{
	PVZP_FORMAT_IGNORE_NEWLINES,
	PVZP_FORMAT_HIDE_UNTIL_MAGNETSHROOM
};

class PvzpStringListFormat
{
public:
	const char*     mFormatName;
	_Font**          mNewFont;
	Color           mNewColor;
	int             mLineSpacingOffset;
	unsigned int    mFormatFlags;

public:
	PvzpStringListFormat();
	PvzpStringListFormat(const char* theFormatName, _Font** theFont, const Color& theColor, int theLineSpacingOffset, unsigned int theFormatFlags);
};
extern int gPvzpStringFormatCount;
extern PvzpStringListFormat* gPvzpStringFormats;

extern const int gLawnStringFormatCount;
extern PvzpStringListFormat gLawnStringFormats[12];

void                PvzpStringListSetColors(PvzpStringListFormat* theFormats, int theCount);
void                PvzpWriteStringSetFormat(const char* theFormat, PvzpStringListFormat& theCurrentFormat);
bool                PvzpStringListReadName(const char*& thePtr, std::string& theName);
bool                PvzpStringListReadValue(const char*& thePtr, std::string& theValue);
bool                PvzpStringListReadItems(const char* theFileText);
bool                PvzpStringListReadFile(const char* theFileName);
void                PvzpStringListLoad(const char* theFileName);
std::string         PvzpStringListFind(std::string_view theName);
std::string			PvzpStringTranslate(std::string_view theString);
std::string			PvzpStringTranslate(const char* theString);
bool                PvzpStringListExists(std::string_view theString);
void                PvzpStringRemoveReturnChars(std::string& theString);
bool                CharIsSpaceInFormat(char theChar, const PvzpStringListFormat& theCurrentFormat);
int                 PvzpWriteString(Graphics* g, const std::string& theString, int theX, int theY, PvzpStringListFormat& theCurrentFormat, int theWidth, DrawStringJustification theJustification, bool drawString, int theOffset, int theLength);
int      PvzpWriteWordWrappedHelper(Graphics* g, const std::string& theString, int theX, int theY, PvzpStringListFormat& theCurrentFormat, int theWidth, DrawStringJustification theJustification, bool drawString, int theOffset, int theLength, int theMaxChars);
int                 PvzpDrawStringWrappedHelper(Graphics* g, const std::string& theText, const Rect& theRect, _Font* theFont, const Color& theColor, DrawStringJustification theJustification, bool drawString);
void		PvzpDrawStringWrapped(Graphics* g, std::string_view theText, const Rect& theRect, _Font* theFont, const Color& theColor, DrawStringJustification theJustification);

#endif  //__PVZPSTRINGFILE_H__
