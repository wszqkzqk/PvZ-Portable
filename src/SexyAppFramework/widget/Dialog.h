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

#ifndef __DIALOG_H__
#define __DIALOG_H__

#include "Widget.h"
#include "ButtonListener.h"
#include <memory>

namespace Sexy
{

class DialogListener;
class ButtonWidget;
class DialogButton;
class _Font;

extern std::string DIALOG_YES_STRING;
extern std::string DIALOG_NO_STRING;
extern std::string DIALOG_OK_STRING;
extern std::string DIALOG_CANCEL_STRING;

struct DialogColorScheme
{
	Color					mHeader;
	Color					mLines;
	Color					mFooter;
	Color					mButtonText;
	Color					mButtonTextHilite;
	Color					mBkg;
	Color					mOutline;
};

class Dialog : public Widget, public ButtonListener
{
public:
	enum
	{
		BUTTONS_NONE,
		BUTTONS_YES_NO,
		BUTTONS_OK_CANCEL,
		BUTTONS_FOOTER
	};

	enum
	{
		ID_YES		= 1000,
		ID_NO		= 1001,
		ID_OK		= 1000,
		ID_CANCEL	= 1001,
		ID_FOOTER	= 1000
	};

	DialogListener*			mDialogListener;
	Image*					mComponentImage;
	DialogColorScheme		mColors;
	std::unique_ptr<DialogButton>	mYesButton;
	std::unique_ptr<DialogButton>	mNoButton;
	int						mNumButtons;

	std::string				mDialogHeader;
	std::string				mDialogFooter;
	std::string				mDialogLines;

	int						mButtonMode;
	std::unique_ptr<_Font>		mHeaderFont;
	std::unique_ptr<_Font>		mLinesFont;
	int						mTextAlign;
	int						mLineSpacingOffset;
	int						mButtonHeight;
	Insets					mBackgroundInsets;
	Insets					mContentInsets;
	int						mSpaceAfterHeader;
	bool					mDragging;
	int						mDragMouseX;
	int						mDragMouseY;
	int						mId;
	bool					mIsModal;
	int						mResult;

	int						mButtonHorzSpacing;
	int						mButtonSidePadding;


public:
	void					EnsureFonts();

public:
	Dialog(Image* theComponentImage, Image* theButtonComponentImage,
		int theId, bool isModal, const std::string& theDialogHeader, const std::string& theDialogLines, const std::string& theDialogFooter, int theButtonMode); //UNICODE

	~Dialog() override;

	virtual void			SetButtonFont(_Font* theFont);
	virtual void			SetHeaderFont(_Font* theFont);
	virtual void			SetLinesFont(_Font* theFont);

	virtual void			SetColors(const DialogColorScheme& theColors);
	virtual void			SetHeaderColor(const Color& theColor);
	virtual void			SetLinesColor(const Color& theColor);
	virtual void			SetButtonTextColor(const Color& theColor);
	virtual void			SetButtonTextHiliteColor(const Color& theColor);
	virtual int				GetPreferredHeight(int theWidth);

	void					Draw(Graphics* g) override;
	void					AddedToManager(WidgetManager* theWidgetManager) override;
	void					RemovedFromManager(WidgetManager* theWidgetManager) override;
	void					OrderInManagerChanged() override;
	void					Resize(int theX, int theY, int theWidth, int theHeight) override;

	void					MouseDown(int x, int y, int theClickCount) override { Widget::MouseDown(x, y, theClickCount); }
	void					MouseDown(int x, int y, int theBtnNum, int theClickCount) override;
	void					MouseDrag(int x, int y) override;
	void					MouseUp(int x, int y) override { Widget::MouseUp(x, y); }
	void					MouseUp(int x, int y, int theClickCount) override { Widget::MouseUp(x, y, theClickCount); }
	void					MouseUp(int x, int y, int theBtnNum, int theClickCount) override;
	void					Update() override;
	virtual	bool			IsModal();
	virtual int				WaitForResult(bool autoKill = true);

	void					ButtonPress(int theId) override;
	void					ButtonDepress(int theId) override;
	void					ButtonDownTick(int) override{}
	void					ButtonMouseEnter(int) override{}
	void					ButtonMouseLeave(int) override{}
	void					ButtonMouseMove(int, int, int) override{}
};

}

#endif //__DIALOG_H__
