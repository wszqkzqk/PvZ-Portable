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

#ifndef __LAWN_COMMON_H__
#define __LAWN_COMMON_H__

#include "../ConstEnums.h"
#include "graphics/Graphics.h"
#include "widget//EditWidget.h"
#include <time.h>

using namespace Sexy;
// using namespace std;

class Board;
class LawnStoneButton;
class NewLawnButton;
namespace Sexy
{
	class Dialog;
	class Checkbox;
	class DialogButton;
	class CheckboxListener;
}

class LawnEditWidget : public EditWidget
{
public:
	Dialog*					mDialog;
	bool					mAutoCapFirstLetter;

public:
	LawnEditWidget(int theId, EditListener* theListener, Dialog* theDialog);
	~LawnEditWidget() override;

	void					KeyDown(KeyCode theKey) override;
	void					KeyChar(char theChar) override;
};

// ====================================================================================================
// ★ 常用逻辑判断
// ====================================================================================================
/*inline*/ bool				ModInRange(int theNumber, int theMod, int theRange = 0);
/*inline*/ bool				GridInRange(int x1, int y1, int x2, int y2, int theRangeX = 1, int theRangeY = 1);

// ====================================================================================================
// ★ 动画、特效与绘制相关
// ====================================================================================================
/*inline*/ void				TileImageHorizontally(Graphics* g, Image* theImage, int theX, int theY, int theWidth);
/*inline*/ void				TileImageVertically(Graphics* g, Image* theImage, int theX, int theY, int theHeight);

// ====================================================================================================
// ★ 控件
// ====================================================================================================
Checkbox*					MakeNewCheckbox(int theId, CheckboxListener* theListener, bool theDefault);
LawnEditWidget*				CreateEditWidget(int theId, EditListener* theListener, Dialog* theDialog);
void						DrawEditBox(Graphics* g, EditWidget* theWidget);

// ====================================================================================================
// ★ 其他
// ====================================================================================================
std::string					GetSavedGameName(GameMode theGameMode, int theProfileId);
std::string					GetLegacySavedGameName(GameMode theGameMode, int theProfileId);
int							GetCurrentDaysSince2000(time_t theTime);

#endif
