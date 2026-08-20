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

#ifndef __MEMORYIMAGE_H__
#define __MEMORYIMAGE_H__

#include <memory>

#include "Image.h"

#define OPTIMIZE_SOFTWARE_DRAWING
#ifdef OPTIMIZE_SOFTWARE_DRAWING
extern bool gOptimizeSoftwareDrawing;
#endif

namespace Sexy
{

const uint32_t MEMORYCHECK_ID = 0x4BEEFADE;

class NativeDisplay;
class SexyAppBase;

class MemoryImage : public Image
{
public:
	std::unique_ptr<uint32_t[]>		mBits;
	int						mBitsChangedCount;
	void*					mRenderData;
	uint32_t				mRenderFlags;	// see GLInterface.h for possible values

	std::unique_ptr<uint32_t[]>		mColorTable;
	std::unique_ptr<uchar[]>		mColorIndices;

	bool					mForcedMode;
	bool					mHasTrans;
	bool					mHasAlpha;
	bool					mIsVolatile;
	bool					mPurgeBits;
	bool					mWantPal;

	std::unique_ptr<uint32_t[]>		mNativeAlphaData;
	std::unique_ptr<uchar[]>		mRLAlphaData;
	std::unique_ptr<uchar[]>		mRLAdditiveData;

	bool					mBitsChanged;
	SexyAppBase*			mApp;

private:
	void					Init();

public:
	virtual void*			GetNativeAlphaData(NativeDisplay *theNative);
	virtual uchar*			GetRLAlphaData();
	virtual uchar*			GetRLAdditiveData(NativeDisplay *theNative);
	virtual void			PurgeBits();
	virtual void			DeleteSWBuffers();
	virtual void			Delete3DBuffers();
	virtual void			DeleteExtraBuffers();
	virtual void			ReInit();

	virtual void			BitsChanged();
	virtual void			CommitBits();

	virtual void			DeleteNativeData();

	void					NormalBlt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor);
	void					AdditiveBlt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor);

	void					NormalDrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor);
	void					AdditiveDrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor);

	void					NormalDrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor);

	void					SlowStretchBlt(Image* theImage, const Rect& theDestRect, const FRect& theSrcRect, const Color& theColor, int theDrawMode);
	void					FastStretchBlt(Image* theImage, const Rect& theDestRect, const FRect& theSrcRect, const Color& theColor, int theDrawMode);
	bool					BltRotatedClipHelper(float &theX, float &theY, const Rect &theSrcRect, const Rect &theClipRect, double theRot, FRect &theDestRect, float theRotCenterX, float theRotCenterY);
	bool					StretchBltClipHelper(const Rect &theSrcRect, const Rect &theClipRect, const Rect &theDestRect, FRect &theSrcRectOut, Rect &theDestRectOut);
	bool					StretchBltMirrorClipHelper(const Rect &theSrcRect, const Rect &theClipRect, const Rect &theDestRect, FRect &theSrcRectOut, Rect &theDestRectOut);
	void					BltMatrixHelper(Image* theImage, float x, float y, const SexyMatrix3 &theMatrix, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, void *theSurface, int theBytePitch, int thePixelFormat, bool blend);
	void					BltTrianglesTexHelper(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles, const Rect &theClipRect, const Color &theColor, int theDrawMode, void *theSurface, int theBytePitch, int thePixelFormat, float tx, float ty, bool blend);

	void					FillScanLinesWithCoverage(Span* theSpans, int theSpanCount, const Color& theColor, int theDrawMode, const uint8_t* theCoverage, int theCoverX, int theCoverY, int theCoverWidth, int theCoverHeight) override;


public:
	MemoryImage();
	MemoryImage(SexyAppBase* theApp);
	MemoryImage(const MemoryImage& theMemoryImage) = delete;
	MemoryImage& operator=(const MemoryImage& theMemoryImage) = delete;
	~MemoryImage() override;

	virtual void			Clear();
	virtual void			SetBits(uint32_t* theBits, int theWidth, int theHeight, bool commitBits = true);
	virtual void			Create(int theWidth, int theHeight);
	virtual uint32_t*		GetBits();

	void					FillRect(const Rect& theRect, const Color& theColor, int theDrawMode) override;
	void					ClearRect(const Rect& theRect) override;
	void					DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode) override;
	void					DrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode) override;

	void					Blt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode, bool linearFilter = true) override;
	void					BltF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect &theClipRect, const Color& theColor, int theDrawMode) override;
	void					BltRotated(Image* theImage, float theX, float theY, const Rect &theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY) override;
	void					StretchBlt(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch) override;
	void					BltMatrix(Image* theImage, float x, float y, const SexyMatrix3 &theMatrix, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, bool blend) override;
	void					BltTrianglesTex(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles, const Rect& theClipRect, const Color &theColor, int theDrawMode, float tx, float ty, bool blend) override;

	virtual void			SetImageMode(bool hasTrans, bool hasAlpha);
	virtual void			SetVolatile(bool isVolatile);

	virtual bool			Palletize();
};

}

#endif //__MEMORYIMAGE_H__
