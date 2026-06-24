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

#ifndef __ZOMBATARWIDGET_H__
#define __ZOMBATARWIDGET_H__

#include "widget/Widget.h"
#include "widget/ButtonListener.h"

class GameSelector;
class LawnApp;
class PlayerInfo;
class NewLawnButton;

using namespace Sexy;

enum ZombatarPage
{
	ZOMBATAR_PAGE_SKIN,
	ZOMBATAR_PAGE_HAIR,
	ZOMBATAR_PAGE_FACIAL_HAIR,
	ZOMBATAR_PAGE_TIDBITS,
	ZOMBATAR_PAGE_EYEWEAR,
	ZOMBATAR_PAGE_CLOTHES,
	ZOMBATAR_PAGE_ACCESSORY,
	ZOMBATAR_PAGE_HATS,
	ZOMBATAR_PAGE_BACKDROPS,
	NUM_ZOMBATAR_PAGES
};

enum ZombatarWidgetState
{
	ZOMBATAR_STATE_LIST,
	ZOMBATAR_STATE_CREATE,
	ZOMBATAR_STATE_CONFIRM
};

class ZombatarWidget : public Widget, public ButtonListener
{
public:
	enum
	{
		ZOMBATAR_BTN_BACK = 300,
		ZOMBATAR_BTN_VIEW,
		ZOMBATAR_BTN_FINISHED,
		ZOMBATAR_BTN_NEW,
		ZOMBATAR_BTN_EDIT,
		ZOMBATAR_BTN_DELETE,
		ZOMBATAR_BTN_PREV_PORTRAIT,
		ZOMBATAR_BTN_NEXT_PORTRAIT,
		ZOMBATAR_BTN_PREV_PAGE,
		ZOMBATAR_BTN_NEXT_PAGE,
		ZOMBATAR_BTN_CATEGORY_BASE
	};

public:
	GameSelector*				mGameSelector;
	LawnApp*					mApp;
	ZombatarWidgetState			mState;
	ZombatarPage				mPage;
	int							mCurrentIndex;
	int							mEditingIndex;
	int							mFacialHairPage;
	int							mMouseX;
	int							mMouseY;
	int							mPart[NUM_ZOMBATAR_PAGES];
	int							mColor[NUM_ZOMBATAR_PAGES];

	NewLawnButton*				mBackButton;
	NewLawnButton*				mViewButton;
	NewLawnButton*				mFinishedButton;
	NewLawnButton*				mNewButton;
	NewLawnButton*				mEditButton;
	NewLawnButton*				mDeleteButton;
	NewLawnButton*				mPrevPortraitButton;
	NewLawnButton*				mNextPortraitButton;
	NewLawnButton*				mPrevPageButton;
	NewLawnButton*				mNextPageButton;

public:
	ZombatarWidget(GameSelector* theGameSelector);
	~ZombatarWidget() override;

	void						Open();
	void						ResetDraft();
	void						LoadCurrentToDraft();
	bool						SaveDraft();
	void						DeleteCurrent();
	bool						CanSaveNewHead() const;
	int							GetHeadCount() const;
	void						ClampCurrentIndex();
	void						ChangeState(ZombatarWidgetState theState);
	void						ChangePage(ZombatarPage thePage);

	void						Draw(Graphics* g) override;
	void						Update() override;
	void						AddedToManager(WidgetManager* theWidgetManager) override;
	void						RemovedFromManager(WidgetManager* theWidgetManager) override;
	void						MouseMove(int x, int y) override;
	void						MouseUp(int x, int y) override;
	void						KeyDown(KeyCode theKey) override;

	void						ButtonPress(int theId) override;
	void						ButtonDepress(int theId) override;
	void						ButtonDownTick(int) override {}
	void						ButtonMouseEnter(int) override {}
	void						ButtonMouseLeave(int) override {}
	void						ButtonMouseMove(int, int, int) override {}

private:
	void						DrawMain(Graphics* g);
	void						DrawList(Graphics* g);
	void						DrawCreate(Graphics* g);
	void						DrawAvatar(Graphics* g, int theX, int theY, const unsigned char* theRecord);
	void						DrawDraftAvatar(Graphics* g, int theX, int theY);
	void						DrawImageColorized(Graphics* g, Image* theImage, int theX, int theY, int theColorIndex);
	Rect						GetCategoryRect(int theIndex) const;
	Rect						GetItemRect(int theIndex) const;
	Rect						GetColorRect(int theIndex) const;
	int							GetItemCountForPage() const;
	bool						PageAllowsNone() const;
	bool						PageAllowsColors() const;
	Image*						GetCategoryImage(ZombatarPage thePage, bool theSelected, bool theOver) const;
	Image*						GetPartImage(ZombatarPage thePage, int theIndex) const;
	Image*						GetPartMaskImage(ZombatarPage thePage, int theIndex) const;
	Image*						GetBackgroundImage(int theIndex) const;
	bool						ExportAvatarPNG(const unsigned char* theRecord, int theExportIndex);
	bool						ExportAllAvatarPNGs();
	void						EraseAvatarPNG(int theExportIndex);
	void						DrawPartImage(Graphics* g, ZombatarPage thePage, int theIndex, int theX, int theY, int theColorIndex);
	void						DecodeRecord(const unsigned char* theRecord, int* thePart, int* theColor) const;
	void						EncodeRecord(unsigned char* theRecord) const;
	void						BackToSelector();
	void						ShowMaxHeadsMessage();
	void						BeginEditCurrent();
	void						HandleGridClick(int x, int y);
	void						HandleColorClick(int x, int y);
	void						UpdateButtonState();
};

#endif
