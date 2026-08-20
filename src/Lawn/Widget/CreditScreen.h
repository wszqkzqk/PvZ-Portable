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

#ifndef __CREDITSCREEN_H__
#define __CREDITSCREEN_H__

#include "../../ConstEnums.h"
#include "widget/Widget.h"
#include "misc/PerfTimer.h"
#include "widget/ButtonListener.h"
#include <memory>

using namespace Sexy;

enum CreditsPhase
{
	CREDITS_MAIN1,
	CREDITS_MAIN2,
	CREDITS_MAIN3,
	CREDITS_END
};

enum CreditLayer
{
	CREDIT_LAYER_BACKGROUND,
	CREDIT_LAYER_ZOMBIE,
	CREDIT_LAYER_TOP
};

enum CreditWordType
{
	WORD_AA,
	WORD_EE,
	WORD_AW,
	WORD_OH,
	WORD_OFF
};

enum CreditBrainType
{
	BRAIN_FLY_ON,
	BRAIN_FAST_ON,
	BRAIN_NEXT_WORD,
	BRAIN_FAST_OFF,
	BRAIN_FLY_OFF,
	BRAIN_OFF
};

class CreditsTiming
{
public:
	float					mFrame;
	CreditWordType			mWordType;
	int						mWordX;
	CreditBrainType			mBrainType;
};

class GameButton;
class LawnApp;
class LawnStoneButton;
class NewLawnButton;
class Reanimation;

class CreditScreen : public Widget, public ButtonListener
{
protected:
	enum
	{
		Credits_Button_Replay,
		Credits_Button_MainMenu
	};

public:
	GameButton*				mCloseButton;
	LawnApp*				mApp;
	CreditsPhase			mCreditsPhase;
	int						mCreditsPhaseCounter;
	ReanimationID			mCreditsReanimID;
	ParticleSystemID		mFogParticleID;
	int						mBlinkCountdown;
	std::unique_ptr<LawnStoneButton>	mMainMenuButton;
	std::unique_ptr<NewLawnButton>	mReplayButton;
	std::unique_ptr<Widget>			mOverlayWidget;
	bool					mDrawBrain;
	float					mBrainPosX;
	float					mBrainPosY;
	int						mUpdateCount;
	int						mDrawCount;
	PerfTimer				mTimerSinceStart;
	bool					mDontSync;
	bool					mCreditsPaused;
	double					mOriginalMusicVolume;
	bool					mPreloaded;
	int						mLastDrawCount;

public:
	CreditScreen(LawnApp* theApp);
	~CreditScreen() override;

	void					Update() override;
	void					Draw(Graphics* g) override;
	void					KeyChar(char theChar) override;
	void					KeyDown(KeyCode theKey) override;
	void					MouseUp(int x, int y, int theClickCount) override;
	void					AddedToManager(WidgetManager* theWidgetManager) override;
	void					RemovedFromManager(WidgetManager* theWidgetManager) override;
	void					ButtonPress(int theId) override;
	void					ButtonDepress(int theId) override;
	void					ButtonDownTick(int) override{}
	void					ButtonMouseEnter(int) override{}
	void					ButtonMouseLeave(int) override{}
	void					ButtonMouseMove(int, int, int) override{}
	Reanimation*			PlayReanim(int aIndex);
	void					JumpToFrame(CreditsPhase thePhase, float theFrame);
	void					GetTiming(CreditsTiming** theBeforeTiming, CreditsTiming** theAfterTiming, float* theFraction);
	static Reanimation*		FindSubReanim(Reanimation* theReanim, ReanimationType theReanimType);
	void					DrawFogEffect(Graphics* g, float theTime);
	void					UpdateBlink();
	void					TurnOffTongues(Reanimation* theReanim, int aParentTrack);
	void					DrawFinalCredits(Graphics* g);
	void					DrawOverlay(Graphics* g) override;
	void					UpdateMovie();
	void					PauseCredits();
	void					PreLoadCredits();
};

void						DrawDisco(Graphics* g, float aCenterX, float aCenterY, float theTime);
void						DrawReanimToPreload(Graphics* g, ReanimationType theReanimType);

class CreditsOverlay : public Widget
{
public:
	CreditScreen*			mParent;

public:
	CreditsOverlay(CreditScreen* theCreditScreen);

	void					Draw(Graphics* g) override;
};

#endif
