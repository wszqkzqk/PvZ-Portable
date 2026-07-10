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

#include "ZombatarWidget.h"
#include "GameSelector.h"
#include "GameButton.h"
#include "../Zombie.h"
#include "../System/PlayerInfo.h"
#include "../System/Zombatar.h"
#include "../../LawnApp.h"
#include "../../Resources.h"
#include "../../GameConstants.h"
#include "../../Sexy.TodLib/TodStringFile.h"
#include "graphics/Graphics.h"
#include "graphics/MemoryImage.h"
#include "graphics/Font.h"
#include "imagelib/ImageLib.h"
#include "misc/KeyCodes.h"
#include "widget/Dialog.h"
#include "widget/WidgetManager.h"

#include <algorithm>
#include <cstring>
#include <vector>

constexpr int ZOMBATAR_COLOR_NONE = -1;
constexpr int ZOMBATAR_SKIN_COLOR_COUNT = 12;
constexpr int ZOMBATAR_PART_COLOR_COUNT = 18;
constexpr int ZOMBATAR_PART_COLOR_BASE = 12;
constexpr int ZOMBATAR_PART_COLOR_BASE_2 = 30;
constexpr int ZOMBATAR_PART_COLOR_NONE_1 = 29;
constexpr int ZOMBATAR_PART_COLOR_NONE_2 = 47;

constexpr int ZOMBATAR_COLOR_MODE_SKIN = 0;
constexpr int ZOMBATAR_COLOR_MODE_1 = 1;
constexpr int ZOMBATAR_COLOR_MODE_2 = 2;
constexpr int ZOMBATAR_COLOR_MODE_NONE = 4;

constexpr int ZOMBATAR_PANEL_X = 25;
constexpr int ZOMBATAR_PANEL_Y = 25;
constexpr int ZOMBATAR_PANEL_W = 560;
constexpr int ZOMBATAR_INNER_X = 152;
constexpr int ZOMBATAR_INNER_Y = 125;
constexpr int ZOMBATAR_COLORS_X = 221;
constexpr int ZOMBATAR_COLORS_Y = 335;
constexpr int ZOMBATAR_PREVIEW_X = 592;
constexpr int ZOMBATAR_PREVIEW_Y = 115;

constexpr int ZOMBATAR_TABS_X = 58;
constexpr int ZOMBATAR_TABS_Y0 = 128;

constexpr int ZOMBATAR_GRID_COLS = 6;
constexpr int ZOMBATAR_GRID_ROWS = 3;
constexpr int ZOMBATAR_GRID_PAGE = 17;
constexpr int ZOMBATAR_GRID_GAP = -4;
constexpr int ZOMBATAR_GRID_BIAS_X = 50;
constexpr int ZOMBATAR_CELL_INSET = 9;
constexpr int ZOMBATAR_CELL_ZOMBIE_MARGIN = 10;
constexpr Color ZOMBATAR_CELL_DIM_COLOR(0x80, 0x80, 0x80, 0x80);

constexpr int ZOMBATAR_COLOR_COLS = 9;
constexpr int ZOMBATAR_COLOR_GAP = 4;
constexpr int ZOMBATAR_COLOR_X = 238;
constexpr int ZOMBATAR_COLOR_Y = 367;
constexpr int ZOMBATAR_COLOR_HINT_X = 240;
constexpr int ZOMBATAR_COLOR_HINT_Y = 380;

constexpr int ZOMBATAR_START_TEXT_Y = 185;
constexpr int ZOMBATAR_START_TEXT_W = 500;
constexpr int ZOMBATAR_START_TEXT_H = 100;

constexpr int ZOMBATAR_BACK_X = 278;
constexpr int ZOMBATAR_BACK_Y = 528;
constexpr int ZOMBATAR_VIEW_X = 65;
constexpr int ZOMBATAR_VIEW_Y = 472;
constexpr int ZOMBATAR_FINISHED_X = 445;
constexpr int ZOMBATAR_FINISHED_Y = 472;
constexpr int ZOMBATAR_ACCEPT_X = 155;
constexpr int ZOMBATAR_CONFIRM_BACK_X = 385;
constexpr int ZOMBATAR_CONFIRM_BTN_Y = 345;
constexpr int ZOMBATAR_NEW_X = 195;
constexpr int ZOMBATAR_NEW_Y = 395;
constexpr int ZOMBATAR_PREV_PORTRAIT_X = 120;
constexpr int ZOMBATAR_NEXT_PORTRAIT_X = 467;
constexpr int ZOMBATAR_PORTRAIT_BTN_Y = 398;
constexpr int ZOMBATAR_PREV_PAGE_X = 175;
constexpr int ZOMBATAR_NEXT_PAGE_X = 497;
constexpr int ZOMBATAR_PAGE_BTN_Y = 372;

constexpr int ZOMBATAR_LIST_PORTRAIT_Y = 175;
constexpr int ZOMBATAR_LIST_CUR_X = 239;
constexpr int ZOMBATAR_LIST_PREV_X = 57;
constexpr int ZOMBATAR_LIST_NEXT_X = 421;
constexpr int ZOMBATAR_LIST_COUNTER_X = 221;
constexpr int ZOMBATAR_LIST_COUNTER_Y = 161;
constexpr int ZOMBATAR_LIST_DELETE_X = 351;
constexpr int ZOMBATAR_LIST_DELETE_Y = 161;
constexpr int ZOMBATAR_LIST_DELETE_RECT_X = 346;
constexpr int ZOMBATAR_LIST_DELETE_RECT_Y = 146;
constexpr int ZOMBATAR_LIST_DELETE_RECT_W = 60;
constexpr int ZOMBATAR_LIST_DELETE_RECT_H = 20;

constexpr int ZOMBATAR_AVATAR_GROUND_X = 600;
constexpr int ZOMBATAR_AVATAR_GROUND_Y = 300;
constexpr int ZOMBATAR_AVATAR_ZOMBIE_X = ZOMBATAR_AVATAR_GROUND_X + 40;
constexpr int ZOMBATAR_AVATAR_ZOMBIE_Y = ZOMBATAR_AVATAR_GROUND_Y + 50;
constexpr int ZOMBATAR_AVATAR_GROUND_CLIP_TOP = 10;
constexpr int ZOMBATAR_AVATAR_GROUND_CLIP_RIGHT = 40;
constexpr int ZOMBATAR_AVATAR_GROUND_CLIP_BOTTOM = 10;

constexpr int ZOMBATAR_CONFIRM_HEADER_X = 305;
constexpr int ZOMBATAR_CONFIRM_HEADER_Y = 185;
constexpr int ZOMBATAR_CONFIRM_TEXT_X = 60;
constexpr int ZOMBATAR_CONFIRM_TEXT_Y = 225;
constexpr int ZOMBATAR_CONFIRM_TEXT_W = 500;
constexpr int ZOMBATAR_CONFIRM_TEXT_H = 100;
constexpr int ZOMBATAR_CONFIRM_ACCEPT_LABEL_X = 195;
constexpr int ZOMBATAR_CONFIRM_BACK_LABEL_X = 435;
constexpr int ZOMBATAR_CONFIRM_LABEL_Y = 335;

constexpr int ClampRange(int theValue, int theMin, int theMax)
{
	return std::max(theMin, std::min(theValue, theMax));
}

constexpr int SlotForPart(ZombatarPage thePage)
{
	switch (thePage)
	{
	case ZOMBATAR_PAGE_SKIN: return ZOMBATAR_SLOT_SKIN_PART;
	case ZOMBATAR_PAGE_CLOTHES: return ZOMBATAR_SLOT_CLOTHES;
	case ZOMBATAR_PAGE_TIDBITS: return ZOMBATAR_SLOT_TIDBITS;
	case ZOMBATAR_PAGE_ACCESSORY: return ZOMBATAR_SLOT_ACCESSORY;
	case ZOMBATAR_PAGE_FACIAL_HAIR: return ZOMBATAR_SLOT_FACIAL_HAIR;
	case ZOMBATAR_PAGE_HAIR: return ZOMBATAR_SLOT_HAIR;
	case ZOMBATAR_PAGE_EYEWEAR: return ZOMBATAR_SLOT_EYEWEAR;
	case ZOMBATAR_PAGE_HATS: return ZOMBATAR_SLOT_HATS;
	case ZOMBATAR_PAGE_BACKDROPS: return ZOMBATAR_SLOT_BACKGROUND;
	default: return ZOMBATAR_SLOT_SKIN_PART;
	}
}

constexpr int SlotForColor(ZombatarPage thePage)
{
	switch (thePage)
	{
	case ZOMBATAR_PAGE_SKIN: return ZOMBATAR_SLOT_SKIN_COLOR;
	case ZOMBATAR_PAGE_CLOTHES: return ZOMBATAR_SLOT_CLOTHES_COLOR;
	case ZOMBATAR_PAGE_TIDBITS: return ZOMBATAR_SLOT_TIDBITS_COLOR;
	case ZOMBATAR_PAGE_ACCESSORY: return ZOMBATAR_SLOT_ACCESSORY_COLOR;
	case ZOMBATAR_PAGE_FACIAL_HAIR: return ZOMBATAR_SLOT_FACIAL_HAIR_COLOR;
	case ZOMBATAR_PAGE_HAIR: return ZOMBATAR_SLOT_HAIR_COLOR;
	case ZOMBATAR_PAGE_EYEWEAR: return ZOMBATAR_SLOT_EYEWEAR_COLOR;
	case ZOMBATAR_PAGE_HATS: return ZOMBATAR_SLOT_HATS_COLOR;
	case ZOMBATAR_PAGE_BACKDROPS: return ZOMBATAR_SLOT_BACKGROUND_COLOR;
	default: return ZOMBATAR_SLOT_SKIN_COLOR;
	}
}

constexpr int GetPartColorMode(ZombatarPage thePage, int thePartIndex)
{
	switch (thePage)
	{
	case ZOMBATAR_PAGE_SKIN: return ZOMBATAR_COLOR_MODE_SKIN;
	case ZOMBATAR_PAGE_CLOTHES: return ZOMBATAR_COLOR_MODE_NONE;
	case ZOMBATAR_PAGE_FACIAL_HAIR: return ZOMBATAR_COLOR_MODE_1;
	case ZOMBATAR_PAGE_HAIR: return thePartIndex == 2 ? ZOMBATAR_COLOR_MODE_NONE : ZOMBATAR_COLOR_MODE_1;
	case ZOMBATAR_PAGE_BACKDROPS: return thePartIndex == 4 ? ZOMBATAR_COLOR_MODE_2 : ZOMBATAR_COLOR_MODE_NONE;
	case ZOMBATAR_PAGE_TIDBITS:
		switch (thePartIndex)
		{
		case 0: case 1: case 2: case 9: case 10: case 11: return ZOMBATAR_COLOR_MODE_2;
		default: return ZOMBATAR_COLOR_MODE_NONE;
		}
	case ZOMBATAR_PAGE_EYEWEAR:
		return thePartIndex < 12 ? ZOMBATAR_COLOR_MODE_2 : ZOMBATAR_COLOR_MODE_NONE;
	case ZOMBATAR_PAGE_ACCESSORY:
		switch (thePartIndex)
		{
		case 7: case 9: case 11: case 12: return ZOMBATAR_COLOR_MODE_2;
		default: return ZOMBATAR_COLOR_MODE_NONE;
		}
	case ZOMBATAR_PAGE_HATS:
		return thePartIndex == 12 ? ZOMBATAR_COLOR_MODE_NONE : ZOMBATAR_COLOR_MODE_2;
	default: return ZOMBATAR_COLOR_MODE_NONE;
	}
}

struct ZombatarPartLayout
{
	int mOffsetX;
	int mOffsetY;
	int mColorOffsetX;
	int mColorOffsetY;
	int mDrawOrder;
};

constexpr ZombatarPartLayout gClothesLayout[12] =
{
	{88, 110, 0, 0, 0}, {75, 100, 0, 0, 0}, {85, 112, 0, 0, 0}, {76, 110, 0, 0, 0},
	{89, 115, 0, 0, 0}, {93, 110, 0, 0, 0}, {78, 105, 0, 0, 0}, {88, 110, 0, 0, 0},
	{88, 102, 0, 0, 0}, {85, 110, 0, 0, 0}, {85, 110, 0, 0, 0}, {79, 112, 0, 0, 0}
};

constexpr ZombatarPartLayout gTidbitsLayout[14] =
{
	{28, 63, 18, 48, 2}, {28, 63, 0, 0, 2}, {46, 111, 0, 0, 2}, {31, 62, 0, 0, 4},
	{31, 58, 0, 0, 4}, {28, 66, 0, 0, 4}, {28, 72, 0, 0, 4}, {33, 55, 0, 0, 4},
	{21, 76, 0, 0, 4}, {36, 71, 0, 0, 2}, {36, 70, 0, 0, 2}, {86, 91, 0, 6, 2},
	{88, 50, 0, 8, 4}, {113, 115, 0, 0, 4}
};

constexpr ZombatarPartLayout gAccessoryLayout[15] =
{
	{103, 110, 0, 0, 4}, {108, 110, 0, 0, 4}, {86, 113, 0, 0, 4}, {131, 95, 0, 0, 4},
	{131, 100, 0, 0, 4}, {131, 100, 0, 0, 4}, {104, 111, 0, 0, 4}, {118, 65, 0, 0, 4},
	{61, 118, 0, 0, 4}, {43, 100, 0, 0, 4}, {135, 92, 0, 0, 4}, {78, 130, 0, 0, 4},
	{68, 145, 0, 0, 4}, {133, 70, 0, 0, 4}, {13, 40, 0, 0, 10}
};

constexpr ZombatarPartLayout gFacialHairLayout[24] =
{
	{35, 107, 1, 0, 6}, {51, 110, 0, 0, 6}, {45, 110, 0, 0, 6}, {38, 105, 3, 2, 6},
	{69, 145, 0, 0, 6}, {48, 112, 0, 0, 6}, {13, 107, 0, 0, 6}, {45, 105, 1, 1, 6},
	{41, 105, 1, 1, 6}, {44, 112, 1, 2, 6}, {43, 88, 1, 4, 6}, {28, 105, 8, 1, 6},
	{45, 110, 0, 0, 6}, {18, 103, 1, 1, 6}, {63, 145, 2, 1, 6}, {63, 140, 1, 1, 6},
	{43, 110, 0, 0, 6}, {43, 110, 0, 0, 6}, {58, 96, 1, 3, 6}, {46, 92, 0, 0, 6},
	{114, 80, 0, 0, 6}, {118, 83, 1, 1, 6}, {13, 87, 3, 4, 6}, {58, 145, 1, 1, 6}
};

constexpr ZombatarPartLayout gHairLayout[16] =
{
	{23, 0, 8, 1, 8}, {23, 25, 2, 3, 8}, {23, 30, 0, 0, 8}, {30, 15, 0, 0, 8},
	{36, 37, 0, 0, 8}, {39, 13, 0, 0, 8}, {51, 22, 0, 0, 8}, {28, 15, 0, 0, 8},
	{128, 55, 0, 0, 8}, {22, 32, 0, 0, 8}, {25, 19, 2, 2, 8}, {51, -5, 2, 2, 8},
	{33, 13, 2, 2, 8}, {9, -2, 1, 5, 8}, {45, 4, 0, -1, 8}, {26, 20, 0, 0, 8}
};

constexpr ZombatarPartLayout gEyewearLayout[16] =
{
	{28, 73, 0, 0, 10}, {31, 85, 0, -1, 10}, {28, 69, 0, 1, 10}, {28, 78, 0, -1, 10},
	{30, 75, -1, -1, 10}, {30, 78, -1, -1, 10}, {50, 90, -1, -1, 10}, {32, 70, -1, -1, 10},
	{36, 100, -1, -1, 10}, {31, 75, -1, -1, 10}, {31, 67, -1, -1, 10}, {38, 95, -1, -1, 10},
	{30, 81, 0, 0, 10}, {35, 64, 0, 0, 10}, {42, 65, 0, 0, 10}, {35, 65, 0, 0, 10}
};

constexpr ZombatarPartLayout gHatsLayout[14] =
{
	{28, 5, 2, 1, 12}, {47, 12, 0, 0, 12}, {36, 20, 15, -1, 12}, {11, 10, 0, 0, 12},
	{41, 16, 0, 0, 12}, {18, 3, 4, -2, 12}, {53, 17, 0, 15, 12}, {3, 0, 0, -2, 12},
	{38, 0, -1, -2, 12}, {13, 45, 0, 0, 12}, {63, 8, 1, 14, 12}, {43, 15, 0, 0, 12},
	{18, 0, 0, 0, 12}, {23, 5, 0, 0, 12}
};

const ZombatarPartLayout* GetPartLayout(ZombatarPage thePage, int theIndex)
{
	switch (thePage)
	{
	case ZOMBATAR_PAGE_CLOTHES: return theIndex < 12 ? &gClothesLayout[theIndex] : nullptr;
	case ZOMBATAR_PAGE_TIDBITS: return theIndex < 14 ? &gTidbitsLayout[theIndex] : nullptr;
	case ZOMBATAR_PAGE_ACCESSORY: return theIndex < 15 ? &gAccessoryLayout[theIndex] : nullptr;
	case ZOMBATAR_PAGE_FACIAL_HAIR:
	{
		int aIdx = theIndex;
		if (aIdx > 16)
			aIdx -= aIdx / 17;
		return aIdx < 24 ? &gFacialHairLayout[aIdx] : nullptr;
	}
	case ZOMBATAR_PAGE_HAIR: return theIndex < 16 ? &gHairLayout[theIndex] : nullptr;
	case ZOMBATAR_PAGE_EYEWEAR: return theIndex < 16 ? &gEyewearLayout[theIndex] : nullptr;
	case ZOMBATAR_PAGE_HATS: return theIndex < 14 ? &gHatsLayout[theIndex] : nullptr;
	default: return nullptr;
	}
}

struct ZombatarDrawPart
{
	ZombatarPage mPage;
	int mIndex;
	int mColor;
	int mDrawOrder;

	bool operator<(const ZombatarDrawPart& theOther) const { return mDrawOrder < theOther.mDrawOrder; }
};

constexpr int ZombatarColorBaseForMode(int theMode)
{
	return theMode == ZOMBATAR_COLOR_MODE_1 ? ZOMBATAR_PART_COLOR_BASE : ZOMBATAR_PART_COLOR_BASE_2;
}

static Rect FitIconRect(Image* theImage, const Rect& theCell)
{
	int aCellW = theCell.mWidth;
	int aCellH = theCell.mHeight;
	int aAvailW = aCellW - 2 * ZOMBATAR_CELL_INSET;
	int aAvailH = aCellH - 2 * ZOMBATAR_CELL_INSET;
	int aW = theImage->mWidth;
	int aH = theImage->mHeight;
	bool aOvflW = static_cast<float>(aW) > static_cast<float>(aAvailW);
	bool aOvflH = static_cast<float>(aH) > static_cast<float>(aAvailH);
	if (aOvflW && (!aOvflH || aW >= aH))
	{
		aH = static_cast<int>(aH * (static_cast<float>(aAvailW) / static_cast<float>(aW)));
		aW = aAvailW;
	}
	else if (aOvflH)
	{
		aW = static_cast<int>(aW * (static_cast<float>(aAvailH) / static_cast<float>(aW)));
		aH = aAvailH;
	}
	return Rect(theCell.mX + (aCellW - aW) / 2, theCell.mY + (aCellH - aH) / 2, aW, aH);
}

constexpr int ZOMBATAR_ITEMS_PER_PAGE[NUM_ZOMBATAR_PAGES] = { 0, 16, 24, 14, 16, 12, 15, 14, 5 };

ZombatarWidget::ZombatarWidget(GameSelector* theGameSelector)
{
	mGameSelector = theGameSelector;
	mApp = theGameSelector->mApp;
	mState = ZOMBATAR_STATE_CREATE;
	mPage = ZOMBATAR_PAGE_SKIN;
	mCurrentIndex = 0;
	mSubPage = 0;
	mMaxSubPages = 0;
	mMouseX = -1;
	mMouseY = -1;
	mHoverGridCell = -1;
	mHoverColorCell = -1;
	mDeleteHover = false;
	mPreviewZombie = nullptr;

	mBackButton = MakeNewButton(ZOMBATAR_BTN_BACK, this, "", nullptr,
		IMAGE_BLANK, IMAGE_ZOMBATAR_MAINMENUBACK_HIGHLIGHT, IMAGE_ZOMBATAR_MAINMENUBACK_HIGHLIGHT);
	{
		int aBackW = IMAGE_ZOMBATAR_MAINMENUBACK_HIGHLIGHT ? IMAGE_ZOMBATAR_MAINMENUBACK_HIGHLIGHT->mWidth : 98;
		int aBackH = IMAGE_ZOMBATAR_MAINMENUBACK_HIGHLIGHT ? IMAGE_ZOMBATAR_MAINMENUBACK_HIGHLIGHT->mHeight : 26;
		mBackButton->Resize(ZOMBATAR_BACK_X, ZOMBATAR_BACK_Y, aBackW, aBackH);
	}

	mViewButton = MakeNewButton(ZOMBATAR_BTN_VIEW, this, "", nullptr,
		IMAGE_ZOMBATAR_VIEW_BUTTON, IMAGE_ZOMBATAR_VIEW_BUTTON_HIGHLIGHT, nullptr);
	mViewButton->Resize(ZOMBATAR_VIEW_X, ZOMBATAR_VIEW_Y, 115, 26);

	mFinishedButton = MakeNewButton(ZOMBATAR_BTN_FINISHED, this, "", nullptr,
		IMAGE_ZOMBATAR_FINISHED_BUTTON, IMAGE_ZOMBATAR_FINISHED_BUTTON_HIGHLIGHT, nullptr);
	mFinishedButton->Resize(ZOMBATAR_FINISHED_X, ZOMBATAR_FINISHED_Y, 103, 26);

	mNewButton = MakeNewButton(ZOMBATAR_BTN_NEW, this, "", nullptr,
		IMAGE_ZOMBATAR_NEWZOMBIE_BUTTON, IMAGE_ZOMBATAR_NEWZOMBIE_BUTTON_HIGHLIGHT, nullptr);
	mNewButton->Resize(ZOMBATAR_NEW_X, ZOMBATAR_NEW_Y, 234, 39);

	mConfirmBackButton = MakeNewButton(ZOMBATAR_BTN_CONFIRM_BACK, this, "", nullptr,
		IMAGE_ZOMBATAR_BACK_BUTTON, IMAGE_ZOMBATAR_BACK_BUTTON_HIGHLIGHT, nullptr);
	mConfirmBackButton->Resize(ZOMBATAR_CONFIRM_BACK_X, ZOMBATAR_CONFIRM_BTN_Y, 98, 26);

	mPrevPortraitButton = MakeNewButton(ZOMBATAR_BTN_PREV_PORTRAIT, this, "", nullptr,
		IMAGE_ZOMBATAR_PREV_BUTTON, IMAGE_ZOMBATAR_PREV_BUTTON_HIGHLIGHT, nullptr);
	mPrevPortraitButton->Resize(ZOMBATAR_PREV_PORTRAIT_X, ZOMBATAR_PORTRAIT_BTN_Y, 33, 38);

	mNextPortraitButton = MakeNewButton(ZOMBATAR_BTN_NEXT_PORTRAIT, this, "", nullptr,
		IMAGE_ZOMBATAR_NEXT_BUTTON, IMAGE_ZOMBATAR_NEXT_BUTTON_HIGHLIGHT, nullptr);
	mNextPortraitButton->Resize(ZOMBATAR_NEXT_PORTRAIT_X, ZOMBATAR_PORTRAIT_BTN_Y, 33, 38);

	mPrevPageButton = MakeNewButton(ZOMBATAR_BTN_PREV_PAGE, this, "", nullptr,
		IMAGE_ZOMBATAR_PREV_BUTTON, IMAGE_ZOMBATAR_PREV_BUTTON_HIGHLIGHT, nullptr);
	mPrevPageButton->Resize(ZOMBATAR_PREV_PAGE_X, ZOMBATAR_PAGE_BTN_Y, 33, 38);

	mNextPageButton = MakeNewButton(ZOMBATAR_BTN_NEXT_PAGE, this, "", nullptr,
		IMAGE_ZOMBATAR_NEXT_BUTTON, IMAGE_ZOMBATAR_NEXT_BUTTON_HIGHLIGHT, nullptr);
	mNextPageButton->Resize(ZOMBATAR_NEXT_PAGE_X, ZOMBATAR_PAGE_BTN_Y, 33, 38);

	Resize(800, 0, BOARD_WIDTH, BOARD_HEIGHT);
	ResetDraft();
	UpdateButtonState();
}

ZombatarWidget::~ZombatarWidget()
{
	DestroyPreviewZombie();
	delete mBackButton;
	delete mViewButton;
	delete mFinishedButton;
	delete mNewButton;
	delete mConfirmBackButton;
	delete mPrevPortraitButton;
	delete mNextPortraitButton;
	delete mPrevPageButton;
	delete mNextPageButton;
}

void ZombatarWidget::AddedToManager(WidgetManager* theWidgetManager)
{
	Widget::AddedToManager(theWidgetManager);
	AddWidget(mBackButton);
	AddWidget(mViewButton);
	AddWidget(mFinishedButton);
	AddWidget(mNewButton);
	AddWidget(mConfirmBackButton);
	AddWidget(mPrevPortraitButton);
	AddWidget(mNextPortraitButton);
	AddWidget(mPrevPageButton);
	AddWidget(mNextPageButton);
}

void ZombatarWidget::RemovedFromManager(WidgetManager* theWidgetManager)
{
	Widget::RemovedFromManager(theWidgetManager);
	DestroyPreviewZombie();
	RemoveWidget(mBackButton);
	RemoveWidget(mViewButton);
	RemoveWidget(mFinishedButton);
	RemoveWidget(mNewButton);
	RemoveWidget(mConfirmBackButton);
	RemoveWidget(mPrevPortraitButton);
	RemoveWidget(mNextPortraitButton);
	RemoveWidget(mPrevPageButton);
	RemoveWidget(mNextPageButton);
}

void ZombatarWidget::Update()
{
	Widget::Update();
	if (mPreviewZombie)
		mPreviewZombie->Update();
	MarkDirty();
}

void ZombatarWidget::Open()
{
	if (!mApp->mPlayerInfo)
		return;

	CreatePreviewZombie();

	mSubPage = 0;
	mMaxSubPages = 0;
	mPage = ZOMBATAR_PAGE_SKIN;
	mDeleteHover = false;
	mCurrentIndex = 0;
	ClampCurrentIndex();

	if (GetHeadCount() > 0)
	{
		ChangeState(ZOMBATAR_STATE_LIST);
		LoadCurrentToDraft();
	}
	else
	{
		ChangeState(ZOMBATAR_STATE_CREATE);
		ResetDraft();
	}

	mGameSelector->SlideTo(-BOARD_WIDTH, 0);
	if (mWidgetManager)
	{
		mWidgetManager->BringToFront(this);
		mWidgetManager->SetFocus(this);
	}
}

void ZombatarWidget::ResetDraft()
{
	for (int i = 0; i < NUM_ZOMBATAR_PAGES; i++)
	{
		mPart[i] = -1;
		mColor[i] = ZOMBATAR_COLOR_NONE;
	}

	mPart[ZOMBATAR_PAGE_SKIN] = 0;
	mColor[ZOMBATAR_PAGE_SKIN] = 0;
	mPart[ZOMBATAR_PAGE_BACKDROPS] = 4;
	mColor[ZOMBATAR_PAGE_BACKDROPS] = ZOMBATAR_PART_COLOR_NONE_2;
}

int ZombatarWidget::GetHeadCount() const
{
	if (!mApp->mPlayerInfo)
		return 0;
	return static_cast<int>(mApp->mPlayerInfo->mZombatarData.size() / ZOMBATAR_RECORD_SIZE);
}

bool ZombatarWidget::CanSaveNewHead() const
{
	return GetHeadCount() < MAX_ZOMBATAR_HEADS;
}

void ZombatarWidget::ShowMaxHeadsMessage()
{
	mApp->LawnMessageBox(DIALOG_MESSAGE, "Zombatar Limit Reached", "This profile already has the maximum number of saved Zombatars.", "[DIALOG_BUTTON_OK]", "", Dialog::BUTTONS_FOOTER);
}

void ZombatarWidget::ClampCurrentIndex()
{
	int aCount = GetHeadCount();
	if (aCount <= 0)
	{
		mCurrentIndex = 0;
	}
	else
	{
		mCurrentIndex = ClampRange(mCurrentIndex, 0, aCount - 1);
	}
}

void ZombatarWidget::DecodeRecord(const unsigned char* theRecord, int* thePart, int* theColor) const
{
	for (int i = 0; i < NUM_ZOMBATAR_PAGES; i++)
	{
		thePart[i] = -1;
		theColor[i] = ZOMBATAR_COLOR_NONE;
	}

	thePart[ZOMBATAR_PAGE_SKIN] = 0;
	{
		int aSkinColor = ZombatarReadSignedRecordSlot(theRecord, ZOMBATAR_SLOT_SKIN_COLOR);
		theColor[ZOMBATAR_PAGE_SKIN] = aSkinColor < 0 ? 0 : std::min(aSkinColor, 11);
	}

	for (int i = ZOMBATAR_PAGE_HAIR; i < NUM_ZOMBATAR_PAGES; i++)
	{
		ZombatarPage aPage = static_cast<ZombatarPage>(i);
		int aItemCount = GetTotalItemsForPage(aPage);
		int aPart = ZombatarReadSignedRecordSlot(theRecord, SlotForPart(aPage));
		int aColor = ZombatarReadSignedRecordSlot(theRecord, SlotForColor(aPage));

		if (aPage == ZOMBATAR_PAGE_BACKDROPS)
			thePart[i] = ClampRange(aPart, 0, aItemCount - 1);
		else
			thePart[i] = (aPart >= 0 && aPart < aItemCount) ? aPart : -1;

		if (aColor < 0)
			theColor[i] = ZOMBATAR_COLOR_NONE;
		else
			theColor[i] = std::min(aColor, 47);
	}
}

void ZombatarWidget::EncodeRecord(unsigned char* theRecord) const
{
	memset(theRecord, 0, ZOMBATAR_RECORD_SIZE);
	ZombatarWriteRecordSlot(theRecord, ZOMBATAR_SLOT_SKIN_PART, ZOMBATAR_COLOR_NONE);
	ZombatarWriteRecordSlot(theRecord, ZOMBATAR_SLOT_SKIN_COLOR, ClampRange(mColor[ZOMBATAR_PAGE_SKIN], 0, 11));

	for (int i = ZOMBATAR_PAGE_HAIR; i < NUM_ZOMBATAR_PAGES; i++)
	{
		ZombatarPage aPage = static_cast<ZombatarPage>(i);
		ZombatarWriteRecordSlot(theRecord, SlotForPart(aPage), mPart[i]);
		ZombatarWriteRecordSlot(theRecord, SlotForColor(aPage), mColor[i]);
	}
}

void ZombatarWidget::LoadCurrentToDraft()
{
	if (!mApp->mPlayerInfo || GetHeadCount() <= 0)
	{
		ResetDraft();
		return;
	}

	ClampCurrentIndex();
	const unsigned char* aRecord = mApp->mPlayerInfo->mZombatarData.data() + mCurrentIndex * ZOMBATAR_RECORD_SIZE;
	DecodeRecord(aRecord, mPart, mColor);
}

bool ZombatarWidget::SaveDraft()
{
	PlayerInfo* aPlayerInfo = mApp->mPlayerInfo;
	if (!aPlayerInfo || !CanSaveNewHead())
		return false;

	size_t aOffset = aPlayerInfo->mZombatarData.size();
	aPlayerInfo->mZombatarData.resize(aOffset + ZOMBATAR_RECORD_SIZE);
	EncodeRecord(aPlayerInfo->mZombatarData.data() + aOffset);
	aPlayerInfo->mZombatarHeadCount = static_cast<uint32_t>(GetHeadCount());
	mCurrentIndex = static_cast<int>(aOffset / ZOMBATAR_RECORD_SIZE);

	if (!ExportAvatarPNG(aPlayerInfo->mZombatarData.data() + aOffset, mCurrentIndex + 1))
	{
		aPlayerInfo->mZombatarData.resize(aOffset);
		aPlayerInfo->mZombatarHeadCount = static_cast<uint32_t>(GetHeadCount());
		ClampCurrentIndex();
		mApp->LawnMessageBox(DIALOG_MESSAGE, "Zombatar Export Failed", "The Zombatar image could not be written.", "[DIALOG_BUTTON_OK]", "", Dialog::BUTTONS_FOOTER);
		return false;
	}

	aPlayerInfo->mZombatarCreatedBefore = 1;
	aPlayerInfo->SaveDetails();
	ResetDraft();
	mPage = ZOMBATAR_PAGE_SKIN;
	mSubPage = 0;
	return true;
}

void ZombatarWidget::DeleteCurrent()
{
	PlayerInfo* aPlayerInfo = mApp->mPlayerInfo;
	int aCount = GetHeadCount();
	if (!aPlayerInfo || aCount <= 0)
		return;

	ClampCurrentIndex();
	std::vector<unsigned char> aOldData = aPlayerInfo->mZombatarData;
	uint32_t aOldHeadCount = aPlayerInfo->mZombatarHeadCount;
	int aOldIndex = mCurrentIndex;
	unsigned char* aData = aPlayerInfo->mZombatarData.data();
	size_t aOffset = static_cast<size_t>(mCurrentIndex) * ZOMBATAR_RECORD_SIZE;
	size_t aTailOffset = aOffset + ZOMBATAR_RECORD_SIZE;
	size_t aTailBytes = aPlayerInfo->mZombatarData.size() - aTailOffset;
	if (aTailBytes > 0)
		memmove(aData + aOffset, aData + aTailOffset, aTailBytes);
	aPlayerInfo->mZombatarData.resize(aPlayerInfo->mZombatarData.size() - ZOMBATAR_RECORD_SIZE);
	aPlayerInfo->mZombatarHeadCount = static_cast<uint32_t>(GetHeadCount());
	if (!ExportAllAvatarPNGs())
	{
		aPlayerInfo->mZombatarData = aOldData;
		aPlayerInfo->mZombatarHeadCount = aOldHeadCount;
		mCurrentIndex = aOldIndex;
		ExportAllAvatarPNGs();
		mApp->LawnMessageBox(DIALOG_MESSAGE, "Zombatar Export Failed", "The Zombatar image files could not be updated.", "[DIALOG_BUTTON_OK]", "", Dialog::BUTTONS_FOOTER);
		return;
	}
	EraseAvatarPNG(aCount);
	ClampCurrentIndex();

	if (GetHeadCount() > 0)
	{
		LoadCurrentToDraft();
		ChangeState(ZOMBATAR_STATE_LIST);
	}
	else
	{
		ResetDraft();
		ChangeState(ZOMBATAR_STATE_CREATE);
	}

	aPlayerInfo->SaveDetails();
}

Rect ZombatarWidget::GetCategoryRect(int theIndex) const
{
	Image* aImage = GetCategoryImage(static_cast<ZombatarPage>(theIndex), false, false);
	int aTabH = aImage ? aImage->mHeight : 39;
	int aTabW = aImage ? aImage->mWidth : 104;
	int aStep = std::max(8, aTabH - 3);
	return Rect(ZOMBATAR_TABS_X, ZOMBATAR_TABS_Y0 + theIndex * aStep, aTabW, aTabH);
}

Rect ZombatarWidget::GetItemHitRect(int theIndex) const
{
	Rect aRect = GetItemRect(theIndex);
	return Rect(aRect.mX + ZOMBATAR_CELL_INSET, aRect.mY + ZOMBATAR_CELL_INSET,
		aRect.mWidth - 2 * ZOMBATAR_CELL_INSET, aRect.mHeight - 2 * ZOMBATAR_CELL_INSET);
}

Rect ZombatarWidget::GetItemRect(int theIndex) const
{
	int aCellW = IMAGE_ZOMBATAR_ACCESSORY_BG ? IMAGE_ZOMBATAR_ACCESSORY_BG->mWidth : 67;
	int aCellH = IMAGE_ZOMBATAR_ACCESSORY_BG ? IMAGE_ZOMBATAR_ACCESSORY_BG->mHeight : 67;
	int aStepX = aCellW + ZOMBATAR_GRID_GAP;
	int aStepY = aCellH + 3 + ZOMBATAR_GRID_GAP;
	int aOriginX = ZOMBATAR_PANEL_X + (ZOMBATAR_PANEL_W - ZOMBATAR_GRID_COLS * aStepX) / 2 + ZOMBATAR_GRID_BIAS_X;
	int aOriginY = ZOMBATAR_PANEL_Y + 2 * ZOMBATAR_GRID_GAP + 120;
	int aCol = theIndex % ZOMBATAR_GRID_COLS;
	int aRow = theIndex / ZOMBATAR_GRID_COLS;
	return Rect(aOriginX + aCol * aStepX, aOriginY + aRow * aStepY, aCellW, aCellH);
}

Rect ZombatarWidget::GetColorRect(int theIndex) const
{
	int aSwatchW = IMAGE_ZOMBATAR_COLORPICKER ? IMAGE_ZOMBATAR_COLORPICKER->mWidth : 21;
	int aSwatchH = IMAGE_ZOMBATAR_COLORPICKER ? IMAGE_ZOMBATAR_COLORPICKER->mHeight : 20;
	int aStepX = aSwatchW + ZOMBATAR_COLOR_GAP;
	int aStepY = aSwatchH + ZOMBATAR_COLOR_GAP;
	int aCol = theIndex % ZOMBATAR_COLOR_COLS;
	int aRow = theIndex / ZOMBATAR_COLOR_COLS;
	return Rect(ZOMBATAR_COLOR_X + aCol * aStepX, ZOMBATAR_COLOR_Y + aRow * aStepY, aSwatchW, aSwatchH);
}

int ZombatarWidget::GetTotalItemsForPage(ZombatarPage thePage) const
{
	return ZOMBATAR_ITEMS_PER_PAGE[ClampRange(static_cast<int>(thePage), 0, NUM_ZOMBATAR_PAGES - 1)];
}

int ZombatarWidget::GetItemCountForPage() const
{
	int aTotal = GetTotalItemsForPage(mPage);
	int aRemaining = aTotal - mSubPage * ZOMBATAR_GRID_PAGE;
	return ClampRange(aRemaining, 0, ZOMBATAR_GRID_PAGE);
}

bool ZombatarWidget::PageAllowsNone() const
{
	return mPage != ZOMBATAR_PAGE_SKIN && mPage != ZOMBATAR_PAGE_BACKDROPS;
}

bool ZombatarWidget::PageAllowsColors() const
{
	if (mPage == ZOMBATAR_PAGE_SKIN)
		return true;
	if (mPart[mPage] < 0)
		return false;
	int aMode = GetPartColorMode(mPage, mPart[mPage]);
	return aMode == ZOMBATAR_COLOR_MODE_1 || aMode == ZOMBATAR_COLOR_MODE_2;
}

Image* ZombatarWidget::GetCategoryImage(ZombatarPage thePage, bool theSelected, bool theOver) const
{
	struct CategoryImages
	{
		Image* mNormal;
		Image* mHighlight;
		Image* mOver;
	};

	static const CategoryImages aImages[] =
	{
		{ IMAGE_ZOMBATAR_SKIN_BUTTON, IMAGE_ZOMBATAR_SKIN_BUTTON_HIGHLIGHT, IMAGE_ZOMBATAR_SKIN_BUTTON_HIGHLIGHT },
		{ IMAGE_ZOMBATAR_HAIR_BUTTON, IMAGE_ZOMBATAR_HAIR_BUTTON_HIGHLIGHT, IMAGE_ZOMBATAR_HAIR_BUTTON_OVER },
		{ IMAGE_ZOMBATAR_FACIAL_HAIR_BUTTON, IMAGE_ZOMBATAR_FACIAL_HAIR_BUTTON_HIGHLIGHT, IMAGE_ZOMBATAR_FACIAL_HAIR_BUTTON_OVER },
		{ IMAGE_ZOMBATAR_TIDBITS_BUTTON, IMAGE_ZOMBATAR_TIDBITS_BUTTON_HIGHLIGHT, IMAGE_ZOMBATAR_TIDBITS_BUTTON_OVER },
		{ IMAGE_ZOMBATAR_EYEWEAR_BUTTON, IMAGE_ZOMBATAR_EYEWEAR_BUTTON_HIGHLIGHT, IMAGE_ZOMBATAR_EYEWEAR_BUTTON_OVER },
		{ IMAGE_ZOMBATAR_CLOTHES_BUTTON, IMAGE_ZOMBATAR_CLOTHES_BUTTON_HIGHLIGHT, IMAGE_ZOMBATAR_CLOTHES_BUTTON_OVER },
		{ IMAGE_ZOMBATAR_ACCESSORY_BUTTON, IMAGE_ZOMBATAR_ACCESSORY_BUTTON_HIGHLIGHT, IMAGE_ZOMBATAR_ACCESSORY_BUTTON_OVER },
		{ IMAGE_ZOMBATAR_HATS_BUTTON, IMAGE_ZOMBATAR_HATS_BUTTON_HIGHLIGHT, IMAGE_ZOMBATAR_HATS_BUTTON_OVER },
		{ IMAGE_ZOMBATAR_BACKDROPS_BUTTON, IMAGE_ZOMBATAR_BACKDROPS_BUTTON_HIGHLIGHT, IMAGE_ZOMBATAR_BACKDROPS_BUTTON_OVER }
	};

	const CategoryImages& aCategoryImages = aImages[ClampRange(static_cast<int>(thePage), 0, NUM_ZOMBATAR_PAGES - 1)];
	if (theSelected)
		return aCategoryImages.mHighlight;
	if (theOver && aCategoryImages.mOver)
		return aCategoryImages.mOver;
	return aCategoryImages.mNormal;
}

Image* ZombatarWidget::GetBackgroundImage(int theIndex) const
{
	static Image* const aImages[] =
	{
		IMAGE_ZOMBATAR_BACKGROUND_CRAZYDAVE,
		IMAGE_ZOMBATAR_BACKGROUND_MENU,
		IMAGE_ZOMBATAR_BACKGROUND_MENU_DOS,
		IMAGE_ZOMBATAR_BACKGROUND_ROOF,
		IMAGE_ZOMBATAR_BACKGROUND_BLANK
	};
	return aImages[ClampRange(theIndex, 0, 4)];
}

Image* ZombatarWidget::GetPartImage(ZombatarPage thePage, int theIndex) const
{
	static Image* const aClothes[] = { IMAGE_ZOMBATAR_CLOTHES_1, IMAGE_ZOMBATAR_CLOTHES_2, IMAGE_ZOMBATAR_CLOTHES_3, IMAGE_ZOMBATAR_CLOTHES_4, IMAGE_ZOMBATAR_CLOTHES_5, IMAGE_ZOMBATAR_CLOTHES_6, IMAGE_ZOMBATAR_CLOTHES_7, IMAGE_ZOMBATAR_CLOTHES_8, IMAGE_ZOMBATAR_CLOTHES_9, IMAGE_ZOMBATAR_CLOTHES_10, IMAGE_ZOMBATAR_CLOTHES_11, IMAGE_ZOMBATAR_CLOTHES_12 };
	static Image* const aHats[] = { IMAGE_ZOMBATAR_HATS_1, IMAGE_ZOMBATAR_HATS_2, IMAGE_ZOMBATAR_HATS_3, IMAGE_ZOMBATAR_HATS_4, IMAGE_ZOMBATAR_HATS_5, IMAGE_ZOMBATAR_HATS_6, IMAGE_ZOMBATAR_HATS_7, IMAGE_ZOMBATAR_HATS_8, IMAGE_ZOMBATAR_HATS_9, IMAGE_ZOMBATAR_HATS_10, IMAGE_ZOMBATAR_HATS_11, IMAGE_ZOMBATAR_HATS_12, IMAGE_ZOMBATAR_HATS_13, IMAGE_ZOMBATAR_HATS_14 };
	static Image* const aHair[] = { IMAGE_ZOMBATAR_HAIR_1, IMAGE_ZOMBATAR_HAIR_2, IMAGE_ZOMBATAR_HAIR_3, IMAGE_ZOMBATAR_HAIR_4, IMAGE_ZOMBATAR_HAIR_5, IMAGE_ZOMBATAR_HAIR_6, IMAGE_ZOMBATAR_HAIR_7, IMAGE_ZOMBATAR_HAIR_8, IMAGE_ZOMBATAR_HAIR_9, IMAGE_ZOMBATAR_HAIR_10, IMAGE_ZOMBATAR_HAIR_11, IMAGE_ZOMBATAR_HAIR_12, IMAGE_ZOMBATAR_HAIR_13, IMAGE_ZOMBATAR_HAIR_14, IMAGE_ZOMBATAR_HAIR_15, IMAGE_ZOMBATAR_HAIR_16 };
	static Image* const aEyewear[] = { IMAGE_ZOMBATAR_EYEWEAR_1, IMAGE_ZOMBATAR_EYEWEAR_2, IMAGE_ZOMBATAR_EYEWEAR_3, IMAGE_ZOMBATAR_EYEWEAR_4, IMAGE_ZOMBATAR_EYEWEAR_5, IMAGE_ZOMBATAR_EYEWEAR_6, IMAGE_ZOMBATAR_EYEWEAR_7, IMAGE_ZOMBATAR_EYEWEAR_8, IMAGE_ZOMBATAR_EYEWEAR_9, IMAGE_ZOMBATAR_EYEWEAR_10, IMAGE_ZOMBATAR_EYEWEAR_11, IMAGE_ZOMBATAR_EYEWEAR_12, IMAGE_ZOMBATAR_EYEWEAR_13, IMAGE_ZOMBATAR_EYEWEAR_14, IMAGE_ZOMBATAR_EYEWEAR_15, IMAGE_ZOMBATAR_EYEWEAR_16 };
	static Image* const aFacial[] = { IMAGE_ZOMBATAR_FACIALHAIR_1, IMAGE_ZOMBATAR_FACIALHAIR_2, IMAGE_ZOMBATAR_FACIALHAIR_3, IMAGE_ZOMBATAR_FACIALHAIR_4, IMAGE_ZOMBATAR_FACIALHAIR_5, IMAGE_ZOMBATAR_FACIALHAIR_6, IMAGE_ZOMBATAR_FACIALHAIR_7, IMAGE_ZOMBATAR_FACIALHAIR_8, IMAGE_ZOMBATAR_FACIALHAIR_9, IMAGE_ZOMBATAR_FACIALHAIR_10, IMAGE_ZOMBATAR_FACIALHAIR_11, IMAGE_ZOMBATAR_FACIALHAIR_12, IMAGE_ZOMBATAR_FACIALHAIR_13, IMAGE_ZOMBATAR_FACIALHAIR_14, IMAGE_ZOMBATAR_FACIALHAIR_15, IMAGE_ZOMBATAR_FACIALHAIR_16, IMAGE_ZOMBATAR_FACIALHAIR_17, IMAGE_ZOMBATAR_FACIALHAIR_18, IMAGE_ZOMBATAR_FACIALHAIR_19, IMAGE_ZOMBATAR_FACIALHAIR_20, IMAGE_ZOMBATAR_FACIALHAIR_21, IMAGE_ZOMBATAR_FACIALHAIR_22, IMAGE_ZOMBATAR_FACIALHAIR_23, IMAGE_ZOMBATAR_FACIALHAIR_24 };
	static Image* const aTidbits[] = { IMAGE_ZOMBATAR_TIDBITS_1, IMAGE_ZOMBATAR_TIDBITS_2, IMAGE_ZOMBATAR_TIDBITS_3, IMAGE_ZOMBATAR_TIDBITS_4, IMAGE_ZOMBATAR_TIDBITS_5, IMAGE_ZOMBATAR_TIDBITS_6, IMAGE_ZOMBATAR_TIDBITS_7, IMAGE_ZOMBATAR_TIDBITS_8, IMAGE_ZOMBATAR_TIDBITS_9, IMAGE_ZOMBATAR_TIDBITS_10, IMAGE_ZOMBATAR_TIDBITS_11, IMAGE_ZOMBATAR_TIDBITS_12, IMAGE_ZOMBATAR_TIDBITS_13, IMAGE_ZOMBATAR_TIDBITS_14 };
	static Image* const aAccessory[] = { IMAGE_ZOMBATAR_ACCESSORY_1, IMAGE_ZOMBATAR_ACCESSORY_2, IMAGE_ZOMBATAR_ACCESSORY_3, IMAGE_ZOMBATAR_ACCESSORY_4, IMAGE_ZOMBATAR_ACCESSORY_5, IMAGE_ZOMBATAR_ACCESSORY_6, IMAGE_ZOMBATAR_ACCESSORY_8, IMAGE_ZOMBATAR_ACCESSORY_9, IMAGE_ZOMBATAR_ACCESSORY_10, IMAGE_ZOMBATAR_ACCESSORY_11, IMAGE_ZOMBATAR_ACCESSORY_12, IMAGE_ZOMBATAR_ACCESSORY_13, IMAGE_ZOMBATAR_ACCESSORY_14, IMAGE_ZOMBATAR_ACCESSORY_15, IMAGE_ZOMBATAR_ACCESSORY_16 };

	if (theIndex < 0)
		return nullptr;
	switch (thePage)
	{
	case ZOMBATAR_PAGE_CLOTHES: return theIndex < 12 ? aClothes[theIndex] : nullptr;
	case ZOMBATAR_PAGE_HATS: return theIndex < 14 ? aHats[theIndex] : nullptr;
	case ZOMBATAR_PAGE_HAIR: return theIndex < 16 ? aHair[theIndex] : nullptr;
	case ZOMBATAR_PAGE_EYEWEAR: return theIndex < 16 ? aEyewear[theIndex] : nullptr;
	case ZOMBATAR_PAGE_FACIAL_HAIR: return theIndex < 24 ? aFacial[theIndex] : nullptr;
	case ZOMBATAR_PAGE_TIDBITS: return theIndex < 14 ? aTidbits[theIndex] : nullptr;
	case ZOMBATAR_PAGE_ACCESSORY: return theIndex < 15 ? aAccessory[theIndex] : nullptr;
	default: return nullptr;
	}
}

Image* ZombatarWidget::GetPartMaskImage(ZombatarPage thePage, int theIndex) const
{
	static Image* const aHatsMasks[] =
	{
		IMAGE_ZOMBATAR_HATS_1_MASK, nullptr, IMAGE_ZOMBATAR_HATS_3_MASK, nullptr, nullptr,
		IMAGE_ZOMBATAR_HATS_6_MASK, IMAGE_ZOMBATAR_HATS_7_MASK, IMAGE_ZOMBATAR_HATS_8_MASK,
		IMAGE_ZOMBATAR_HATS_9_MASK, nullptr, IMAGE_ZOMBATAR_HATS_11_MASK, nullptr, nullptr, nullptr
	};
	static Image* const aHairMasks[] =
	{
		IMAGE_ZOMBATAR_HAIR_1_MASK, IMAGE_ZOMBATAR_HAIR_2_MASK, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, IMAGE_ZOMBATAR_HAIR_11_MASK, IMAGE_ZOMBATAR_HAIR_12_MASK,
		IMAGE_ZOMBATAR_HAIR_13_MASK, IMAGE_ZOMBATAR_HAIR_14_MASK, IMAGE_ZOMBATAR_HAIR_15_MASK, nullptr
	};
	static Image* const aEyewearMasks[] =
	{
		IMAGE_ZOMBATAR_EYEWEAR_1_MASK, IMAGE_ZOMBATAR_EYEWEAR_2_MASK, IMAGE_ZOMBATAR_EYEWEAR_3_MASK,
		IMAGE_ZOMBATAR_EYEWEAR_4_MASK, IMAGE_ZOMBATAR_EYEWEAR_5_MASK, IMAGE_ZOMBATAR_EYEWEAR_6_MASK,
		IMAGE_ZOMBATAR_EYEWEAR_7_MASK, IMAGE_ZOMBATAR_EYEWEAR_8_MASK, IMAGE_ZOMBATAR_EYEWEAR_9_MASK,
		IMAGE_ZOMBATAR_EYEWEAR_10_MASK, IMAGE_ZOMBATAR_EYEWEAR_11_MASK, IMAGE_ZOMBATAR_EYEWEAR_12_MASK,
		nullptr, nullptr, nullptr, nullptr
	};
	static Image* const aFacialMasks[] =
	{
		IMAGE_ZOMBATAR_FACIALHAIR_1_MASK, nullptr, nullptr, IMAGE_ZOMBATAR_FACIALHAIR_4_MASK,
		nullptr, nullptr, nullptr, IMAGE_ZOMBATAR_FACIALHAIR_8_MASK, IMAGE_ZOMBATAR_FACIALHAIR_9_MASK,
		IMAGE_ZOMBATAR_FACIALHAIR_10_MASK, IMAGE_ZOMBATAR_FACIALHAIR_11_MASK, IMAGE_ZOMBATAR_FACIALHAIR_12_MASK,
		nullptr, IMAGE_ZOMBATAR_FACIALHAIR_14_MASK, IMAGE_ZOMBATAR_FACIALHAIR_15_MASK, IMAGE_ZOMBATAR_FACIALHAIR_16_MASK,
		nullptr, IMAGE_ZOMBATAR_FACIALHAIR_18_MASK, nullptr, nullptr, IMAGE_ZOMBATAR_FACIALHAIR_21_MASK,
		IMAGE_ZOMBATAR_FACIALHAIR_22_MASK, IMAGE_ZOMBATAR_FACIALHAIR_23_MASK, IMAGE_ZOMBATAR_FACIALHAIR_24_MASK
	};

	if (theIndex < 0)
		return nullptr;
	switch (thePage)
	{
	case ZOMBATAR_PAGE_HATS: return theIndex < 14 ? aHatsMasks[theIndex] : nullptr;
	case ZOMBATAR_PAGE_HAIR: return theIndex < 16 ? aHairMasks[theIndex] : nullptr;
	case ZOMBATAR_PAGE_EYEWEAR: return theIndex < 16 ? aEyewearMasks[theIndex] : nullptr;
	case ZOMBATAR_PAGE_FACIAL_HAIR: return theIndex < 24 ? aFacialMasks[theIndex] : nullptr;
	default: return nullptr;
	}
}

void ZombatarWidget::DrawImageColorized(Graphics* g, Image* theImage, int theX, int theY, int theColorIndex)
{
	if (!theImage)
		return;

	g->SetColorizeImages(true);
	g->SetColor(ZombatarGetColor(theColorIndex));
	g->DrawImage(theImage, theX, theY);
	g->SetColorizeImages(false);
	g->SetColor(Color::White);
}

void ZombatarWidget::DrawPartImage(Graphics* g, ZombatarPage thePage, int theIndex, int theX, int theY, int theColorIndex)
{
	Image* aImage = GetPartImage(thePage, theIndex);
	Image* aMask = GetPartMaskImage(thePage, theIndex);
	if (!aImage)
		return;

	const ZombatarPartLayout* aLayout = GetPartLayout(thePage, theIndex);
	int aPosX = theX + (aLayout ? aLayout->mOffsetX : 0);
	int aPosY = theY + (aLayout ? aLayout->mOffsetY : 0);

	if (GetPartColorMode(thePage, theIndex) == ZOMBATAR_COLOR_MODE_NONE)
	{
		g->DrawImage(aImage, aPosX, aPosY);
		return;
	}

	if (aMask)
	{
		int aColorOffX = aLayout ? aLayout->mColorOffsetX : 0;
		int aColorOffY = aLayout ? aLayout->mColorOffsetY : 0;
		DrawImageColorized(g, aMask, aPosX + aColorOffX, aPosY + aColorOffY, theColorIndex);
		g->DrawImage(aImage, aPosX, aPosY);
	}
	else
	{
		DrawImageColorized(g, aImage, aPosX, aPosY, theColorIndex);
	}
}

void ZombatarWidget::DrawAvatar(Graphics* g, int theX, int theY, const unsigned char* theRecord)
{
	int aPart[NUM_ZOMBATAR_PAGES];
	int aColor[NUM_ZOMBATAR_PAGES];
	DecodeRecord(theRecord, aPart, aColor);

	Image* aBackground = GetBackgroundImage(aPart[ZOMBATAR_PAGE_BACKDROPS]);
	if (GetPartColorMode(ZOMBATAR_PAGE_BACKDROPS, aPart[ZOMBATAR_PAGE_BACKDROPS]) == ZOMBATAR_COLOR_MODE_2)
		DrawImageColorized(g, aBackground, theX, theY, aColor[ZOMBATAR_PAGE_BACKDROPS]);
	else
		g->DrawImage(aBackground, theX, theY);

	int aBlankX = theX;
	int aBlankY = theY;
	if (IMAGE_ZOMBATAR_BACKGROUND_BLANK && IMAGE_ZOMBATAR_ZOMBIE_BLANK)
	{
		aBlankX = theX + (IMAGE_ZOMBATAR_BACKGROUND_BLANK->mWidth - IMAGE_ZOMBATAR_ZOMBIE_BLANK->mWidth);
		aBlankY = theY + (IMAGE_ZOMBATAR_BACKGROUND_BLANK->mHeight - IMAGE_ZOMBATAR_ZOMBIE_BLANK->mHeight);
	}
	DrawImageColorized(g, IMAGE_ZOMBATAR_ZOMBIE_BLANK_SKIN, aBlankX, aBlankY, aColor[ZOMBATAR_PAGE_SKIN]);
	g->DrawImage(IMAGE_ZOMBATAR_ZOMBIE_BLANK, aBlankX, aBlankY);

	ZombatarDrawPart aParts[7];
	int aNumParts = 0;
	for (int i = ZOMBATAR_PAGE_HAIR; i < ZOMBATAR_PAGE_BACKDROPS; i++)
	{
		ZombatarPage aPage = static_cast<ZombatarPage>(i);
		int aIndex = aPart[i];
		if (aIndex >= 0)
		{
			const ZombatarPartLayout* aLayout = GetPartLayout(aPage, aIndex);
			aParts[aNumParts].mPage = aPage;
			aParts[aNumParts].mIndex = aIndex;
			aParts[aNumParts].mColor = aColor[i];
			aParts[aNumParts].mDrawOrder = aLayout ? aLayout->mDrawOrder : 0;
			aNumParts++;
		}
	}
	std::stable_sort(aParts, aParts + aNumParts);
	for (int i = 0; i < aNumParts; i++)
		DrawPartImage(g, aParts[i].mPage, aParts[i].mIndex, theX, theY, aParts[i].mColor);
}

void ZombatarWidget::DrawDraftAvatar(Graphics* g, int theX, int theY)
{
	unsigned char aRecord[ZOMBATAR_RECORD_SIZE];
	EncodeRecord(aRecord);
	DrawAvatar(g, theX, theY, aRecord);
}

void ZombatarWidget::DrawColorSwatches(Graphics* g, int thePaletteBase, int theCount, int theSavedColor)
{
	g->SetColorizeImages(true);
	for (int i = 0; i < theCount; i++)
	{
		int aPalette = thePaletteBase + i;
		int aAlpha = 0x40;
		if (aPalette == theSavedColor)
			aAlpha = 0xff;
		else if (i == mHoverColorCell)
			aAlpha = 0x80;
		Color aColor = ZombatarGetColor(aPalette);
		g->SetColor(Color(aColor.mRed, aColor.mGreen, aColor.mBlue, aAlpha));
		Image* aImage = (aPalette == ZOMBATAR_PART_COLOR_NONE_1 || aPalette == ZOMBATAR_PART_COLOR_NONE_2) ? IMAGE_ZOMBATAR_COLORPICKER_NONE : IMAGE_ZOMBATAR_COLORPICKER;
		Rect aRect = GetColorRect(i);
		g->DrawImage(aImage, aRect.mX, aRect.mY);
	}
	g->SetColorizeImages(false);
	g->SetColor(Color::White);
}

void ZombatarWidget::CreatePreviewZombie()
{
	if (mPreviewZombie)
		return;

	mPreviewZombie = new Zombie();
	mPreviewZombie->mApp = mApp;
	mPreviewZombie->mBoard = nullptr;
	mPreviewZombie->ZombieInitialize(0, ZombieType::ZOMBIE_FLAG, false, nullptr, Zombie::ZOMBIE_WAVE_UI);
	mPreviewZombie->mPosX = 0.0f;
	mPreviewZombie->mPosY = 0.0f;
	mPreviewZombie->mX = 0;
	mPreviewZombie->mY = 0;
}

void ZombatarWidget::DestroyPreviewZombie()
{
	if (!mPreviewZombie)
		return;

	mPreviewZombie->DieNoLoot();
	delete mPreviewZombie;
	mPreviewZombie = nullptr;
}

void ZombatarWidget::DrawAvatarBox(Graphics* g)
{
	if (!mPreviewZombie)
		return;

	const unsigned char* aRecord = nullptr;
	unsigned char aDraft[ZOMBATAR_RECORD_SIZE];
	if (mState == ZOMBATAR_STATE_LIST)
	{
		if (GetHeadCount() <= 0)
			return;
		ClampCurrentIndex();
		aRecord = mApp->mPlayerInfo->mZombatarData.data() + mCurrentIndex * ZOMBATAR_RECORD_SIZE;
	}
	else if (mState == ZOMBATAR_STATE_CREATE || mState == ZOMBATAR_STATE_CONFIRM)
	{
		EncodeRecord(aDraft);
		aRecord = aDraft;
	}
	else
	{
		return;
	}

	mPreviewZombie->ApplyZombatarHead(aRecord);

	Image* aGround = IMAGE_ALMANAC_GROUNDDAY;
	if (aGround)
	{
		g->SetClipRect(ZOMBATAR_AVATAR_GROUND_X, ZOMBATAR_AVATAR_GROUND_Y + ZOMBATAR_AVATAR_GROUND_CLIP_TOP,
			aGround->mWidth - ZOMBATAR_AVATAR_GROUND_CLIP_RIGHT,
			aGround->mHeight - ZOMBATAR_AVATAR_GROUND_CLIP_TOP - ZOMBATAR_AVATAR_GROUND_CLIP_BOTTOM);
		g->DrawImage(aGround, ZOMBATAR_AVATAR_GROUND_X, ZOMBATAR_AVATAR_GROUND_Y);
		g->ClearClipRect();
	}

	mPreviewZombie->mPosX = static_cast<float>(ZOMBATAR_AVATAR_ZOMBIE_X);
	mPreviewZombie->mPosY = static_cast<float>(ZOMBATAR_AVATAR_ZOMBIE_Y);
	mPreviewZombie->mX = ZOMBATAR_AVATAR_ZOMBIE_X;
	mPreviewZombie->mY = ZOMBATAR_AVATAR_ZOMBIE_Y;

	Graphics aZombieGraphics = Graphics(*g);
	mPreviewZombie->BeginDraw(&aZombieGraphics);
	mPreviewZombie->Draw(&aZombieGraphics);
}

bool ZombatarWidget::ExportAvatarPNG(const unsigned char* theRecord, int theExportIndex)
{
	PlayerInfo* aPlayerInfo = mApp->mPlayerInfo;
	if (!aPlayerInfo)
		return false;

	Image* aBackground = GetBackgroundImage(ZombatarReadSignedRecordSlot(theRecord, ZOMBATAR_SLOT_BACKGROUND));
	if (!aBackground)
		return false;

	MemoryImage aMemoryImage(mApp);
	aMemoryImage.Create(aBackground->mWidth, aBackground->mHeight);
	uint32_t* aMemoryBits = aMemoryImage.GetBits();
	memset(aMemoryBits, 0, sizeof(uint32_t) * aBackground->mWidth * aBackground->mHeight);

	Graphics aGraphics(&aMemoryImage);
	DrawAvatar(&aGraphics, 0, 0, theRecord);
	aMemoryImage.CommitBits();

	ImageLib::Image aExportImage;
	aExportImage.mWidth = aMemoryImage.mWidth;
	aExportImage.mHeight = aMemoryImage.mHeight;
	aExportImage.mBits = new uint32_t[aExportImage.mWidth * aExportImage.mHeight];

	uint32_t* aSourceBits = aMemoryImage.GetBits();
	int aPixelCount = aExportImage.mWidth * aExportImage.mHeight;
	for (int i = 0; i < aPixelCount; i++)
		aExportImage.mBits[i] = Sexy::ToLE32(aSourceBits[i]);

	MkDir(GetAppDataPath("userdata"));
	return ImageLib::WritePNGImage(GetAppDataPath(StrFormat("userdata/Zombatar_%d_%d.png", aPlayerInfo->mId, theExportIndex)), &aExportImage);
}

bool ZombatarWidget::ExportAllAvatarPNGs()
{
	PlayerInfo* aPlayerInfo = mApp->mPlayerInfo;
	if (!aPlayerInfo)
		return false;

	int aCount = GetHeadCount();
	for (int i = 0; i < aCount; i++)
	{
		const unsigned char* aRecord = aPlayerInfo->mZombatarData.data() + static_cast<size_t>(i) * ZOMBATAR_RECORD_SIZE;
		if (!ExportAvatarPNG(aRecord, i + 1))
			return false;
	}
	return true;
}

void ZombatarWidget::EraseAvatarPNG(int theExportIndex)
{
	if (theExportIndex <= 0)
		return;
	PlayerInfo* aPlayerInfo = mApp->mPlayerInfo;
	if (!aPlayerInfo)
		return;
	mApp->EraseFile(GetAppDataPath(StrFormat("userdata/Zombatar_%d_%d.png", aPlayerInfo->mId, theExportIndex)));
}

void ZombatarWidget::Draw(Graphics* g)
{
	if (!mApp->mPlayerInfo)
		return;

	DrawMain(g);
	DrawAvatarBox(g);
	if (mState != ZOMBATAR_STATE_LIST)
		DrawDraftAvatar(g, ZOMBATAR_PREVIEW_X, ZOMBATAR_PREVIEW_Y);
	if (mState == ZOMBATAR_STATE_LIST)
		DrawList(g);
	else if (mState == ZOMBATAR_STATE_CONFIRM)
		DrawConfirm(g);
	else
		DrawCreate(g);

	if (IMAGE_ZOMBATAR_DISPLAY_WINDOW)
		g->DrawImage(IMAGE_ZOMBATAR_DISPLAY_WINDOW, 5, 0);
}

void ZombatarWidget::DrawMain(Graphics* g)
{
	g->DrawImage(IMAGE_ZOMBATAR_MAIN_BG, 0, 0);
}

void ZombatarWidget::DrawList(Graphics* g)
{
	int aCount = GetHeadCount();
	ClampCurrentIndex();
	if (aCount <= 0)
		return;

	if (mCurrentIndex > 0)
		DrawAvatar(g, ZOMBATAR_LIST_PREV_X, ZOMBATAR_LIST_PORTRAIT_Y, mApp->mPlayerInfo->mZombatarData.data() + (mCurrentIndex - 1) * ZOMBATAR_RECORD_SIZE);
	DrawAvatar(g, ZOMBATAR_LIST_CUR_X, ZOMBATAR_LIST_PORTRAIT_Y, mApp->mPlayerInfo->mZombatarData.data() + mCurrentIndex * ZOMBATAR_RECORD_SIZE);
	if (mCurrentIndex + 1 < aCount)
		DrawAvatar(g, ZOMBATAR_LIST_NEXT_X, ZOMBATAR_LIST_PORTRAIT_Y, mApp->mPlayerInfo->mZombatarData.data() + (mCurrentIndex + 1) * ZOMBATAR_RECORD_SIZE);

	DrawAvatar(g, ZOMBATAR_PREVIEW_X, ZOMBATAR_PREVIEW_Y, mApp->mPlayerInfo->mZombatarData.data() + mCurrentIndex * ZOMBATAR_RECORD_SIZE);

	g->SetFont(FONT_BRIANNETOD12);
	g->SetColor(Color(255, 255, 255));
	g->DrawString(Sexy::StrFormat("%d / %d", mCurrentIndex + 1, aCount), ZOMBATAR_LIST_COUNTER_X, ZOMBATAR_LIST_COUNTER_Y);

	g->SetFont(FONT_DWARVENTODCRAFT12);
	g->SetColor(mDeleteHover ? Color(22, 253, 5) : Color(255, 255, 255));
	g->DrawString("Delete?", ZOMBATAR_LIST_DELETE_X, ZOMBATAR_LIST_DELETE_Y);
	g->SetColor(Color::White);
}

void ZombatarWidget::DrawCreate(Graphics* g)
{
	g->DrawImage(IMAGE_ZOMBATAR_WIDGET_BG, ZOMBATAR_PANEL_X, ZOMBATAR_PANEL_Y);
	g->DrawImage(IMAGE_ZOMBATAR_WIDGET_INNER_BG, ZOMBATAR_INNER_X, ZOMBATAR_INNER_Y);

	for (int i = 0; i < NUM_ZOMBATAR_PAGES; i++)
	{
		Rect aRect = GetCategoryRect(i);
		g->DrawImage(GetCategoryImage(static_cast<ZombatarPage>(i), i == mPage, aRect.Contains(mMouseX, mMouseY)), aRect.mX, aRect.mY);
	}

	if (mPage == ZOMBATAR_PAGE_SKIN)
	{
		int aPanelW = IMAGE_ZOMBATAR_WIDGET_BG ? IMAGE_ZOMBATAR_WIDGET_BG->mWidth : ZOMBATAR_PANEL_W;
		TodDrawStringWrapped(g, "[ZOMBATAR_START_TEXT]",
			Rect(ZOMBATAR_PANEL_X + aPanelW / 2 - 200, ZOMBATAR_START_TEXT_Y, ZOMBATAR_START_TEXT_W, ZOMBATAR_START_TEXT_H),
			FONT_DWARVENTODCRAFT15, Color(254, 227, 0, 175), DS_ALIGN_CENTER);
		g->SetColor(Color::White);
	}

	int aItemCount = GetItemCountForPage();
	int aBaseIndex = mSubPage * ZOMBATAR_GRID_PAGE;
	for (int i = 0; i < aItemCount; i++)
	{
		Rect aRect = GetItemRect(i);
		int aPartIndex = aBaseIndex + i;
		bool aSelected = mPart[mPage] == aPartIndex;
		bool aHover = i == mHoverGridCell;
		bool aDim = !aSelected && !aHover;

		if (aDim)
		{
			g->SetColorizeImages(true);
			g->SetColor(ZOMBATAR_CELL_DIM_COLOR);
		}
		g->DrawImage(aHover ? IMAGE_ZOMBATAR_ACCESSORY_BG_HIGHLIGHT : IMAGE_ZOMBATAR_ACCESSORY_BG, aRect.mX, aRect.mY);

		if (mPage == ZOMBATAR_PAGE_CLOTHES && IMAGE_ZOMBATAR_ZOMBIE_BLANK_SKIN && IMAGE_ZOMBATAR_ZOMBIE_BLANK)
		{
			g->PushState();
			g->ClipRect(Rect(aRect.mX + ZOMBATAR_CELL_INSET, aRect.mY + ZOMBATAR_CELL_INSET,
				aRect.mWidth - ZOMBATAR_CELL_INSET, aRect.mHeight - ZOMBATAR_CELL_INSET));
			Color aSkinColor = ZombatarGetColor(mColor[ZOMBATAR_PAGE_SKIN]);
			aSkinColor.mAlpha = aDim ? 0x80 : 0xff;
			g->SetColorizeImages(true);
			g->SetColor(aSkinColor);
			g->DrawImage(IMAGE_ZOMBATAR_ZOMBIE_BLANK_SKIN,
				Rect(aRect.mX - ZOMBATAR_CELL_ZOMBIE_MARGIN, aRect.mY - ZOMBATAR_CELL_ZOMBIE_MARGIN, aRect.mWidth, aRect.mHeight),
				Rect(0, 0, IMAGE_ZOMBATAR_ZOMBIE_BLANK_SKIN->mWidth, IMAGE_ZOMBATAR_ZOMBIE_BLANK_SKIN->mHeight));
			g->SetColorizeImages(false);
			g->DrawImage(IMAGE_ZOMBATAR_ZOMBIE_BLANK,
				Rect(aRect.mX - ZOMBATAR_CELL_ZOMBIE_MARGIN, aRect.mY - ZOMBATAR_CELL_ZOMBIE_MARGIN, aRect.mWidth, aRect.mHeight),
				Rect(0, 0, IMAGE_ZOMBATAR_ZOMBIE_BLANK->mWidth, IMAGE_ZOMBATAR_ZOMBIE_BLANK->mHeight));
			g->PopState();
			if (aDim)
			{
				g->SetColorizeImages(true);
				g->SetColor(ZOMBATAR_CELL_DIM_COLOR);
			}
		}

		Image* aImage = (mPage == ZOMBATAR_PAGE_BACKDROPS) ? GetBackgroundImage(aPartIndex) : GetPartImage(mPage, aPartIndex);
		if (aImage)
		{
			Rect aIconRect = FitIconRect(aImage, aRect);
			Image* aMask = GetPartMaskImage(mPage, aPartIndex);
			if (aMask)
			{
				const ZombatarPartLayout* aLayout = GetPartLayout(mPage, aPartIndex);
				float aScaleX = (float)aIconRect.mWidth / (float)aImage->mWidth;
				float aScaleY = (float)aIconRect.mHeight / (float)aImage->mHeight;
				int aMaskX = aIconRect.mX + (aLayout ? (int)(aLayout->mColorOffsetX * aScaleX) : 0);
				int aMaskY = aIconRect.mY + (aLayout ? (int)(aLayout->mColorOffsetY * aScaleY) : 0);
				g->DrawImage(aMask, Rect(aMaskX, aMaskY, (int)(aMask->mWidth * aScaleX), (int)(aMask->mHeight * aScaleY)), Rect(0, 0, aMask->mWidth, aMask->mHeight));
			}
			g->DrawImage(aImage, aIconRect, Rect(0, 0, aImage->mWidth, aImage->mHeight));
		}

		if (aDim)
		{
			g->SetColorizeImages(false);
			g->SetColor(Color::White);
		}
	}

	if (PageAllowsNone())
	{
		Rect aRect = GetItemRect(aItemCount);
		bool aSelected = mPart[mPage] < 0;
		bool aHover = mHoverGridCell == aItemCount;
		bool aDim = !aSelected && !aHover;
		if (aDim)
		{
			g->SetColorizeImages(true);
			g->SetColor(ZOMBATAR_CELL_DIM_COLOR);
		}
		g->DrawImage(aHover ? IMAGE_ZOMBATAR_ACCESSORY_BG_HIGHLIGHT : IMAGE_ZOMBATAR_ACCESSORY_BG, aRect.mX, aRect.mY);
		g->DrawImage(IMAGE_ZOMBATAR_ACCESSORY_BG_NONE, aRect.mX, aRect.mY);
		if (aDim)
		{
			g->SetColorizeImages(false);
			g->SetColor(Color::White);
		}
	}

	g->DrawImage(IMAGE_ZOMBATAR_COLORS_BG, ZOMBATAR_COLORS_X, ZOMBATAR_COLORS_Y);
	if (mPage == ZOMBATAR_PAGE_SKIN)
	{
		DrawColorSwatches(g, 0, ZOMBATAR_SKIN_COLOR_COUNT, mColor[ZOMBATAR_PAGE_SKIN]);
	}
	else
	{
		int aMode = mPart[mPage] < 0 ? ZOMBATAR_COLOR_MODE_NONE : GetPartColorMode(mPage, mPart[mPage]);
		if (aMode == ZOMBATAR_COLOR_MODE_NONE)
		{
			const char* aKey = mPart[mPage] < 0 ? "[ZOMBATAR_COLOR_ITEM_NOT_CHOSEN]" : "[ZOMBATAR_COLOR_NOT_APPLICABLE]";
			int aColorsW = IMAGE_ZOMBATAR_COLORS_BG ? IMAGE_ZOMBATAR_COLORS_BG->mWidth : 261;
			int aColorsH = IMAGE_ZOMBATAR_COLORS_BG ? IMAGE_ZOMBATAR_COLORS_BG->mHeight : 96;
			TodDrawStringWrapped(g, aKey, Rect(ZOMBATAR_COLOR_HINT_X, ZOMBATAR_COLOR_HINT_Y, aColorsW - 40, aColorsH),
				FONT_BRIANNETOD12, Color::White, DS_ALIGN_LEFT);
		}
		else
		{
			DrawColorSwatches(g, ZombatarColorBaseForMode(aMode), ZOMBATAR_PART_COLOR_COUNT, mColor[mPage]);
		}
	}

	if (mMaxSubPages > 0)
	{
		g->SetFont(FONT_DWARVENTODCRAFT12);
		g->SetColor(Color(255, 255, 255));
		std::string aPage = Sexy::StrFormat("Page %d / %d", mSubPage + 1, mMaxSubPages + 1);
		g->DrawString(aPage, 321, 441);
	}
}

void ZombatarWidget::DrawConfirm(Graphics* g)
{
	g->DrawImage(IMAGE_ZOMBATAR_WIDGET_BG, ZOMBATAR_PANEL_X, ZOMBATAR_PANEL_Y);

	g->SetFont(FONT_HOUSEOFTERROR28);
	g->SetColor(Color(254, 227, 0));
	std::string aHeader = TodStringTranslate("[ZOMBATAR_FINISHED_WARNING_HEADER]");
	g->DrawString(aHeader, ZOMBATAR_CONFIRM_HEADER_X - FONT_HOUSEOFTERROR28->StringWidth(aHeader) / 2, ZOMBATAR_CONFIRM_HEADER_Y);

	TodDrawStringWrapped(g, "[ZOMBATAR_FINISHED_WARNING_TEXT]",
		Rect(ZOMBATAR_CONFIRM_TEXT_X, ZOMBATAR_CONFIRM_TEXT_Y, ZOMBATAR_CONFIRM_TEXT_W, ZOMBATAR_CONFIRM_TEXT_H),
		FONT_CONTINUUMBOLD14, Color::White, DS_ALIGN_CENTER);

	g->SetFont(FONT_BRIANNETOD12);
	g->SetColor(Color(255, 255, 255));
	std::string aAccept = TodStringTranslate("[ZOMBATAR_FINISHED_BUTTON_TEXT]");
	g->DrawString(aAccept, ZOMBATAR_CONFIRM_ACCEPT_LABEL_X - FONT_BRIANNETOD12->StringWidth(aAccept) / 2, ZOMBATAR_CONFIRM_LABEL_Y);
	std::string aBack = TodStringTranslate("[ZOMBATAR_BACK_BUTTON_TEXT]");
	g->DrawString(aBack, ZOMBATAR_CONFIRM_BACK_LABEL_X - FONT_BRIANNETOD12->StringWidth(aBack) / 2, ZOMBATAR_CONFIRM_LABEL_Y);
}

void ZombatarWidget::ChangeState(ZombatarWidgetState theState)
{
	mState = theState;
	UpdateButtonState();
}

void ZombatarWidget::ChangePage(ZombatarPage thePage)
{
	mPage = thePage;
	mSubPage = 0;
	UpdateButtonState();
}

void ZombatarWidget::UpdateButtonState()
{
	int aCount = GetHeadCount();
	bool aList = mState == ZOMBATAR_STATE_LIST;
	bool aCreate = mState == ZOMBATAR_STATE_CREATE;
	bool aConfirm = mState == ZOMBATAR_STATE_CONFIRM;

	int aTotal = GetTotalItemsForPage(mPage);
	mMaxSubPages = (aTotal > ZOMBATAR_GRID_PAGE) ? (aTotal - 1) / ZOMBATAR_GRID_PAGE : 0;
	mSubPage = ClampRange(mSubPage, 0, mMaxSubPages);
	bool aPaged = aCreate && mMaxSubPages > 0;

	mBackButton->SetVisible(aCreate || aList || aConfirm);
	mConfirmBackButton->SetVisible(aConfirm);
	mViewButton->SetVisible(aCreate && aCount > 0);
	mFinishedButton->SetVisible(aCreate || aConfirm);
	mNewButton->SetVisible(aList);
	mPrevPortraitButton->SetVisible(aList && aCount > 1);
	mNextPortraitButton->SetVisible(aList && aCount > 1);
	mPrevPageButton->SetVisible(aPaged);
	mNextPageButton->SetVisible(aPaged);

	mNewButton->mDisabled = !CanSaveNewHead();
	mPrevPortraitButton->mDisabled = mCurrentIndex <= 0;
	mNextPortraitButton->mDisabled = mCurrentIndex + 1 >= aCount;
	mPrevPageButton->mDisabled = mSubPage <= 0;
	mNextPageButton->mDisabled = mSubPage >= mMaxSubPages;
	mFinishedButton->mDisabled = aCreate && !CanSaveNewHead();

	if (aConfirm)
		mFinishedButton->Resize(ZOMBATAR_ACCEPT_X, ZOMBATAR_CONFIRM_BTN_Y, 103, 26);
	else
		mFinishedButton->Resize(ZOMBATAR_FINISHED_X, ZOMBATAR_FINISHED_Y, 103, 26);
}

void ZombatarWidget::MouseMove(int x, int y)
{
	mMouseX = x;
	mMouseY = y;
	mHoverGridCell = -1;
	mHoverColorCell = -1;

	if (mState == ZOMBATAR_STATE_LIST)
	{
		mDeleteHover = Rect(ZOMBATAR_LIST_DELETE_RECT_X, ZOMBATAR_LIST_DELETE_RECT_Y, ZOMBATAR_LIST_DELETE_RECT_W, ZOMBATAR_LIST_DELETE_RECT_H).Contains(x, y);
		return;
	}

	if (mState != ZOMBATAR_STATE_CREATE)
		return;

	int aItemCount = GetItemCountForPage();
	for (int i = 0; i < aItemCount; i++)
	{
		if (GetItemHitRect(i).Contains(x, y))
		{
			mHoverGridCell = i;
			break;
		}
	}
	if (mHoverGridCell < 0 && PageAllowsNone() && GetItemHitRect(aItemCount).Contains(x, y))
		mHoverGridCell = aItemCount;

	if (PageAllowsColors())
	{
		int aColorCount = mPage == ZOMBATAR_PAGE_SKIN ? ZOMBATAR_SKIN_COLOR_COUNT : ZOMBATAR_PART_COLOR_COUNT;
		for (int i = 0; i < aColorCount; i++)
		{
			if (GetColorRect(i).Contains(x, y))
			{
				mHoverColorCell = i;
				break;
			}
		}
	}
}

void ZombatarWidget::HandleGridClick(int x, int y)
{
	if (mState != ZOMBATAR_STATE_CREATE)
		return;

	int aItemCount = GetItemCountForPage();
	int aBaseIndex = mSubPage * ZOMBATAR_GRID_PAGE;
	for (int i = 0; i < aItemCount; i++)
	{
		if (GetItemHitRect(i).Contains(x, y))
		{
			mPart[mPage] = aBaseIndex + i;
			UpdateButtonState();
			return;
		}
	}

	if (PageAllowsNone() && GetItemHitRect(aItemCount).Contains(x, y))
	{
		mPart[mPage] = -1;
		UpdateButtonState();
	}
}

void ZombatarWidget::HandleColorClick(int x, int y)
{
	if (mState != ZOMBATAR_STATE_CREATE || !PageAllowsColors())
		return;

	if (mPage == ZOMBATAR_PAGE_SKIN)
	{
		for (int i = 0; i < ZOMBATAR_SKIN_COLOR_COUNT; i++)
		{
			if (GetColorRect(i).Contains(x, y))
			{
				mColor[ZOMBATAR_PAGE_SKIN] = i;
				return;
			}
		}
		return;
	}

	int aMode = GetPartColorMode(mPage, mPart[mPage]);
	int aBase = ZombatarColorBaseForMode(aMode);
	for (int i = 0; i < ZOMBATAR_PART_COLOR_COUNT; i++)
	{
		if (GetColorRect(i).Contains(x, y))
		{
			mColor[mPage] = aBase + i;
			return;
		}
	}
}

void ZombatarWidget::MouseUp(int x, int y)
{
	if (!mApp->mPlayerInfo)
		return;

	if (mState == ZOMBATAR_STATE_CREATE)
	{
		for (int i = 0; i < NUM_ZOMBATAR_PAGES; i++)
		{
			if (GetCategoryRect(i).Contains(x, y))
			{
				ChangePage(static_cast<ZombatarPage>(i));
				return;
			}
		}

		HandleGridClick(x, y);
		HandleColorClick(x, y);
	}
	else if (mState == ZOMBATAR_STATE_LIST)
	{
		if (GetHeadCount() > 0 && Rect(ZOMBATAR_LIST_DELETE_RECT_X, ZOMBATAR_LIST_DELETE_RECT_Y, ZOMBATAR_LIST_DELETE_RECT_W, ZOMBATAR_LIST_DELETE_RECT_H).Contains(x, y))
		{
			int aResult = mApp->LawnMessageBox(
				DIALOG_ZOMBATAR_DELETE,
				"[ZOMBATAR_DELETE_HEADER]",
				"[ZOMBATAR_DELETE_BODY]",
				"[ZOMBATAR_DELETE_BUTTON]",
				"[DIALOG_BUTTON_NO]",
				Dialog::BUTTONS_YES_NO);
			if (aResult == Dialog::ID_YES)
				DeleteCurrent();
		}
	}
}

void ZombatarWidget::ButtonPress(int theId)
{
	(void)theId;
	mApp->PlaySample(SOUND_GRAVEBUTTON);
}

void ZombatarWidget::ButtonDepress(int theId)
{
	if (!mApp->mPlayerInfo)
		return;

	switch (theId)
	{
	case ZOMBATAR_BTN_BACK:
		BackToSelector();
		break;

	case ZOMBATAR_BTN_CONFIRM_BACK:
		ChangeState(ZOMBATAR_STATE_CREATE);
		break;

	case ZOMBATAR_BTN_VIEW:
		ResetDraft();
		ChangeState(ZOMBATAR_STATE_LIST);
		break;

	case ZOMBATAR_BTN_FINISHED:
		if (mState == ZOMBATAR_STATE_CREATE)
		{
			if (CanSaveNewHead())
				ChangeState(ZOMBATAR_STATE_CONFIRM);
			else
				ShowMaxHeadsMessage();
		}
		else if (mState == ZOMBATAR_STATE_CONFIRM)
		{
			if (SaveDraft())
				ChangeState(ZOMBATAR_STATE_LIST);
		}
		break;

	case ZOMBATAR_BTN_NEW:
		if (CanSaveNewHead())
		{
			ResetDraft();
			mPage = ZOMBATAR_PAGE_SKIN;
			mSubPage = 0;
			ChangeState(ZOMBATAR_STATE_CREATE);
		}
		else
		{
			ShowMaxHeadsMessage();
		}
		break;

	case ZOMBATAR_BTN_PREV_PORTRAIT:
		if (mCurrentIndex > 0)
		{
			mCurrentIndex--;
			LoadCurrentToDraft();
			UpdateButtonState();
		}
		break;

	case ZOMBATAR_BTN_NEXT_PORTRAIT:
		if (mCurrentIndex + 1 < GetHeadCount())
		{
			mCurrentIndex++;
			LoadCurrentToDraft();
			UpdateButtonState();
		}
		break;

	case ZOMBATAR_BTN_PREV_PAGE:
		if (mSubPage > 0)
		{
			mSubPage--;
			UpdateButtonState();
		}
		break;

	case ZOMBATAR_BTN_NEXT_PAGE:
		if (mSubPage < mMaxSubPages)
		{
			mSubPage++;
			UpdateButtonState();
		}
		break;
	}
}

void ZombatarWidget::KeyDown(KeyCode theKey)
{
	if (theKey == KEYCODE_ESCAPE)
	{
		if (mState == ZOMBATAR_STATE_CONFIRM)
			ChangeState(ZOMBATAR_STATE_CREATE);
		else
			BackToSelector();
	}
}

void ZombatarWidget::BackToSelector()
{
	mPage = ZOMBATAR_PAGE_SKIN;
	mSubPage = 0;
	ChangeState(GetHeadCount() > 0 ? ZOMBATAR_STATE_LIST : ZOMBATAR_STATE_CREATE);
	ResetDraft();
	mGameSelector->SlideTo(0, 0);
	if (mWidgetManager)
		mWidgetManager->SetFocus(mGameSelector);
}
