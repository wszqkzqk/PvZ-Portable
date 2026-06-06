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

#ifndef __CHALLENGESCREEN_H__
#define __CHALLENGESCREEN_H__

#include "../../ConstEnums.h"
#include "widget/Dialog.h"
using namespace Sexy;

#define NUM_CHALLENGE_MODES (static_cast<int>(GameMode::NUM_GAME_MODES) - 1)

class LawnApp;
class ToolTipWidget;
class NewLawnButton;
class ChallengeScreen : public Widget, public ButtonListener
{
private:
    enum
    {
        ChallengeScreen_Back = 100,
        ChallengeScreen_Mode = 200,
        ChallengeScreen_Page = 300
    };

public:
    NewLawnButton*              mBackButton;
    ButtonWidget*               mPageButton[MAX_CHALLANGE_PAGES];
    ButtonWidget*               mChallengeButtons[NUM_CHALLENGE_MODES];
    LawnApp*                    mApp;
    ToolTipWidget*              mToolTip;
    ChallengePage               mPageIndex;
    bool                        mCheatEnableChallenges;
    UnlockingState              mUnlockState;
    int                         mUnlockStateCounter;
    int                         mUnlockChallengeIndex;
    float                       mLockShakeX;
    float                       mLockShakeY;
    bool                        mLimboPageUnlocked;
    int                         mClickCount;
    uint32_t                    mLastClickTime;

public:
    ChallengeScreen(LawnApp* theApp, ChallengePage thePage);
    virtual ~ChallengeScreen();
    void                        SetUnlockChallengeIndex(ChallengePage thePage, bool theIsIZombie = false);
    int                         MoreTrophiesNeeded(int theChallengeIndex);
    /*inline*/ bool             ShowPageButtons();
    void                        UpdateButtons();
    int                         AccomplishmentsNeeded(int theChallengeIndex);
    void                        DrawButton(Graphics* g, int theChallengeIndex);
    virtual void                Draw(Graphics* g);
    virtual void                Update();
    virtual void                AddedToManager(WidgetManager* theWidgetManager);
    virtual void                RemovedFromManager(WidgetManager* theWidgetManager);
    virtual void                ButtonPress(int theId);
    virtual void                ButtonDownTick(int){}
    virtual void                ButtonMouseEnter(int){}
    virtual void                ButtonMouseLeave(int){}
    virtual void                ButtonMouseMove(int, int, int){}
    virtual void                ButtonDepress(int theId);
    void                        UpdateToolTip();
    virtual void                MouseDown(int x, int y, int theClickCount);
//  virtual void                KeyChar(char theChar);

    /*inline*/ bool             IsScaryPotterLevel(GameMode theGameMode);
    /*inline*/ bool             IsIZombieLevel(GameMode theGameMode);
};

class ChallengeDefinition
{
public:
    GameMode                    mChallengeMode;
    int                         mChallengeIconIndex;
    ChallengePage               mPage;
    int                         mRow;
    int                         mCol;
    const char*             mChallengeName;
};
extern ChallengeDefinition gChallengeDefs[NUM_CHALLENGE_MODES];

ChallengeDefinition& GetChallengeDefinition(int theChallengeMode);

#endif
