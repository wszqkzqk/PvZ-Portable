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

#ifndef __GAMEBUTTON_H__
#define __GAMEBUTTON_H__

#include "misc/SexyVector.h"
#include "widget/DialogButton.h"

//using namespace std;
using namespace Sexy;

class LawnApp;
class GameButton
{
public:
	enum
	{
		BUTTON_LABEL_LEFT = -1,
		BUTTON_LABEL_CENTER = 0,
		BUTTON_LABEL_RIGHT = 1
	};
	enum
	{
		COLOR_LABEL = 0,
		COLOR_LABEL_HILITE = 1,
		COLOR_DARK_OUTLINE = 2,
		COLOR_LIGHT_OUTLINE = 3,
		COLOR_MEDIUM_OUTLINE = 4,
		COLOR_BKG = 5,
		NUM_COLORS = 6
	};

public:
	LawnApp*				mApp;
	Widget*					mParentWidget;
	int						mX;
	int						mY;
	int						mWidth;
	int						mHeight;
	bool					mIsOver;
	bool					mIsDown;
	bool					mDisabled;
	Color					mColors[6];
	int						mId;
	std::string				mLabel;
	int						mLabelJustify;
	_Font*					mFont;
	Image*					mButtonImage;
	Image*					mOverImage;
	Image*					mDownImage;
	Image*					mDisabledImage;
	Image*					mOverOverlayImage;
	Rect					mNormalRect;
	Rect					mOverRect;
	Rect					mDownRect;
	Rect					mDisabledRect;
	bool					mInverted;
	bool					mBtnNoDraw;
	bool					mFrameNoDraw;
	double					mOverAlpha;
	double					mOverAlphaSpeed;
	double					mOverAlphaFadeInSpeed;
	bool					mDrawStoneButton;
	int						mTextOffsetX;
	int						mTextOffsetY;
	int						mButtonOffsetX;
	int						mButtonOffsetY;

public:
	GameButton(int theId);
	~GameButton();

	static /*inline*/ bool	HaveButtonImage(Image* theImage, Rect& theRect);
	void					DrawButtonImage(Graphics* g, Image* theImage, Rect& theRect, int theX, int theY);
	/*inline*/ void			SetFont(_Font* theFont);
	/*inline*/ bool			IsButtonDown();
	void					Draw(Graphics* g);
	/*inline*/ void			SetDisabled(bool theDisabled);
	/*inline*/ bool			IsMouseOver();
	void					Update();
	/*inline*/ void			Resize(int theX, int theY, int theWidth, int theHeight);
	/*inline*/ void			SetLabel(std::string_view theLabel);
};

class LawnStoneButton : public DialogButton
{
public:
	LawnStoneButton(Image* theComponentImage, int theId, ButtonListener* theListener) : DialogButton(theComponentImage, theId, theListener) { }

	void					Draw(Graphics* g) override;
	/*inline*/ void			SetLabel(std::string_view theLabel);
};

class NewLawnButton : public DialogButton
{
public:
    _Font*					mHiliteFont;
    int						mTextDownOffsetX;
    int						mTextDownOffsetY;
	int						mButtonOffsetX;		// static layout offset, set once at creation
	int						mButtonOffsetY;
	bool					mUsePolygonShape;
	SexyVector2				mPolygonShape[4];

public:
    NewLawnButton(Image* theComponentImage, int theId, ButtonListener* theListener);
	~NewLawnButton() override;
	
    void					Draw(Graphics* g) override;
	bool					IsPointVisible(int x, int y) override;
    void					SetLabel(std::string_view theLabel);
};

LawnStoneButton*			MakeButton(int theId, ButtonListener* theListener, std::string_view theText);
NewLawnButton*				MakeNewButton(int theId, ButtonListener* theListener, std::string_view theText, _Font* theFont, Image* theImageNormal, Image* theImageOver, Image* theImageDown);
void						DrawStoneButton(Graphics* g, int x, int y, int theWidth, int theHeight, bool isDown, bool isHighLighted, const std::string& theLabel);

#endif
