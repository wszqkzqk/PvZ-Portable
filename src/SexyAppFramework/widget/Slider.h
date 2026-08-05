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

#ifndef __SLIDER_H__
#define __SLIDER_H__

#include "Widget.h"

namespace Sexy
{

class SliderListener;

class Slider : public Widget
{
public:
	SliderListener*			mListener;
	double					mVal;
	int						mId;
	Image*					mTrackImage;
	Image*					mThumbImage;

	bool					mDragging;
	int						mRelX;
	int						mRelY;

	bool					mHorizontal;

public:
	Slider(Image* theTrackImage, Image* theThumbImage, int theId, SliderListener* theListener);

	virtual void			SetValue(double theValue);

	virtual bool			HasTransparencies();
	void					Draw(Graphics* g) override;

	void					MouseMove(int x, int y) override;
	void					MouseDown(int x, int y, int theClickCount) override;
	void					MouseDrag(int x, int y) override;
	void					MouseUp(int x, int y) override;
	void					MouseLeave() override;
};

}

#endif //__SLIDER_H__
