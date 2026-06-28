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
#include "../Zombie.h"
#include "GameButton.h"
#include "../../LawnApp.h"
#include "ContinueDialog.h"
#include "../../Resources.h"
#include "../../Sexy.TodLib/TodFoley.h"
#include "../../Sexy.TodLib/TodStringFile.h"

// GOTY @Patoke: 0x435E40
ContinueDialog::ContinueDialog(LawnApp* theApp) : LawnDialog(
	theApp, 
	Dialogs::DIALOG_CONTINUE, 
	true, 
	theApp->GetString("CONTINUE_GAME_HEADER", "CONTINUE GAME?"), 
	"", 
	"[DIALOG_BUTTON_CANCEL]", 
	Dialog::BUTTONS_FOOTER)
{
    if (theApp->IsAdventureMode())
    {
        mDialogLines = mApp->GetString("CONTINUE_GAME_OR_RESTART",
            "Do you want to continue your current game or restart the level?");
        mContinueButton = MakeButton(ContinueDialog::ContinueDialog_Continue, this, "[CONTINUE_BUTTON]");
        mNewGameButton = MakeButton(ContinueDialog::ContinueDialog_NewGame, this, "[RESTART_LEVEL]");
    }
    else
    {
        mDialogLines = mApp->GetString("CONTINUE_GAME",
            "Do you want to continue your current game or start a new game?");
        mContinueButton = MakeButton(ContinueDialog::ContinueDialog_Continue, this, "[CONTINUE_BUTTON]");
        mNewGameButton = MakeButton(ContinueDialog::ContinueDialog_NewGame, this, "[NEW_GAME_BUTTON]");
    }

    mTallBottom = true;
    CalcSize(10, 60);

    int aBtnLeft = IMAGE_BUTTON_LEFT->mWidth;
    int aBtnMid = IMAGE_BUTTON_MIDDLE->mWidth;
    int aBtnRight = IMAGE_BUTTON_RIGHT->mWidth;
    int aBtnWidth = aBtnLeft + aBtnMid * 3 + aBtnRight;
    int aInsetH = mContentInsets.mLeft + mContentInsets.mRight + mBackgroundInsets.mLeft + mBackgroundInsets.mRight;
    // Min width that keeps the Continue and New Game buttons from overlapping.
    int aMinCancelWidth = 2 * aBtnWidth - 40;
    int aSteps = (aMinCancelWidth - aBtnLeft - aBtnRight + aBtnMid - 1) / aBtnMid;
    int aMinWidth = aBtnLeft + aBtnRight + aSteps * aBtnMid + aInsetH - 8;
    if (mWidth < aMinWidth)
    {
        int aTopMidWidth = IMAGE_DIALOG_TOPMIDDLE->mWidth;
        int aImageWidth = IMAGE_DIALOG_TOPLEFT->mWidth + IMAGE_DIALOG_TOPRIGHT->mWidth + aTopMidWidth;
        int aWidth = aMinWidth;
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
        Resize(mX, mY, aWidth, mHeight);
    }
}

ContinueDialog::~ContinueDialog()
{
    delete mContinueButton;
    delete mNewGameButton;
}

int ContinueDialog::GetPreferredHeight(int theWidth)
{
    return LawnDialog::GetPreferredHeight(theWidth) + 40;
}

void ContinueDialog::Resize(int theX, int theY, int theWidth, int theHeight)
{
    LawnDialog::Resize(theX, theY, theWidth, theHeight);

    int aBtnWidth = IMAGE_BUTTON_LEFT->mWidth + IMAGE_BUTTON_MIDDLE->mWidth * 3 + IMAGE_BUTTON_RIGHT->mWidth;
    int aBtnHeight = mLawnYesButton->mHeight;

    mContinueButton->Resize(mLawnYesButton->mX - 20, mLawnYesButton->mY - aBtnHeight, aBtnWidth, aBtnHeight);
    mNewGameButton->Resize(mLawnYesButton->mX + mLawnYesButton->mWidth - aBtnWidth + 20, mContinueButton->mY, aBtnWidth, aBtnHeight);
}

void ContinueDialog::AddedToManager(WidgetManager* theWidgetManager)
{
    LawnDialog::AddedToManager(theWidgetManager);
    AddWidget(mContinueButton);
    AddWidget(mNewGameButton);
}

void ContinueDialog::RemovedFromManager(WidgetManager* theWidgetManager)
{
    LawnDialog::RemovedFromManager(theWidgetManager);
    RemoveWidget(mContinueButton);
    RemoveWidget(mNewGameButton);
}

void ContinueDialog::KeyDown(KeyCode theKey)
{
    if (theKey == KeyCode::KEYCODE_ESCAPE)
    {
        ButtonDepress(Dialog::ID_FOOTER);
        return;
    }

    if (theKey == KeyCode::KEYCODE_RETURN || theKey == KeyCode::KEYCODE_SPACE)
    {
        ButtonDepress(ContinueDialog::ContinueDialog_Continue);
        return;
    }

    LawnDialog::KeyDown(theKey);
}

void ContinueDialog::RestartLoopingSounds()
{
    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_RAINING_SEEDS || mApp->IsStormyNightLevel())
    {
        mApp->PlayFoley(FoleyType::FOLEY_RAIN);
    }

    Zombie* aZombie = nullptr;
    while (mApp->mBoard->IterateZombies(aZombie))
    {
        if (aZombie->mPlayingSong)
        {
            aZombie->StartZombieSound();
        }
    }
}

void ContinueDialog::ButtonDepress(int theId)
{
    if (theId == ContinueDialog::ContinueDialog_Continue)
    {
        RestartLoopingSounds();
        mApp->KillDialog(mId);
    }
    else if (theId == ContinueDialog::ContinueDialog_NewGame)
    {
        if (mApp->IsAdventureMode())
        {
            LawnDialog* aDialog = (LawnDialog*)mApp->DoDialog(
                Dialogs::DIALOG_RESTARTCONFIRM, 
                true, 
                "[RESTART_LEVEL_HEADER]", 
                "[RESTART_LEVEL_BODY]", 
                "", 
                Dialog::BUTTONS_OK_CANCEL
            );
            aDialog->mLawnYesButton->mLabel = TodStringTranslate("[RESTART_BUTTON]");
            //aDialog->CalcSize(0, 0);
        }
        else
        {
            LawnDialog* aDialog = (LawnDialog*)mApp->DoDialog(
                Dialogs::DIALOG_RESTARTCONFIRM, 
                true, 
                mApp->GetString("NEW_GAME_HEADER", "New Game?"), 
                mApp->GetString("NEW_GAME", "Are you sure that you want to start a new game?"), 
                "", 
                Dialog::BUTTONS_OK_CANCEL
            );
            aDialog->mLawnYesButton->mLabel = TodStringTranslate("[NEW_GAME_BUTTON]");
            //aDialog->CalcSize(0, 0);
        }
    }
    else
    {
        mApp->KillDialog(mId);
        mApp->mBoardResult = BoardResult::BOARDRESULT_QUIT;
        if (mApp->IsAdventureMode())
        {
            mApp->ShowGameSelector();
        }
        else if (mApp->IsSurvivalMode())
        {
            mApp->KillBoard();
            mApp->ShowChallengeScreen(ChallengePage::CHALLENGE_PAGE_SURVIVAL);
        }
        else if (mApp->IsPuzzleMode())
        {
            mApp->KillBoard();
            mApp->ShowChallengeScreen(ChallengePage::CHALLENGE_PAGE_PUZZLE);
        }
        else
        {
            mApp->KillBoard();
            mApp->ShowChallengeScreen(ChallengePage::CHALLENGE_PAGE_CHALLENGE);
        }
    }
}
