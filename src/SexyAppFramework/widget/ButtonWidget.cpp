/*
 * Portions of this file are based on the PopCap Games Framework
 * Copyright (C) 2005-2009 PopCap Games, Inc.
 * 
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
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

#include "ButtonWidget.h"
#include "graphics/Image.h"
#include "graphics/Font.h"
#include "WidgetManager.h"
#include "ButtonListener.h"
#include "SexyAppBase.h"

using namespace Sexy;

static int gButtonWidgetColors[][3] = {
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{255, 255, 255},
	{132, 132, 132},
	{212, 212, 212}};

ButtonWidget::ButtonWidget(int theId, ButtonListener* theButtonListener)	
{
	mId = theId;
	mFont = nullptr;
	mLabelJustify = BUTTON_LABEL_CENTER;
	mButtonImage = nullptr;
	mOverImage = nullptr;
	mDownImage = nullptr;
	mDisabledImage = nullptr;
	mInverted = false;
	mBtnNoDraw = false;
	mFrameNoDraw = false;
	mButtonListener = theButtonListener;
	mHasAlpha = true;

	mOverAlpha = 0;
	mOverAlphaSpeed = 0;
	mOverAlphaFadeInSpeed = 0;

	SetColors(gButtonWidgetColors, NUM_COLORS);
}

ButtonWidget::~ButtonWidget()
{
	delete mFont;
}

void ButtonWidget::SetFont(_Font* theFont)
{
	delete mFont;
	mFont = theFont->Duplicate();
}

bool ButtonWidget::IsButtonDown()
{
	return mIsDown && mIsOver && !mDisabled;
}

bool ButtonWidget::HaveButtonImage(Image *theImage, const Rect &theRect)
{
	return (theImage!=nullptr || theRect.mWidth!=0);
}
	
void ButtonWidget::DrawButtonImage(Graphics *g, Image *theImage, const Rect &theRect, int x, int y)
{
	if (theRect.mWidth != 0)
		g->DrawImage(mButtonImage,x,y,theRect);
	else
		g->DrawImage(theImage,x,y);
}

void ButtonWidget::Draw(Graphics* g)
{
	if (mBtnNoDraw)
		return;

	if ((mFont == nullptr) && (mLabel.length() > 0) && (mWidgetManager->mApp->mDefaultFont != nullptr))
		mFont = mWidgetManager->mApp->mDefaultFont->Duplicate();

	bool isDown = mIsDown && mIsOver && !mDisabled;
	isDown ^= mInverted;

	int aFontX = 0; // BUTTON_LABEL_LEFT
	int aFontY = 0;
	
	if (mFont != nullptr)
	{
		if (mLabelJustify == BUTTON_LABEL_CENTER)
			aFontX = (mWidth - mFont->StringWidth(mLabel))/2;
		else if (mLabelJustify == BUTTON_LABEL_RIGHT)
			aFontX = mWidth - mFont->StringWidth(mLabel);
		aFontY = (mHeight + mFont->GetAscent() - mFont->GetAscent()/6 - 1)/2;

		//aFontX = (mWidth - mFont->StringWidth(mLabel))/2;
		//aFontY = (mHeight - mFont->GetHeight())/2 + mFont->GetAscent() - 1;		
	}

	g->SetFont(mFont);
	
	if ((mButtonImage == nullptr) && (mDownImage == nullptr))
	{
		if (!mFrameNoDraw)
		{
			g->SetColor(mColors[COLOR_BKG]);
			g->FillRect(0, 0, mWidth, mHeight);
		}

		if (isDown)
		{
			if (!mFrameNoDraw)
			{
				g->SetColor(mColors[COLOR_DARK_OUTLINE]);
				g->FillRect(0, 0, mWidth-1, 1);
				g->FillRect(0, 0, 1, mHeight-1);
				
				g->SetColor(mColors[COLOR_LIGHT_OUTLINE]);
				g->FillRect(0, mHeight - 1, mWidth, 1);
				g->FillRect(mWidth - 1, 0, 1, mHeight);									
		
				g->SetColor(mColors[COLOR_MEDIUM_OUTLINE]);
				g->FillRect(1, 1, mWidth - 3, 1);
				g->FillRect(1, 1, 1, mHeight - 3);
			}

			if (mIsOver)
				g->SetColor(mColors[COLOR_LABEL_HILITE]);
			else
				g->SetColor(mColors[COLOR_LABEL]);

			g->DrawString(mLabel, aFontX+1, aFontY+1);
		}
		else
		{			
			if (!mFrameNoDraw)
			{
				g->SetColor(mColors[COLOR_LIGHT_OUTLINE]);
				g->FillRect(0, 0, mWidth-1, 1);
				g->FillRect(0, 0, 1, mHeight-1);
				
				g->SetColor(mColors[COLOR_DARK_OUTLINE]);
				g->FillRect(0, mHeight - 1, mWidth, 1);
				g->FillRect(mWidth - 1, 0, 1, mHeight);									
		
				g->SetColor(mColors[COLOR_MEDIUM_OUTLINE]);
				g->FillRect(1, mHeight - 2, mWidth - 2, 1);
				g->FillRect(mWidth - 2, 1, 1, mHeight - 2);			
			}
			
			if (mIsOver)
				g->SetColor(mColors[COLOR_LABEL_HILITE]);
			else
				g->SetColor(mColors[COLOR_LABEL]);

			g->DrawString(mLabel, aFontX, aFontY);		
		}		
	}
	else
	{
		if (!isDown)
		{
			if (mDisabled && HaveButtonImage(mDisabledImage,mDisabledRect))
				DrawButtonImage(g,mDisabledImage,mDisabledRect,0,0);
			else if ((mOverAlpha > 0) && HaveButtonImage(mOverImage,mOverRect))
			{
				if (HaveButtonImage(mButtonImage, mNormalRect)  && mOverAlpha<1)
					DrawButtonImage(g,mButtonImage,mNormalRect,0,0);

				g->SetColorizeImages(true);
				g->SetColor(Color(255,255,255,(int)(mOverAlpha * 255)));
				DrawButtonImage(g,mOverImage,mOverRect,0,0);
				g->SetColorizeImages(false);
			}
			else if ((mIsOver || mIsDown) && HaveButtonImage(mOverImage,mOverRect))
			{
				DrawButtonImage(g,mOverImage,mOverRect,0,0);
			}
			else if (HaveButtonImage(mButtonImage,mNormalRect))
				DrawButtonImage(g,mButtonImage,mNormalRect,0,0);

			if (mIsOver)
				g->SetColor(mColors[COLOR_LABEL_HILITE]);
			else
				g->SetColor(mColors[COLOR_LABEL]);
			g->DrawString(mLabel, aFontX, aFontY);
		}
		else
		{
			if (HaveButtonImage(mDownImage, mDownRect))
				DrawButtonImage(g, mDownImage, mDownRect, 0, 0);
			else if (HaveButtonImage(mOverImage,mOverRect))
				DrawButtonImage(g, mOverImage, mOverRect, 1, 1);
			else
				DrawButtonImage(g, mButtonImage, mNormalRect, 1, 1);

			g->SetColor(mColors[COLOR_LABEL_HILITE]);
			g->DrawString(mLabel, aFontX+1, aFontY+1);
		}
	}
}

void ButtonWidget::SetDisabled(bool isDisabled)
{
	Widget::SetDisabled(isDisabled);
	
	if (HaveButtonImage(mDisabledImage,mDisabledRect))
		MarkDirty();
}

void ButtonWidget::MouseEnter()
{
	Widget::MouseEnter();

	if (mOverAlphaFadeInSpeed==0 && mOverAlpha>0)
		mOverAlpha = 0;
	
	if (mIsDown || (HaveButtonImage(mOverImage,mOverRect)) || (mColors[COLOR_LABEL_HILITE] != mColors[COLOR_LABEL]))
		MarkDirty();
	
	mButtonListener->ButtonMouseEnter(mId);
}

void ButtonWidget::MouseLeave()
{
	Widget::MouseLeave();

	if (mOverAlphaSpeed==0 && mOverAlpha>0)
		mOverAlpha = 0;
	else if (mOverAlphaSpeed>0 && mOverAlpha==0) // fade out from full
		mOverAlpha = 1;

	if (mIsDown || HaveButtonImage(mOverImage,mOverRect) || (mColors[COLOR_LABEL_HILITE] != mColors[COLOR_LABEL]))
		MarkDirty();
	
	mButtonListener->ButtonMouseLeave(mId);
}

void ButtonWidget::MouseMove(int theX, int theY)
{
	Widget::MouseMove(theX, theY);
	
	mButtonListener->ButtonMouseMove(mId, theX, theY);
}

void ButtonWidget::MouseDown(int theX, int theY, int theBtnNum, int theClickCount)
{
	Widget::MouseDown(theX, theY, theBtnNum, theClickCount);
		
	mButtonListener->ButtonPress(mId);
	
	MarkDirty();
}

void ButtonWidget::MouseUp(int theX, int theY, int theBtnNum, int theClickCount)
{	
	Widget::MouseUp(theX, theY, theBtnNum, theClickCount);

	if (mIsOver && mWidgetManager->mHasFocus)
		mButtonListener->ButtonDepress(mId);
	
	MarkDirty();
}

void ButtonWidget::Update()
{
	Widget::Update();

	if (mIsDown && mIsOver)
		mButtonListener->ButtonDownTick(mId);

	if (!mIsDown && !mIsOver && (mOverAlpha > 0))
	{
		if (mOverAlphaSpeed>0)
		{
			mOverAlpha -= mOverAlphaSpeed;
			if (mOverAlpha < 0)
				mOverAlpha = 0;
		}
		else
			mOverAlpha = 0;

		MarkDirty();
	}
	else if (mIsOver && mOverAlphaFadeInSpeed>0 && mOverAlpha<1)
	{
		mOverAlpha += mOverAlphaFadeInSpeed;
		if (mOverAlpha > 1)
			mOverAlpha = 1;
		MarkDirty();
	}
}
