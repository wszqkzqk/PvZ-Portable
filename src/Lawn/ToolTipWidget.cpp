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

#include "../Resources.h"
#include "ToolTipWidget.h"
#include "../GameConstants.h"
#include "../PvzpLib/PvzpCommon.h"
#include "graphics/Font.h"
#include "../PvzpLib/PvzpStringFile.h"

using namespace Sexy;

ToolTipWidget::ToolTipWidget()
{
	mX = 0;
	mY = 0;
	mWidth = 0;
	mHeight = 0;
	mVisible = 1;
	mCenter = 0;
	mMinLeft = 0;
	mMaxBottom = BOARD_HEIGHT;
	mGetsLinesWidth = 0;
	mWarningFlashCounter = 0;
	mMaxLinesWidth = 0;
}

void ToolTipWidget::GetLines(std::vector<std::string_view>& theLines)
{
	int aLineWidth = 0;
	size_t aLineStart = 0;
	size_t aCurPos = 0;
	char32_t aPrevChar = 0;
	std::string_view aLabelView(mLabel);

	int aBreakDrawLen = -1;
	size_t aBreakResumePos = 0;

	while (aCurPos < mLabel.size())
	{
		size_t aCharStart = aCurPos;
		char32_t aChar;
		if (!Sexy::UTF8DecodeNext(mLabel, aCurPos, aChar))
		{
			aCurPos = aCharStart + 1;
			continue;
		}
		if (aChar == U'\r')  // skip CR for CRLF/LF compatibility
			continue;
		size_t aCharEnd = aCurPos;

		if (aChar == U'\n')
		{
			theLines.push_back(aLabelView.substr(aLineStart, aCharStart - aLineStart));
			aLineWidth = 0;
			aLineStart = aCharEnd;
			aBreakDrawLen = -1;
			aPrevChar = 0;
			continue;
		}

		aLineWidth += FONT_PICO129->CharWidth(aChar);

		if (aChar == U' ')
		{
			aBreakDrawLen = aCharStart - aLineStart;
			aBreakResumePos = aCharEnd;
			if (aLineWidth >= mGetsLinesWidth)
			{
				theLines.push_back(aLabelView.substr(aLineStart, aBreakDrawLen));
				aCurPos = aBreakResumePos;
				while (aCurPos < mLabel.size() && mLabel[aCurPos] == ' ')
					aCurPos++;
				aLineStart = aCurPos;
				aLineWidth = 0;
				aBreakDrawLen = -1;
				aPrevChar = 0;
				continue;
			}
		}
		else if (Sexy::IsAutoBreakChar(aChar) &&
			!Sexy::IsClosingPunctuation(aChar) &&
			aCharStart > aLineStart &&
			!Sexy::IsOpeningPunctuation(aPrevChar))
		{
			aBreakDrawLen = aCharStart - aLineStart;
			aBreakResumePos = aCharStart;
			if (aLineWidth >= mGetsLinesWidth)
			{
				theLines.push_back(aLabelView.substr(aLineStart, aBreakDrawLen));
				aCurPos = aBreakResumePos;
				aLineStart = aCurPos;
				aLineWidth = 0;
				aBreakDrawLen = -1;
				aPrevChar = 0;
				continue;
			}
		}
		aPrevChar = aChar;
	}

	if (aLineStart < mLabel.size())
	{
		theLines.push_back(aLabelView.substr(aLineStart));
	}
}

void ToolTipWidget::CalculateSize()
{
	std::vector<std::string_view> aLines;

	int aTitleWidth = FONT_TINYBOLD->StringWidth(mTitle);
	int aWarningWidth = FONT_PICO129->StringWidth(mWarningText);
	int aMaxWidth = std::max(aTitleWidth, aWarningWidth);

	mGetsLinesWidth = std::max(aMaxWidth - 30, 100);
	if (mMaxLinesWidth > 0)
		mGetsLinesWidth = std::min(mGetsLinesWidth, mMaxLinesWidth);
	GetLines(aLines);

	for (size_t i = 0; i < aLines.size(); i++)
	{
		int aLineWidth = FONT_PICO129->StringWidth(aLines[i]);
		aMaxWidth = std::max(aMaxWidth, aLineWidth);
	}

	int aHeight = 6;
	if (!mTitle.empty())
	{
		aHeight = FONT_TINYBOLD->GetAscent() + 8;
	}
	if (!mWarningText.empty())
	{
		aHeight += FONT_TINYBOLD->GetAscent() + 2;
	}
	aHeight += aLines.size() * FONT_PICO129->GetAscent();

	mWidth = aMaxWidth + 10;
	mHeight = aHeight + aLines.size() * 2 - 2;
}

void ToolTipWidget::SetLabel(std::string_view theLabel)
{
	mLabel = PvzpStringTranslate(theLabel);
	CalculateSize();
}

void ToolTipWidget::SetTitle(std::string_view theTitle)
{
	mTitle = PvzpStringTranslate(theTitle);
	CalculateSize();
}

void ToolTipWidget::SetWarningText(std::string_view theWarningText)
{
	mWarningText = PvzpStringTranslate(theWarningText);
	CalculateSize();
}

void ToolTipWidget::Draw(Graphics* g)
{
	if (!mVisible)
		return;

	int aPosX = mX;
	if (mCenter)
	{
		aPosX -= mWidth / 2;
	}
	if (mMinLeft - g->mTransX > aPosX)  // aPosX + g->mTransX < mMinLeft
	{
		aPosX = mMinLeft - static_cast<int>(g->mTransX);
	}
	else if (aPosX + mWidth + g->mTransX > BOARD_WIDTH)
	{
		aPosX = BOARD_WIDTH - g->mTransX - mWidth;
	}

	int aPosY = mY;
	if (-g->mTransY > aPosY)  // aPosY + g->mTransY > 0
	{
		aPosY = static_cast<int>(-g->mTransY);
	}
	else if (mMaxBottom < mY + mHeight + g->mTransY)
	{
		aPosY = mMaxBottom - static_cast<int>(g->mTransY) - mHeight;
	}

	g->SetColor(Color(255, 255, 200, 255));
	g->FillRect(aPosX, aPosY, mWidth, mHeight);
	g->SetColor(Color::Black);
	g->DrawRect(aPosX, aPosY, mWidth - 1, mHeight - 1);
	aPosY++;

	if (!mTitle.empty())
	{
		g->SetFont(FONT_TINYBOLD);
		g->DrawString(mTitle, aPosX + (mWidth - FONT_TINYBOLD->StringWidth(mTitle)) / 2, aPosY + FONT_TINYBOLD->GetAscent());
		aPosY += FONT_TINYBOLD->GetAscent() + 2;
	}

	if (!mWarningText.empty())
	{
		g->SetFont(FONT_PICO129);
		int x = aPosX + (mWidth - FONT_PICO129->StringWidth(mWarningText)) / 2;
		int y = aPosY + FONT_PICO129->GetAscent();

		Color aWarningColor(255, 0, 0);
		if (mWarningFlashCounter > 0 && mWarningFlashCounter % 20 < 10)
		{
			aWarningColor = Color(0, 0, 0);
		}

		g->SetColor(aWarningColor);
		g->DrawString(mWarningText, x, y);
		g->SetColor(Color::Black);

		aPosY += FONT_PICO129->GetAscent() + 2;
	}

	std::vector<std::string_view> aLines;
	GetLines(aLines);

	g->SetFont(FONT_PICO129);
	for (size_t i = 0; i < aLines.size(); i++)
	{
		std::string_view aLine = aLines[i];
		g->DrawString(aLine, aPosX + (mWidth - FONT_PICO129->StringWidth(aLine)) / 2, aPosY + FONT_PICO129->GetAscent());
		aPosY += FONT_PICO129->GetAscent() + 2;
	}
}
