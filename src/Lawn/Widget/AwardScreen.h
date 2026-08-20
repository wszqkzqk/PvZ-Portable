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

#ifndef __AWARDSCREEN_H__
#define __AWARDSCREEN_H__

#include "../../ConstEnums.h"
#include "widget/Widget.h"
#include <memory>
using namespace Sexy;

class LawnApp;
class GameButton;

class AchievementScreenItem {
public:
	int mId; //+0x00
	int mStartAnimTime; //+0x04
	int mEndAnimTime; //+0x08
	int mDestY; //+0x0C
	int mStartY; //+0x10
	int mY; //+0x14
};

class AwardScreen : public Widget
{
private:
	enum
	{
		AwardScreen_Start = 100,
		AwardScreen_Menu = 101
	};

public:
	std::unique_ptr<GameButton>		mStartButton;
	std::unique_ptr<GameButton>		mMenuButton;
	LawnApp*							mApp;
	int									mFadeInCounter;
	AwardType							mAwardType;				//+0xB8
	std::unique_ptr<GameButton>		mContinueButton;		//+0xA8
	bool								mShowStartButtonAfterAchievements;	//+0xAC
	bool								mShowMenuButtonAfterAchievements;	//+0xAD
	int									mAchievementAnimTime;	//+0xBC
	bool								mShowingAchievements;	//+0xD8
	std::vector<AchievementScreenItem>	mAchievementItems;		//+0xC0

public:
	AwardScreen(LawnApp* theApp, AwardType theAwardType, bool theShowingAchievements = false);
	~AwardScreen() override;

	bool		IsPaperNote();
	void				Resize(int theX, int theY, int theWidth, int theHeight) override { Widget::Resize(theX, theY, theWidth, theHeight); }
	static void			DrawBottom(Graphics* g, std::string_view theTitle, std::string_view theAward, std::string_view theMessage);
	void				DrawAwardSeed(Graphics* g);
	void				Draw(Graphics* g) override;
	void				Update() override;
	void				AddedToManager(WidgetManager* theWidgetManager) override { Widget::AddedToManager(theWidgetManager); }
	void				RemovedFromManager(WidgetManager* theWidgetManager) override { Widget::RemovedFromManager(theWidgetManager); }
	void				KeyDown(KeyCode theKey) override;
	void				StartButtonPressed();
	void				MouseDown(int x, int y, int theClickCount) override;
	void				MouseUp(int x, int y, int theClickCount) override;
	void				DrawAchievements(Graphics* g);
	void				AchievementsContinuePressed();
};

#endif
