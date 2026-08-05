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

#ifndef __TOOLTIPWIDGET_H__
#define __TOOLTIPWIDGET_H__

#include "../SexyAppFramework/Common.h"
namespace Sexy
{
	class Graphics;
}

class ToolTipWidget
{
public:
	std::string			mTitle;
	std::string			mLabel;
	std::string			mWarningText;
	int                 mX;
	int                 mY;
	int                 mWidth;
	int                 mHeight;
	bool                mVisible;
	bool                mCenter;
	int                 mMinLeft;
	int                 mMaxBottom;
	int                 mGetsLinesWidth;
	int                 mWarningFlashCounter;
	int                 mMaxLinesWidth;         // max line width override (0 = auto)

public:
	ToolTipWidget();

	void                Draw(Sexy::Graphics* g);
	void                SetLabel(std::string_view theLabel);
	void                SetTitle(std::string_view theTitle);
	void                SetWarningText(std::string_view theWarningText);
	void                CalculateSize();
	void                GetLines(std::vector<std::string>& theLines);
	inline void         FlashWarning() { mWarningFlashCounter = 70; }
	inline void         Update() { if (mWarningFlashCounter > 0) mWarningFlashCounter--; }
	inline void         SetPosition(int theX, int theY) { mX = theX; mY = theY; }
};

#endif
