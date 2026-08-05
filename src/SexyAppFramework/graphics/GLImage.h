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

#ifndef __GLIMAGE_H__
#define __GLIMAGE_H__

#include "MemoryImage.h"

namespace Sexy
{

class GLInterface;
class SysFont;

class GLImage : public MemoryImage
{
protected:
	friend class			SysFont;

public:
	GLInterface*			mGLInterface;

public:
	void					FillScanLinesWithCoverage(Span* theSpans, int theSpanCount, const Color& theColor, int theDrawMode, const uint8_t* theCoverage, int theCoverX, int theCoverY, int theCoverWidth, int theCoverHeight) override;

	static bool				Check3D(GLImage *theImage);
	static bool				Check3D(Image *theImage);

public:
	GLImage();
	GLImage(GLInterface* theGLInterface);
	~GLImage() override;

	void					Create(int theWidth, int theHeight) override;

	bool					PolyFill3D(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor, int theDrawMode, int tx, int ty) override;
	void					FillRect(const Rect& theRect, const Color& theColor, int theDrawMode) override;
	void					DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode) override;
	void					DrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode) override;
	void					Blt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode, bool linearFilter = true) override;
	void					BltF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect &theClipRect, const Color& theColor, int theDrawMode) override;
	void					BltRotated(Image* theImage, float theX, float theY, const Rect &theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY) override;
	void					StretchBlt(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch) override;
	void					BltMatrix(Image* theImage, float x, float y, const SexyMatrix3 &theMatrix, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, bool blend) override;
	void					BltTrianglesTex(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles, const Rect& theClipRect, const Color &theColor, int theDrawMode, float tx, float ty, bool blend) override;

	void					BltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode, bool linearFilter = true) override;
	void					StretchBltMirror(Image* theImage, const Rect& theDestRectOrig, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fastStretch) override;

	void					PurgeBits() override;
};

}

#endif // __GLIMAGE_H__
