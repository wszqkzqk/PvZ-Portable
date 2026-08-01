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

#include "DialogButton.h"
#include "graphics/Font.h"
#include "WidgetManager.h"
#include "SexyAppBase.h"

using namespace Sexy;

static int gDialogButtonColors[][3] = {
	{255, 255, 255},
	{255, 255, 255},
	{0, 0, 0},
	{255, 255, 255},
	{132, 132, 132},
	{212, 212, 212}};

DialogButton::DialogButton(Image* theComponentImage, int theId, ButtonListener* theListener) :
	ButtonWidget(theId, theListener)
{
	mComponentImage = theComponentImage;

	mTextOffsetX = mTextOffsetY = 0;
	mTranslateX = mTranslateY = 1;
	mDoFinger = true;

	SetColors(gDialogButtonColors, NUM_COLORS);
}

void DialogButton::Draw(Graphics* g)
{
	if (mBtnNoDraw)
		return;

	if (mComponentImage==nullptr)
	{
		ButtonWidget::Draw(g);
		return;
	}

	if ((mFont == nullptr) && (mLabel.length() > 0) && (mWidgetManager->mApp->mDefaultFont != nullptr))
		mFont = mWidgetManager->mApp->mDefaultFont->Duplicate();

	bool doTranslate = IsButtonDown();

	if (mNormalRect.mWidth==0)
	{
		if (doTranslate)
			g->Translate(mTranslateX, mTranslateY);

		g->DrawImageBox(Rect(0, 0, mWidth, mHeight), mComponentImage);
	}
	else
	{
		if (mDisabled && (mDisabledRect.mWidth > 0) && (mDisabledRect.mHeight > 0))
			g->DrawImageBox(mDisabledRect, Rect(0, 0, mWidth, mHeight), mComponentImage);
		else if (IsButtonDown())	
			g->DrawImageBox(mDownRect, Rect(0, 0, mWidth, mHeight), mComponentImage);
		else if ((mOverAlpha > 0))
		{
			if (mOverAlpha<1)
				g->DrawImageBox(mNormalRect, Rect(0, 0, mWidth, mHeight), mComponentImage);

			g->SetColorizeImages(true);
			g->SetColor(Color(255,255,255,(int)(mOverAlpha * 255)));
			g->DrawImageBox(mOverRect, Rect(0, 0, mWidth, mHeight), mComponentImage);
			g->SetColorizeImages(false);
		}
		else if(mIsOver)
			g->DrawImageBox(mOverRect, Rect(0, 0, mWidth, mHeight), mComponentImage);
		else
			g->DrawImageBox(mNormalRect, Rect(0, 0, mWidth, mHeight), mComponentImage);
		
		if (doTranslate)
			g->Translate(mTranslateX, mTranslateY);
	}

	if (mFont != nullptr)
	{
		g->SetFont(mFont);		

		if (mIsOver)
			g->SetColor(mColors[COLOR_LABEL_HILITE]);
		else
			g->SetColor(mColors[COLOR_LABEL]);
		
		int aFontX = (mWidth - mFont->StringWidth(mLabel))/2;
		int aFontY = (mHeight + mFont->GetAscent() - mFont->GetAscentPadding() - mFont->GetAscent()/6 - 1)/2;

		g->DrawString(mLabel, aFontX + mTextOffsetX, aFontY + mTextOffsetY);
	}

	if (doTranslate)
		g->Translate(-mTranslateX, -mTranslateY);
}
