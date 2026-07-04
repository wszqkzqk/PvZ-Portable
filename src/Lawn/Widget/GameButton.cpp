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
#include "../../Resources.h"
#include "../../Sexy.TodLib/TodCommon.h"
#include "graphics/Font.h"
//#include "graphics/SysFont.h"
#include "graphics/Graphics.h"
#include "../../Sexy.TodLib/TodStringFile.h"
#include "widget/WidgetManager.h"

static Color gGameButtonColors[6] = { Color(0, 0, 0), Color(0, 0, 0), Color(0, 0, 0), Color(255, 255, 255), Color(132, 132, 132), Color(212, 212, 212) };

void DrawStoneButton(Graphics* g, int x, int y, int theWidth, int theHeight, bool isDown, bool isHighLighted, const std::string& theLabel)
{
	Image* aLeftImage = Sexy::IMAGE_BUTTON_LEFT;
	Image* aMiddleImage = Sexy::IMAGE_BUTTON_MIDDLE;
	Image* aRightImage = Sexy::IMAGE_BUTTON_RIGHT;
	int aFontX = x;
	int aFontY = y;
	int aImageX = x;
	if (isDown)
	{
		aLeftImage = Sexy::IMAGE_BUTTON_DOWN_LEFT;
		aMiddleImage = Sexy::IMAGE_BUTTON_DOWN_MIDDLE;
		aRightImage = Sexy::IMAGE_BUTTON_DOWN_RIGHT;
		aFontX++;
		aFontY++;
		aImageX++;
	}

	int aRepeat = (theWidth - aLeftImage->mWidth - aRightImage->mWidth) / aMiddleImage->mWidth;
	g->DrawImage(aLeftImage, aImageX, y);
	aImageX += aLeftImage->mWidth;
	while (aRepeat > 0)
	{
		g->DrawImage(aMiddleImage, aImageX, y);
		aImageX += aMiddleImage->mWidth;
		--aRepeat;
	}
	g->DrawImage(aRightImage, aImageX, y);

	g->SetFont(isHighLighted ? Sexy::FONT_DWARVENTODCRAFT18BRIGHTGREENINSET : Sexy::FONT_DWARVENTODCRAFT18GREENINSET);
	aFontX += (theWidth - Sexy::FONT_DWARVENTODCRAFT18GREENINSET->StringWidth(theLabel)) / 2 + 1;
	aFontY += (theHeight - Sexy::FONT_DWARVENTODCRAFT18GREENINSET->GetAscent() / 6 - 1 + Sexy::FONT_DWARVENTODCRAFT18GREENINSET->GetAscent()) / 2 - 4;
	g->SetColor(Color::White);
	g->DrawString(theLabel, aFontX, aFontY);
}

GameButton::GameButton(int theId)
{
	mLabel = "";
	mNormalRect = mOverRect = mDownRect = mDisabledRect = Rect();
	mOverAlpha = mOverAlphaSpeed = mOverAlphaFadeInSpeed = 0;
	mApp = (LawnApp*)gSexyAppBase;
	mId = theId;
	mLabelJustify = 0;
	mFont = nullptr;
	mButtonImage = mOverImage = mDownImage = mDisabledImage = mOverOverlayImage = nullptr;
	mInverted = mBtnNoDraw = mFrameNoDraw = false;
	mIsOver = mIsDown = mDisabled = false;
	mX = mY = mWidth = mHeight = 0;
	mParentWidget = nullptr;
	mDrawStoneButton = false;
	mTextOffsetX = mTextOffsetY = mButtonOffsetX = mButtonOffsetY = 0;
	for (int i = 0; i < 6; i++) mColors[i] = gGameButtonColors[i];
}

GameButton::~GameButton()
{
	if (mFont)
		delete mFont;
}

bool GameButton::HaveButtonImage(Image* theImage, Rect& theRect)
{ 
	return theImage != nullptr || theRect.mWidth != 0;
}

void GameButton::DrawButtonImage(Graphics* g, Image* theImage, Rect& theRect, int theX, int theY)
{
	int aPosX = theX + mButtonOffsetX;
	int aPosY = theY + mButtonOffsetY;
	if (theRect.mWidth != 0)
		g->DrawImage(mButtonImage, aPosX, aPosY, theRect);
	else
		g->DrawImage(theImage, aPosX, aPosY);
}

void GameButton::SetDisabled(bool theDisabled)
{ 
	mDisabled = theDisabled;
}

void GameButton::SetFont(_Font* theFont)
{
	if (mFont)
		delete mFont;
	
	mFont = theFont->Duplicate();
}

bool GameButton::IsButtonDown()
{ 
	return mIsDown && mIsOver && !mDisabled && !mBtnNoDraw;
}

// GOTY @Patoke: 0x44AAC0
void GameButton::Draw(Graphics* g)
{
	if (mBtnNoDraw)
		return;

	bool isDown = IsButtonDown() ^ mInverted;
	bool isHighLighted = IsMouseOver();
	if (mDrawStoneButton)
	{
		DrawStoneButton(g, mX, mY, mWidth, mHeight, isDown, isHighLighted, mLabel);
		return;
	}

	g->mTransX += mX;
	g->mTransY += mY;
	if (!mFont && mLabel.size() > 0)
		mFont = FONT_PICO129->Duplicate();
	
	int aFontX = mTextOffsetX;
	int aFontY = mTextOffsetY;
	if (mFont)
	{
		if (mLabelJustify == GameButton::BUTTON_LABEL_CENTER)
			aFontX += (mWidth - mFont->StringWidth(mLabel)) / 2;
		else if (mLabelJustify == GameButton::BUTTON_LABEL_RIGHT)
			aFontX += mWidth - mFont->StringWidth(mLabel);

		aFontY += (mHeight - mFont->GetAscent() / 6 + mFont->GetAscent() - 1) / 2;
	}
	g->SetFont(mFont);

	if (!isDown)
	{
		if (mDisabled && HaveButtonImage(mDisabledImage, mDisabledRect))
			DrawButtonImage(g, mDisabledImage, mDisabledRect, 0, 0);
		else if (mOverAlpha > 0.0f && HaveButtonImage(mOverImage, mOverRect))
		{
			if (HaveButtonImage(mButtonImage, mNormalRect) && mOverAlpha < 1.0f)  // 未完全过渡结束
				DrawButtonImage(g, mButtonImage, mNormalRect, 0, 0);

			g->SetColorizeImages(true);
			g->SetColor(Color(255, 255, 255, mOverAlpha * 255));
			DrawButtonImage(g, mOverImage, mOverRect, 0, 0);
			g->SetColorizeImages(false);
		}
		else if (isHighLighted && HaveButtonImage(mOverImage, mOverRect))
			DrawButtonImage(g, mOverImage, mOverRect, 0, 0);
		else if (HaveButtonImage(mButtonImage, mNormalRect))
			DrawButtonImage(g, mButtonImage, mNormalRect, 0, 0);

		g->SetColor(mColors[isHighLighted ? GameButton::COLOR_LABEL_HILITE : GameButton::COLOR_LABEL]);
		g->DrawString(mLabel, aFontX, aFontY);

		if (isHighLighted && mOverOverlayImage)
		{
			g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
			DrawButtonImage(g, mOverOverlayImage, mNormalRect, 0, 0);
			g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
		}
	}
	else
	{
		if (HaveButtonImage(mDownImage, mDownRect))
			DrawButtonImage(g, mDownImage, mDownRect, 0, 0);
		else if (HaveButtonImage(mOverImage, mOverRect))
			DrawButtonImage(g, mOverImage, mOverRect, 1, 1);
		else
			DrawButtonImage(g, mButtonImage, mNormalRect, 1, 1);
		
		g->SetColor(mColors[GameButton::COLOR_LABEL_HILITE]);
		g->DrawString(mLabel, aFontX + 1, aFontY + 1);

		if (isHighLighted && mOverOverlayImage)
		{
			g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
			DrawButtonImage(g, mOverOverlayImage, mNormalRect, 0, 0);
			g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
		}
	}

	g->Translate(-mX, -mY);
}

void GameButton::Resize(int theX, int theY, int theWidth, int theHeight)
{ 
	mX = theX;
	mY = theY;
	mWidth = theWidth;
	mHeight = theHeight;
}

bool GameButton::IsMouseOver()
{
	if (mDisabled || mBtnNoDraw)
		return false;

	WidgetManager* aManager = mApp->mWidgetManager;
	bool aFocusMatches = aManager->mFocusWidget && aManager->mFocusWidget == mParentWidget;
	if (!aFocusMatches && mApp->GetDialogCount() > 0)
		return false;

	int aMouseX = aManager->mLastMouseX, aMouseY = aManager->mLastMouseY;
	if (mParentWidget)
	{
		Point anAbsPos = mParentWidget->GetAbsPos();
		aMouseX -= anAbsPos.mX;
		aMouseY -= anAbsPos.mY;
	}
	return Rect(mX, mY, mWidth, mHeight).Contains(aMouseX, aMouseY);
}

// GOTY @Patoke: 0x44AF50
void GameButton::Update()
{
	WidgetManager* aManager = mApp->mWidgetManager;
	mIsOver = IsMouseOver();

	bool aFocusMatches = aManager->mFocusWidget && aManager->mFocusWidget == mParentWidget;
	if (aFocusMatches || mApp->GetDialogCount() <= 0)
		mIsDown = aManager->mDownButtons & 5;
	else
		mIsDown = false;

	if (!mIsDown && !mIsOver && mOverAlpha > 0)
	{
		if (mOverAlphaSpeed < 0)
		{ 
			mOverAlpha = 0;
			return;
		}
		mOverAlpha -= mOverAlphaSpeed;
		if (mOverAlpha < 0)
			mOverAlpha = 0;
	}
	else if (mIsOver && mOverAlphaFadeInSpeed > 0 && mOverAlpha < 1)
	{
		if (mOverAlphaFadeInSpeed > 0)
		{
			mOverAlpha += mOverAlphaFadeInSpeed;
			if (mOverAlpha > 1)
				mOverAlpha = 1;
		}
		else
			mOverAlpha = 1;
	}
}

void GameButton::SetLabel(std::string_view theLabel)
{
	mLabel = TodStringTranslate(theLabel);
}

void NewLawnButton::SetLabel(std::string_view theLabel)
{
	mLabel = TodStringTranslate(theLabel);
}

void LawnStoneButton::SetLabel(std::string_view theLabel)
{
	mLabel = TodStringTranslate(theLabel);
}

void LawnStoneButton::Draw(Graphics* g)
{
	if (mBtnNoDraw)
		return;

	bool isDown = (mIsDown && mIsOver && !mDisabled) ^ mInverted;
	DrawStoneButton(g, 0, 0, mWidth, mHeight, isDown, mIsOver, mLabel);
}

LawnStoneButton* MakeButton(int theId, ButtonListener* theListener, std::string_view theText)
{
	LawnStoneButton* aButton = new LawnStoneButton(nullptr, theId, theListener);
	aButton->SetLabel(theText);

	aButton->mTranslateX = 1;
	aButton->mTranslateY = 1;
	aButton->mHasAlpha = true;
	aButton->mHasTransparencies = true;
	aButton->mHeight = 33;
	return aButton;
}

// GOTY @Patoke: 0x44B2B0
NewLawnButton::NewLawnButton(Image* theComponentImage, int theId, ButtonListener* theListener) : DialogButton(theComponentImage, theId, theListener)
{
	mHiliteFont = nullptr;
	mTextDownOffsetX = 0;
	mTextDownOffsetY = 0;
	mButtonOffsetX = 0;
	mButtonOffsetY = 0;
	mUsePolygonShape = false;
	SetColor(ButtonWidget::COLOR_BKG, Color::White);
}

NewLawnButton::~NewLawnButton()
{
	//if (mHiliteFont)
	//	delete mHiliteFont;
}

// GOTY @Patoke: 0x44B383
void NewLawnButton::Draw(Graphics* g)
{
	if (mBtnNoDraw)
		return;

	bool isDown = (mIsDown && mIsOver && !mDisabled) ^ mInverted;
	int aFontX = mTextOffsetX + mTranslateX;
	int aFontY = mTextOffsetY + mTranslateY;
	if (mFont)
	{
		if (mLabelJustify == ButtonWidget::BUTTON_LABEL_CENTER)
			aFontX += (mWidth - mFont->StringWidth(mLabel)) / 2;
		else if (mLabelJustify == ButtonWidget::BUTTON_LABEL_RIGHT)
			aFontX += mWidth - mFont->StringWidth(mLabel);

		aFontY += (mHeight - mFont->GetAscent() / 6 + mFont->GetAscent() - 1) / 2;
	}

	g->SetColorizeImages(true);
	if (!isDown)
	{
		g->SetColor(mColors[ButtonWidget::COLOR_BKG]);
		if (mDisabled && HaveButtonImage(mDisabledImage, mDisabledRect))
			DrawButtonImage(g, mDisabledImage, mDisabledRect, mButtonOffsetX, mButtonOffsetY);
		else if (mOverAlpha > 0.0f && HaveButtonImage(mOverImage, mOverRect))
		{
			if (HaveButtonImage(mButtonImage, mNormalRect) && mOverAlpha < 1.0f)  // 未完全过渡结束
				DrawButtonImage(g, mButtonImage, mNormalRect, mButtonOffsetX, mButtonOffsetY);

			g->mColor.mAlpha = mOverAlpha * 255;
			DrawButtonImage(g, mOverImage, mOverRect, mButtonOffsetX, mButtonOffsetY);
		}
		else if ((mIsOver || mIsDown) && HaveButtonImage(mOverImage, mOverRect))
			DrawButtonImage(g, mOverImage, mOverRect, mButtonOffsetX, mButtonOffsetY);
		else if (HaveButtonImage(mButtonImage, mNormalRect))
			DrawButtonImage(g, mButtonImage, mNormalRect, mButtonOffsetX, mButtonOffsetY);

		g->SetColorizeImages(false);
		if (mIsOver)
		{
			g->SetFont(mHiliteFont ? mHiliteFont : mFont);
			g->SetColor(mColors[ButtonWidget::COLOR_LABEL_HILITE]);
		}
		else
		{
			g->SetFont(mFont);
			g->SetColor(mColors[ButtonWidget::COLOR_LABEL]);
		}
		g->DrawString(mLabel, aFontX, aFontY);
	}
	else
	{
		g->SetColor(mColors[ButtonWidget::COLOR_BKG]);
		if (HaveButtonImage(mDownImage, mDownRect))
			DrawButtonImage(g, mDownImage, mDownRect, mButtonOffsetX + mTranslateX, mButtonOffsetY + mTranslateY);
		else if (HaveButtonImage(mOverImage, mOverRect))
			DrawButtonImage(g, mOverImage, mOverRect, mButtonOffsetX + mTranslateX, mButtonOffsetY + mTranslateY);
		else
			DrawButtonImage(g, mButtonImage, mNormalRect, mButtonOffsetX + mTranslateX, mButtonOffsetY + mTranslateY);

		g->SetColorizeImages(false);
		g->SetFont(mHiliteFont ? mHiliteFont : mFont);
		g->SetColor(mColors[ButtonWidget::COLOR_LABEL_HILITE]);
		g->DrawString(mLabel, aFontX + mTextDownOffsetX, aFontY + mTextDownOffsetY);
	}
}

// GOTY @Patoke: 0x44B7C0
bool NewLawnButton::IsPointVisible(int x, int y)
{
	if (!mUsePolygonShape)
		return DialogButton::IsPointVisible(x, y);

	return TodIsPointInPolygon(mPolygonShape, 4, SexyVector2(x, y));
}

// GOTY @Patoke: 0x44B810
NewLawnButton* MakeNewButton(int theId, ButtonListener* theListener, std::string_view theText, _Font* theFont, Image* theImageNormal, Image* theImageOver, Image* theImageDown)
{
	NewLawnButton* aButton = new NewLawnButton(nullptr, theId, theListener);
	aButton->SetFont(theFont == nullptr ? Sexy::FONT_BRIANNETOD12 : theFont);
	aButton->SetLabel(theText);

	aButton->mWidth = theImageNormal->mWidth;
	aButton->mHeight = theImageNormal->mHeight;
	aButton->mButtonImage = theImageNormal;
	aButton->mDownImage = theImageDown;
	aButton->mOverImage = theImageOver;
	aButton->mHasAlpha = true;
	aButton->mHasTransparencies = true;
	aButton->mTranslateX = 1;
	aButton->mTranslateY = 1;

	return aButton;
}

void NewLawnButton::SetOffset(int theX, int theY) {
	this->mButtonOffsetX = theX;
	this->mButtonOffsetY = theY;
}
