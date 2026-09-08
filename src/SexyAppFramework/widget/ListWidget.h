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

#ifndef __LISTWIDGET_H__
#define __LISTWIDGET_H__

#include "ScrollListener.h"
#include "Widget.h"
#include <optional>

namespace Sexy
{

class ScrollbarWidget;
class ListListener;
class _Font;

struct ListWidgetColorScheme
{
	Color					mBkg;
	Color					mOutline;
	Color					mText;
	Color					mHilite;
	Color					mSelect;
	std::optional<Color>	mSelectText;
};

class ListWidget : public Widget, public ScrollListener
{
public:
	enum
	{
		JUSTIFY_LEFT			=0,
		JUSTIFY_CENTER,
		JUSTIFY_RIGHT
	};

public:
	int							mId;
	_Font*						mFont;
	ScrollbarWidget*			mScrollbar;
	int							mJustify;
	ListWidgetColorScheme		mColors;

	std::vector<std::string>	mLines;
	std::vector<Color>			mLineColors;
	double						mPosition;
	double						mPageSize;
	int							mHiliteIdx;
	int							mSelectIdx;
	ListListener*				mListListener;
	ListWidget*					mParent;
	ListWidget*					mChild;
	bool						mSortFromChild;
	bool						mDrawOutline;
	int							mMaxNumericPlaces;
	int							mItemHeight;

	bool						mDrawSelectWhenHilited;
	bool						mDoFingerWhenHilited;

	void						SetHilite(int theHiliteIdx, bool notifyListener = false);

public:
	ListWidget(int theId, _Font *theFont, ListListener *theListListener);
	~ListWidget() override;

	void						RemovedFromManager(WidgetManager *theManager) override;

	virtual std::string			GetSortKey(int theIdx);
	virtual void				Sort(bool ascending);
	virtual std::string			GetStringAt(int theIdx);
	void						Resize(int theX, int theY, int theWidth, int theHeight) override;
	virtual void				SetColors(const ListWidgetColorScheme& theColors);
	virtual int					AddLine(const std::string& theLine, bool alphabetical);
	virtual void				SetLine(int theIdx, const std::string& theString);
	virtual int					GetLineCount();
	virtual int					GetLineIdx(const std::string& theLine);
	virtual void				SetColor(const std::string& theLine, const Color& theColor);
	virtual void				SetLineColor(int theIdx, const Color& theColor);
	virtual void				RemoveLine(int theIdx);
	virtual void				RemoveAll();
	virtual int					GetOptimalWidth();
	virtual int					GetOptimalHeight();
	void						OrderInManagerChanged() override;
	void						Draw(Graphics *g) override;
	void						ScrollPosition(int theId, double thePosition) override;
	void						MouseMove(int x, int y) override;
	void						MouseWheel(int theDelta) override;
	void						MouseDown(int x, int y, int theClickCount) override { Widget::MouseDown(x, y, theClickCount); }
	void						MouseDown(int x, int y, int theBtnNum, int theClickCount) override;
	void						MouseLeave() override;
	virtual void				SetSelect(int theSelectIdx);
};

}

#endif // __LISTWIDGET_H__
