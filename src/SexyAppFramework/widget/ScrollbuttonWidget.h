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

#ifndef __SCROLLBUTTONWIDGET_H__
#define __SCROLLBUTTONWIDGET_H__

#include "widget/ButtonWidget.h"

namespace Sexy
{

class ButtonListener;

class ScrollbuttonWidget : public ButtonWidget
{
public:
	bool				mHorizontal;

	// mType is a new way of doing things (1 = up, 2 = down, 3 = left, 4 = right).
	// This overrides mHorizontal and mId.
	int					mType;

public:
	ScrollbuttonWidget(int theId, ButtonListener *theButtonListener, int theType = 0);
	~ScrollbuttonWidget() override;

	void				Draw(Graphics *g) override;
};

}

#endif //__SCROLLBUTTONWIDGET_H__
