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

#ifndef __TEXTWIDGET_H__
#define __TEXTWIDGET_H__

#include "Widget.h"
#include "ScrollListener.h"

namespace Sexy
{

class ScrollbarWidget;
class _Font;

class TextWidget : public Widget, public ScrollListener
{
public:
	_Font*				mFont;
	ScrollbarWidget*	mScrollbar;

	std::vector<std::string>	mLogicalLines;
	std::vector<std::string>	mPhysicalLines;
	std::vector<int>	mLineMap;
	double				mPosition;
	double				mPageSize;
	bool				mStickToBottom;
	int					mHiliteArea[2][2];
	int					mMaxLines;

public:
	TextWidget();

	virtual std::vector<std::string> GetLines();
	virtual void SetLines(std::vector<std::string> theNewLines);
	virtual void Clear();
	virtual void DrawColorString(Graphics* g, std::string_view theString, int x, int y, bool useColors);
	virtual void DrawColorStringHilited(Graphics* g, std::string_view theString, int x, int y, int theStartPos, int theEndPos);
	virtual int GetStringIndex(std::string_view theString, int thePixel);

	virtual int GetColorStringWidth(std::string_view theString);
	void         Resize(int theX, int theY, int theWidth, int theHeight) override;
	virtual Color GetLastColor(std::string_view theString);
	virtual void AddToPhysicalLines(int theIdx, const std::string& theLine);

	virtual void AddLine(const std::string& theString);
	virtual bool SelectionReversed();
	virtual void GetSelectedIndices(int theLineIdx, int* theIndices);
	void         Draw(Graphics* g) override;
	void         ScrollPosition(int theId, double thePosition) override;
	virtual void GetTextIndexAt(int x, int y, int* thePosArray);
	virtual std::string GetSelection();

	void         MouseDown(int x, int y, int theClickCount) override;
	void         MouseDrag(int x, int y) override;

	void         KeyDown(KeyCode theKey) override;
};

}

#endif //__TEXTWIDGET_H__
