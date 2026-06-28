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

#ifndef __WIDGET_H__
#define __WIDGET_H__

#include "Common.h"
#include "graphics/Color.h"
#include "Insets.h"
#include "graphics/Graphics.h"
#include "misc/KeyCodes.h"
#include "WidgetContainer.h"

namespace Sexy
{

class WidgetManager;

class Widget : public WidgetContainer
{
public:			
	bool					mVisible;
	bool					mMouseVisible;		
	bool					mDisabled;
	bool					mHasFocus;	
	bool					mIsDown;
	bool					mIsOver;
	bool					mHasTransparencies;	
	std::vector<Color>		mColors;
	Insets					mMouseInsets;
	bool					mDoFinger;
	bool					mWantsFocus;

	Widget*					mTabPrev;
	Widget*					mTabNext;	

	static bool				mWriteColoredString;  // controls whether ^color^ works in calls to WriteString	

	std::vector<std::string> mLoadedResourceNames; // headshot2017: for unloading resources on destructor

	void					WidgetRemovedHelper();

public:
	Widget();
	~Widget() override;
		
	virtual void			OrderInManagerChanged();
	virtual void			SetVisible(bool isVisible);
	
	virtual void			SetColors(int theColors[][3], int theNumColors);
	virtual void			SetColors(int theColors[][4], int theNumColors);
	virtual void			SetColor(int theIdx, const Color& theColor);
	virtual const Color&	GetColor(int theIdx);
	virtual Color			GetColor(int theIdx, const Color& theDefaultColor);	
		
	virtual void			SetDisabled(bool isDisabled);
	virtual void			ShowFinger(bool on);
	
	virtual void			Resize(int theX, int theY, int theWidth, int theHeight);
	virtual void			Resize(const Rect& theRect);
	virtual void			Move(int theNewX, int theNewY);	
	virtual bool			WantsFocus();
	void					Draw(Graphics* g) override; // Already translated
	virtual void			DrawOverlay(Graphics* g);
	virtual void			DrawOverlay(Graphics* g, int thePriority);
	void					Update() override;
//	virtual void			UpdateF(float theFrac);
	virtual void			GotFocus();
	virtual void			LostFocus();	
	virtual void			KeyChar(char theChar);
	virtual void			KeyText(std::string_view theText);
	virtual void			KeyDown(KeyCode theKey);
	virtual void			KeyUp(KeyCode theKey);	
	virtual void			MouseEnter();
	virtual void			MouseLeave();
	virtual void			MouseMove(int x, int y);
	virtual void			MouseDown(int x, int y, int theClickCount);
	virtual void			MouseDown(int x, int y, int theBtnNum, int theClickCount);
	virtual void			MouseUp(int x, int y);
	virtual void			MouseUp(int x, int y, int theClickCount);
	virtual void			MouseUp(int x, int y, int theBtnNum, int theClickCount);
	virtual void			MouseDrag(int x, int y);
	virtual void			MouseWheel(int theDelta);
	virtual bool			IsPointVisible(int x, int y);
	
	//////// Helper functions
	
	virtual Rect			WriteCenteredLine(Graphics* g, int anOffset, std::string_view theLine);
	virtual Rect			WriteCenteredLine(Graphics* g, int anOffset, std::string_view theLine, Color theColor1, Color theColor2, const Point& theShadowOffset = Point(1,2));

	virtual int				WriteString(Graphics* g, std::string_view theString, int theX, int theY, int theWidth = -1, int theJustification = -1, bool drawString = true, int theOffset = 0, int theLength = -1);
	virtual int				WriteWordWrapped(Graphics* g, const Rect& theRect, std::string_view theLine, int theLineSpacing, int theJustification);
	virtual int				GetWordWrappedHeight(Graphics* g, int theWidth, std::string_view theLine, int aLineSpacing);
	virtual int				GetNumDigits(int theNumber);
	virtual void			WriteNumberFromStrip(Graphics* g, int theNumber, int theX, int theY, Image* theNumberStrip, int aSpacing);
	virtual bool			Contains(int theX, int theY);
	virtual Rect			GetInsetRect();	
	void					DeferOverlay(int thePriority = 0);	

	//////// Layout functions
	int						Left()							{ return mX; } 
	int						Top()							{ return mY; }
	int						Right()							{ return mX + mWidth; }
	int						Bottom()						{ return mY + mHeight; }
	int						Width()							{ return mWidth; }
	int						Height()						{ return mHeight; }

	void					Layout(int theLayoutFlags, Widget *theRelativeWidget, int theLeftPad = 0, int theTopPad = 0, int theWidthPad = 0, int theHeightPad = 0);
};

/////// Layout flags used in Widget::Layout method
enum LayoutFlags
{
	LAY_SameWidth		=		0x0001,
	LAY_SameHeight		=		0x0002,

	LAY_SetLeft			=		0x0010,
	LAY_SetTop			=		0x0020,
	LAY_SetWidth		=		0x0040,
	LAY_SetHeight		=		0x0080,

	LAY_Above			=		0x0100,
	LAY_Below			=		0x0200,
	LAY_Right			=		0x0400,
	LAY_Left			=		0x0800,

	LAY_SameLeft		=		0x1000,
	LAY_SameRight		=		0x2000,
	LAY_SameTop			=		0x4000,
	LAY_SameBottom		=		0x8000,

	LAY_GrowToRight		=		0x10000,
	LAY_GrowToLeft		=		0x20000,
	LAY_GrowToTop		=		0x40000,
	LAY_GrowToBottom	=		0x80000,

	LAY_HCenter			=		0x100000,
	LAY_VCenter			=		0x200000,
	LAY_Max				=		0x400000,

	LAY_SameSize		=		LAY_SameWidth | LAY_SameHeight,
	LAY_SameCorner		=		LAY_SameLeft  | LAY_SameTop,
	LAY_SetPos			=		LAY_SetLeft   | LAY_SetTop,
	LAY_SetSize			=		LAY_SetWidth  | LAY_SetHeight
};


}

#endif //__WIDGET_H__
