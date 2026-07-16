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

#ifndef __EDITWIDGET_H__
#define __EDITWIDGET_H__

#include "Widget.h"

namespace Sexy
{

class _Font;
class EditListener;

class EditWidget : public Widget
{
public:
	enum
	{
		COLOR_BKG,
		COLOR_OUTLINE,
		COLOR_TEXT,
		COLOR_HILITE,
		COLOR_HILITE_TEXT,
		NUM_COLORS
	};

	int						mId;
	std::string				mString;
	_Font*					mFont;

	struct WidthCheck
	{
		_Font *mFont;
		int mWidth;
	};
	typedef std::list<WidthCheck> WidthCheckList;
	WidthCheckList				mWidthCheckList;
	
	EditListener*			mEditListener;		
	bool					mShowingCursor;
	bool					mDrawSelOverride; // set this to true to draw selected text even when not in focus
	bool					mHadDoubleClick;	// Used to fix a bug with double clicking to hilite a word after the widget manager started calling mouse drag before mouse down/up events
	bool					mHadFocusBeforePress; // mHasFocus snapshot taken in WantsFocus (pre-SetFocus)
	int						mCursorPos;
	int						mHilitePos;
	int						mBlinkAcc;
	int						mBlinkDelay;
	int						mLeftPos;		
	int						mMaxChars;
	int						mMaxPixels;
	
	std::string				mUndoString;
	int						mUndoCursor;
	int						mUndoHilitePos;
	int						mLastModifyIdx;


protected:
	virtual void			ProcessKey(KeyCode theKey, char theChar);
	void					InsertTextAtCursor(std::string_view theText);
	virtual void			HiliteWord();
	void					UpdateCaretPos();
	void					UpdateTextInputArea();
	int						GetCaretXOffset();

public:
	virtual void			SetFont(_Font* theFont, _Font* theWidthCheckFont = nullptr);
	virtual void			SetText(const std::string& theText, bool leftPosToZero = true);
	virtual bool			IsPartOfWord(char32_t theChar);
	virtual int				GetCharAt(int x, int y);

	void					Resize(int theX, int theY, int theWidth, int theHeight) override;
	void					Draw(Graphics* g) override; // Already translated;

	void					Update() override;
	void					MarkDirty() override;

	bool					WantsFocus() override;
	void					GotFocus() override;
	void					LostFocus() override;
	virtual void			FocusCursor(bool bigJump);

	void					KeyDown(KeyCode theKey) override;
	void					KeyChar(char theChar) override;
	void					KeyText(std::string_view theText) override;

	void					MouseDown(int x, int y, int theClickCount) override { Widget::MouseDown(x, y, theClickCount); }
	void					MouseDown(int x, int y, int theBtnNum, int theClickCount) override;
	void					MouseUp(int x, int y) override { Widget::MouseUp(x, y); }
	void					MouseUp(int x, int y, int theClickCount) override { Widget::MouseUp(x, y, theClickCount); }
	void					MouseUp(int x, int y, int theBtnNum, int theClickCount) override;
	void					MouseDrag(int x, int y) override;
	void					MouseEnter() override;
	void					MouseLeave() override;
	void					ClearWidthCheckFonts();
	void					AddWidthCheckFont(_Font *theFont, int theMaxPixels = -1); // defaults to mMaxPixels
	void					EnforceMaxChars();
	void					EnforceMaxPixels();

public:
	EditWidget(int theId, EditListener* theEditListener);
	~EditWidget() override;
};

}

#endif //__EDITWIDGET_H__
