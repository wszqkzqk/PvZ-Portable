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

// @Patoke: implement file
#include "AchievementsScreen.h"
#include "../Board.h"
#include "GameButton.h"
#include "GameSelector.h"
#include "../../LawnApp.h"
#include "AlmanacDialog.h"
#include "../../Resources.h"
#include "../System/Music.h"
#include "../../GameConstants.h"
#include "../System/PlayerInfo.h"
#include "../System/ProfileMgr.h"
#include "../../Sexy.TodLib/TodFoley.h"
#include "../../Sexy.TodLib/TodDebug.h"
#include "graphics/Font.h"
#include "../../Sexy.TodLib/Reanimator.h"
#include "../../Sexy.TodLib/TodParticle.h"
#include "widget/Dialog.h"
#include "widget/WidgetManager.h"

Rect aBackButtonRect = { 120, 35, 130, 80 };

AchievementItem gAchievementList[MAX_ACHIEVEMENTS] = {
	{ "Home Lawn Security", "Complete Adventure Mode." },
	{ "Nobel Peas Prize", "Get the golden sunflower trophy." },
	{ "Better Off Dead", "Get to a streak of 10 in I, Zombie Endless" },
	{ "China Shop", "Get to a streak of 15 in Vasebreaker Endless" },
	{ "SPUDOW!", "Blow up a zombie using a Potato Mine." },
	{ "Explodonator", "Take out 10 full-sized zombies with a single Cherry Bomb." },
	{ "Morticulturalist", "Collect all 49 plants (including plants from Crazy Dave's shop)." },
	{ "Don't Pea in the Pool", "Complete a daytime pool level without using pea shooters of any kind." },
	{ "Roll Some Heads", "Bowl over 5 zombies with a single Wall-Nut." },
	{ "Grounded", "Defeat a normal roof level without using any catapult plants." },
	{ "Zombologist", "Discover the Yeti zombie." },
	{ "Penny Pincher", "Pick up 30 coins in a row on a single level without letting any disappear." },
	{ "Sunny Days", "Get 8000 sun during a single level." },
	{ "Popcorn Party", "Defeat 2 Gargantuars with Corn Cob missiles in a single level." },
	{ "Good Morning", "Complete a daytime level by planting only Mushrooms and Coffee Beans." },
	{ "No Fungus Among Us", "Complete a nighttime level without planting any Mushrooms." },
	{ "Beyond the Grave", "Beat all 20 mini games." },
	{ "Immortal", "Survive 20 waves of pure zombie ferocity." },
	{ "Towering Wisdom", "Grow the Tree of Wisdom to 100 feet." },
	{ "Mustache Mode", "Enable Mustache Mode" }
};

// GOTY @Patoke: 0x401000
AchievementsWidget::AchievementsWidget(LawnApp* theApp) {
	mApp = theApp;
	mWidth = 800;
	mHeight = IMAGE_ACHEESEMENTS_CHINA->mHeight + IMAGE_SELECTORSCREEN_ACHIEVEMENTS_BG->mHeight + 15700;
	mScrollDirection = -1;
	mScrollValue = 0;
	mDefaultScrollValue = 30;
	mScrollDecay = 1;
	mDidPressMoreButton = false;
	mMoreRockRect = Rect(710, 470, IMAGE_ACHEESEMENTS_MORE_ROCK->mWidth - 25, IMAGE_ACHEESEMENTS_MORE_ROCK->mHeight - 50);
}

// GOTY @Patoke: 0x4010E0
AchievementsWidget::~AchievementsWidget() {

}

// GOTY @Patoke: 0x401A10
void AchievementsWidget::Update() {
	MarkDirty();
	if (mScrollValue <= 0)
		return;

	if (mScrollValue > mDefaultScrollValue)
		mScrollValue = mDefaultScrollValue;

	mScrollValue -= mScrollDecay;

	int aNewY = mY + mScrollValue * mScrollDirection;
	if (aNewY >= -1)
		aNewY = -1;
	//if (aNewY >= mApp->mHeight)
	//	aNewY = mApp->mHeight;

	int aMaxScroll = 2 * mApp->mHeight + 50 - mHeight;
	if (aNewY <= aMaxScroll)
		aNewY = aMaxScroll;

	mY = aNewY;

	int aDelta = aNewY - mY;
	mMoreRockRect.mY += aDelta;
	aBackButtonRect.mY += aDelta;

	if (mScrollValue <= 0)
		mScrollValue = 0;
}

// GOTY @Patoke: 0x401160
void AchievementsWidget::Draw(Graphics* g) {
	g->DrawImage(IMAGE_SELECTORSCREEN_ACHIEVEMENTS_BG, 0, 0);

	int aHeight = IMAGE_SELECTORSCREEN_ACHIEVEMENTS_BG->mHeight;
	for (int i = 1; i <= 70; i++)
		g->DrawImage(IMAGE_ACHEESEMENTS_HOLE_TILE, 0, aHeight * i);

	g->DrawImage(IMAGE_ACHEESEMENTS_BOOKWORM, 0, 1125);
	g->DrawImage(IMAGE_ACHEESEMENTS_BEJEWELED, 0, 2250);
	g->DrawImage(IMAGE_ACHEESEMENTS_CHUZZLE, 0, 4500);
	g->DrawImage(IMAGE_ACHEESEMENTS_PEGGLE, 0, 6750);
	g->DrawImage(IMAGE_ACHEESEMENTS_PIPE, 0, 9000);
	g->DrawImage(IMAGE_ACHEESEMENTS_ZUMA, 0, 11250);

	g->DrawImage(IMAGE_ACHEESEMENTS_CHINA, 0, mHeight - IMAGE_ACHEESEMENTS_CHINA->mHeight - /*50*/ 650);
	
	if (aBackButtonRect.Contains(mWidgetManager->mLastMouseX - mX, mWidgetManager->mLastMouseY - mY))
		g->DrawImage(IMAGE_ACHEESEMENTS_BACK_HIGHLIGHT, 128, 55);

	for (int i = 0; i < MAX_ACHIEVEMENTS; i++) {
		bool aHasAchievement;
		if (mApp->mPlayerInfo) aHasAchievement = mApp->mPlayerInfo->mEarnedAchievements[i];
		else aHasAchievement = false;

		int aCurrAchievementOff = 57 * int(i / 2);
		int aImageXPos = i % 2 == 0 ? 120 : 410;
		int aImageYPos = 178 + aCurrAchievementOff;
		int aTextXPos = aImageXPos + 70;
		int aTextYPos = aImageYPos + 16;

		// Achievement images
		Rect aSrcRect(70 * (i % 7), 70 * (i / 7), 70, 70);
		Rect aDestRect(aImageXPos, aImageYPos, 56, 56);
		
		g->SetColorizeImages(true);
		g->SetColor(aHasAchievement ? Color(255, 255, 255) : Color(255, 255, 255, 32));

		g->DrawImage(IMAGE_ACHEESEMENTS_ICONS, aDestRect, aSrcRect);
		g->SetColorizeImages(false);
		
		// Achievement titles
		g->SetFont(FONT_DWARVENTODCRAFT15);
		g->SetColor(Color(21, 175, 0));

		std::string aName = mApp->GetString(gAchievementList[i].name, gAchievementList[i].name);
		g->DrawString(aName, aTextXPos, aTextYPos);

		// Achievement descriptions	
		Rect aPos = Rect(aTextXPos, aTextYPos + 3, 212, 60);
		
		g->SetFont(FONT_DWARVENTODCRAFT12);
		g->SetColor(Color(255, 255, 255));

		std::string aDesc = mApp->GetString(gAchievementList[i].description, gAchievementList[i].description);
		g->WriteWordWrapped(aPos, aDesc, 12);
	}

	g->DrawImage(IMAGE_ACHEESEMENTS_MORE_ROCK, 700, 450);

	bool aIsHighlight = mMoreRockRect.Contains(mWidgetManager->mLastMouseX - mX, mWidgetManager->mLastMouseY - mY);
	if (mDidPressMoreButton) {
		g->DrawImage(aIsHighlight ? IMAGE_ACHEESEMENTS_TOP_BUTTON_HIGHLIGHT : IMAGE_ACHEESEMENTS_TOP_BUTTON, 700, 450);
	}
	else {
		g->DrawImage(aIsHighlight ? IMAGE_ACHEESEMENTS_MORE_BUTTON_HIGHLIGHT : IMAGE_ACHEESEMENTS_MORE_BUTTON, 700, 450);
	}
}

// GOTY @Patoke: 0x4019D0
void AchievementsWidget::KeyDown(KeyCode theKey) {
	if (theKey == KEYCODE_UP) {
		mScrollValue = mDefaultScrollValue;
		mScrollDirection = 1;
	}
	else if (theKey == KEYCODE_DOWN) {
		mScrollValue = mDefaultScrollValue;
		mScrollDirection = -1;
	}
	else if (theKey == KEYCODE_ESCAPE) {
		mApp->mGameSelector->SlideTo(0, 0);
		mApp->mGameSelector->mWidgetManager->SetFocus(mApp->mGameSelector);
	}
}

// GOTY @Patoke: 0x4017F0
void AchievementsWidget::MouseDown(int x, int y, int theClickCount) {
	(void)theClickCount;
	if (aBackButtonRect.Contains(x, y))
		mApp->PlaySample(SOUND_GRAVEBUTTON);

	if (mMoreRockRect.Contains(x, y))
		mApp->PlaySample(SOUND_GRAVEBUTTON);
}

// GOTY @Patoke: 0x401890
void AchievementsWidget::MouseUp(int x, int y, int theClickCount) {
	Point aPos = Point(x, y);
	if (aBackButtonRect.Contains(aPos)) {
		mApp->mGameSelector->SlideTo(0, 0);
		mApp->mGameSelector->mWidgetManager->SetFocus(mApp->mGameSelector);
	}

	if (mMoreRockRect.Contains(aPos)) {
		mDidPressMoreButton = !mDidPressMoreButton;
		mScrollDirection = mDidPressMoreButton ? -1 : 1;
		mScrollValue = 20;
	}

	(void)theClickCount;
}

// GOTY @Patoke: 0x4019A0
void AchievementsWidget::MouseWheel(int theDelta) {
	mScrollValue = mDefaultScrollValue;

	if (theDelta > 0)
		mScrollDirection = 1;
	else if (theDelta < 0)
		mScrollDirection = -1;
}

// GOTY @Patoke: 0x459670
void ReportAchievement::GiveAchievement(LawnApp* theApp, int theAchievement, bool theForceGive) {
	// todo @Patoke: finish adding the achievement give events
	if (!theApp->mPlayerInfo)
		return;

	if (theApp->mPlayerInfo->mEarnedAchievements[theAchievement])
		return;

	theApp->mPlayerInfo->mEarnedAchievements[theAchievement] = true;

	if (!theForceGive)
		return;

	std::string aAchievementName = theApp->GetString(gAchievementList[theAchievement].name, gAchievementList[theAchievement].name);
	std::string aFormat = theApp->GetString("%s Achievement!", "%s Achievement!");
	std::string aMessage = Sexy::StrFormat(aFormat.c_str(), aAchievementName.c_str());

	if (theApp->mBoard)
		theApp->mBoard->DisplayAdvice(aMessage, MESSAGE_STYLE_ACHIEVEMENT, AdviceType::ADVICE_NONE);
	theApp->PlaySample(SOUND_ACHIEVEMENT);
}

// GOTY @Patoke: 0x44D5B0
void ReportAchievement::AchievementInitForPlayer(LawnApp* theApp) {
	if (!theApp || !theApp->mPlayerInfo)
		return;
	// @windowslover1234, for whatever reason, these three checks cause crashes on Windows, but not Linux.
	if (theApp->HasFinishedAdventure()) {
		GiveAchievement(theApp, AchievementId::HomeSecurity, true);
	}

	if (theApp->EarnedGoldTrophy()) {
		GiveAchievement(theApp, AchievementId::NovelPeasPrize, true);
	}

	if (theApp->CanSpawnYetis()) {
		GiveAchievement(theApp, AchievementId::Zombologist, true);
	}

	int aTreeSize = theApp->mPlayerInfo->mChallengeRecords[GAMEMODE_TREE_OF_WISDOM - GAMEMODE_SURVIVAL_NORMAL_STAGE_1];
	if (aTreeSize >= 100) {
		GiveAchievement(theApp, AchievementId::ToweringWisdom, true);
	}

	bool aGiveAchievement = true;
	for (int aSeedType = SeedType::SEED_GATLINGPEA; aSeedType <= SeedType::SEED_IMITATER; aSeedType++) {
		if (!theApp->HasSeedType(SeedType(aSeedType)))
			aGiveAchievement = false;
	}

	if (aGiveAchievement) {
		GiveAchievement(theApp, AchievementId::Morticulturalist, aGiveAchievement);
	}
}
