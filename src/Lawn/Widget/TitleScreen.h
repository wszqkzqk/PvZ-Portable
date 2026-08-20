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

#ifndef __TITLESCREEN_H__
#define __TITLESCREEN_H__

#include <atomic>
#include <memory>

#include "widget/Widget.h"
#include "widget/ButtonListener.h"

using namespace Sexy;

enum TitleState
{
	TITLESTATE_WAITING_FOR_FIRST_DRAW,
	TITLESTATE_POPCAP_LOGO,
	TITLESTATE_PARTNER_LOGO,
	TITLESTATE_SCREEN
};

namespace Sexy
{
	class HyperlinkWidget;
}

class LawnApp;

class TitleScreen :public Sexy::Widget, public Sexy::ButtonListener
{
public:
	enum
	{
		TitleScreen_Start,
		TitleScreen_Register
	};

public:
	std::unique_ptr<HyperlinkWidget>		mStartButton;
	float					mCurBarWidth;
	float					mTotalBarWidth;
	float					mBarVel;
	float					mBarStartProgress;
	bool					mRegisterClicked;
	bool					mLoadingThreadComplete;
	int						mTitleAge;
	KeyCode					mQuickLoadKey;
	bool					mNeedRegister;
	bool					mNeedShowRegisterBox;
	bool					mDrawnYet;
	bool					mNeedToInit;
	float					mPrevLoadingPercent;
	TitleState				mTitleState;
	int						mTitleStateCounter;
	int						mTitleStateDuration;
	bool					mDisplayPartnerLogo;
	std::atomic<bool>		mLoaderScreenIsLoaded;
	LawnApp*				mApp;

public:
	TitleScreen(LawnApp* theApp);
	~TitleScreen() override;

	void					Update() override;
	void					Draw(Graphics* g) override;
	void					Resize(int theX, int theY, int theWidth, int theHeight) override;
	void					AddedToManager(WidgetManager* theWidgetManager) override;
	void					RemovedFromManager(WidgetManager* theWidgetManager) override;
	void					ButtonPress(int theId) override;
	void					ButtonDepress(int theId) override;
	void					ButtonDownTick(int) override{}
	void					ButtonMouseEnter(int) override{}
	void					ButtonMouseLeave(int) override{}
	void					ButtonMouseMove(int, int, int) override{}
	void					MouseDown(int x, int y, int theClickCount) override;
	void					KeyDown(KeyCode theKey) override;
	void					SetRegistered();
	void					DrawToPreload(Graphics* g);
};

#endif
