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

#include "TitleScreen.h"
#include "widget/HyperlinkWidget.h"
#include "widget/WidgetManager.h"
#include "graphics/ImageFont.h"
#include "sound/SoundManager.h"
#include "../../LawnApp.h"
#include "../../Resources.h"
#include "../../Sexy.TodLib/TodCommon.h"
#include "misc/SexyMatrix.h"
#include "../../Sexy.TodLib/TodStringFile.h"
#include "../../Sexy.TodLib/EffectSystem.h"
#include "../../Sexy.TodLib/TodDebug.h"
#include "../../Sexy.TodLib/Reanimator.h"
#include "../../GameConstants.h"
#include "../System/Music.h"

// GOTY @Patoke: 0x498220
TitleScreen::TitleScreen(LawnApp* theApp)
{
	mCurBarWidth = 0.0f;
	mTotalBarWidth = 314.0f;
	mBarVel = 0.2f;
	mBarStartProgress = 0.0f;
	mPrevLoadingPercent = 0.0f;
	mApp = theApp;
	mTitleAge = 0;
	mNeedRegister = false;
	mRegisterClicked = false;
	mNeedShowRegisterBox = false;
	mLoadingThreadComplete = false;
	mDrawnYet = false;
	mNeedToInit = true;
	mQuickLoadKey = KeyCode::KEYCODE_UNKNOWN;
	mDisplayPartnerLogo = mApp->GetBoolean("DisplayPartnerLogo", false);
	mTitleState = TitleState::TITLESTATE_WAITING_FOR_FIRST_DRAW;
	mTitleStateDuration = 0;
	mTitleStateCounter = 0;
	mLoaderScreenIsLoaded = false;

	mStartButton = new Sexy::HyperlinkWidget(0, this);
	mStartButton->mColor = Sexy::Color(218, 184, 33);
	mStartButton->mOverColor = Sexy::Color(250, 90, 15);
	mStartButton->mUnderlineSize = 0;
	mStartButton->mDisabled = true;
	mStartButton->mVisible = false;
}

TitleScreen::~TitleScreen()
{
	if (mStartButton)
	{
		delete mStartButton;
	}

#ifdef LOW_MEMORY
	// Selectively delete only title-screen-specific resources from LoaderBar group.
	mApp->mResourceManager->DeleteImage("IMAGE_LOADBAR_DIRT");
	mApp->mResourceManager->DeleteImage("IMAGE_LOADBAR_GRASS");
	mApp->mResourceManager->DeleteImage("IMAGE_TITLESCREEN");
	mApp->mResourceManager->DeleteSound("SOUND_LOADINGBAR_ZOMBIE");
#endif
}

void TitleScreen::DrawToPreload(Graphics* g)
{
	g->DrawImageF(IMAGE_PLANTSHADOW, 1000.0f, 0.0f);
}

void TitleScreen::Draw(Graphics* g)
{
	g->SetLinearBlend(true);

	if (mTitleState == TitleState::TITLESTATE_WAITING_FOR_FIRST_DRAW)
	{
		g->SetColor(Color::Black);
		g->FillRect(0, 0, mWidth, mHeight);

		if (!mDrawnYet)
		{
			TodTraceAndLogLn("First Draw Time: %d ms", SDL_GetTicks() - mApp->mTimeLoaded);
			TodHesitationTrace("TitleScreen First Draw");
			mDrawnYet = true;
		}

		return;
	}
	
	if (mTitleState == TitleState::TITLESTATE_POPCAP_LOGO)
	{
		g->SetColor(Color::Black);
		g->FillRect(0, 0, mWidth, mHeight);

		int anAlpha = 255;
		if (mTitleStateCounter < mTitleStateDuration - 50)
		{
			if (!mDisplayPartnerLogo)
			{
				anAlpha = TodAnimateCurve(50, 0, mTitleStateCounter, 255, 0, TodCurves::CURVE_LINEAR);
			}
		}
		else
		{
			anAlpha = TodAnimateCurve(mTitleStateDuration, mTitleStateDuration - 50, mTitleStateCounter, 0, 255, TodCurves::CURVE_LINEAR);
		}
		g->SetColorizeImages(true);
		g->SetColor(Color(255, 255, 255, anAlpha));
		g->DrawImage(IMAGE_POPCAP_LOGO, (mWidth - IMAGE_POPCAP_LOGO->mWidth) / 2, (mHeight - IMAGE_POPCAP_LOGO->mHeight) / 2);
		g->SetColorizeImages(false);

		return;
	}

	if (mTitleState == TitleState::TITLESTATE_PARTNER_LOGO)
	{
		g->SetColor(Color::Black);
		g->FillRect(0, 0, mWidth, mHeight);

		g->SetColorizeImages(true);
		int anAlpha = 255;
		if (mTitleStateCounter >= mTitleStateDuration - 35)
		{
			anAlpha = TodAnimateCurve(mTitleStateDuration, mTitleStateDuration - 35, mTitleStateCounter, 0, 255, TodCurves::CURVE_LINEAR);
			g->SetColor(Color(255, 255, 255, 255 - anAlpha));
			g->DrawImage(IMAGE_POPCAP_LOGO, (mWidth - IMAGE_POPCAP_LOGO->mWidth) / 2, (mHeight - IMAGE_POPCAP_LOGO->mHeight) / 2);
		}
		else
		{
			anAlpha = TodAnimateCurve(35, 0, mTitleStateCounter, 255, 0, TodCurves::CURVE_LINEAR);
		}
		g->SetColor(Color(255, 255, 255, anAlpha));
		g->DrawImage(IMAGE_PARTNER_LOGO, (mWidth - IMAGE_PARTNER_LOGO->mWidth) / 2, (mHeight - IMAGE_PARTNER_LOGO->mHeight) / 2);
		g->SetColorizeImages(false);

		return;
	}

	if (!mLoaderScreenIsLoaded)
	{
		g->SetColor(Color::Black);
		g->FillRect(0, 0, mWidth, mHeight);
		return;
	}

	g->DrawImage(IMAGE_TITLESCREEN, 0, 0);
	if (mNeedToInit)
	{
		return;
	}

	int aLogoY;
	if (mTitleStateCounter > 60)
	{
		aLogoY = TodAnimateCurve(100, 60, mTitleStateCounter, -150, 10, CURVE_EASE_IN);
	}
	else
	{
		aLogoY = TodAnimateCurve(60, 50, mTitleStateCounter, 10, 15, CURVE_BOUNCE);
	}
	g->DrawImage(IMAGE_PVZ_LOGO, mWidth / 2 - IMAGE_PVZ_LOGO->mWidth / 2, aLogoY);

	int aGrassX = mStartButton->mX;
	int aGrassY = mStartButton->mY - 17;

	//Sexy::PrintF("%d %d\n", aGrassX, aGrassY);
	g->DrawImage(IMAGE_LOADBAR_DIRT, aGrassX, aGrassY + 18);

	if (mCurBarWidth >= mTotalBarWidth)
	{
		g->DrawImage(IMAGE_LOADBAR_GRASS, aGrassX, aGrassY);

		if (mLoadingThreadComplete)
		{
			DrawToPreload(g);
		}
	}
	else
	{
		Graphics aClipG(*g);
		aClipG.ClipRect(240, aGrassY, mCurBarWidth, IMAGE_LOADBAR_GRASS->mHeight);
		aClipG.DrawImage(IMAGE_LOADBAR_GRASS, aGrassX, aGrassY);

		float aRollLen = mCurBarWidth * 0.94f;
		float aRotation = -aRollLen / 180 * PI * 2;
		float aScale = TodAnimateCurveFloatTime(0, mTotalBarWidth, mCurBarWidth, 1, 0.5f, TodCurves::CURVE_LINEAR);
		SexyTransform2D aTransform;
		TodScaleRotateTransformMatrix(aTransform, aGrassX + 11.0f + aRollLen, aGrassY - 3.0f - 35.0f * aScale + 35.0f, aRotation, aScale, aScale);
		Rect aSrcRect(0, 0, IMAGE_REANIM_SODROLLCAP->mWidth, IMAGE_REANIM_SODROLLCAP->mHeight);
		TodBltMatrix(g, IMAGE_REANIM_SODROLLCAP, aTransform, g->mClipRect, Color::White, g->mDrawMode, aSrcRect);
	}

	Reanimation* aReanim = nullptr;
	while (mApp->mEffectSystem->mReanimationHolder->mReanimations.IterateNext(aReanim))
	{
		aReanim->Draw(g);
	}
}

// GOTY @Patoke: 0x498A20
void TitleScreen::Update()
{
	Widget::Update();
	if (mApp->mShutdown)
	{
		return;
	}

	MarkDirty();
	if (!mDrawnYet)
	{
		return;
	}

	if (mTitleState == TitleState::TITLESTATE_WAITING_FOR_FIRST_DRAW)
	{
		mApp->mMusic->MusicTitleScreenInit();
		mApp->StartLoadingThread();

		mTitleState = TitleState::TITLESTATE_POPCAP_LOGO;
		if (mDisplayPartnerLogo)
		{
			mTitleStateDuration = 150;
		}
		else
		{
			mTitleStateDuration = 200;
		}
		
		mTitleStateCounter = mTitleStateDuration;
	}

	if (mQuickLoadKey != KeyCode::KEYCODE_UNKNOWN && mTitleState != TitleState::TITLESTATE_SCREEN)
	{
		mTitleState = TitleState::TITLESTATE_SCREEN;
		mTitleStateDuration = 0;
		mTitleStateCounter = 100;
	}

	mTitleAge++;
	if (mTitleStateCounter > 0)
	{
		mTitleStateCounter--;
	}

	if (mTitleState == TitleState::TITLESTATE_POPCAP_LOGO)
	{
		if (mTitleStateCounter == 0)
		{
			if (mDisplayPartnerLogo)
			{
				mTitleState = TitleState::TITLESTATE_SCREEN;
				mTitleStateDuration = 200;
				mTitleStateCounter = 200;
			}
			else
			{
				mTitleState = TitleState::TITLESTATE_SCREEN;
				mTitleStateDuration = 100;
				mTitleStateCounter = 100;
			}
		}

		return;
	}
	else if (mTitleState == TitleState::TITLESTATE_PARTNER_LOGO)
	{
		if (mTitleStateCounter == 0)
		{
			mTitleState = TitleState::TITLESTATE_SCREEN;
			mTitleStateDuration = 100;
			mTitleStateCounter = 100;
		}

		return;
	}

	if (!mLoaderScreenIsLoaded)
	{
		return;
	}

	float aCurrentProgress = mApp->GetLoadingThreadProgress();
	if (mNeedToInit)
	{
		mNeedToInit = false;

		mStartButton->mLabel = TodStringTranslate("[LOADING]");
		mStartButton->SetFont(FONT_BRIANNETOD16);
		mStartButton->Resize((mWidth - IMAGE_LOADBAR_DIRT->mWidth) / 2, 650, mTotalBarWidth, 50);
		mStartButton->mVisible = true;

		float aEstimatedTotalLoadTime;
		if (aCurrentProgress > 0.000001f)
		{
			aEstimatedTotalLoadTime = mTitleAge / aCurrentProgress;
		}
		else
		{
			aEstimatedTotalLoadTime = 3000;
		}

		float aLoadTime = aEstimatedTotalLoadTime * (1 - aCurrentProgress);
		aLoadTime = ClampFloat(aLoadTime, 100, 3000);
		mBarVel = mTotalBarWidth / aLoadTime;
		mBarStartProgress = std::min(aCurrentProgress, 0.9f);
	}

	float aLoadingPercent = (aCurrentProgress - mBarStartProgress) / (1 - mBarStartProgress);

	int aButtonY;
	if (mTitleStateCounter > 10)
	{
		aButtonY = TodAnimateCurve(60, 10, mTitleStateCounter, 650, 534, TodCurves::CURVE_EASE_IN);
	}
	else
	{
		aButtonY = TodAnimateCurve(10, 0, mTitleStateCounter, 534, 529, TodCurves::CURVE_BOUNCE);
	}
	mStartButton->Resize(mStartButton->mX, aButtonY, mTotalBarWidth, mStartButton->mHeight);

	if (mTitleStateCounter > 0)
	{
		return;
	}

	mApp->mEffectSystem->Update();

	float aPrevWidth = mCurBarWidth;
	mCurBarWidth += mBarVel;
	if (!mLoadingThreadComplete)
	{
		if (mCurBarWidth > mTotalBarWidth * 0.99f)
		{
			mCurBarWidth = mTotalBarWidth * 0.99f;
		}
	}
	else if (mCurBarWidth > mTotalBarWidth)
	{
		mStartButton->mLabel = TodStringTranslate("[CLICK_TO_START]");
		mCurBarWidth = mTotalBarWidth;
	}

	if (aLoadingPercent > mPrevLoadingPercent + 0.01f || mLoadingThreadComplete)
	{
		float aBarWidth = TodAnimateCurveFloatTime(0, 1, aLoadingPercent, 0, mTotalBarWidth, TodCurves::CURVE_EASE_IN);
		float aDiff = aBarWidth - mCurBarWidth;
		float aAcceleration = TodAnimateCurveFloatTime(0, 1, aLoadingPercent, 0.0001f, 0.00001f, TodCurves::CURVE_LINEAR);
		if (mLoadingThreadComplete)
		{
			aAcceleration = 0.0001f;
		}
		mBarVel += aDiff * abs(aDiff) * aAcceleration;

		float aMinVelocity = TodAnimateCurveFloatTime(0, 1, aLoadingPercent, 0.2f, 0.01f, TodCurves::CURVE_LINEAR);
		float aMaxVelocity = 2;
		if (mApp->mTodCheatKeys)
		{
			aMinVelocity = 0;
			aMaxVelocity = 100;
		}

		if (mBarVel < aMinVelocity)
		{
			mBarVel = aMinVelocity;
		}
		else if (mBarVel > aMaxVelocity)
		{
			mBarVel = aMaxVelocity;
		}

		mPrevLoadingPercent = aLoadingPercent;
	}

	if (!mLoadingThreadComplete && mApp->mLoadingThreadCompleted)
	{
		mLoadingThreadComplete = true;
		mStartButton->SetDisabled(false);

		if (mQuickLoadKey == KeyCode::KEYCODE_ASCIIEND)
		{
			mApp->FastLoad(GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN);
		}
		else if (mQuickLoadKey == (KeyCode)0x4D)
		{
			mApp->LoadingCompleted();
		}
		else if (mQuickLoadKey == (KeyCode)0x53)
		{
			mApp->LoadingCompleted();
			mApp->KillGameSelector();
			mApp->ShowChallengeScreen(ChallengePage::CHALLENGE_PAGE_SURVIVAL);
		}
		else if (mQuickLoadKey == (KeyCode)0x43)
		{
			mApp->LoadingCompleted();
			mApp->KillGameSelector();
			mApp->ShowChallengeScreen(ChallengePage::CHALLENGE_PAGE_CHALLENGE);
		}
		else if (mQuickLoadKey == (KeyCode)0x55)
		{
			mApp->LoadingCompleted();
			mApp->KillGameSelector();
			mApp->PreNewGame(GameMode::GAMEMODE_UPSELL, false);
		}
		else if (mQuickLoadKey == (KeyCode)0x49)
		{
			mApp->LoadingCompleted();
			mApp->KillGameSelector();
			mApp->PreNewGame(GameMode::GAMEMODE_INTRO, false);
		}
		else if (mQuickLoadKey == (KeyCode)0x50)
		{
			mApp->LoadingCompleted();
			mApp->KillGameSelector();
			mApp->ShowChallengeScreen(ChallengePage::CHALLENGE_PAGE_PUZZLE);
		}
		else if (mQuickLoadKey == (KeyCode)0x52)
		{
			mApp->LoadingCompleted();
			mApp->KillGameSelector();
			mApp->ShowCreditScreen();
		}
		else if (mApp->mTodCheatKeys && mApp->mPlayerInfo && mQuickLoadKey == (KeyCode)0x54)
		{
			mApp->FastLoad(GameMode::GAMEMODE_ADVENTURE);
		}
		else
		{
			mStartButton->SetVisible(true);
		}
	}

	float aTriggerPoint[] = { 
		mTotalBarWidth * 0.11f, 
		mTotalBarWidth * 0.32f, 
		mTotalBarWidth * 0.54f, 
		mTotalBarWidth * 0.72f, 
		mTotalBarWidth * 0.91f 
	};

	for (size_t i = 0; i < LENGTH(aTriggerPoint); i++)
	{
		if (aPrevWidth < aTriggerPoint[i] && mCurBarWidth >= aTriggerPoint[i])
		{
			ReanimationType aReanimType = ReanimationType::REANIM_LOADBAR_SPROUT;
			if (i == 4)
			{
				aReanimType = ReanimationType::REANIM_LOADBAR_ZOMBIEHEAD;
			}
			float aPosX = aTriggerPoint[i] + 225.0f;
			float aPosY = 511.0f;
			Reanimation* aSproutReanim = mApp->AddReanimation(aPosX, aPosY, 0, aReanimType);
			aSproutReanim->mAnimRate = 18.0f;
			aSproutReanim->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;

			if (i == 1 || i == 3)
			{
				aSproutReanim->OverrideScale(-1.0f, 1.0f);
			}
			else if (i == 2)
			{
				aSproutReanim->SetPosition(aPosX, aPosY - 5.0f);
				aSproutReanim->OverrideScale(1.1f, 1.3f);
			}
			else if (i == 4)
			{
				aSproutReanim->SetPosition(aPosX - 20.0f, aPosY);
			}

			if (i == 4)
			{
				mApp->PlaySample(SOUND_LOADINGBAR_FLOWER);
				mApp->PlaySample(SOUND_LOADINGBAR_ZOMBIE);
			}
			else
			{
				mApp->PlaySample(SOUND_LOADINGBAR_FLOWER);
			}
		}
	}
}

void TitleScreen::Resize(int theX, int theY, int theWidth, int theHeight)
{
	Widget::Resize(theX, theY, theWidth, theHeight);
}

void TitleScreen::AddedToManager(Sexy::WidgetManager* theWidgetManager)
{
	Widget::AddedToManager(theWidgetManager);
	theWidgetManager->AddWidget(mStartButton);
}

void TitleScreen::RemovedFromManager(Sexy::WidgetManager* theWidgetManager)
{
	Widget::RemovedFromManager(theWidgetManager);
	theWidgetManager->RemoveWidget(mStartButton);
}

void TitleScreen::ButtonPress(int theId)
{
	(void)theId;
	mApp->PlaySample(Sexy::SOUND_BUTTONCLICK);
}

void TitleScreen::ButtonDepress(int theId)
{
	switch (theId)
	{
	case TitleScreen::TitleScreen_Start:
		mApp->LoadingCompleted();
		break;

	case TitleScreen::TitleScreen_Register:
		mRegisterClicked = true;
		break;
	}
}

void TitleScreen::MouseDown(int x, int y, int theClickCount)
{
	(void)x;(void)y;(void)theClickCount;
	if (mLoadingThreadComplete)
	{
		mApp->PlaySample(Sexy::SOUND_BUTTONCLICK);
		mApp->LoadingCompleted();
	}
}

void TitleScreen::KeyDown(KeyCode theKey)
{
	if (mLoadingThreadComplete)
	{
		mApp->PlaySample(Sexy::SOUND_BUTTONCLICK);
		mApp->LoadingCompleted();
	}

	if (mApp->mTodCheatKeys && mApp->mPlayerInfo)
	{
		mQuickLoadKey = theKey;
	}
}
