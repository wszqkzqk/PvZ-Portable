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

#include "Slider.h"
#include "graphics/Graphics.h"
#include "graphics/Image.h"
#include "SliderListener.h"
#include "WidgetManager.h"
#include "SexyAppBase.h"

using namespace Sexy;

Slider::Slider(Image* theTrackImage, Image* theThumbImage, int theId, SliderListener* theListener) :
	mListener(theListener),
	mVal(0.0),
	mId(theId),
	mTrackImage(theTrackImage),
	mThumbImage(theThumbImage)
{
	mDragging = false;
	mHorizontal = true;
	mRelX = mRelY = 0;
}

void Slider::SetValue(double theValue)
{
	mVal = theValue;
	if (mVal < 0.0)
		mVal = 0.0;
	else if (mVal > 1.0)
		mVal = 1.0;

	MarkDirtyFull();
}

bool Slider::HasTransparencies()
{
	return true;
}

void Slider::Draw(Graphics* g)
{
	if (mTrackImage != nullptr)
	{
		int cw = mHorizontal ? mTrackImage->GetWidth()/3 : mTrackImage->GetWidth();
		int ch = mHorizontal ? mTrackImage->GetHeight() : mTrackImage->GetHeight()/3;

		if (mHorizontal)
		{
			int ty = (mHeight - ch) / 2;

			g->DrawImage(mTrackImage, 0, ty, Rect(0, 0, cw, ch));

			Graphics aClipG(*g);
			aClipG.ClipRect(cw, ty, mWidth - cw*2, ch);
			for (int i = 0; i < (mWidth-cw*2+cw-1)/cw; i++)
				aClipG.DrawImage(mTrackImage, cw + i*cw, ty, Rect(cw, 0, cw, ch));

			g->DrawImage(mTrackImage, mWidth-cw, ty, Rect(cw*2, 0, cw, ch));
		}
		else
		{
			g->DrawImage(mTrackImage, 0, 0, Rect(0, 0, cw, ch));
			Graphics aClipG(*g);
			aClipG.ClipRect(0, ch, cw, mHeight - ch * 2);
			for (int i = 0; i < (mHeight-ch*2+ch-1)/ch; i++)
				aClipG.DrawImage(mTrackImage, 0, ch + i*ch, Rect(0, ch, cw, ch));

			g->DrawImage(mTrackImage, 0, mHeight-ch, Rect(0, ch*2, cw, ch));
		}
	}

	if (mHorizontal && (mThumbImage != nullptr))
		g->DrawImage(mThumbImage, (int) (mVal * (mWidth - mThumbImage->GetWidth())), (mHeight - mThumbImage->GetHeight()) / 2);
	else if (!mHorizontal && (mThumbImage != nullptr))
		g->DrawImage(mThumbImage, (mWidth - mThumbImage->GetWidth()) / 2, (int) (mVal * (mHeight - mThumbImage->GetHeight())));

	//g->SetColor(Color(255, 255, 0));
	//g->FillRect(0, 0, mWidth, mHeight);
}

void Slider::MouseDown(int x, int y, [[maybe_unused]] int theClickCount)
{
	if (mHorizontal)
	{
		int aThumbX = (int) (mVal * (mWidth - mThumbImage->GetWidth()));

		if ((x >= aThumbX) && (x < aThumbX + mThumbImage->GetWidth()))
		{
			mWidgetManager->mApp->SetCursor(CURSOR_DRAGGING);
			mDragging = true;
			mRelX = x - aThumbX;
		}
		else
		{
			// clicked on the bar, set position to mouse click
			double pos = (double)x / mWidth;
			SetValue(pos);
		}
	}
	else
	{
		int aThumbY = (int) (mVal * (mHeight - mThumbImage->GetHeight()));

		if ((y >= aThumbY) && (y < aThumbY + mThumbImage->GetHeight()))
		{
			mWidgetManager->mApp->SetCursor(CURSOR_DRAGGING);
			mDragging = true;
			mRelY = y - aThumbY;
		}
		else
		{
			// clicked on the bar, set position to mouse click
			double pos = (double)y / mHeight;
			SetValue(pos);
		}
	}
}

void Slider::MouseMove(int x, int y)
{
	if (mHorizontal)
	{
		int aThumbX = (int) (mVal * (mWidth - mThumbImage->GetWidth()));

		if ((x >= aThumbX) && (x < aThumbX + mThumbImage->GetWidth()))
			mWidgetManager->mApp->SetCursor(CURSOR_DRAGGING);
		else
			mWidgetManager->mApp->SetCursor(CURSOR_POINTER);
	}
	else
	{
		int aThumbY = (int) (mVal * (mHeight - mThumbImage->GetHeight()));

		if ((y >= aThumbY) && (y < aThumbY + mThumbImage->GetHeight()))
			mWidgetManager->mApp->SetCursor(CURSOR_DRAGGING);
		else
			mWidgetManager->mApp->SetCursor(CURSOR_POINTER);
	}
}

void Slider::MouseDrag(int x, int y)
{
	if (mDragging)
	{
		double anOldVal = mVal;

		if (mHorizontal)
			mVal = (x - mRelX) / (double) (mWidth - mThumbImage->GetWidth());
		else
			mVal = (y - mRelY) / (double) (mHeight - mThumbImage->GetHeight());

		if (mVal < 0.0)
			mVal = 0.0;
		if (mVal > 1.0)
			mVal = 1.0;

		if (mVal != anOldVal)
		{
			mListener->SliderVal(mId, mVal);
			MarkDirtyFull();
		}
	}
}

void Slider::MouseUp([[maybe_unused]] int x, [[maybe_unused]] int y)
{
	mDragging = false;
	mWidgetManager->mApp->SetCursor(CURSOR_POINTER);
	mListener->SliderVal(mId, mVal);
}

void Slider::MouseLeave()
{
	if (!mDragging)
		mWidgetManager->mApp->SetCursor(CURSOR_POINTER);
}
