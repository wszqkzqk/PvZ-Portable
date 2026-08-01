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

#include "GLImage.h"
#include "misc/Debug.h"
#include "graphics/GLInterface.h"
#include "SexyAppBase.h"
#include "Image.h"

using namespace Sexy;

GLImage::GLImage() : MemoryImage(gSexyAppBase)
{
	mGLInterface = gSexyAppBase->mGLInterface;
	if (mGLInterface)
		mGLInterface->AddGLImage(this);
}

GLImage::GLImage(GLInterface *theGLInterface)
	: MemoryImage(theGLInterface ? theGLInterface->mApp : gSexyAppBase)
{
	mGLInterface = theGLInterface;
	if (mGLInterface)
		mGLInterface->AddGLImage(this);
}

GLImage::~GLImage()
{
	if (mGLInterface)
		mGLInterface->RemoveGLImage(this);
}

void GLImage::Create(int theWidth, int theHeight)
{
	MemoryImage::Create(theWidth, theHeight);
}

void GLImage::FillScanLinesWithCoverage(Span* theSpans, int theSpanCount, const Color& theColor, int theDrawMode, const uint8_t* theCoverage, int theCoverX, int theCoverY, int theCoverWidth, int theCoverHeight)
{
	if (theSpanCount == 0) return;

	int l = theSpans[0].mX, t = theSpans[0].mY;
	int r = l + theSpans[0].mWidth, b = t;
	for (int i = 1; i < theSpanCount; ++i)  //此循环结束后，Rect(l, t, r - l + 1, b - t + 1) 即为包含所有 Span 的最小矩形区域
	{
		l = std::min(theSpans[i].mX, l);
		r = std::max(theSpans[i].mX + theSpans[i].mWidth - 1, r);
		t = std::min(theSpans[i].mY, t);
		b = std::max(theSpans[i].mY, b);
	}
	for (int i = 0; i < theSpanCount; ++i)  //此循环将所有 Span 的绝对坐标更改为在上述矩形区域内的相对坐标
	{
		theSpans[i].mX -= l;
		theSpans[i].mY -= t;
	}

	MemoryImage aTempImage;
	aTempImage.Create(r-l+1, b-t+1);  //创建一个与最小矩形区域相同大小的 MemoryImage
	//theCoverX - l 和 theCoverY - t 分别将绝对坐标转化为 MemoryImage 上的相对坐标
	aTempImage.FillScanLinesWithCoverage(theSpans, theSpanCount, theColor, theDrawMode, theCoverage, theCoverX - l, theCoverY - t, theCoverWidth, theCoverHeight);
	Blt(&aTempImage, l, t, Rect(0, 0, r-l+1, b-t+1), Color::White, theDrawMode);
	return;
}

bool GLImage::Check3D(GLImage *theImage)
{
	return true;
}

bool GLImage::Check3D(Image *theImage)
{
	GLImage *anImage = dynamic_cast<GLImage*>(theImage);
	return anImage != 0;
}

void GLImage::PurgeBits()
{
	mPurgeBits = true;

	CommitBits();
	GetBits();

	MemoryImage::PurgeBits();
}

bool GLImage::PolyFill3D(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor, int theDrawMode, int tx, int ty)
{
	mGLInterface->FillPoly(theVertices,theNumVertices,theClipRect,theColor,theDrawMode,tx,ty);
	return true;
}

void GLImage::FillRect(const Rect& theRect, const Color& theColor, int theDrawMode)
{
	mGLInterface->FillRect(theRect,theColor,theDrawMode);
}

void GLImage::DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode)
{
	mGLInterface->DrawLine(theStartX,theStartY,theEndX,theEndY,theColor,theDrawMode);
}

void GLImage::DrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode)
{
	mGLInterface->DrawLine(theStartX,theStartY,theEndX,theEndY,theColor,theDrawMode);
}

void GLImage::Blt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode, bool linearFilter)
{
	theImage->mDrawn = true;

	//if (gDebug)
	//	mApp->CopyToClipboard("+DDImage::Blt");

	DBG_ASSERTE((theColor.mRed >= 0) && (theColor.mRed <= 255));
	DBG_ASSERTE((theColor.mGreen >= 0) && (theColor.mGreen <= 255));
	DBG_ASSERTE((theColor.mBlue >= 0) && (theColor.mBlue <= 255));
	DBG_ASSERTE((theColor.mAlpha >= 0) && (theColor.mAlpha <= 255));

	CommitBits();

	mGLInterface->Blt(theImage,theX,theY,theSrcRect,theColor,theDrawMode,linearFilter);
}

void GLImage::BltF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect &theClipRect, const Color& theColor, int theDrawMode)
{
	theImage->mDrawn = true;

	FRect aClipRect(theClipRect.mX,theClipRect.mY,theClipRect.mWidth,theClipRect.mHeight);
	FRect aDestRect(theX,theY,theSrcRect.mWidth,theSrcRect.mHeight);

	FRect anIntersect = aDestRect.Intersection(aClipRect);
	if (anIntersect.mWidth!=aDestRect.mWidth || anIntersect.mHeight!=aDestRect.mHeight)
	{
		if (anIntersect.mWidth!=0 && anIntersect.mHeight!=0)
			mGLInterface->BltClipF(theImage,theX,theY,theSrcRect,&theClipRect,theColor,theDrawMode);
	}
	else
		mGLInterface->Blt(theImage,theX,theY,theSrcRect,theColor,theDrawMode,true);
}

void GLImage::BltRotated(Image* theImage, float theX, float theY, const Rect &theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY)
{
	theImage->mDrawn = true;

	//if (mNoLock)
		//return;	

	CommitBits();

	mGLInterface->BltRotated(theImage,theX,theY,&theClipRect,theColor,theDrawMode,theRot,theRotCenterX,theRotCenterY,theSrcRect);
}

void GLImage::StretchBlt(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch)
{
	theImage->mDrawn = true;

	CommitBits();

	mGLInterface->StretchBlt(theImage,theDestRect,theSrcRect,&theClipRect,theColor,theDrawMode,fastStretch);
}

void GLImage::BltMatrix(Image* theImage, float x, float y, const SexyMatrix3 &theMatrix, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, bool blend)
{
	theImage->mDrawn = true;

	mGLInterface->BltTransformed(theImage,&theClipRect,theColor,theDrawMode,theSrcRect,theMatrix,blend,x,y,true);
}

void GLImage::BltTrianglesTex(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles, const Rect& theClipRect, const Color &theColor, int theDrawMode, float tx, float ty, bool blend)
{
	theTexture->mDrawn = true;

	mGLInterface->DrawTrianglesTex(theVertices,theNumTriangles,theColor,theDrawMode,theTexture,tx,ty,blend);
}

void GLImage::BltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode, bool linearFilter)
{
	DBG_ASSERTE((theColor.mRed >= 0) && (theColor.mRed <= 255));
	DBG_ASSERTE((theColor.mGreen >= 0) && (theColor.mGreen <= 255));
	DBG_ASSERTE((theColor.mBlue >= 0) && (theColor.mBlue <= 255));
	DBG_ASSERTE((theColor.mAlpha >= 0) && (theColor.mAlpha <= 255));

	CommitBits();

	mGLInterface->BltMirror(theImage,theX,theY,theSrcRect,theColor,theDrawMode,linearFilter);
}

void GLImage::StretchBltMirror(Image* theImage, const Rect& theDestRectOrig, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch)
{
	theImage->mDrawn = true;

	CommitBits();

	mGLInterface->StretchBlt(theImage,theDestRectOrig,theSrcRect,&theClipRect,theColor,theDrawMode,fastStretch,true);
}
