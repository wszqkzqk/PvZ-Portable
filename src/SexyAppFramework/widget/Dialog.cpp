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

#include "Common.h"
#include "Dialog.h"
#include "DialogButton.h"
#include "SexyAppBase.h"
#include "WidgetManager.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include "graphics/Font.h"

using namespace Sexy;


std::string Sexy::DIALOG_YES_STRING				= "YES";
std::string Sexy::DIALOG_NO_STRING				= "NO";
std::string Sexy::DIALOG_OK_STRING				= "OK";
std::string Sexy::DIALOG_CANCEL_STRING			= "CANCEL";

static constexpr DialogColorScheme gDefaultDialogColors{
	.mHeader = Color(255, 255, 255),
	.mLines = Color(255, 255, 0),
	.mFooter = Color(255, 255, 255),
	.mButtonText = Color(255, 255, 255),
	.mButtonTextHilite = Color(255, 255, 255),
	.mBkg = Color(80, 80, 80),
	.mOutline = Color(255, 255, 255),
};

Dialog::Dialog(Image* theComponentImage, Image* theButtonComponentImage, int theId, bool isModal, const std::string& theDialogHeader, const std::string& theDialogLines, const std::string& theDialogFooter, int theButtonMode)
{
	mId = theId;
	mResult = 0x7FFFFFFF;
	mComponentImage = theComponentImage;
	mIsModal = isModal;
	mContentInsets = Insets(24, 24, 24, 24);
	mTextAlign = 0;
	mLineSpacingOffset = 0;
	mSpaceAfterHeader = 10;
	mButtonSidePadding = 0;
	mButtonHorzSpacing = 8;
	mDialogListener = gSexyAppBase;

	mDialogHeader = theDialogHeader;
	mDialogFooter = theDialogFooter;
	mButtonMode = theButtonMode;

	if ((mButtonMode == BUTTONS_YES_NO) || (mButtonMode == BUTTONS_OK_CANCEL))
	{
		mYesButton = std::make_unique<DialogButton>(theButtonComponentImage, ID_YES, this);
		mNoButton = std::make_unique<DialogButton>(theButtonComponentImage, ID_NO, this);

		if (mButtonMode == BUTTONS_YES_NO)
		{
			mYesButton->mLabel = DIALOG_YES_STRING;
			mNoButton->mLabel = DIALOG_NO_STRING;
		}
		else
		{
			mYesButton->mLabel = DIALOG_OK_STRING;
			mNoButton->mLabel = DIALOG_CANCEL_STRING;
		}
	}
	else if (mButtonMode == BUTTONS_FOOTER)
	{
		mYesButton = std::make_unique<DialogButton>(theButtonComponentImage, ID_FOOTER, this);
		mYesButton->mLabel = mDialogFooter;
	}
	else
	{
		mNumButtons = 0;
	}

	mDialogLines = theDialogLines;

	mButtonHeight = (theButtonComponentImage == nullptr) ? 24 : theButtonComponentImage->mHeight;

	mHasTransparencies = true;
	mHasAlpha = true;

	mDragging = false;
	mPriority = 1;

	DialogColorScheme aColors = gDefaultDialogColors;
	if (theButtonComponentImage == nullptr)
	{
		aColors.mButtonText = Color(0, 0, 0);
		aColors.mButtonTextHilite = Color(0, 0, 0);
	}

	SetColors(aColors);
}


Dialog::~Dialog() = default;

void Dialog::SetColors(const DialogColorScheme& theColors)
{
	mColors = theColors;
	MarkDirty();

	if (mYesButton != nullptr)
	{
		mYesButton->SetLabelColor(mColors.mButtonText);
		mYesButton->SetLabelHiliteColor(mColors.mButtonTextHilite);
	}
	if (mNoButton != nullptr)
	{
		mNoButton->SetLabelColor(mColors.mButtonText);
		mNoButton->SetLabelHiliteColor(mColors.mButtonTextHilite);
	}
}

void Dialog::SetHeaderColor(const Color& theColor)
{
	mColors.mHeader = theColor;
	MarkDirty();
}

void Dialog::SetLinesColor(const Color& theColor)
{
	mColors.mLines = theColor;
	MarkDirty();
}

void Dialog::SetButtonTextColor(const Color& theColor)
{
	mColors.mButtonText = theColor;
	MarkDirty();

	if (mYesButton != nullptr)
		mYesButton->SetLabelColor(theColor);
	if (mNoButton != nullptr)
		mNoButton->SetLabelColor(theColor);
}

void Dialog::SetButtonTextHiliteColor(const Color& theColor)
{
	mColors.mButtonTextHilite = theColor;
	MarkDirty();

	if (mYesButton != nullptr)
		mYesButton->SetLabelHiliteColor(theColor);
	if (mNoButton != nullptr)
		mNoButton->SetLabelHiliteColor(theColor);
}

void Dialog::SetButtonFont(_Font* theFont)
{
	if (mYesButton != nullptr)
		mYesButton->SetFont(theFont);

	if (mNoButton != nullptr)
		mNoButton->SetFont(theFont);
}

void Dialog::SetHeaderFont(_Font* theFont)
{
	mHeaderFont.reset(theFont->Duplicate());
}

void Dialog::SetLinesFont(_Font* theFont)
{
	mLinesFont.reset(theFont->Duplicate());
}

void Dialog::EnsureFonts()
{
	if (mHeaderFont == nullptr)
		mHeaderFont.reset(gSexyAppBase->mDefaultFont.load()->Duplicate());
	if (mLinesFont == nullptr)
		mLinesFont.reset(gSexyAppBase->mDefaultFont.load()->Duplicate());
}

int	Dialog::GetPreferredHeight(int theWidth)
{
	EnsureFonts();

	int aHeight = mContentInsets.mTop + mContentInsets.mBottom + mBackgroundInsets.mTop + mBackgroundInsets.mBottom;

	bool needSpace = false;
	if (mDialogHeader.length() > 0)
	{
		aHeight += mHeaderFont->GetHeight() - mHeaderFont->GetAscentPadding();
		needSpace = true;
	}

	if (mDialogLines.length() > 0)
	{
		if (needSpace)
			aHeight += mSpaceAfterHeader;
		Graphics g;
		g.SetFont(mLinesFont.get());
		aHeight += GetWordWrappedHeight(&g, theWidth-mContentInsets.mLeft-mContentInsets.mRight-mBackgroundInsets.mLeft-mBackgroundInsets.mRight-4, mDialogLines, mLinesFont->GetLineSpacing() + mLineSpacingOffset);
		needSpace = true;
	}

	if ((mDialogFooter.length() != 0) && (mButtonMode != BUTTONS_FOOTER))
	{
		if (needSpace)
			aHeight += 8;
		aHeight += mHeaderFont->GetLineSpacing();
		needSpace = true;
	}

	if (mYesButton != nullptr)
	{
		if (needSpace)
			aHeight += 8;
		aHeight += mButtonHeight + 8;
	}

	return aHeight;
}

void Dialog::Draw(Graphics* g)
{
	EnsureFonts();

	Rect aBoxRect(mBackgroundInsets.mLeft,mBackgroundInsets.mTop,mWidth-mBackgroundInsets.mLeft-mBackgroundInsets.mRight,mHeight-mBackgroundInsets.mTop-mBackgroundInsets.mBottom);
	if (mComponentImage != nullptr)
	{
		g->DrawImageBox(aBoxRect,mComponentImage);
	}
	else
	{
		g->SetColor(mColors.mOutline);
		g->DrawRect(12, 12, mWidth - 12*2 - 1, mHeight - 12*2 - 1);
		g->SetColor(mColors.mBkg);
		g->FillRect(12+1, 12+1, mWidth - 12*2 - 2, mHeight - 12*2 - 2);

		g->SetColor(Color(0, 0, 0, 128));
		g->FillRect(mWidth - 12, 12*2, 12, mHeight - 12*3);
		g->FillRect(12*2, mHeight-12, mWidth - 12*2, 12);
	}

	int aCurY = mContentInsets.mTop + mBackgroundInsets.mTop;

	if (mDialogHeader.length() > 0)
	{
		aCurY += mHeaderFont->GetAscent() - mHeaderFont->GetAscentPadding();

		g->SetFont(mHeaderFont.get());
		g->SetColor(mColors.mHeader);
		WriteCenteredLine(g, aCurY, mDialogHeader);

		aCurY += mHeaderFont->GetHeight() - mHeaderFont->GetAscent();

		aCurY += mSpaceAfterHeader;
	}

	g->SetFont(mLinesFont.get());
	g->SetColor(mColors.mLines);

	Rect aRect(mBackgroundInsets.mLeft+mContentInsets.mLeft+2, aCurY, mWidth-mContentInsets.mLeft-mContentInsets.mRight-mBackgroundInsets.mLeft-mBackgroundInsets.mRight-4, 0);
	aCurY += WriteWordWrapped(g, aRect, mDialogLines, mLinesFont->GetLineSpacing() + mLineSpacingOffset, mTextAlign);

	if ((mDialogFooter.length() != 0) && (mButtonMode != BUTTONS_FOOTER))
	{
		aCurY += 8;
		aCurY += mHeaderFont->GetLineSpacing();

		g->SetFont(mHeaderFont.get());
		g->SetColor(mColors.mFooter);
		WriteCenteredLine(g, aCurY, mDialogFooter);
	}
}

void Dialog::AddedToManager(WidgetManager* theWidgetManager)
{
	Widget::AddedToManager(theWidgetManager);

	if (mYesButton != nullptr)
		theWidgetManager->AddWidget(mYesButton.get());
	if (mNoButton != nullptr)
		theWidgetManager->AddWidget(mNoButton.get());
}

void Dialog::RemovedFromManager(WidgetManager* theWidgetManager)
{
	Widget::RemovedFromManager(theWidgetManager);

	if (mYesButton != nullptr)
		theWidgetManager->RemoveWidget(mYesButton.get());
	if (mNoButton != nullptr)
		theWidgetManager->RemoveWidget(mNoButton.get());
}

void Dialog::OrderInManagerChanged()
{
	Widget::OrderInManagerChanged();
	if (mYesButton != nullptr)
		mWidgetManager->PutInfront(mYesButton.get(),this);
	if (mNoButton != nullptr)
		mWidgetManager->PutInfront(mNoButton.get(),this);
}

void Dialog::Resize(int theX, int theY, int theWidth, int theHeight)
{
	Widget::Resize(theX, theY, theWidth, theHeight);

	if ((mYesButton != nullptr) && (mNoButton != nullptr))
	{
		int aBtnWidth = (mWidth - mContentInsets.mLeft - mContentInsets.mRight - mBackgroundInsets.mLeft - mBackgroundInsets.mRight - mButtonSidePadding*2 - mButtonHorzSpacing) / 2;
		int aBtnHeight = mButtonHeight;

		mYesButton->Resize(mX + mBackgroundInsets.mLeft + mContentInsets.mLeft + mButtonSidePadding, mY + mHeight - mContentInsets.mBottom - mBackgroundInsets.mBottom - aBtnHeight, aBtnWidth, aBtnHeight);
		mNoButton->Resize(mYesButton->mX + aBtnWidth + mButtonHorzSpacing, mYesButton->mY, aBtnWidth, aBtnHeight);
	}
	else if (mYesButton != nullptr)
	{
		int aBtnHeight = mButtonHeight;

		mYesButton->Resize(mX + mContentInsets.mLeft + mBackgroundInsets.mLeft, mY + mHeight - mContentInsets.mBottom - mBackgroundInsets.mBottom - aBtnHeight,
			mWidth - mContentInsets.mLeft - mContentInsets.mRight - mBackgroundInsets.mLeft - mBackgroundInsets.mRight, aBtnHeight);
	}
}

void Dialog::MouseDown(int x, int y, int theBtnNum, int theClickCount)
{
	if (theClickCount == 1)
	{
		mWidgetManager->mApp->SetCursor(CURSOR_DRAGGING);
		mDragging = true;
		mDragMouseX = x;
		mDragMouseY = y;
	}
	Widget::MouseDown(x,y,theBtnNum, theClickCount);
}

void Dialog::MouseDrag(int x, int y)
{
	if (mDragging)
	{
		int aNewX = mX + x - mDragMouseX;
		int aNewY = mY + y - mDragMouseY;

		if (aNewX < -8)
			aNewX = -8;
		else if (aNewX + mWidth > mWidgetManager->mWidth + 8)
			aNewX = mWidgetManager->mWidth - mWidth + 8;

		if (aNewY < -8)
			aNewY = -8;
		else if (aNewY + mHeight > mWidgetManager->mHeight + 8)
			aNewY = mWidgetManager->mHeight- mHeight + 8;

		mDragMouseX = mX + x - aNewX;
		mDragMouseY = mY + y - aNewY;

		if (mDragMouseX < 8)
			mDragMouseX = 8;
		else if (mDragMouseX > mWidth-9)
			mDragMouseX = mWidth-9;

		if (mDragMouseY < 8)
			mDragMouseY = 8;
		else if (mDragMouseY > mHeight-9)
			mDragMouseY = mHeight-9;

		Move(aNewX, aNewY);
	}
}

void Dialog::MouseUp(int x, int y, int theBtnNum, int theClickCount)
{
	if (mDragging)
	{
		mWidgetManager->mApp->SetCursor(CURSOR_POINTER);
		mDragging = false;
	}
	Widget::MouseUp(x,y, theBtnNum, theClickCount);
}

void Dialog::Update()
{
	Widget::Update();

	//Move(mX, mY+1);
}

bool Dialog::IsModal()
{
	return mIsModal;
}

int Dialog::WaitForResult(bool autoKill)
{
#ifdef __EMSCRIPTEN__
	const auto isWaitingForResult = [this]() {
		return mWidgetManager != nullptr && mResult == 0x7FFFFFFF;
	};

	while (isWaitingForResult())
	{
		bool updated = false;
		if (!gSexyAppBase->UpdateAppStep(&updated))
			break;

		// While still waiting, complete pending stages in one rAF to avoid FPS drops and input lag.
		while (isWaitingForResult() &&
			(gSexyAppBase->mUpdateAppState != UPDATESTATE_PROCESS_DONE || gSexyAppBase->mHasPendingDraw))
		{
			if (!gSexyAppBase->UpdateAppStep(&updated))
				break;
		}

		if (isWaitingForResult())
			emscripten_sleep(0);
	}
#else
	while ((gSexyAppBase->UpdateAppStep(nullptr)) && (mWidgetManager != nullptr) && (mResult == 0x7FFFFFFF));
#endif

	if (autoKill)
		gSexyAppBase->KillDialog(mId);

	return mResult;
}

void Dialog::ButtonPress(int theId)
{
	if ((theId == ID_YES) || (theId == ID_NO))
		mDialogListener->DialogButtonPress(mId, theId);
}

void Dialog::ButtonDepress(int theId)
{
	if ((theId == ID_YES) || (theId == ID_NO))
	{
		mResult = theId;
		mDialogListener->DialogButtonDepress(mId, theId);
	}
}
