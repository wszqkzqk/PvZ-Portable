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

#include "ListWidget.h"
#include <utility>
#include "graphics/Font.h"
#include "WidgetManager.h"
#include "ScrollbarWidget.h"
#include "ListListener.h"
#include "SexyAppBase.h"

using namespace Sexy;

static int gInitialListWidgetColors[][3] = {{255, 255, 255}, {255, 255, 255}, {0, 0, 0}, {0, 192, 0}, {0, 0, 128}, {255, 255, 255}};

ListWidget::ListWidget(int theId, _Font *theFont, ListListener *theListListener)
{
	mJustify = JUSTIFY_LEFT;
	mHiliteIdx = -1;
	mSelectIdx = -1;

	if (theFont != nullptr)
		mItemHeight = theFont->GetHeight();
	else
		mItemHeight = -1;

	SetColors(gInitialListWidgetColors, 6);
		
	mId = theId;
	mFont = theFont;
	mListListener = theListListener;
	mParent = nullptr;
	mChild = nullptr;
	mScrollbar = nullptr;
	mPosition = 0;
	mPageSize = 0;
	mSortFromChild = false;
	mDrawOutline = true;
	mMaxNumericPlaces = 0;	
	mDrawSelectWhenHilited = false;
	mDoFingerWhenHilited = true;
}
	
ListWidget::~ListWidget() = default;

void ListWidget::RemovedFromManager(WidgetManager *theManager)
{
	Widget::RemovedFromManager(theManager);
	if (mListListener)
		mListListener->ListClosed(mId);
}

std::string ListWidget::GetSortKey(int theIdx)
{
	std::string aString = mLines[theIdx];
	
	while (aString.length() < (ulong) mMaxNumericPlaces) 
		aString = "0" + aString;

	if (mSortFromChild) 
		return mChild->GetSortKey(theIdx) + aString;
	else 
	{
		if (mChild == nullptr)	
			return aString;
		else 
			return aString + mChild->GetSortKey(theIdx);
	}

	return "";
}
	
void ListWidget::Sort(bool ascending) 
{
	int aCount = mLines.size();
	std::vector<int> aMap(aCount);
	std::vector<std::string> aKeys(aCount);

	int i;
	for (i = 0; i < aCount; i++) 
	{
		aMap[i] = i;
		aKeys[i] = GetSortKey(i);		
	}
		
	for (i = 1; i < aCount; i++) 
		for (int j = 0; j < aCount - i; j++) 
		{
			int aComp = aKeys[j].compare(aKeys[j+1]);
			if ((ascending && (aComp > 0)) || (!ascending && (aComp < 0)))
			{
				std::swap(aMap[j], aMap[j + 1]);
				std::swap(aKeys[j], aKeys[j + 1]);
			}
		}
		
	ListWidget *aListWidget = this;
	while (aListWidget->mParent != nullptr)	
		aListWidget = aListWidget->mParent;

	while (aListWidget != nullptr) 
	{
		std::vector<std::string> aNewLines;
		std::vector<Color> aNewLineColors;
			
		for (int i = 0; i < aCount; i++)
		{
			aNewLines.push_back(aListWidget->mLines[aMap[i]]);
			aNewLineColors.push_back(aListWidget->mLineColors[aMap[i]]);
		}
		
		aListWidget->mLines = aNewLines;
		aListWidget->mLineColors = aNewLineColors;
		
		aListWidget->MarkDirty();
						
		aListWidget = aListWidget->mChild;
	}
}
	
std::string ListWidget::GetStringAt(int theIdx)
{
	return
		mLines[theIdx];
}
	
void ListWidget::Resize(int theX, int theY, int theWidth, int theHeight)
{
	Widget::Resize(theX, theY, theWidth, theHeight);
	
	double aPageSize = 1;			
	int anItemHeight = (mItemHeight != -1) ? mItemHeight : mFont->GetHeight();
	
	if (mHeight > anItemHeight+8) 
		aPageSize = (mHeight - 8.0) / anItemHeight;
		
	mPageSize = aPageSize;
		
	if (mScrollbar != nullptr) 
		mScrollbar->SetPageSize(aPageSize);
}
	
int ListWidget::AddLine(const std::string& theLine, bool alphabetical)
{	
	int anIdx = -1;
	bool inserted = false;

	if (alphabetical) 
	{	
		for (int i = 0;	i < (int) mLines.size(); i++) 		
			if (strcmp(theLine.c_str(), mLines[i].c_str()) < 0) 
			{
				anIdx = i;
						
				ListWidget *aListWidget = this;

				while (aListWidget->mParent != nullptr) 
					aListWidget = aListWidget->mParent;

				while (aListWidget != nullptr) 
				{
					if (aListWidget == this)
						aListWidget->mLines.insert(aListWidget->mLines.begin() + i, theLine);
					else 
						aListWidget->mLines.insert(aListWidget->mLines.begin() + i, "-");
					
					aListWidget->mLineColors.insert(aListWidget->mLineColors.begin() + i, mColors[COLOR_TEXT]);
					aListWidget->MarkDirty();
					
					aListWidget = aListWidget->mChild;
				}
						
				inserted = true;
				break;
			}
	}
		
	if (!inserted) 
	{
		anIdx = mLines.size();
			
		ListWidget *aListWidget = this;
		
		while (aListWidget->mParent!=nullptr) 
			aListWidget = aListWidget->mParent;

		while (aListWidget!=nullptr) 
		{
			if (aListWidget==this) 
				aListWidget->mLines.push_back(theLine);
			else 
				aListWidget->mLines.push_back("-");
						
			aListWidget->mLineColors.push_back(mColors[COLOR_TEXT]);
			aListWidget->MarkDirty();
				
			aListWidget = aListWidget->mChild;
		}
	}
		
	if (mScrollbar!=nullptr)
		mScrollbar->SetMaxValue(mLines.size());		
		
	return anIdx;
}

	
void ListWidget::SetLine(int theIdx, const std::string& theString)
{
	mLines[theIdx] = theString;	
	MarkDirty();
}
	
int ListWidget::GetLineCount()
{
	return mLines.size();	
}
	
int ListWidget::GetLineIdx(const std::string& theLine)
{	
	for (ulong i = 0; i < mLines.size(); i++)	
		if (strcmp(mLines[i].c_str(), theLine.c_str()) == 0)
			return i;
	
	return -1;
}
	
void ListWidget::SetColor(const std::string& theLine, const Color& theColor)
{
	int anIdx = GetLineIdx(theLine);
	SetLineColor(anIdx, theColor);	
}

void ListWidget::SetColor(int theIdx, const Color& theColor)
{
	Widget::SetColor(theIdx, theColor);
}
		
void ListWidget::SetLineColor(int theIdx, const Color& theColor)
{
	if ((theIdx >= 0) && (theIdx < (int)mLines.size()))
	{
		ListWidget *aListWidget = this;

		while (aListWidget->mParent != nullptr) 
			aListWidget = aListWidget->mParent;

		while (aListWidget != nullptr) 
		{
			aListWidget->mLineColors[theIdx] = theColor;
			aListWidget->MarkDirty();
				
			aListWidget = aListWidget->mChild;
		}			
	}
}

void ListWidget::RemoveLine(int theIdx) 
{
	if (theIdx != -1) 
	{
		ListWidget *aListWidget = this;

		while (aListWidget->mParent != nullptr) 
			aListWidget = aListWidget->mParent;

		while (aListWidget != nullptr) 
		{
			aListWidget->mLines.erase(aListWidget->mLines.begin() + theIdx);
			aListWidget->mLineColors.erase(aListWidget->mLineColors.begin() + theIdx);
				
			aListWidget->MarkDirty();
			aListWidget = aListWidget->mChild;	
		}
	}
		
	if (mScrollbar != nullptr) 
		mScrollbar->SetMaxValue(mLines.size());
}
	
void ListWidget::RemoveAll() 
{
	ListWidget *aListWidget = this;

	while (aListWidget->mParent != nullptr) 
		aListWidget = aListWidget->mParent;			

	while (aListWidget != nullptr) 
	{
		aListWidget->mLines.clear();
		aListWidget->mLineColors.clear();
		aListWidget->mSelectIdx = -1;
		aListWidget->mHiliteIdx = -1;
				
		aListWidget->MarkDirty();
		aListWidget = aListWidget->mChild;	
	}

	if (mScrollbar != nullptr) 
		mScrollbar->SetMaxValue(mLines.size());
}
	
int ListWidget::GetOptimalWidth() 
{
	int aMaxWidth = 0;
	
	for (ulong i = 0; i < mLines.size(); i++) 
		aMaxWidth = std::max(aMaxWidth, mFont->StringWidth(mLines[i]));

	return aMaxWidth + 16;
}
	
int ListWidget::GetOptimalHeight() 
{
	int anItemHeight = (mItemHeight != -1) ? mItemHeight : mFont->GetHeight();

	return anItemHeight * mLines.size() + 8;
}
	
void ListWidget::OrderInManagerChanged()
{
	Widget::OrderInManagerChanged();

	if (mChild != nullptr)
		gSexyAppBase->mWidgetManager->PutInfront(mChild, this);
	if (mScrollbar != nullptr)
		gSexyAppBase->mWidgetManager->PutInfront(mScrollbar, this);
}

void ListWidget::Draw(Graphics *g) 
{
	g->SetColor(mColors[COLOR_BKG]);
	g->FillRect(0, 0, mWidth, mHeight);
		
	Graphics aClipG(*g);
	aClipG.ClipRect(4, 4, mWidth - 8, mHeight - 8);
		
	Graphics aSelectClipG(*g);
	aSelectClipG.ClipRect(0, 4, mWidth, mHeight - 8);
				
	aClipG.SetFont(mFont);
		
	int aFirstLine = (int) mPosition;
	int aLastLine = std::min((int) mLines.size()-1, (int) mPosition + (int) mPageSize + 1);
		
	int anItemHeight, anItemOffset;
	if (mItemHeight != -1)
	{
		anItemHeight = mItemHeight;
		anItemOffset = (anItemHeight-mFont->GetHeight())/2;
	}
	else
	{
		anItemHeight = mFont->GetHeight();
		anItemOffset = 0;
	}
	
	for (int i = aFirstLine; i <= aLastLine; i++) 
	{
		int aDrawY = 4 + (int) ((i - mPosition)*anItemHeight);
			
		if (i == mSelectIdx || (i==mHiliteIdx && mDrawSelectWhenHilited)) 
		{
			aSelectClipG.SetColor(mColors[COLOR_SELECT]);
			aSelectClipG.FillRect(0, aDrawY, mWidth, anItemHeight);
		}
			
		if (i == mHiliteIdx)
			aClipG.SetColor(mColors[COLOR_HILITE]);
		else if ((i == mSelectIdx) && (mColors.size() > COLOR_SELECT_TEXT))
			aClipG.SetColor(mColors[COLOR_SELECT_TEXT]);
		else
			aClipG.SetColor(mLineColors[i]);

		std::string aString = mLines[i];
		int aFontX;
		switch (mJustify) 
		{
		case JUSTIFY_LEFT:
			aFontX = 4;
			break;
		case JUSTIFY_CENTER:
			aFontX = (mWidth - mFont->StringWidth(aString))/2;
			break;
		default:
			aFontX = mWidth - mFont->StringWidth(aString) - 4;
			break;
		}
			
		aClipG.DrawString(aString, aFontX, aDrawY + mFont->GetAscent() + anItemOffset);
	}
							
	if (mDrawOutline) 
	{
		g->SetColor(mColors[COLOR_OUTLINE]);		
		g->DrawRect(0, 0, mWidth-1, mHeight-1);
	}
}
	
void ListWidget::ScrollPosition(int theId, double thePosition) 
{
	if (mChild != nullptr)	
		mChild->ScrollPosition(theId, thePosition);
		
	mPosition = thePosition;						
	MarkDirty();
}

void ListWidget::SetHilite(int theHiliteIdx, bool notifyListener)
{
	int anOldIdx = mHiliteIdx;
	mHiliteIdx = theHiliteIdx;
	if (anOldIdx!=mHiliteIdx && notifyListener && mListListener!=nullptr)
		mListListener->ListHiliteChanged(mId,anOldIdx,mHiliteIdx);
}

	
void ListWidget::MouseMove(int x, int y) 
{
	(void)x;
	int anItemHeight = (mItemHeight != -1) ? mItemHeight : mFont->GetHeight();
		
	int aNewHilite = (int) (((y - 4) / (double) anItemHeight) + mPosition);
	if ((aNewHilite < 0) || (aNewHilite >= (int) mLines.size())) 
		aNewHilite = -1;
		
	if (aNewHilite != mHiliteIdx) 
	{
		ListWidget* aListWidget = this;

		while (aListWidget->mParent != nullptr) 
			aListWidget = aListWidget->mParent;

		while (aListWidget != nullptr) 
		{
			aListWidget->SetHilite(aNewHilite,true);
			aListWidget->MarkDirty();
			aListWidget = aListWidget->mChild;	
		}
									
		if (mHiliteIdx == -1 || !mDoFingerWhenHilited) 
			mWidgetManager->mApp->SetCursor(CURSOR_POINTER);
		else 
			mWidgetManager->mApp->SetCursor(CURSOR_HAND);
	}
}
	
void ListWidget::MouseDown(int x, int y, int theBtnNum, int theClickCount) 
{
	(void)x;(void)y;(void)theBtnNum;
	if ((mHiliteIdx != -1) && (mListListener != nullptr))
		mListListener->ListClicked(mId, mHiliteIdx, theClickCount);
}
	
void ListWidget::MouseLeave() 
{
	ListWidget *aListWidget = this;

	while (aListWidget->mParent != nullptr)
		aListWidget = aListWidget->mParent;			

	while (aListWidget != nullptr) 
	{
		aListWidget->SetHilite(-1,true);	
		aListWidget->MarkDirty();	
		aListWidget = aListWidget->mChild;
	}
		
	mWidgetManager->mApp->SetCursor(CURSOR_POINTER);
}
	
void ListWidget::SetSelect(int theSelectIdx)
{
	ListWidget *aListWidget = this;

	while (aListWidget->mParent != nullptr)
		aListWidget = aListWidget->mParent;

	while (aListWidget != nullptr)	
	{
		aListWidget->mSelectIdx = theSelectIdx;
		aListWidget->MarkDirty();	
		aListWidget = aListWidget->mChild;
	}
}


void ListWidget::MouseWheel(int theDelta)
{
	if (mScrollbar != nullptr)
	{
		int aScrollAmount = 5;
		if (theDelta > 0)
		{
			mScrollbar->SetValue(mScrollbar->mValue - aScrollAmount);
//			mScrollbar->mUpdateMode = ScrollbarWidget::UPDATE_MODE_PGUP;
//			mScrollbar->mUpdateAcc = 0;
		}
		else if (theDelta < 0)
		{
			mScrollbar->SetValue(mScrollbar->mValue + aScrollAmount);
//			mScrollbar->mUpdateMode = ScrollbarWidget::UPDATE_MODE_PGDN;
//			mScrollbar->mUpdateAcc = 0;
		}
	}
}
