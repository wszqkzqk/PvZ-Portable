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
constexpr int ZOMBATAR_PART_COLOR_BASE = 12;
constexpr int ZOMBATAR_PART_COLOR_NONE_SWATCH = 17;

constexpr int ZOMBATAR_PANEL_X = 25;
constexpr int ZOMBATAR_PANEL_Y = 25;
constexpr int ZOMBATAR_PANEL_W = 560;
constexpr int ZOMBATAR_INNER_X = 152;
constexpr int ZOMBATAR_INNER_Y = 125;
constexpr int ZOMBATAR_COLORS_X = 196;
constexpr int ZOMBATAR_COLORS_Y = 310;
constexpr int ZOMBATAR_PREVIEW_X = 592;
constexpr int ZOMBATAR_PREVIEW_Y = 115;

constexpr int ZOMBATAR_TABS_X = 58;
constexpr int ZOMBATAR_TABS_Y0 = 128;

constexpr int ZOMBATAR_GRID_COLS = 6;
constexpr int ZOMBATAR_GRID_ROWS = 3;
constexpr int ZOMBATAR_GRID_PAGE = ZOMBATAR_GRID_COLS * ZOMBATAR_GRID_ROWS;
constexpr int ZOMBATAR_GRID_GAP = -4;
constexpr int ZOMBATAR_GRID_BIAS_X = 50;

constexpr int ZOMBATAR_COLOR_COLS = 9;
constexpr int ZOMBATAR_COLOR_GAP = 4;
constexpr int ZOMBATAR_COLOR_X = 213;
constexpr int ZOMBATAR_COLOR_Y = 342;

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

constexpr int ZOMBATAR_CONFIRM_HEADER_X = 305;
constexpr int ZOMBATAR_CONFIRM_HEADER_Y = 185;
constexpr int ZOMBATAR_CONFIRM_TEXT_X = 60;
constexpr int ZOMBATAR_CONFIRM_TEXT_Y = 225;
constexpr int ZOMBATAR_CONFIRM_TEXT_W = 500;
constexpr int ZOMBATAR_CONFIRM_TEXT_H = 100;

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
	mDeleteHover = false;

	mBackButton = MakeNewButton(ZOMBATAR_BTN_BACK, this, "", nullptr,
		IMAGE_ZOMBATAR_BACK_BUTTON, IMAGE_ZOMBATAR_BACK_BUTTON_HIGHLIGHT, nullptr);
	mBackButton->Resize(ZOMBATAR_BACK_X, ZOMBATAR_BACK_Y, 98, 26);

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
	MarkDirty();
}

void ZombatarWidget::Open()
{
	if (!mApp->mPlayerInfo)
		return;

	mSubPage = 0;
	mMaxSubPages = 0;
	mPage = ZOMBATAR_PAGE_SKIN;
	mDeleteHover = false;
	mCurrentIndex = mApp->mPlayerInfo->mZombatarIndex;
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
	mColor[ZOMBATAR_PAGE_BACKDROPS] = ZOMBATAR_COLOR_NONE;
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
		if (mApp->mPlayerInfo)
			mApp->mPlayerInfo->mZombatarIndex = -1;
	}
	else
	{
		mCurrentIndex = ClampRange(mCurrentIndex, 0, aCount - 1);
		if (mApp->mPlayerInfo)
			mApp->mPlayerInfo->mZombatarIndex = mCurrentIndex;
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
	theColor[ZOMBATAR_PAGE_SKIN] = ClampRange(static_cast<int>(ZombatarReadRecordSlot(theRecord, ZOMBATAR_SLOT_SKIN_COLOR)), 0, 11);

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
	aPlayerInfo->mZombatarCreatedBefore = 1;
	mCurrentIndex = static_cast<int>(aOffset / ZOMBATAR_RECORD_SIZE);
	aPlayerInfo->mZombatarIndex = mCurrentIndex;

	if (!ExportAvatarPNG(aPlayerInfo->mZombatarData.data() + aOffset, mCurrentIndex + 1))
	{
		aPlayerInfo->mZombatarData.resize(aOffset);
		aPlayerInfo->mZombatarHeadCount = static_cast<uint32_t>(GetHeadCount());
		ClampCurrentIndex();
		mApp->LawnMessageBox(DIALOG_MESSAGE, "Zombatar Export Failed", "The Zombatar image could not be written.", "[DIALOG_BUTTON_OK]", "", Dialog::BUTTONS_FOOTER);
		return false;
	}

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
	int32_t aOldZombatarIndex = aPlayerInfo->mZombatarIndex;
	int aOldIndex = mCurrentIndex;
	unsigned char* aData = aPlayerInfo->mZombatarData.data();
	size_t aOffset = static_cast<size_t>(mCurrentIndex) * ZOMBATAR_RECORD_SIZE;
	size_t aTailOffset = aOffset + ZOMBATAR_RECORD_SIZE;
	size_t aTailBytes = aPlayerInfo->mZombatarData.size() - aTailOffset;
	if (aTailBytes > 0)
		memmove(aData + aOffset, aData + aTailOffset, aTailBytes);
	aPlayerInfo->mZombatarData.resize(aPlayerInfo->mZombatarData.size() - ZOMBATAR_RECORD_SIZE);
	aPlayerInfo->mZombatarHeadCount = static_cast<uint32_t>(GetHeadCount());
	aPlayerInfo->mZombatarIndex = GetHeadCount() > 0 ? ClampRange(aOldIndex, 0, GetHeadCount() - 1) : -1;
	if (!ExportAllAvatarPNGs())
	{
		aPlayerInfo->mZombatarData = aOldData;
		aPlayerInfo->mZombatarHeadCount = aOldHeadCount;
		aPlayerInfo->mZombatarIndex = aOldZombatarIndex;
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

Rect ZombatarWidget::GetItemRect(int theIndex) const
{
	int aCellW = IMAGE_ZOMBATAR_ACCESSORY_BG ? IMAGE_ZOMBATAR_ACCESSORY_BG->mWidth : 67;
	int aCellH = IMAGE_ZOMBATAR_ACCESSORY_BG ? IMAGE_ZOMBATAR_ACCESSORY_BG->mHeight : 67;
	int aStepX = aCellW + ZOMBATAR_GRID_GAP;
	int aStepY = aCellH + 3 + ZOMBATAR_GRID_GAP;
	int aOriginX = ZOMBATAR_PANEL_X + (ZOMBATAR_PANEL_W - ZOMBATAR_GRID_COLS * aStepX) / 2 + ZOMBATAR_GRID_BIAS_X;
	int aOriginY = ZOMBATAR_PANEL_Y + 2 * ZOMBATAR_GRID_GAP + 120;
	int aCol = theIndex % ZOMBATAR_GRID_COLS;
	int aRow = (theIndex % ZOMBATAR_GRID_PAGE) / ZOMBATAR_GRID_COLS;
	return Rect(aOriginX + aCol * aStepX, aOriginY + aRow * aStepY, aCellW, aCellH);
}

Rect ZombatarWidget::GetColorRect(int theIndex) const
{
	int aSwatchW = IMAGE_ZOMBATAR_COLORPICKER ? IMAGE_ZOMBATAR_COLORPICKER->mWidth : 21;
	int aSwatchH = IMAGE_ZOMBATAR_COLORPICKER ? IMAGE_ZOMBATAR_COLORPICKER->mHeight : 20;
	int aStep = aSwatchW + ZOMBATAR_COLOR_GAP;
	int aCol = theIndex % ZOMBATAR_COLOR_COLS;
	int aRow = theIndex / ZOMBATAR_COLOR_COLS;
	return Rect(ZOMBATAR_COLOR_X + aCol * aStep, ZOMBATAR_COLOR_Y + aRow * aStep, aSwatchW, aSwatchH);
}

int ZombatarWidget::GetTotalItemsForPage(ZombatarPage thePage) const
{
	switch (thePage)
	{
	case ZOMBATAR_PAGE_SKIN: return 0;
	case ZOMBATAR_PAGE_HAIR: return 16;
	case ZOMBATAR_PAGE_FACIAL_HAIR: return 24;
	case ZOMBATAR_PAGE_TIDBITS: return 14;
	case ZOMBATAR_PAGE_EYEWEAR: return 16;
	case ZOMBATAR_PAGE_CLOTHES: return 12;
	case ZOMBATAR_PAGE_ACCESSORY: return 15;
	case ZOMBATAR_PAGE_HATS: return 14;
	case ZOMBATAR_PAGE_BACKDROPS: return 5;
	default: return 0;
	}
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
	return mPart[mPage] >= 0;
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
	static Image* aImages[] =
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
	static Image* aClothes[] = { IMAGE_ZOMBATAR_CLOTHES_1, IMAGE_ZOMBATAR_CLOTHES_2, IMAGE_ZOMBATAR_CLOTHES_3, IMAGE_ZOMBATAR_CLOTHES_4, IMAGE_ZOMBATAR_CLOTHES_5, IMAGE_ZOMBATAR_CLOTHES_6, IMAGE_ZOMBATAR_CLOTHES_7, IMAGE_ZOMBATAR_CLOTHES_8, IMAGE_ZOMBATAR_CLOTHES_9, IMAGE_ZOMBATAR_CLOTHES_10, IMAGE_ZOMBATAR_CLOTHES_11, IMAGE_ZOMBATAR_CLOTHES_12 };
	static Image* aHats[] = { IMAGE_ZOMBATAR_HATS_1, IMAGE_ZOMBATAR_HATS_2, IMAGE_ZOMBATAR_HATS_3, IMAGE_ZOMBATAR_HATS_4, IMAGE_ZOMBATAR_HATS_5, IMAGE_ZOMBATAR_HATS_6, IMAGE_ZOMBATAR_HATS_7, IMAGE_ZOMBATAR_HATS_8, IMAGE_ZOMBATAR_HATS_9, IMAGE_ZOMBATAR_HATS_10, IMAGE_ZOMBATAR_HATS_11, IMAGE_ZOMBATAR_HATS_12, IMAGE_ZOMBATAR_HATS_13, IMAGE_ZOMBATAR_HATS_14 };
	static Image* aHair[] = { IMAGE_ZOMBATAR_HAIR_1, IMAGE_ZOMBATAR_HAIR_2, IMAGE_ZOMBATAR_HAIR_3, IMAGE_ZOMBATAR_HAIR_4, IMAGE_ZOMBATAR_HAIR_5, IMAGE_ZOMBATAR_HAIR_6, IMAGE_ZOMBATAR_HAIR_7, IMAGE_ZOMBATAR_HAIR_8, IMAGE_ZOMBATAR_HAIR_9, IMAGE_ZOMBATAR_HAIR_10, IMAGE_ZOMBATAR_HAIR_11, IMAGE_ZOMBATAR_HAIR_12, IMAGE_ZOMBATAR_HAIR_13, IMAGE_ZOMBATAR_HAIR_14, IMAGE_ZOMBATAR_HAIR_15, IMAGE_ZOMBATAR_HAIR_16 };
	static Image* aEyewear[] = { IMAGE_ZOMBATAR_EYEWEAR_1, IMAGE_ZOMBATAR_EYEWEAR_2, IMAGE_ZOMBATAR_EYEWEAR_3, IMAGE_ZOMBATAR_EYEWEAR_4, IMAGE_ZOMBATAR_EYEWEAR_5, IMAGE_ZOMBATAR_EYEWEAR_6, IMAGE_ZOMBATAR_EYEWEAR_7, IMAGE_ZOMBATAR_EYEWEAR_8, IMAGE_ZOMBATAR_EYEWEAR_9, IMAGE_ZOMBATAR_EYEWEAR_10, IMAGE_ZOMBATAR_EYEWEAR_11, IMAGE_ZOMBATAR_EYEWEAR_12, IMAGE_ZOMBATAR_EYEWEAR_13, IMAGE_ZOMBATAR_EYEWEAR_14, IMAGE_ZOMBATAR_EYEWEAR_15, IMAGE_ZOMBATAR_EYEWEAR_16 };
	static Image* aFacial[] = { IMAGE_ZOMBATAR_FACIALHAIR_1, IMAGE_ZOMBATAR_FACIALHAIR_2, IMAGE_ZOMBATAR_FACIALHAIR_3, IMAGE_ZOMBATAR_FACIALHAIR_4, IMAGE_ZOMBATAR_FACIALHAIR_5, IMAGE_ZOMBATAR_FACIALHAIR_6, IMAGE_ZOMBATAR_FACIALHAIR_7, IMAGE_ZOMBATAR_FACIALHAIR_8, IMAGE_ZOMBATAR_FACIALHAIR_9, IMAGE_ZOMBATAR_FACIALHAIR_10, IMAGE_ZOMBATAR_FACIALHAIR_11, IMAGE_ZOMBATAR_FACIALHAIR_12, IMAGE_ZOMBATAR_FACIALHAIR_13, IMAGE_ZOMBATAR_FACIALHAIR_14, IMAGE_ZOMBATAR_FACIALHAIR_15, IMAGE_ZOMBATAR_FACIALHAIR_16, IMAGE_ZOMBATAR_FACIALHAIR_17, IMAGE_ZOMBATAR_FACIALHAIR_18, IMAGE_ZOMBATAR_FACIALHAIR_19, IMAGE_ZOMBATAR_FACIALHAIR_20, IMAGE_ZOMBATAR_FACIALHAIR_21, IMAGE_ZOMBATAR_FACIALHAIR_22, IMAGE_ZOMBATAR_FACIALHAIR_23, IMAGE_ZOMBATAR_FACIALHAIR_24 };
	static Image* aTidbits[] = { IMAGE_ZOMBATAR_TIDBITS_1, IMAGE_ZOMBATAR_TIDBITS_2, IMAGE_ZOMBATAR_TIDBITS_3, IMAGE_ZOMBATAR_TIDBITS_4, IMAGE_ZOMBATAR_TIDBITS_5, IMAGE_ZOMBATAR_TIDBITS_6, IMAGE_ZOMBATAR_TIDBITS_7, IMAGE_ZOMBATAR_TIDBITS_8, IMAGE_ZOMBATAR_TIDBITS_9, IMAGE_ZOMBATAR_TIDBITS_10, IMAGE_ZOMBATAR_TIDBITS_11, IMAGE_ZOMBATAR_TIDBITS_12, IMAGE_ZOMBATAR_TIDBITS_13, IMAGE_ZOMBATAR_TIDBITS_14 };
	static Image* aAccessory[] = { IMAGE_ZOMBATAR_ACCESSORY_1, IMAGE_ZOMBATAR_ACCESSORY_2, IMAGE_ZOMBATAR_ACCESSORY_3, IMAGE_ZOMBATAR_ACCESSORY_4, IMAGE_ZOMBATAR_ACCESSORY_5, IMAGE_ZOMBATAR_ACCESSORY_6, IMAGE_ZOMBATAR_ACCESSORY_8, IMAGE_ZOMBATAR_ACCESSORY_9, IMAGE_ZOMBATAR_ACCESSORY_10, IMAGE_ZOMBATAR_ACCESSORY_11, IMAGE_ZOMBATAR_ACCESSORY_12, IMAGE_ZOMBATAR_ACCESSORY_13, IMAGE_ZOMBATAR_ACCESSORY_14, IMAGE_ZOMBATAR_ACCESSORY_15, IMAGE_ZOMBATAR_ACCESSORY_16 };

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
	static Image* aHatsMasks[] =
	{
		IMAGE_ZOMBATAR_HATS_1_MASK, nullptr, IMAGE_ZOMBATAR_HATS_3_MASK, nullptr, nullptr,
		IMAGE_ZOMBATAR_HATS_6_MASK, IMAGE_ZOMBATAR_HATS_7_MASK, IMAGE_ZOMBATAR_HATS_8_MASK,
		IMAGE_ZOMBATAR_HATS_9_MASK, nullptr, IMAGE_ZOMBATAR_HATS_11_MASK, nullptr, nullptr, nullptr
	};
	static Image* aHairMasks[] =
	{
		IMAGE_ZOMBATAR_HAIR_1_MASK, IMAGE_ZOMBATAR_HAIR_2_MASK, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, IMAGE_ZOMBATAR_HAIR_11_MASK, IMAGE_ZOMBATAR_HAIR_12_MASK,
		IMAGE_ZOMBATAR_HAIR_13_MASK, IMAGE_ZOMBATAR_HAIR_14_MASK, IMAGE_ZOMBATAR_HAIR_15_MASK, nullptr
	};
	static Image* aEyewearMasks[] =
	{
		IMAGE_ZOMBATAR_EYEWEAR_1_MASK, IMAGE_ZOMBATAR_EYEWEAR_2_MASK, IMAGE_ZOMBATAR_EYEWEAR_3_MASK,
		IMAGE_ZOMBATAR_EYEWEAR_4_MASK, IMAGE_ZOMBATAR_EYEWEAR_5_MASK, IMAGE_ZOMBATAR_EYEWEAR_6_MASK,
		IMAGE_ZOMBATAR_EYEWEAR_7_MASK, IMAGE_ZOMBATAR_EYEWEAR_8_MASK, IMAGE_ZOMBATAR_EYEWEAR_9_MASK,
		IMAGE_ZOMBATAR_EYEWEAR_10_MASK, IMAGE_ZOMBATAR_EYEWEAR_11_MASK, IMAGE_ZOMBATAR_EYEWEAR_12_MASK,
		nullptr, nullptr, nullptr, nullptr
	};
	static Image* aFacialMasks[] =
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

	if (aMask)
	{
		g->DrawImage(aImage, theX, theY);
		DrawImageColorized(g, aMask, theX, theY, theColorIndex);
	}
	else
	{
		DrawImageColorized(g, aImage, theX, theY, theColorIndex);
	}
}

void ZombatarWidget::DrawAvatar(Graphics* g, int theX, int theY, const unsigned char* theRecord)
{
	int aPart[NUM_ZOMBATAR_PAGES];
	int aColor[NUM_ZOMBATAR_PAGES];
	DecodeRecord(theRecord, aPart, aColor);

	g->DrawImage(GetBackgroundImage(aPart[ZOMBATAR_PAGE_BACKDROPS]), theX, theY);
	DrawImageColorized(g, IMAGE_ZOMBATAR_ZOMBIE_BLANK_SKIN, theX, theY, aColor[ZOMBATAR_PAGE_SKIN]);
	g->DrawImage(IMAGE_ZOMBATAR_ZOMBIE_BLANK, theX, theY);
	DrawPartImage(g, ZOMBATAR_PAGE_CLOTHES, aPart[ZOMBATAR_PAGE_CLOTHES], theX, theY, aColor[ZOMBATAR_PAGE_CLOTHES]);
	DrawPartImage(g, ZOMBATAR_PAGE_ACCESSORY, aPart[ZOMBATAR_PAGE_ACCESSORY], theX, theY, aColor[ZOMBATAR_PAGE_ACCESSORY]);
	DrawPartImage(g, ZOMBATAR_PAGE_TIDBITS, aPart[ZOMBATAR_PAGE_TIDBITS], theX, theY, aColor[ZOMBATAR_PAGE_TIDBITS]);
	DrawPartImage(g, ZOMBATAR_PAGE_FACIAL_HAIR, aPart[ZOMBATAR_PAGE_FACIAL_HAIR], theX, theY, aColor[ZOMBATAR_PAGE_FACIAL_HAIR]);
	DrawPartImage(g, ZOMBATAR_PAGE_HAIR, aPart[ZOMBATAR_PAGE_HAIR], theX, theY, aColor[ZOMBATAR_PAGE_HAIR]);
	DrawPartImage(g, ZOMBATAR_PAGE_EYEWEAR, aPart[ZOMBATAR_PAGE_EYEWEAR], theX, theY, aColor[ZOMBATAR_PAGE_EYEWEAR]);
	DrawPartImage(g, ZOMBATAR_PAGE_HATS, aPart[ZOMBATAR_PAGE_HATS], theX, theY, aColor[ZOMBATAR_PAGE_HATS]);
}

void ZombatarWidget::DrawDraftAvatar(Graphics* g, int theX, int theY)
{
	unsigned char aRecord[ZOMBATAR_RECORD_SIZE];
	EncodeRecord(aRecord);
	DrawAvatar(g, theX, theY, aRecord);
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
	memcpy(aExportImage.mBits, aSourceBits, sizeof(uint32_t) * aExportImage.mWidth * aExportImage.mHeight);

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
	if (mState == ZOMBATAR_STATE_LIST)
		DrawList(g);
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

	g->SetFont(FONT_DWARVENTODCRAFT12);
	g->SetColor(Color(255, 255, 255));
	g->DrawString(Sexy::StrFormat("%d / %d", mCurrentIndex + 1, aCount), ZOMBATAR_LIST_COUNTER_X, ZOMBATAR_LIST_COUNTER_Y);

	g->SetColor(mDeleteHover ? Color(255, 96, 96) : Color(255, 255, 255));
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

	DrawDraftAvatar(g, ZOMBATAR_PREVIEW_X, ZOMBATAR_PREVIEW_Y);

	int aItemCount = GetItemCountForPage();
	int aBaseIndex = mSubPage * ZOMBATAR_GRID_PAGE;
	for (int i = 0; i < aItemCount; i++)
	{
		Rect aRect = GetItemRect(i);
		bool aSelected = mPart[mPage] == aBaseIndex + i;
		g->DrawImage(aSelected ? IMAGE_ZOMBATAR_ACCESSORY_BG_HIGHLIGHT : IMAGE_ZOMBATAR_ACCESSORY_BG, aRect.mX, aRect.mY);
		if (mPage == ZOMBATAR_PAGE_BACKDROPS)
		{
			Image* aImage = GetBackgroundImage(aBaseIndex + i);
			if (aImage)
				g->DrawImage(aImage, Rect(aRect.mX + 6, aRect.mY + 4, std::max(8, aRect.mWidth - 12), std::max(8, aRect.mHeight - 8)), Rect(0, 0, aImage->mWidth, aImage->mHeight));
		}
		else
		{
			Image* aImage = GetPartImage(mPage, aBaseIndex + i);
			if (aImage)
				g->DrawImage(aImage, Rect(aRect.mX + 3, aRect.mY + 1, std::max(8, aRect.mWidth - 6), std::max(8, aRect.mHeight - 6)), Rect(0, 0, aImage->mWidth, aImage->mHeight));
		}
	}

	if (PageAllowsNone())
	{
		Rect aRect = GetItemRect(aItemCount);
		g->DrawImage(IMAGE_ZOMBATAR_ACCESSORY_BG_NONE, aRect.mX, aRect.mY);
		if (mPart[mPage] < 0)
			g->DrawImage(IMAGE_ZOMBATAR_ACCESSORY_BG_HIGHLIGHT, aRect.mX, aRect.mY);
	}

	if (PageAllowsColors())
	{
		g->DrawImage(IMAGE_ZOMBATAR_COLORS_BG, ZOMBATAR_COLORS_X, ZOMBATAR_COLORS_Y);
		bool aSkin = mPage == ZOMBATAR_PAGE_SKIN;
		int aCount = aSkin ? 12 : 18;
		for (int i = 0; i < aCount; i++)
		{
			Rect aRect = GetColorRect(i);
			bool aNone = !aSkin && i == ZOMBATAR_PART_COLOR_NONE_SWATCH;
			bool aSelected;
			int aPalette;
			if (aSkin)
			{
				aSelected = mColor[ZOMBATAR_PAGE_SKIN] == i;
				aPalette = i;
			}
			else if (aNone)
			{
				aSelected = mColor[mPage] < 0;
				aPalette = 29;
			}
			else
			{
				aSelected = mColor[mPage] == ZOMBATAR_PART_COLOR_BASE + i;
				aPalette = ZOMBATAR_PART_COLOR_BASE + i;
			}

			g->DrawImage(aSelected ? IMAGE_ZOMBATAR_COLORPICKER_HIGHLIGHT : (aNone ? IMAGE_ZOMBATAR_COLORPICKER_NONE : IMAGE_ZOMBATAR_COLORPICKER), aRect.mX, aRect.mY);
			if (!aNone)
			{
				g->SetColor(ZombatarGetColor(aPalette));
				g->FillRect(aRect.mX + 4, aRect.mY + 4, std::max(4, aRect.mWidth - 8), std::max(4, aRect.mHeight - 8));
				g->SetColor(Color::White);
			}
		}
	}

	if (mMaxSubPages > 0)
	{
		g->SetFont(FONT_DWARVENTODCRAFT12);
		g->SetColor(Color(255, 255, 255));
		std::string aPage = Sexy::StrFormat("Page %d / %d", mSubPage + 1, mMaxSubPages + 1);
		g->DrawString(aPage, ZOMBATAR_PANEL_X + ZOMBATAR_PANEL_W / 2 - FONT_DWARVENTODCRAFT12->StringWidth(aPage) / 2, ZOMBATAR_PAGE_BTN_Y + 40);
	}

	if (mState == ZOMBATAR_STATE_CONFIRM)
	{
		g->SetFont(FONT_DWARVENTODCRAFT18);
		g->SetColor(Color(254, 227, 0));
		std::string aHeader = TodStringTranslate("[ZOMBATAR_FINISHED_WARNING_HEADER]");
		g->DrawString(aHeader, ZOMBATAR_CONFIRM_HEADER_X - FONT_DWARVENTODCRAFT18->StringWidth(aHeader) / 2, ZOMBATAR_CONFIRM_HEADER_Y);
		g->SetFont(FONT_DWARVENTODCRAFT12);
		g->SetColor(Color(255, 255, 255));
		g->WriteWordWrapped(Rect(ZOMBATAR_CONFIRM_TEXT_X, ZOMBATAR_CONFIRM_TEXT_Y, ZOMBATAR_CONFIRM_TEXT_W, ZOMBATAR_CONFIRM_TEXT_H), TodStringTranslate("[ZOMBATAR_FINISHED_WARNING_TEXT]"), 18);
	}
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
	int aTotal = GetTotalItemsForPage(thePage);
	mMaxSubPages = (aTotal > ZOMBATAR_GRID_PAGE) ? (aTotal - 1) / ZOMBATAR_GRID_PAGE : 0;
	UpdateButtonState();
}

void ZombatarWidget::UpdateButtonState()
{
	int aCount = GetHeadCount();
	bool aList = mState == ZOMBATAR_STATE_LIST;
	bool aCreate = mState == ZOMBATAR_STATE_CREATE;
	bool aConfirm = mState == ZOMBATAR_STATE_CONFIRM;
	bool aPaged = aCreate && mMaxSubPages > 0;

	mBackButton->SetVisible(aCreate || aList);
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
	{
		mFinishedButton->mButtonImage = IMAGE_ZOMBATAR_ACCEPT_BUTTON;
		mFinishedButton->mOverImage = IMAGE_ZOMBATAR_ACCEPT_BUTTON_HIGHLIGHT;
		mFinishedButton->mDownImage = nullptr;
		mFinishedButton->Resize(ZOMBATAR_ACCEPT_X, ZOMBATAR_CONFIRM_BTN_Y, 98, 26);
	}
	else
	{
		mFinishedButton->mButtonImage = IMAGE_ZOMBATAR_FINISHED_BUTTON;
		mFinishedButton->mOverImage = IMAGE_ZOMBATAR_FINISHED_BUTTON_HIGHLIGHT;
		mFinishedButton->mDownImage = nullptr;
		mFinishedButton->Resize(ZOMBATAR_FINISHED_X, ZOMBATAR_FINISHED_Y, 103, 26);
	}
}

void ZombatarWidget::MouseMove(int x, int y)
{
	mMouseX = x;
	mMouseY = y;
	if (mState == ZOMBATAR_STATE_LIST)
		mDeleteHover = Rect(ZOMBATAR_LIST_DELETE_RECT_X, ZOMBATAR_LIST_DELETE_RECT_Y, ZOMBATAR_LIST_DELETE_RECT_W, ZOMBATAR_LIST_DELETE_RECT_H).Contains(x, y);
}

void ZombatarWidget::HandleGridClick(int x, int y)
{
	if (mState != ZOMBATAR_STATE_CREATE)
		return;

	int aItemCount = GetItemCountForPage();
	int aBaseIndex = mSubPage * ZOMBATAR_GRID_PAGE;
	for (int i = 0; i < aItemCount; i++)
	{
		if (GetItemRect(i).Contains(x, y))
		{
			mPart[mPage] = aBaseIndex + i;
			UpdateButtonState();
			return;
		}
	}

	if (PageAllowsNone() && GetItemRect(aItemCount).Contains(x, y))
	{
		mPart[mPage] = -1;
		UpdateButtonState();
	}
}

void ZombatarWidget::HandleColorClick(int x, int y)
{
	if (mState != ZOMBATAR_STATE_CREATE || !PageAllowsColors())
		return;

	bool aSkin = mPage == ZOMBATAR_PAGE_SKIN;
	int aCount = aSkin ? 12 : 18;
	for (int i = 0; i < aCount; i++)
	{
		if (GetColorRect(i).Contains(x, y))
		{
			if (aSkin)
				mColor[ZOMBATAR_PAGE_SKIN] = i;
			else
				mColor[mPage] = (i == ZOMBATAR_PART_COLOR_NONE_SWATCH) ? ZOMBATAR_COLOR_NONE : ZOMBATAR_PART_COLOR_BASE + i;
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
				DIALOG_MESSAGE,
				"Delete Zombatar?",
				"Delete this saved Zombatar from this profile?",
				"[DIALOG_BUTTON_YES]",
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
		if (mState == ZOMBATAR_STATE_CREATE && GetHeadCount() > 0)
		{
			ClampCurrentIndex();
			LoadCurrentToDraft();
			ChangeState(ZOMBATAR_STATE_LIST);
		}
		else
			BackToSelector();
		break;

	case ZOMBATAR_BTN_CONFIRM_BACK:
		ChangeState(ZOMBATAR_STATE_CREATE);
		break;

	case ZOMBATAR_BTN_VIEW:
		ClampCurrentIndex();
		LoadCurrentToDraft();
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
			mApp->mPlayerInfo->mZombatarIndex = mCurrentIndex;
			LoadCurrentToDraft();
			UpdateButtonState();
		}
		break;

	case ZOMBATAR_BTN_NEXT_PORTRAIT:
		if (mCurrentIndex + 1 < GetHeadCount())
		{
			mCurrentIndex++;
			mApp->mPlayerInfo->mZombatarIndex = mCurrentIndex;
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
		else if (mState == ZOMBATAR_STATE_CREATE && GetHeadCount() > 0)
		{
			LoadCurrentToDraft();
			ChangeState(ZOMBATAR_STATE_LIST);
		}
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
