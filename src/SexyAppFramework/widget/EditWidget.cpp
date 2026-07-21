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

#include "EditWidget.h"
#include <algorithm>
#include "graphics/ImageFont.h"
#include "WidgetManager.h"
#include "SexyAppBase.h"
#include "EditListener.h"
#include "../../Resources.h" // bad

using namespace Sexy;

static int gEditWidgetColors[][3] = 
{{255, 255, 255},
{0, 0, 0},
{0, 0, 0},
{0, 0, 0},
{255, 255, 255}};

EditWidget::EditWidget(int theId, EditListener* theEditListener)
{		
	mId = theId;
	mEditListener = theEditListener;	
	mFont = nullptr;

	mHadDoubleClick = false;
	mHadFocusBeforePress = false;
	mHilitePos = -1;
	mLastModifyIdx = -1; // cursor position right after the last modification
	mLeftPos = 0;
	mUndoCursor = 0;
	mUndoHilitePos = 0;
	mBlinkAcc = 0;
	mCursorPos = 0;
	mShowingCursor = false;
	mDrawSelOverride = false;
	mMaxChars = -1;
	mMaxPixels = -1;
	mBlinkDelay = 40;

	SetColors(gEditWidgetColors, NUM_COLORS);
}

EditWidget::~EditWidget()
{
	delete mFont;
	ClearWidthCheckFonts();

}

void EditWidget::ClearWidthCheckFonts()
{
	for (WidthCheckList::iterator anItr = mWidthCheckList.begin(); anItr != mWidthCheckList.end(); ++anItr)
		delete anItr->mFont;

	mWidthCheckList.clear();
}

void EditWidget::AddWidthCheckFont(_Font *theFont, int theMaxPixels)
{
	mWidthCheckList.push_back(WidthCheck());
	WidthCheck &aCheck = mWidthCheckList.back();
	aCheck.mWidth = theMaxPixels;
	aCheck.mFont = theFont->Duplicate();
}

void EditWidget::SetText(const std::string& theText, bool leftPosToZero)
{
	mString = theText;
	mCursorPos = mString.length();
	mHilitePos = 0;
	if (leftPosToZero)
		mLeftPos = 0;
	else
		FocusCursor(true);
	
	MarkDirty();
}

bool EditWidget::WantsFocus()
{
	mHadFocusBeforePress = mHasFocus; // WidgetManager calls WantsFocus right before SetFocus on each press
	return true;
}

void EditWidget::Resize(int theX, int theY, int theWidth, int theHeight)
{
	Widget::Resize(theX, theY, theWidth, theHeight);

	FocusCursor(false);		
}

void EditWidget::SetFont(_Font* theFont, _Font* theWidthCheckFont)
{
	delete mFont;
	mFont = theFont->Duplicate();

	ClearWidthCheckFonts();
	if (theWidthCheckFont != nullptr)
		AddWidthCheckFont(theWidthCheckFont);
}

void EditWidget::Draw(Graphics* g) // Already translated
{	
	if (mFont == nullptr)
		mFont = FONT_PICO129->Duplicate();
		//mFont = new SysFont(mWidgetManager->mApp, "Arial Unicode MS", 10, false);

	std::string_view aString = mString;

	g->SetColor(mColors[COLOR_BKG]);			
	g->FillRect(0, 0, mWidth, mHeight);
	
	for (int i = 0; i < 2; i++)
	{
		Graphics* aClipG = g->Create();		
		aClipG->SetFont(mFont);
				
		if (i == 1)
		{
			int aCursorX = GetCaretXOffset();
			int aHiliteX = aCursorX + 2;
			if ((mHilitePos != -1) && (mCursorPos != mHilitePos))
				aHiliteX = mFont->StringWidth(aString.substr(0, mHilitePos)) - mFont->StringWidth(aString.substr(0, mLeftPos));
			
			if (!mShowingCursor)
				aCursorX += 2;								
			
			aCursorX = std::min(std::max(0, aCursorX), mWidth-8);
			aHiliteX = std::min(std::max(0, aHiliteX), mWidth-8);
			
			aClipG->ClipRect(4 + std::min(aCursorX, aHiliteX), (mHeight - mFont->GetHeight())/2, abs(aHiliteX - aCursorX), mFont->GetHeight());
		}
		else
			aClipG->ClipRect(4, 0, mWidth-8, mHeight);			
		
		bool hasfocus = mHasFocus || mDrawSelOverride;
		if (i == 1 && hasfocus)
		{
			aClipG->SetColor(mColors[COLOR_HILITE]);
			aClipG->FillRect(0, 0, mWidth, mHeight);
		}
	
		if (i == 0 || !hasfocus)
			aClipG->SetColor(mColors[COLOR_TEXT]);
		else
			aClipG->SetColor(mColors[COLOR_HILITE_TEXT]);			
		aClipG->DrawString(aString.substr(mLeftPos), 4, (mHeight - mFont->GetHeight())/2 + mFont->GetAscent());
		
		delete aClipG;
	}		
			
	g->SetColor(mColors[COLOR_OUTLINE]);
	g->DrawRect(0, 0, mWidth-1, mHeight-1);				
}

void EditWidget::UpdateCaretPos()
{
	SexyAppBase *anApp = mWidgetManager->mApp;

	Point aPoint = GetAbsPos();

	if (aPoint.mX<10) aPoint.mX = 10;
	else if (aPoint.mX>anApp->mWidth-10) aPoint.mX = anApp->mWidth-10;
	if (aPoint.mY<10) aPoint.mY = 10;
	else if (aPoint.mY>anApp->mHeight-10) aPoint.mY = anApp->mHeight-10;

	//SetCaretPos(aPoint.mX,aPoint.mY);
}

int EditWidget::GetCaretXOffset()
{
	return mFont->StringWidth(mString.substr(0, mCursorPos)) - mFont->StringWidth(mString.substr(0, mLeftPos));
}

void EditWidget::UpdateTextInputArea()
{
	if (mFont == nullptr || mWidgetManager == nullptr || !mHasFocus) // FocusCursor may fire while unattached or unfocused
		return;

	int aCursorX = std::min(std::max(0, GetCaretXOffset()), mWidth-8);

	Point anAbsPos = GetAbsPos();
	int aTextY = anAbsPos.mY + (mHeight - mFont->GetHeight())/2; // match where the caret is drawn
	mWidgetManager->mApp->SetTextInputRect(Rect(anAbsPos.mX + 4 + aCursorX, aTextY, 1, mFont->GetHeight()));
}

void EditWidget::GotFocus()
{
	Widget::GotFocus();

	mShowingCursor = true;
	mBlinkAcc = 0;
	MarkDirty();

	UpdateTextInputArea(); // set before StartTextInput so the platform anchors the IME upfront

	std::string value;
	bool wrote = mWidgetManager->mApp->StartTextInput(value);
	if (wrote)
		SetText(value);
}

void EditWidget::LostFocus()
{
	Widget::LostFocus();

	mWidgetManager->mApp->StopTextInput();
	mShowingCursor = false;	
	MarkDirty();
}

void EditWidget::Update()
{
	Widget::Update();

	if (mHasFocus)
	{
		if (mWidgetManager->mApp->mTabletPC)
		{
			UpdateCaretPos();
		}

		if (++mBlinkAcc > mBlinkDelay)
		{
			MarkDirty();
			mBlinkAcc = 0;
			mShowingCursor = !mShowingCursor;			
		}		
	}	
}

void EditWidget::EnforceMaxChars()
{
	if ((mMaxChars != -1) && (UTF8CodePointCount(mString) > (size_t)mMaxChars))
		mString = mString.substr(0, UTF8ByteOffsetForCodePoint(mString, (size_t)mMaxChars));
}

void EditWidget::EnforceMaxPixels()
{
	if (mMaxPixels<=0 && mWidthCheckList.empty()) // no width checking in effect
		return;

	if (mWidthCheckList.empty())
	{
		while (mFont->StringWidth(mString) > mMaxPixels)
			mString = mString.substr(0, UTF8PrevBoundary(mString, mString.length()));

		return;
	}

	for (WidthCheckList::iterator anItr = mWidthCheckList.begin(); anItr != mWidthCheckList.end(); ++anItr)
	{
		int aWidth = anItr->mWidth;
		if (aWidth<=0)
		{
			aWidth = mMaxPixels;
			if (aWidth<=0)
				continue;
		}

		while (anItr->mFont->StringWidth(mString) > aWidth)
			mString = mString.substr(0, UTF8PrevBoundary(mString, mString.length()));
	}
}

bool EditWidget::IsPartOfWord(char32_t theChar)
{
	return (((theChar >= U'A') && (theChar <= U'Z')) ||
			((theChar >= U'a') && (theChar <= U'z')) ||
			((theChar >= U'0') && (theChar <= U'9')) ||
			(theChar >= 0x80) ||
			(theChar == U'_'));
}

void EditWidget::ProcessKey(KeyCode theKey, char theChar)
{
	bool shiftDown = mWidgetManager->mKeyDown[KEYCODE_SHIFT];
	bool controlDown = mWidgetManager->mKeyDown[KEYCODE_CONTROL];

	if ((theKey == KEYCODE_SHIFT) || (theKey == KEYCODE_CONTROL))
		return;

	bool bigChange = false;
	bool removeHilite = !shiftDown;
	
	if (shiftDown && (mHilitePos == -1))
		mHilitePos = mCursorPos;
	
	std::string anOldString = mString;
	int anOldCursorPos = mCursorPos;
	int anOldHilitePos = mHilitePos;
	if ((theChar == 3) || (theChar == 24))
	{
		// Copy	selection

		if ((mHilitePos != -1) && (mHilitePos != mCursorPos))
		{
			int aSelStart = std::min(mCursorPos, mHilitePos);
			int aSelLen = std::max(mCursorPos, mHilitePos) - aSelStart;
			mWidgetManager->mApp->CopyToClipboard(mString.substr(aSelStart, aSelLen));

			if (theChar == 3)
			{
				removeHilite = false;
			}
			else
			{
				mString = mString.substr(0, std::min(mCursorPos, mHilitePos)) + mString.substr(std::max(mCursorPos, mHilitePos));
				mCursorPos = std::min(mCursorPos, mHilitePos);
				mHilitePos = -1;
				bigChange = true;
			}
		}
	}
	else if (theChar == 22)
	{
		// Paste selection

		std::string aBaseString = mWidgetManager->mApp->GetClipboard();

		if (aBaseString.length() > 0)
		{
			size_t aLineEnd = aBaseString.find_first_of("\r\n");
			if (aLineEnd != std::string::npos)
				aBaseString = aBaseString.substr(0, aLineEnd);

			InsertTextAtCursor(aBaseString);
			bigChange = (mString != anOldString) ||
				(mCursorPos != anOldCursorPos) ||
				(mHilitePos != anOldHilitePos);
		}
	}
	else if (theChar == 26)
	{
		// Undo
		
		mLastModifyIdx = -1;
		
		std::string aSwapString = mString;
		int aSwapCursorPos = mCursorPos;
		int aSwapHilitePos = mHilitePos;			
		
		mString = mUndoString;
		mCursorPos = mUndoCursor;
		mHilitePos = mUndoHilitePos;
					
		mUndoString = aSwapString;
		mUndoCursor = aSwapCursorPos;
		mUndoHilitePos = aSwapHilitePos;			
		
		removeHilite = false;						
	}
	else if (theKey == KEYCODE_LEFT)
	{
		if (controlDown)
		{
			// Get to a word
			while (mCursorPos > 0)
			{
				size_t aPrev = UTF8PrevBoundary(mString, mCursorPos);
				if (IsPartOfWord(UTF8CodePointAt(mString, aPrev)))
					break;
				mCursorPos = aPrev;
			}

			// Go beyond the word
			while (mCursorPos > 0)
			{
				size_t aPrev = UTF8PrevBoundary(mString, mCursorPos);
				if (!IsPartOfWord(UTF8CodePointAt(mString, aPrev)))
					break;
				mCursorPos = aPrev;
			}
		}
		else if (shiftDown || (mHilitePos == -1))
			mCursorPos = UTF8PrevBoundary(mString, mCursorPos);
		else
			mCursorPos = std::min(mCursorPos, mHilitePos);
	}
	else if (theKey == KEYCODE_RIGHT)
	{
		if (controlDown)
		{
			// Get to whitespace
			while (mCursorPos < (int)mString.length())
			{
				if (!IsPartOfWord(UTF8CodePointAt(mString, mCursorPos)))
					break;
				mCursorPos = UTF8NextBoundary(mString, mCursorPos);
			}

			// Go beyond the whitespace
			while (mCursorPos < (int)mString.length())
			{
				if (IsPartOfWord(UTF8CodePointAt(mString, mCursorPos)))
					break;
				mCursorPos = UTF8NextBoundary(mString, mCursorPos);
			}
		}
		else if (shiftDown || (mHilitePos == -1))
			mCursorPos = UTF8NextBoundary(mString, mCursorPos);
		else
			mCursorPos = std::max(mCursorPos, mHilitePos);
	}
	else if (theKey == KEYCODE_BACK)
	{
		if (mString.length() > 0)
		{
			if ((mHilitePos != -1) && (mHilitePos != mCursorPos))
			{
				// Delete selection
				mString = mString.substr(0, std::min(mCursorPos, mHilitePos)) + mString.substr(std::max(mCursorPos, mHilitePos));
				mCursorPos = std::min(mCursorPos, mHilitePos);
				mHilitePos = -1;
				
				bigChange = true;
			}
			else
			{
				// Delete char behind cursor
				if (mCursorPos > 0)
				{
					size_t aPrev = UTF8PrevBoundary(mString, mCursorPos);
					if (mCursorPos != mLastModifyIdx)
						bigChange = true;
					mString = mString.substr(0, aPrev) + mString.substr(mCursorPos);
					mCursorPos = aPrev;
					mLastModifyIdx = mCursorPos;
				}
				mHilitePos = -1;
			}
		}
	}
	else if (theKey == KEYCODE_DELETE)
	{
		if (mString.length() > 0)
		{
			if ((mHilitePos != -1) && (mHilitePos != mCursorPos))
			{
				// Delete selection
				mString = mString.substr(0, std::min(mCursorPos, mHilitePos)) + mString.substr(std::max(mCursorPos, mHilitePos));
				mCursorPos = std::min(mCursorPos, mHilitePos);
				mHilitePos = -1;
				
				bigChange = true;
			}
			else
			{
				// Delete char in front of cursor
				if (mCursorPos < (int)mString.length())
				{
					if (mCursorPos != mLastModifyIdx)
						bigChange = true;
					size_t aNext = UTF8NextBoundary(mString, mCursorPos);
					mString = mString.substr(0, mCursorPos) + mString.substr(aNext);
					mLastModifyIdx = mCursorPos;
				}
			}
		}	
	}
	else if (theKey == KEYCODE_HOME)
	{
		mCursorPos = 0;	
	}
	else if (theKey == KEYCODE_END)
	{
		mCursorPos = mString.length();	
	}
	else if (theKey == KEYCODE_RETURN)
	{
		mEditListener->EditWidgetText(mId, mString);		
	}
	else
	{
		unsigned int uTheChar = (unsigned int)theChar;
		unsigned int range = 127;
		if (gSexyAppBase->mbAllowExtendedChars)
			range = 255;

		if ((uTheChar >= 32) && (uTheChar <= range))
		{
			KeyText(std::string_view(&theChar, 1));
			if (mString == anOldString) // nothing renderable inserted
				removeHilite = false;
		}
		else
			removeHilite = false;
	}
	
	EnforceMaxChars();

	EnforceMaxPixels();

	mCursorPos = std::clamp(mCursorPos, 0, (int) mString.length());
	
	if (anOldCursorPos != mCursorPos)
	{
		mBlinkAcc = 0;
		mShowingCursor = true;
	}
	
	FocusCursor(true);
	
	if (removeHilite || mHilitePos==mCursorPos)
		mHilitePos = -1;
	
	if (bigChange)
	{
		mUndoString = anOldString;
		mUndoCursor = anOldCursorPos;
		mUndoHilitePos = anOldHilitePos;
	}
	
	MarkDirty();
}

void EditWidget::InsertTextAtCursor(std::string_view theText)
{
	if (mFont == nullptr)
		return;

	// Keep only code points the font can render.
	std::string aInsert;
	size_t aOffset = 0;
	while (aOffset < theText.size())
	{
		size_t aCharStart = aOffset;
		char32_t aChar = 0;
		if (!UTF8DecodeNext(theText, aOffset, aChar))
		{
			aOffset = aCharStart + 1; // skip the invalid byte, keep the rest
			continue;
		}
		if (aChar >= 32 && mFont->CharWidth(aChar) != 0)
			aInsert.append(theText.data() + aCharStart, aOffset - aCharStart);
	}

	if (aInsert.empty())
		return;

	if ((mHilitePos != -1) && (mHilitePos != mCursorPos))
	{
		// Replace selection with inserted text.
		mString = mString.substr(0, std::min(mCursorPos, mHilitePos)) + aInsert + mString.substr(std::max(mCursorPos, mHilitePos));
		mCursorPos = std::min(mCursorPos, mHilitePos);
	}
	else
	{
		mString = mString.substr(0, mCursorPos) + aInsert + mString.substr(mCursorPos);
	}

	mCursorPos += aInsert.size();
	mHilitePos = -1;
}

void EditWidget::KeyText(std::string_view theText)
{
	if (theText.empty())
		return;

	std::string anOldString = mString;
	int anOldCursorPos = mCursorPos;
	int anOldHilitePos = mHilitePos;
	int aPreInsertCursorPos = mCursorPos;
	bool aHadSelection = (mHilitePos != -1) && (mHilitePos != mCursorPos);

	InsertTextAtCursor(theText);

	EnforceMaxChars();

	EnforceMaxPixels();

	mCursorPos = std::clamp(mCursorPos, 0, (int)mString.length());

	if ((mString != anOldString) ||
		(mCursorPos != anOldCursorPos) ||
		(mHilitePos != anOldHilitePos))
	{
		if (aHadSelection || aPreInsertCursorPos != mLastModifyIdx)
		{
			mUndoString = anOldString;
			mUndoCursor = anOldCursorPos;
			mUndoHilitePos = anOldHilitePos;
		}
		mLastModifyIdx = mCursorPos;
		mHilitePos = -1;
		mBlinkAcc = 0;
		mShowingCursor = true;
	}
	else
	{
		mLastModifyIdx = -1;
	}

	FocusCursor(true);
	MarkDirty();
}

void EditWidget::KeyDown(KeyCode theKey)
{
	if (((theKey < 'A') || (theKey >= 'Z')))
		ProcessKey(theKey, 0);

	Widget::KeyDown(theKey);
}

void EditWidget::KeyChar(char theChar)
{
//	if (mEditListener->AllowChar(mId, theChar))
		ProcessKey(KEYCODE_UNKNOWN, theChar);

	Widget::KeyChar(theChar);
}

int EditWidget::GetCharAt(int x, int y)
{
	(void)y;
	int aPos = 0;

	std::string_view aString = mString;

	for (int i = mLeftPos; i < (int)aString.length(); )
	{
		int aNext = UTF8NextBoundary(aString, i);
		std::string_view aLoSubStr = aString.substr(mLeftPos, i - mLeftPos);
		std::string_view aHiSubStr = aString.substr(mLeftPos, aNext - mLeftPos);

		int aLoLen = mFont->StringWidth(aLoSubStr);
		int aHiLen = mFont->StringWidth(aHiSubStr);
		if (x >= (aLoLen+aHiLen)/2 + 5)
			aPos = aNext;
		i = aNext;
	}

	return aPos;
}

void EditWidget::FocusCursor(bool bigJump)
{
	while (mCursorPos < mLeftPos)
	{
		if (bigJump)
		{
			for (int i = 0; i < 10 && mLeftPos > 0; i++)
				mLeftPos = UTF8PrevBoundary(mString, mLeftPos);
		}
		else
			mLeftPos = UTF8PrevBoundary(mString, mLeftPos);
		MarkDirty();
	}

	if (mFont != nullptr)
	{
		std::string_view aString = mString;
		while ((mWidth-8 > 0) && (mFont->StringWidth(aString.substr(0, mCursorPos)) - mFont->StringWidth(aString.substr(0, mLeftPos)) >= mWidth-8))
		{
			if (bigJump)
			{
				for (int i = 0; i < 10 && mLeftPos < (int)mString.length(); i++)
					mLeftPos = UTF8NextBoundary(mString, mLeftPos);
			}
			else
				mLeftPos = UTF8NextBoundary(mString, mLeftPos);

			MarkDirty();
		}
	}

	UpdateTextInputArea();
}

void EditWidget::MouseDown(int x, int y, int theBtnNum, int theClickCount)
{
	Widget::MouseDown(x, y, theBtnNum, theClickCount);

	if (mHadFocusBeforePress) // restart the session on a repeat press; the first press goes through GotFocus
	{
		std::string value;
		bool wrote = mWidgetManager->mApp->StartTextInput(value);
		if (wrote)
			SetText(value);
	}

	mHilitePos = -1;
	mCursorPos = GetCharAt(x, y);
	
	if (theClickCount > 1)
	{
		mHadDoubleClick = true;
		HiliteWord();
	}
	
	MarkDirty();
	
	FocusCursor(false);
}

void EditWidget::MouseUp(int x, int y, int theBtnNum, int theClickCount)
{
	Widget::MouseUp(x,y,theBtnNum,theClickCount);
	if (mHilitePos==mCursorPos)
		mHilitePos = -1;
	
	if (mHadDoubleClick)
	{		
		mHilitePos = -1;
		mCursorPos = GetCharAt(x, y);		

		mHadDoubleClick = false;
		HiliteWord();
	}

	MarkDirty();
}

void EditWidget::HiliteWord()
{
	std::string_view aString = mString;

	if (mCursorPos < (int)aString.length())
	{
		// Find first space before word
		mHilitePos = mCursorPos;
		while (mHilitePos > 0)
		{
			size_t aPrev = UTF8PrevBoundary(aString, mHilitePos);
			if (!IsPartOfWord(UTF8CodePointAt(aString, aPrev)))
				break;
			mHilitePos = aPrev;
		}

		// Find first space after word
		while (mCursorPos < (int)aString.length())
		{
			if (!IsPartOfWord(UTF8CodePointAt(aString, mCursorPos)))
				break;
			mCursorPos = UTF8NextBoundary(aString, mCursorPos);
		}
	}
}

void EditWidget::MouseDrag(int x, int y)
{
	Widget::MouseDrag(x, y);

	if (mHilitePos == -1)
		mHilitePos = mCursorPos;
	
	mCursorPos = GetCharAt(x, y);
	MarkDirty();
	
	FocusCursor(false);
}

void EditWidget::MouseEnter()
{
	Widget::MouseEnter();

	mWidgetManager->mApp->SetCursor(CURSOR_TEXT);
}

void EditWidget::MouseLeave()
{	
	Widget::MouseLeave();

	mWidgetManager->mApp->SetCursor(CURSOR_POINTER);
}

void EditWidget::MarkDirty()
{
	if (mColors[COLOR_BKG].mAlpha != 255)
		Widget::MarkDirtyFull();
	else
		Widget::MarkDirty();
}
