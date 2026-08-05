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

#ifndef __CHECKBOX_H__
#define __CHECKBOX_H__

#include "Widget.h"

namespace Sexy
{

class CheckboxListener;
class Image;

class Checkbox : public Widget
{
protected:
	CheckboxListener*		mListener;

public:
	int						mId;

	bool					mChecked;

	Image*					mUncheckedImage;
	Image*					mCheckedImage;

	Rect					mCheckedRect;
	Rect					mUncheckedRect;

	Color					mOutlineColor;	// These are only used if no image is specified
	Color					mBkgColor;
	Color					mCheckColor;

public:
	virtual void			SetChecked(bool checked, bool tellListener = true);
	virtual bool			IsChecked();

	void					MouseDown(int x, int y, int theClickCount) override { Widget::MouseDown(x, y, theClickCount); }
	void					MouseDown(int x, int y, int theBtnNum, int theClickCount) override;
	void					Draw(Graphics* g) override;

public:
	Checkbox(Image* theUncheckedImage, Image* theCheckedImage, int theId, CheckboxListener* theCheckboxListener);
};

}

#endif //__CHECKBOX_H__
