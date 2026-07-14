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

#ifndef __WIDGETCONTAINER_H__
#define __WIDGETCONTAINER_H__

#include "Common.h"
#include "misc/Rect.h"
#include "misc/Flags.h"

namespace Sexy
{

class Graphics;
class Widget;
class WidgetManager;

typedef std::list<Widget*> WidgetList;


class WidgetContainer
{
public:
	WidgetList				mWidgets;
	WidgetManager*			mWidgetManager;
	WidgetContainer*		mParent;

	bool					mUpdateIteratorModified;
	WidgetList::iterator	mUpdateIterator;
	uint					mLastWMUpdateCount;
	uint					mUpdateCnt;
	bool					mDirty;
	int						mX;
	int						mY;
	int						mWidth;
	int						mHeight;
	bool					mHasAlpha;
	bool					mClip;
	FlagsMod				mWidgetFlagsMod;
	int						mPriority;
	int						mZOrder;

public:	
	Widget*					GetWidgetAtHelper(int x, int y, int theFlags, bool* found, int* theWidgetX, int* theWidgetY);
	bool					IsBelowHelper(Widget* theWidget1, Widget* theWidget2, bool* found);
	void					InsertWidgetHelper(const WidgetList::iterator &where, Widget *theWidget);

public:	
	WidgetContainer();
	virtual ~WidgetContainer();

	virtual Rect			GetRect();
	virtual bool			Intersects(WidgetContainer* theWidget);	

	virtual void			AddWidget(Widget* theWidget);
	virtual void			RemoveWidget(Widget* theWidget);	
	virtual bool			HasWidget(Widget* theWidget);	
	virtual void			DisableWidget(Widget* theWidget);
	virtual void			RemoveAllWidgets(bool doDelete = false, bool recursive = false);
	
	virtual void			SetFocus(Widget* theWidget);
	virtual bool			IsBelow(Widget* theWidget1, Widget* theWidget2);			
	virtual void			MarkAllDirty();
	virtual void			BringToFront(Widget* theWidget);
	virtual void			BringToBack(Widget* theWidget);
	virtual void			PutBehind(Widget* theWidget, Widget* theRefWidget);
	virtual void			PutInfront(Widget* theWidget, Widget* theRefWidget);
	virtual Point			GetAbsPos(); // relative to top level

	virtual void			MarkDirty();
	virtual void			MarkDirtyFull();
	virtual void			MarkDirtyFull(WidgetContainer* theWidget);
	virtual void			MarkDirty(WidgetContainer* theWidget);

	virtual void			AddedToManager(WidgetManager* theWidgetManager);
	virtual void			RemovedFromManager(WidgetManager* theWidgetManager);			

	virtual void			Update();
	virtual void			UpdateAll(ModalFlags* theFlags);
//	virtual void			UpdateF(float theFrac);
	virtual void			UpdateFAll(ModalFlags* theFlags, float theFrac);
	virtual void			Draw(Graphics* g);
	virtual void			DrawAll(ModalFlags* theFlags, Graphics* g);	
	virtual void			SysColorChangedAll();
	virtual void			SysColorChanged();
};

};

#endif //__WIDGETCONTAINER_H__
