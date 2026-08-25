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

#ifndef __GLINTERFACE_H__
#define __GLINTERFACE_H__

#include <mutex>
#include <set>
#include "Common.h"
#include "graphics/GLPlatform.h"
#include "graphics/MemoryImage.h"
#include "graphics/NativeDisplay.h"
#include "misc/Rect.h"
#include "misc/Ratio.h"
#include "misc/SexyMatrix.h"
#include <memory>

namespace Sexy
{

class SexyAppBase;
class GLImage;
class SexyMatrix3;
class TriVertex;

typedef std::set<GLImage*> GLImageSet;

enum RenderImageFlags
{
	RenderImageFlag_MinimizeNumSubdivisions	=			0x0001,		// subdivide image into fewest possible textures (may use more memory)
	RenderImageFlag_Use64By64Subdivisions	=			0x0002,		// good to use with image strips so the entire texture isn't pulled in when drawing just a piece
	RenderImageFlag_UseA4R4G4B4				=			0x0004,		// images with not too many color gradients work well in this format
	RenderImageFlag_UseA8R8G8B8				=			0x0008,		// non-alpha images will be stored as R5G6B5 by default so use this option if you want a 32-bit non-alpha image
	RenderImageFlag_Repeat					=			0x0010,		// use GL_REPEAT sampling for this texture
	RenderImageFlag_TextureMask				=			0x001F		// mask for flags that affect texture state/layout (excludes transient flags like sanding)
};

struct TextureDataPiece
{
	GLuint mTexture;
	int mWidth,mHeight;
};

enum PixelFormat
{
	PixelFormat_Unknown				=			0x0000,
	PixelFormat_A8R8G8B8			=			0x0001,
	PixelFormat_A4R4G4B4			=			0x0002,
	PixelFormat_R5G6B5				=			0x0004,
	PixelFormat_Palette8			=			0x0008
};

struct GLVertex
{
	float sx;
	float sy;
	float sz;
	uint32_t color;
	float tu;
	float tv;
};

struct VertexList
{
	enum { MAX_STACK_VERTS = 100 };
	GLVertex mStackVerts[MAX_STACK_VERTS];
	GLVertex* mVerts;
	int mSize;
	int mCapacity;

	typedef int size_type;

	VertexList() : mVerts(mStackVerts), mSize(0), mCapacity(MAX_STACK_VERTS) { }
	VertexList(const VertexList& theList) : mVerts(mStackVerts), mSize(0), mCapacity(MAX_STACK_VERTS)
	{
		reserve(theList.mSize);
		mSize = theList.mSize;
		memcpy(mVerts, theList.mVerts, mSize * sizeof(mVerts[0]));
	}

	~VertexList()
	{
		if (mVerts != mStackVerts)
			delete[] mVerts;
	}

	void reserve(int theCapacity)
	{
		if (mCapacity < theCapacity)
		{
			mCapacity = theCapacity;
			GLVertex* aNewList = new GLVertex[theCapacity];
			memcpy(aNewList, mVerts, mSize * sizeof(mVerts[0]));
			if (mVerts != mStackVerts)
				delete[] mVerts;

			mVerts = aNewList;
		}
	}

	void push_back(const GLVertex& theVert)
	{
		if (mSize == mCapacity)
			reserve(mCapacity * 2);

		mVerts[mSize++] = theVert;
	}

	void operator=(const VertexList& theList)
	{
		reserve(theList.mSize);
		mSize = theList.mSize;
		memcpy(mVerts, theList.mVerts, mSize * sizeof(mVerts[0]));
	}


	GLVertex& operator[](int thePos)
	{
		return mVerts[thePos];
	}

	int size() { return mSize; }
	void clear() { mSize = 0; }
};

struct TextureData
{
public:
	typedef std::vector<TextureDataPiece> TextureVector;

	TextureVector mTextures;

	int mWidth,mHeight;
	int mTexVecWidth, mTexVecHeight;
	int mTexPieceWidth, mTexPieceHeight;
	int mBitsChangedCount;
	int mTexMemSize;
	float mMaxTotalU, mMaxTotalV;
	PixelFormat mPixelFormat;
	int mImageFlags;

	TextureData();
	~TextureData();

	void ReleaseTextures();

	void CreateTextureDimensions(MemoryImage *theImage);
	void CreateTextures(MemoryImage *theImage);
	void CheckCreateTextures(MemoryImage *theImage);
	GLuint& GetTexture(int x, int y, int &width, int &height, float &u1, float &v1, float &u2, float &v2, float *uvBounds);
	GLuint& GetTextureF(float x, float y, float &width, float &height, float &u1, float &v1, float &u2, float &v2, float *uvBounds);

	void Blt(float theX, float theY, const Rect& theSrcRect, const Color& theColor);
	void BltTransformed(const SexyMatrix3 &theTrans, const Rect& theSrcRect, const Color& theColor, const Rect *theClipRect = nullptr, float theX = 0, float theY = 0, bool center = false);
	void BltTriangles(const TriVertex theVertices[][3], int theNumTriangles, unsigned int theColor, float tx = 0, float ty = 0, bool clampUv = true);
};

class GLInterface : public NativeDisplay
{
public:
	SexyAppBase*			mApp;

	std::mutex				mCritSect;
	int						mWidth;
	int						mHeight;
	int						mDisplayWidth;
	int						mDisplayHeight;

	Rect					mPresentationRect;
	int						mRefreshRate;
	int						mMillisecondsPerFrame;

	std::unique_ptr<GLImage>	mScreenImage;

	int						mNextCursorX;
	int						mNextCursorY;
	int						mCursorX;
	int						mCursorY;

	typedef std::set<MemoryImage*> ImageSet;
	ImageSet mImageSet;
	GLImageSet				mGLImageSet;

	typedef std::list<SexyMatrix3> TransformStack;
	TransformStack mTransformStack;

	void					SetDrawMode(int theDrawMode);

public:
	void					AddGLImage(GLImage* theDDImage);
	void					RemoveGLImage(GLImage* theDDImage);
	void					Remove3DData(MemoryImage* theImage);

public:
	GLInterface(SexyAppBase* theApp);
	~GLInterface() override;

	GLImage*				GetScreenImage();
	void					UpdateViewport();
	int						Init(bool IsWindowed);
	bool					Redraw(Rect* theClipRect = nullptr);
	void					SetVideoOnlyDraw(bool videoOnly);

	void					SetCursorPos(int theCursorX, int theCursorY);

public:
	void					PushTransform(const SexyMatrix3 &theTransform, bool concatenate = true);
	void					PopTransform();

	bool					PreDraw();
	void					Flush();

	bool					CreateImageTexture(MemoryImage* theImage);
	bool					RecoverBits(MemoryImage* theImage);
	void					Blt(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode, bool linearFilter = true);
	void					BltClipF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect *theClipRect, const Color& theColor, int theDrawMode, bool linearFilter = true);
	void					BltMirror(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode, bool linearFilter = true);
	void					StretchBlt(Image* theImage,  const Rect& theDestRect, const Rect& theSrcRect, const Rect* theClipRect, const Color &theColor, int theDrawMode, bool fastStretch, bool mirror = false);
	void					BltRotated(Image* theImage, float theX, float theY, const Rect* theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY, const Rect& theSrcRect);
	void					BltTransformed(Image* theImage, const Rect* theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, const SexyMatrix3 &theTransform, bool linearFilter, float theX = 0, float theY = 0, bool center = false);
	void					DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode);
	void					FillRect(const Rect& theRect, const Color& theColor, int theDrawMode);
	void					DrawTriangle(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor, int theDrawMode);
	void					DrawTriangleTex(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor, int theDrawMode, Image *theTexture, bool blend = true);
	void					DrawTrianglesTex(const TriVertex theVertices[][3], int theNumTriangles, const Color &theColor, int theDrawMode, Image *theTexture, float tx = 0, float ty = 0, bool blend = true);
	void					DrawTrianglesTexStrip(const TriVertex theVertices[], int theNumTriangles, const Color &theColor, int theDrawMode, Image *theTexture, float tx = 0, float ty = 0, bool blend = true);
	void					FillPoly(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor, int theDrawMode, int tx, int ty);
};

}

#endif // __GLINTERFACE_H__
