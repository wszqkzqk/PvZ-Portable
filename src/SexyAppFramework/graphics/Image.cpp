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

#include "Image.h"
#include "Graphics.h"

using namespace Sexy;

Image::Image()
{
	mWidth = 0;
	mHeight = 0;

	mNumRows = 1;
	mNumCols = 1;

	mDrawn = false;
}

Image::Image(const Image& theImage) :
	mWidth(theImage.mWidth),
	mHeight(theImage.mHeight),
	mNumRows(theImage.mNumRows),
	mNumCols(theImage.mNumCols)
{
	mDrawn = false;
	if (theImage.mAnimInfo != nullptr)
		mAnimInfo = std::make_unique<AnimInfo>(*theImage.mAnimInfo);
}

Image::~Image() = default;

int Image::GetWidth()
{
	return mWidth;
}

int	Image::GetHeight()
{
	return mHeight;
}

Rect Image::GetCelRect(int theCel)
{
	int h = GetCelHeight();
	int w = GetCelWidth();
	int x = (theCel % mNumCols) * w;
	int y = (theCel / mNumCols) * h;

	return Rect(x, y, w, h);
}

Rect Image::GetCelRect(int theCol, int theRow)
{
	int h = GetCelHeight();
	int w = GetCelWidth();
	int x = theCol * w;
	int y = theRow * h;

	return Rect(x, y, w, h);
}

AnimInfo::AnimInfo()
{
	mAnimType = AnimType_None;
	mFrameDelay = 1;
	mNumCels = 1;
}

void AnimInfo::SetPerFrameDelay(int theFrame, int theTime)
{
	if ((int)mPerFrameDelay.size()<=theFrame)
		mPerFrameDelay.resize(theFrame+1);

	mPerFrameDelay[theFrame] = theTime;
}

void AnimInfo::Compute(int theNumCels, int theBeginFrameTime, int theEndFrameTime)
{
	int i;

	mNumCels = theNumCels;
	if (mNumCels<=0)
		mNumCels = 1;

	if (mFrameDelay<=0)
		mFrameDelay = 1;

	if (mAnimType==AnimType_PingPong && mNumCels>1)
	{
		mFrameMap.resize(theNumCels*2-2);
		int index = 0;
		for (i=0; i<theNumCels; i++)
			mFrameMap[index++] = i;
		for (i=theNumCels-2; i>=1; i--)
			mFrameMap[index++] = i;
	}

	if (!mFrameMap.empty())
		mNumCels = (int)mFrameMap.size();

	if (theBeginFrameTime>0)
		SetPerFrameDelay(0,theBeginFrameTime);

	if (theEndFrameTime>0)
		SetPerFrameDelay(mNumCels-1,theEndFrameTime);

	if (!mPerFrameDelay.empty())
	{
		mTotalAnimTime = 0;
		mPerFrameDelay.resize(mNumCels);

		for (i=0; i<mNumCels; i++)
		{
			if (mPerFrameDelay[i]<=0)
				mPerFrameDelay[i] = mFrameDelay;

			mTotalAnimTime += mPerFrameDelay[i];
		}
	}
	else
		mTotalAnimTime = mFrameDelay*mNumCels;

	if (!mFrameMap.empty())
		mFrameMap.resize(mNumCels);
}

int AnimInfo::GetPerFrameCel(int theTime)
{
	for (int i=0; i<mNumCels; i++)
	{
		theTime -= mPerFrameDelay[i];
		if (theTime<0)
			return i;
	}

	return mNumCels-1;
}



int AnimInfo::GetCel(int theTime)
{
	if (mAnimType==AnimType_Once && theTime>=mTotalAnimTime)
	{
		if (!mFrameMap.empty())
			return mFrameMap[mFrameMap.size()-1];
		else
			return mNumCels-1;
	}

	theTime = theTime%mTotalAnimTime;

	int aFrame;
	if (!mPerFrameDelay.empty())
		aFrame = GetPerFrameCel(theTime);
	else
		aFrame = (theTime/mFrameDelay)%mNumCels;

	if (mFrameMap.empty())
		return aFrame;
	else
		return mFrameMap[aFrame];
}

int	Image::GetAnimCel(int theTime)
{
	if (mAnimInfo==nullptr)
		return 0;
	else
		return mAnimInfo->GetCel(theTime);
}

Rect Image::GetAnimCelRect(int theTime)
{
	Rect aRect;
	int aCel = GetAnimCel(theTime);
	int aCelWidth = GetCelWidth();
	int aCelHeight = GetCelHeight();
	if (mNumCols>1)
		return Rect(aCel*aCelWidth,0,aCelWidth,mHeight);
	else
		return Rect(0,aCel*aCelHeight,mWidth,aCelHeight);
}

void Image::CopyAttributes(Image *from)
{
	mNumCols = from->mNumCols;
	mNumRows = from->mNumRows;
	mAnimInfo.reset();
	if (from->mAnimInfo != nullptr)
		mAnimInfo = std::make_unique<AnimInfo>(*from->mAnimInfo);
}

Graphics* Image::GetGraphics()
{
	Graphics* g = new Graphics(this);

	return g;
}


void Image::FillRect(const Rect&, const Color&, int){}

void Image::ClearRect(const Rect&){}


void Image::DrawRect(const Rect& theRect, const Color& theColor, int theDrawMode)
{
	FillRect(Rect(theRect.mX, theRect.mY, theRect.mWidth + 1, 1), theColor, theDrawMode);
	FillRect(Rect(theRect.mX, theRect.mY + theRect.mHeight, theRect.mWidth + 1, 1), theColor, theDrawMode);
	FillRect(Rect(theRect.mX, theRect.mY + 1, 1, theRect.mHeight - 1), theColor, theDrawMode);
	FillRect(Rect(theRect.mX + theRect.mWidth, theRect.mY + 1, 1, theRect.mHeight - 1), theColor, theDrawMode);
}

void Image::DrawLine(double, double, double, double, const Color&, int){}
void Image::DrawLineAA(double, double, double, double, const Color&, int){}

void Image::FillScanLines(Span* theSpans, int theSpanCount, const Color& theColor, int theDrawMode)
{
	for (int i = 0; i < theSpanCount; i++)
	{
		Span* aSpan = &theSpans[i];
		FillRect(Rect(aSpan->mX, aSpan->mY, aSpan->mWidth, 1), theColor, theDrawMode);
	}
}


void Image::FillScanLinesWithCoverage(Span*, int, const Color&, int, const uint8_t*, int, int, int, int){}
bool Image::PolyFill3D(const Point*, int, const Rect*, const Color&,int ,int, int)
{
	return false;
}
void Image::Blt(Image*, int, int, const Rect&, const Color&, int, bool){}
void Image::BltF(Image*, float, float, const Rect&, const Rect&, const Color&, int){}
void Image::BltRotated(Image*, float, float, const Rect &, const Rect&, const Color&, int, double, float, float){}
void Image::StretchBlt(Image*, const Rect&, const Rect&, const Rect&, const Color&, int, bool){}
void Image::BltMatrix(Image*, float, float, const SexyMatrix3&, const Rect&, const Color&, int, const Rect&, bool){}
void Image::BltTrianglesTex(Image*, const TriVertex (*)[3], int, const Rect&, const Color &, int, float, float, bool){}
void Image::BltMirror(Image*, int, int, const Rect&, const Color&, int, bool){}
void Image::StretchBltMirror(Image*, const Rect&, const Rect&, const Rect&, const Color&, int, bool){}
