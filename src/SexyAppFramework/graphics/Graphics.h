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

#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include "Common.h"
#include "misc/Rect.h"
#include "Color.h"
#include "Image.h"
#include "TriVertex.h"

namespace Sexy
{

class _Font;
class SexyMatrix3;
class Transform;

struct Edge
{
    double mX;
    double mDX;
    int i;
	double b;
};

class Graphics;

class GraphicsState
{
public:
	static Image			mStaticImage;
	Image*					mDestImage;
	float					mTransX;
	float					mTransY;
	float					mScaleX;
	float					mScaleY;
	float					mScaleOrigX;
	float					mScaleOrigY;
	Rect					mClipRect;
	Color					mColor;
	_Font*					mFont;
	int						mDrawMode;
	bool					mColorizeImages;
	bool					mFastStretch;
	bool					mWriteColoredString;
	bool					mLinearBlend;
	bool					mIs3D;

public:
	void					CopyStateFrom(const GraphicsState* theState);
};

typedef std::list<GraphicsState> GraphicsStateList;

class Graphics : public GraphicsState
{
public:	
	enum
	{
		DRAWMODE_NORMAL,
		DRAWMODE_ADDITIVE
	};
	
	Edge*					mPFActiveEdgeList;
	int						mPFNumActiveEdges;
	static const Point*		mPFPoints;
	int						mPFNumVertices;

	GraphicsStateList		mStateStack;

protected:	
	static int				PFCompareInd(const void* u, const void* v);
	static int				PFCompareActive(const void* u, const void* v);
	void					PFDelete(int i); 
	void					PFInsert(int i, int y);

	void					DrawImageTransformHelper(Image* theImage, const Transform &theTransform, const Rect &theSrcRect, float x, float y, bool useFloat);

public:
	Graphics(const Graphics& theGraphics);
	Graphics(Image* theDestImage = nullptr);
	virtual ~Graphics();	

	void					PushState();
	void					PopState();

	Graphics*				Create();
	
	void					SetFont(_Font* theFont);
	_Font*					GetFont();

	void					SetColor(const Color& theColor);
	const Color&			GetColor();
	
	void					SetDrawMode(int theDrawMode);
	int						GetDrawMode();
	
	void					SetColorizeImages(bool colorizeImages);
	bool					GetColorizeImages();

	void					SetFastStretch(bool fastStretch);
	bool					GetFastStretch();

	void					SetLinearBlend(bool linear); // for DrawImageMatrix, DrawImageTransform, etc...
	bool					GetLinearBlend();

	void					FillRect(int theX, int theY, int theWidth, int theHeight);
	void					FillRect(const Rect& theRect);
	void					DrawRect(int theX, int theY, int theWidth, int theHeight);	
	void					DrawRect(const Rect& theRect);
	void					ClearRect(int theX, int theY, int theWidth, int theHeight);	
	void					ClearRect(const Rect& theRect);
	void					DrawString(std::string_view theString, int theX, int theY);
	
private:
	bool					DrawLineClipHelper(double* theStartX, double* theStartY, double *theEndX, double* theEndY);
public:
	void					DrawLine(int theStartX, int theStartY, int theEndX, int theEndY);
	void					DrawLineAA(int theStartX, int theStartY, int theEndX, int theEndY);
	void					PolyFill(const Point *theVertexList, int theNumVertices, bool convex = false);
	void					PolyFillAA(const Point *theVertexList, int theNumVertices, bool convex = false);

	void					DrawImage(Image* theImage, int theX, int theY);
	void					DrawImage(Image* theImage, int theX, int theY, const Rect& theSrcRect);
	void					DrawImage(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect);
	void					DrawImage(Image* theImage, int theX, int theY, int theStretchedWidth, int theStretchedHeight);
	void					DrawImageF(Image* theImage, float theX, float theY);
	void					DrawImageF(Image* theImage, float theX, float theY, const Rect& theSrcRect);

	void					DrawImageMirror(Image* theImage, int theX, int theY, bool mirror = true);
	void					DrawImageMirror(Image* theImage, int theX, int theY, const Rect& theSrcRect, bool mirror = true);
	void					DrawImageMirror(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect, bool mirror = true);

	void					DrawImageRotated(Image* theImage, int theX, int theY, double theRot, const Rect *theSrcRect = nullptr);
	void					DrawImageRotated(Image* theImage, int theX, int theY, double theRot, int theRotCenterX, int theRotCenterY, const Rect *theSrcRect = nullptr);
	void					DrawImageRotatedF(Image* theImage, float theX, float theY, double theRot, const Rect *theSrcRect = nullptr);
	void					DrawImageRotatedF(Image* theImage, float theX, float theY, double theRot, float theRotCenterX, float theRotCenterY, const Rect *theSrcRect = nullptr);

	void					DrawImageMatrix(Image* theImage, const SexyMatrix3 &theMatrix, float x = 0, float y = 0);
	void					DrawImageMatrix(Image* theImage, const SexyMatrix3 &theMatrix, const Rect &theSrcRect, float x = 0, float y = 0);
	void					DrawImageTransform(Image* theImage, const Transform &theTransform, float x = 0, float y = 0);
	void					DrawImageTransform(Image* theImage, const Transform &theTransform, const Rect &theSrcRect, float x = 0, float y = 0);
	void					DrawImageTransformF(Image* theImage, const Transform &theTransform, float x = 0, float y = 0);
	void					DrawImageTransformF(Image* theImage, const Transform &theTransform, const Rect &theSrcRect, float x = 0, float y = 0);
	void					DrawTriangleTex(Image *theTexture, const TriVertex &v1, const TriVertex &v2, const TriVertex &v3);
	void					DrawTrianglesTex(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles);

	void					DrawImageCel(Image* theImageStrip, int theX, int theY, int theCel);
	void					DrawImageCel(Image* theImageStrip, const Rect& theDestRect, int theCel);
	void					DrawImageCel(Image* theImageStrip, int theX, int theY, int theCelCol, int theCelRow);
	void					DrawImageCel(Image* theImageStrip, const Rect& theDestRect, int theCelCol, int theCelRow);

	void					DrawImageAnim(Image* theImageAnim, int theX, int theY, int theTime);

	void					ClearClipRect();
	void					SetClipRect(int theX, int theY, int theWidth, int theHeight);
	void					SetClipRect(const Rect& theRect);
	void					ClipRect(int theX, int theY, int theWidth, int theHeight);
	void					ClipRect(const Rect& theRect);
	void					Translate(int theTransX, int theTransY);
	void					TranslateF(float theTransX, float theTransY);

	// In progress: Only affects DrawImage
	void					SetScale(float theScaleX, float theScaleY, float theOrigX, float theOrigY);

	int						StringWidth(std::string_view theString);
	void					DrawImageBox(const Rect& theDest, Image* theComponentImage);
	void					DrawImageBox(const Rect& theSrc, const Rect& theDest, Image* theComponentImage);

	int						WriteString(std::string_view theString, int theX, int theY, int theWidth = -1, int theJustification = 0, bool drawString = true, int theOffset = 0, int theLength = -1, int theOldColor = -1);
	int						WriteWordWrapped(const Rect& theRect, std::string_view theLine, int theLineSpacing = -1, int theJustification = -1, int *theMaxWidth = nullptr, int theMaxChars = -1, int* theLastWidth = nullptr);
	int						DrawStringColor(std::string_view theString, int theX, int theY, int theOldColor = -1); //works like DrawString but can have color tags like ^ff0000^.
	int						DrawStringWordWrapped(std::string_view theLine, int theX, int theY, int theWrapWidth = 10000000, int theLineSpacing = -1, int theJustification = -1, int *theMaxWidth = nullptr); //works like DrawString but also word wraps
	int						GetWordWrappedHeight(int theWidth, std::string_view theLine, int theLineSpacing = -1, int *theMaxWidth = nullptr);

	bool					Is3D() { return mIs3D; }
};

class GraphicsAutoState
{
public:
	Graphics*				mG;

public:
	
	GraphicsAutoState(Graphics* theG) : mG(theG)
	{
		mG->PushState();
	}

	~GraphicsAutoState()
	{
		mG->PopState();
	}
};

}

#endif //__GRAPHICS_H__
