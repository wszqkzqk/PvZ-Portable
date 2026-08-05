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

#include "ScrollbarWidget.h"
#include "WidgetManager.h"
#include "ScrollListener.h"
#include "ScrollbuttonWidget.h"

using namespace Sexy;

ScrollbarWidget::ScrollbarWidget(int theId, ScrollListener* theScrollListener)
{
	mId = theId;
	mScrollListener = theScrollListener;

	SetDisabled(true);

	mUpButton = new ScrollbuttonWidget(0, this);
	mUpButton->SetDisabled(true);

	mDownButton = new ScrollbuttonWidget(1, this);
	mDownButton->SetDisabled(true);

	mInvisIfNoScroll = false;
	mPressedOnThumb = false;
	mValue = 0;
	mMaxValue = 0;
	mPageSize = 0;
	mUpdateAcc = 0;
	mButtonAcc = 0;
	mUpdateMode = 0;
	mHorizontal = false;

	AddWidget(mUpButton);
	AddWidget(mDownButton);
}

ScrollbarWidget::~ScrollbarWidget()
{
	RemoveAllWidgets(true);
}

void ScrollbarWidget::SetInvisIfNoScroll(bool invisIfNoScroll)
{
	mInvisIfNoScroll = invisIfNoScroll;

	if (mInvisIfNoScroll)
	{
		SetVisible(false);
		mDownButton->SetVisible(false);
		mUpButton->SetVisible(false);
	}
}

void ScrollbarWidget::SetHorizontal(bool isHorizontal)
{
	mHorizontal = isHorizontal;
	mDownButton->mHorizontal = mHorizontal;
	mUpButton->mHorizontal = mHorizontal;
}


void ScrollbarWidget::ResizeScrollbar(int theX, int theY, int theWidth, int theHeight)
{
	Resize(theX, theY, theWidth, theHeight);
	if (mHorizontal)
	{
		theX = theY = 0; // Use New Container Stuff
		mUpButton->Resize(theX, theY, theHeight, theHeight);
		mDownButton->Resize(theX + theWidth - theHeight, theY, theHeight, theHeight);
	}
	else
	{
		theX = theY = 0; // Use New Container Stuff
		mUpButton->Resize(theX, theY, theWidth, theWidth);
		mDownButton->Resize(theX, theY + theHeight - theWidth, theWidth, theWidth);
	}
}

void ScrollbarWidget::SetMaxValue(double theNewMaxValue)
{
	mMaxValue = theNewMaxValue;
	ClampValue();
	MarkDirty();
}

void ScrollbarWidget::SetPageSize(double theNewPageSize)
{
	mPageSize = theNewPageSize;
	ClampValue();
	MarkDirty();
}

void ScrollbarWidget::SetValue(double theNewValue)
{
	mValue = theNewValue;
	ClampValue();
	mScrollListener->ScrollPosition(mId, mValue);
	MarkDirty();
}

bool ScrollbarWidget::AtBottom()
{
	return mMaxValue - mPageSize - mValue <= 1.0;
}

void ScrollbarWidget::GoToBottom()
{
	mValue = mMaxValue - mPageSize;
	ClampValue();
	SetValue(mValue);
}

void ScrollbarWidget::DrawThumb(Graphics *g, int theX, int theY, int theWidth, int theHeight)
{
	g->SetColor(Color(212, 212, 212));
	g->FillRect(theX, theY, theWidth, theHeight);

	g->SetColor(Color(255, 255, 255));
	g->FillRect(theX + 1, theY + 1, theWidth-2, 1);
	g->FillRect(theX + 1, theY + 1, 1, theHeight-2);

	g->SetColor(Color::Black);
	g->FillRect(theX, theY + theHeight - 1, theWidth, 1);
	g->FillRect(theX + theWidth - 1, theY, 1, theHeight);

	g->SetColor(Color(132, 132, 132));
	g->FillRect(theX + 1, theY + theHeight - 2, theWidth - 2, 1);
	g->FillRect(theX + theWidth - 2, theY + 1, 1, theHeight - 2);
}

int ScrollbarWidget::GetTrackSize()
{
	if (mHorizontal)
		return mWidth-2*mUpButton->mWidth;
	else
		return mHeight-2*mUpButton->mWidth;
}

int ScrollbarWidget::GetThumbSize()
{
	if (mPageSize > mMaxValue)
		return 0;

	int aThumbSize = (int) ((double) ((GetTrackSize() * mPageSize / mMaxValue)) + 0.5);
	return std::max(8, aThumbSize);
}

int ScrollbarWidget::GetThumbPosition()
{
	if (mPageSize > mMaxValue) return mUpButton->mWidth;
	return (int) ((double) (((mValue * (GetTrackSize() - GetThumbSize())) / (mMaxValue - mPageSize))) + 0.5) + mUpButton->mWidth;
}

void ScrollbarWidget::Draw(Graphics *g)
{
	int aThumbSize = GetThumbSize();
	int aThumbPosition = GetThumbPosition();

	if (mHorizontal)
	{
		if (mUpdateMode == UPDATE_MODE_PGUP)
			g->SetColor(Color(48, 48, 48));
		else
			g->SetColor(Color(232, 232, 232));
		g->FillRect(0, 0, aThumbPosition, mHeight);

		if (aThumbSize > 0)
			DrawThumb(g, aThumbPosition, 0, aThumbSize, mHeight);

		if (mUpdateMode == UPDATE_MODE_PGDN)
			g->SetColor(Color(48, 48, 48));
		else
			g->SetColor(Color(232, 232, 232));
		g->FillRect(aThumbPosition + aThumbSize, 0, mWidth - aThumbPosition - aThumbSize, mHeight);
	}
	else
	{
		if (mUpdateMode == UPDATE_MODE_PGUP)
			g->SetColor(Color(48, 48, 48));
		else
			g->SetColor(Color(232, 232, 232));
		g->FillRect(0, 0, mWidth, aThumbPosition);

		if (aThumbSize > 0)
			DrawThumb(g, 0, aThumbPosition, mWidth, aThumbSize);

		if (mUpdateMode == UPDATE_MODE_PGDN)
			g->SetColor(Color(48, 48, 48));
		else
			g->SetColor(Color(232, 232, 232));
		g->FillRect(0, aThumbPosition + aThumbSize, mWidth, mHeight - aThumbPosition - aThumbSize);
	}
}

void ScrollbarWidget::ClampValue()
{
	double anOldValue = mValue;

	if (mValue > mMaxValue - mPageSize)
		mValue = mMaxValue - mPageSize;
	if (mValue < 0.0)
		mValue = 0.0;

	bool canScroll = mPageSize < mMaxValue;

	SetDisabled(!canScroll);
	mUpButton->SetDisabled(!canScroll);
	mDownButton->SetDisabled(!canScroll);

	if (mInvisIfNoScroll)
	{
		SetVisible(canScroll);
		mDownButton->SetVisible(canScroll);
		mUpButton->SetVisible(canScroll);
	}

	if (mValue != anOldValue) mScrollListener->ScrollPosition(mId, mValue);
}

void ScrollbarWidget::SetThumbPosition(int thePosition)
{
	SetValue(((thePosition-mUpButton->mWidth) * (mMaxValue - mPageSize)) / (GetTrackSize() - GetThumbSize()));
}

void ScrollbarWidget::ButtonPress(int theId)
{
	mButtonAcc = 0;

	if (theId == 0)
		SetValue(mValue-1);
	else
		SetValue(mValue+1);
}

void ScrollbarWidget::ButtonDepress(int){}

void ScrollbarWidget::ButtonDownTick(int theId)
{
	if (theId == 0)
	{
		if (++mButtonAcc >= 25)
		{
			SetValue(mValue-1);
			mButtonAcc = 24;
		}
	}
	else
	{
		if (++mButtonAcc >= 25)
		{
			SetValue(mValue+1);
			mButtonAcc = 24;
		}
	}
}

void ScrollbarWidget::Update()
{
	Widget::Update();

	switch (mUpdateMode)
	{
	case UPDATE_MODE_PGUP:
		if (ThumbCompare(mLastMouseX, mLastMouseY) != -1)
		{
			mUpdateMode = UPDATE_MODE_IDLE;
			MarkDirty();
		}
		else if (++mUpdateAcc >= 25)
		{
			SetValue(mValue - mPageSize);
			mUpdateAcc = 20;
		}
		break;
	case UPDATE_MODE_PGDN:
		if (ThumbCompare(mLastMouseX, mLastMouseY) != 1)
		{
			mUpdateMode = UPDATE_MODE_IDLE;
			MarkDirty();
		}
		else if (++mUpdateAcc >= 25)
		{
			SetValue(mValue + mPageSize);
			mUpdateAcc = 20;
		}
		break;
	}
}

int ScrollbarWidget::ThumbCompare(int x, int y)
{
	int aTrackPosition;

	if (mHorizontal)
		aTrackPosition = x;
	else
		aTrackPosition = y;

	if (aTrackPosition < GetThumbPosition())
		return -1;

	if (aTrackPosition >= GetThumbPosition() + GetThumbSize())
		return 1;

	return 0;
}

void ScrollbarWidget::MouseDown(int x, int y, int theBtnNum, int theClickCount)
{
	Widget::MouseDown(x, y, theBtnNum, theClickCount);

	if (!mDisabled)
	{
		switch (ThumbCompare(x, y))
		{
		case -1:
			SetValue(mValue - mPageSize);
			mUpdateMode = UPDATE_MODE_PGUP;
			mUpdateAcc = 0;
			break;
		case 0:
			mPressedOnThumb = true;
			mMouseDownThumbPos = GetThumbPosition();
			mMouseDownX = x;
			mMouseDownY = y;
			break;
		case 1:
			SetValue(mValue + mPageSize);
			mUpdateMode = UPDATE_MODE_PGDN;
			mUpdateAcc = 0;
			break;
		}
	}

	mLastMouseX = x;
	mLastMouseY = y;
}

void ScrollbarWidget::MouseUp(int x, int y, int theBtnNum, int theClickCount)
{
	Widget::MouseUp(x, y, theBtnNum, theClickCount);

	mUpdateMode = 0;
	mPressedOnThumb = false;
	MarkDirty();
}

void ScrollbarWidget::MouseDrag(int x, int y)
{
	Widget::MouseDrag(x, y);

	if (mPressedOnThumb)
	{
		if (mHorizontal)
			SetThumbPosition(mMouseDownThumbPos + x - mMouseDownX);
		else
			SetThumbPosition(mMouseDownThumbPos + y - mMouseDownY);
	}

	mLastMouseX = x;
	mLastMouseY = y;
}
