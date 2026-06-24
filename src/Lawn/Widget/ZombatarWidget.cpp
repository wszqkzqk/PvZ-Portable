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

static const int ZOMBATAR_DEFAULT_COLOR = 17;

static int ClampRange(int theValue, int theMin, int theMax)
{
	return std::max(theMin, std::min(theValue, theMax));
}

static int SlotForPart(ZombatarPage thePage)
{
	switch (thePage)
	{
	case ZOMBATAR_PAGE_CLOTHES: return ZOMBATAR_SLOT_CLOTHES;
	case ZOMBATAR_PAGE_TIDBITS: return ZOMBATAR_SLOT_TIDBITS;
	case ZOMBATAR_PAGE_ACCESSORY: return ZOMBATAR_SLOT_ACCESSORY;
	case ZOMBATAR_PAGE_FACIAL_HAIR: return ZOMBATAR_SLOT_FACIAL_HAIR;
	case ZOMBATAR_PAGE_HAIR: return ZOMBATAR_SLOT_HAIR;
	case ZOMBATAR_PAGE_EYEWEAR: return ZOMBATAR_SLOT_EYEWEAR;
	case ZOMBATAR_PAGE_HATS: return ZOMBATAR_SLOT_HATS;
	case ZOMBATAR_PAGE_BACKDROPS: return ZOMBATAR_SLOT_BACKGROUND;
	default: return ZOMBATAR_SLOT_SKIN_COLOR;
	}
}

static int SlotForColor(ZombatarPage thePage)
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
	mEditingIndex = -1;
	mFacialHairPage = 0;
	mMouseX = -1;
	mMouseY = -1;

	mBackButton = MakeNewButton(ZOMBATAR_BTN_BACK, this, "", nullptr,
		IMAGE_ZOMBATAR_BACK_BUTTON, IMAGE_ZOMBATAR_BACK_BUTTON_HIGHLIGHT, IMAGE_ZOMBATAR_BACK_BUTTON_HIGHLIGHT);
	mBackButton->Resize(30, 510, 160, 62);

	mViewButton = MakeNewButton(ZOMBATAR_BTN_VIEW, this, "", nullptr,
		IMAGE_ZOMBATAR_VIEW_BUTTON, IMAGE_ZOMBATAR_VIEW_BUTTON_HIGHLIGHT, IMAGE_ZOMBATAR_VIEW_BUTTON_HIGHLIGHT);
	mViewButton->Resize(346, 522, 160, 62);

	mFinishedButton = MakeNewButton(ZOMBATAR_BTN_FINISHED, this, "", nullptr,
		IMAGE_ZOMBATAR_FINISHED_BUTTON, IMAGE_ZOMBATAR_FINISHED_BUTTON_HIGHLIGHT, IMAGE_ZOMBATAR_FINISHED_BUTTON_HIGHLIGHT);
	mFinishedButton->Resize(554, 522, 210, 62);

	mNewButton = MakeNewButton(ZOMBATAR_BTN_NEW, this, "", nullptr,
		IMAGE_ZOMBATAR_NEWZOMBIE_BUTTON, IMAGE_ZOMBATAR_NEWZOMBIE_BUTTON_HIGHLIGHT, IMAGE_ZOMBATAR_NEWZOMBIE_BUTTON_HIGHLIGHT);
	mNewButton->Resize(250, 440, 230, 70);

	mEditButton = MakeNewButton(ZOMBATAR_BTN_EDIT, this, "Edit", FONT_DWARVENTODCRAFT12,
		IMAGE_BLANK, IMAGE_BLANK, IMAGE_BLANK);
	mEditButton->Resize(500, 440, 130, 44);
	mEditButton->SetColor(ButtonWidget::COLOR_LABEL, Color::White);
	mEditButton->SetColor(ButtonWidget::COLOR_LABEL_HILITE, Color(0, 255, 40));

	mDeleteButton = MakeNewButton(ZOMBATAR_BTN_DELETE, this, "Delete", FONT_DWARVENTODCRAFT12,
		IMAGE_BLANK, IMAGE_BLANK, IMAGE_BLANK);
	mDeleteButton->Resize(500, 490, 130, 44);
	mDeleteButton->SetColor(ButtonWidget::COLOR_LABEL, Color(255, 96, 96));
	mDeleteButton->SetColor(ButtonWidget::COLOR_LABEL_HILITE, Color(255, 180, 180));

	mPrevPortraitButton = MakeNewButton(ZOMBATAR_BTN_PREV_PORTRAIT, this, "", nullptr,
		IMAGE_ZOMBATAR_PREV_BUTTON, IMAGE_ZOMBATAR_PREV_BUTTON_HIGHLIGHT, IMAGE_ZOMBATAR_PREV_BUTTON_HIGHLIGHT);
	mPrevPortraitButton->Resize(164, 250, 80, 80);

	mNextPortraitButton = MakeNewButton(ZOMBATAR_BTN_NEXT_PORTRAIT, this, "", nullptr,
		IMAGE_ZOMBATAR_NEXT_BUTTON, IMAGE_ZOMBATAR_NEXT_BUTTON_HIGHLIGHT, IMAGE_ZOMBATAR_NEXT_BUTTON_HIGHLIGHT);
	mNextPortraitButton->Resize(588, 250, 80, 80);

	mPrevPageButton = MakeNewButton(ZOMBATAR_BTN_PREV_PAGE, this, "", nullptr,
		IMAGE_ZOMBATAR_PREV_BUTTON, IMAGE_ZOMBATAR_PREV_BUTTON_HIGHLIGHT, IMAGE_ZOMBATAR_PREV_BUTTON_HIGHLIGHT);
	mPrevPageButton->Resize(366, 392, 80, 60);

	mNextPageButton = MakeNewButton(ZOMBATAR_BTN_NEXT_PAGE, this, "", nullptr,
		IMAGE_ZOMBATAR_NEXT_BUTTON, IMAGE_ZOMBATAR_NEXT_BUTTON_HIGHLIGHT, IMAGE_ZOMBATAR_NEXT_BUTTON_HIGHLIGHT);
	mNextPageButton->Resize(646, 392, 80, 60);

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
	delete mEditButton;
	delete mDeleteButton;
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
	AddWidget(mEditButton);
	AddWidget(mDeleteButton);
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
	RemoveWidget(mEditButton);
	RemoveWidget(mDeleteButton);
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

	// Re-sync with the active profile so switching users never shows a stale draft.
	mEditingIndex = -1;
	mFacialHairPage = 0;
	mPage = ZOMBATAR_PAGE_SKIN;
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
		mColor[i] = ZOMBATAR_DEFAULT_COLOR;
	}

	mPart[ZOMBATAR_PAGE_SKIN] = 0;
	mColor[ZOMBATAR_PAGE_SKIN] = 0;
	mPart[ZOMBATAR_PAGE_BACKDROPS] = 4;
	mColor[ZOMBATAR_PAGE_BACKDROPS] = ZOMBATAR_DEFAULT_COLOR;
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
		theColor[i] = ZOMBATAR_DEFAULT_COLOR;
	}

	thePart[ZOMBATAR_PAGE_SKIN] = 0;
	theColor[ZOMBATAR_PAGE_SKIN] = ClampRange(static_cast<int>(ZombatarReadRecordSlot(theRecord, ZOMBATAR_SLOT_SKIN_COLOR)), 0, 11);

	for (int i = ZOMBATAR_PAGE_HAIR; i < NUM_ZOMBATAR_PAGES; i++)
	{
		ZombatarPage aPage = static_cast<ZombatarPage>(i);
		int aItemCount = 0;
		switch (aPage)
		{
		case ZOMBATAR_PAGE_HAIR: aItemCount = 16; break;
		case ZOMBATAR_PAGE_FACIAL_HAIR: aItemCount = 24; break;
		case ZOMBATAR_PAGE_TIDBITS: aItemCount = 14; break;
		case ZOMBATAR_PAGE_EYEWEAR: aItemCount = 16; break;
		case ZOMBATAR_PAGE_CLOTHES: aItemCount = 12; break;
		case ZOMBATAR_PAGE_ACCESSORY: aItemCount = 15; break;
		case ZOMBATAR_PAGE_HATS: aItemCount = 14; break;
		case ZOMBATAR_PAGE_BACKDROPS: aItemCount = 5; break;
		default: break;
		}

		int aPart = ZombatarReadSignedRecordSlot(theRecord, SlotForPart(aPage));
		int aColor = ZombatarReadSignedRecordSlot(theRecord, SlotForColor(aPage));
		if (aPage == ZOMBATAR_PAGE_BACKDROPS)
			thePart[i] = ClampRange(aPart, 0, aItemCount - 1);
		else
			thePart[i] = (aPart >= 0 && aPart < aItemCount) ? aPart : -1;
		theColor[i] = ClampRange(aColor, 0, 17);
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
		ZombatarWriteRecordSlot(theRecord, SlotForColor(aPage), ClampRange(mColor[i], 0, 17));
	}

	ZombatarWriteRecordSlot(theRecord, ZOMBATAR_SLOT_RESERVED, 0);
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
	if (!aPlayerInfo)
		return false;
	if (mEditingIndex < 0 && !CanSaveNewHead())
		return false;

	size_t aOffset;
	int aExportIndex;
	if (mEditingIndex >= 0)
	{
		aOffset = static_cast<size_t>(mEditingIndex) * ZOMBATAR_RECORD_SIZE;
		if (aOffset + ZOMBATAR_RECORD_SIZE > aPlayerInfo->mZombatarData.size())
			return false;
		EncodeRecord(aPlayerInfo->mZombatarData.data() + aOffset);
		aExportIndex = mEditingIndex + 1;
	}
	else
	{
		aOffset = aPlayerInfo->mZombatarData.size();
		aPlayerInfo->mZombatarData.resize(aOffset + ZOMBATAR_RECORD_SIZE);
		EncodeRecord(aPlayerInfo->mZombatarData.data() + aOffset);
		aPlayerInfo->mZombatarCreatedBefore = 1;
		aExportIndex = static_cast<int>(aOffset / ZOMBATAR_RECORD_SIZE) + 1;
	}

	aPlayerInfo->mZombatarHeadCount = static_cast<uint32_t>(GetHeadCount());
	mCurrentIndex = static_cast<int>(aOffset / ZOMBATAR_RECORD_SIZE);
	aPlayerInfo->mZombatarIndex = mCurrentIndex;

	if (!ExportAvatarPNG(aPlayerInfo->mZombatarData.data() + aOffset, aExportIndex))
	{
		if (mEditingIndex < 0)
			aPlayerInfo->mZombatarData.resize(aOffset);
		aPlayerInfo->mZombatarHeadCount = static_cast<uint32_t>(GetHeadCount());
		ClampCurrentIndex();
		mApp->LawnMessageBox(DIALOG_MESSAGE, "Zombatar Export Failed", "The Zombatar image could not be written.", "[DIALOG_BUTTON_OK]", "", Dialog::BUTTONS_FOOTER);
		return false;
	}

	aPlayerInfo->SaveDetails();
	mEditingIndex = -1;
	ResetDraft();
	mPage = ZOMBATAR_PAGE_SKIN;
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

void ZombatarWidget::BeginEditCurrent()
{
	if (GetHeadCount() <= 0)
		return;
	ClampCurrentIndex();
	mEditingIndex = mCurrentIndex;
	LoadCurrentToDraft();
	mFacialHairPage = 0;
	ChangeState(ZOMBATAR_STATE_CREATE);
}

Rect ZombatarWidget::GetCategoryRect(int theIndex) const
{
	return Rect(368 + (theIndex % 3) * 124, 96 + (theIndex / 3) * 46, 112, 38);
}

Rect ZombatarWidget::GetItemRect(int theIndex) const
{
	return Rect(374 + (theIndex % 6) * 62, 266 + (theIndex / 6) * 58, 54, 50);
}

Rect ZombatarWidget::GetColorRect(int theIndex) const
{
	return Rect(396 + (theIndex % 6) * 42, 474 + (theIndex / 6) * 34, 30, 28);
}

int ZombatarWidget::GetItemCountForPage() const
{
	switch (mPage)
	{
	case ZOMBATAR_PAGE_SKIN: return 12;
	case ZOMBATAR_PAGE_HAIR: return 16;
	case ZOMBATAR_PAGE_FACIAL_HAIR: return mFacialHairPage == 0 ? 17 : 7;
	case ZOMBATAR_PAGE_TIDBITS: return 14;
	case ZOMBATAR_PAGE_EYEWEAR: return 16;
	case ZOMBATAR_PAGE_CLOTHES: return 12;
	case ZOMBATAR_PAGE_ACCESSORY: return 15;
	case ZOMBATAR_PAGE_HATS: return 14;
	case ZOMBATAR_PAGE_BACKDROPS: return 5;
	default: return 0;
	}
}

bool ZombatarWidget::PageAllowsNone() const
{
	return mPage != ZOMBATAR_PAGE_SKIN && mPage != ZOMBATAR_PAGE_BACKDROPS;
}

bool ZombatarWidget::PageAllowsColors() const
{
	// The skin page already exposes its full palette in the top grid, so the
	// bottom color picker is only shown for part pages with a selected part.
	return mPage != ZOMBATAR_PAGE_SKIN && mPart[mPage] >= 0;
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
	// Zombatar exposes 15 accessory slots; the visual sequence skips asset 7 and ends on asset 16.
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

	// ImageLib::WritePNGImage expects native-endian 0xAARRGGBB pixels (it calls
	// png_set_bgr), which is exactly what MemoryImage produces. Copying the bits
	// verbatim keeps the export correct on both little- and big-endian targets.
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
}

void ZombatarWidget::DrawMain(Graphics* g)
{
	g->DrawImage(IMAGE_ZOMBATAR_MAIN_BG, 0, 0);
	if (IMAGE_ZOMBATAR_LOGO)
		g->DrawImage(IMAGE_ZOMBATAR_LOGO, 18, 18);
}

void ZombatarWidget::DrawList(Graphics* g)
{
	int aCount = GetHeadCount();
	ClampCurrentIndex();
	if (aCount > 0)
	{
		const unsigned char* aRecord = mApp->mPlayerInfo->mZombatarData.data() + mCurrentIndex * ZOMBATAR_RECORD_SIZE;
		DrawAvatar(g, 250, 112, aRecord);
	}

	g->SetFont(FONT_DWARVENTODCRAFT18);
	g->SetColor(Color(255, 255, 255));
	g->DrawString(Sexy::StrFormat("%d / %d", aCount > 0 ? mCurrentIndex + 1 : 0, aCount), 360, 390);
}

void ZombatarWidget::DrawCreate(Graphics* g)
{
	DrawDraftAvatar(g, 70, 112);
	g->DrawImage(IMAGE_ZOMBATAR_WIDGET_BG, 345, 74);
	g->DrawImage(IMAGE_ZOMBATAR_WIDGET_INNER_BG, 360, 248);
	g->DrawImage(IMAGE_ZOMBATAR_COLORS_BG, 378, 452);

	for (int i = 0; i < NUM_ZOMBATAR_PAGES; i++)
	{
		Rect aRect = GetCategoryRect(i);
		g->DrawImage(GetCategoryImage(static_cast<ZombatarPage>(i), i == mPage, aRect.Contains(mMouseX, mMouseY)), aRect.mX, aRect.mY);
	}

	int aItemCount = GetItemCountForPage();
	int aBaseIndex = (mPage == ZOMBATAR_PAGE_FACIAL_HAIR && mFacialHairPage == 1) ? 17 : 0;
	for (int i = 0; i < aItemCount; i++)
	{
		Rect aRect = GetItemRect(i);
		bool aSelected = (mPage == ZOMBATAR_PAGE_SKIN) ? mColor[ZOMBATAR_PAGE_SKIN] == i : mPart[mPage] == aBaseIndex + i;
		g->DrawImage(aSelected ? IMAGE_ZOMBATAR_ACCESSORY_BG_HIGHLIGHT : IMAGE_ZOMBATAR_ACCESSORY_BG, aRect.mX, aRect.mY);
		if (mPage == ZOMBATAR_PAGE_SKIN)
		{
			g->SetColor(ZombatarGetColor(i));
			g->FillRect(aRect.mX + 9, aRect.mY + 8, 34, 32);
			g->SetColor(Color::White);
		}
		else if (mPage == ZOMBATAR_PAGE_BACKDROPS)
		{
			Image* aImage = GetBackgroundImage(i);
			g->DrawImage(aImage, Rect(aRect.mX + 6, aRect.mY + 4, 42, 40), Rect(0, 0, aImage->mWidth, aImage->mHeight));
		}
		else
		{
			Image* aImage = GetPartImage(mPage, aBaseIndex + i);
			if (aImage)
				g->DrawImage(aImage, Rect(aRect.mX + 3, aRect.mY + 1, 48, 46), Rect(0, 0, aImage->mWidth, aImage->mHeight));
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
		for (int i = 0; i < 18; i++)
		{
			Rect aRect = GetColorRect(i);
			g->DrawImage(mColor[mPage] == i ? IMAGE_ZOMBATAR_COLORPICKER_HIGHLIGHT : (i == ZOMBATAR_DEFAULT_COLOR ? IMAGE_ZOMBATAR_COLORPICKER_NONE : IMAGE_ZOMBATAR_COLORPICKER), aRect.mX, aRect.mY);
			if (i != ZOMBATAR_DEFAULT_COLOR)
			{
				g->SetColor(ZombatarGetColor(i));
				g->FillRect(aRect.mX + 6, aRect.mY + 5, 18, 17);
				g->SetColor(Color::White);
			}
		}
	}

	if (mState == ZOMBATAR_STATE_CONFIRM)
	{
		g->SetFont(FONT_DWARVENTODCRAFT12);
		g->SetColor(Color(255, 255, 255));
		g->WriteWordWrapped(Rect(348, 220, 355, 44), "Save this Zombatar to your profile?", 18);
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
	mFacialHairPage = 0;
	UpdateButtonState();
}

void ZombatarWidget::UpdateButtonState()
{
	int aCount = GetHeadCount();
	bool aList = mState == ZOMBATAR_STATE_LIST;
	bool aCreate = mState == ZOMBATAR_STATE_CREATE;
	bool aConfirm = mState == ZOMBATAR_STATE_CONFIRM;
	bool aFacial = mPage == ZOMBATAR_PAGE_FACIAL_HAIR;

	mBackButton->SetVisible(true);
	mViewButton->SetVisible(aCreate && aCount > 0);
	mFinishedButton->SetVisible(aCreate || aConfirm);
	mNewButton->SetVisible(aList);
	mEditButton->SetVisible(aList && aCount > 0);
	mDeleteButton->SetVisible(aList && aCount > 0);
	mPrevPortraitButton->SetVisible(aList && aCount > 1);
	mNextPortraitButton->SetVisible(aList && aCount > 1);
	mPrevPageButton->SetVisible(aCreate && aFacial);
	mNextPageButton->SetVisible(aCreate && aFacial);

	mNewButton->mDisabled = !CanSaveNewHead();
	mEditButton->mDisabled = aCount <= 0;
	mDeleteButton->mDisabled = aCount <= 0;
	mPrevPortraitButton->mDisabled = mCurrentIndex <= 0;
	mNextPortraitButton->mDisabled = mCurrentIndex + 1 >= aCount;
	mPrevPageButton->mDisabled = mFacialHairPage <= 0;
	mNextPageButton->mDisabled = mFacialHairPage >= 1;
	mFinishedButton->mDisabled = aCreate && mEditingIndex < 0 && !CanSaveNewHead();

	if (aConfirm)
	{
		mFinishedButton->mButtonImage = IMAGE_ZOMBATAR_ACCEPT_BUTTON;
		mFinishedButton->mOverImage = IMAGE_ZOMBATAR_ACCEPT_BUTTON_HIGHLIGHT;
		mFinishedButton->mDownImage = IMAGE_ZOMBATAR_ACCEPT_BUTTON_HIGHLIGHT;
	}
	else
	{
		mFinishedButton->mButtonImage = IMAGE_ZOMBATAR_FINISHED_BUTTON;
		mFinishedButton->mOverImage = IMAGE_ZOMBATAR_FINISHED_BUTTON_HIGHLIGHT;
		mFinishedButton->mDownImage = IMAGE_ZOMBATAR_FINISHED_BUTTON_HIGHLIGHT;
	}
}

void ZombatarWidget::MouseMove(int x, int y)
{
	mMouseX = x;
	mMouseY = y;
}

void ZombatarWidget::HandleGridClick(int x, int y)
{
	if (mState != ZOMBATAR_STATE_CREATE)
		return;

	int aItemCount = GetItemCountForPage();
	int aBaseIndex = (mPage == ZOMBATAR_PAGE_FACIAL_HAIR && mFacialHairPage == 1) ? 17 : 0;
	for (int i = 0; i < aItemCount; i++)
	{
		if (GetItemRect(i).Contains(x, y))
		{
			if (mPage == ZOMBATAR_PAGE_SKIN)
				mColor[ZOMBATAR_PAGE_SKIN] = i;
			else
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

	for (int i = 0; i < 18; i++)
	{
		if (GetColorRect(i).Contains(x, y))
		{
			mColor[mPage] = i;
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
		if (mState == ZOMBATAR_STATE_CONFIRM)
			ChangeState(ZOMBATAR_STATE_CREATE);
		else if (mState == ZOMBATAR_STATE_CREATE && GetHeadCount() > 0)
		{
			mEditingIndex = -1;
			ClampCurrentIndex();
			LoadCurrentToDraft();
			ChangeState(ZOMBATAR_STATE_LIST);
		}
		else
			BackToSelector();
		break;

	case ZOMBATAR_BTN_VIEW:
		mEditingIndex = -1;
		ClampCurrentIndex();
		LoadCurrentToDraft();
		ChangeState(ZOMBATAR_STATE_LIST);
		break;

	case ZOMBATAR_BTN_FINISHED:
		if (mState == ZOMBATAR_STATE_CREATE)
		{
			if (mEditingIndex >= 0 || CanSaveNewHead())
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
			mEditingIndex = -1;
			ResetDraft();
			mPage = ZOMBATAR_PAGE_SKIN;
			ChangeState(ZOMBATAR_STATE_CREATE);
		}
		else
		{
			ShowMaxHeadsMessage();
		}
		break;

	case ZOMBATAR_BTN_EDIT:
		BeginEditCurrent();
		break;

	case ZOMBATAR_BTN_DELETE:
		if (GetHeadCount() > 0)
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
		if (mFacialHairPage > 0)
		{
			mFacialHairPage--;
			UpdateButtonState();
		}
		break;

	case ZOMBATAR_BTN_NEXT_PAGE:
		if (mFacialHairPage < 1)
		{
			mFacialHairPage++;
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
			mEditingIndex = -1;
			LoadCurrentToDraft();
			ChangeState(ZOMBATAR_STATE_LIST);
		}
		else
			BackToSelector();
	}
}

void ZombatarWidget::BackToSelector()
{
	mEditingIndex = -1;
	mPage = ZOMBATAR_PAGE_SKIN;
	ChangeState(GetHeadCount() > 0 ? ZOMBATAR_STATE_LIST : ZOMBATAR_STATE_CREATE);
	ResetDraft();
	mGameSelector->SlideTo(0, 0);
	if (mWidgetManager)
		mWidgetManager->SetFocus(mGameSelector);
}
