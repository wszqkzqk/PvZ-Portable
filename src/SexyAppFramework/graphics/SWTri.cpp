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

#include "SWTri.h"
#include "misc/Debug.h"

using namespace Sexy;

static SWHelper::XYZStruct	vertexReservoir[64];
static unsigned int			vertexReservoirUsed = 0;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int FixedFloor(int x)
{
	if (x>0)
		return x&0xFFFF0000;
	else
		return (x&0xFFFF0000)-0x10000;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ********************* BEGIN CLIPPING STUFF *********************************************************************
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static inline void lClip(SWHelper::XYZStruct & dst, const SWHelper::XYZStruct & on, const SWHelper::XYZStruct & off, const float edge)
{
   float	delta = (edge - off.mX) / (on.mX - off.mX);
   dst.mX = off.mX + (on.mX - off.mX) * delta;
   dst.mY = off.mY + (on.mY - off.mY) * delta;
   dst.mU = off.mU + (on.mU - off.mU) * delta;
   dst.mV = off.mV + (on.mV - off.mV) * delta;
   dst.mDiffuse =	((int) ((((off.mDiffuse >> 24)&0xff) + (((on.mDiffuse >> 24)&0xff) - ((off.mDiffuse >> 24)&0xff)) * delta))<<24) |
			((int) ((((off.mDiffuse >> 16)&0xff) + (((on.mDiffuse >> 16)&0xff) - ((off.mDiffuse >> 16)&0xff)) * delta))<<16) |
			((int) ((((off.mDiffuse >>  8)&0xff) + (((on.mDiffuse >>  8)&0xff) - ((off.mDiffuse >>  8)&0xff)) * delta))<<8 ) |
			((int) ((((off.mDiffuse >>  0)&0xff) + (((on.mDiffuse >>  0)&0xff) - ((off.mDiffuse >>  0)&0xff)) * delta))    );
}

// --------------------------------------------------------------------------------------------------------------------------------

static inline void rClip(SWHelper::XYZStruct & dst, const SWHelper::XYZStruct & on, const SWHelper::XYZStruct & off, const float edge)
{
   float	delta = (edge - off.mX) / (on.mX - off.mX);
   dst.mX = off.mX + (on.mX - off.mX) * delta;
   dst.mY = off.mY + (on.mY - off.mY) * delta;
   dst.mU = off.mU + (on.mU - off.mU) * delta;
   dst.mV = off.mV + (on.mV - off.mV) * delta;
   dst.mDiffuse =	((int) ((((off.mDiffuse >> 24)&0xff) + (((on.mDiffuse >> 24)&0xff) - ((off.mDiffuse >> 24)&0xff)) * delta))<<24) |
			((int) ((((off.mDiffuse >> 16)&0xff) + (((on.mDiffuse >> 16)&0xff) - ((off.mDiffuse >> 16)&0xff)) * delta))<<16) |
			((int) ((((off.mDiffuse >>  8)&0xff) + (((on.mDiffuse >>  8)&0xff) - ((off.mDiffuse >>  8)&0xff)) * delta))<<8 ) |
			((int) ((((off.mDiffuse >>  0)&0xff) + (((on.mDiffuse >>  0)&0xff) - ((off.mDiffuse >>  0)&0xff)) * delta))    );
}

// --------------------------------------------------------------------------------------------------------------------------------

static inline void tClip(SWHelper::XYZStruct & dst, const SWHelper::XYZStruct & on, const SWHelper::XYZStruct & off, const float edge)
{
   float	delta = (edge - off.mY) / (on.mY - off.mY);
   dst.mX = off.mX + (on.mX - off.mX) * delta;
   dst.mY = off.mY + (on.mY - off.mY) * delta;
   dst.mU = off.mU + (on.mU - off.mU) * delta;
   dst.mV = off.mV + (on.mV - off.mV) * delta;
   dst.mDiffuse =	((int) ((((off.mDiffuse >> 24)&0xff) + (((on.mDiffuse >> 24)&0xff) - ((off.mDiffuse >> 24)&0xff)) * delta))<<24) |
			((int) ((((off.mDiffuse >> 16)&0xff) + (((on.mDiffuse >> 16)&0xff) - ((off.mDiffuse >> 16)&0xff)) * delta))<<16) |
			((int) ((((off.mDiffuse >>  8)&0xff) + (((on.mDiffuse >>  8)&0xff) - ((off.mDiffuse >>  8)&0xff)) * delta))<<8 ) |
			((int) ((((off.mDiffuse >>  0)&0xff) + (((on.mDiffuse >>  0)&0xff) - ((off.mDiffuse >>  0)&0xff)) * delta))    );
}

// --------------------------------------------------------------------------------------------------------------------------------

static inline void bClip(SWHelper::XYZStruct & dst, const SWHelper::XYZStruct & on, const SWHelper::XYZStruct & off, const float edge)
{
   float	delta = (edge - off.mY) / (on.mY - off.mY);
   dst.mX = off.mX + (on.mX - off.mX) * delta;
   dst.mY = off.mY + (on.mY - off.mY) * delta;
   dst.mU = off.mU + (on.mU - off.mU) * delta;
   dst.mV = off.mV + (on.mV - off.mV) * delta;
   dst.mDiffuse =	((int) ((((off.mDiffuse >> 24)&0xff) + (((on.mDiffuse >> 24)&0xff) - ((off.mDiffuse >> 24)&0xff)) * delta))<<24) |
			((int) ((((off.mDiffuse >> 16)&0xff) + (((on.mDiffuse >> 16)&0xff) - ((off.mDiffuse >> 16)&0xff)) * delta))<<16) |
			((int) ((((off.mDiffuse >>  8)&0xff) + (((on.mDiffuse >>  8)&0xff) - ((off.mDiffuse >>  8)&0xff)) * delta))<<8 ) |
			((int) ((((off.mDiffuse >>  0)&0xff) + (((on.mDiffuse >>  0)&0xff) - ((off.mDiffuse >>  0)&0xff)) * delta))    );
}

// --------------------------------------------------------------------------------------------------------------------------------

static inline unsigned int leClip(SWHelper::XYZStruct ** src, SWHelper::XYZStruct ** dst, const float edge)
{
   SWHelper::XYZStruct ** _dst = dst;

   for (SWHelper::XYZStruct **  v = src; *v; ++v)
   {
      SWHelper::XYZStruct *  cur = *v;
      SWHelper::XYZStruct *  nex = *(v+1) ? *(v+1):*src;

      switch((cur->mX < edge ? 1:0)|(nex->mX < edge ? 2:0))
      {
         case 0:
            *dst = *v;
            ++dst;
            break;
         case 1:
         {
            SWHelper::XYZStruct &  tmp = vertexReservoir[vertexReservoirUsed++];
            lClip(tmp, *nex, *cur, edge);
            *dst = &tmp;
            ++dst;
            break;
         }
         case 2:
         {
            *dst = *v;
            ++dst;
            SWHelper::XYZStruct &  tmp = vertexReservoir[vertexReservoirUsed++];
            lClip(tmp, *cur, *nex, edge);
            *dst = &tmp;
            ++dst;
            break;
         }
      }
   }
   *dst = 0;
   return static_cast<int>(dst - _dst);
}

// --------------------------------------------------------------------------------------------------------------------------------

static inline unsigned int reClip(SWHelper::XYZStruct ** src, SWHelper::XYZStruct ** dst, const float edge)
{
   SWHelper::XYZStruct ** _dst = dst;

   for (SWHelper::XYZStruct ** v = src; *v; ++v)
   {
      SWHelper::XYZStruct *  cur = *v;
      SWHelper::XYZStruct *  nex = *(v+1) ? *(v+1):*src;

      switch((cur->mX > edge ? 1:0)|(nex->mX > edge ? 2:0))
      {
         case 0:
            *dst = *v;
            ++dst;
            break;
         case 1:
         {
            SWHelper::XYZStruct &  tmp = vertexReservoir[vertexReservoirUsed++];
            rClip(tmp, *nex, *cur, edge);
            *dst = &tmp;
            ++dst;
            break;
         }
         case 2:
         {
            *dst = *v;
            ++dst;
            SWHelper::XYZStruct &  tmp = vertexReservoir[vertexReservoirUsed++];
            rClip(tmp, *cur, *nex, edge);
            *dst = &tmp;
            ++dst;
            break;
         }
      }
   }
   *dst = 0;
   return static_cast<int>(dst - _dst);
}

// --------------------------------------------------------------------------------------------------------------------------------

static inline unsigned int teClip(SWHelper::XYZStruct ** src, SWHelper::XYZStruct ** dst, const float edge)
{
   SWHelper::XYZStruct ** _dst = dst;

   for (SWHelper::XYZStruct **  v = src; *v; ++v)
   {
      SWHelper::XYZStruct *  cur = *v;
      SWHelper::XYZStruct *  nex = *(v+1) ? *(v+1):*src;

      switch((cur->mY < edge ? 1:0)|(nex->mY < edge ? 2:0))
      {
         case 0:
            *dst = *v;
            ++dst;
            break;
         case 1:
         {
            SWHelper::XYZStruct &  tmp = vertexReservoir[vertexReservoirUsed++];
            tClip(tmp, *nex, *cur, edge);
            *dst = &tmp;
            ++dst;
            break;
         }
         case 2:
         {
            *dst = *v;
            ++dst;
            SWHelper::XYZStruct &  tmp = vertexReservoir[vertexReservoirUsed++];
            tClip(tmp, *cur, *nex, edge);
            *dst = &tmp;
            ++dst;
            break;
         }
      }
   }
   *dst = 0;
   return static_cast<int>(dst - _dst);
}

// --------------------------------------------------------------------------------------------------------------------------------

static inline unsigned int beClip(SWHelper::XYZStruct ** src, SWHelper::XYZStruct ** dst, const float edge)
{
   SWHelper::XYZStruct ** _dst = dst;

   for (SWHelper::XYZStruct ** v = src; *v; ++v)
   {
      SWHelper::XYZStruct *  cur = *v;
      SWHelper::XYZStruct *  nex = *(v+1) ? *(v+1):*src;

      switch((cur->mY > edge ? 1:0)|(nex->mY > edge ? 2:0))
      {
         case 0:
            *dst = *v;
            ++dst;
            break;
         case 1:
         {
            SWHelper::XYZStruct &  tmp = vertexReservoir[vertexReservoirUsed++];
            bClip(tmp, *nex, *cur, edge);
            *dst = &tmp;
            ++dst;
            break;
         }
         case 2:
         {
            *dst = *v;
            ++dst;
            SWHelper::XYZStruct &  tmp = vertexReservoir[vertexReservoirUsed++];
            bClip(tmp, *cur, *nex, edge);
            *dst = &tmp;
            ++dst;
            break;
         }
      }
   }
   *dst = 0;
   return static_cast<int>(dst - _dst);
}

// --------------------------------------------------------------------------------------------------------------------------------

static inline int	clipShape(SWHelper::XYZStruct ** dst, SWHelper::XYZStruct ** src, const float left, const float right, const float top, const float bottom)
{
   vertexReservoirUsed = 0;

   SWHelper::XYZStruct *  buf[64];
   SWHelper::XYZStruct *  ptr[4];
   ptr[0] = src[0];
   ptr[1] = src[1];
   ptr[2] = src[2];
   ptr[3] = 0;
   if (leClip(ptr, buf, left) < 3) return 0;
   if (reClip(buf, dst, right) < 3) return 0;
   if (teClip(dst, buf, top) < 3) return 0;
   return beClip(buf, dst, bottom);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ********************* END CLIPPING STUFF ***********************************************************************
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void SWHelper::SWDrawShape(XYZStruct *theVerts, int theNumVerts, MemoryImage *theImage, const Color &theColor, int theDrawMode, const Rect &theClipRect, void *theSurface, int thePitch, int thePixelFormat, bool blend, bool vertexColor)
{
   (void)theDrawMode;
	float	tclx0 = theClipRect.mX;
	float	tcly0 = theClipRect.mY;
	float	tclx1 = theClipRect.mX + theClipRect.mWidth - 1;
	float	tcly1 = theClipRect.mY + theClipRect.mHeight - 1;

	//
	// Okay, now we're gonna render.  We have the vertex list.
	// mSWRenderPtr is a char* pointer to the upper left
	// bit of the of render array.
	//
	struct XYZStruct *aTVertPtr=theVerts;

	//
	// Some notes:
	//
	// If mSWTexture==nullptr, no texture is selected and pure white can be substituted (clipped to
	// the current material color).
	//
	// If mAlphaMode!=mAlphaModeOn, then there's no need to do any alpha rendering, period.
	// All alpha, even in textures with it, should be ignored.  Otherwise, Alpha basically
	// boils down to: texture pixel alpha * mClippedMaterial.diffuse.a
	//
	// The current material is stored in mClippedMaterial.  You only need to ref the
	// diffuse color: 
	//					mClippedMaterial.diffuse.a
	//					mClippedMaterial.diffuse.r
	//					mClippedMaterial.diffuse.g
	//					mClippedMaterial.diffuse.b
	//
	// The triangle cull can one one of three things:
	// mCullMode=mCullModeNone (cull no triangles)
	// mCullMode=mCullModeCW (cull CW triangles)
	// mCullMode=mCullModeCCW (cull CCW triangles)
	//
	// Texture clamping
	// For this implementation, all uv coordinates will be between 0-1.  However, in DirectX, it
	// is sometimes necessary to clamp textures in order to prevent pixel bleed.  So, though you
	// won't need to implement any texture clamping, please put a few notes in for us so that we
	// can later flesh it out if we decide the software renderer is fast enough to handle something
	// like Hamsterball.
	//
	// Clipping
	// There is a RECT structure defined called mClipArea.  Rect is x,y,width,height -- it's NOT in
	// x1,y1,x2,y2 format.  Anything outside of that rect should just not be clipped.  If you decide
	// it's easier to clip against clip planes using a transform, then please scroll up to look at
	// the Clip() functions-- they translate the coordinates into clip planes.
	// SPECIAL NOTE: The clipping is in "absolute" coordinates.  It is not in a 640x480 capable
	// format.  Our framework allows us to specify a 'real' resolution to use, and conforms everything
	// down to whatever the screen is displaying.  So, we usually use 800x600 for absolute resolution in
	// game.
	// 
	// So, to turn the clip rectangle into the "screen resolution" from "page resolution" (page is what
	// we work in, screen is what displays), multiply the rectangle member variables by mPageWidthAdjust
	// and mPageHeightAdjust.
	//
	// "Game" coordinates to "Screen" coordinates:
	// int aLeft=mClipArea.mX*mPageWidthAdjust;
	// int aTop=mClipArea.mY*mPageHeightAdjust;
	// int aRight=(mClipArea.mX+mClipArea.mSWTexture->mTextureInfo.dwWidth)*mPageWidthAdjust;
	// int aBottom=(mClipArea.mY+mClipArea.mSWTexture->mTextureInfo.dwHeight)*mPageHeightAdjust;
	//
	// Another note: it *is* possible for the clip area to be bigger than the screen (for various
	// reasons).  So, a final clip to 0,0-mSWTexture->mTextureInfo.dwWidth,mSWTexture->mTextureInfo.dwHeight is necessary when translating vertices.
	//

	//
	// Render the actual triangle strip.
	//
	int aTriCounter=0;
	bool aOddTriangle=false;

	// Our global diffuse value

	SWDiffuse	globalDiffuse;
	{
		globalDiffuse.a  = theColor.mAlpha;
		globalDiffuse.r  = theColor.mRed;
		globalDiffuse.g  = theColor.mGreen;
		globalDiffuse.b  = theColor.mBlue;
	}

	// rendering flags

	bool	globalargb = theColor!=Color::White;

	if (theImage)
		theImage->CommitBits();

	bool	textured = theImage!=nullptr;
	bool	talpha = (textured && (theImage->mHasAlpha || theImage->mHasTrans || blend));

	for (;;)
	{
		//
		// Triangle strips... vertices go:
		// 1-2-3 2-4-3 3-4-5 4-6-5 ... every other triangle
		// swaps #2 & #3 vertices for draw to maintain clockwise order.
		//
		if (aTriCounter+3>theNumVerts) break;

		//
		// Picking triangle direction for culling...
		//
		struct XYZStruct *aTriRef[64];
		if (!aOddTriangle)
		{
			aTriRef[0]=aTVertPtr;
			aTriRef[1]=aTVertPtr+1;
			aTriRef[2]=aTVertPtr+2;
		}
		else
		{
			aTriRef[0]=aTVertPtr;
			aTriRef[1]=aTVertPtr+2;
			aTriRef[2]=aTVertPtr+1;
		}
		aTriRef[3] = 0;

		// Clip

		XYZStruct *	clipped[64];
		float		clipX0 = tclx0;
		float		clipY0 = tcly0;
		float		clipX1 = tclx1;
		float		clipY1 = tcly1;

		unsigned int	vCount = clipShape(clipped, aTriRef, clipX0, clipX1, clipY0, clipY1);

		if (vCount)
		{
			unsigned int *	pFrameBuffer = reinterpret_cast<unsigned int *>(theSurface);
			SWVertex	pVerts[64];
			SWTextureInfo	textureInfo;

			for (unsigned int i = 0; i < vCount; ++i)
			{
				pVerts[i].x = static_cast<int>(clipped[i]->mX * 65536.0f);
				pVerts[i].y = static_cast<int>(clipped[i]->mY * 65536.0f);
			}
		
			if (textured)
			{
				for (unsigned int i = 0; i < vCount; ++i)
				{
					pVerts[i].u = static_cast<int>(clipped[i]->mU * (float) theImage->mWidth * 65536.0f);
					pVerts[i].v = static_cast<int>(clipped[i]->mV * (float) theImage->mHeight * 65536.0f);
				}

				textureInfo.pTexture = reinterpret_cast<unsigned int *>(theImage->GetBits());
				textureInfo.pitch = theImage->mWidth;
				textureInfo.height = theImage->mHeight;
				textureInfo.endpos = theImage->mWidth*theImage->mHeight;
//				unsigned int	temp = static_cast<unsigned int>(mSWTexture->mTextureInfo.lPitch) / (mSWTexture->mTextureInfo.ddpfPixelFormat.dwRGBBitCount / 8);
				unsigned int	temp = theImage->mWidth;
				temp >>= 1;
				textureInfo.vShift = 0;
				while(temp) {textureInfo.vShift += 1; temp >>= 1;}
				textureInfo.vShift = 16 - textureInfo.vShift;

				textureInfo.uMask = static_cast<unsigned int>(theImage->mWidth - 1) << 16;
				textureInfo.vMask = static_cast<unsigned int>(theImage->mHeight - 1) << 16;
			}

			if (vertexColor)
			{
				for (unsigned int i = 0; i < vCount; ++i)
				{
					pVerts[i].a = ((unsigned int)clipped[i]->mDiffuse >>  8) & 0xff0000;
					pVerts[i].r = ((unsigned int)clipped[i]->mDiffuse >>  0) & 0xff0000;
					pVerts[i].g = ((unsigned int)clipped[i]->mDiffuse <<  8) & 0xff0000;
					pVerts[i].b = ((unsigned int)clipped[i]->mDiffuse << 16) & 0xff0000;
				}
			}

			SWDrawTriangle(textured, talpha, vertexColor, globalargb, pVerts, pFrameBuffer, thePitch, &textureInfo, globalDiffuse, thePixelFormat, blend);

			if (vCount > 3)
			{
				for (unsigned int extraVert = 2; extraVert < vCount-1; ++extraVert)
				{
					pVerts[1] = pVerts[extraVert];
					pVerts[2] = pVerts[extraVert+1];
					SWDrawTriangle(textured, talpha, vertexColor, globalargb, pVerts, pFrameBuffer, thePitch, &textureInfo, globalDiffuse, thePixelFormat, blend);
				}
			}
		}

		aTVertPtr++;
		aTriCounter++;
		aOddTriangle=!aOddTriangle;
	}
}


static DrawTriFunc gDrawTriFunc[128] = {0};
void Sexy::SWTri_AddDrawTriFunc(bool textured, bool talpha, bool mod_argb, bool global_argb, int thePixelFormat, bool blend, DrawTriFunc theFunc)
{
	int aType = (blend?1:0) | (global_argb?2:0) | (mod_argb?4:0) | (talpha?8:0) | (textured?16:0);
	switch (thePixelFormat)
	{
		case 0x8888: aType |= 0<<5; break;
		case 0x888: aType |= 1<<5; break;
		case 0x565: aType |= 2<<5; break;
		case 0x555: aType |= 3<<5; break;
	}
	gDrawTriFunc[aType] = theFunc;
}

void Sexy::SWTri_AddAllDrawTriFuncs()
{
	gDrawTriFunc[0] = DrawTriangle_8888_TEX0_TALPHA0_MOD0_GLOB0_BLEND0;
	gDrawTriFunc[1] = DrawTriangle_8888_TEX0_TALPHA0_MOD0_GLOB0_BLEND1;
	gDrawTriFunc[2] = DrawTriangle_8888_TEX0_TALPHA0_MOD0_GLOB1_BLEND0;
	gDrawTriFunc[3] = DrawTriangle_8888_TEX0_TALPHA0_MOD0_GLOB1_BLEND1;
	gDrawTriFunc[4] = DrawTriangle_8888_TEX0_TALPHA0_MOD1_GLOB0_BLEND0;
	gDrawTriFunc[5] = DrawTriangle_8888_TEX0_TALPHA0_MOD1_GLOB0_BLEND1;
	gDrawTriFunc[6] = DrawTriangle_8888_TEX0_TALPHA0_MOD1_GLOB1_BLEND0;
	gDrawTriFunc[7] = DrawTriangle_8888_TEX0_TALPHA0_MOD1_GLOB1_BLEND1;
	gDrawTriFunc[8] = DrawTriangle_8888_TEX0_TALPHA1_MOD0_GLOB0_BLEND0;
	gDrawTriFunc[9] = DrawTriangle_8888_TEX0_TALPHA1_MOD0_GLOB0_BLEND1;
	gDrawTriFunc[10] = DrawTriangle_8888_TEX0_TALPHA1_MOD0_GLOB1_BLEND0;
	gDrawTriFunc[11] = DrawTriangle_8888_TEX0_TALPHA1_MOD0_GLOB1_BLEND1;
	gDrawTriFunc[12] = DrawTriangle_8888_TEX0_TALPHA1_MOD1_GLOB0_BLEND0;
	gDrawTriFunc[13] = DrawTriangle_8888_TEX0_TALPHA1_MOD1_GLOB0_BLEND1;
	gDrawTriFunc[14] = DrawTriangle_8888_TEX0_TALPHA1_MOD1_GLOB1_BLEND0;
	gDrawTriFunc[15] = DrawTriangle_8888_TEX0_TALPHA1_MOD1_GLOB1_BLEND1;
	gDrawTriFunc[16] = DrawTriangle_8888_TEX1_TALPHA0_MOD0_GLOB0_BLEND0;
	gDrawTriFunc[17] = DrawTriangle_8888_TEX1_TALPHA0_MOD0_GLOB0_BLEND1;
	gDrawTriFunc[18] = DrawTriangle_8888_TEX1_TALPHA0_MOD0_GLOB1_BLEND0;
	gDrawTriFunc[19] = DrawTriangle_8888_TEX1_TALPHA0_MOD0_GLOB1_BLEND1;
	gDrawTriFunc[20] = DrawTriangle_8888_TEX1_TALPHA0_MOD1_GLOB0_BLEND0;
	gDrawTriFunc[21] = DrawTriangle_8888_TEX1_TALPHA0_MOD1_GLOB0_BLEND1;
	gDrawTriFunc[22] = DrawTriangle_8888_TEX1_TALPHA0_MOD1_GLOB1_BLEND0;
	gDrawTriFunc[23] = DrawTriangle_8888_TEX1_TALPHA0_MOD1_GLOB1_BLEND1;
	gDrawTriFunc[24] = DrawTriangle_8888_TEX1_TALPHA1_MOD0_GLOB0_BLEND0;
	gDrawTriFunc[25] = DrawTriangle_8888_TEX1_TALPHA1_MOD0_GLOB0_BLEND1;
	gDrawTriFunc[26] = DrawTriangle_8888_TEX1_TALPHA1_MOD0_GLOB1_BLEND0;
	gDrawTriFunc[27] = DrawTriangle_8888_TEX1_TALPHA1_MOD0_GLOB1_BLEND1;
	gDrawTriFunc[28] = DrawTriangle_8888_TEX1_TALPHA1_MOD1_GLOB0_BLEND0;
	gDrawTriFunc[29] = DrawTriangle_8888_TEX1_TALPHA1_MOD1_GLOB0_BLEND1;
	gDrawTriFunc[30] = DrawTriangle_8888_TEX1_TALPHA1_MOD1_GLOB1_BLEND0;
	gDrawTriFunc[31] = DrawTriangle_8888_TEX1_TALPHA1_MOD1_GLOB1_BLEND1;
	gDrawTriFunc[32] = DrawTriangle_0888_TEX0_TALPHA0_MOD0_GLOB0_BLEND0;
	gDrawTriFunc[33] = DrawTriangle_0888_TEX0_TALPHA0_MOD0_GLOB0_BLEND1;
	gDrawTriFunc[34] = DrawTriangle_0888_TEX0_TALPHA0_MOD0_GLOB1_BLEND0;
	gDrawTriFunc[35] = DrawTriangle_0888_TEX0_TALPHA0_MOD0_GLOB1_BLEND1;
	gDrawTriFunc[36] = DrawTriangle_0888_TEX0_TALPHA0_MOD1_GLOB0_BLEND0;
	gDrawTriFunc[37] = DrawTriangle_0888_TEX0_TALPHA0_MOD1_GLOB0_BLEND1;
	gDrawTriFunc[38] = DrawTriangle_0888_TEX0_TALPHA0_MOD1_GLOB1_BLEND0;
	gDrawTriFunc[39] = DrawTriangle_0888_TEX0_TALPHA0_MOD1_GLOB1_BLEND1;
	gDrawTriFunc[40] = DrawTriangle_0888_TEX0_TALPHA1_MOD0_GLOB0_BLEND0;
	gDrawTriFunc[41] = DrawTriangle_0888_TEX0_TALPHA1_MOD0_GLOB0_BLEND1;
	gDrawTriFunc[42] = DrawTriangle_0888_TEX0_TALPHA1_MOD0_GLOB1_BLEND0;
	gDrawTriFunc[43] = DrawTriangle_0888_TEX0_TALPHA1_MOD0_GLOB1_BLEND1;
	gDrawTriFunc[44] = DrawTriangle_0888_TEX0_TALPHA1_MOD1_GLOB0_BLEND0;
	gDrawTriFunc[45] = DrawTriangle_0888_TEX0_TALPHA1_MOD1_GLOB0_BLEND1;
	gDrawTriFunc[46] = DrawTriangle_0888_TEX0_TALPHA1_MOD1_GLOB1_BLEND0;
	gDrawTriFunc[47] = DrawTriangle_0888_TEX0_TALPHA1_MOD1_GLOB1_BLEND1;
	gDrawTriFunc[48] = DrawTriangle_0888_TEX1_TALPHA0_MOD0_GLOB0_BLEND0;
	gDrawTriFunc[49] = DrawTriangle_0888_TEX1_TALPHA0_MOD0_GLOB0_BLEND1;
	gDrawTriFunc[50] = DrawTriangle_0888_TEX1_TALPHA0_MOD0_GLOB1_BLEND0;
	gDrawTriFunc[51] = DrawTriangle_0888_TEX1_TALPHA0_MOD0_GLOB1_BLEND1;
	gDrawTriFunc[52] = DrawTriangle_0888_TEX1_TALPHA0_MOD1_GLOB0_BLEND0;
	gDrawTriFunc[53] = DrawTriangle_0888_TEX1_TALPHA0_MOD1_GLOB0_BLEND1;
	gDrawTriFunc[54] = DrawTriangle_0888_TEX1_TALPHA0_MOD1_GLOB1_BLEND0;
	gDrawTriFunc[55] = DrawTriangle_0888_TEX1_TALPHA0_MOD1_GLOB1_BLEND1;
	gDrawTriFunc[56] = DrawTriangle_0888_TEX1_TALPHA1_MOD0_GLOB0_BLEND0;
	gDrawTriFunc[57] = DrawTriangle_0888_TEX1_TALPHA1_MOD0_GLOB0_BLEND1;
	gDrawTriFunc[58] = DrawTriangle_0888_TEX1_TALPHA1_MOD0_GLOB1_BLEND0;
	gDrawTriFunc[59] = DrawTriangle_0888_TEX1_TALPHA1_MOD0_GLOB1_BLEND1;
	gDrawTriFunc[60] = DrawTriangle_0888_TEX1_TALPHA1_MOD1_GLOB0_BLEND0;
	gDrawTriFunc[61] = DrawTriangle_0888_TEX1_TALPHA1_MOD1_GLOB0_BLEND1;
	gDrawTriFunc[62] = DrawTriangle_0888_TEX1_TALPHA1_MOD1_GLOB1_BLEND0;
	gDrawTriFunc[63] = DrawTriangle_0888_TEX1_TALPHA1_MOD1_GLOB1_BLEND1;
	gDrawTriFunc[64] = DrawTriangle_0565_TEX0_TALPHA0_MOD0_GLOB0_BLEND0;
	gDrawTriFunc[65] = DrawTriangle_0565_TEX0_TALPHA0_MOD0_GLOB0_BLEND1;
	gDrawTriFunc[66] = DrawTriangle_0565_TEX0_TALPHA0_MOD0_GLOB1_BLEND0;
	gDrawTriFunc[67] = DrawTriangle_0565_TEX0_TALPHA0_MOD0_GLOB1_BLEND1;
	gDrawTriFunc[68] = DrawTriangle_0565_TEX0_TALPHA0_MOD1_GLOB0_BLEND0;
	gDrawTriFunc[69] = DrawTriangle_0565_TEX0_TALPHA0_MOD1_GLOB0_BLEND1;
	gDrawTriFunc[70] = DrawTriangle_0565_TEX0_TALPHA0_MOD1_GLOB1_BLEND0;
	gDrawTriFunc[71] = DrawTriangle_0565_TEX0_TALPHA0_MOD1_GLOB1_BLEND1;
	gDrawTriFunc[72] = DrawTriangle_0565_TEX0_TALPHA1_MOD0_GLOB0_BLEND0;
	gDrawTriFunc[73] = DrawTriangle_0565_TEX0_TALPHA1_MOD0_GLOB0_BLEND1;
	gDrawTriFunc[74] = DrawTriangle_0565_TEX0_TALPHA1_MOD0_GLOB1_BLEND0;
	gDrawTriFunc[75] = DrawTriangle_0565_TEX0_TALPHA1_MOD0_GLOB1_BLEND1;
	gDrawTriFunc[76] = DrawTriangle_0565_TEX0_TALPHA1_MOD1_GLOB0_BLEND0;
	gDrawTriFunc[77] = DrawTriangle_0565_TEX0_TALPHA1_MOD1_GLOB0_BLEND1;
	gDrawTriFunc[78] = DrawTriangle_0565_TEX0_TALPHA1_MOD1_GLOB1_BLEND0;
	gDrawTriFunc[79] = DrawTriangle_0565_TEX0_TALPHA1_MOD1_GLOB1_BLEND1;
	gDrawTriFunc[80] = DrawTriangle_0565_TEX1_TALPHA0_MOD0_GLOB0_BLEND0;
	gDrawTriFunc[81] = DrawTriangle_0565_TEX1_TALPHA0_MOD0_GLOB0_BLEND1;
	gDrawTriFunc[82] = DrawTriangle_0565_TEX1_TALPHA0_MOD0_GLOB1_BLEND0;
	gDrawTriFunc[83] = DrawTriangle_0565_TEX1_TALPHA0_MOD0_GLOB1_BLEND1;
	gDrawTriFunc[84] = DrawTriangle_0565_TEX1_TALPHA0_MOD1_GLOB0_BLEND0;
	gDrawTriFunc[85] = DrawTriangle_0565_TEX1_TALPHA0_MOD1_GLOB0_BLEND1;
	gDrawTriFunc[86] = DrawTriangle_0565_TEX1_TALPHA0_MOD1_GLOB1_BLEND0;
	gDrawTriFunc[87] = DrawTriangle_0565_TEX1_TALPHA0_MOD1_GLOB1_BLEND1;
	gDrawTriFunc[88] = DrawTriangle_0565_TEX1_TALPHA1_MOD0_GLOB0_BLEND0;
	gDrawTriFunc[89] = DrawTriangle_0565_TEX1_TALPHA1_MOD0_GLOB0_BLEND1;
	gDrawTriFunc[90] = DrawTriangle_0565_TEX1_TALPHA1_MOD0_GLOB1_BLEND0;
	gDrawTriFunc[91] = DrawTriangle_0565_TEX1_TALPHA1_MOD0_GLOB1_BLEND1;
	gDrawTriFunc[92] = DrawTriangle_0565_TEX1_TALPHA1_MOD1_GLOB0_BLEND0;
	gDrawTriFunc[93] = DrawTriangle_0565_TEX1_TALPHA1_MOD1_GLOB0_BLEND1;
	gDrawTriFunc[94] = DrawTriangle_0565_TEX1_TALPHA1_MOD1_GLOB1_BLEND0;
	gDrawTriFunc[95] = DrawTriangle_0565_TEX1_TALPHA1_MOD1_GLOB1_BLEND1;
	gDrawTriFunc[96] = DrawTriangle_0555_TEX0_TALPHA0_MOD0_GLOB0_BLEND0;
	gDrawTriFunc[97] = DrawTriangle_0555_TEX0_TALPHA0_MOD0_GLOB0_BLEND1;
	gDrawTriFunc[98] = DrawTriangle_0555_TEX0_TALPHA0_MOD0_GLOB1_BLEND0;
	gDrawTriFunc[99] = DrawTriangle_0555_TEX0_TALPHA0_MOD0_GLOB1_BLEND1;
	gDrawTriFunc[100] = DrawTriangle_0555_TEX0_TALPHA0_MOD1_GLOB0_BLEND0;
	gDrawTriFunc[101] = DrawTriangle_0555_TEX0_TALPHA0_MOD1_GLOB0_BLEND1;
	gDrawTriFunc[102] = DrawTriangle_0555_TEX0_TALPHA0_MOD1_GLOB1_BLEND0;
	gDrawTriFunc[103] = DrawTriangle_0555_TEX0_TALPHA0_MOD1_GLOB1_BLEND1;
	gDrawTriFunc[104] = DrawTriangle_0555_TEX0_TALPHA1_MOD0_GLOB0_BLEND0;
	gDrawTriFunc[105] = DrawTriangle_0555_TEX0_TALPHA1_MOD0_GLOB0_BLEND1;
	gDrawTriFunc[106] = DrawTriangle_0555_TEX0_TALPHA1_MOD0_GLOB1_BLEND0;
	gDrawTriFunc[107] = DrawTriangle_0555_TEX0_TALPHA1_MOD0_GLOB1_BLEND1;
	gDrawTriFunc[108] = DrawTriangle_0555_TEX0_TALPHA1_MOD1_GLOB0_BLEND0;
	gDrawTriFunc[109] = DrawTriangle_0555_TEX0_TALPHA1_MOD1_GLOB0_BLEND1;
	gDrawTriFunc[110] = DrawTriangle_0555_TEX0_TALPHA1_MOD1_GLOB1_BLEND0;
	gDrawTriFunc[111] = DrawTriangle_0555_TEX0_TALPHA1_MOD1_GLOB1_BLEND1;
	gDrawTriFunc[112] = DrawTriangle_0555_TEX1_TALPHA0_MOD0_GLOB0_BLEND0;
	gDrawTriFunc[113] = DrawTriangle_0555_TEX1_TALPHA0_MOD0_GLOB0_BLEND1;
	gDrawTriFunc[114] = DrawTriangle_0555_TEX1_TALPHA0_MOD0_GLOB1_BLEND0;
	gDrawTriFunc[115] = DrawTriangle_0555_TEX1_TALPHA0_MOD0_GLOB1_BLEND1;
	gDrawTriFunc[116] = DrawTriangle_0555_TEX1_TALPHA0_MOD1_GLOB0_BLEND0;
	gDrawTriFunc[117] = DrawTriangle_0555_TEX1_TALPHA0_MOD1_GLOB0_BLEND1;
	gDrawTriFunc[118] = DrawTriangle_0555_TEX1_TALPHA0_MOD1_GLOB1_BLEND0;
	gDrawTriFunc[119] = DrawTriangle_0555_TEX1_TALPHA0_MOD1_GLOB1_BLEND1;
	gDrawTriFunc[120] = DrawTriangle_0555_TEX1_TALPHA1_MOD0_GLOB0_BLEND0;
	gDrawTriFunc[121] = DrawTriangle_0555_TEX1_TALPHA1_MOD0_GLOB0_BLEND1;
	gDrawTriFunc[122] = DrawTriangle_0555_TEX1_TALPHA1_MOD0_GLOB1_BLEND0;
	gDrawTriFunc[123] = DrawTriangle_0555_TEX1_TALPHA1_MOD0_GLOB1_BLEND1;
	gDrawTriFunc[124] = DrawTriangle_0555_TEX1_TALPHA1_MOD1_GLOB0_BLEND0;
	gDrawTriFunc[125] = DrawTriangle_0555_TEX1_TALPHA1_MOD1_GLOB0_BLEND1;
	gDrawTriFunc[126] = DrawTriangle_0555_TEX1_TALPHA1_MOD1_GLOB1_BLEND0;
	gDrawTriFunc[127] = DrawTriangle_0555_TEX1_TALPHA1_MOD1_GLOB1_BLEND1;
}

#include "SWTri/SWTri_DrawTriangleInc1.inc"

void	SWHelper::SWDrawTriangle(bool textured, bool talpha, bool mod_argb, bool global_argb, SWVertex * pVerts, unsigned int * pFrameBuffer, const unsigned int bytepitch, const SWTextureInfo * textureInfo, SWDiffuse & globalDiffuse, int thePixelFormat, bool blend)
{
	int aType = (blend?1:0) | (global_argb?2:0) | (mod_argb?4:0) | (talpha?8:0) | (textured?16:0);
	switch (thePixelFormat)
	{
		case 0x8888: aType |= 0<<5; break;
		case 0x888: aType |= 1<<5; break;
		case 0x565: aType |= 2<<5; break;
		case 0x555: aType |= 3<<5; break;
	}
	DrawTriFunc aFunc = gDrawTriFunc[aType];
	if (aFunc==nullptr)
	{
		DBG_ASSERT("You need to call SWTri_AddDrawTriFunc or SWTri_AddAllDrawTriFuncs"==nullptr);
	}
	else
		aFunc(pVerts, pFrameBuffer, bytepitch, textureInfo, globalDiffuse);
}
