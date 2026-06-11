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

#ifndef __GAMESELECTOR_H__
#define __GAMESELECTOR_H__

#include "../../ConstEnums.h"
#include "widget/Widget.h"
#include "widget/ButtonListener.h"
#include "AchievementsScreen.h"
#include "GameButton.h"

class LawnApp;
class ToolTipWidget;
class ZombatarWidget;
namespace Sexy
{
    class DialogButton;
}

using namespace Sexy;

enum SelectorAnimState
{
    SELECTOR_OPEN,
    SELECTOR_NEW_USER,
    SELECTOR_SHOW_SIGN,
    SELECTOR_IDLE
};

class GameSelector : public Widget, public ButtonListener
{
private:
    enum
    {
        GameSelector_Adventure = 100,
        GameSelector_Minigame = 101,
        GameSelector_Puzzle = 102,
        GameSelector_Options = 103,
        GameSelector_Help = 104,
        GameSelector_Quit = 105,
        GameSelector_ChangeUser = 106,
        GameSelector_Store = 107,
        GameSelector_Almanac = 108,
        GameSelector_ZenGarden = 109,
        GameSelector_Survival = 110,
        GameSelector_Zombatar = 111, // @Patoke: add stuff after 110
        GameSelector_AchievementsBack = 112,
        GameSelector_Achievements = 113,
        GameSelector_QuickPlay = 114
    };

public:
    LawnApp*                    mApp;
    NewLawnButton*              mAdventureButton;
    NewLawnButton*              mMinigameButton;
    NewLawnButton*              mPuzzleButton;
    NewLawnButton*              mOptionsButton;
    NewLawnButton*              mQuitButton;
    NewLawnButton*              mHelpButton;
    NewLawnButton*              mStoreButton;
    NewLawnButton*              mAlmanacButton;
    NewLawnButton*              mZenGardenButton;
    NewLawnButton*              mSurvivalButton;
    NewLawnButton*              mChangeUserButton;
    NewLawnButton*              mZombatarButton;             //+GOTY @Patoke: 0xC0
    NewLawnButton*              mAchievementsButton;        //+GOTY @Patoke: 0xC4
    NewLawnButton*              mQuickPlayButton;           //+GOTY @Patoke: 0xC8
    Widget*                     mOverlayWidget;
    bool                        mStartingGame;
    int                         mStartingGameCounter;
    bool                        mMinigamesLocked;
    bool                        mPuzzleLocked;
    bool                        mSurvivalLocked;
    bool                        mShowStartButton;
    ParticleSystemID            mTrophyParticleID;
    ReanimationID               mSelectorReanimID;
    ReanimationID               mCloudReanimID[6];
    int                         mCloudCounter[6];
    ReanimationID               mFlowerReanimID[3];
    ReanimationID               mLeafReanimID;
    ReanimationID               mHandReanimID;
    int                         mLeafCounter;
    SelectorAnimState           mSelectorState;
    int                         mLevel;
    bool                        mLoading;
    ToolTipWidget*              mToolTip;
    bool                        mHasTrophy;
    bool                        mUnlockSelectorCheat;
    int                         mSlideCounter;              //+GOTY @Patoke: 0x154
    int                         mStartX;                    //+GOTY @Patoke: 0x158
    int                         mStartY;                    //+GOTY @Patoke: 0x15C
    int                         mDestX;                     //+GOTY @Patoke: 0x160
    int                         mDestY;                     //+GOTY @Patoke: 0x164
    ZombatarWidget*             mZombatarWidget;            //+GOTY @Patoke: 0x168
    AchievementsWidget*         mAchievementsWidget;        //+GOTY @Patoke: 0x16C

public:
    GameSelector(LawnApp* theApp);
    ~GameSelector() override;

    void                        SyncProfile(bool theShowLoading);
    void                        Draw(Graphics* g) override;
    void                        DrawOverlay(Graphics* g) override;
    void                        Update() override;
    void                        AddedToManager(WidgetManager* theWidgetManager) override;
    void                        RemovedFromManager(WidgetManager* theWidgetManager) override;
    void                        OrderInManagerChanged() override;
    void                        ButtonMouseEnter(int theId) override;
    void                        ButtonPress(int theId) override;
    void                        ButtonDepress(int theId) override;
    void                        ButtonDownTick(int) override{}
    void                        ButtonMouseLeave(int) override{}
    void                        ButtonMouseMove(int, int, int) override{}
    void                        KeyDown(KeyCode theKey) override;
    void                        KeyChar(char theChar) override;
    void                        MouseDown(int x, int y, int theClickCount) override;
    void                        TrackButton(DialogButton* theButton, const char* theTrackName, float theOffsetX, float theOffsetY);
    void                        SyncButtons();
    void                        AddTrophySparkle();
    void                        ClickedAdventure();
    void                        UpdateTooltip();
    /*inline*/ bool             ShouldDoZenTuturialBeforeAdventure();
    void                        AddPreviewProfiles();
    // @Patoke: implement functions
    /*inline*/ void             SlideTo(int theX, int theY);
    void                        ShowZombatarScreen();
    void                        ShowAchievementsScreen(); // @Patoke: unofficial name
};

class GameSelectorOverlay : public Widget
{
public:
    GameSelector*               mParent;

public:
    GameSelectorOverlay(GameSelector* theGameSelector);
    ~GameSelectorOverlay() override { }

    void         Draw(Graphics* g) override;
};

#endif
