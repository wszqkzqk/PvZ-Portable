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
#include "GameButton.h"
#include "StoreScreen.h"
#include "../ZenGarden.h"
#include "GameSelector.h"
#include "../../LawnApp.h"
#include "AlmanacDialog.h"
#include "../../Resources.h"
#include "../System/Music.h"
#include "../ToolTipWidget.h"
#include "../System/SaveGame.h"
#include "../../GameConstants.h"
#include "../System/PlayerInfo.h"
#include "../System/ProfileMgr.h"
#include "../System/TypingCheck.h"
#include "../../Sexy.TodLib/TodFoley.h"
#include "../../Sexy.TodLib/TodDebug.h"
#include "graphics/Font.h"
#include "../../Sexy.TodLib/Reanimator.h"
#include "../../Sexy.TodLib/TodParticle.h"
#include "widget/Dialog.h"
#include "widget/WidgetManager.h"

static float gFlowerCenter[3][2] = { { 765.0f, 483.0f }, { 663.0f, 455.0f }, { 701.0f, 439.0f } };

void GameSelectorOverlay::Draw(Graphics* g)
{ 
	mParent->DrawOverlay(g);
}

GameSelectorOverlay::GameSelectorOverlay(GameSelector* theGameSelector)
{
	mParent = theGameSelector;
	mMouseVisible = false;
	mHasAlpha = true;
}

// GOTY @Patoke: 0x44B8D0
GameSelector::GameSelector(LawnApp* theApp)
{
	TodHesitationTrace("pregameselector");
	mLoadedResourceNames.push_back("DelayLoad_Zombatar");

	for (std::string& resource : mLoadedResourceNames)
		TodLoadResources(resource.c_str());

	mApp = theApp;
	mLevel = 1;
	mLoading = false;
	mHasTrophy = false;
	mToolTip = new ToolTipWidget();

	mAdventureButton = MakeNewButton(
		GameSelector::GameSelector_Adventure, 
		this, 
		"", 
		nullptr, 
		Sexy::IMAGE_REANIM_SELECTORSCREEN_ADVENTURE_BUTTON, 
		Sexy::IMAGE_REANIM_SELECTORSCREEN_ADVENTURE_HIGHLIGHT, 
		Sexy::IMAGE_REANIM_SELECTORSCREEN_ADVENTURE_HIGHLIGHT
	);
	
	mAdventureButton->Resize(0, 0, Sexy::IMAGE_REANIM_SELECTORSCREEN_ADVENTURE_BUTTON->mWidth, 125);
	mAdventureButton->mClip = false;
	mAdventureButton->mBtnNoDraw = true;
	mAdventureButton->mMouseVisible = false;
	mAdventureButton->mPolygonShape[0] = SexyVector2(7.0f, 1.0f);
	mAdventureButton->mPolygonShape[1] = SexyVector2(328.0f, 30.0f);
	mAdventureButton->mPolygonShape[2] = SexyVector2(314.0f, 125.0f);
	mAdventureButton->mPolygonShape[3] = SexyVector2(1.0f, 78.0f);
	mAdventureButton->mUsePolygonShape = true;

	mMinigameButton = MakeNewButton(
		GameSelector::GameSelector_Minigame, 
		this, 
		"", 
		nullptr, 
		Sexy::IMAGE_REANIM_SELECTORSCREEN_SURVIVAL_BUTTON, 
		Sexy::IMAGE_REANIM_SELECTORSCREEN_SURVIVAL_HIGHLIGHT, 
		Sexy::IMAGE_REANIM_SELECTORSCREEN_SURVIVAL_HIGHLIGHT
	);
	mMinigameButton->Resize(0, 0, Sexy::IMAGE_REANIM_SELECTORSCREEN_SURVIVAL_BUTTON->mWidth, 130);
	mMinigameButton->mClip = false;
	mMinigameButton->mBtnNoDraw = true;
	mMinigameButton->mMouseVisible = false;
	mMinigameButton->mPolygonShape[0] = SexyVector2(4.0f, 2.0f);
	mMinigameButton->mPolygonShape[1] = SexyVector2(312.0f, 51.0f);
	mMinigameButton->mPolygonShape[2] = SexyVector2(296.0f, 130.0f);
	mMinigameButton->mPolygonShape[3] = SexyVector2(7.0f, 77.0f);
	mMinigameButton->mUsePolygonShape = true;

	mPuzzleButton = MakeNewButton(
		GameSelector::GameSelector_Puzzle, 
		this, 
		"", 
		nullptr, 
		Sexy::IMAGE_REANIM_SELECTORSCREEN_CHALLENGES_BUTTON, 
		Sexy::IMAGE_REANIM_SELECTORSCREEN_CHALLENGES_HIGHLIGHT, 
		Sexy::IMAGE_REANIM_SELECTORSCREEN_CHALLENGES_HIGHLIGHT
	);
	mPuzzleButton->Resize(0, 0, Sexy::IMAGE_REANIM_SELECTORSCREEN_CHALLENGES_BUTTON->mWidth, 121);
	mPuzzleButton->mClip = false;
	mPuzzleButton->mBtnNoDraw = true;
	mPuzzleButton->mMouseVisible = false;
	mPuzzleButton->mPolygonShape[0] = SexyVector2(2.0f, 0.0f);
	mPuzzleButton->mPolygonShape[1] = SexyVector2(281.0f, 55.0f);
	mPuzzleButton->mPolygonShape[2] = SexyVector2(268.0f, 121.0f);
	mPuzzleButton->mPolygonShape[3] = SexyVector2(3.0f, 60.0f);
	mPuzzleButton->mUsePolygonShape = true;

	mSurvivalButton = MakeNewButton(
		GameSelector::GameSelector_Survival,
		this, 
		"", 
		nullptr, 
		Sexy::IMAGE_REANIM_SELECTORSCREEN_VASEBREAKER_BUTTON, 
		Sexy::IMAGE_REANIM_SELECTORSCREEN_VASEBREAKER_HIGHLIGHT, 
		Sexy::IMAGE_REANIM_SELECTORSCREEN_VASEBREAKER_HIGHLIGHT
	);
	mSurvivalButton->Resize(0, 0, Sexy::IMAGE_REANIM_SELECTORSCREEN_VASEBREAKER_BUTTON->mWidth, 124);
	mSurvivalButton->mClip = false;
	mSurvivalButton->mBtnNoDraw = true;
	mSurvivalButton->mMouseVisible = false;
	mSurvivalButton->mPolygonShape[0] = SexyVector2(7.0f, 1.0f);
	mSurvivalButton->mPolygonShape[1] = SexyVector2(267.0f, 62.0f);
	mSurvivalButton->mPolygonShape[2] = SexyVector2(257.0f, 124.0f);
	mSurvivalButton->mPolygonShape[3] = SexyVector2(7.0f, 57.0f);
	mSurvivalButton->mUsePolygonShape = true;

	// @Patoke: add these button defs
	mZombatarButton = MakeNewButton(
		GameSelector::GameSelector_Zombatar,
		this,
		"",
		nullptr,
		Sexy::IMAGE_BLANK,
		Sexy::IMAGE_BLANK,
		Sexy::IMAGE_BLANK
	);
	mZombatarButton->Resize(0, 0, Sexy::IMAGE_REANIM_SELECTORSCREEN_WOODSIGN3_PRESS->mWidth, Sexy::IMAGE_REANIM_SELECTORSCREEN_WOODSIGN3_PRESS->mHeight);
	mZombatarButton->mClip = false;
	mZombatarButton->mBtnNoDraw = true;
	mZombatarButton->mMouseVisible = false;

	mAchievementsButton = MakeNewButton(
		GameSelector::GameSelector_Achievements,
		this,
		"",
		nullptr,
		Sexy::IMAGE_SELECTORSCREEN_ACHIEVEMENTS_PEDESTAL,
		Sexy::IMAGE_SELECTORSCREEN_ACHIEVEMENTS_PEDESTAL_PRESS,
		Sexy::IMAGE_SELECTORSCREEN_ACHIEVEMENTS_PEDESTAL_PRESS
	);
	mAchievementsButton->Resize(20, mApp->mHeight - Sexy::IMAGE_SELECTORSCREEN_ACHIEVEMENTS_PEDESTAL->mHeight - 35, Sexy::IMAGE_SELECTORSCREEN_ACHIEVEMENTS_PEDESTAL->mWidth, Sexy::IMAGE_SELECTORSCREEN_ACHIEVEMENTS_PEDESTAL->mHeight);
	mAchievementsButton->mClip = false;
	mAchievementsButton->mBtnNoDraw = mHasTrophy;
	mAchievementsButton->mMouseVisible = false;

	mQuickPlayButton = MakeNewButton(
		GameSelector::GameSelector_QuickPlay,
		this,
		"",
		nullptr,
		Sexy::IMAGE_QUICKPLAY_BACK_BUTTON,
		Sexy::IMAGE_QUICKPLAY_BACK_BUTTON_HIGHLIGHT,
		Sexy::IMAGE_QUICKPLAY_BACK_BUTTON_HIGHLIGHT
	);
	mQuickPlayButton->Resize(mApp->mWidth - 150, 455, Sexy::IMAGE_QUICKPLAY_BACK_BUTTON->mWidth, Sexy::IMAGE_QUICKPLAY_BACK_BUTTON->mHeight);

	mZenGardenButton = MakeNewButton(
		GameSelector::GameSelector_ZenGarden, 
		this, 
		"", 
		nullptr, 
		Sexy::IMAGE_SELECTORSCREEN_ZENGARDEN, 
		Sexy::IMAGE_SELECTORSCREEN_ZENGARDENHIGHLIGHT, 
		Sexy::IMAGE_SELECTORSCREEN_ZENGARDENHIGHLIGHT
	);
	mZenGardenButton->Resize(0, 0, 130, 130);
	mZenGardenButton->mMouseVisible = false;
	mZenGardenButton->mClip = false;

	mOptionsButton = MakeNewButton(
		GameSelector::GameSelector_Options, 
		this, 
		"", 
		nullptr, 
		Sexy::IMAGE_SELECTORSCREEN_OPTIONS1, 
		Sexy::IMAGE_SELECTORSCREEN_OPTIONS2, 
		Sexy::IMAGE_SELECTORSCREEN_OPTIONS2
	);
	mOptionsButton->Resize(0, 0, Sexy::IMAGE_SELECTORSCREEN_OPTIONS1->mWidth, Sexy::IMAGE_SELECTORSCREEN_OPTIONS1->mHeight + 23);
	mOptionsButton->mClip = false; // @Patoke: not in original but fixes stuff
	mOptionsButton->mBtnNoDraw = true;
	mOptionsButton->mMouseVisible = false;
	mOptionsButton->mButtonOffsetY = 15;

	mHelpButton = MakeNewButton(
		GameSelector::GameSelector_Help, 
		this, 
		"", 
		nullptr, 
		Sexy::IMAGE_SELECTORSCREEN_HELP1, 
		Sexy::IMAGE_SELECTORSCREEN_HELP2, 
		Sexy::IMAGE_SELECTORSCREEN_HELP2
	);
	mHelpButton->Resize(0, 0, Sexy::IMAGE_SELECTORSCREEN_HELP1->mWidth, Sexy::IMAGE_SELECTORSCREEN_HELP1->mHeight + 33);
	mHelpButton->mClip = false; // @Patoke: not in original but fixes stuff
	mHelpButton->mBtnNoDraw = true;
	mHelpButton->mMouseVisible = false;
	mHelpButton->mButtonOffsetY = 30;

	mQuitButton = MakeNewButton(
		GameSelector::GameSelector_Quit, 
		this, 
		"", 
		nullptr, 
		Sexy::IMAGE_SELECTORSCREEN_QUIT1, 
		Sexy::IMAGE_SELECTORSCREEN_QUIT2, 
		Sexy::IMAGE_SELECTORSCREEN_QUIT2
	);
	mQuitButton->Resize(0, 0, Sexy::IMAGE_SELECTORSCREEN_QUIT1->mWidth + 10, Sexy::IMAGE_SELECTORSCREEN_QUIT1->mHeight + 10);
	mQuitButton->mClip = false; // @Patoke: not in original but fixes stuff
	mQuitButton->mBtnNoDraw = true;
	mQuitButton->mMouseVisible = false;
	mQuitButton->mButtonOffsetX = 5;
	mQuitButton->mButtonOffsetY = 5;

	mChangeUserButton = MakeNewButton(
		GameSelector::GameSelector_ChangeUser, 
		this, 
		"", 
		nullptr, 
		Sexy::IMAGE_BLANK, 
		Sexy::IMAGE_BLANK, 
		Sexy::IMAGE_BLANK
	);
	mChangeUserButton->Resize(0, 0, 250, 30);
	mChangeUserButton->mBtnNoDraw = true;
	mChangeUserButton->mMouseVisible = false;

	mOverlayWidget = new GameSelectorOverlay(this);
	mOverlayWidget->Resize(0, 0, BOARD_WIDTH, BOARD_HEIGHT);

	mStoreButton = MakeNewButton(
		GameSelector::GameSelector_Store, 
		this, 
		"", 
		nullptr, 
		Sexy::IMAGE_SELECTORSCREEN_STORE, 
		Sexy::IMAGE_SELECTORSCREEN_STOREHIGHLIGHT, 
		Sexy::IMAGE_SELECTORSCREEN_STOREHIGHLIGHT
	);
	mStoreButton->Resize(405, 484, Sexy::IMAGE_SELECTORSCREEN_STORE->mWidth, Sexy::IMAGE_SELECTORSCREEN_STORE->mHeight);
	mStoreButton->mClip = false; // @Patoke: not in original but fixes stuff
	mStoreButton->mMouseVisible = false;
	
	mAlmanacButton = MakeNewButton(
		GameSelector::GameSelector_Almanac, 
		this, 
		"", 
		nullptr, 
		Sexy::IMAGE_SELECTORSCREEN_ALMANAC, 
		Sexy::IMAGE_SELECTORSCREEN_ALMANACHIGHLIGHT, 
		Sexy::IMAGE_SELECTORSCREEN_ALMANACHIGHLIGHT
	);
	mAlmanacButton->Resize(327, 428, Sexy::IMAGE_SELECTORSCREEN_ALMANAC->mWidth, Sexy::IMAGE_SELECTORSCREEN_ALMANAC->mHeight);
	mAlmanacButton->mClip = false; // @Patoke: not in original but fixes stuff
	mAlmanacButton->mMouseVisible = false;

	mApp->mMusic->MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_TITLE_CRAZY_DAVE_MAIN_THEME);

	mStartingGame = false;
	mStartingGameCounter = 0;
	mMinigamesLocked = false;
	mPuzzleLocked = false;
	mSurvivalLocked = false;
	mUnlockSelectorCheat = false;
	mTrophyParticleID = ParticleSystemID::PARTICLESYSTEMID_NULL;
	mShowStartButton = false;

	Reanimation* aSelectorReanim = mApp->AddReanimation(0.5f, 0.5f, 0, ReanimationType::REANIM_SELECTOR_SCREEN);
	aSelectorReanim->PlayReanim("anim_open", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 30.0f);
	aSelectorReanim->AssignRenderGroupToPrefix("flower", RENDER_GROUP_HIDDEN);
	aSelectorReanim->AssignRenderGroupToPrefix("leaf", RENDER_GROUP_HIDDEN);
	aSelectorReanim->AssignRenderGroupToTrack("SelectorScreen_BG", 1);
	mSelectorReanimID = mApp->ReanimationGetID(aSelectorReanim);
	mSelectorState = SelectorAnimState::SELECTOR_OPEN;
	int aFrameStart, aFrameCount;
	aSelectorReanim->GetFramesForLayer("anim_sign", aFrameStart, aFrameCount);
	aSelectorReanim->mFrameBasePose = aFrameStart + aFrameCount - 1;

	for (int i = 0; i < 6; i++)
	{
		Reanimation* aCloudReanim = mApp->AddReanimation(0.5f, 0.5f, 0, ReanimationType::REANIM_SELECTOR_SCREEN);
		std::string aAnimName = Sexy::StrFormat("anim_cloud%d", (i > 1 ? i + 2 : i + 1));
		aCloudReanim->PlayReanim(aAnimName.c_str(), ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 0.0f);
		mCloudReanimID[i] = mApp->ReanimationGetID(aCloudReanim);
		mCloudCounter[i] = RandRangeInt(-6000, 2000);
		if (mCloudCounter[i] < 0)
		{
			aCloudReanim->mAnimTime = -mCloudCounter[i] / 6000.0f;
			aCloudReanim->mAnimRate = 0.5f;
			mCloudCounter[i] = 0;
		}
		else
			aCloudReanim->mAnimRate = 0.0f;
	}

	for (int i = 0; i < 3; i++)
	{
		Reanimation* aFlowerReanim = mApp->AddReanimation(0.5f, 0.5f, 0, ReanimationType::REANIM_SELECTOR_SCREEN);
		std::string aAnimName = Sexy::StrFormat("anim_flower%d", i + 1);
		aFlowerReanim->PlayReanim(aAnimName.c_str(), ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 0.0f);
		aFlowerReanim->mAnimRate = 0.0f;
		aFlowerReanim->AttachToAnotherReanimation(aSelectorReanim, "SelectorScreen_BG_Right");
		aFlowerReanim->mIsAttachment = false;
		mFlowerReanimID[i] = mApp->ReanimationGetID(aFlowerReanim);
	}

	Reanimation* aLeafReanim = mApp->AddReanimation(0.5f, 0.5f, 0, ReanimationType::REANIM_SELECTOR_SCREEN);
	aLeafReanim->PlayReanim("anim_grass", ReanimLoopType::REANIM_LOOP, 0, 6.0f);
	aLeafReanim->mAnimRate = 0.0f;
	mLeafReanimID = mApp->ReanimationGetID(aLeafReanim);
	mLeafCounter = 50;

	SyncProfile(false);
	mApp->PlaySample(Sexy::SOUND_ROLL_IN);

	// @Patoke: add new var init
	mSlideCounter = 0;
	mStartX = 0;
	mStartY = 0;
	mDestX = 0;
	mDestY = 0;
	//mZombatarWidget = new ZombatarWidget(this);
	//mZombatarWidget->Resize(800, 0, mApp->mWidth, mApp->mHeight);
	mAchievementsWidget = new AchievementsWidget(this->mApp);
	mAchievementsWidget->Move(0, mApp->mHeight);

	TodHesitationTrace("gameselectorinit");
}

GameSelector::~GameSelector()
{
	if (mAdventureButton)
		delete mAdventureButton;
	if (mMinigameButton)
		delete mMinigameButton;
	if (mPuzzleButton)
		delete mPuzzleButton;
	if (mOptionsButton)
		delete mOptionsButton;
	if (mQuitButton)
		delete mQuitButton;
	if (mHelpButton)
		delete mHelpButton;
	if (mOverlayWidget)
		delete mOverlayWidget;
	if (mStoreButton)
		delete mStoreButton;
	if (mAlmanacButton)
		delete mAlmanacButton;
	if (mZenGardenButton)
		delete mZenGardenButton;
	if (mSurvivalButton)
		delete mSurvivalButton;
	if (mChangeUserButton)
		delete mChangeUserButton;
	// @Patoke: new widgets
	if (mZombatarButton)
		delete mZombatarButton;
	//if (mZombatarWidget) // todo @Patoke: add zombatar
	//	delete mZombatarWidget;
	if (mAchievementsButton)
		delete mAchievementsButton;
	if (mAchievementsWidget)
		delete mAchievementsWidget;
	if (mQuickPlayButton)
		delete mQuickPlayButton;

	delete mToolTip;
}

// GOTY @Patoke: 0x44CDD0
void GameSelector::SyncButtons()
{
	bool aAlmanacAvailable = mApp->CanShowAlmanac() || mUnlockSelectorCheat;
	bool aStoreOpen = mApp->CanShowStore() || mUnlockSelectorCheat;
	bool aZenGardenOpen = mApp->CanShowZenGarden() || mUnlockSelectorCheat;

	mAlmanacButton->mDisabled = !aAlmanacAvailable;
	mAlmanacButton->mVisible = aAlmanacAvailable;
	mStoreButton->mDisabled = !aStoreOpen;
	mStoreButton->mVisible = aStoreOpen;
	mZombatarButton->mDisabled = false; // @Patoke: added these
	mZombatarButton->mVisible = true;

	Reanimation* aSelectorReanim = mApp->ReanimationGet(mSelectorReanimID);
	if (aAlmanacAvailable)
	{
		aSelectorReanim->AssignRenderGroupToPrefix("almanac_key_shadow", RENDER_GROUP_NORMAL);
		if (aStoreOpen)
			aSelectorReanim->SetImageOverride("almanac_key_shadow", nullptr);
		else
			aSelectorReanim->SetImageOverride("almanac_key_shadow", Sexy::IMAGE_REANIM_SELECTORSCREEN_ALMANAC_SHADOW);
	}
	else if (aStoreOpen)
	{
		aSelectorReanim->AssignRenderGroupToPrefix("almanac_key_shadow", RENDER_GROUP_NORMAL);
		aSelectorReanim->SetImageOverride("almanac_key_shadow", Sexy::IMAGE_REANIM_SELECTORSCREEN_KEY_SHADOW);
	}
	else
		aSelectorReanim->AssignRenderGroupToPrefix("almanac_key_shadow", RENDER_GROUP_HIDDEN);

	mZenGardenButton->mDisabled = !aZenGardenOpen;
	mZenGardenButton->mVisible = aZenGardenOpen;

	// @Patoke: all of these are already assigned in the constructor, why assign them here? (this fixes the hover highlight)
	if (mMinigamesLocked)
	{
		//mMinigameButton->mOverImage = Sexy::IMAGE_REANIM_SELECTORSCREEN_SURVIVAL_HIGHLIGHT;
		//mMinigameButton->mDownImage = Sexy::IMAGE_REANIM_SELECTORSCREEN_SURVIVAL_HIGHLIGHT;
		mMinigameButton->SetColor(ButtonWidget::COLOR_BKG, Color(128, 128, 128));
	}
	else
	{
		//mMinigameButton->mOverImage = Sexy::IMAGE_REANIM_SELECTORSCREEN_SURVIVAL_HIGHLIGHT;
		//mMinigameButton->mDownImage = Sexy::IMAGE_REANIM_SELECTORSCREEN_SURVIVAL_HIGHLIGHT;
		mMinigameButton->SetColor(ButtonWidget::COLOR_BKG, Color::White);
	}
	if (mPuzzleLocked)
	{
		//mPuzzleButton->mOverImage = Sexy::IMAGE_REANIM_SELECTORSCREEN_CHALLENGES_HIGHLIGHT;
		//mPuzzleButton->mDownImage = Sexy::IMAGE_REANIM_SELECTORSCREEN_CHALLENGES_HIGHLIGHT;
		mPuzzleButton->SetColor(ButtonWidget::COLOR_BKG, Color(128, 128, 128));
	}
	else
	{
		//mPuzzleButton->mOverImage = Sexy::IMAGE_REANIM_SELECTORSCREEN_CHALLENGES_HIGHLIGHT;
		//mPuzzleButton->mDownImage = Sexy::IMAGE_REANIM_SELECTORSCREEN_CHALLENGES_HIGHLIGHT;
		mPuzzleButton->SetColor(ButtonWidget::COLOR_BKG, Color::White);
	}
	if (mSurvivalLocked)
	{
		//mSurvivalButton->mOverImage = Sexy::IMAGE_REANIM_SELECTORSCREEN_VASEBREAKER_HIGHLIGHT;
		//mSurvivalButton->mDownImage = Sexy::IMAGE_REANIM_SELECTORSCREEN_VASEBREAKER_HIGHLIGHT;
		mSurvivalButton->SetColor(ButtonWidget::COLOR_BKG, Color(128, 128, 128));
	}
	else
	{
		//mSurvivalButton->mOverImage = Sexy::IMAGE_REANIM_SELECTORSCREEN_VASEBREAKER_HIGHLIGHT;
		//mSurvivalButton->mDownImage = Sexy::IMAGE_REANIM_SELECTORSCREEN_VASEBREAKER_HIGHLIGHT;
		mSurvivalButton->SetColor(ButtonWidget::COLOR_BKG, Color::White);
	}

	ReanimatorTrackInstance* aMinigameTrack = aSelectorReanim->GetTrackInstanceByName("SelectorScreen_Survival_button");
	ReanimatorTrackInstance* aPuzzleTrack = aSelectorReanim->GetTrackInstanceByName("SelectorScreen_Challenges_button");
	ReanimatorTrackInstance* aSurvivalTrack = aSelectorReanim->GetTrackInstanceByName("SelectorScreen_ZenGarden_button");
	aMinigameTrack->mTrackColor = mMinigameButton->GetColor(ButtonWidget::COLOR_BKG);
	aPuzzleTrack->mTrackColor = mPuzzleButton->GetColor(ButtonWidget::COLOR_BKG);
	aSurvivalTrack->mTrackColor = mSurvivalButton->GetColor(ButtonWidget::COLOR_BKG);

	if (mShowStartButton)
	{
		aSelectorReanim->AssignRenderGroupToPrefix("SelectorScreen_Adventure_", RENDER_GROUP_HIDDEN);

		mAdventureButton->mButtonImage = Sexy::IMAGE_REANIM_SELECTORSCREEN_STARTADVENTURE_BUTTON;
		mAdventureButton->mOverImage = Sexy::IMAGE_REANIM_SELECTORSCREEN_STARTADVENTURE_HIGHLIGHT;
		mAdventureButton->mDownImage = Sexy::IMAGE_REANIM_SELECTORSCREEN_STARTADVENTURE_HIGHLIGHT;
	}
	else
	{
		aSelectorReanim->AssignRenderGroupToPrefix("SelectorScreen_StartAdventure_", RENDER_GROUP_HIDDEN);

		mAdventureButton->mButtonImage = Sexy::IMAGE_REANIM_SELECTORSCREEN_ADVENTURE_BUTTON;
		mAdventureButton->mOverImage = Sexy::IMAGE_REANIM_SELECTORSCREEN_ADVENTURE_HIGHLIGHT;
		mAdventureButton->mDownImage = Sexy::IMAGE_REANIM_SELECTORSCREEN_ADVENTURE_HIGHLIGHT;
	}
}

// GOTY @Patoke: 0x44D230
void GameSelector::AddTrophySparkle()
{
	TOD_ASSERT(mTrophyParticleID == PARTICLESYSTEMID_NULL);
	TodParticleSystem* aTrophyParticle = mApp->AddTodParticle(85.0f, 330.0f, RenderLayer::RENDER_LAYER_TOP, ParticleEffect::PARTICLE_TROPHY_SPARKLE);
	mTrophyParticleID = mApp->ParticleGetID(aTrophyParticle);
}

// GOTY @Patoke: 0x44D270
void GameSelector::SyncProfile(bool theShowLoading)
{
	if (theShowLoading)
	{
		mLoading = true;

		mApp->DoDialog(Dialogs::DIALOG_MESSAGE, true, "Loading...", "", "", Dialog::BUTTONS_NONE);
		mApp->DrawDirtyStuff();
		mApp->PreloadForUser();
		mApp->KillDialog(Dialogs::DIALOG_MESSAGE);

		mLoading = false;
	}

	TodParticleSystem* aTrophyParticle = mApp->ParticleTryToGet(mTrophyParticleID);
	if (aTrophyParticle)
	{
		aTrophyParticle->ParticleSystemDie();
		mTrophyParticleID = ParticleSystemID::PARTICLESYSTEMID_NULL;
	}

	mLevel = 1;
	if (mApp->mPlayerInfo)
		mLevel = mApp->mPlayerInfo->GetLevel();
	mShowStartButton = true;
	mMinigamesLocked = true;
	mPuzzleLocked = true;
	mSurvivalLocked = true;
	if (mApp->mPlayerInfo && !mApp->IsIceDemo())
	{
		if (mLevel >= 2)
			mShowStartButton = false;

		if (mApp->HasFinishedAdventure())
		{
			mMinigamesLocked = false;
			mSurvivalLocked = false;
			mPuzzleLocked = false;
			mShowStartButton = false;
		}

		if (mApp->mPlayerInfo->mHasUnlockedMinigames)
			mMinigamesLocked = false;
		if (mApp->mPlayerInfo->mHasUnlockedPuzzleMode)
			mPuzzleLocked = false;
		if (mApp->mPlayerInfo->mHasUnlockedSurvivalMode)
			mSurvivalLocked = false;

		if (mApp->IsTrialStageLocked())
		{
			mPuzzleLocked = true;
			mSurvivalLocked = true;
		}
	}

	if (mApp->HasFinishedAdventure() && !mApp->IsTrialStageLocked())
		mHasTrophy = true;
	else
		mHasTrophy = false;
	if (mHasTrophy && mSelectorState != SelectorAnimState::SELECTOR_OPEN)
		AddTrophySparkle();

	SyncButtons();
	AlmanacInitForPlayer();
	BoardInitForPlayer();
	ReportAchievement::AchievementInitForPlayer(mApp); // @Patoke: add call
}

// GOTY @Patoke: seems to be inlined? 0x44DCC6
void GameSelector::Draw(Graphics* g)
{
	if (mApp->GetDialog(Dialogs::DIALOG_STORE) || mApp->GetDialog(Dialogs::DIALOG_ALMANAC))
		return;

	g->SetLinearBlend(true);
	Reanimation* aSelectorReanim = mApp->ReanimationGet(mSelectorReanimID);
	aSelectorReanim->DrawRenderGroup(g, 1);  // "SelectorScreen_BG"
	for (int i = 0; i < 6; i++)
		mApp->ReanimationGet(mCloudReanimID[i])->Draw(g);
	aSelectorReanim->DrawRenderGroup(g, RENDER_GROUP_NORMAL);

	if (mSelectorState == SelectorAnimState::SELECTOR_OPEN)
	{
		int aBGIdx = aSelectorReanim->FindTrackIndex("SelectorScreen_BG_Right");
		ReanimatorTransform aTransform;
		aSelectorReanim->GetCurrentTransform(aBGIdx, &aTransform);
		float aFractionalOffsetX = fmod(aTransform.mTransX, 1.0f);
		float aFractionalOffsetY = fmod(aTransform.mTransY, 1.0f);
		g->DrawImageF(
			mOptionsButton->mButtonImage,
			mOptionsButton->mX + mOptionsButton->mButtonOffsetX + aFractionalOffsetX,
			mOptionsButton->mY + mOptionsButton->mButtonOffsetY + aFractionalOffsetY
		);
		g->DrawImageF(
			mQuitButton->mButtonImage,
			mQuitButton->mX + mQuitButton->mButtonOffsetX + aFractionalOffsetX,
			mQuitButton->mY + mQuitButton->mButtonOffsetY + aFractionalOffsetY
		);
		g->DrawImageF(
			mHelpButton->mButtonImage, 
			mHelpButton->mX + mHelpButton->mButtonOffsetX + aFractionalOffsetX, 
			mHelpButton->mY + mHelpButton->mButtonOffsetY + aFractionalOffsetY
		);
	}

	if (mApp->mPlayerInfo && mApp->mPlayerInfo->mName.size() &&
		mSelectorState != SelectorAnimState::SELECTOR_OPEN && mSelectorState != SelectorAnimState::SELECTOR_NEW_USER)
	{
		std::string aWelcomeStr = mApp->mPlayerInfo->mName + '!';

		int aSignIdx = aSelectorReanim->FindTrackIndex("woodsign1");
		SexyTransform2D aOverlayMatrix;
		aSelectorReanim->GetAttachmentOverlayMatrix(aSignIdx, aOverlayMatrix);
		float aStringWidth = Sexy::FONT_BRIANNETOD16->StringWidth(aWelcomeStr);
		SexyTransform2D aOffsetMatrix;
		// @Patoke: add position so it moves when sliding to position
		aOffsetMatrix.Translate(170.5f - static_cast<int>(aStringWidth * 0.5f) + mX, 102.5f + mY);
		TodDrawStringMatrix(g, Sexy::FONT_BRIANNETOD16, aOverlayMatrix * aOffsetMatrix, aWelcomeStr, Color(255, 245, 200));

	}
}

// GOTY @Patoke: 0x44D750
void GameSelector::DrawOverlay(Graphics* g)
{
	g->SetLinearBlend(true);
	if (mApp->mPlayerInfo == nullptr)
		return;

	if (!mApp->IsIceDemo() && !mShowStartButton)
	{
		int aOffsetX, aOffsetY;
		if (mAdventureButton->mIsDown && mAdventureButton->mIsOver)
		{
			aOffsetX = 1;
			aOffsetY = 1;
		}
		else
		{
			aOffsetX = 0;
			aOffsetY = 0;
		}

		Reanimation* aSelectorReanim = mApp->ReanimationGet(mSelectorReanimID);
		int aRightIdx = aSelectorReanim->FindTrackIndex("SelectorScreen_BG_Right");
		ReanimatorTransform aTransform;
		aSelectorReanim->GetCurrentTransform(aRightIdx, &aTransform);
		float aTransAreaX = aTransform.mTransX + aOffsetX;
		float aTransAreaY = aTransform.mTransY + aOffsetY;
		float aTransSubX = aTransAreaX;
		float aTransSubY = aTransAreaY;

		int aStage = ClampInt((mLevel - 1) / 10 + 1, 1, 6);  // 大关
		int aSub = mLevel - (aStage - 1) * 10;  // 小关
		if (mApp->IsTrialStageLocked() && (mLevel >= 25 || mApp->HasFinishedAdventure()))
		{
			aStage = 3;
			aSub = 4;
		}
		else
		{
			if (aStage == 1)
			{
				aTransAreaY += 1.0f;
			}
			else if (aStage == 4)
			{
				aTransAreaX -= 1.0f;
			}
			if (aSub == 3)
			{
				aTransSubX -= 1.0f;
			}
		}

		g->SetColorizeImages(true);
		g->SetColor(mAdventureButton->mColors[ButtonWidget::COLOR_BKG]);
		// @Patoke: changed positions for GOTY adventure icon
		TodDrawImageCelF(g, Sexy::IMAGE_SELECTORSCREEN_LEVELNUMBERS, aTransAreaX + 486.0f, aTransAreaY + 47.f, aStage, 0);  // 绘制大关数
		if (aSub < 10)
		{
			TodDrawImageCelF(g, Sexy::IMAGE_SELECTORSCREEN_LEVELNUMBERS, aTransSubX + 509.f, aTransSubY + 50.f, aSub, 0);
		}
		else if (aSub == 10)
		{
			TodDrawImageCelF(g, Sexy::IMAGE_SELECTORSCREEN_LEVELNUMBERS, aTransSubX + 509.f, aTransSubY + 50.f, 1, 0);
			TodDrawImageCelF(g, Sexy::IMAGE_SELECTORSCREEN_LEVELNUMBERS, aTransSubX + 518.f, aTransSubY + 51.f, 0, 0);
		}
		g->SetColorizeImages(false);
	}

	if (mZenGardenButton->mVisible && mApp->mZenGarden->PlantsNeedWater())
	{
		g->DrawImage(Sexy::IMAGE_PLANTSPEECHBUBBLE, mZenGardenButton->mX + 106, mZenGardenButton->mY + 36);
		g->DrawImage(Sexy::IMAGE_WATERDROP, mZenGardenButton->mX + 123, mZenGardenButton->mY + 45);
	}

	Reanimation* aHandReanim = mApp->ReanimationTryToGet(mHandReanimID);
	if (aHandReanim)
	{
		g->SetClipRect(0, 0, BOARD_WIDTH, BOARD_HEIGHT - 40);
		aHandReanim->Draw(g);
		g->ClearClipRect();
	}
	mApp->ReanimationGet(mLeafReanimID)->Draw(g);
	for (int i = 0; i < 3; i++)
		mApp->ReanimationGet(mFlowerReanimID[i])->Draw(g);

	// @Minerscale: Trophy needs to draw in the DrawOverlay
	Reanimation* aSelectorReanim = mApp->ReanimationGet(mSelectorReanimID);

	int aLeftIdx = aSelectorReanim->FindTrackIndex("SelectorScreen_BG_Left");
	ReanimatorTransform aTransformLeft;
	aSelectorReanim->GetCurrentTransform(aLeftIdx, &aTransformLeft);
	if (mHasTrophy)
	{
		// @Patoke: updated pos to match GOTY
		if (mApp->EarnedGoldTrophy())
			TodDrawImageCelF(g, Sexy::IMAGE_SUNFLOWER_TROPHY, aTransformLeft.mTransX + 12.f, aTransformLeft.mTransY + 345.f, 1, 0);
		else
			TodDrawImageCelF(g, Sexy::IMAGE_SUNFLOWER_TROPHY, aTransformLeft.mTransX + 12.f, aTransformLeft.mTransY + 345.f, 0, 0);
		
		TodParticleSystem* aTrophyParticle = mApp->ParticleTryToGet(mTrophyParticleID);
		if (aTrophyParticle)
			aTrophyParticle->Draw(g);
	}

	mToolTip->Draw(g);
}

// GOTY @Patoke: 0x44DE6D
void GameSelector::UpdateTooltip()
{
	if (!mApp->HasFinishedAdventure() || mApp->GetDialog(Dialogs::DIALOG_MESSAGE))
		return;

	if (mHasTrophy)
	{
		int aMouseX = mApp->mWidgetManager->mLastMouseX;
		int aMouseY = mApp->mWidgetManager->mLastMouseY;
		if (aMouseX >= 50 && aMouseX < 135 && aMouseY >= 275 && aMouseY < 500)
		{
			if (mApp->EarnedGoldTrophy())
			{
				mToolTip->SetLabel(LawnApp::Pluralize(mApp->mPlayerInfo->mFinishedAdventure, "[GOLD_SUNFLOWER_TOOLTIP]", "[GOLD_SUNFLOWER_TOOLTIP_PLURAL]"));
				mToolTip->mX = 32;
				mToolTip->mY = 510;
				mToolTip->mVisible = true;
			}
			else
			{
				mToolTip->SetLabel("[SILVER_SUNFLOWER_TOOLTIP]");
				mToolTip->mX = 20;
				mToolTip->mY = 495;
				mToolTip->mVisible = true;
			}

			return;
		}
	}

	mToolTip->mVisible = false;
	mToolTip->Update();
}

// GOTY @Patoke: 0x44E030
void GameSelector::Update()
{
	Widget::Update();
	MarkDirty();
	UpdateTooltip();

	// @Patoke: implemented this
	if (mSlideCounter > 0) {
		int aNewX = TodAnimateCurve(75, 0, mSlideCounter, mStartX, mDestX, TodCurves::CURVE_EASE_IN_OUT);
		int aNewY = TodAnimateCurve(75, 0, mSlideCounter, mStartY, mDestY, TodCurves::CURVE_EASE_IN_OUT);
		Move(aNewX, aNewY);

		// @Patoke: not from the original binaries but fixes bugs
		mOverlayWidget->Move(aNewX, aNewY);
		mAchievementsWidget->mY = aNewY + mApp->mHeight;
		mAdventureButton->SetOffset(aNewX, aNewY);
		mMinigameButton->SetOffset(aNewX, aNewY);
		mPuzzleButton->SetOffset(aNewX, aNewY);
		mOptionsButton->SetOffset(aNewX, aNewY + 15);
		mQuitButton->SetOffset(aNewX, aNewY + 5);
		mHelpButton->SetOffset(aNewX, aNewY + 30);
		mStoreButton->SetOffset(aNewX, aNewY);
		mAlmanacButton->SetOffset(aNewX, aNewY);
		mZenGardenButton->SetOffset(aNewX, aNewY);
		mSurvivalButton->SetOffset(aNewX, aNewY);
		mChangeUserButton->SetOffset(aNewX, aNewY);
		mZombatarButton->SetOffset(aNewX, aNewY);
		mAchievementsButton->SetOffset(aNewX, aNewY);
		mQuickPlayButton->SetOffset(aNewX, aNewY);

		// Make sure these are drawn even outside of bounds (force redraw)
		mAchievementsButton->MarkDirty();
		mOptionsButton->MarkDirty();
		mHelpButton->MarkDirty();
		mQuitButton->MarkDirty();
		mStoreButton->MarkDirty();
		mZenGardenButton->MarkDirty();

		mSlideCounter--;
	}

	mApp->mZenGarden->UpdatePlantNeeds();

	TodParticleSystem* aParticle = mApp->ParticleTryToGet(mTrophyParticleID);
	if (aParticle)
		aParticle->Update();

	if (mStartingGame)
	{
		mStartingGameCounter++;
		if (mStartingGameCounter > 450)
		{
			mApp->KillGameSelector();

			if (mApp->IsIceDemo())
			{
				mApp->PreNewGame(GameMode::GAMEMODE_CHALLENGE_ICE, false);
				return;
			}
			if (mApp->IsFirstTimeAdventureMode() && mLevel == 1 && !mApp->SaveFileExists())
			{
				mApp->PreNewGame(GameMode::GAMEMODE_INTRO, false);
				return;
			}
			if (mApp->mPlayerInfo->mNeedsMagicTacoReward && mLevel == 35)
			{
				StoreScreen* aStore = mApp->ShowStoreScreen();
				aStore->SetupForIntro(601);
				aStore->WaitForResult(true);
				mApp->PreNewGame(GameMode::GAMEMODE_ADVENTURE, false);
				return;
			}
			if (ShouldDoZenTuturialBeforeAdventure())
			{
				mApp->PreNewGame(GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN, false);
				mApp->mZenGarden->SetupForZenTutorial();
				return;
			}
			mApp->PreNewGame(GameMode::GAMEMODE_ADVENTURE, true);
			return;
		}

		mAdventureButton->SetColor(ButtonWidget::COLOR_BKG, mStartingGameCounter % 20 < 10 ? Color(80, 80, 80) : Color::White);
		if (mStartingGameCounter == 125)
			mApp->PlaySample(Sexy::SOUND_EVILLAUGH);
	}

	Reanimation* aSelectorReanim = mApp->ReanimationGet(mSelectorReanimID);
	switch (mSelectorState)
	{
	case SelectorAnimState::SELECTOR_OPEN:
		if (mWidgetManager)
			mWidgetManager->RehupMouse();
		if (aSelectorReanim->mLoopCount > 0)
		{
			aSelectorReanim->AssignRenderGroupToTrack("SelectorScreen_Adventure_button", RENDER_GROUP_HIDDEN);
			aSelectorReanim->AssignRenderGroupToTrack("SelectorScreen_StartAdventure_button", RENDER_GROUP_HIDDEN);
			aSelectorReanim->AssignRenderGroupToTrack("SelectorScreen_Survival_button", RENDER_GROUP_HIDDEN);
			aSelectorReanim->AssignRenderGroupToTrack("SelectorScreen_Challenges_button", RENDER_GROUP_HIDDEN);
			aSelectorReanim->AssignRenderGroupToTrack("SelectorScreen_ZenGarden_button", RENDER_GROUP_HIDDEN);
			mAdventureButton->mBtnNoDraw = false;
			mMinigameButton->mBtnNoDraw = false;
			mPuzzleButton->mBtnNoDraw = false;
			mSurvivalButton->mBtnNoDraw = false;
			mZenGardenButton->mBtnNoDraw = false;
			mHelpButton->mBtnNoDraw = false;
			mOptionsButton->mBtnNoDraw = false;
			mQuitButton->mBtnNoDraw = false;
			mZombatarButton->mBtnNoDraw = false; // @Patoke: new widgets
			mAchievementsButton->mBtnNoDraw = false;
			mAdventureButton->mMouseVisible = true;
			mMinigameButton->mMouseVisible = true;
			mPuzzleButton->mMouseVisible = true;
			mSurvivalButton->mMouseVisible = true;
			mZenGardenButton->mMouseVisible = true;
			mHelpButton->mMouseVisible = true;
			mOptionsButton->mMouseVisible = true;
			mQuitButton->mMouseVisible = true;
			mStoreButton->mMouseVisible = true;
			mAlmanacButton->mMouseVisible = true;
			mChangeUserButton->mMouseVisible = true;
			mZombatarButton->mMouseVisible = true; // @Patoke: new widgets
			mAchievementsButton->mMouseVisible = true;

			if (mApp->mPlayerInfo == nullptr)
			{
				mApp->DoCreateUserDialog();
				if (gIsPartnerBuild)
					AddPreviewProfiles();

				mSelectorState = SelectorAnimState::SELECTOR_NEW_USER;
			}
			else
			{
				aSelectorReanim->PlayReanim("anim_sign", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 30.0f);
				mSelectorState = SelectorAnimState::SELECTOR_IDLE;
			}

			if (mHasTrophy)
				AddTrophySparkle();

			if (mApp->mPlayerInfo && mApp->mPlayerInfo->mNeedsMessageOnGameSelector)
			{
				mApp->mPlayerInfo->mNeedsMessageOnGameSelector = 0;
				mApp->WriteCurrentUserConfig();
				mApp->LawnMessageBox(
					Dialogs::DIALOG_MESSAGE, 
					"[ADVENTURE_COMPLETE_HEADER]", 
					"[ADVENTURE_COMPLETE_BODY]", 
					"[DIALOG_BUTTON_OK]", 
					"", 
					Dialog::BUTTONS_FOOTER
				);
			}
		}
		break;
	case SelectorAnimState::SELECTOR_NEW_USER:
		if (mApp->GetDialog(Dialogs::DIALOG_CREATEUSER) == nullptr)
		{
			aSelectorReanim->PlayReanim("anim_sign", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 30.0f);
			mSelectorState = SelectorAnimState::SELECTOR_SHOW_SIGN;
		}
		break;
	case SelectorAnimState::SELECTOR_SHOW_SIGN:
		if (aSelectorReanim->mLoopCount > 0)
			mSelectorState = SelectorAnimState::SELECTOR_IDLE;
		break;
	case SelectorAnimState::SELECTOR_IDLE:
		break;
	}

	for (int i = 0; i < 6; i++)
	{
		Reanimation* aCloudReanim = mApp->ReanimationGet(mCloudReanimID[i]);
		if (mCloudCounter[i] > 0)
		{
			if (--mCloudCounter[i] == 0)
			{
				aCloudReanim->mLoopCount = 0;
				aCloudReanim->mAnimTime = 0.0f;
				aCloudReanim->mAnimRate = 0.5f;
			}
		}
		else if (aCloudReanim->mLoopCount > 0)
			mCloudCounter[i] = RandRangeInt(2000, 4000);
	}
	aSelectorReanim->Update();

	Reanimation* aLeafReanim = mApp->ReanimationGet(mLeafReanimID);
	aLeafReanim->Update();
	int aLeafTrackIndex = aSelectorReanim->FindTrackIndex("SelectorScreen_BG_Right");
	ReanimatorTransform aLeafTransform;
	aSelectorReanim->GetCurrentTransform(aLeafTrackIndex, &aLeafTransform);
	aLeafReanim->SetPosition(aLeafTransform.mTransX - 71.0f, aLeafTransform.mTransY - 41.0f);
	if (--mLeafCounter == 0)
	{
		float aRate = RandRangeFloat(3.0f, 12.0f);
		aLeafReanim->PlayReanim("anim_grass", ReanimLoopType::REANIM_LOOP, 20, aRate);
		mLeafCounter = RandRangeInt(200, 400);
	}

	for (int i = 0; i < 6; i++)
		mApp->ReanimationGet(mCloudReanimID[i])->Update();
	Reanimation* aHandReanim = mApp->ReanimationTryToGet(mHandReanimID);
	if (aHandReanim)
		aHandReanim->Update();

	TrackButton(mAdventureButton, mShowStartButton ? "SelectorScreen_StartAdventure_button" : "SelectorScreen_Adventure_button", 0.0f, 0.0f);
	TrackButton(mMinigameButton, "SelectorScreen_Survival_button", 0.0f, 0.0f);
	TrackButton(mPuzzleButton, "SelectorScreen_Challenges_button", 0.0f, 0.0f);
	TrackButton(mSurvivalButton, "SelectorScreen_ZenGarden_button", 0.0f, 0.0f);
	TrackButton(mZenGardenButton, "SelectorScreen_BG_Right", 100.0f, 360.0f);
	TrackButton(mOptionsButton, "SelectorScreen_BG_Right", 494.0f, 434.0f);
	TrackButton(mQuitButton, "SelectorScreen_BG_Right", 644.0f, 469.0f);
	TrackButton(mHelpButton, "SelectorScreen_BG_Right", 576.0f, 458.0f);
	TrackButton(mAlmanacButton, "SelectorScreen_BG_Right", 256.0f, 387.0f);
	TrackButton(mStoreButton, "SelectorScreen_BG_Right", 334.0f, 441.0f);
	TrackButton(mChangeUserButton, "woodsign2", 24.0f, 10.0f);
	TrackButton(mZombatarButton, "woodsign3", 0.f, 0.f); // @Patoke: add shart here
	TrackButton(mAchievementsButton, "SelectorScreen_BG_Left", 20.f, 480.f);
	aSelectorReanim->SetImageOverride("woodsign2", (mChangeUserButton->mIsOver || mChangeUserButton->mIsDown) ? Sexy::IMAGE_REANIM_SELECTORSCREEN_WOODSIGN2_PRESS : nullptr);
	aSelectorReanim->SetImageOverride("woodsign3", (mZombatarButton->mIsOver || mZombatarButton->mIsDown) ? Sexy::IMAGE_REANIM_SELECTORSCREEN_WOODSIGN3_PRESS : nullptr);
}

// GOTY @Patoke: 0x44EA40
void GameSelector::TrackButton(DialogButton* theButton, const char* theTrackName, float theOffsetX, float theOffsetY)
{
	Reanimation* aSelectorReanim = mApp->ReanimationGet(mSelectorReanimID);
	int aTrackIndex = aSelectorReanim->FindTrackIndex(theTrackName);
	ReanimatorTransform aTransform;
	aSelectorReanim->GetCurrentTransform(aTrackIndex, &aTransform);
	
	theButton->mX = static_cast<int>(aTransform.mTransX + theOffsetX);
	theButton->mY = static_cast<int>(aTransform.mTransY + theOffsetY);
}

void GameSelector::AddedToManager(WidgetManager* theWidgetManager)
{
	Widget::AddedToManager(theWidgetManager);

	theWidgetManager->AddWidget(mAdventureButton);
	theWidgetManager->AddWidget(mMinigameButton);
	theWidgetManager->AddWidget(mPuzzleButton);
	theWidgetManager->AddWidget(mOptionsButton);
	theWidgetManager->AddWidget(mQuitButton);
	theWidgetManager->AddWidget(mHelpButton);
	theWidgetManager->AddWidget(mStoreButton);
	theWidgetManager->AddWidget(mAlmanacButton);
	theWidgetManager->AddWidget(mSurvivalButton);
	theWidgetManager->AddWidget(mZenGardenButton);
	theWidgetManager->AddWidget(mChangeUserButton);
	theWidgetManager->AddWidget(mOverlayWidget);
	theWidgetManager->AddWidget(mZombatarButton); // @Patoke: add new widgets
	//theWidgetManager->AddWidget(mZombatarWidget);
	theWidgetManager->AddWidget(mAchievementsButton);
	theWidgetManager->AddWidget(mAchievementsWidget);
	//theWidgetManager->AddWidget(mQuickPlayButton);
}

void GameSelector::RemovedFromManager(WidgetManager* theWidgetManager)
{
	Widget::RemovedFromManager(theWidgetManager);

	theWidgetManager->RemoveWidget(mAdventureButton);
	theWidgetManager->RemoveWidget(mMinigameButton);
	theWidgetManager->RemoveWidget(mPuzzleButton);
	theWidgetManager->RemoveWidget(mOptionsButton);
	theWidgetManager->RemoveWidget(mQuitButton);
	theWidgetManager->RemoveWidget(mHelpButton);
	theWidgetManager->RemoveWidget(mStoreButton);
	theWidgetManager->RemoveWidget(mAlmanacButton);
	theWidgetManager->RemoveWidget(mSurvivalButton);
	theWidgetManager->RemoveWidget(mZenGardenButton);
	theWidgetManager->RemoveWidget(mChangeUserButton);
	theWidgetManager->RemoveWidget(mOverlayWidget);
	theWidgetManager->RemoveWidget(mZombatarButton); // @Patoke: new widgets
	//theWidgetManager->RemoveWidget(mZombatarWidget);
	theWidgetManager->RemoveWidget(mAchievementsButton);
	theWidgetManager->RemoveWidget(mAchievementsWidget);
	//theWidgetManager->RemoveWidget(mQuickPlayButton);
}

void GameSelector::OrderInManagerChanged()
{
	mWidgetManager->PutInfront(mAchievementsWidget, this);
	mWidgetManager->PutInfront(mOverlayWidget, this);
	mWidgetManager->PutInfront(mAlmanacButton, this);
	mWidgetManager->PutInfront(mStoreButton, this);
	mWidgetManager->PutInfront(mHelpButton, this);
	mWidgetManager->PutInfront(mQuitButton, this);
	mWidgetManager->PutInfront(mOptionsButton, this);
	mWidgetManager->PutInfront(mAdventureButton, this);
	mWidgetManager->PutInfront(mMinigameButton, this);
	mWidgetManager->PutInfront(mPuzzleButton, this);
	mWidgetManager->PutInfront(mZenGardenButton, this);
	mWidgetManager->PutInfront(mSurvivalButton, this);
	mWidgetManager->PutInfront(mChangeUserButton, this);
	mWidgetManager->PutInfront(mZombatarButton, this); // @Patoke: z order for new widgets
	mWidgetManager->PutInfront(mAchievementsButton, this);
	//mWidgetManager->PutInfront(mQuickPlayButton, this);
}

// GOTY @Patoke: 0x44EB11
void GameSelector::KeyDown(KeyCode theKey)
{
	if (mApp->mKonamiCheck->Check(theKey))
	{
		mApp->PlayFoley(FoleyType::FOLEY_DROP);
		return;
	}
	if (mApp->mMustacheCheck->Check(theKey) || mApp->mMoustacheCheck->Check(theKey))
	{
		mApp->PlayFoley(FoleyType::FOLEY_POLEVAULT);
		mApp->mMustacheMode = !mApp->mMustacheMode;
		ReportAchievement::GiveAchievement(mApp, MustacheMode, false);
		return;
	}
	if (mApp->mSuperMowerCheck->Check(theKey) || mApp->mSuperMowerCheck2->Check(theKey))
	{
		mApp->PlayFoley(FoleyType::FOLEY_ZAMBONI);
		mApp->mSuperMowerMode = !mApp->mSuperMowerMode;
		return;
	}
	if (mApp->mFutureCheck->Check(theKey))
	{
		mApp->PlaySample(Sexy::SOUND_BOING);
		mApp->mFutureMode = !mApp->mFutureMode;
		return;
	}
	if (mApp->mPinataCheck->Check(theKey))
	{
		if (mApp->CanDoPinataMode())
		{
			mApp->PlayFoley(FoleyType::FOLEY_JUICY);
			mApp->mPinataMode = !mApp->mPinataMode;
			return;
		}
		else
		{
			mApp->PlaySample(Sexy::SOUND_BUZZER);
			return;
		}
	}
	if (mApp->mDanceCheck->Check(theKey))
	{
		if (mApp->CanDoDanceMode())
		{
			mApp->PlayFoley(FoleyType::FOLEY_DANCER);
			mApp->mDanceMode = !mApp->mDanceMode;
			return;
		}
		else
		{
			mApp->PlaySample(Sexy::SOUND_BUZZER);
			return;
		}
	}
	if (mApp->mDaisyCheck->Check(theKey))
	{
		if (mApp->CanDoDaisyMode())
		{
			mApp->PlaySample(Sexy::SOUND_LOADINGBAR_FLOWER);
			mApp->mDaisyMode = !mApp->mDaisyMode;
			return;
		}
		else
		{
			mApp->PlaySample(Sexy::SOUND_BUZZER);
			return;
		}
	}
	if (mApp->mSukhbirCheck->Check(theKey))
	{
		mApp->PlaySample(Sexy::SOUND_SUKHBIR);
		mApp->mSukhbirMode = !mApp->mSukhbirMode;
		return;
	}
}

// GOTY @Patoke: 0x44EEE0
void GameSelector::KeyChar(char theChar)
{
	if (mStartingGame)
		return;

	if ((gIsPartnerBuild || mApp->mDebugKeysEnabled) && theChar == 'u' && mApp->mPlayerInfo)
	{
		TodTraceAndLog("Selector cheat key '%c'", theChar);

		mApp->mPlayerInfo->mFinishedAdventure = 2;
		mApp->mPlayerInfo->AddCoins(50000);
		mApp->mPlayerInfo->mHasUsedCheatKeys = true;
		mApp->mPlayerInfo->mHasUnlockedMinigames = true;
		mApp->mPlayerInfo->mHasUnlockedPuzzleMode = true;
		mApp->mPlayerInfo->mHasUnlockedSurvivalMode = true;

		for (int i = 1; i < 100; i++)
			if (i != static_cast<int>(GameMode::GAMEMODE_TREE_OF_WISDOM) && i != static_cast<int>(GameMode::GAMEMODE_SCARY_POTTER_ENDLESS) &&
				i != static_cast<int>(GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_ENDLESS) && i != static_cast<int>(GameMode::GAMEMODE_SURVIVAL_ENDLESS_STAGE_3))
				mApp->mPlayerInfo->mChallengeRecords[i - 1] = 20;
		SyncProfile(true);

		mApp->EraseFile(GetSavedGameName(GameMode::GAMEMODE_ADVENTURE, mApp->mPlayerInfo->mId));
		mApp->EraseFile(GetLegacySavedGameName(GameMode::GAMEMODE_ADVENTURE, mApp->mPlayerInfo->mId));
	}

	if (mApp->mDebugKeysEnabled)
	{
		TodTraceAndLog("Selector cheat key '%c'", theChar);
		if (theChar == 'c' || theChar == 'C')
		{
			mMinigamesLocked = false;
			mPuzzleLocked = false;
			mSurvivalLocked = false;
			SyncButtons();
		}
	}
}

// GOTY @Patoke: 0x44F040
void GameSelector::MouseDown(int x, int y, int theClickCount)
{
	(void)theClickCount;
	for (int i = 0; i < 3; i++)
	{
		Reanimation* aFlowerReanim = mApp->ReanimationGet(mFlowerReanimID[i]);
		if (aFlowerReanim->mAnimRate <= 0.0f && Distance2D(x, y, gFlowerCenter[i][0], gFlowerCenter[i][1]) < 20.0f)
		{
			aFlowerReanim->mAnimRate = 24.0f;
			mApp->PlayFoley(FoleyType::FOLEY_LIMBS_POP);
		}
	}

	if (mApp->mTodCheatKeys && mStartingGame && mStartingGameCounter < 450)
		mStartingGameCounter = 450;
}

// GOTY @Patoke: 0x44F1A0
void GameSelector::ButtonMouseEnter(int theId)
{
	if ((theId == GameSelector::GameSelector_Minigame && mMinigamesLocked) ||
		(theId == GameSelector::GameSelector_Puzzle && mPuzzleLocked) ||
		(theId == GameSelector::GameSelector_Survival && mSurvivalLocked))
		return;

	mApp->PlayFoley(FoleyType::FOLEY_BLEEP);
}

// GOTY @Patoke: 0x44F220
void GameSelector::ButtonPress(int theId)
{
	if (theId == GameSelector::GameSelector_Adventure || theId == GameSelector::GameSelector_Minigame ||
		theId == GameSelector::GameSelector_Puzzle || theId == GameSelector::GameSelector_Survival ||
		theId == GameSelector::GameSelector_Zombatar) // @Patoke: add case
		mApp->PlaySample(Sexy::SOUND_GRAVEBUTTON);
	else
		mApp->PlaySample(Sexy::SOUND_TAP);
}

// GOTY @Patoke: 0x44F270
void GameSelector::ClickedAdventure()
{
	if (mApp->IsTrialStageLocked() && (mLevel >= 25 || mApp->HasFinishedAdventure()))
	{
		if (mApp->LawnMessageBox(
			Dialogs::DIALOG_MESSAGE,
			"[REPLAY_LEVEL_HEADER]",
			"[REPLAY_LEVEL_BODY]",
			"[DIALOG_BUTTON_YES]",
			"[DIALOG_BUTTON_NO]",
			Dialog::BUTTONS_YES_NO) == Dialog::ID_NO)
			return;

		mApp->mPlayerInfo->mLevel = 24;
		mApp->mPlayerInfo->mFinishedAdventure = 0;
		mApp->EraseFile(GetSavedGameName(GameMode::GAMEMODE_ADVENTURE, mApp->mPlayerInfo->mId));
		mApp->EraseFile(GetLegacySavedGameName(GameMode::GAMEMODE_ADVENTURE, mApp->mPlayerInfo->mId));
	}

	mApp->mMusic->StopAllMusic();
	mApp->PlaySample(Sexy::SOUND_LOSEMUSIC);
	mStartingGame = true;
	mAdventureButton->SetDisabled(true);
	mMinigameButton->SetDisabled(true);
	mPuzzleButton->SetDisabled(true);
	mOptionsButton->SetDisabled(true);
	mQuitButton->SetDisabled(true);
	mHelpButton->SetDisabled(true);
	mChangeUserButton->SetDisabled(true);
	mStoreButton->SetDisabled(true);
	mAlmanacButton->SetDisabled(true);
	mSurvivalButton->SetDisabled(true);
	mZenGardenButton->SetDisabled(true);
	mZombatarButton->SetDisabled(true); // @Patoke: added new widgets
	mAchievementsButton->SetDisabled(true);

	Reanimation* aHandReanim = mApp->AddReanimation(-70.0f, 10.0f, 0, ReanimationType::REANIM_ZOMBIE_HAND);
	aHandReanim->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
	mHandReanimID = mApp->ReanimationGetID(aHandReanim);
	mApp->PlayFoley(FoleyType::FOLEY_DIRT_RISE);
	for (int i = 0; i < aHandReanim->mDefinition->mTracks.count; i++)
		if (!strncasecmp(aHandReanim->mDefinition->mTracks.tracks[i].mName, "rock", 4))
			aHandReanim->mTrackInstances[i].mIgnoreClipRect = true;
}

// GOTY @Patoke: 0x44F590
bool GameSelector::ShouldDoZenTuturialBeforeAdventure()
{
	return !mApp->HasFinishedAdventure() && mApp->mPlayerInfo->GetLevel() == 45 && mApp->mPlayerInfo->mNumPottedPlants == 0;
}

// GOTY @Patoke: 0x44F5C0
void GameSelector::ButtonDepress(int theId)
{
	if (mSlideCounter > 0)
		return;

	if (theId == GameSelector::GameSelector_Minigame && mMinigamesLocked)
	{
		mApp->LawnMessageBox(Dialogs::DIALOG_MESSAGE, "[MODE_LOCKED]", "[MINIGAME_LOCKED_MESSAGE]", "[DIALOG_BUTTON_OK]", "", Dialog::BUTTONS_FOOTER);
		return;
	}
	if (theId == GameSelector::GameSelector_Puzzle && mPuzzleLocked)
	{
		mApp->LawnMessageBox(Dialogs::DIALOG_MESSAGE, "[MODE_LOCKED]", "[PUZZLE_LOCKED_MESSAGE]", "[DIALOG_BUTTON_OK]", "", Dialog::BUTTONS_FOOTER);
		return;
	}
	if (theId == GameSelector::GameSelector_Survival && mSurvivalLocked)
	{
		mApp->LawnMessageBox(Dialogs::DIALOG_MESSAGE, "[MODE_LOCKED]", "[SURVIVAL_LOCKED_MESSAGE]", "[DIALOG_BUTTON_OK]", "", Dialog::BUTTONS_FOOTER);
		return;
	}

	switch (theId)
	{
	case GameSelector::GameSelector_Adventure:
		ClickedAdventure();
		break;
	case GameSelector::GameSelector_Minigame:
		mApp->KillGameSelector();
		mApp->ShowChallengeScreen(ChallengePage::CHALLENGE_PAGE_CHALLENGE);
		break;
	case GameSelector::GameSelector_Puzzle:
		mApp->KillGameSelector();
		mApp->ShowChallengeScreen(ChallengePage::CHALLENGE_PAGE_PUZZLE);
		break;
	case GameSelector::GameSelector_Survival:
		mApp->KillGameSelector();
		mApp->ShowChallengeScreen(ChallengePage::CHALLENGE_PAGE_SURVIVAL);
		break;
	case GameSelector::GameSelector_Quit:
		mApp->ConfirmQuit();
		break;
	case GameSelector::GameSelector_Help:
		mApp->KillGameSelector();
		mApp->ShowAwardScreen(AwardType::AWARD_HELP_ZOMBIENOTE, false);
		break;
	case GameSelector::GameSelector_Options:
		mApp->DoNewOptions(true);
		break;
	case GameSelector::GameSelector_ChangeUser:
		mApp->DoUserDialog();
		break;
	case GameSelector::GameSelector_Store:
	{
		StoreScreen* aStore = mApp->ShowStoreScreen();
		aStore->WaitForResult(true);
		if (aStore->mGoToTreeNow)
		{
			mApp->KillGameSelector();
			mApp->PreNewGame(GameMode::GAMEMODE_TREE_OF_WISDOM, false);
		}
		else
			mApp->mMusic->MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_TITLE_CRAZY_DAVE_MAIN_THEME);

		break;
	}
	case GameSelector::GameSelector_Almanac:
		mApp->DoAlmanacDialog()->WaitForResult(true);
		mApp->mMusic->MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_TITLE_CRAZY_DAVE_MAIN_THEME);
		break;
	case GameSelector::GameSelector_ZenGarden:
		mApp->KillGameSelector();
		mApp->PreNewGame(GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN, false);
		if (ShouldDoZenTuturialBeforeAdventure())
			mApp->mZenGarden->SetupForZenTutorial();
		break;
	case GameSelector::GameSelector_Zombatar:
		//if (mApp->mPlayerInfo->mAckZombatarTOS)
		//	GameSelector::ShowZombatarScreen();
		//else
		//	LawnApp::ShowZombatarTOS();
		break;
	case GameSelector::GameSelector_AchievementsBack: // @Patoke: seems to be unused
		//SlideTo(0, 0);
		break;
	case GameSelector::GameSelector_Achievements:
		ShowAchievementsScreen();
		break;
	case GameSelector::GameSelector_QuickPlay:
		// GameSelector::ShowQuickPlayScreen();
		break;
	}
}

// GOTY @Patoke: 0x44F880
void GameSelector::AddPreviewProfiles()
{
	PlayerInfo* aProfile;

	aProfile = mApp->mProfileMgr->AddProfile("2 Night");
	if (aProfile)
	{
		aProfile->mLevel = 11;
		aProfile->SaveDetails();
	}

	aProfile = mApp->mProfileMgr->AddProfile("3 Pool");
	if (aProfile)
	{
		aProfile->mLevel = 21;
		aProfile->mHasUnlockedMinigames = 1;
		aProfile->mCoins = 400;
		aProfile->mPurchases[StoreItem::STORE_ITEM_PACKET_UPGRADE] = 1;
		aProfile->SaveDetails();
	}

	aProfile = mApp->mProfileMgr->AddProfile("4 Fog");
	if (aProfile)
	{
		aProfile->mLevel = 31;
		aProfile->mHasUnlockedMinigames = 1;
		aProfile->mHasUnlockedSurvivalMode = 1;
		aProfile->mPurchases[StoreItem::STORE_ITEM_PACKET_UPGRADE] = 2;
		aProfile->mPurchases[StoreItem::STORE_ITEM_POOL_CLEANER] = 1;
		aProfile->mCoins = 400;
		aProfile->SaveDetails();
	}

	aProfile = mApp->mProfileMgr->AddProfile("5 Roof");
	if (aProfile)
	{
		aProfile->mLevel = 41;
		aProfile->mHasUnlockedMinigames = 1;
		aProfile->mHasUnlockedPuzzleMode = 1;
		aProfile->mHasUnlockedSurvivalMode = 1;
		aProfile->mPurchases[StoreItem::STORE_ITEM_PACKET_UPGRADE] = 2;
		aProfile->mPurchases[StoreItem::STORE_ITEM_POOL_CLEANER] = 1;
		aProfile->mCoins = 500;
		aProfile->SaveDetails();
	}

	aProfile = mApp->mProfileMgr->AddProfile("Complete");
	if (aProfile)
	{
		aProfile->mLevel = 1;
		aProfile->mFinishedAdventure = 1;
		aProfile->mHasUnlockedMinigames = 1;
		aProfile->mHasUnlockedPuzzleMode = 1;
		aProfile->mHasUnlockedSurvivalMode = 1;
		aProfile->mPurchases[StoreItem::STORE_ITEM_PACKET_UPGRADE] = 2;
		aProfile->mPurchases[StoreItem::STORE_ITEM_POOL_CLEANER] = 1;
		aProfile->mPurchases[StoreItem::STORE_ITEM_ROOF_CLEANER] = 1;
		aProfile->mCoins = 1000;
		aProfile->SaveDetails();
	}

	aProfile = mApp->mProfileMgr->AddProfile("Full Unlock");
	if (aProfile)
	{
		aProfile->mLevel = 1;
		aProfile->mFinishedAdventure = 2;
		aProfile->AddCoins(50000);
		aProfile->mPurchases[StoreItem::STORE_ITEM_FERTILIZER] = PURCHASE_COUNT_OFFSET + 5;
		aProfile->mPurchases[StoreItem::STORE_ITEM_BUG_SPRAY] = PURCHASE_COUNT_OFFSET + 5;
		aProfile->mPurchases[StoreItem::STORE_ITEM_CHOCOLATE] = PURCHASE_COUNT_OFFSET + 5;
		aProfile->mPurchases[StoreItem::STORE_ITEM_TREE_FOOD] = PURCHASE_COUNT_OFFSET + 5;
		aProfile->mHasUnlockedMinigames = 1;
		aProfile->mHasUnlockedPuzzleMode = 1;
		aProfile->mPurchases[StoreItem::STORE_ITEM_PLANT_GATLINGPEA] = 1;
		aProfile->mPurchases[StoreItem::STORE_ITEM_PLANT_TWINSUNFLOWER] = 1;
		aProfile->mPurchases[StoreItem::STORE_ITEM_PLANT_GLOOMSHROOM] = 1;
		aProfile->mPurchases[StoreItem::STORE_ITEM_PLANT_CATTAIL] = 1;
		aProfile->mPurchases[StoreItem::STORE_ITEM_PLANT_WINTERMELON] = 1;
		aProfile->mPurchases[StoreItem::STORE_ITEM_PLANT_GOLD_MAGNET] = 1;
		aProfile->mPurchases[StoreItem::STORE_ITEM_PLANT_SPIKEROCK] = 1;
		aProfile->mPurchases[StoreItem::STORE_ITEM_PLANT_COBCANNON] = 1;
		aProfile->mPurchases[StoreItem::STORE_ITEM_PLANT_IMITATER] = 1;
		aProfile->mPurchases[StoreItem::STORE_ITEM_PACKET_UPGRADE] = 3;
		aProfile->mPurchases[StoreItem::STORE_ITEM_POOL_CLEANER] = 1;
		aProfile->mPurchases[StoreItem::STORE_ITEM_ROOF_CLEANER] = 1;
		aProfile->mPurchases[StoreItem::STORE_ITEM_PHONOGRAPH] = 1;
		aProfile->mPurchases[StoreItem::STORE_ITEM_GARDENING_GLOVE] = 1;
		aProfile->mPurchases[StoreItem::STORE_ITEM_MUSHROOM_GARDEN] = 1;
		aProfile->mPurchases[StoreItem::STORE_ITEM_WHEEL_BARROW] = 1;
		aProfile->mPurchases[StoreItem::STORE_ITEM_AQUARIUM_GARDEN] = 1;
		aProfile->mPurchases[StoreItem::STORE_ITEM_TREE_OF_WISDOM] = 1;

		aProfile->mChallengeRecords[static_cast<int>(GameMode::GAMEMODE_TREE_OF_WISDOM) - 1] = 1;
		for (int i = 1; i < 100; i++)
			if (i != static_cast<int>(GameMode::GAMEMODE_TREE_OF_WISDOM) && i != static_cast<int>(GameMode::GAMEMODE_SCARY_POTTER_ENDLESS) &&
				i != static_cast<int>(GameMode::GAMEMODE_PUZZLE_I_ZOMBIE_ENDLESS) && i != static_cast<int>(GameMode::GAMEMODE_SURVIVAL_ENDLESS_STAGE_3))
				mApp->mPlayerInfo->mChallengeRecords[i - 1] = 20;

		aProfile->SaveDetails();
	}
}

// @Patoke: implemented functions
// GOTY @Patoke: 0x450140
void GameSelector::SlideTo(int theX, int theY) {
	if (mSlideCounter > 0)
		return;

	mSlideCounter = 75;
	mDestX = theX;
	mDestY = theY;
	mStartX = mX;
	mStartY = mY;
}

// GOTY @Patoke: 0x450200
void GameSelector::ShowAchievementsScreen() {
	SlideTo(0, -mApp->mHeight);
	mWidgetManager->SetFocus(mAchievementsWidget);
}
