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

#include "TodDebug.h"
#include "TodCommon.h"
#include "TodStringFile.h"
#include "paklib/PakInterface.h"
#include "graphics/Font.h"

int gTodStringFormatCount;
TodStringListFormat* gTodStringFormats;

const int gLawnStringFormatCount = 12;
TodStringListFormat gLawnStringFormats[12] = {    // GOTY @Patoke: 0x7248EC
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
	{ "CREDITS2",         nullptr,    Color(0,    0,      0,      0),         2,      0U } // @Patoke: wrong size (2 duplicates)
};

TodStringListFormat::TodStringListFormat()
{
	mFormatName = "";
	mNewFont = nullptr;
	mLineSpacingOffset = 0;
	mFormatFlags = 0U;
}

TodStringListFormat::TodStringListFormat(const char* theFormatName, _Font** theFont, const Color& theColor, int theLineSpacingOffset, unsigned int theFormatFlags) : 
	mFormatName(theFormatName), mNewFont(theFont), mNewColor(theColor), mLineSpacingOffset(theLineSpacingOffset), mFormatFlags(theFormatFlags)
{ 
}

void TodStringListSetColors(TodStringListFormat* theFormats, int theCount)
{
	gTodStringFormats = theFormats;
	gTodStringFormatCount = theCount;
}

bool TodStringListReadName(const char*& thePtr, std::string& theName)
{
	const char* aNameStart = strchr(thePtr, '[');
	if (aNameStart == nullptr)  // 如果文本中不存在“[”
	{
		if (strspn(thePtr, " \n\r\t") != strlen(thePtr))  // 如果文本不全是空白字符
		{
			TodTrace("Failed to find string name");
			return false;
		}

		theName = "";
		return true;
	}
	else
	{
		const char* aNameEnd = strchr(aNameStart + 1, ']');
		if (aNameEnd == nullptr)  // 如果“[”后不存在“]”
		{
			TodTrace("Failed to find ']'");
			return false;
		}

		int aCount = aNameEnd - aNameStart - 1;
		theName = Sexy::Trim(std::string(aNameStart + 1, aCount));  // 取得中括号之间的部分并去除字符串前后的空白字符
		if (theName.size() == 0)
		{
			TodTrace("Name Too Short");
			return false;
		}

		thePtr += aCount + 2;  // 移动读取指针至“]”后
		return true;
	}
}

void TodStringRemoveReturnChars(std::string& theString)
{
	for (size_t i = 0; i < theString.size(); )
	{
		if (theString[i] == '\r')
			theString.replace(i, 1, "", 0);  // 原版中此处的“1”和“""”已内联至函数内部
		else
			i++;
	}
}

bool TodStringListReadValue(const char*& thePtr, std::string& theValue)
{
	const char* aValueEnd = strchr(thePtr, '[');
	int aLen = aValueEnd ? aValueEnd - thePtr : strlen(thePtr);
	theValue = Sexy::Trim(std::string(thePtr, aLen));  // 如果存在下一个“[”，则取到“[”前为止；否则，取剩下的全部
	TodStringRemoveReturnChars(theValue);  // 移除所有的换行符
	thePtr += aLen;  // 移动读取指针至“[”处（或结尾处）
	return true;
}

bool TodStringListReadItems(const char* theFileText)
{
	const char* aPtr = theFileText;
	std::string aName;
	std::string aValue;

	for (;;)
	{
		if (!TodStringListReadName(aPtr, aName))  // 读取一个标签
			return false;
		if (aName.size() == 0)  // 读取成功但没有读取到标签，表明读取完成
			return true;
		if (!TodStringListReadValue(aPtr, aValue))  // 读取对应的内容
			return false;

		std::string aNameUpper = Sexy::StringToUpper(aName);
		gSexyAppBase->SetString(aNameUpper, aValue);
	}
}

bool TodStringListReadFile(const char* theFileName)
{
	std::string aFileContent;
	if (!gSexyAppBase->ReadUTF8StringFromFile(theFileName, &aFileContent))
	{
		TodTrace("Failed to open '%s'", theFileName);
		return false;
	}

	return TodStringListReadItems(aFileContent.c_str());
}

void TodStringListLoad(const char* theFileName)
{
	if (!TodStringListReadFile(theFileName))
		TodErrorMessageBox(Sexy::StrFormat("Failed to load string list file '%s'", theFileName).c_str(), "Error");
}

std::string TodStringListFind(const std::string& theName)
{
	auto anItr = gSexyAppBase->mStringProperties.find(theName);
	if (anItr != gSexyAppBase->mStringProperties.end())
	{
		return anItr->second;
	}
	else
	{
		return Sexy::StrFormat("<Missing %s>", theName.c_str());
	}
}

// GOTY @Patoke: 0x523B90
std::string TodStringTranslate(const std::string& theString)
{
	if (theString.size() >= 3 && theString[0] == '[')
	{
		std::string aName = theString.substr(1, theString.size() - 2);  // 取“[”与“]”中间的部分
		return TodStringListFind(aName);
	}
	return theString;
}

std::string TodStringTranslate(const char* theString)
{
	if (theString != nullptr)
	{
		int aLen = strlen(theString);
		if (aLen >= 3 && theString[0] == '[')
		{
			std::string aName(theString, 1, aLen - 2);  // 取“[”与“]”中间的部分
			return TodStringListFind(aName);
		}
		else
			return theString;
	}
	else
		return "";
}

bool TodStringListExists(const std::string& theString)
{
	if (theString.size() >= 3 && theString[0] == '[')
	{
		std::string aName = theString.substr(1, theString.size() - 2);  // 取“[”与“]”中间的部分
		return gSexyAppBase->mStringProperties.find(aName) != gSexyAppBase->mStringProperties.end();
	}
	return false;
}

// GOTY @Patoke: 0x523E20
void TodWriteStringSetFormat(const char* theFormat, TodStringListFormat& theCurrentFormat)
{
	for (int i = 0; i < gTodStringFormatCount; i++)
	{
		const TodStringListFormat& aFormat = gTodStringFormats[i];
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

bool CharIsSpaceInFormat(char theChar, const TodStringListFormat& theCurrentFormat)
{
	return theChar == ' ' || (TestBit(theCurrentFormat.mFormatFlags, TodStringFormatFlag::TOD_FORMAT_IGNORE_NEWLINES) && theChar == '\n');
}

int TodWriteString(Graphics* g, const std::string& theString, int theX, int theY, TodStringListFormat& theCurrentFormat, int theWidth, DrawStringJustification theJustification, bool drawString, int theOffset, int theLength)
{
	_Font* aFont = *theCurrentFormat.mNewFont;
	if (drawString)  // 如果需要实际绘制
	{
		int aSpareX = theWidth - TodWriteString(g, theString, theX, theY, theCurrentFormat, theWidth, DrawStringJustification::DS_ALIGN_LEFT, false, theOffset, theLength);
		switch (theJustification)  // 根据对齐方式调整实际绘制的横坐标
		{
		case DrawStringJustification::DS_ALIGN_RIGHT:
		case DrawStringJustification::DS_ALIGN_RIGHT_VERTICAL_MIDDLE:
			theX += aSpareX;
			break;
		case DrawStringJustification::DS_ALIGN_CENTER:
		case DrawStringJustification::DS_ALIGN_CENTER_VERTICAL_MIDDLE:
			theX += aSpareX / 2;
			break;
		case DrawStringJustification::DS_ALIGN_LEFT:
		case DrawStringJustification::DS_ALIGN_LEFT_VERTICAL_MIDDLE:
			break;
		}
	}

	if (theLength < 0 || theOffset + theLength > static_cast<int>(theString.size()))
		theLength = theString.size();
	else
		theLength = theOffset + theLength;  // 将 theLength 更改为子串结束位置

	std::string aString;
	int aXOffset = 0;
	bool aPrevCharWasSpace = false;
	for (int i = theOffset; i < theLength; i++)
	{
		if (theString[i] == '{')
		{
			const char* aFormatStart = theString.c_str() + i;
			const char* aFormatEnd = strchr(aFormatStart + 1, '}');
			if (aFormatEnd != nullptr)  // 如果存在完整的“{FORMAT}”控制字符
			{
				i += aFormatEnd - aFormatStart;  // i 移动至 "}" 处
				if (drawString)  // 如果需要实际绘制
					aFont->DrawString(g, theX + aXOffset, theY, aString, theCurrentFormat.mNewColor, g->mClipRect);  // 将已经积攒的字符进行绘制
				
				aXOffset += aFont->StringWidth(aString);  // 横向偏移值加上绘制的字符串的宽度
				aString.assign("");  // 清空字符串
				TodWriteStringSetFormat(aFormatStart + 1, theCurrentFormat);  // 根据当前控制字符调整格式
				// _Font* aFont = *theCurrentFormat.mNewFont; // unused
			}
		}
		else
		{
			if (TestBit(theCurrentFormat.mFormatFlags, TodStringFormatFlag::TOD_FORMAT_IGNORE_NEWLINES))  // 如果将换行符视作空格
			{
				if (CharIsSpaceInFormat(theString[i], theCurrentFormat))  // 如果当前字符是空格
				{
					if (!aPrevCharWasSpace)  // 如果前一个字符不是空格
						aString.append(1, ' ');  // 积攒一个空格
					continue;
				}
				else
					aPrevCharWasSpace = false;  // 确保字符串中至多只能连续出现 1 个空格字符
			}

			aString.append(1, theString[i]);
		}
	}

	if (drawString)  // 如果需要实际绘制
		aFont->DrawString(g, theX + aXOffset, theY, aString, theCurrentFormat.mNewColor, g->mClipRect);  // 将已经积攒的字符进行绘制
	return aXOffset + aFont->StringWidth(aString);
}

int TodWriteWordWrappedHelper(Graphics* g, const std::string& theString, int theX, int theY, TodStringListFormat& theCurrentFormat, int theWidth, DrawStringJustification theJustification, bool drawString, int theOffset, int theLength, int theMaxChars)
{
	if (theOffset + theLength > theMaxChars)
	{
		theLength = theMaxChars - theOffset;
		if (theLength <= 0)
			return -1;
	}
	return TodWriteString(g, theString, theX, theY, theCurrentFormat, theWidth, theJustification, drawString, theOffset, theLength);
}

// GOTY @Patoke: 0x5241C0
int TodDrawStringWrappedHelper(Graphics* g, const std::string& theText, const Rect& theRect, _Font* theFont, const Color& theColor, DrawStringJustification theJustification, bool drawString)
{
	int theMaxChars = theText.size();
	TodStringListFormat aCurrentFormat;
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
				TodWriteStringSetFormat(aFormat, aCurrentFormat);
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
			!TestBit(aCurrentFormat.mFormatFlags, TodStringFormatFlag::TOD_FORMAT_IGNORE_NEWLINES);
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
					TodWriteWordWrappedHelper(
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

				aLineWidth = TodWriteWordWrappedHelper(
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

			if (aLineWidth > aMaxWidth)
				aMaxWidth = aLineWidth;
			aYOffset += aLineSpacing;
			aLineFeedPos = aCurPos;
			aBreakDrawLen = -1;
			aCurWidth = 0;
			aPrevChar = 0;
		}
	}

	if (aLineFeedPos < theText.size())
	{
		int aLastLineLength = TodWriteWordWrappedHelper(
			g,
			theText,
			theRect.mX,
			theRect.mY + aYOffset,
			aCurrentFormat,
			theRect.mWidth,
			theJustification,
			drawString,
			aLineFeedPos, // 上次换行的位置即为最后一行开始的位置
			theText.size() - aLineFeedPos, // 绘制部分为从上次换行的位置开始的所有剩余文本
			theMaxChars
		);  // 绘制最后一行的文本
		if (aLastLineLength >= 0)
			aYOffset += aLineSpacing;
	}
	else
		aYOffset += aLineSpacing;

	return (*aCurrentFormat.mNewFont)->GetDescent() + aYOffset - aLineSpacing;
}

// GOTY @Patoke: 0x5246A0
void TodDrawStringWrapped(Graphics* g, const std::string& theText, const Rect& theRect, _Font* theFont, const Color& theColor, DrawStringJustification theJustification)
{
	std::string aTextFinal = TodStringTranslate(theText);
	Rect aRectTodUse = theRect;
	if (theJustification == DrawStringJustification::DS_ALIGN_LEFT_VERTICAL_MIDDLE ||
		theJustification == DrawStringJustification::DS_ALIGN_RIGHT_VERTICAL_MIDDLE ||
		theJustification == DrawStringJustification::DS_ALIGN_CENTER_VERTICAL_MIDDLE)  // 如果纵向需要居中
	{
		aRectTodUse.mY += (aRectTodUse.mHeight - TodDrawStringWrappedHelper(g, aTextFinal, aRectTodUse, theFont, theColor, theJustification, false)) / 2;
	}
	TodDrawStringWrappedHelper(g, aTextFinal, aRectTodUse, theFont, theColor, theJustification, true);
}
