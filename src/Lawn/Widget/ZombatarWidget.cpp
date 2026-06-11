/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "ZombatarWidget.h"
#include "GameSelector.h"
#include "../../LawnApp.h"
#include "../../Resources.h"
#include "../../GameConstants.h"
#include "../System/PlayerInfo.h"
#include "../System/Zombatar.h"
#include "../../SexyAppFramework/graphics/Graphics.h"
#include "../../SexyAppFramework/graphics/MemoryImage.h"
#include "../../SexyAppFramework/imagelib/ImageLib.h"
#include "../../SexyAppFramework/graphics/Font.h"
#include "../../SexyAppFramework/misc/KeyCodes.h"
#include "../../SexyAppFramework/widget/Dialog.h"
#include "../../SexyAppFramework/widget/WidgetManager.h"

#include <algorithm>
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
    mFacialHairPage = 0;
    mMouseX = -1;
    mMouseY = -1;
    mTosScroll = 0;
    mTosAccepted = false;
    Resize(800, 0, BOARD_WIDTH, BOARD_HEIGHT);
    ResetDraft();
}

ZombatarWidget::~ZombatarWidget()
{
}

void ZombatarWidget::Open()
{
    if (!mApp->mPlayerInfo)
        return;

    mCurrentIndex = mApp->mPlayerInfo->mZombatarIndex;
    ClampCurrentIndex();
    if (!mApp->mPlayerInfo->mZombatarAccepted)
    {
        mState = ZOMBATAR_STATE_TOS;
        mTosAccepted = false;
        mTosScroll = 0;
    }
    else if (GetHeadCount() > 0)
    {
        mState = ZOMBATAR_STATE_LIST;
        LoadCurrentToDraft();
    }
    else
    {
        mState = ZOMBATAR_STATE_CREATE;
        ResetDraft();
    }

    mPage = ZOMBATAR_PAGE_SKIN;
    mGameSelector->SlideTo(-BOARD_WIDTH, 0);
    if (mWidgetManager)
        mWidgetManager->SetFocus(this);
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
    if (!aPlayerInfo || !CanSaveNewHead())
        return false;

    size_t aOffset = aPlayerInfo->mZombatarData.size();
    aPlayerInfo->mZombatarData.resize(aOffset + ZOMBATAR_RECORD_SIZE);
    EncodeRecord(aPlayerInfo->mZombatarData.data() + aOffset);
    aPlayerInfo->mZombatarHeadCount = static_cast<uint32_t>(GetHeadCount());
    aPlayerInfo->mZombatarCreatedBefore = 1;
    mCurrentIndex = GetHeadCount() - 1;
    aPlayerInfo->mZombatarIndex = mCurrentIndex;
    if (!ExportAvatarPNG(aPlayerInfo->mZombatarData.data() + aOffset, mCurrentIndex + 1))
    {
        aPlayerInfo->mZombatarData.resize(aOffset);
        aPlayerInfo->mZombatarHeadCount = static_cast<uint32_t>(GetHeadCount());
        aPlayerInfo->mZombatarIndex = GetHeadCount() > 0 ? mCurrentIndex : -1;
        ClampCurrentIndex();
        mApp->LawnMessageBox(DIALOG_MESSAGE, "Zombatar Export Failed", "The Zombatar image could not be written.", "[DIALOG_BUTTON_OK]", "", Dialog::BUTTONS_FOOTER);
        return false;
    }
    aPlayerInfo->SaveDetails();
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
        mState = ZOMBATAR_STATE_LIST;
    }
    else
    {
        ResetDraft();
        mState = ZOMBATAR_STATE_CREATE;
    }

    aPlayerInfo->SaveDetails();
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
    return mPage == ZOMBATAR_PAGE_SKIN || mPart[mPage] >= 0;
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

void ZombatarWidget::DrawButtonImage(Graphics* g, Image* theImage, Image* theHighlightImage, const Rect& theRect, bool theEnabled)
{
    Image* aImage = theEnabled && theHighlightImage && theRect.Contains(mMouseX, mMouseY) ? theHighlightImage : theImage;
    if (aImage)
        g->DrawImage(aImage, theRect.mX, theRect.mY);
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
    for (int i = 0; i < aExportImage.mWidth * aExportImage.mHeight; i++)
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
    if (mState == ZOMBATAR_STATE_TOS)
        DrawTOS(g);
    else if (mState == ZOMBATAR_STATE_LIST)
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

void ZombatarWidget::DrawTOS(Graphics* g)
{
    g->SetFont(FONT_DWARVENTODCRAFT18);
    g->SetColor(Color(255, 255, 255));
    g->DrawString("Zombatar", 292, 132);
    g->SetFont(FONT_DWARVENTODCRAFT12);
    g->PushState();
    g->ClipRect(Rect(176, 170, 430, 192));
    g->WriteWordWrapped(Rect(188, 176 - mTosScroll, 390, 700),
        "Create custom zombie avatars for this profile. Saved Zombatars stay in the user file and can be edited from the main menu. Zombatar portraits are stored with this user profile and can be exported as PNG images when they are saved.",
        18);
    g->PopState();
    g->DrawImage(IMAGE_ZOMBATAR_TOS_SLIDER, 620, 166);
    int aThumbY = 166 + (mTosScroll * 170) / 510;
    g->DrawImage(IMAGE_ZOMBATAR_TOS_SLIDER_THUMB, 613, aThumbY);
    g->DrawImage(IMAGE_ZOMBATAR_TOS_ARROW, 584, aThumbY + 7);
    g->SetColor(Color(255, 255, 255));
    g->DrawRect(Rect(246, 382, 22, 22));
    if (mTosAccepted)
    {
        g->FillRect(251, 387, 12, 12);
    }
    g->DrawString("I accept", 278, 400);
    DrawButtonImage(g, IMAGE_ZOMBATAR_BEGIN_BUTTON, IMAGE_ZOMBATAR_BEGIN_BUTTON_HIGHLIGHT, Rect(300, 405, 210, 80), mTosAccepted);
    DrawButtonImage(g, IMAGE_ZOMBATAR_BACK_BUTTON, IMAGE_ZOMBATAR_BACK_BUTTON_HIGHLIGHT, Rect(30, 510, 160, 62), true);
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
    if (mCurrentIndex > 0)
        DrawButtonImage(g, IMAGE_ZOMBATAR_PREV_BUTTON, IMAGE_ZOMBATAR_PREV_BUTTON_HIGHLIGHT, Rect(164, 250, 80, 80), true);
    if (mCurrentIndex + 1 < aCount)
        DrawButtonImage(g, IMAGE_ZOMBATAR_NEXT_BUTTON, IMAGE_ZOMBATAR_NEXT_BUTTON_HIGHLIGHT, Rect(588, 250, 80, 80), true);
    DrawButtonImage(g, IMAGE_ZOMBATAR_NEWZOMBIE_BUTTON, IMAGE_ZOMBATAR_NEWZOMBIE_BUTTON_HIGHLIGHT, Rect(260, 440, 230, 70), CanSaveNewHead());
    DrawButtonImage(g, IMAGE_ZOMBATAR_BACK_BUTTON, IMAGE_ZOMBATAR_BACK_BUTTON_HIGHLIGHT, Rect(30, 510, 160, 62), true);
    g->SetFont(FONT_DWARVENTODCRAFT12);
    g->DrawString("Delete", 522, 514);
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

    if (mPage == ZOMBATAR_PAGE_FACIAL_HAIR)
    {
        DrawButtonImage(g, IMAGE_ZOMBATAR_PREV_BUTTON, IMAGE_ZOMBATAR_PREV_BUTTON_HIGHLIGHT, Rect(366, 392, 80, 60), mFacialHairPage > 0);
        DrawButtonImage(g, IMAGE_ZOMBATAR_NEXT_BUTTON, IMAGE_ZOMBATAR_NEXT_BUTTON_HIGHLIGHT, Rect(646, 392, 80, 60), mFacialHairPage == 0);
    }

    int aColorCount = mPage == ZOMBATAR_PAGE_SKIN ? 12 : 18;
    if (PageAllowsColors())
    {
        for (int i = 0; i < aColorCount; i++)
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

    DrawButtonImage(g, mState == ZOMBATAR_STATE_CONFIRM ? IMAGE_ZOMBATAR_ACCEPT_BUTTON : IMAGE_ZOMBATAR_FINISHED_BUTTON, mState == ZOMBATAR_STATE_CONFIRM ? IMAGE_ZOMBATAR_ACCEPT_BUTTON_HIGHLIGHT : IMAGE_ZOMBATAR_FINISHED_BUTTON_HIGHLIGHT, Rect(554, 522, 210, 62), CanSaveNewHead());
    if (GetHeadCount() > 0 && mState == ZOMBATAR_STATE_CREATE)
        DrawButtonImage(g, IMAGE_ZOMBATAR_VIEW_BUTTON, IMAGE_ZOMBATAR_VIEW_BUTTON_HIGHLIGHT, Rect(346, 522, 160, 62), true);
    DrawButtonImage(g, IMAGE_ZOMBATAR_BACK_BUTTON, IMAGE_ZOMBATAR_BACK_BUTTON_HIGHLIGHT, Rect(30, 510, 160, 62), true);
    if (mState == ZOMBATAR_STATE_CONFIRM)
    {
        g->SetFont(FONT_DWARVENTODCRAFT12);
        g->SetColor(Color(255, 255, 255));
        g->WriteWordWrapped(Rect(348, 220, 355, 44), "Save this Zombatar to your profile?", 18);
    }
}

void ZombatarWidget::MouseMove(int x, int y)
{
    mMouseX = x;
    mMouseY = y;
}

void ZombatarWidget::MouseDrag(int x, int y)
{
    MouseMove(x, y);
    if (mState == ZOMBATAR_STATE_TOS && Rect(600, 160, 54, 220).Contains(x, y))
        mTosScroll = ClampRange(((y - 166) * 510) / 170, 0, 510);
}

void ZombatarWidget::MouseWheel(int theDelta)
{
    if (mState == ZOMBATAR_STATE_TOS)
        mTosScroll = ClampRange(mTosScroll - theDelta * 20, 0, 510);
}

void ZombatarWidget::MouseDown(int x, int y, int theClickCount)
{
    (void)theClickCount;
    if (!mApp->mPlayerInfo)
        return;

    if (Rect(30, 510, 160, 62).Contains(x, y))
    {
        if (mState == ZOMBATAR_STATE_CONFIRM)
            mState = ZOMBATAR_STATE_CREATE;
        else
            BackToSelector();
        mApp->PlaySample(SOUND_GRAVEBUTTON);
        return;
    }

    if (mState == ZOMBATAR_STATE_TOS)
    {
        if (Rect(246, 382, 160, 28).Contains(x, y))
        {
            mTosAccepted = !mTosAccepted;
            mApp->PlaySample(SOUND_GRAVEBUTTON);
        }
        else if (Rect(600, 160, 54, 220).Contains(x, y))
        {
            mTosScroll = ClampRange(((y - 166) * 510) / 170, 0, 510);
        }
        else if (Rect(300, 405, 210, 80).Contains(x, y) && mTosAccepted)
        {
            mApp->mPlayerInfo->mZombatarAccepted = 1;
            mApp->mPlayerInfo->SaveDetails();
            mState = GetHeadCount() > 0 ? ZOMBATAR_STATE_LIST : ZOMBATAR_STATE_CREATE;
            mApp->PlaySample(SOUND_GRAVEBUTTON);
        }
        return;
    }

    if (mState == ZOMBATAR_STATE_LIST)
    {
        int aCount = GetHeadCount();
        if (Rect(164, 250, 80, 80).Contains(x, y) && mCurrentIndex > 0)
        {
            mCurrentIndex--;
            mApp->mPlayerInfo->mZombatarIndex = mCurrentIndex;
            LoadCurrentToDraft();
        }
        else if (Rect(588, 250, 80, 80).Contains(x, y) && mCurrentIndex + 1 < aCount)
        {
            mCurrentIndex++;
            mApp->mPlayerInfo->mZombatarIndex = mCurrentIndex;
            LoadCurrentToDraft();
        }
        else if (Rect(260, 440, 230, 70).Contains(x, y))
        {
            if (CanSaveNewHead())
            {
                ResetDraft();
                mState = ZOMBATAR_STATE_CREATE;
            }
            else
            {
                ShowMaxHeadsMessage();
            }
        }
        else if (Rect(510, 440, 110, 70).Contains(x, y))
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
        mApp->PlaySample(SOUND_GRAVEBUTTON);
        return;
    }

    if (mState == ZOMBATAR_STATE_CONFIRM)
    {
        if (Rect(554, 522, 210, 62).Contains(x, y))
        {
            if (CanSaveNewHead())
            {
                if (SaveDraft())
                    mState = ZOMBATAR_STATE_LIST;
            }
            else
            {
                ShowMaxHeadsMessage();
            }
            mApp->PlaySample(SOUND_GRAVEBUTTON);
        }
        return;
    }

    for (int i = 0; i < NUM_ZOMBATAR_PAGES; i++)
    {
        if (GetCategoryRect(i).Contains(x, y))
        {
            mPage = static_cast<ZombatarPage>(i);
            return;
        }
    }

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
            return;
        }
    }

    if (PageAllowsNone() && GetItemRect(aItemCount).Contains(x, y))
    {
        mPart[mPage] = -1;
        return;
    }

    if (mPage == ZOMBATAR_PAGE_FACIAL_HAIR)
    {
        if (Rect(366, 392, 80, 60).Contains(x, y))
            mFacialHairPage = 0;
        else if (Rect(646, 392, 80, 60).Contains(x, y))
            mFacialHairPage = 1;
    }

    if (PageAllowsColors())
    {
        int aColorCount = mPage == ZOMBATAR_PAGE_SKIN ? 12 : 18;
        for (int i = 0; i < aColorCount; i++)
        {
            if (GetColorRect(i).Contains(x, y))
            {
                mColor[mPage] = i;
                return;
            }
        }
    }

    if (Rect(554, 522, 210, 62).Contains(x, y))
    {
        if (CanSaveNewHead())
            mState = ZOMBATAR_STATE_CONFIRM;
        else
            ShowMaxHeadsMessage();
        mApp->PlaySample(SOUND_GRAVEBUTTON);
        return;
    }

    if (Rect(346, 522, 160, 62).Contains(x, y) && GetHeadCount() > 0)
    {
        mState = ZOMBATAR_STATE_LIST;
        ClampCurrentIndex();
        LoadCurrentToDraft();
        mApp->PlaySample(SOUND_GRAVEBUTTON);
    }
}

void ZombatarWidget::KeyDown(KeyCode theKey)
{
    if (theKey == KEYCODE_ESCAPE)
    {
        if (mState == ZOMBATAR_STATE_CONFIRM)
            mState = ZOMBATAR_STATE_CREATE;
        else
            BackToSelector();
    }
}

void ZombatarWidget::BackToSelector()
{
    mState = GetHeadCount() > 0 ? ZOMBATAR_STATE_LIST : ZOMBATAR_STATE_CREATE;
    mPage = ZOMBATAR_PAGE_SKIN;
    ResetDraft();
    mGameSelector->SlideTo(0, 0);
    if (mWidgetManager)
        mWidgetManager->SetFocus(mGameSelector);
}
