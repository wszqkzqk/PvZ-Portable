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
using namespace Sexy;

class LawnApp;
class GameButton;

class AchievementScreenItem {
public:
	int mId; //+GOTY @Patoke: 0x00
	int mStartAnimTime; //+GOTY @Patoke: 0x04
	int mEndAnimTime; //+GOTY @Patoke: 0x08
	int mDestY; //+GOTY @Patoke: 0x0C
	int mStartY; //+GOTY @Patoke: 0x10
	int mY; //+GOTY @Patoke: 0x14
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
	GameButton*							mStartButton;
	GameButton*							mMenuButton;
	LawnApp*							mApp;
	int									mFadeInCounter;
	AwardType							mAwardType;				//+GOTY @Patoke: 0xB8
	GameButton*							mContinueButton;		//+GOTY @Patoke: 0xA8
	bool								mShowStartButtonAfterAchievements;	//+GOTY @Patoke: 0xAC
	bool								mShowMenuButtonAfterAchievements;	//+GOTY @Patoke: 0xAD
	int									mAchievementAnimTime;	//+GOTY @Patoke: 0xBC
	bool								mShowingAchievements;	//+GOTY @Patoke: 0xD8
	std::vector<AchievementScreenItem>	mAchievementItems;		//+GOTY @Patoke: 0xC0

public:
	// @Patoke: added argument
	AwardScreen(LawnApp* theApp, AwardType theAwardType, bool theShowingAchievements = false);
	~AwardScreen() override;

	/*inline*/ bool		IsPaperNote();
	void				Resize(int theX, int theY, int theWidth, int theHeight) override { Widget::Resize(theX, theY, theWidth, theHeight); }
	static void			DrawBottom(Graphics* g, const std::string& theTitle, const std::string& theAward, const std::string& theMessage);
	void				DrawAwardSeed(Graphics* g);
	void				Draw(Graphics* g) override;
	void				Update() override;
	void				AddedToManager(WidgetManager* theWidgetManager) override { Widget::AddedToManager(theWidgetManager); }
	void				RemovedFromManager(WidgetManager* theWidgetManager) override { Widget::RemovedFromManager(theWidgetManager); }
	void				KeyDown(KeyCode theKey) override;
	void				StartButtonPressed();
	void				MouseDown(int x, int y, int theClickCount) override;
	void				MouseUp(int x, int y, int theClickCount) override;
	// @Patoke: implement functions
	void				DrawAchievements(Graphics* g);
	void				AchievementsContinuePressed();
};

#endif
