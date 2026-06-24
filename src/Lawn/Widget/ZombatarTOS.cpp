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

#include "ZombatarTOS.h"
#include "ZombatarWidget.h"
#include "GameSelector.h"
#include "GameButton.h"
#include "../LawnCommon.h"
#include "../System/PlayerInfo.h"
#include "../../LawnApp.h"
#include "../../Resources.h"
#include "../../GameConstants.h"
#include "../../ConstEnums.h"
#include "../../Sexy.TodLib/TodStringFile.h"
#include "widget/Slider.h"
#include "widget/Checkbox.h"
#include "widget/WidgetManager.h"
#include "graphics/Font.h"
#include "graphics/Graphics.h"

#include <algorithm>

using namespace Sexy;

static const char* ZOMBATAR_TOS_DEFAULT_BODY =
	"Create custom zombie avatars for this profile. Saved Zombatars stay in the user file and can be edited "
	"from the main menu. Zombatar portraits are stored with this user profile and can be exported as PNG "
	"images when they are saved.";

static std::string ZombatarTOSBody()
{
	std::string aText = TodStringTranslate("[ZOMBATAR_TOS]");
	if (aText.rfind("<Missing", 0) == 0 || aText.empty())
		return ZOMBATAR_TOS_DEFAULT_BODY;
	return aText;
}

static std::string ZombatarTOSHeader()
{
	std::string aText = TodStringTranslate("[ZOMBATAR_TOS_HEADER]");
	if (aText.rfind("<Missing", 0) == 0 || aText.empty())
		return "Zombatar";
	return aText;
}

ZombatarTOS::ZombatarTOS(LawnApp* theApp) : LawnDialog(theApp, Dialogs::DIALOG_ZOMBATAR_TOS, true, ZombatarTOSHeader(), "", "", Dialog::BUTTONS_NONE)
{
	mApp = theApp;
	mTextHeight = 0;
	mClipHeight = 0;

	mTOSSlider = new Slider(IMAGE_ZOMBATAR_TOS_SLIDER, IMAGE_ZOMBATAR_TOS_SLIDER_THUMB, ZombatarTOS::ZombatarTOS_Slider, this);
	mTOSSlider->mHorizontal = false;
	mTOSSlider->SetValue(0);

	mBackButton = MakeNewButton(ZombatarTOS::ZombatarTOS_Back, this, "", nullptr,
		IMAGE_ZOMBATAR_BACK_BUTTON, IMAGE_ZOMBATAR_BACK_BUTTON_HIGHLIGHT, IMAGE_ZOMBATAR_BACK_BUTTON_HIGHLIGHT);

	mAcceptButton = MakeNewButton(ZombatarTOS::ZombatarTOS_Accept, this, "", nullptr,
		IMAGE_ZOMBATAR_ACCEPT_BUTTON, IMAGE_ZOMBATAR_ACCEPT_BUTTON_HIGHLIGHT, IMAGE_ZOMBATAR_ACCEPT_BUTTON_HIGHLIGHT);
	mAcceptButton->mDisabled = true;

	mTOSCheckbox = MakeNewCheckbox(ZombatarTOS::ZombatarTOS_TOSCheckbox, this, !theApp->mIsWindowed);

	CalcSize(110, 220);
}

ZombatarTOS::~ZombatarTOS()
{
	delete mTOSSlider;
	delete mBackButton;
	delete mAcceptButton;
	delete mTOSCheckbox;
}

void ZombatarTOS::AddedToManager(WidgetManager* theWidgetManager)
{
	Dialog::AddedToManager(theWidgetManager);
	AddWidget(mTOSSlider);
	AddWidget(mBackButton);
	AddWidget(mAcceptButton);
	AddWidget(mTOSCheckbox);
}

void ZombatarTOS::RemovedFromManager(WidgetManager* theWidgetManager)
{
	Dialog::RemovedFromManager(theWidgetManager);
	RemoveWidget(mTOSSlider);
	RemoveWidget(mBackButton);
	RemoveWidget(mAcceptButton);
	RemoveWidget(mTOSCheckbox);
}

void ZombatarTOS::Resize(int theX, int theY, int theWidth, int theHeight)
{
	LawnDialog::Resize(theX, theY, theWidth, theHeight);

	int aBtnWidth = IMAGE_ZOMBATAR_BACK_BUTTON ? IMAGE_ZOMBATAR_BACK_BUTTON->mWidth : 160;
	int aBtnHeight = IMAGE_ZOMBATAR_BACK_BUTTON ? IMAGE_ZOMBATAR_BACK_BUTTON->mHeight : 62;

	mTOSSlider->Resize(GetWidth() - 55, GetTop() + 8, 40, std::max(80, mHeight - 260));
	mBackButton->Resize(GetLeft(), mHeight - 95, aBtnWidth, aBtnHeight);
	mAcceptButton->Resize(GetWidth() - aBtnWidth - 10, mHeight - 95, aBtnWidth, aBtnHeight);
	mTOSCheckbox->Resize(GetWidth() - 110, mHeight - 100, 46, 45);
}

void ZombatarTOS::Draw(Graphics* g)
{
	LawnDialog::Draw(g);

	std::string aBody = ZombatarTOSBody();
	int aLeft = GetLeft() + 8;
	int aTop = GetTop() + 80;
	int aWidth = GetWidth() - 70;
	mClipHeight = std::max(40, mHeight - 270);

	if (mTextHeight <= 0)
		mTextHeight = TodDrawStringWrappedHelper(g, aBody, Rect(0, 0, aWidth, 0), FONT_PICO129, Color::White, DrawStringJustification::DS_ALIGN_LEFT, false);

	int aMaxScroll = std::max(0, mTextHeight - mClipHeight);
	int aOffset = static_cast<int>(mTOSSlider->mVal * aMaxScroll);

	g->PushState();
	g->ClipRect(Rect(GetLeft(), aTop, aWidth + 12, mClipHeight));
	TodDrawStringWrapped(g, aBody, Rect(aLeft, aTop - aOffset, aWidth, mTextHeight + 8), FONT_PICO129, Color(255, 255, 255), DrawStringJustification::DS_ALIGN_LEFT);
	g->PopState();

	// Hide the scrollbar entirely when the text fits without scrolling.
	mTOSSlider->SetVisible(aMaxScroll > 0);
}

void ZombatarTOS::Update()
{
	LawnDialog::Update();
}

void ZombatarTOS::ButtonPress(int theId)
{
	if (theId == ZombatarTOS::ZombatarTOS_Accept || theId == ZombatarTOS::ZombatarTOS_Back)
		mApp->PlaySample(SOUND_BUTTONCLICK);
}

void ZombatarTOS::ButtonDepress(int theId)
{
	switch (theId)
	{
	case ZombatarTOS::ZombatarTOS_Back:
		mApp->KillDialog(mId);
		break;
	case ZombatarTOS::ZombatarTOS_Accept:
		mApp->KillDialog(mId);
		if (mApp->mPlayerInfo)
		{
			mApp->mPlayerInfo->mZombatarAccepted = 1;
			mApp->mPlayerInfo->SaveDetails();
		}
		if (mApp->mGameSelector && mApp->mGameSelector->mZombatarWidget)
			mApp->mGameSelector->mZombatarWidget->Open();
		break;
	}
}

void ZombatarTOS::CheckboxChecked(int theId, bool checked)
{
	if (theId == ZombatarTOS::ZombatarTOS_TOSCheckbox)
		mAcceptButton->mDisabled = !checked;
}

void ZombatarTOS::SliderVal(int theId, double theVal)
{
	(void)theId;
	(void)theVal;
	MarkDirty();
}
