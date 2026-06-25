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

#include "../Board.h"
#include "LawnDialog.h"
#include "GameButton.h"
#include "../../LawnApp.h"
#include "../LawnCommon.h"
#include "../../Resources.h"
#include "ChallengeScreen.h"
#include "../../Sexy.TodLib/TodDebug.h"
#include "../../Sexy.TodLib/Reanimator.h"
#include "../../Sexy.TodLib/EffectSystem.h"
#include "../../Sexy.TodLib/TodStringFile.h"
#include "graphics/ImageFont.h"


LawnDialog::LawnDialog(LawnApp* theApp, int theId, bool isModal, const std::string& theDialogHeader, const std::string& theDialogLines, const std::string& theDialogFooter, int theButtonMode) :
	Dialog(nullptr, nullptr, theId, isModal, theDialogHeader, theDialogLines, "", BUTTONS_NONE)
{
    mApp = theApp;
    mButtonDelay = -1;
    mReanimation = new ReanimationWidget();
    mReanimation->mLawnDialog = this;
    mDrawStandardBack = true;
    mTallBottom = false;
    mVerticalCenterText = true;
    mDialogHeader = TodStringTranslate(theDialogHeader);
    mDialogLines = TodStringTranslate(theDialogLines);
    SetColor(0, { 0xE0,0xBB,0x62 });
    SetColor(1, { 0xE0,0xBB,0x62 });
    SetHeaderFont(Sexy::FONT_DWARVENTODCRAFT24);
    SetLinesFont(Sexy::FONT_DWARVENTODCRAFT15);
    mContentInsets = Insets(36, 35, 46, 36);

    // @Patoke: these dialogs had the wrong local name
    if (theButtonMode == 1)
    {
        mLawnYesButton = MakeButton(1000, this, mApp->GetString("BUTTON_YES", "Yes"));
        mLawnNoButton = MakeButton(1001, this, mApp->GetString("BUTTON_NO", "No"));
    }
    else if (theButtonMode == 2)
    {
        mLawnYesButton = MakeButton(1000, this, mApp->GetString("BUTTON_OK", "Ok"));
        mLawnNoButton = MakeButton(1001, this, mApp->GetString("BUTTON_CANCEL", "Cancel"));
    }
    else if (theButtonMode == 3)
    {
        mLawnYesButton = MakeButton(1000, this, theDialogFooter);
        mLawnNoButton = nullptr;
    }
    else
    {
        mLawnYesButton = nullptr;
        mLawnNoButton = nullptr;
    }

    mApp->SetCursor(CURSOR_POINTER);
    CalcSize(0, 0);
}

LawnDialog::~LawnDialog()
{
    if (mReanimation) delete mReanimation;
    if (mLawnYesButton) delete mLawnYesButton;
    if (mLawnNoButton) delete mLawnNoButton;
}

int LawnDialog::GetLeft()
{
    return mContentInsets.mLeft + mBackgroundInsets.mLeft;
}

int LawnDialog::GetWidth()
{
    return mWidth - mContentInsets.mLeft - mContentInsets.mRight - mBackgroundInsets.mLeft - mBackgroundInsets.mRight;
}

int LawnDialog::GetTop()
{
    return mContentInsets.mTop + mBackgroundInsets.mTop + 99;
}

void LawnDialog::CalcSize(int theExtraX, int theExtraY)
{
    // 根据边距计算一个最小宽度
    int aWidth = mBackgroundInsets.mLeft + mBackgroundInsets.mRight + mContentInsets.mLeft + mContentInsets.mRight + theExtraX;
    // 最小宽度需要额外考虑到标题字符串的宽度
    if (mDialogHeader.size() > 0) aWidth += mHeaderFont->StringWidth(mDialogHeader);
    // 对话框贴图的最小宽度
    int aTopMidWidth = Sexy::IMAGE_DIALOG_TOPMIDDLE->mWidth;
    int aImageWidth = Sexy::IMAGE_DIALOG_TOPLEFT->mWidth + Sexy::IMAGE_DIALOG_TOPRIGHT->mWidth + aTopMidWidth;
    // 计算最终的宽度
    if (aWidth <= aImageWidth)
    {
        aWidth = aImageWidth;
    }
    else if (aTopMidWidth > 0)
    {
        int anExtraWidth = (aWidth - aImageWidth) % aTopMidWidth;
        if (anExtraWidth)
        {
            aWidth += aTopMidWidth - anExtraWidth;
        }
    }

    // 根据边距计算一个最小高度
    int aHeight = mBackgroundInsets.mTop + mBackgroundInsets.mBottom + mContentInsets.mTop + mContentInsets.mBottom + theExtraY + DIALOG_HEADER_OFFSET;
    // 额外考虑到标题字符串的高度
    if (mDialogHeader.size() > 0)
    {
        aHeight += -mHeaderFont->GetAscentPadding() + mHeaderFont->GetHeight() + mSpaceAfterHeader;
    }
    // 对话框宽度和高度额外考虑正文文本
    if (mDialogLines.size() > 0)
    {
        aWidth += aTopMidWidth;
        Graphics g;
        g.SetFont(mLinesFont);
        int aBasicWidth = aWidth - mBackgroundInsets.mLeft - mBackgroundInsets.mRight - mContentInsets.mLeft - mContentInsets.mRight - 4;
        aHeight += GetWordWrappedHeight(&g, aBasicWidth, mDialogLines, mLinesFont->GetLineSpacing() + mLineSpacingOffset) + 30;
    }
    aHeight += mButtonHeight;
    // 对话框贴图的最小高度
    int aBottomHeight = (mTallBottom ? Sexy::IMAGE_DIALOG_BIGBOTTOMLEFT : Sexy::IMAGE_DIALOG_BOTTOMLEFT)->mHeight;
    int aImageHeight = Sexy::IMAGE_DIALOG_TOPLEFT->mHeight + aBottomHeight + DIALOG_HEADER_OFFSET;
    // 计算最终的高度
    if (aHeight < aImageHeight)
    {
        aHeight = aImageHeight;
    }
    else
    {
        int aCenterHeight = Sexy::IMAGE_DIALOG_CENTERLEFT->mHeight;
        int anExtraHeight = (aHeight - aImageHeight) % aCenterHeight;
        if (anExtraHeight) aHeight += aCenterHeight - anExtraHeight;
    }

    Resize(mX, mY, aWidth, aHeight);
}

void LawnDialog::AddedToManager(WidgetManager* theWidgetManager)
{
    Dialog::AddedToManager(theWidgetManager);
    AddWidget(mReanimation);
    if (mLawnYesButton) AddWidget(mLawnYesButton);
    if (mLawnNoButton) AddWidget(mLawnNoButton);
}

void LawnDialog::RemovedFromManager(WidgetManager* theWidgetManager)
{
    Dialog::RemovedFromManager(theWidgetManager);
    if (mLawnYesButton) RemoveWidget(mLawnYesButton);
    if (mLawnNoButton) RemoveWidget(mLawnNoButton);
    RemoveWidget(mReanimation);

    if (mReanimation->mReanim)
    {
        mApp->mEffectSystem->mReanimationHolder->mReanimations.DataArrayFree(mReanimation->mReanim);
        mReanimation->mReanim = nullptr;
    }
}

void LawnDialog::SetButtonDelay(int theDelay)
{
    mButtonDelay = theDelay;
    if (mLawnYesButton) mLawnYesButton->SetDisabled(true);
    if (mLawnNoButton) mLawnNoButton->SetDisabled(true);
}

void LawnDialog::Update()
{
    Dialog::Update();
    if (!mButtonDelay)
    {
        if (mLawnYesButton) mLawnYesButton->SetDisabled(false);
        if (mLawnNoButton) mLawnNoButton->SetDisabled(false);
    }
    MarkDirty();
}

void LawnDialog::ButtonPress(int theId)
{
    (void)theId;
    mApp->PlaySample(Sexy::SOUND_GRAVEBUTTON);
}

void LawnDialog::ButtonDepress(int theId)
{
    if (mUpdateCnt > mButtonDelay)
    {
        Dialog::ButtonDepress(theId);
    }
}

void LawnDialog::CheckboxChecked()
{
    mApp->PlaySample(Sexy::SOUND_BUTTONCLICK);
}

void LawnDialog::KeyDown(KeyCode theKey)
{
    if (mId == DIALOG_PAUSED && mApp->mBoard)
    {
        mApp->mBoard->DoTypingCheck(theKey);
    }

    if (mId != DIALOG_ALMANAC)
    {
        if (theKey == KEYCODE_SPACE || theKey == KEYCODE_RETURN || theKey == 'y' || theKey == 'Y')
        {
            Dialog::ButtonDepress(Dialog::ID_YES);
        }
        else if (theKey == KEYCODE_ESCAPE || theKey == 'n' || theKey == 'N')
        {
            if (mLawnNoButton)
            {
                Dialog::ButtonDepress(Dialog::ID_NO);
            }
            else if (theKey == KEYCODE_ESCAPE && mLawnYesButton)
            {
                Dialog::ButtonDepress(Dialog::ID_YES);
            }
        }
    }
}

void LawnDialog::Resize(int theX, int theY, int theWidth, int theHeight)
{
    Dialog::Resize(theX, theY, theWidth, theHeight);

    int aButtonAreaX = mContentInsets.mLeft + mBackgroundInsets.mLeft - 5;
    int aButtonAreaY = mHeight - mContentInsets.mBottom - mBackgroundInsets.mBottom - IMAGE_BUTTON_LEFT->mHeight + 2;
    int aButtonAreaWidth = mWidth - mContentInsets.mRight - mBackgroundInsets.mRight - mBackgroundInsets.mLeft - mContentInsets.mLeft + 8;
    // 按钮贴图限制下，按钮的最小宽度
    int aButtonMinWidth = IMAGE_BUTTON_LEFT->mWidth + IMAGE_BUTTON_RIGHT->mWidth;
    // 按钮区域额外空余的宽度
    int aBtnMidWidth = IMAGE_BUTTON_MIDDLE->mWidth;
    int aButtonExtraWidth = (aButtonAreaWidth - 10) / 2 - aBtnMidWidth - aButtonMinWidth + 1;
    // 计算按钮可以拓展的额外宽度
    if (aButtonExtraWidth <= 0)
    {
        aButtonExtraWidth = 0;
    }
    else if (aBtnMidWidth > 0)
    {
        int anExtraWidth = aButtonExtraWidth % aBtnMidWidth;
        if (anExtraWidth)
        {
            // 不足中部贴图宽度的部分补充至中部贴图宽度
            aButtonExtraWidth += aBtnMidWidth - anExtraWidth;
        }
    }
    // 最终宽度
    int aButtonWidth = aButtonMinWidth + aButtonExtraWidth;

    if (mTallBottom)
    {
        aButtonAreaY += 5;
    }

    if (mLawnYesButton && mLawnNoButton)
    {
        mLawnYesButton->Resize(aButtonAreaX, aButtonAreaY, aButtonWidth, IMAGE_BUTTON_LEFT->mHeight);
        mLawnNoButton->Resize(aButtonAreaWidth - aButtonWidth + aButtonAreaX, aButtonAreaY, aButtonWidth, IMAGE_BUTTON_LEFT->mHeight);
    }
    else if (mLawnYesButton)
    {
        aButtonExtraWidth = aButtonAreaWidth - aBtnMidWidth - aButtonMinWidth + 1;
        if (aButtonExtraWidth <= 0)
        {
            aButtonExtraWidth = 0;
        }
        else if (aBtnMidWidth > 0)
        {
            int anExtraWidth = aButtonExtraWidth % aBtnMidWidth;
            if (anExtraWidth)
            {
                aButtonExtraWidth += aBtnMidWidth - anExtraWidth;
            }
        }
        int aButtonWidth = aButtonMinWidth + aButtonExtraWidth;

        mLawnYesButton->Resize(aButtonAreaX + (aButtonAreaWidth - aButtonWidth) / 2, aButtonAreaY, aButtonWidth, IMAGE_BUTTON_LEFT->mHeight);
    }

    if (mReanimation->mReanim)
    {
        mReanimation->Resize(mReanimation->mPosX, mReanimation->mPosY + DIALOG_HEADER_OFFSET, mReanimation->mWidth, mReanimation->mHeight);
    }
}

void LawnDialog::Draw(Graphics* g)
{
    if (!mDrawStandardBack)
        return;

    Image* aBottomLeftImage = IMAGE_DIALOG_BOTTOMLEFT;
    Image* aBottomMiddleImage = IMAGE_DIALOG_BOTTOMMIDDLE;
    Image* aBottomRightImage = IMAGE_DIALOG_BOTTOMRIGHT;
    if (mTallBottom)
    {
        aBottomLeftImage = IMAGE_DIALOG_BIGBOTTOMLEFT;
        aBottomMiddleImage = IMAGE_DIALOG_BIGBOTTOMMIDDLE;
        aBottomRightImage = IMAGE_DIALOG_BIGBOTTOMRIGHT;
    }

    int aRepeatX = (mWidth - IMAGE_DIALOG_TOPRIGHT->mWidth - IMAGE_DIALOG_TOPLEFT->mWidth) / IMAGE_DIALOG_TOPMIDDLE->mWidth;
    int aRepeatY = (mHeight - IMAGE_DIALOG_TOPLEFT->mHeight - aBottomLeftImage->mHeight - DIALOG_HEADER_OFFSET) / IMAGE_DIALOG_CENTERLEFT->mHeight;

    int aPosX = 0;
    int aPosY = DIALOG_HEADER_OFFSET;
    g->DrawImage(IMAGE_DIALOG_TOPLEFT, aPosX, aPosY);
    aPosX += IMAGE_DIALOG_TOPLEFT->mWidth;
    for (int i = 0; i < aRepeatX; i++)
    {
        g->DrawImage(IMAGE_DIALOG_TOPMIDDLE, aPosX, aPosY);
        aPosX += IMAGE_DIALOG_TOPMIDDLE->mWidth;
    }
    g->DrawImage(IMAGE_DIALOG_TOPRIGHT, aPosX, aPosY);

    aPosY += IMAGE_DIALOG_TOPRIGHT->mHeight;
    for (int y = 0; y < aRepeatY; y++)
    {
        aPosX = 0;

        g->DrawImage(IMAGE_DIALOG_CENTERLEFT, aPosX, aPosY);
        aPosX += IMAGE_DIALOG_CENTERLEFT->mWidth;

        for (int x = 0; x < aRepeatX; x++)
        {
            g->DrawImage(IMAGE_DIALOG_CENTERMIDDLE, aPosX, aPosY);
            aPosX += IMAGE_DIALOG_CENTERMIDDLE->mWidth;
        }

        g->DrawImage(IMAGE_DIALOG_CENTERRIGHT, aPosX, aPosY);
        aPosY += IMAGE_DIALOG_CENTERLEFT->mHeight;
    }

    aPosX = 0;
    g->DrawImage(aBottomLeftImage, aPosX, aPosY);
    aPosX += aBottomLeftImage->mWidth;
    for (int i = 0; i < aRepeatX; i++)
    {
        g->DrawImage(aBottomMiddleImage, aPosX, aPosY);
        aPosX += aBottomMiddleImage->mWidth;
    }
    g->DrawImage(aBottomRightImage, aPosX, aPosY);
    g->DrawImage(IMAGE_DIALOG_HEADER, (mWidth - IMAGE_DIALOG_HEADER->mWidth) / 2 - 5, 0);

    int aFontY = mContentInsets.mTop + mBackgroundInsets.mTop + DIALOG_HEADER_OFFSET;
    if (mDialogHeader.size() > 0)
    {
        int aOffsetY = aFontY - mHeaderFont->GetAscentPadding() + mHeaderFont->GetAscent();
        g->SetFont(mHeaderFont);
        g->SetColor(mColors[Dialog::COLOR_HEADER]);
        WriteCenteredLine(g, aOffsetY, mDialogHeader);
        aFontY = aOffsetY - mHeaderFont->GetAscent() + mHeaderFont->GetHeight() + mSpaceAfterHeader;
    }

    g->SetFont(mLinesFont);
    g->SetColor(mColors[Dialog::COLOR_LINES]);
    int aLinesAreaWidth = mWidth - mContentInsets.mLeft - mContentInsets.mRight - mBackgroundInsets.mLeft - mBackgroundInsets.mRight - 4;
    Rect aRect(mBackgroundInsets.mLeft + mContentInsets.mLeft + 2, aFontY, aLinesAreaWidth, 0);
    if (mVerticalCenterText)
    {
        int aLinesHeight = GetWordWrappedHeight(g, aLinesAreaWidth, mDialogLines, mLinesFont->GetLineSpacing() + mLineSpacingOffset);
        int aLinesAreaHeight = mHeight - mContentInsets.mBottom - mBackgroundInsets.mBottom - mButtonHeight - aFontY - 55;
        if (mTallBottom)
        {
            aLinesAreaHeight -= 36;
        }
        aRect.mY += (aLinesAreaHeight - aLinesHeight) / 2;
    }
    WriteWordWrapped(g, aRect, mDialogLines, mLinesFont->GetLineSpacing() + mLineSpacingOffset, mTextAlign);
}

ReanimationWidget::ReanimationWidget()
{
    mApp = (LawnApp*)gSexyAppBase;
    mPosX = 0.0f;
    mPosY = 0.0f;
    mReanim = nullptr;
    mMouseVisible = false;
    mClip = false;
    mLawnDialog = nullptr;
    mHasAlpha = true;
}

ReanimationWidget::~ReanimationWidget()
{
    Dispose();
}

void ReanimationWidget::Dispose()
{
    if (mReanim)
    {
        mApp->mEffectSystem->mReanimationHolder->mReanimations.DataArrayFree(mReanim);
        mReanim = nullptr;
    }
}

void ReanimationWidget::AddReanimation(float x, float y, ReanimationType theReanimationType)
{
    TOD_ASSERT(mReanim == nullptr);
    
    mPosX = x;
    mPosY = y;
    mReanim = mApp->mEffectSystem->mReanimationHolder->AllocReanimation(x, y, 0, theReanimationType);
    mReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
    mReanim->mIsAttachment = true;
    if (mReanim->TrackExists("anim_idle"))
        mReanim->SetFramesForLayer("anim_idle");

    Resize(x, y, 10, 10);
}

void ReanimationWidget::Draw(Graphics* g)
{
    if (mReanim)
        mReanim->Draw(g);
}

void ReanimationWidget::Update()
{
    if (mReanim)
    {
        mReanim->Update();
        MarkDirty();
    }
}

GameOverDialog::GameOverDialog(const std::string& theMessage, bool theShowChallengeName) : LawnDialog(
    gLawnApp, 
    Dialogs::DIALOG_GAME_OVER, 
    true, 
    "[GAME_OVER]", 
    theMessage,
    "", 
    Dialog::BUTTONS_FOOTER)
{
    mMenuButton = nullptr;
    mLawnYesButton->SetLabel("[TRY_AGAIN]");
    if (theShowChallengeName)
    {
        mDialogHeader = TodStringTranslate(mApp->GetCurrentChallengeDef().mChallengeName);
    }

    if (theMessage.size() == 0)
    {
        mContentInsets.mTop += 15;
    }
    CalcSize(0, 0);
    mApp->CenterDialog(this, mWidth, mHeight);
    mClip = false;

    mMenuButton = MakeButton(1, this, "[MAIN_MENU_BUTTON]");
    mMenuButton->Resize(635 - mX, -10 - mY, 163, 46);

    gLawnApp->mBoard->mShowShovel = false;
    gLawnApp->mBoard->mMenuButton->mBtnNoDraw = true;
}

GameOverDialog::~GameOverDialog()
{
    delete mMenuButton;
}

void GameOverDialog::KeyDown(KeyCode theKey)
{
    if (theKey == KeyCode::KEYCODE_ESCAPE)
    {
        ButtonDepress(1);
        return;
    }

    LawnDialog::KeyDown(theKey);
}

void GameOverDialog::ButtonDepress(int theId)
{
    if (theId == 1)
    {
        mApp->KillDialog(Dialogs::DIALOG_GAME_OVER);
        mApp->KillBoard();
        if (mApp->IsSurvivalMode())
        {
            mApp->ShowChallengeScreen(ChallengePage::CHALLENGE_PAGE_SURVIVAL);
        }
        else if (mApp->IsPuzzleMode())
        {
            mApp->ShowChallengeScreen(ChallengePage::CHALLENGE_PAGE_PUZZLE);
        }
        else if (mApp->IsAdventureMode())
        {
            mApp->ShowGameSelector();
        }
        else
        {
            mApp->ShowChallengeScreen(ChallengePage::CHALLENGE_PAGE_CHALLENGE);
        }
    }
    else if (theId == Dialog::ID_FOOTER)
    {
        mApp->KillDialog(Dialogs::DIALOG_GAME_OVER);
        mApp->EndLevel();
    }
}

void GameOverDialog::AddedToManager(WidgetManager* theWidgetManager)
{
    LawnDialog::AddedToManager(theWidgetManager);
    if (mMenuButton)
    {
        AddWidget(mMenuButton);
    }
}

void GameOverDialog::RemovedFromManager(WidgetManager* theWidgetManager)
{
    LawnDialog::RemovedFromManager(theWidgetManager);
    if (mMenuButton)
    {
        RemoveWidget(mMenuButton);
    }
}

void GameOverDialog::MouseDrag(int x, int y)
{
    LawnDialog::MouseDrag(x, y);
    if (mMenuButton)
    {
        mMenuButton->Resize(635 - mX, -10 - mY, 163, 46);
    }
}
