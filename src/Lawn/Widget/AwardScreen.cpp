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

#include "../Plant.h"
#include "../Board.h"
#include "GameButton.h"
#include "StoreScreen.h"
#include "AwardScreen.h"
#include "../ZenGarden.h"
#include "../SeedPacket.h"
#include "../../LawnApp.h"
#include "AlmanacDialog.h"
#include "../System/Music.h"
#include "../../Resources.h"
#include "../../GameConstants.h"
#include "../System/PlayerInfo.h"
#include "../../Sexy.TodLib/TodFoley.h"
#include "../../Sexy.TodLib/TodCommon.h"
#include "../../Sexy.TodLib/TodStringFile.h"
#include "AchievementsScreen.h"

// GOTY @Patoke: 0x4063E0
AwardScreen::AwardScreen(LawnApp* theApp, AwardType theAwardType, bool theShowingAchievements)
{
	mApp = theApp;
	mClip = false;
	mFadeInCounter = 180;
	mAchievementAnimTime = 0;
	mAwardType = theAwardType;
	mShowingAchievements = theShowingAchievements;

	mLoadedResourceNames.push_back("DelayLoad_AwardScreen");

	// @Patoke: implemented
	if (!theShowingAchievements) {
		mShowingAchievements = false;
	}
	else {
		mLoadedResourceNames.push_back("DelayLoad_ChallengeScreen");

		for (int i = 0; i < MAX_ACHIEVEMENTS; i++) {
			if (mApp->mPlayerInfo->mEarnedAchievements[i] && !mApp->mPlayerInfo->mShownAchievements[i]) {
				mApp->mPlayerInfo->mShownAchievements[i] = true;

				AchievementScreenItem aAchievementItem{};
				aAchievementItem.mId = i;
				aAchievementItem.mStartAnimTime = 100 * mAchievementItems.size() + 150;
				aAchievementItem.mEndAnimTime = aAchievementItem.mStartAnimTime + 100;
				aAchievementItem.mStartY = 750;
				aAchievementItem.mY = aAchievementItem.mStartY;
				mAchievementItems.push_back(aAchievementItem);
			}
		}

		if (!mAchievementItems.empty()) {
			//int aDestY = 284 - ((76 * mAchievementItems.size()) >> 1);
			int aDestY = 284 - ((76 * mAchievementItems.size()) / 2);

			for (size_t j = 0; j < mAchievementItems.size(); j++) {
				mAchievementItems[j].mDestY = aDestY;
				aDestY += 76;
			}
		}
		else {
			mShowingAchievements = false;
		}

		mApp->WriteCurrentUserConfig();
	}

	int aLevel = mApp->mPlayerInfo->GetLevel();
	if (mAwardType == AWARD_CREDITS_ZOMBIENOTE)
	{
		mLoadedResourceNames.push_back("DelayLoad_Background6");
		mLoadedResourceNames.push_back("DelayLoad_ZombieNote");
		mLoadedResourceNames.push_back("DelayLoad_Credits");
	}
	else if (mAwardType == AWARD_HELP_ZOMBIENOTE)
	{
		mLoadedResourceNames.push_back("DelayLoad_Background1");
		mLoadedResourceNames.push_back("DelayLoad_ZombieNote");
		mLoadedResourceNames.push_back("DelayLoad_ZombieNoteHelp");
	}
	else if (mApp->IsAdventureMode())
	{
		if (aLevel == 10)
		{
			mLoadedResourceNames.push_back("DelayLoad_Background1");
			mLoadedResourceNames.push_back("DelayLoad_ZombieNote");
			mLoadedResourceNames.push_back("DelayLoad_ZombieNote1");
		}
		else if (aLevel == 20)
		{
			mLoadedResourceNames.push_back("DelayLoad_Background2");
			mLoadedResourceNames.push_back("DelayLoad_ZombieNote");
			mLoadedResourceNames.push_back("DelayLoad_ZombieNote2");
		}
		else if (aLevel == 30)
		{
			mLoadedResourceNames.push_back("DelayLoad_Background1");
			mLoadedResourceNames.push_back("DelayLoad_ZombieNote");
			mLoadedResourceNames.push_back("DelayLoad_ZombieNote3");
		}
		else if (aLevel == 40)
		{
			mLoadedResourceNames.push_back("DelayLoad_Background2");
			mLoadedResourceNames.push_back("DelayLoad_ZombieNote");
			mLoadedResourceNames.push_back("DelayLoad_ZombieNote4");
		}
		else if (aLevel == 50)
		{
			mLoadedResourceNames.push_back("DelayLoad_Background1");
			mLoadedResourceNames.push_back("DelayLoad_ZombieNote");
			mLoadedResourceNames.push_back("DelayLoad_ZombieFinalNote");
		}
	}

	for (std::string& resource : mLoadedResourceNames)
		TodLoadResources(resource.c_str());

	mStartButton = new GameButton(AwardScreen::AwardScreen_Start);
	mStartButton->mButtonImage = Sexy::IMAGE_SEEDCHOOSER_BUTTON;
	mStartButton->mOverImage = nullptr;
	mStartButton->mDownImage = nullptr;
	mStartButton->mDisabledImage = Sexy::IMAGE_SEEDCHOOSER_BUTTON_DISABLED;
	mStartButton->mOverOverlayImage = Sexy::IMAGE_SEEDCHOOSER_BUTTON_GLOW;
	mStartButton->SetFont(Sexy::FONT_DWARVENTODCRAFT15);
	mStartButton->mColors[ButtonWidget::COLOR_LABEL] = Color(213, 159, 43);
	mStartButton->mColors[ButtonWidget::COLOR_LABEL_HILITE] = Color(213, 159, 43);
	mStartButton->Resize(324, 500, 156, 42);
	mStartButton->mTextOffsetY = -1;

	// @Patoke: implemented
	mContinueButton = new GameButton(AwardScreen::AwardScreen_Start);
	mContinueButton->mButtonImage = Sexy::IMAGE_SEEDCHOOSER_BUTTON;
	mContinueButton->mOverImage = nullptr;
	mContinueButton->mDownImage = nullptr;
	mContinueButton->mDisabledImage = Sexy::IMAGE_SEEDCHOOSER_BUTTON_DISABLED;
	mContinueButton->mOverOverlayImage = Sexy::IMAGE_SEEDCHOOSER_BUTTON_GLOW;
	mContinueButton->SetFont(Sexy::FONT_DWARVENTODCRAFT15);
	mContinueButton->mColors[ButtonWidget::COLOR_LABEL] = Color(213, 159, 43);
	mContinueButton->mColors[ButtonWidget::COLOR_LABEL_HILITE] = Color(213, 159, 43);
	mContinueButton->Resize(324, 515, 156, 42);
	mContinueButton->mParentWidget = this;
	mContinueButton->mTextOffsetY = -1;
	mContinueButton->mBtnNoDraw = true;
	mContinueButton->mDisabled = true;

	mMenuButton = new GameButton(AwardScreen::AwardScreen_Menu);
	mMenuButton->SetLabel("[AWARD_MAIN_MENU_BUTTON]");
	mMenuButton->mButtonImage = Sexy::IMAGE_SEEDCHOOSER_BUTTON2;
	mMenuButton->mOverImage = Sexy::IMAGE_SEEDCHOOSER_BUTTON2_GLOW;
	mMenuButton->mDownImage = nullptr;
	mMenuButton->SetFont(Sexy::FONT_BRIANNETOD12);
	mMenuButton->mColors[ButtonWidget::COLOR_LABEL] = Color(42, 42, 90);
	mMenuButton->mColors[ButtonWidget::COLOR_LABEL_HILITE] = Color(42, 42, 90);
	mMenuButton->mParentWidget = this;
	mMenuButton->Resize(677, 16, 111, 26);
	mMenuButton->mTextOffsetY = 1;
	if (!mApp->HasFinishedAdventure() && aLevel <= 3)
	{
		mMenuButton->mBtnNoDraw = true;
		mMenuButton->mDisabled = true;
	}

	if (mAwardType == AWARD_CREDITS_ZOMBIENOTE)
	{
		mStartButton->SetLabel("[ROLL_CREDITS]");
		mStartButton->mButtonImage = Sexy::IMAGE_CREDITS_PLAYBUTTON;
		mStartButton->mOverImage = nullptr;
		mStartButton->mDownImage = nullptr;
		mStartButton->mDisabledImage = nullptr;
		mStartButton->mOverOverlayImage = nullptr;
		mStartButton->SetFont(Sexy::FONT_HOUSEOFTERROR20);
		mStartButton->mColors[ButtonWidget::COLOR_LABEL] = Color(255, 255, 255);
		mStartButton->mColors[ButtonWidget::COLOR_LABEL_HILITE] = Color(213, 159, 43);
		mStartButton->Resize(325, 505, 190, 73);
		mStartButton->mTextOffsetX = 33;
		mStartButton->mTextOffsetY = -2;
		mStartButton->mButtonOffsetX = -2;
		mStartButton->mButtonOffsetY = 8;
		mStartButton->mParentWidget = this;
	}
	else if (mAwardType == AWARD_HELP_ZOMBIENOTE)
	{
		mStartButton->SetLabel("[MAIN_MENU_BUTTON]");
		mMenuButton->mBtnNoDraw = true;
		mMenuButton->mDisabled = true;
	}
	else if (!mApp->IsAdventureMode())
	{
		mStartButton->SetLabel("[MAIN_MENU_BUTTON]");
		mMenuButton->mBtnNoDraw = true;
		mMenuButton->mDisabled = true;
	}
	else if (aLevel == 1 && mApp->HasFinishedAdventure())
	{
		ReportAchievement::GiveAchievement(mApp, HomeSecurity, false); // @Patoke: add achievement
		mStartButton->SetLabel("[CONTINUE_BUTTON]");
		mMenuButton->mBtnNoDraw = true;
		mMenuButton->mDisabled = true;
	}
	else if (aLevel == 15)
		mStartButton->SetLabel("[VIEW_ALMANAC_BUTTON]");
	else if (aLevel == 25 || aLevel == 35 || aLevel == 45)
		mStartButton->SetLabel("[CONTINUE_BUTTON]");
	else
		mStartButton->SetLabel("[NEXT_LEVEL_BUTTON]");

	// @Patoke: implemented
	if (mApp->IsAdventureMode() && mApp->EarnedGoldTrophy()) {
		ReportAchievement::GiveAchievement(mApp, NovelPeasPrize, false);
	}

	if (mApp->IsFirstTimeAdventureMode() && aLevel == 25 && mApp->IsTrialStageLocked() && !mApp->mPlayerInfo->mHasSeenUpsell)
	{
		mMenuButton->mBtnNoDraw = true;
		mMenuButton->mDisabled = true;
	}

	// @Patoke: implemented
	if (mShowingAchievements) {
		mShowStartButtonAfterAchievements = !mStartButton->mBtnNoDraw;
		mShowMenuButtonAfterAchievements = !mMenuButton->mBtnNoDraw;
		mStartButton->mBtnNoDraw = true;
		mStartButton->mDisabled = true;
		mMenuButton->mBtnNoDraw = true;
		mMenuButton->mDisabled = true;
		mContinueButton->SetLabel("[CONTINUE_BUTTON]");
	}

	if (IsPaperNote())
	{
		mApp->mMusic->StopAllMusic();
		mStartButton->mY += 20;
		mApp->PlayFoley(FOLEY_PAPER);
	}
	else
		mApp->mMusic->MakeSureMusicIsPlaying(MUSIC_TUNE_ZEN_GARDEN);
}

AwardScreen::~AwardScreen()
{
	if (mStartButton) delete mStartButton;
	if (mContinueButton) delete mContinueButton; // @Patoke: add new button
	if (mMenuButton) delete mMenuButton;
}

bool AwardScreen::IsPaperNote()
{
	if (mAwardType == AWARD_CREDITS_ZOMBIENOTE || mAwardType == AWARD_HELP_ZOMBIENOTE)
		return true;

	int aLevel = mApp->mPlayerInfo->GetLevel();
	return mApp->IsAdventureMode() && (aLevel == 10 || aLevel == 20 || aLevel == 30 || aLevel == 40 || aLevel == 50);
}

void AwardScreen::DrawBottom(Graphics* g, std::string_view theTitle, std::string_view theAward, std::string_view theMessage)
{
	g->DrawImage(Sexy::IMAGE_AWARDSCREEN_BACK, 0, 0);
	TodDrawString(g, theTitle, BOARD_WIDTH / 2, 58, Sexy::FONT_DWARVENTODCRAFT24, Color(213, 159, 43), DS_ALIGN_CENTER);
	TodDrawString(g, theAward, BOARD_WIDTH / 2, 326, Sexy::FONT_DWARVENTODCRAFT18YELLOW, Color::White, DS_ALIGN_CENTER);
	TodDrawStringWrapped(g, theMessage, Rect(285, 360, 230, 90), Sexy::FONT_BRIANNETOD16, Color(40, 50, 90), DS_ALIGN_CENTER_VERTICAL_MIDDLE);
}

void AwardScreen::DrawAwardSeed(Graphics* g)
{
	SeedType aSeedType = mApp->GetAwardSeedForLevel(mApp->mPlayerInfo->GetLevel() - 1);
	std::string aAward = Plant::GetNameString(aSeedType, SEED_NONE);
	std::string aMessage;
	if (mApp->IsTrialStageLocked() && aSeedType >= SEED_SQUASH && aSeedType != SEED_TANGLEKELP)
		aMessage = "[AVAILABLE_IN_FULL_VERSION]";
	else
		aMessage = Plant::GetToolTip(aSeedType);
	DrawBottom(g, "[NEW_PLANT]", aAward, aMessage);

	g->SetScale(2, 2, 350, 129);
	DrawSeedPacket(g, 350, 129, aSeedType, SEED_NONE, 0, 255, true, false);
	g->SetScale(1, 1, 0, 0);
}

// GOTY @Patoke: 0x4081C0
void AwardScreen::Draw(Graphics* g)
{
	g->SetLinearBlend(true);

	int aLevel = mApp->mPlayerInfo->GetLevel();
	if (mShowingAchievements) // @Patoke: add call
		DrawAchievements(g);
	else if (mAwardType == AWARD_CREDITS_ZOMBIENOTE)
	{
		g->SetColor(Color(125, 200, 255, 255));
		g->SetColorizeImages(true);
		g->DrawImage(Sexy::IMAGE_BACKGROUND6BOSS, -900, -400, 2800, 1200);
		g->SetColorizeImages(false);
		g->SetColor(Color(0, 0, 0, 64));
		g->FillRect(0, 525, BOARD_WIDTH, BOARD_HEIGHT);
		g->DrawImage(Sexy::IMAGE_ZOMBIE_NOTE, 75, 60);
		g->DrawImage(Sexy::IMAGE_CREDITS_ZOMBIENOTE, 149, 103, 475, 325);
	}
	else if (mAwardType == AWARD_HELP_ZOMBIENOTE)
	{
		g->DrawImage(Sexy::IMAGE_BACKGROUND1, -700, -300, 2800, 1200);
		g->DrawImage(Sexy::IMAGE_ZOMBIE_NOTE, 80, 80);
		g->DrawImage(Sexy::IMAGE_ZOMBIE_NOTE_HELP, 131, 132);
	}
	else if (mAwardType != AWARD_ACHIEVEMENTONLY) // @Patoke: add check
	{
		if (!mApp->IsAdventureMode())
		{
			if (mApp->EarnedGoldTrophy())
			{
				DrawBottom(g, "[BEAT_GAME_MESSAGE1]", "[GOLD_SUNFLOWER_TROPHY]", "[BEAT_GAME_MESSAGE2]");
				TodDrawImageCelCenterScaledF(g, Sexy::IMAGE_SUNFLOWER_TROPHY, 330, 80, 1, 0.7f, 0.7f);
			}
			else
			{
				const char* aMsgChar;
				if (mApp->IsSurvivalMode())
				{
					int aNumTrophies = mApp->GetNumTrophies(CHALLENGE_PAGE_SURVIVAL);
					aMsgChar =
						aNumTrophies <= 7 ? "[YOU_UNLOCKED_A_SURVIVAL]" :
						aNumTrophies == 10 ? "[YOU_UNLOCKED_ENDLESS_SURVIVAL]" : "[EARN_MORE_TROPHIES_FOR_ENDLESS_SURVIVAL]";

				}
				else if (mApp->IsScaryPotterLevel())
					aMsgChar = "[UNLOCKED_VASEBREAKER_LEVEL]";
				else if (mApp->IsPuzzleMode())
					aMsgChar = "[UNLOCKED_I_ZOMBIE_LEVEL]";
				else
					aMsgChar = mApp->GetNumTrophies(CHALLENGE_PAGE_CHALLENGE) <= 17 ? "[CHALLENGE_UNLOCKED]" : "[GET_MORE_TROPHIES]";

				DrawBottom(g, "[GOT_TROPHY]", "[TROPHY]", aMsgChar);
				g->DrawImage(Sexy::IMAGE_TROPHY_HI_RES, BOARD_WIDTH / 2 - Sexy::IMAGE_TROPHY_HI_RES->mWidth / 2, 137);
			}
		}
		else if (aLevel == 5)
		{
			DrawBottom(g, "[GOT_SHOVEL]", "[SHOVEL]", "[SHOVEL_DESCRIPTION]");
			g->DrawImage(Sexy::IMAGE_SHOVEL_HI_RES, BOARD_WIDTH / 2 - Sexy::IMAGE_SHOVEL_HI_RES->mWidth / 2, 137);
		}
		else if (aLevel == 10)
		{
			g->DrawImage(Sexy::IMAGE_BACKGROUND1, -700, -300, 2800, 1200);
			g->DrawImage(Sexy::IMAGE_ZOMBIE_NOTE, 80, 80);
			g->DrawImage(Sexy::IMAGE_ZOMBIE_NOTE1, 131, 132);
			TodDrawString(g, "[FOUND_NOTE]", BOARD_WIDTH / 2, 70, Sexy::FONT_DWARVENTODCRAFT24, Color(255, 200, 0, 255), DS_ALIGN_CENTER);
		}
		else if (aLevel == 15)
		{
			DrawBottom(g, "[FOUND_SUBURBAN_ALMANAC]", "[SUBURBAN_ALMANAC]", "[SUBURBAN_ALMANAC_DESCRIPTION]");
			g->DrawImage(Sexy::IMAGE_ALMANAC, BOARD_WIDTH / 2 - Sexy::IMAGE_ALMANAC->mWidth / 2, 160);
		}
		else if (aLevel == 20)
		{
			g->DrawImage(Sexy::IMAGE_BACKGROUND2, -700, -300, 2800, 1200);
			g->DrawImage(Sexy::IMAGE_ZOMBIE_NOTE, 80, 80);
			g->DrawImage(Sexy::IMAGE_ZOMBIE_NOTE2, 133, 127);
			TodDrawString(g, "[FOUND_NOTE]", BOARD_WIDTH / 2, 70, Sexy::FONT_DWARVENTODCRAFT24, Color(255, 200, 0, 255), DS_ALIGN_CENTER);
		}
		else if (aLevel == 25)
		{
			DrawBottom(g, "[FOUND_KEYS]", "[KEYS]", "[KEYS_DESCRIPTION]");
			g->DrawImage(Sexy::IMAGE_CARKEYS, BOARD_WIDTH / 2 - Sexy::IMAGE_CARKEYS->mWidth / 2, 160);
		}
		else if (aLevel == 30)
		{
			g->DrawImage(Sexy::IMAGE_BACKGROUND1, -700, -300, 2800, 1200);
			g->DrawImage(Sexy::IMAGE_ZOMBIE_NOTE, 80, 80);
			g->DrawImage(Sexy::IMAGE_ZOMBIE_NOTE3, 120, 117);
			TodDrawString(g, "[FOUND_NOTE]", BOARD_WIDTH / 2, 70, Sexy::FONT_DWARVENTODCRAFT24, Color(255, 200, 0, 255), DS_ALIGN_CENTER);
		}
		else if (aLevel == 35)
		{
			DrawBottom(g, "[FOUND_TACO]", "[TACO]", "[TACO_DESCRIPTION]");
			g->DrawImage(Sexy::IMAGE_TACO, BOARD_WIDTH / 2 - Sexy::IMAGE_TACO->mWidth / 2, 160);
		}
		else if (aLevel == 40)
		{
			g->DrawImage(Sexy::IMAGE_BACKGROUND2, -700, -300, 2800, 1200);
			g->DrawImage(Sexy::IMAGE_ZOMBIE_NOTE, 80, 80);
			g->DrawImage(Sexy::IMAGE_ZOMBIE_NOTE4, 102, 117);
			TodDrawString(g, "[FOUND_NOTE]", BOARD_WIDTH / 2, 70, Sexy::FONT_DWARVENTODCRAFT24, Color(255, 200, 0, 255), DS_ALIGN_CENTER);
		}
		else if (aLevel == 45)
		{
			DrawBottom(g, "[FOUND_WATERING_CAN]", "[WATERING_CAN]", "[WATERING_CAN_DESCRIPTION]");
			g->DrawImage(Sexy::IMAGE_WATERINGCAN, BOARD_WIDTH / 2 - Sexy::IMAGE_WATERINGCAN->mWidth / 2, 160);
		}
		else if (aLevel == 50)
		{
			g->DrawImage(Sexy::IMAGE_BACKGROUND1, -700, -300, 2800, 1200);
			g->DrawImage(Sexy::IMAGE_ZOMBIE_NOTE, 80, 80);
			g->DrawImage(Sexy::IMAGE_ZOMBIE_FINAL_NOTE, 114, 138);
			TodDrawString(g, "[FOUND_NOTE]", BOARD_WIDTH / 2, 70, Sexy::FONT_DWARVENTODCRAFT24, Color(255, 200, 0, 255), DS_ALIGN_CENTER);
		}
		else if (aLevel == 1 && mApp->HasFinishedAdventure())
		{
			DrawBottom(g, "[WIN_MESSAGE1]", "[SILVER_SUNFLOWER_TROPHY]", "[WIN_MESSAGE2]");
			TodDrawImageCelCenterScaledF(g, Sexy::IMAGE_SUNFLOWER_TROPHY, 325, 65, 0, 0.7f, 0.7f);
		}
		else
		{
			DrawAwardSeed(g);
		}
	}

	mStartButton->Draw(g);
	mMenuButton->Draw(g);
	mContinueButton->Draw(g); // @Patoke: add call

	int aFadeInAlpha = TodAnimateCurve(180, 0, mFadeInCounter, 255, 0, CURVE_LINEAR);
	g->SetColor(IsPaperNote() ? Color(0, 0, 0, aFadeInAlpha) : Color(255, 255, 255, aFadeInAlpha));
	g->FillRect(0, 0, BOARD_WIDTH, BOARD_HEIGHT);
}

// GOTY @Patoke: 0x408FE0
void AwardScreen::Update()
{
	Widget::Update();
	// @Patoke: implemented
	if (mShowingAchievements) {
		mAchievementAnimTime++;

		for (size_t i = 0; i < mAchievementItems.size(); i++) {
			if (mAchievementAnimTime >= mAchievementItems[i].mStartAnimTime) {
				mAchievementItems[i].mY = TodAnimateCurve(mAchievementItems[i].mStartAnimTime, mAchievementItems[i].mEndAnimTime, mAchievementAnimTime, mAchievementItems[i].mStartY, mAchievementItems[i].mDestY, CURVE_EASE_IN_OUT);
			}

			if (mAchievementItems[mAchievementItems.size() - 1].mY == mAchievementItems[mAchievementItems.size() - 1].mDestY) {
				mContinueButton->mBtnNoDraw = false;
				mContinueButton->mDisabled = false;
			}
		}
	}

	if (mApp->GetDialogCount() > 0) return;
	mStartButton->Update();
	mMenuButton->Update();
	mContinueButton->Update(); // @Patoke: add call
	mApp->SetCursor(mStartButton->IsMouseOver() || mMenuButton->IsMouseOver() || mContinueButton->IsMouseOver() ? CURSOR_HAND : CURSOR_POINTER);
	MarkDirty();
	if (mFadeInCounter > 0) mFadeInCounter--;
}

void AwardScreen::KeyDown(KeyCode theKey)
{
	if (theKey == KeyCode::KEYCODE_SPACE || theKey == KeyCode::KEYCODE_RETURN)
	{
		StartButtonPressed();
		return;
	}

	if (theKey == KeyCode::KEYCODE_ESCAPE)
	{
		if (!mMenuButton->mDisabled && !mMenuButton->mBtnNoDraw)
		{
			mApp->KillAwardScreen();
			mApp->ShowGameSelector();
		}
		else
		{
			StartButtonPressed();
		}
	}
}

// GOTY @Patoke: 0x409530
void AwardScreen::StartButtonPressed()
{
	if (mApp->GetDialog(DIALOG_STORE))
		return;

	if (mAwardType == AWARD_CREDITS_ZOMBIENOTE)
	{
		mApp->KillAwardScreen();
		mApp->ShowCreditScreen();
	}
	else if (mAwardType == AWARD_HELP_ZOMBIENOTE)
	{
		mApp->KillAwardScreen();
		mApp->ShowGameSelector();
	}
	else if (mApp->IsSurvivalMode())
	{
		mApp->KillAwardScreen();
		mApp->ShowChallengeScreen(CHALLENGE_PAGE_SURVIVAL);
	}
	else if (mApp->IsPuzzleMode())
	{
		mApp->KillAwardScreen();
		mApp->ShowChallengeScreen(CHALLENGE_PAGE_PUZZLE);
	}
	else if (mApp->IsChallengeMode())
	{
		mApp->KillAwardScreen();
		mApp->ShowChallengeScreen(CHALLENGE_PAGE_CHALLENGE);
	}
	else
	{
		int aLevel = mApp->mPlayerInfo->GetLevel();
		if (aLevel == 1)
		{
			mApp->KillAwardScreen();
			if (mApp->HasFinishedAdventure())
			{
				mApp->ShowAwardScreen(AWARD_CREDITS_ZOMBIENOTE, false);
			}
			else
			{
				mApp->PreNewGame(GAMEMODE_ADVENTURE, false);
			}
		}
		else
		{
			if (aLevel == 15)
			{
				mApp->DoAlmanacDialog()->WaitForResult();
			}
			else if (aLevel == 25)
			{
				StoreScreen* aStore = mApp->ShowStoreScreen();
				aStore->SetupForIntro(301);
				aStore->WaitForResult(true);

				if (aStore->mPurchasedFullVersion)
				{
					mApp->KillAwardScreen();
					mApp->ShowGameSelector();
					return;
				}
				if (mApp->IsTrialStageLocked())
				{
					mApp->KillAwardScreen();
					mApp->PreNewGame(GAMEMODE_UPSELL, false);
					if (!mApp->mPlayerInfo->mHasSeenUpsell)
					{
						mApp->mBoard->mStoreButton->mBtnNoDraw = true;
						mApp->mPlayerInfo->mHasSeenUpsell = true;
					}
					return;
				}
			}
			else if (aLevel == 35)
			{
				StoreScreen* aStore = mApp->ShowStoreScreen();
				aStore->SetupForIntro(601);
				aStore->WaitForResult(true);
			}
			else if (aLevel == 42)
			{
				StoreScreen* aStore = mApp->ShowStoreScreen();
				aStore->SetupForIntro(3100);
				aStore->WaitForResult(true);
			}
			else if (aLevel == 45)
			{
				mApp->KillAwardScreen();
				mApp->PreNewGame(GAMEMODE_CHALLENGE_ZEN_GARDEN, false);
				mApp->mZenGarden->SetupForZenTutorial();
				return;
			}

			mApp->KillAwardScreen();
			mApp->PreNewGame(GAMEMODE_ADVENTURE, false);
		}
	}
}

// GOTY @Patoke: 0x4097A0
void AwardScreen::MouseDown(int x, int y, int theClickCount)
{
	(void)x;(void)y;
	if (theClickCount == 1) {
		if (mStartButton->IsMouseOver() || mMenuButton->IsMouseOver() || mContinueButton->IsMouseOver())
			mApp->PlaySample(Sexy::SOUND_TAP);
	}
}

// GOTY @Patoke: 0x409840
void AwardScreen::MouseUp(int x, int y, int theClickCount)
{
	(void)x;(void)y;
	if (theClickCount == 1)
	{
		if (mStartButton->IsMouseOver())
			StartButtonPressed();
		if (mContinueButton->IsMouseOver()) // @Patoke: add call
			AchievementsContinuePressed();
		if (mMenuButton->IsMouseOver())
		{
			mApp->KillAwardScreen();
			mApp->ShowGameSelector();
		}
	}
}

// @Patoke: implement functions
// GOTY @Patoke: 0x407C20
void AwardScreen::DrawAchievements(Graphics* g) {
	g->SetColorizeImages(true);
	// Rect aTextWrap = Rect(0, 0, 0, 77); // unused
	g->SetColor(Color(255, 255, 255));
	g->FillRect(0, 0, mWidth, mHeight);
	g->SetColorizeImages(false);

	g->DrawImage(IMAGE_CHALLENGE_BACKGROUND, 0, 0);

	TodDrawString(g, mApp->GetString("ACHIEVEMENTS_TITLE", "ACHIEVEMENTS"), BOARD_WIDTH / 2, 58, FONT_HOUSEOFTERROR28, Color(220, 220, 220), DS_ALIGN_CENTER);

	for (size_t i = 0; i < mAchievementItems.size(); i++) {
		std::string aAchievementName = std::string(gAchievementList[mAchievementItems[i].mId].name);
		std::string aAchievementDesc = std::string(gAchievementList[mAchievementItems[i].mId].description);
		aAchievementName.append(" Earned!");

		Rect aSrcRect = Rect(70 * (mAchievementItems[i].mId % 7), 70 * (mAchievementItems[i].mId / 7), 70, 70);
		Rect aDestRect = Rect(220, mAchievementItems[i].mY + 10, 70, 70);
		Rect aTextRect = Rect(300, mAchievementItems[i].mY + 20, 300, 60);

		g->DrawImage(IMAGE_ACHEESEMENTS_ICONS, aDestRect, aSrcRect);

		TodDrawString(g, aAchievementName, 450, mAchievementItems[i].mY + 25, FONT_DWARVENTODCRAFT15, Color(224, 187, 98), DS_ALIGN_CENTER);
		TodDrawStringWrapped(g, aAchievementDesc, aTextRect, FONT_DWARVENTODCRAFT12, Color(255, 255, 255), DS_ALIGN_CENTER_VERTICAL_MIDDLE);
	}
}

// GOTY @Patoke: 0x409400
void AwardScreen::AchievementsContinuePressed() {
	if (mAwardType == AWARD_ACHIEVEMENTONLY) {
		mApp->KillAwardScreen();
		if (mApp->IsAdventureMode()) {
			mApp->PreNewGame(mApp->mGameMode, false);
		}
		else if (mApp->IsSurvivalMode()) {
			mApp->ShowChallengeScreen(ChallengePage::CHALLENGE_PAGE_SURVIVAL);
		}
		else if (mApp->IsPuzzleMode()) {
			mApp->ShowChallengeScreen(ChallengePage::CHALLENGE_PAGE_PUZZLE);
		}
		else {
			mApp->ShowChallengeScreen(ChallengePage::CHALLENGE_PAGE_CHALLENGE);
		}
	}
	else {
		mStartButton->mBtnNoDraw = !mShowStartButtonAfterAchievements;
		mStartButton->mDisabled = !mShowStartButtonAfterAchievements;
		mMenuButton->mBtnNoDraw = !mShowMenuButtonAfterAchievements;
		mMenuButton->mDisabled = !mShowMenuButtonAfterAchievements;
		mContinueButton->mDisabled = true;
		mContinueButton->mBtnNoDraw = true;
		mShowingAchievements = false;
		int level = mApp->mPlayerInfo->GetLevel();
		if (mApp->IsAdventureMode() && level == 1 && mApp->HasFinishedAdventure()) {
			mApp->KillAwardScreen();
			mApp->ShowAwardScreen(AWARD_CREDITS_ZOMBIENOTE, false);
		}
	}
}
