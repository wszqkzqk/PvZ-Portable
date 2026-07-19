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

#include <time.h>
#include "Board.h"
#include "Plant.h"
#include "../LawnApp.h"
#include "LawnCommon.h"
#include "../Resources.h"
#include "../GameConstants.h"
#include "../Sexy.TodLib/TodCommon.h"
#include "graphics/Font.h"
#include "widget/Dialog.h"
#include "misc/SexyMatrix.h"
#include "widget/Checkbox.h"

int gLawnEditWidgetColors[][4] = {
    { 0,   0,   0,   0 },
    { 0,   0,   0,   0 },
    { 240, 240, 255, 255 },
    { 255, 255, 255, 255 },
    { 0,   0,   0,   255 },
};

// 判断在 [theNumber - theRange, theNumber + theRange] 区间内是否存在 theMod 的整数倍数
bool ModInRange(int theNumber, int theMod, int theRange)
{
	theRange = abs(theRange);
	for (int i = theNumber - theRange; i <= theNumber + theRange; i++)
		if (i % theMod == 0) return true;
	return false;
}

// 判断点 (x1, y1) 是否位于点 (x2, y2) 周围的 (theRangeX, theRangeY) 范围内
bool GridInRange(int x1, int y1, int x2, int y2, int theRangeX, int theRangeY)
{
	return x1 >= x2 - theRangeX && x1 <= x2 + theRangeX && y1 >= y2 - theRangeY && y1 <= y2 + theRangeY;
}

void TileImageHorizontally(Graphics* g, Image* theImage, int theX, int theY, int theWidth)
{
	while (theWidth > 0)
	{
		int aImageWidth = std::min(theWidth, theImage->GetWidth());
		g->DrawImage(theImage, theX, theY, Rect(0, 0, aImageWidth, theImage->GetHeight()));
		theX += aImageWidth;
		theWidth -= aImageWidth;
	}
}

void TileImageVertically(Graphics* g, Image* theImage, int theX, int theY, int theHeight)
{
	while (theHeight > 0)
	{
		int aImageHeight = std::min(theHeight, theImage->GetHeight());
		g->DrawImage(theImage, theX, theY, Rect(0, 0, theImage->GetWidth(), aImageHeight));
		theY += aImageHeight;
		theHeight -= aImageHeight;
	}
}

LawnEditWidget::LawnEditWidget(int theId, EditListener* theListener, Dialog* theDialog) : EditWidget(theId, theListener)
{
    mDialog = theDialog;
    mAutoCapFirstLetter = true;
}

LawnEditWidget::~LawnEditWidget()
{
}

void LawnEditWidget::KeyDown(KeyCode theKey)
{
    EditWidget::KeyDown(theKey);
    if (theKey == KeyCode::KEYCODE_ESCAPE)
        mDialog->KeyDown(KeyCode::KEYCODE_ESCAPE);
}

void LawnEditWidget::KeyChar(char theChar)
{
    if (mAutoCapFirstLetter && isalpha(theChar))
    {
        theChar = toupper(theChar);
        mAutoCapFirstLetter = false;
    }

    EditWidget::KeyChar(theChar);
}

LawnEditWidget* CreateEditWidget(int theId, EditListener* theListener, Dialog* theDialog)
{
    LawnEditWidget* aEditWidget = new LawnEditWidget(theId, theListener, theDialog);
    aEditWidget->SetFont(Sexy::FONT_BRIANNETOD16);
    aEditWidget->SetColors(gLawnEditWidgetColors, EditWidget::NUM_COLORS);
    aEditWidget->mBlinkDelay = 14;

    return aEditWidget;
}

void DrawEditBox(Graphics* g, EditWidget* theWidget)
{
    Rect aDest(theWidget->mX - 8, theWidget->mY - 4, theWidget->mWidth + 16, theWidget->mHeight + 8);
    g->DrawImageBox(aDest, IMAGE_EDITBOX);
}

Checkbox* MakeNewCheckbox(int theId, CheckboxListener* theListener, bool theDefault)
{
    Checkbox* aCheckbox = new Checkbox(Sexy::IMAGE_OPTIONS_CHECKBOX0, Sexy::IMAGE_OPTIONS_CHECKBOX1, theId, theListener);
    aCheckbox->mChecked = theDefault;
    aCheckbox->mHasAlpha = true;
    aCheckbox->mHasTransparencies = true;

    return aCheckbox;
}

std::string GetSavedGameName(GameMode theGameMode, int theProfileId)
{
    return GetAppDataPath(StrFormat("userdata/game%d_%d.v4", theProfileId, static_cast<int>(theGameMode)));
}

// GOTY @Patoke: 0x45A200
std::string GetLegacySavedGameName(GameMode theGameMode, int theProfileId)
{
    return GetAppDataPath(StrFormat("userdata/game%d_%d.dat", theProfileId, static_cast<int>(theGameMode)));
}

int GetCurrentDaysSince2000(time_t theTime)
{
    tm aNowTM = *localtime(&theTime);

    int dy = aNowTM.tm_year - 100;
    return dy * 365 + (dy - 1) / 400 - (dy - 1) / 100 + (dy - 1) / 4 + aNowTM.tm_yday + 1;
}
