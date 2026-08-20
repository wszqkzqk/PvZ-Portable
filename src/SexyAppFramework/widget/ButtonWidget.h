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

#ifndef __BUTTONWIDGET_H__
#define __BUTTONWIDGET_H__

#include "Widget.h"
#include <memory>

namespace Sexy
{

class Image;
class ButtonListener;

class ButtonWidget : public Widget
{
public:
	enum {
		BUTTON_LABEL_LEFT	= -1,
		BUTTON_LABEL_CENTER,
		BUTTON_LABEL_RIGHT
	};
	enum
	{
		COLOR_LABEL,
		COLOR_LABEL_HILITE,
		COLOR_DARK_OUTLINE,
		COLOR_LIGHT_OUTLINE,
		COLOR_MEDIUM_OUTLINE,
		COLOR_BKG,
		NUM_COLORS
	};

	int						mId;
	std::string				mLabel;
	int						mLabelJustify;
	std::unique_ptr<_Font>		mFont;
	Image*					mButtonImage;
	Image*					mOverImage;
	Image*					mDownImage;
	Image*					mDisabledImage;
	Rect					mNormalRect;
	Rect					mOverRect;
	Rect					mDownRect;
	Rect					mDisabledRect;

	bool					mInverted;
	bool					mBtnNoDraw;
	bool					mFrameNoDraw;
	ButtonListener*			mButtonListener;

	double					mOverAlpha;
	double					mOverAlphaSpeed;
	double					mOverAlphaFadeInSpeed;

	bool					HaveButtonImage(Image *theImage, const Rect &theRect);
	virtual void			DrawButtonImage(Graphics *g, Image *theImage, const Rect &theRect, int x, int y);


public:
	ButtonWidget(int theId, ButtonListener* theButtonListener);
	~ButtonWidget() override;

	virtual void			SetFont(_Font* theFont);
	virtual bool			IsButtonDown();
	void					Draw(Graphics* g) override;
	void					SetDisabled(bool isDisabled) override;
	void					MouseEnter() override;
	void					MouseLeave() override;
	void					MouseMove(int theX, int theY) override;
	void					MouseDown(int theX, int theY, int theClickCount) override { Widget::MouseDown(theX, theY, theClickCount); }
	void					MouseDown(int theX, int theY, int theBtnNum, int theClickCount) override;
	void					MouseUp(int theX, int theY) override { Widget::MouseUp(theX, theY); }
	void					MouseUp(int theX, int theY, int theClickCount) override { Widget::MouseUp(theX, theY, theClickCount); }
	void					MouseUp(int theX, int theY, int theBtnNum, int theClickCount) override;
	void					Update() override;
};

}

#endif //__BUTTONWIDGET_H__
