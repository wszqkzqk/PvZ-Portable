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

#include "TextWidget.h"
#include "graphics/Graphics.h"
#include "ScrollbarWidget.h"
#include "WidgetManager.h"
#include "graphics/Font.h"

using namespace Sexy;

TextWidget::TextWidget()
{
	mFont = nullptr;
	mPosition = 0;
	mPageSize = 0;
	mStickToBottom = true;
	mMaxLines = 2048;
	mScrollbar = nullptr;
}

std::vector<std::string> TextWidget::GetLines()
{
	return mLogicalLines;
}

void TextWidget::SetLines(std::vector<std::string> theNewLines)
{
	mLogicalLines = theNewLines;
}

void TextWidget::Clear()
{
	mLogicalLines.clear();
	mPhysicalLines.clear();
	mPosition = 0.0;
	mScrollbar->SetMaxValue(0.0);
	MarkDirty();
}

void TextWidget::DrawColorString(Graphics* g, std::string_view theString, int x, int y, bool useColors)
{
	int aWidth = 0;

	if (useColors)
		g->SetColor(Color(0, 0, 0));

	std::string aCurString = "";
	for (int i = 0; i < (int)theString.length(); i++)
	{
		// Widestrings are cringe, can safely compare to zero
		// if (theString[i] == 0x100)
		if (theString[i] == 0x00)
		{
			if (aCurString.length() > 0)
				g->DrawString(aCurString, x + aWidth, y);

			aWidth += g->GetFont()->StringWidth(aCurString);
			aCurString = "";
			if (useColors)
				g->SetColor(Color(theString[i+1], theString[i+2], theString[i+3]));
			i += 3;
		}
		else
			aCurString += theString[i];
	}

	if (aCurString.length() > 0)
		g->DrawString(aCurString, x + aWidth, y);
}

void TextWidget::DrawColorStringHilited(Graphics* g, std::string_view theString, int x, int y, int theStartPos, int theEndPos)
{
	DrawColorString(g, theString, x, y, true);

	if (theEndPos > theStartPos)
	{
		int aXOfs = GetColorStringWidth(theString.substr(0, theStartPos));
		int aWidth = GetColorStringWidth(theString.substr(0, theEndPos)) - aXOfs;

		Graphics aClipG(*g);
		aClipG.ClipRect(x + aXOfs, y - g->GetFont()->GetAscent(), aWidth, g->GetFont()->GetHeight());

		g->SetColor(Color(0, 0, 128));
		g->FillRect(x + aXOfs, y - g->GetFont()->GetAscent(), aWidth, g->GetFont()->GetHeight());

		aClipG.SetColor(Color(255, 255, 255));
		DrawColorString(&aClipG, theString, x, y, false);
	}
}

int TextWidget::GetStringIndex(std::string_view theString, int thePixel)
{
	int aPos = 0;

	for (int i = 0; i < (int)theString.length(); i++)
	{
		std::string_view aLoSubStr = theString.substr(0, i);
		std::string_view aHiSubStr = theString.substr(0, i+1);

		int aLoLen = GetColorStringWidth(aLoSubStr);
		int aHiLen = GetColorStringWidth(aHiSubStr);
		if (thePixel >= (aLoLen+aHiLen)/2)
			aPos = i+1;
	}

	return aPos;
}

//UNICODE
int TextWidget::GetColorStringWidth(std::string_view theString)
{
	int aWidth = 0;
	std::string aTempString;

	for (int i = 0; i < (int)theString.length(); i++)
	{
		// Removed wide strings because they're cringe
		if (theString[i] == 0x00)
		{
			aWidth += mFont->StringWidth(aTempString);
			aTempString = "";

			i += 3;
		}
		else
			aTempString += theString[i];
	}

	aWidth += mFont->StringWidth(aTempString);

	return aWidth;
}

void TextWidget::Resize(int theX, int theY, int theWidth, int theHeight)
{
	Widget::Resize(theX, theY, theWidth, theHeight);

	double aPageSize = 1;
	if (mHeight > mFont->GetHeight()+16)
		aPageSize = (mHeight - 8.0) / mFont->GetHeight();

	int aLogValue = 0;
	if (mLineMap.size() > 0)
	{
		aLogValue = mLineMap[(int) mScrollbar->mValue];
	}

	int aNewPhysValue = 0;

	mLineMap.clear();
	mPhysicalLines.clear();
	for (int i = 0; i < (int)mLogicalLines.size(); i++)
	{
		if (i == aLogValue)
			aNewPhysValue = mPhysicalLines.size();

		AddToPhysicalLines(i, mLogicalLines[i]);
	}

	bool atBottom = mScrollbar->AtBottom();

	mPageSize = aPageSize;
	mScrollbar->SetMaxValue(mPhysicalLines.size());
	mScrollbar->SetPageSize((int) aPageSize);
	mScrollbar->SetValue(aNewPhysValue);

	if ((mStickToBottom) && (atBottom))
		mScrollbar->GoToBottom();
}

//UNICODE
Color TextWidget::GetLastColor(std::string_view theString)
{
	int anIdx = theString.rfind((char)0xFF);
	if (anIdx < 0)
		return Color(0, 0, 0);

	return Color(theString[anIdx+1], theString[anIdx+2], theString[anIdx+3]);
}

//UNICODE
void TextWidget::AddToPhysicalLines(int theIdx, const std::string& theLine)
{
	std::string aCurString = "";

	if (GetColorStringWidth(theLine) <= mWidth - 8)
	{
		aCurString = theLine;
	}
	else
	{
		int aCurPos = 0;
		while (aCurPos < (int)theLine.length())
		{
			int aNextCheckPos = aCurPos;
			while ((aNextCheckPos < (int)theLine.length()) && (theLine[aNextCheckPos] == ' '))
				aNextCheckPos++;

			int aSpacePos = theLine.find(" ", aNextCheckPos);
			if (aSpacePos == -1)
				aSpacePos = theLine.length();

			std::string aNewString = aCurString + theLine.substr(aCurPos, aSpacePos - aCurPos);
			if (GetColorStringWidth(aNewString) > mWidth-8)
			{
				mPhysicalLines.push_back(aCurString);
				mLineMap.push_back(theIdx);
				Color aColor = GetLastColor(aCurString);
				aCurString = "  " [char(0xFF) + (char) aColor.mRed + (char) aColor.mGreen + (char) aColor.mBlue] +
					theLine.substr(aNextCheckPos, aSpacePos - aNextCheckPos);
			}
			else
				aCurString = aNewString;

			aCurPos = aSpacePos;
		}
	}

	if ((aCurString.compare("") != 0) || (theLine.compare("") == 0))
	{
		mPhysicalLines.push_back(aCurString);
		mLineMap.push_back(theIdx);
	}
}

//UNICODE
void TextWidget::AddLine(const std::string& theLine)
{
	std::string aLine = theLine;

	if (aLine.compare("") == 0)
		aLine = " ";

	bool atBottom = mScrollbar->AtBottom();

	mLogicalLines.push_back(aLine);

	if ((int)mLogicalLines.size() > mMaxLines)
	{
		// Remove an extra 10 lines, for safty
		int aNumLinesToRemove = mLogicalLines.size() - mMaxLines + 10;

		mLogicalLines.erase(mLogicalLines.begin(), mLogicalLines.begin() + aNumLinesToRemove);

		int aPhysLineRemoveCount = 0;

		// Remove all physical lines and linemap entries relating to deleted logical lines
		while (mLineMap[aPhysLineRemoveCount] < aNumLinesToRemove)
		{
			aPhysLineRemoveCount++;
		}

		mLineMap.erase(mLineMap.begin(), mLineMap.begin() + aPhysLineRemoveCount);
		mPhysicalLines.erase(mPhysicalLines.begin(), mPhysicalLines.begin() + aPhysLineRemoveCount);

		// Offset the line map numbers
		int i;
		for (i = 0; i < (int)mLineMap.size(); i++)
		{
			mLineMap[i] -= aNumLinesToRemove;
		}

		// Move the hilited area
		for (i = 0; i < 2; i++)
		{
			mHiliteArea[i][1] -= aNumLinesToRemove;
			if (mHiliteArea[i][1] < 0)
			{
				mHiliteArea[i][0] = 0;
				mHiliteArea[i][1] = 0;
			}
		}

		mScrollbar->SetValue(mScrollbar->mValue - aNumLinesToRemove);
	}

	AddToPhysicalLines(mLogicalLines.size()-1, aLine);

	mScrollbar->SetMaxValue(mPhysicalLines.size());

	if (atBottom)
		mScrollbar->GoToBottom();

	MarkDirty();
}

bool TextWidget::SelectionReversed()
{
	return ((mHiliteArea[1][1] < mHiliteArea[0][1]) ||
			((mHiliteArea[1][1] == mHiliteArea[0][1]) &&
				(mHiliteArea[1][0] < mHiliteArea[0][0])));
}

void TextWidget::GetSelectedIndices(int theLineIdx, int* theIndices)
{
	int aXor = SelectionReversed() ? 1 : 0;
	for (int aPosIdx = 0; aPosIdx < 2; aPosIdx++)
	{
		int aVal;

		if (mHiliteArea[aPosIdx][1] < theLineIdx)
			aVal = 0;
		else if (mHiliteArea[aPosIdx][1] == theLineIdx)
			aVal = mHiliteArea[aPosIdx][0];
		else
			aVal = mPhysicalLines[theLineIdx].length();

		theIndices[aPosIdx ^ aXor] = aVal;
	}
}

void TextWidget::Draw(Graphics* g)
{
	g->SetColor(Color(255, 255, 255));
	g->FillRect(0, 0, mWidth, mHeight);

	Graphics aClipG(*g);
	aClipG.ClipRect(4, 4, mWidth - 8, mHeight - 8);

	aClipG.SetColor(Color(0, 0, 0));
	aClipG.SetFont(mFont);

	int aFirstLine = (int) mPosition;
	int aLastLine = std::min((int) mPhysicalLines.size()-1, (int) mPosition + (int) mPageSize + 1);

	for (int i = aFirstLine; i <= aLastLine; i++)
	{
		int aYPos = 4 + (int) ((i - (int) mPosition)*mFont->GetHeight()) + mFont->GetAscent();
		std::string aString = mPhysicalLines[i];

		int aHilitePos[2];
		GetSelectedIndices(i, aHilitePos);
		DrawColorStringHilited(&aClipG, aString, 4, aYPos, aHilitePos[0], aHilitePos[1]);
	}
}

void TextWidget::ScrollPosition(int theId, double thePosition)
{
	(void)theId;
	mPosition = thePosition;
	MarkDirty();
}

void TextWidget::GetTextIndexAt(int x, int y, int* thePosArray)
{
	int aLineNum = (int) (mScrollbar->mValue + (y / (double) mFont->GetHeight()));
	if (y < 0)
	{
		thePosArray[0] = 0;
		thePosArray[1] = 0;
	}
	else if (aLineNum < (int)mPhysicalLines.size())
	{
		thePosArray[0] = GetStringIndex(mPhysicalLines[aLineNum], x);
		thePosArray[1] = aLineNum;
	}
	else
	{
		if (mPhysicalLines.size() > 0)
		{
			thePosArray[0] = mPhysicalLines[mPhysicalLines.size()-1].length();
			thePosArray[1] = mPhysicalLines.size() - 1;
		}
	}
}

void TextWidget::MouseDown(int x, int y, int theClickCount)
{
	Widget::MouseDown(x, y, theClickCount);

	GetTextIndexAt(x-4, y-4, mHiliteArea[0]);
	mHiliteArea[1][0] = mHiliteArea[0][0];
	mHiliteArea[1][1] = mHiliteArea[0][1];
	MarkDirty();
}

void TextWidget::MouseDrag(int x, int y)
{
	Widget::MouseDrag(x, y);

	GetTextIndexAt(x-4, y-4, mHiliteArea[1]);
	MarkDirty();
}

std::string TextWidget::GetSelection()
{
	std::string aSelString = "";
	int aSelIndices[2];
	bool first = true;

	bool reverse = SelectionReversed();
	for (int aLineNum = mHiliteArea[reverse ? 1 : 0][1]; aLineNum <= mHiliteArea[reverse ? 0 : 1][1]; aLineNum++)
	{
		std::string aString = mPhysicalLines[aLineNum];

		GetSelectedIndices(aLineNum, aSelIndices);

		if (!first)
			aSelString += "\r\n";

		for (int aStrIdx = aSelIndices[0]; aStrIdx < aSelIndices[1]; aStrIdx++)
		{
			char aChar = aString[aStrIdx];
			// Widechars are cringe
			if (aChar != 0x00)
				aSelString += aChar;
			else
				aStrIdx += 3;
		}

		first = false;
	}

	return aSelString;
}

void TextWidget::KeyDown(KeyCode){}
