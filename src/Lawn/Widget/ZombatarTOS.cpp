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
#include "misc/KeyCodes.h"
#include "graphics/Font.h"
#include "graphics/Graphics.h"

#include <algorithm>

using namespace Sexy;

constexpr int TOS_DIALOG_W = 600;
constexpr int TOS_DIALOG_H = 450;
constexpr int TOS_SLIDER_X = 500;
constexpr int TOS_SLIDER_Y = 140;
constexpr int TOS_SLIDER_W = 29;
constexpr int TOS_SLIDER_H = 135;
constexpr int TOS_BACK_X = 40;
constexpr int TOS_ACCEPT_X = 450;
constexpr int TOS_BUTTON_Y = 344;
constexpr int TOS_CHECK_X = 400;
constexpr int TOS_CHECK_Y = 340;
constexpr int TOS_TEXT_X = 50;
constexpr int TOS_TEXT_Y = 130;
constexpr int TOS_TEXT_W = 435;
constexpr int TOS_CLIP_H = 160;
constexpr int TOS_ARROW_X = 420;
constexpr int TOS_ARROW_Y = 290;
constexpr int TOS_CHECK_W = 45;
constexpr int TOS_CHECK_H = 45;

ZombatarTOS::ZombatarTOS(LawnApp* theApp) : LawnDialog(theApp, Dialogs::DIALOG_ZOMBATAR_TOS, true, "[ZOMBATAR_TOS_HEADER]", "", "", Dialog::BUTTONS_NONE)
{
	mTextHeight = 0;
	mFlashArrow = false;
	mArrowAlpha = 0;
	mArrowDir = 3;

	mTOSSlider = new Slider(IMAGE_ZOMBATAR_TOS_SLIDER, IMAGE_ZOMBATAR_TOS_SLIDER_THUMB, ZombatarTOS::ZombatarTOS_Slider, this);
	mTOSSlider->mHorizontal = false;
	mTOSSlider->SetValue(0);

	mBackButton = MakeNewButton(ZombatarTOS::ZombatarTOS_Back, this, "", nullptr,
		IMAGE_ZOMBATAR_BACK_BUTTON, IMAGE_ZOMBATAR_BACK_BUTTON_HIGHLIGHT, nullptr);

	mAcceptButton = MakeNewButton(ZombatarTOS::ZombatarTOS_Accept, this, "", nullptr,
		IMAGE_ZOMBATAR_ACCEPT_BUTTON, IMAGE_ZOMBATAR_ACCEPT_BUTTON_HIGHLIGHT, nullptr);

	mTOSCheckbox = MakeNewCheckbox(ZombatarTOS::ZombatarTOS_TOSCheckbox, this, false);

	Resize(0, 0, TOS_DIALOG_W, TOS_DIALOG_H);
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

	int aBackWidth = IMAGE_ZOMBATAR_BACK_BUTTON ? IMAGE_ZOMBATAR_BACK_BUTTON->mWidth : 98;
	int aBackHeight = IMAGE_ZOMBATAR_BACK_BUTTON ? IMAGE_ZOMBATAR_BACK_BUTTON->mHeight : 26;
	int aAcceptWidth = IMAGE_ZOMBATAR_ACCEPT_BUTTON ? IMAGE_ZOMBATAR_ACCEPT_BUTTON->mWidth : aBackWidth;
	int aAcceptHeight = IMAGE_ZOMBATAR_ACCEPT_BUTTON ? IMAGE_ZOMBATAR_ACCEPT_BUTTON->mHeight : aBackHeight;

	mTOSSlider->Resize(TOS_SLIDER_X, TOS_SLIDER_Y, TOS_SLIDER_W, TOS_SLIDER_H);
	mBackButton->Resize(TOS_BACK_X, TOS_BUTTON_Y, aBackWidth, aBackHeight);
	mAcceptButton->Resize(TOS_ACCEPT_X, TOS_BUTTON_Y, aAcceptWidth, aAcceptHeight);
	mTOSCheckbox->Resize(TOS_CHECK_X, TOS_CHECK_Y, TOS_CHECK_W, TOS_CHECK_H);
}

void ZombatarTOS::Draw(Graphics* g)
{
	LawnDialog::Draw(g);

	if (mBody.empty())
		mBody = TodStringTranslate("[ZOMBATAR_TOS]");
	if (mTextHeight <= 0)
		mTextHeight = TodDrawStringWrappedHelper(g, mBody, Rect(0, 0, TOS_TEXT_W, 0), FONT_PICO129, Color::White, DrawStringJustification::DS_ALIGN_LEFT, false);

	int aMaxScroll = std::max(0, mTextHeight - TOS_TEXT_Y);
	int aOffset = static_cast<int>(mTOSSlider->mVal * aMaxScroll);

	g->PushState();
	g->ClipRect(Rect(TOS_TEXT_X, TOS_TEXT_Y, TOS_TEXT_W, TOS_CLIP_H));
	TodDrawStringWrapped(g, mBody, Rect(TOS_TEXT_X, TOS_TEXT_Y - aOffset, TOS_TEXT_W, mTextHeight), FONT_PICO129, Color::White, DrawStringJustification::DS_ALIGN_LEFT);
	g->PopState();

	if (mFlashArrow && IMAGE_ZOMBATAR_TOS_ARROW)
	{
		g->SetColorizeImages(true);
		g->SetColor(Color(255, 255, 255, mArrowAlpha));
		g->DrawImage(IMAGE_ZOMBATAR_TOS_ARROW, TOS_ARROW_X, TOS_ARROW_Y);
		g->SetColorizeImages(false);
		g->SetColor(Color::White);
	}
}

void ZombatarTOS::Update()
{
	LawnDialog::Update();
	if (mFlashArrow)
	{
		mArrowAlpha += mArrowDir;
		if (mArrowAlpha >= 255)
		{
			mArrowAlpha = 255;
			mArrowDir = -3;
		}
		else if (mArrowAlpha <= 0)
		{
			mArrowAlpha = 0;
			mArrowDir = 3;
		}
		MarkDirty();
	}
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
		if (!mTOSCheckbox->mChecked)
		{
			mFlashArrow = true;
			mArrowAlpha = 0;
			mArrowDir = 3;
			return;
		}
		if (mApp->mPlayerInfo)
		{
			mApp->mPlayerInfo->mZombatarAccepted = 1;
			mApp->mPlayerInfo->SaveDetails();
		}
		mApp->KillDialog(mId);
		if (mApp->mGameSelector && mApp->mGameSelector->mZombatarWidget)
			mApp->mGameSelector->mZombatarWidget->Open();
		break;
	}
}

void ZombatarTOS::KeyDown(KeyCode theKey)
{
	if (theKey == KeyCode::KEYCODE_ESCAPE)
	{
		ButtonDepress(ZombatarTOS::ZombatarTOS_Back);
		return;
	}

	LawnDialog::KeyDown(theKey);
}

void ZombatarTOS::MouseWheel(int theDelta)
{
	if (mTextHeight <= TOS_CLIP_H)
		return;

	int aMaxScroll = std::max(0, mTextHeight - TOS_CLIP_H);
	int aOffset = static_cast<int>(mTOSSlider->mVal * aMaxScroll);
	aOffset -= theDelta * 12;
	mTOSSlider->SetValue(std::max(0.0, std::min(1.0, static_cast<double>(aOffset) / aMaxScroll)));
}

void ZombatarTOS::CheckboxChecked(int theId, bool checked)
{
	if (theId == ZombatarTOS::ZombatarTOS_TOSCheckbox && checked)
	{
		mFlashArrow = false;
		mArrowAlpha = 0;
	}
}

void ZombatarTOS::SliderVal(int theId, double theVal)
{
	(void)theId;
	(void)theVal;
	MarkDirty();
}
