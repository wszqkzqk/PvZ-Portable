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

#ifndef __LAWNDIALOG_H__
#define __LAWNDIALOG_H__

#include "../../Sexy.TodLib/TodCommon.h"
#include "widget/Dialog.h"

constexpr const int DIALOG_HEADER_OFFSET = 45;

class LawnApp;
class LawnDialog;
class Reanimation;
class LawnStoneButton;
class ReanimationWidget;
namespace Sexy
{
	class Checkbox;
	class DialogButton;
	class CheckboxListener;
}

//using namespace std;
using namespace Sexy;

class ReanimationWidget : public Widget
{
public:
    LawnApp*				mApp;
    Reanimation*			mReanim;
    LawnDialog*				mLawnDialog;
    float					mPosX;
    float					mPosY;

public:
	ReanimationWidget();
	~ReanimationWidget() override;

	/*inline*/ void			Dispose();
	void					Draw(Graphics*) override;
	void					Update() override;
	void					AddReanimation(float x, float y, ReanimationType theReanimationType);
};

class LawnDialog : public Dialog
{
public:
	LawnApp*				mApp;
	int						mButtonDelay;
	ReanimationWidget*		mReanimation;
	bool					mDrawStandardBack;
	LawnStoneButton*		mLawnYesButton;
	LawnStoneButton*		mLawnNoButton;
	bool					mTallBottom;
	bool					mVerticalCenterText;

public:
	LawnDialog(LawnApp* theApp, int theId, bool isModal, const std::string& theDialogHeader, const std::string& theDialogLines, const std::string& theDialogFooter, int theButtonMode);
	~LawnDialog() override;

	int						GetLeft();
	int						GetWidth();
	int						GetTop();
	virtual void			SetButtonDelay(int theDelay);
	void					Update() override;
	void					ButtonPress(int theId) override;
	void					ButtonDepress(int theId) override;
	virtual void			CheckboxChecked();
	void					KeyDown(KeyCode theKey) override;
	void					AddedToManager(WidgetManager* theWidgetManager) override;
	void					RemovedFromManager(WidgetManager* theWidgetManager) override;
	void					Resize(int theX, int theY, int theWidth, int theHeight) override;
	void					Draw(Graphics* g) override;
	void					CalcSize(int theExtraX, int theExtraY);
};

class GameOverDialog : public LawnDialog
{
public:
	DialogButton*			mMenuButton;

public:
	GameOverDialog(const std::string& theMessage, bool theShowChallengeName);
	~GameOverDialog() override;

	void					KeyDown(KeyCode theKey) override;
	void					ButtonDepress(int theId) override;
	void					AddedToManager(WidgetManager* theWidgetManager) override;
	void					RemovedFromManager(WidgetManager* theWidgetManager) override;
	void					MouseDrag(int x, int y) override;
};

#endif
