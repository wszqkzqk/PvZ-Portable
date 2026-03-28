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

#include "GameButton.h"
#include "../../LawnApp.h"
#include "../System/Music.h"
#include "ChallengeScreen.h"
#include "../../Resources.h"
#include "../ToolTipWidget.h"
#include "../System/PlayerInfo.h"
#include "../../Sexy.TodLib/TodDebug.h"
#include "../../Sexy.TodLib/TodFoley.h"
#include "../../Sexy.TodLib/TodCommon.h"
#include "misc/Debug.h"
#include "../../Sexy.TodLib/TodStringFile.h"
#include "widget/WidgetManager.h"
#include <SDL.h>

constinit const ChallengeDefinition gChallengeDefs[NUM_CHALLENGE_MODES] = {
	{ .mChallengeMode = GameMode::GAMEMODE_SURVIVAL_NORMAL_STAGE_1, .mChallengeIconIndex = 0, .mPage = ChallengePage::CHALLENGE_PAGE_SURVIVAL, .mRow = 0, .mCol = 0, .mChallengeName = "[SURVIVAL_DAY_NORMAL]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SURVIVAL_NORMAL_STAGE_2, .mChallengeIconIndex = 1, .mPage = ChallengePage::CHALLENGE_PAGE_SURVIVAL, .mRow = 0, .mCol = 1, .mChallengeName = "[SURVIVAL_NIGHT_NORMAL]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SURVIVAL_NORMAL_STAGE_3, .mChallengeIconIndex = 2, .mPage = ChallengePage::CHALLENGE_PAGE_SURVIVAL, .mRow = 0, .mCol = 2, .mChallengeName = "[SURVIVAL_POOL_NORMAL]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SURVIVAL_NORMAL_STAGE_4, .mChallengeIconIndex = 3, .mPage = ChallengePage::CHALLENGE_PAGE_SURVIVAL, .mRow = 0, .mCol = 3, .mChallengeName = "[SURVIVAL_FOG_NORMAL]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SURVIVAL_NORMAL_STAGE_5, .mChallengeIconIndex = 4, .mPage = ChallengePage::CHALLENGE_PAGE_SURVIVAL, .mRow = 0, .mCol = 4, .mChallengeName = "[SURVIVAL_ROOF_NORMAL]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SURVIVAL_HARD_STAGE_1, .mChallengeIconIndex = 5, .mPage = ChallengePage::CHALLENGE_PAGE_SURVIVAL, .mRow = 1, .mCol = 0, .mChallengeName = "[SURVIVAL_DAY_HARD]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SURVIVAL_HARD_STAGE_2, .mChallengeIconIndex = 6, .mPage = ChallengePage::CHALLENGE_PAGE_SURVIVAL, .mRow = 1, .mCol = 1, .mChallengeName = "[SURVIVAL_NIGHT_HARD]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SURVIVAL_HARD_STAGE_3, .mChallengeIconIndex = 7, .mPage = ChallengePage::CHALLENGE_PAGE_SURVIVAL, .mRow = 1, .mCol = 2, .mChallengeName = "[SURVIVAL_POOL_HARD]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SURVIVAL_HARD_STAGE_4, .mChallengeIconIndex = 8, .mPage = ChallengePage::CHALLENGE_PAGE_SURVIVAL, .mRow = 1, .mCol = 3, .mChallengeName = "[SURVIVAL_FOG_HARD]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SURVIVAL_HARD_STAGE_5, .mChallengeIconIndex = 9, .mPage = ChallengePage::CHALLENGE_PAGE_SURVIVAL, .mRow = 1, .mCol = 4, .mChallengeName = "[SURVIVAL_ROOF_HARD]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SURVIVAL_ENDLESS_STAGE_1, .mChallengeIconIndex = 10, .mPage = ChallengePage::CHALLENGE_PAGE_LIMBO, .mRow = 3, .mCol = 0, .mChallengeName = "[SURVIVAL_DAY_ENDLESS]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SURVIVAL_ENDLESS_STAGE_2, .mChallengeIconIndex = 10, .mPage = ChallengePage::CHALLENGE_PAGE_LIMBO, .mRow = 3, .mCol = 1, .mChallengeName = "[SURVIVAL_NIGHT_ENDLESS]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SURVIVAL_ENDLESS_STAGE_3, .mChallengeIconIndex = 10, .mPage = ChallengePage::CHALLENGE_PAGE_SURVIVAL, .mRow = 2, .mCol = 2, .mChallengeName = "[SURVIVAL_POOL_ENDLESS]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SURVIVAL_ENDLESS_STAGE_4, .mChallengeIconIndex = 10, .mPage = ChallengePage::CHALLENGE_PAGE_LIMBO, .mRow = 3, .mCol = 2, .mChallengeName = "[SURVIVAL_FOG_ENDLESS]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SURVIVAL_ENDLESS_STAGE_5, .mChallengeIconIndex = 10, .mPage = ChallengePage::CHALLENGE_PAGE_LIMBO, .mRow = 3, .mCol = 3, .mChallengeName = "[SURVIVAL_ROOF_ENDLESS]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS, .mChallengeIconIndex = 0, .mPage = ChallengePage::CHALLENGE_PAGE_CHALLENGE, .mRow = 0, .mCol = 0, .mChallengeName = "[WAR_AND_PEAS]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_WALLNUT_BOWLING, .mChallengeIconIndex = 6, .mPage = ChallengePage::CHALLENGE_PAGE_CHALLENGE, .mRow = 0, .mCol = 1, .mChallengeName = "[WALL_NUT_BOWLING]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_SLOT_MACHINE, .mChallengeIconIndex = 2, .mPage = ChallengePage::CHALLENGE_PAGE_CHALLENGE, .mRow = 0, .mCol = 2, .mChallengeName = "[SLOT_MACHINE]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_RAINING_SEEDS, .mChallengeIconIndex = 3, .mPage = ChallengePage::CHALLENGE_PAGE_CHALLENGE, .mRow = 0, .mCol = 3, .mChallengeName = "[ITS_RAINING_SEEDS]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_BEGHOULED, .mChallengeIconIndex = 1, .mPage = ChallengePage::CHALLENGE_PAGE_CHALLENGE, .mRow = 0, .mCol = 4, .mChallengeName = "[BEGHOULED]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_INVISIGHOUL, .mChallengeIconIndex = 8, .mPage = ChallengePage::CHALLENGE_PAGE_CHALLENGE, .mRow = 1, .mCol = 0, .mChallengeName = "[INVISIGHOUL]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_SEEING_STARS, .mChallengeIconIndex = 5, .mPage = ChallengePage::CHALLENGE_PAGE_CHALLENGE, .mRow = 1, .mCol = 1, .mChallengeName = "[SEEING_STARS]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_ZOMBIQUARIUM, .mChallengeIconIndex = 7, .mPage = ChallengePage::CHALLENGE_PAGE_CHALLENGE, .mRow = 1, .mCol = 2, .mChallengeName = "[ZOMBIQUARIUM]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST, .mChallengeIconIndex = 20, .mPage = ChallengePage::CHALLENGE_PAGE_CHALLENGE, .mRow = 1, .mCol = 3, .mChallengeName = "[BEGHOULED_TWIST]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_LITTLE_TROUBLE, .mChallengeIconIndex = 12, .mPage = ChallengePage::CHALLENGE_PAGE_CHALLENGE, .mRow = 1, .mCol = 4, .mChallengeName = "[LITTLE_TROUBLE]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_PORTAL_COMBAT, .mChallengeIconIndex = 15, .mPage = ChallengePage::CHALLENGE_PAGE_CHALLENGE, .mRow = 2, .mCol = 0, .mChallengeName = "[PORTAL_COMBAT]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_COLUMN, .mChallengeIconIndex = 4, .mPage = ChallengePage::CHALLENGE_PAGE_CHALLENGE, .mRow = 2, .mCol = 1, .mChallengeName = "[COLUMN_AS_YOU_SEE_EM]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_BOBSLED_BONANZA, .mChallengeIconIndex = 17, .mPage = ChallengePage::CHALLENGE_PAGE_CHALLENGE, .mRow = 2, .mCol = 2, .mChallengeName = "[BOBSLED_BONANZA]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_SPEED, .mChallengeIconIndex = 18, .mPage = ChallengePage::CHALLENGE_PAGE_CHALLENGE, .mRow = 2, .mCol = 3, .mChallengeName = "[ZOMBIES_ON_SPEED]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_WHACK_A_ZOMBIE, .mChallengeIconIndex = 16, .mPage = ChallengePage::CHALLENGE_PAGE_CHALLENGE, .mRow = 2, .mCol = 4, .mChallengeName = "[WHACK_A_ZOMBIE]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_LAST_STAND, .mChallengeIconIndex = 21, .mPage = ChallengePage::CHALLENGE_PAGE_CHALLENGE, .mRow = 3, .mCol = 0, .mChallengeName = "[LAST_STAND]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_WAR_AND_PEAS_2, .mChallengeIconIndex = 0, .mPage = ChallengePage::CHALLENGE_PAGE_CHALLENGE, .mRow = 3, .mCol = 1, .mChallengeName = "[WAR_AND_PEAS_2]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_WALLNUT_BOWLING_2, .mChallengeIconIndex = 6, .mPage = ChallengePage::CHALLENGE_PAGE_CHALLENGE, .mRow = 3, .mCol = 2, .mChallengeName = "[WALL_NUT_BOWLING_EXTREME]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_POGO_PARTY, .mChallengeIconIndex = 14, .mPage = ChallengePage::CHALLENGE_PAGE_CHALLENGE, .mRow = 3, .mCol = 3, .mChallengeName = "[POGO_PARTY]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_FINAL_BOSS, .mChallengeIconIndex = 19, .mPage = ChallengePage::CHALLENGE_PAGE_CHALLENGE, .mRow = 3, .mCol = 4, .mChallengeName = "[FINAL_BOSS]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_ART_CHALLENGE_WALLNUT, .mChallengeIconIndex = 0, .mPage = ChallengePage::CHALLENGE_PAGE_LIMBO, .mRow = 0, .mCol = 0, .mChallengeName = "[ART_CHALLENGE_WALL_NUT]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_SUNNY_DAY, .mChallengeIconIndex = 1, .mPage = ChallengePage::CHALLENGE_PAGE_LIMBO, .mRow = 0, .mCol = 1, .mChallengeName = "[SUNNY_DAY]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_RESODDED, .mChallengeIconIndex = 2, .mPage = ChallengePage::CHALLENGE_PAGE_LIMBO, .mRow = 0, .mCol = 2, .mChallengeName = "[UNSODDED]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_BIG_TIME, .mChallengeIconIndex = 3, .mPage = ChallengePage::CHALLENGE_PAGE_LIMBO, .mRow = 0, .mCol = 3, .mChallengeName = "[BIG_TIME]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_ART_CHALLENGE_SUNFLOWER, .mChallengeIconIndex = 4, .mPage = ChallengePage::CHALLENGE_PAGE_LIMBO, .mRow = 0, .mCol = 4, .mChallengeName = "[ART_CHALLENGE_SUNFLOWER]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_AIR_RAID, .mChallengeIconIndex = 5, .mPage = ChallengePage::CHALLENGE_PAGE_LIMBO, .mRow = 1, .mCol = 0, .mChallengeName = "[AIR_RAID]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_ICE, .mChallengeIconIndex = 6, .mPage = ChallengePage::CHALLENGE_PAGE_LIMBO, .mRow = 1, .mCol = 1, .mChallengeName = "[ICE_LEVEL]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN, .mChallengeIconIndex = 7, .mPage = ChallengePage::CHALLENGE_PAGE_LIMBO, .mRow = 1, .mCol = 2, .mChallengeName = "[ZEN_GARDEN]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_HIGH_GRAVITY, .mChallengeIconIndex = 8, .mPage = ChallengePage::CHALLENGE_PAGE_LIMBO, .mRow = 1, .mCol = 3, .mChallengeName = "[HIGH_GRAVITY]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_GRAVE_DANGER, .mChallengeIconIndex = 11, .mPage = ChallengePage::CHALLENGE_PAGE_LIMBO, .mRow = 1, .mCol = 4, .mChallengeName = "[GRAVE_DANGER]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_SHOVEL, .mChallengeIconIndex = 10, .mPage = ChallengePage::CHALLENGE_PAGE_LIMBO, .mRow = 2, .mCol = 0, .mChallengeName = "[CAN_YOU_DIG_IT]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_STORMY_NIGHT, .mChallengeIconIndex = 13, .mPage = ChallengePage::CHALLENGE_PAGE_LIMBO, .mRow = 2, .mCol = 1, .mChallengeName = "[DARK_STORMY_NIGHT]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_BUNGEE_BLITZ, .mChallengeIconIndex = 9, .mPage = ChallengePage::CHALLENGE_PAGE_LIMBO, .mRow = 2, .mCol = 2, .mChallengeName = "[BUNGEE_BLITZ]" },
	{ .mChallengeMode = GameMode::GAMEMODE_CHALLENGE_SQUIRREL, .mChallengeIconIndex = 10, .mPage = ChallengePage::CHALLENGE_PAGE_LIMBO, .mRow = 2, .mCol = 3, .mChallengeName = "Squirrel" },
	{ .mChallengeMode = GameMode::GAMEMODE_TREE_OF_WISDOM, .mChallengeIconIndex = 10, .mPage = ChallengePage::CHALLENGE_PAGE_LIMBO, .mRow = 2, .mCol = 4, .mChallengeName = "Tree of Wisdom" }, // @Patoke: replaced for english
	{ .mChallengeMode = GameMode::GAMEMODE_SCARY_POTTER_1, .mChallengeIconIndex = 10, .mPage = ChallengePage::CHALLENGE_PAGE_PUZZLE, .mRow = 0, .mCol = 0, .mChallengeName = "[SCARY_POTTER_1]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SCARY_POTTER_2, .mChallengeIconIndex = 10, .mPage = ChallengePage::CHALLENGE_PAGE_PUZZLE, .mRow = 0, .mCol = 1, .mChallengeName = "[SCARY_POTTER_2]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SCARY_POTTER_3, .mChallengeIconIndex = 10, .mPage = ChallengePage::CHALLENGE_PAGE_PUZZLE, .mRow = 0, .mCol = 2, .mChallengeName = "[SCARY_POTTER_3]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SCARY_POTTER_4, .mChallengeIconIndex = 10, .mPage = ChallengePage::CHALLENGE_PAGE_PUZZLE, .mRow = 0, .mCol = 3, .mChallengeName = "[SCARY_POTTER_4]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SCARY_POTTER_5, .mChallengeIconIndex = 10, .mPage = ChallengePage::CHALLENGE_PAGE_PUZZLE, .mRow = 0, .mCol = 4, .mChallengeName = "[SCARY_POTTER_5]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SCARY_POTTER_6, .mChallengeIconIndex = 10, .mPage = ChallengePage::CHALLENGE_PAGE_PUZZLE, .mRow = 1, .mCol = 0, .mChallengeName = "[SCARY_POTTER_6]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SCARY_POTTER_7, .mChallengeIconIndex = 10, .mPage = ChallengePage::CHALLENGE_PAGE_PUZZLE, .mRow = 1, .mCol = 1, .mChallengeName = "[SCARY_POTTER_7]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SCARY_POTTER_8, .mChallengeIconIndex = 10, .mPage = ChallengePage::CHALLENGE_PAGE_PUZZLE, .mRow = 1, .mCol = 2, .mChallengeName = "[SCARY_POTTER_8]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SCARY_POTTER_9, .mChallengeIconIndex = 10, .mPage = ChallengePage::CHALLENGE_PAGE_PUZZLE, .mRow = 1, .mCol = 3, .mChallengeName = "[SCARY_POTTER_9]" },
	{ .mChallengeMode = GameMode::GAMEMODE_SCARY_POTTER_ENDLESS, .mChallengeIconIndex = 10, .mPage = ChallengePage::CHALLENGE_PAGE_PUZZLE, .mRow = 1, .mCol = 4, .mChallengeName = "[SCARY_POTTER_ENDLESS]" },
	{ .mChallengeMode = GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_1, .mChallengeIconIndex = 11, .mPage = ChallengePage::CHALLENGE_PAGE_PUZZLE, .mRow = 2, .mCol = 0, .mChallengeName = "[I_ZOMBIE_1]" },
	{ .mChallengeMode = GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_2, .mChallengeIconIndex = 11, .mPage = ChallengePage::CHALLENGE_PAGE_PUZZLE, .mRow = 2, .mCol = 1, .mChallengeName = "[I_ZOMBIE_2]" },
	{ .mChallengeMode = GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_3, .mChallengeIconIndex = 11, .mPage = ChallengePage::CHALLENGE_PAGE_PUZZLE, .mRow = 2, .mCol = 2, .mChallengeName = "[I_ZOMBIE_3]" },
	{ .mChallengeMode = GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_4, .mChallengeIconIndex = 11, .mPage = ChallengePage::CHALLENGE_PAGE_PUZZLE, .mRow = 2, .mCol = 3, .mChallengeName = "[I_ZOMBIE_4]" },
	{ .mChallengeMode = GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_5, .mChallengeIconIndex = 11, .mPage = ChallengePage::CHALLENGE_PAGE_PUZZLE, .mRow = 2, .mCol = 4, .mChallengeName = "[I_ZOMBIE_5]" },
	{ .mChallengeMode = GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_6, .mChallengeIconIndex = 11, .mPage = ChallengePage::CHALLENGE_PAGE_PUZZLE, .mRow = 3, .mCol = 0, .mChallengeName = "[I_ZOMBIE_6]" },
	{ .mChallengeMode = GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_7, .mChallengeIconIndex = 11, .mPage = ChallengePage::CHALLENGE_PAGE_PUZZLE, .mRow = 3, .mCol = 1, .mChallengeName = "[I_ZOMBIE_7]" },
	{ .mChallengeMode = GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_8, .mChallengeIconIndex = 11, .mPage = ChallengePage::CHALLENGE_PAGE_PUZZLE, .mRow = 3, .mCol = 2, .mChallengeName = "[I_ZOMBIE_8]" },
	{ .mChallengeMode = GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_9, .mChallengeIconIndex = 11, .mPage = ChallengePage::CHALLENGE_PAGE_PUZZLE, .mRow = 3, .mCol = 3, .mChallengeName = "[I_ZOMBIE_9]" },
	{ .mChallengeMode = GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_ENDLESS, .mChallengeIconIndex = 11, .mPage = ChallengePage::CHALLENGE_PAGE_PUZZLE, .mRow = 3, .mCol = 4, .mChallengeName = "[I_ZOMBIE_ENDLESS]" },
	{ .mChallengeMode = GameMode::GAMEMODE_UPSELL, .mChallengeIconIndex = 10, .mPage = ChallengePage::CHALLENGE_PAGE_LIMBO, .mRow = 3, .mCol = 4, .mChallengeName = "Upsell" },
	{ .mChallengeMode = GameMode::GAMEMODE_INTRO, .mChallengeIconIndex = 10, .mPage = ChallengePage::CHALLENGE_PAGE_LIMBO, .mRow = 2, .mCol = 3, .mChallengeName = "Intro" }
};

// GOTY @Patoke: 0x430810
ChallengeScreen::ChallengeScreen(LawnApp* theApp, ChallengePage thePage)
{
	mLockShakeX = 0;
	mLockShakeY = 0;
	mPageIndex = thePage;
	mApp = theApp;
	mClip = false;
	mCheatEnableChallenges = false;
	mUnlockState = UNLOCK_OFF;
	mUnlockChallengeIndex = -1;
	mUnlockStateCounter = 0;
	mLimboPageUnlocked = false;
	mClickCount = 0;
	mLastClickTime = 0;
	mLoadedResourceNames.push_back("DelayLoad_ChallengeScreen");

	for (std::string& resource : mLoadedResourceNames)
		TodLoadResources(resource.c_str());

	mBackButton = MakeNewButton(ChallengeScreen::ChallengeScreen_Back, this, "[BACK_TO_MENU]", nullptr, Sexy::IMAGE_SEEDCHOOSER_BUTTON2, 
		Sexy::IMAGE_SEEDCHOOSER_BUTTON2_GLOW, Sexy::IMAGE_SEEDCHOOSER_BUTTON2_GLOW);
	mBackButton->mTextDownOffsetX = 1;
	mBackButton->mTextDownOffsetY = 1;
	mBackButton->mColors[ButtonWidget::COLOR_LABEL] = Color(42, 42, 90);
	mBackButton->mColors[ButtonWidget::COLOR_LABEL_HILITE] = Color(42, 42, 90);
	mBackButton->Resize(18, 568, 111, 26);

	for (int aPageIdx = CHALLENGE_PAGE_SURVIVAL; aPageIdx < MAX_CHALLANGE_PAGES; aPageIdx++)
	{
		ButtonWidget* aPageButton = new ButtonWidget(ChallengeScreen::ChallengeScreen_Page + aPageIdx, this);
		aPageButton->mDoFinger = true;
		mPageButton[aPageIdx] = aPageButton;
		if (aPageIdx == CHALLENGE_PAGE_LIMBO)
			aPageButton->mLabel = TodStringTranslate("Limbo Page");
		else
			aPageButton->mLabel = TodReplaceNumberString("[PAGE_X]", "{PAGE}", aPageIdx);
		aPageButton->mButtonImage = Sexy::IMAGE_BLANK;
		aPageButton->mOverImage = Sexy::IMAGE_BLANK;
		aPageButton->mDownImage = Sexy::IMAGE_BLANK;
		aPageButton->SetFont(Sexy::FONT_BRIANNETOD12);
		aPageButton->mColors[ButtonWidget::COLOR_LABEL] = Color(255, 240, 0);
		aPageButton->mColors[ButtonWidget::COLOR_LABEL_HILITE] = Color(220, 220, 0);
		aPageButton->Resize(200 + 100 * aPageIdx, 540, 100, 75);
		if (!ShowPageButtons() || aPageIdx == CHALLENGE_PAGE_SURVIVAL || aPageIdx == CHALLENGE_PAGE_PUZZLE)
			aPageButton->mVisible = false;
	}
	
	for (int aChallengeMode = 0; aChallengeMode < NUM_CHALLENGE_MODES; aChallengeMode++)
	{
		const ChallengeDefinition& aChlDef = GetChallengeDefinition(aChallengeMode);
		ButtonWidget* aChallengeButton = new ButtonWidget(ChallengeScreen::ChallengeScreen_Mode + aChallengeMode, this);
		mChallengeButtons[aChallengeMode] = aChallengeButton;
		aChallengeButton->mDoFinger = true;
		aChallengeButton->mFrameNoDraw = true;
		if (aChlDef.mPage == CHALLENGE_PAGE_CHALLENGE || aChlDef.mPage == CHALLENGE_PAGE_LIMBO || aChlDef.mPage == CHALLENGE_PAGE_PUZZLE)
			aChallengeButton->Resize(38 + aChlDef.mCol * 155, 93 + aChlDef.mRow * 119, 104, 115);
		else
			aChallengeButton->Resize(38 + aChlDef.mCol * 155, 125 + aChlDef.mRow * 145, 104, 115);
		if (MoreTrophiesNeeded(aChallengeMode))
		{
			aChallengeButton->mDoFinger = false;
			aChallengeButton->mDisabled = true;
		}
	}

	mToolTip = new ToolTipWidget();
	mToolTip->mCenter = true;
	mToolTip->mVisible = false;
	UpdateButtons();
	
	if (mApp->mGameMode != GAMEMODE_UPSELL || mApp->mGameScene != SCENE_LEVEL_INTRO)
		mApp->mMusic->MakeSureMusicIsPlaying(MUSIC_TUNE_CHOOSE_YOUR_SEEDS);

	// bool aIsIZombie = false; // Unused
	if (mPageIndex == CHALLENGE_PAGE_SURVIVAL && mApp->mPlayerInfo->mHasNewSurvival)
	{
		SetUnlockChallengeIndex(mPageIndex, false);
		mApp->mPlayerInfo->mHasNewSurvival = false;
	}
	else if (mPageIndex == CHALLENGE_PAGE_CHALLENGE && mApp->mPlayerInfo->mHasNewMiniGame)
	{
		SetUnlockChallengeIndex(mPageIndex, false);
		mApp->mPlayerInfo->mHasNewMiniGame = false;
	}
	else if (mPageIndex == CHALLENGE_PAGE_PUZZLE)
	{
		if (mApp->mPlayerInfo->mHasNewScaryPotter)
		{
			SetUnlockChallengeIndex(mPageIndex, false);
			mApp->mPlayerInfo->mHasNewScaryPotter = false;
		}
		else if (mApp->mPlayerInfo->mHasNewIZombie)
		{
			SetUnlockChallengeIndex(mPageIndex, true);
			mApp->mPlayerInfo->mHasNewIZombie = false;
		}
	}
}

ChallengeScreen::~ChallengeScreen()
{
	delete mBackButton;
	for (ButtonWidget* aPageButton : mPageButton) delete aPageButton;
	for (ButtonWidget* aChallengeButton : mChallengeButtons) delete aChallengeButton;
	delete mToolTip;
}

const ChallengeDefinition& GetChallengeDefinition(int theChallengeMode)
{
	TOD_ASSERT(theChallengeMode >= 0 && theChallengeMode < NUM_CHALLENGE_MODES);

	const ChallengeDefinition& aDef = gChallengeDefs[theChallengeMode];
	(void)aDef; // Unused in Release mode
	TOD_ASSERT(aDef.mChallengeMode == theChallengeMode + GAMEMODE_SURVIVAL_NORMAL_STAGE_1);

	return gChallengeDefs[theChallengeMode];
}

bool ChallengeScreen::IsScaryPotterLevel(GameMode theGameMode)
{
	return theGameMode >= GAMEMODE_SCARY_POTTER_1 && theGameMode <= GAMEMODE_SCARY_POTTER_ENDLESS;
}

bool ChallengeScreen::IsIZombieLevel(GameMode theGameMode)
{
	return theGameMode >= GAMEMODE_PUZZLE_I_ZOMBIE_1 && theGameMode <= GAMEMODE_PUZZLE_I_ZOMBIE_ENDLESS;
}

void ChallengeScreen::SetUnlockChallengeIndex(ChallengePage thePage, bool theIsIZombie)
{
	mUnlockState = UNLOCK_SHAKING;
	mUnlockStateCounter = 100;
	mUnlockChallengeIndex = 0;
	for (int aChallengeMode = 0; aChallengeMode < NUM_CHALLENGE_MODES; aChallengeMode++)
	{
		const ChallengeDefinition& aDef = GetChallengeDefinition(aChallengeMode);
		if (aDef.mPage == thePage)
		{
			if (thePage != CHALLENGE_PAGE_PUZZLE || (!theIsIZombie && IsScaryPotterLevel(aDef.mChallengeMode)) || (theIsIZombie && IsIZombieLevel(aDef.mChallengeMode)))
			{
				if (AccomplishmentsNeeded(aChallengeMode) <= 0)
				{
					mUnlockChallengeIndex = aChallengeMode;
				}
			}
		}
	}
}

int ChallengeScreen::MoreTrophiesNeeded(int theChallengeIndex)
{
	const ChallengeDefinition& aDef = GetChallengeDefinition(theChallengeIndex);
	if (mApp->mGameMode == GAMEMODE_UPSELL && mApp->mGameScene == SCENE_LEVEL_INTRO)
	{
		return aDef.mChallengeMode == GAMEMODE_CHALLENGE_FINAL_BOSS ? 1 : 0;
	}
	
	if (mApp->IsTrialStageLocked())
	{
		if (mPageIndex == CHALLENGE_PAGE_PUZZLE && aDef.mChallengeMode >= GAMEMODE_SCARY_POTTER_4)
		{
			return aDef.mChallengeMode == GAMEMODE_SCARY_POTTER_4 ? 1 : 2;
		}
		else if (mPageIndex == CHALLENGE_PAGE_CHALLENGE && aDef.mChallengeMode >= GAMEMODE_CHALLENGE_RAINING_SEEDS)
		{
			return aDef.mChallengeMode == GAMEMODE_CHALLENGE_RAINING_SEEDS ? 1 : 2;
		}
		else if (mPageIndex == CHALLENGE_PAGE_SURVIVAL && aDef.mChallengeMode >= GAMEMODE_SURVIVAL_NORMAL_STAGE_4)
		{
			return aDef.mChallengeMode == GAMEMODE_SURVIVAL_NORMAL_STAGE_4 ? 1 : 2;
		}
	}

	if (aDef.mPage == CHALLENGE_PAGE_PUZZLE)
	{
		if (IsScaryPotterLevel(aDef.mChallengeMode))
		{
			int aLevelsCompleted = 0;
			for (const ChallengeDefinition& aSPDef : gChallengeDefs)
			{
				if (IsScaryPotterLevel(aSPDef.mChallengeMode) && mApp->HasBeatenChallenge(aSPDef.mChallengeMode))
				{
					aLevelsCompleted++;
				}
			}

			if (aDef.mChallengeMode < GAMEMODE_SCARY_POTTER_4 || mApp->HasFinishedAdventure() || aLevelsCompleted < 3)
			{
				return ClampInt(aDef.mChallengeMode - GAMEMODE_SCARY_POTTER_1 - aLevelsCompleted, 0, 9);
			}
			else
			{
				return aDef.mChallengeMode == GAMEMODE_SCARY_POTTER_4 ? 1 : 2;
			}
		}
		else if (IsIZombieLevel(aDef.mChallengeMode))
		{
			int aLevelsCompleted = 0;
			for (const ChallengeDefinition& aIZDef : gChallengeDefs)
			{
				if (IsIZombieLevel(aIZDef.mChallengeMode) && mApp->HasBeatenChallenge(aIZDef.mChallengeMode))
				{
					aLevelsCompleted++;
				}
			}

			if (aDef.mChallengeMode < GAMEMODE_PUZZLE_I_ZOMBIE_4 || mApp->HasFinishedAdventure() || aLevelsCompleted < 3)
			{
				return ClampInt(aDef.mChallengeMode - GAMEMODE_PUZZLE_I_ZOMBIE_1 - aLevelsCompleted, 0, 9);
			}
			else
			{
				return aDef.mChallengeMode == GAMEMODE_PUZZLE_I_ZOMBIE_4 ? 1 : 2;
			}
		}
	}
	else
	{
		int aIdxInPage = aDef.mRow * 5 + aDef.mCol;
		if ((aDef.mPage == CHALLENGE_PAGE_CHALLENGE || aDef.mPage == CHALLENGE_PAGE_SURVIVAL) && !mApp->HasFinishedAdventure())
		{
			return aIdxInPage < 3 ? 0 : aIdxInPage == 3 ? 1 : 2;
		}
		else
		{
			int aNumTrophies = mApp->GetNumTrophies(aDef.mPage);
			if (aDef.mPage == CHALLENGE_PAGE_LIMBO)
			{
				return 0;
			}
			if (mApp->IsSurvivalEndless(aDef.mChallengeMode))
			{
				return 10 - aNumTrophies;
			}
			if (aDef.mPage == CHALLENGE_PAGE_SURVIVAL || aDef.mPage == CHALLENGE_PAGE_CHALLENGE)
			{
				aNumTrophies += 3;
			}
			else
			{
				TOD_ASSERT(false);
			}

			return aIdxInPage >= aNumTrophies ? aIdxInPage - aNumTrophies + 1 : 0;
		}
	}

	unreachable();
}

bool ChallengeScreen::ShowPageButtons()
{
	return mApp->mTodCheatKeys && mPageIndex != CHALLENGE_PAGE_SURVIVAL && mPageIndex != CHALLENGE_PAGE_PUZZLE;
}

void ChallengeScreen::UpdateButtons()
{
	for (int aChallengeMode = 0; aChallengeMode < NUM_CHALLENGE_MODES; aChallengeMode++)
		mChallengeButtons[aChallengeMode]->mVisible = GetChallengeDefinition(aChallengeMode).mPage == mPageIndex;
	for (int aPage = 0; aPage < MAX_CHALLANGE_PAGES; aPage++)
	{
		ButtonWidget* aPageButton = mPageButton[aPage];

		if (mLimboPageUnlocked && aPage == CHALLENGE_PAGE_LIMBO)
			aPageButton->mVisible = true;

		if (aPage == mPageIndex)
		{
			aPageButton->mColors[ButtonWidget::COLOR_LABEL] = Color(64, 64, 64);
			aPageButton->mDisabled = true;
		}
		else
		{
			aPageButton->mColors[ButtonWidget::COLOR_LABEL] = Color(255, 240, 0);
			aPageButton->mDisabled = false;
		}
	}
}

int ChallengeScreen::AccomplishmentsNeeded(int theChallengeIndex)
{
	int aTrophiesNeeded = MoreTrophiesNeeded(theChallengeIndex);
	GameMode aGameMode = GetChallengeDefinition(theChallengeIndex).mChallengeMode;
	if (mApp->IsSurvivalEndless(aGameMode) && aTrophiesNeeded <= 3 && mApp->GetNumTrophies(CHALLENGE_PAGE_SURVIVAL) < 10 &&
		mApp->HasFinishedAdventure() && !mApp->IsTrialStageLocked()) aTrophiesNeeded = 1;
	return mCheatEnableChallenges ? 0 : aTrophiesNeeded;
}

void ChallengeScreen::DrawButton(Graphics* g, int theChallengeIndex)
{
	ButtonWidget* aChallengeButton = mChallengeButtons[theChallengeIndex];
	if (aChallengeButton->mVisible)
	{
		const ChallengeDefinition& aDef = GetChallengeDefinition(theChallengeIndex);
		int aPosX = aChallengeButton->mX;
		int aPosY = aChallengeButton->mY;
		if (aChallengeButton->mIsDown)
		{
			aPosX++;
			aPosY++;
		}

		if (AccomplishmentsNeeded(theChallengeIndex) <= 1)
		{
			// ============================================================================================
			// ▲ 绘制按钮上的小游戏图标
			// ============================================================================================
			if (aChallengeButton->mDisabled)
			{
				g->SetColor(Color(92, 92, 92));
				g->SetColorizeImages(true);
			}
			if (theChallengeIndex == mUnlockChallengeIndex)
			{
				if (mUnlockState == UNLOCK_SHAKING)
				{
					g->SetColor(Color(92, 92, 92));
				}
				else if (mUnlockState == UNLOCK_FADING)
				{
					int aColor = TodAnimateCurve(50, 25, mUnlockStateCounter, 92, 255, CURVE_LINEAR);
					g->SetColor(Color(aColor, aColor, aColor));
				}
				g->SetColorizeImages(true);
			}

			if (mPageIndex == CHALLENGE_PAGE_SURVIVAL)
			{
				g->DrawImageCel(Sexy::IMAGE_SURVIVAL_THUMBNAILS, aPosX + 13, aPosY + 4, aDef.mChallengeIconIndex);
			}
			else
			{
				g->DrawImageCel(Sexy::IMAGE_CHALLENGE_THUMBNAILS, aPosX + 13, aPosY + 4, aDef.mChallengeIconIndex);
			}

			// ============================================================================================
			// ▲ 绘制小游戏按钮边框
			// ============================================================================================
			bool aHighLight = aChallengeButton->mIsOver && theChallengeIndex != mUnlockChallengeIndex;
			g->SetColorizeImages(false);
			g->DrawImage(aHighLight ? Sexy::IMAGE_CHALLENGE_WINDOW : Sexy::IMAGE_CHALLENGE_WINDOW_HIGHLIGHT, aPosX - 6, aPosY - 2);

			// ============================================================================================
			// ▲ 绘制小游戏的名称
			// ============================================================================================
			Color aTextColor = aHighLight ? Color(250, 40, 40) : Color(42, 42, 90);
			std::string aName = TodStringTranslate(aDef.mChallengeName);
			if (aChallengeButton->mDisabled || (theChallengeIndex == mUnlockChallengeIndex && mUnlockState == UNLOCK_SHAKING))
			{
				aName = "?";
			}

			int aNameLen = aName.size();
			int aAutoWrapNum = mApp->GetInteger("CHALLENGE_SCREEN_BUTTON_AUTO_WRAP_NUM", 13);
			if (aNameLen < aAutoWrapNum)
			{
				TodDrawString(g, aName, aPosX + 52, aPosY + 96, Sexy::FONT_BRIANNETOD12, aTextColor, DS_ALIGN_CENTER);
			}
			else
			{
				// 先尝试在名称字符串的后半段取空格以将字符串分隔为两行，若后半段中无空格则在整个字符串中寻找空格
				int aHalfPos = (mPageIndex == CHALLENGE_PAGE_SURVIVAL && !aChallengeButton->mDisabled) ? 7 : (aNameLen / 2 - 1);
				const char* aSpacedChar = strchr(aName.c_str() + aHalfPos, ' ');
				if (aSpacedChar == nullptr)
				{
					aSpacedChar = strchr(aName.c_str(), ' ');
				}

				// 分别计算取得两行文本的长度
				int aLine1Len = aNameLen;
				int aLine2Len = 0;
				if (aSpacedChar != nullptr)
				{
					aLine1Len = aSpacedChar - aName.c_str();
					aLine2Len = aNameLen - aLine1Len - 1;
				}

				// 分别绘制两行文本字符串
				TodDrawString(g, aName.substr(0, aLine1Len), aPosX + 52, aPosY + 88, Sexy::FONT_BRIANNETOD12, aTextColor, DS_ALIGN_CENTER);
				if (aLine2Len > 0)
				{
					TodDrawString(g, aName.substr(aLine1Len + 1, aLine2Len), aPosX + 52, aPosY + 102, Sexy::FONT_BRIANNETOD12, aTextColor, DS_ALIGN_CENTER);
				}
			}

			// ============================================================================================
			// ▲ 绘制关卡锁定或关卡完成的贴图以及关卡最高记录的文本等
			// ============================================================================================
			uint32_t aRecord = mApp->mPlayerInfo->mChallengeRecords[theChallengeIndex];
			if (theChallengeIndex == mUnlockChallengeIndex)
			{
				Image* aLockImage = Sexy::IMAGE_LOCK;
				if (mUnlockState == UNLOCK_FADING)
				{
					aLockImage = Sexy::IMAGE_LOCK_OPEN;
					g->SetColor(Color(255, 255, 255, TodAnimateCurve(25, 0, mUnlockStateCounter, 255, 0, CURVE_LINEAR)));
					g->SetColorizeImages(true);
				}
				TodDrawImageScaledF(g, aLockImage, aPosX + 24 + mLockShakeX, aPosY + 9 + mLockShakeY, 0.7f, 0.7f);
				g->SetColorizeImages(false);
			}
			else if (aRecord > 0)
			{
				if (mApp->HasBeatenChallenge(aDef.mChallengeMode))
				{
					g->DrawImage(Sexy::IMAGE_MINIGAME_TROPHY, aPosX - 6, aPosY - 2);
				}
				else if (mApp->IsEndlessScaryPotter(aDef.mChallengeMode) || mApp->IsEndlessIZombie(aDef.mChallengeMode))
				{
					std::string aAchievement = mApp->Pluralize(aRecord, "[ONE_FLAG]", "[COUNT_FLAGS]");
					TodDrawString(g, aAchievement, aPosX + 48, aPosY + 48, Sexy::FONT_CONTINUUMBOLD14OUTLINE, Color::White, DS_ALIGN_CENTER);
					TodDrawString(g, aAchievement, aPosX + 48, aPosY + 48, Sexy::FONT_CONTINUUMBOLD14, Color(255, 0, 0), DS_ALIGN_CENTER);
				}
				else if (mApp->IsSurvivalEndless(aDef.mChallengeMode))
				{
					std::string aAchievement = TodReplaceNumberString("[LONGEST_STREAK]", "{STREAK}", aRecord);
					Rect aRect(aPosX, aPosY + 15, 96, 200);
					TodDrawStringWrapped(g, aAchievement, aRect, Sexy::FONT_CONTINUUMBOLD14OUTLINE, Color::White, DS_ALIGN_CENTER);
					TodDrawStringWrapped(g, aAchievement, aRect, Sexy::FONT_CONTINUUMBOLD14, Color(255, 0, 0), DS_ALIGN_CENTER);
				}
			}
			else if (aChallengeButton->mDisabled)
			{
				TodDrawImageScaledF(g, Sexy::IMAGE_LOCK, aPosX + 24, aPosY + 9, 0.7f, 0.7f);
			}
		}
		else
		{
			g->DrawImage(Sexy::IMAGE_CHALLENGE_BLANK, aPosX, aPosY);
		}
	}
}

void ChallengeScreen::Draw(Graphics* g)
{
	g->SetLinearBlend(true);
	g->DrawImage(Sexy::IMAGE_CHALLENGE_BACKGROUND, 0, 0);

	std::string aTitleString =
		mPageIndex == CHALLENGE_PAGE_SURVIVAL ? "[PICK_AREA]" : 
		mPageIndex == CHALLENGE_PAGE_PUZZLE ? "[SCARY_POTTER]" : "[PICK_CHALLENGE]";
	TodDrawString(g, aTitleString, 400, 58, Sexy::FONT_HOUSEOFTERROR28, Color(220, 220, 220), DS_ALIGN_CENTER);

	int aTrophiesGot = mApp->GetNumTrophies(mPageIndex);
	int aTrophiesTotal = mPageIndex == CHALLENGE_PAGE_SURVIVAL ? 10 : mPageIndex == CHALLENGE_PAGE_CHALLENGE ? 20 : mPageIndex == CHALLENGE_PAGE_PUZZLE ? 18 : 0;
	if (aTrophiesTotal > 0)
	{
		std::string aTrophyString = StrFormat("%d/%d", aTrophiesGot, aTrophiesTotal);
		TodDrawString(g, aTrophyString, 739, 73, Sexy::FONT_DWARVENTODCRAFT12, Color(255, 240, 0), DS_ALIGN_CENTER);
	}
	TodDrawImageScaledF(g, Sexy::IMAGE_TROPHY, 718, 26, 0.5f, 0.5f);

	for (int aChallengeMode = 0; aChallengeMode < NUM_CHALLENGE_MODES; aChallengeMode++)
		DrawButton(g, aChallengeMode);

	mToolTip->Draw(g);
}

void ChallengeScreen::Update()
{
	Widget::Update();
	UpdateToolTip();

	if (mUnlockStateCounter > 0) mUnlockStateCounter--;
	if (mUnlockState == UNLOCK_SHAKING)
	{
		if (mUnlockStateCounter == 0)
		{
			mApp->PlayFoley(FOLEY_PAPER);
			mUnlockState = UNLOCK_FADING;
			mUnlockStateCounter = 50;
			mLockShakeX = 0;
			mLockShakeY = 0;
		}
		else
		{
			mLockShakeX = RandRangeFloat(-2, 2);
			mLockShakeY = RandRangeFloat(-2, 2);
		}
	}
	else if (mUnlockState == UNLOCK_FADING && mUnlockStateCounter == 0)
	{
		mUnlockState = UNLOCK_OFF;
		mUnlockStateCounter = 0;
		mUnlockChallengeIndex = -1;
	}

	MarkDirty();
}

void ChallengeScreen::AddedToManager(WidgetManager* theWidgetManager)
{
	Widget::AddedToManager(theWidgetManager);
	AddWidget(mBackButton);
	for (ButtonWidget* aButton : mPageButton) AddWidget(aButton);
	for (ButtonWidget* aButton : mChallengeButtons) AddWidget(aButton);
}

void ChallengeScreen::RemovedFromManager(WidgetManager* theWidgetManager)
{
	Widget::RemovedFromManager(theWidgetManager);
	RemoveWidget(mBackButton);
	for (ButtonWidget* aButton : mPageButton) RemoveWidget(aButton);
	for (ButtonWidget* aButton : mChallengeButtons) RemoveWidget(aButton);
}

void ChallengeScreen::ButtonPress(int theId)
{
	(void)theId;
	mApp->PlaySample(Sexy::SOUND_BUTTONCLICK);
}

void ChallengeScreen::ButtonDepress(int theId)
{
	if (theId == ChallengeScreen::ChallengeScreen_Back)
	{
		mApp->KillChallengeScreen();
		mApp->DoBackToMain();
	}

	int aChallengeMode = theId - ChallengeScreen::ChallengeScreen_Mode;
	if (aChallengeMode >= 0 && aChallengeMode < NUM_CHALLENGE_MODES)
	{
		mApp->KillChallengeScreen();
		mApp->PreNewGame((GameMode)(aChallengeMode + 1), true);
	}

	int aPageIndex = theId - ChallengeScreen::ChallengeScreen_Page;
	if (aPageIndex >= 0 && aPageIndex < 4)
	{
		mPageIndex = (ChallengePage)aPageIndex;
		UpdateButtons();
	}
}

void ChallengeScreen::KeyDown(KeyCode theKey)
{
	if (theKey == KeyCode::KEYCODE_ESCAPE)
	{
		ButtonDepress(ChallengeScreen::ChallengeScreen_Back);
	}
}

void ChallengeScreen::UpdateToolTip()
{
	if (!mApp->mWidgetManager->mMouseIn || !mApp->mActive)
	{
		mToolTip->mVisible = false;
		return;
	}

	for (int aChallengeMode = 0; aChallengeMode < NUM_CHALLENGE_MODES; aChallengeMode++)
	{
		const ChallengeDefinition& aDef = GetChallengeDefinition(aChallengeMode);
		ButtonWidget* aChallengeButton = mChallengeButtons[aChallengeMode];
		if (aChallengeButton->mVisible && aChallengeButton->mDisabled &&
			aChallengeButton->Contains(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY) &&
			AccomplishmentsNeeded(aChallengeMode) <= 1)
		{
			mToolTip->mX = aChallengeButton->mWidth / 2 + aChallengeButton->mX;
			mToolTip->mY = aChallengeButton->mY;
			if (MoreTrophiesNeeded(aChallengeMode) > 0)
			{
				std::string aLabel;
				if (mPageIndex == CHALLENGE_PAGE_PUZZLE)
				{
					if (IsScaryPotterLevel(aDef.mChallengeMode))
					{
						if (!mApp->HasFinishedAdventure() && aDef.mChallengeMode == GAMEMODE_SCARY_POTTER_4)
						{
							aLabel = "[FINISH_ADVENTURE_TOOLTIP]";
						}
						else
						{
							aLabel = "[ONE_MORE_SCARY_POTTER_TOOLTIP]";
						}
					}
					else if (IsIZombieLevel(aDef.mChallengeMode))
					{
						if (!mApp->HasFinishedAdventure() && aDef.mChallengeMode == GAMEMODE_PUZZLE_I_ZOMBIE_4)
						{
							aLabel = "[FINISH_ADVENTURE_TOOLTIP]";
						}
						else
						{
							aLabel = "[ONE_MORE_IZOMBIE_TOOLTIP]";
						}
					}
				}
				else if (!mApp->HasFinishedAdventure() || mApp->IsTrialStageLocked())
				{
					aLabel = "[FINISH_ADVENTURE_TOOLTIP]";
				}
				else if (mApp->IsSurvivalEndless(aDef.mChallengeMode))
				{
					aLabel = "[10_SURVIVAL_TOOLTIP]";
				}
				else if (mPageIndex == CHALLENGE_PAGE_SURVIVAL)
				{
					aLabel = "[ONE_MORE_SURVIVAL_TOOLTIP]";
				}
				else if (mPageIndex == CHALLENGE_PAGE_CHALLENGE)
				{
					aLabel = "[ONE_MORE_CHALLENGE_TOOLTIP]";
				}
				else continue;

				mToolTip->SetLabel(aLabel);
				mToolTip->mVisible = true;
				return;
			} // end if (MoreTrophiesNeeded(aChallengeMode) > 0)
		} // end 需要显示标签的条件判断
	}

	mToolTip->mVisible = false;
}

void ChallengeScreen::MouseDown(int x, int y, int theClickCount)
{
	Widget::MouseDown(x, y, theClickCount);

	if (mLimboPageUnlocked)
		return;

	constexpr int MAX_GAP_MS = 200;
	constexpr int CLICKS_NEEDED = 5;

	uint32_t aNow = SDL_GetTicks();
	if (aNow - mLastClickTime > MAX_GAP_MS)
		mClickCount = 0;
	mLastClickTime = aNow;
	mClickCount++;
	if (mClickCount >= CLICKS_NEEDED)
	{
		mLimboPageUnlocked = true;
		UpdateButtons();
	}
}
