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

#include "PvzpDebug.h"
#include "PvzpCommon.h"
#include "PvzpStringFile.h"
#include "paklib/PakInterface.h"
#include "graphics/Font.h"

int gPvzpStringFormatCount;
PvzpStringListFormat* gPvzpStringFormats;

const int gLawnStringFormatCount = 12;
PvzpStringListFormat gLawnStringFormats[12] = {
	{ "NORMAL",           nullptr,    Color(40,   50,     90,     255),       0,      0U },
	{ "FLAVOR",           nullptr,    Color(143,  67,     27,     255),       0,      1U },
	{ "KEYWORD",          nullptr,    Color(143,  67,     27,     255),       0,      0U },
	{ "NOCTURNAL",        nullptr,    Color(136,  50,     170,    255),       0,      0U },
	{ "AQUATIC",          nullptr,    Color(11,   161,    219,    255),       0,      0U },
	{ "STAT",             nullptr,    Color(204,  36,     29,     255),       0,      0U },
	{ "METAL",            nullptr,    Color(204,  36,     29,     255),       0,      2U },
	{ "KEYMETAL",         nullptr,    Color(143,  67,     27,     255),       0,      2U },
	{ "SHORTLINE",        nullptr,    Color(0,    0,      0,      0),         -9,     0U },
	{ "EXTRASHORTLINE",   nullptr,    Color(0,    0,      0,      0),         -14,    0U },
	{ "CREDITS1",         nullptr,    Color(0,    0,      0,      0),         3,      0U },
	{ "CREDITS2",         nullptr,    Color(0,    0,      0,      0),         2,      0U } // wrong size (2 duplicates)
};

PvzpStringListFormat::PvzpStringListFormat()
{
	mFormatName = "";
	mNewFont = nullptr;
	mLineSpacingOffset = 0;
	mFormatFlags = 0U;
}

PvzpStringListFormat::PvzpStringListFormat(const char* theFormatName, _Font** theFont, const Color& theColor, int theLineSpacingOffset, unsigned int theFormatFlags) :
	mFormatName(theFormatName), mNewFont(theFont), mNewColor(theColor), mLineSpacingOffset(theLineSpacingOffset), mFormatFlags(theFormatFlags)
{
}

void PvzpStringListSetColors(PvzpStringListFormat* theFormats, int theCount)
{
	gPvzpStringFormats = theFormats;
	gPvzpStringFormatCount = theCount;
}

bool PvzpStringListReadName(const char*& thePtr, std::string& theName)
{
	const char* aNameStart = strchr(thePtr, '[');
	if (aNameStart == nullptr)
	{
		if (strspn(thePtr, " \n\r\t") != strlen(thePtr))  // the remaining text is not all whitespace
		{
			PvzpTrace("Failed to find string name");
			return false;
		}

		theName = "";
		return true;
	}
	else
	{
		const char* aNameEnd = strchr(aNameStart + 1, ']');
		if (aNameEnd == nullptr)
		{
			PvzpTrace("Failed to find ']'");
			return false;
		}

		int aCount = aNameEnd - aNameStart - 1;
		theName = Sexy::Trim(std::string(aNameStart + 1, aCount));
		if (theName.size() == 0)
		{
			PvzpTrace("Name Too Short");
			return false;
		}

		thePtr += aCount + 2;  // advance past ']'
		return true;
	}
}

void PvzpStringRemoveReturnChars(std::string& theString)
{
	for (size_t i = 0; i < theString.size(); )
	{
		if (theString[i] == '\r')
			theString.replace(i, 1, "", 0);
		else
			i++;
	}
}

bool PvzpStringListReadValue(const char*& thePtr, std::string& theValue)
{
	const char* aValueEnd = strchr(thePtr, '[');
	int aLen = aValueEnd ? aValueEnd - thePtr : strlen(thePtr);
	theValue = Sexy::Trim(std::string(thePtr, aLen));  // up to the next '[', or the rest of the text
	PvzpStringRemoveReturnChars(theValue);
	thePtr += aLen;  // advance to the next '[' (or the end)
	return true;
}

bool PvzpStringListReadItems(const char* theFileText)
{
	const char* aPtr = theFileText;
	std::string aName;
	std::string aValue;

	for (;;)
	{
		if (!PvzpStringListReadName(aPtr, aName))
			return false;
		if (aName.size() == 0)  // a successful read with an empty name means reading is done
			return true;
		if (!PvzpStringListReadValue(aPtr, aValue))
			return false;

		std::string aNameUpper = Sexy::StringToUpper(aName);
		gSexyAppBase->SetString(aNameUpper, aValue);
	}
}

bool PvzpStringListReadFile(const char* theFileName)
{
	std::string aFileContent;
	if (!gSexyAppBase->ReadUTF8StringFromFile(theFileName, &aFileContent))
	{
		PvzpTrace("Failed to open '%s'", theFileName);
		return false;
	}

	return PvzpStringListReadItems(aFileContent.c_str());
}

void PvzpStringListLoad(const char* theFileName)
{
	if (!PvzpStringListReadFile(theFileName))
		PvzpErrorMessageBox(Sexy::StrFormat("Failed to load string list file '%s'", theFileName).c_str(), "Error");
}

std::string_view PvzpStringListFind(std::string_view theName)
{
	auto anItr = gSexyAppBase->mStringProperties.find(theName);
	if (anItr != gSexyAppBase->mStringProperties.end())
	{
		return anItr->second;
	}
	else
	{
		thread_local std::string aMissing;
		aMissing = "<Missing " + std::string(theName) + ">";
		return aMissing;
	}
}

std::string_view PvzpStringTranslate(std::string_view theString)
{
	if (theString.size() >= 3 && theString[0] == '[')
	{
		std::string_view aName = theString.substr(1, theString.size() - 2);
		return PvzpStringListFind(aName);
	}
	return theString;
}

std::optional<std::string_view> PvzpStringTryTranslate(std::string_view theString)
{
	if (theString.size() >= 3 && theString[0] == '[')
	{
		std::string_view aName = theString.substr(1, theString.size() - 2);
		auto anItr = gSexyAppBase->mStringProperties.find(aName);
		if (anItr != gSexyAppBase->mStringProperties.end())
			return anItr->second;
	}
	return std::nullopt;
}

bool PvzpStringListExists(std::string_view theString)
{
	return PvzpStringTryTranslate(theString).has_value();
}

void PvzpWriteStringSetFormat(const char* theFormat, PvzpStringListFormat& theCurrentFormat)
{
	for (int i = 0; i < gPvzpStringFormatCount; i++)
	{
		const PvzpStringListFormat& aFormat = gPvzpStringFormats[i];
		if (strncmp(theFormat, aFormat.mFormatName, strlen(aFormat.mFormatName)) == 0)
		{
			if (aFormat.mNewFont != nullptr)
				theCurrentFormat.mNewFont = aFormat.mNewFont;
			if (aFormat.mNewColor != Color(0, 0, 0, 0))
				theCurrentFormat.mNewColor = aFormat.mNewColor;
			theCurrentFormat.mLineSpacingOffset = aFormat.mLineSpacingOffset;
			theCurrentFormat.mFormatFlags = aFormat.mFormatFlags;
			return;
		}
	}
}

bool CharIsSpaceInFormat(char theChar, const PvzpStringListFormat& theCurrentFormat)
{
	return theChar == ' ' || (TestBit(theCurrentFormat.mFormatFlags, PvzpStringFormatFlag::PVZP_FORMAT_IGNORE_NEWLINES) && theChar == '\n');
}

int PvzpWriteString(Graphics* g, const std::string& theString, int theX, int theY, PvzpStringListFormat& theCurrentFormat, int theWidth, DrawStringJustification theJustification, bool drawString, int theOffset, int theLength)
{
	_Font* aFont = *theCurrentFormat.mNewFont;
	if (drawString)
	{
		const auto aMeasureSpareX = [&]() -> int {
			PvzpStringListFormat aMeasureFormat = theCurrentFormat;
			return theWidth - PvzpWriteString(g, theString, theX, theY, aMeasureFormat, theWidth, DrawStringJustification::DS_ALIGN_LEFT, false, theOffset, theLength);
		};
		switch (theJustification)
		{
		case DrawStringJustification::DS_ALIGN_RIGHT:
		case DrawStringJustification::DS_ALIGN_RIGHT_VERTICAL_MIDDLE:
			theX += aMeasureSpareX();
			break;
		case DrawStringJustification::DS_ALIGN_CENTER:
		case DrawStringJustification::DS_ALIGN_CENTER_VERTICAL_MIDDLE:
			theX += aMeasureSpareX() / 2;
			break;
		default:
			break;
		}
	}

	if (theLength < 0 || theOffset + theLength > static_cast<int>(theString.size()))
		theLength = theString.size();
	else
		theLength = theOffset + theLength;  // theLength becomes the end position of the substring

	std::string aString;
	int aXOffset = 0;
	bool aPrevCharWasSpace = false;
	for (int i = theOffset; i < theLength; i++)
	{
		if (theString[i] == '{')
		{
			const char* aFormatStart = theString.c_str() + i;
			const char* aFormatEnd = strchr(aFormatStart + 1, '}');
			if (aFormatEnd != nullptr)  // a complete "{FORMAT}" control code
			{
				i += aFormatEnd - aFormatStart;  // move i to '}'
				if (drawString)
					aFont->DrawString(g, theX + aXOffset, theY, aString, theCurrentFormat.mNewColor, g->mClipRect);  // draw the accumulated text

				aXOffset += aFont->StringWidth(aString);
				aString.assign("");
				PvzpWriteStringSetFormat(aFormatStart + 1, theCurrentFormat);
				// _Font* aFont = *theCurrentFormat.mNewFont; // unused
			}
		}
		else
		{
			if (TestBit(theCurrentFormat.mFormatFlags, PvzpStringFormatFlag::PVZP_FORMAT_IGNORE_NEWLINES))  // newlines are treated as spaces
			{
				if (CharIsSpaceInFormat(theString[i], theCurrentFormat))
				{
					if (!aPrevCharWasSpace)
						aString.append(1, ' ');
					continue;
				}
				else
					aPrevCharWasSpace = false;  // collapse consecutive spaces into one
			}

			aString.append(1, theString[i]);
		}
	}

	if (drawString)
		aFont->DrawString(g, theX + aXOffset, theY, aString, theCurrentFormat.mNewColor, g->mClipRect);  // draw the accumulated text
	return aXOffset + aFont->StringWidth(aString);
}

int PvzpWriteWordWrappedHelper(Graphics* g, const std::string& theString, int theX, int theY, PvzpStringListFormat& theCurrentFormat, int theWidth, DrawStringJustification theJustification, bool drawString, int theOffset, int theLength, int theMaxChars)
{
	if (theOffset + theLength > theMaxChars)
	{
		theLength = theMaxChars - theOffset;
		if (theLength <= 0)
			return -1;
	}
	return PvzpWriteString(g, theString, theX, theY, theCurrentFormat, theWidth, theJustification, drawString, theOffset, theLength);
}

int PvzpDrawStringWrappedHelper(Graphics* g, const std::string& theText, const Rect& theRect, _Font* theFont, const Color& theColor, DrawStringJustification theJustification, bool drawString)
{
	int theMaxChars = theText.size();
	PvzpStringListFormat aCurrentFormat;
	aCurrentFormat.mNewFont = &theFont;
	aCurrentFormat.mNewColor = theColor;
	aCurrentFormat.mFormatName = "";
	aCurrentFormat.mLineSpacingOffset = 0;
	aCurrentFormat.mFormatFlags = 0U;

	int aYOffset = theFont->GetAscent() - theFont->GetAscentPadding();
	int aLineSpacing = theFont->GetLineSpacing() + aCurrentFormat.mLineSpacingOffset;
	size_t aLineFeedPos = 0;
	size_t aCurPos = 0;
	int aCurWidth = 0;
	char32_t aCurChar = 0;
	char32_t aPrevChar = 0;
	int aMaxWidth = 0;

	int aBreakDrawLen = -1;       // bytes from aLineFeedPos to draw (-1 = no break point)
	size_t aBreakResumePos = 0;   // byte offset where next line starts
	bool aBreakSkipSpaces = false; // skip consecutive spaces after break

	while (aCurPos < theText.size())
	{
		size_t aCharStart = aCurPos;

		if (theText[aCurPos] == '{')
		{
			const char* aFmtStart = aCurPos + theText.c_str();
			const char* aFormat = aFmtStart + 1;
			const char* aFmtEnd = strchr(aFormat, '}');
			if (aFmtEnd != nullptr)
			{
				aCurPos += aFmtEnd - aFmtStart + 1;
				int aOldAscentOffset = theFont->GetAscent() - theFont->GetAscentPadding();
				Color aExistingColor = aCurrentFormat.mNewColor;
				PvzpWriteStringSetFormat(aFormat, aCurrentFormat);
				aCurrentFormat.mNewColor = aExistingColor;
				int aNewAscentOffset = (*aCurrentFormat.mNewFont)->GetAscent() - (*aCurrentFormat.mNewFont)->GetAscentPadding();
				aLineSpacing = (*aCurrentFormat.mNewFont)->GetLineSpacing() + aCurrentFormat.mLineSpacingOffset;
				aYOffset += aNewAscentOffset - aOldAscentOffset;
				continue;
			}
		}

		if (!Sexy::UTF8DecodeNext(theText, aCurPos, aCurChar))
		{
			aCurPos = aCharStart + 1;
			continue;
		}
		if (aCurChar == U'\r')  // skip CR for CRLF/LF compatibility
			continue;
		size_t aCharEnd = aCurPos;
		bool aIsNewline = (aCurChar == U'\n') &&
			!TestBit(aCurrentFormat.mFormatFlags, PvzpStringFormatFlag::PVZP_FORMAT_IGNORE_NEWLINES);
		bool aIsSpace = !aIsNewline && (aCurChar == U' ' ||
			(aCurChar < 0x80 && CharIsSpaceInFormat(static_cast<char>(aCurChar), aCurrentFormat)));

		if (aIsSpace)
		{
			aBreakDrawLen = aCharStart - aLineFeedPos;
			aBreakResumePos = aCharEnd;
			aBreakSkipSpaces = true;
			aCurChar = U' ';
		}
		else if (aIsNewline)
		{
			aBreakDrawLen = aCharStart - aLineFeedPos;
			aBreakResumePos = aCharEnd;
			aBreakSkipSpaces = false;
			aCurWidth = theRect.mWidth + 1;
		}

		aCurWidth += (*aCurrentFormat.mNewFont)->CharWidthKern(aCurChar, aPrevChar);

		if (!aIsSpace && !aIsNewline && Sexy::IsAutoBreakChar(aCurChar) &&
			!Sexy::IsClosingPunctuation(aCurChar) &&
			aCharStart > aLineFeedPos &&
			!Sexy::IsOpeningPunctuation(aPrevChar))
		{
			aBreakDrawLen = aCharStart - aLineFeedPos;
			aBreakResumePos = aCharStart;
			aBreakSkipSpaces = false;
		}
		aPrevChar = aCurChar;

		if (aCurWidth > theRect.mWidth)
		{
			int aLineWidth;
			if (aBreakDrawLen >= 0)
			{
				int aCurY = static_cast<int>(g->mTransY) + theRect.mY + aYOffset;
				if (aCurY >= g->mClipRect.mY && aCurY <= g->mClipRect.mY + g->mClipRect.mHeight + aLineSpacing)
				{
					PvzpWriteWordWrappedHelper(
						g,
						theText,
						theRect.mX,
						theRect.mY + aYOffset,
						aCurrentFormat,
						theRect.mWidth,
						theJustification,
						drawString,
						aLineFeedPos,
						aBreakDrawLen,
						theMaxChars
					);
				}

				aLineWidth = aCurWidth;
				if (aLineWidth < 0)
					break;

				aCurPos = aBreakResumePos;
				if (aBreakSkipSpaces)
					while (aCurPos < theText.size() && theText[aCurPos] == ' ')
						aCurPos++;
			}
			else
			{
				// No break point: force break, ensure at least one char per line
				size_t aDrawEnd = aCharStart;
				if (aDrawEnd <= aLineFeedPos)
					aDrawEnd = aCharEnd;

				aLineWidth = PvzpWriteWordWrappedHelper(
					g,
					theText,
					theRect.mX,
					theRect.mY + aYOffset,
					aCurrentFormat,
					theRect.mWidth,
					theJustification,
					drawString,
					aLineFeedPos,
					aDrawEnd - aLineFeedPos,
					theMaxChars
				);
				if (aLineWidth < 0)
					break;

				aCurPos = aDrawEnd;
			}

			aMaxWidth = std::max(aMaxWidth, aLineWidth);
			aYOffset += aLineSpacing;
			aLineFeedPos = aCurPos;
			aBreakDrawLen = -1;
			aCurWidth = 0;
			aPrevChar = 0;
		}
	}

	if (aLineFeedPos < theText.size())
	{
		int aLastLineLength = PvzpWriteWordWrappedHelper(
			g,
			theText,
			theRect.mX,
			theRect.mY + aYOffset,
			aCurrentFormat,
			theRect.mWidth,
			theJustification,
			drawString,
			aLineFeedPos, // the last line starts at the last line break
			theText.size() - aLineFeedPos,
			theMaxChars
		);  // draw the last line
		if (aLastLineLength >= 0)
			aYOffset += aLineSpacing;
	}
	else
		aYOffset += aLineSpacing;

	return (*aCurrentFormat.mNewFont)->GetDescent() + aYOffset - aLineSpacing;
}

void PvzpDrawStringWrapped(Graphics* g, std::string_view theText, const Rect& theRect, _Font* theFont, const Color& theColor, DrawStringJustification theJustification)
{
	std::string aTextFinal(PvzpStringTranslate(theText));
	Rect aRectPvzpUse = theRect;
	if (theJustification == DrawStringJustification::DS_ALIGN_LEFT_VERTICAL_MIDDLE ||
		theJustification == DrawStringJustification::DS_ALIGN_RIGHT_VERTICAL_MIDDLE ||
		theJustification == DrawStringJustification::DS_ALIGN_CENTER_VERTICAL_MIDDLE)  // vertical centering required
	{
		aRectPvzpUse.mY += (aRectPvzpUse.mHeight - PvzpDrawStringWrappedHelper(g, aTextFinal, aRectPvzpUse, theFont, theColor, theJustification, false)) / 2;
	}
	PvzpDrawStringWrappedHelper(g, aTextFinal, aRectPvzpUse, theFont, theColor, theJustification, true);
}
