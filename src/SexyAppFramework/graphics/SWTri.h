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

#pragma once

#include "Color.h"
#include "MemoryImage.h"
#include "misc/Rect.h"
#include "misc/SexyMatrix.h"

namespace Sexy
{

class SWHelper
{
public:
	struct XYZStruct
	{
		float	mX;
		float	mY;
		float	mU;
		float	mV;
		long	mDiffuse;
	};

	struct SWVertex
	{
		int		x, y;
		int		a, r, g, b;
		int		u, v;
	};
	struct	SWTextureInfo
	{
		const unsigned int *	pTexture;
		unsigned int		vShift, uMask, vMask;
		int pitch;
		unsigned int endpos;
		int height;
	};
	struct	SWDiffuse
	{
		unsigned int		a, r, g, b;
	};

	typedef	int64_t	signed64;

public:
	// For drawing
	static void						SWDrawShape(XYZStruct *theVerts, int theNumVerts, MemoryImage *theImage, const Color &theColor, int theDrawMode, const Rect &theClipRect, void *theSurface, int thePitch, int thePixelFormat, bool blend, bool vertexColor);
	static void						SWDrawTriangle(bool textured, bool talpha, bool mod_argb, bool global_argb, SWVertex * pVerts, unsigned int * pFrameBuffer, const unsigned int pitch, const SWTextureInfo * textureInfo, SWDiffuse & globalDiffuse, int thePixelFormat, bool blend);
};

typedef void(*DrawTriFunc)(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
void	SWTri_AddAllDrawTriFuncs();
void	SWTri_AddDrawTriFunc(bool textured, bool talpha, bool mod_argb, bool global_argb, int thePixelFormat, bool blend, DrawTriFunc theFunc);

extern void DrawTriangle_8888_TEX0_TALPHA0_MOD0_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX0_TALPHA0_MOD0_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX0_TALPHA0_MOD0_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX0_TALPHA0_MOD0_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX0_TALPHA0_MOD1_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX0_TALPHA0_MOD1_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX0_TALPHA0_MOD1_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX0_TALPHA0_MOD1_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX0_TALPHA1_MOD0_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX0_TALPHA1_MOD0_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX0_TALPHA1_MOD0_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX0_TALPHA1_MOD0_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX0_TALPHA1_MOD1_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX0_TALPHA1_MOD1_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX0_TALPHA1_MOD1_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX0_TALPHA1_MOD1_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX1_TALPHA0_MOD0_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX1_TALPHA0_MOD0_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX1_TALPHA0_MOD0_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX1_TALPHA0_MOD0_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX1_TALPHA0_MOD1_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX1_TALPHA0_MOD1_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX1_TALPHA0_MOD1_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX1_TALPHA0_MOD1_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX1_TALPHA1_MOD0_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX1_TALPHA1_MOD0_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX1_TALPHA1_MOD0_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX1_TALPHA1_MOD0_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX1_TALPHA1_MOD1_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX1_TALPHA1_MOD1_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX1_TALPHA1_MOD1_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_8888_TEX1_TALPHA1_MOD1_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX0_TALPHA0_MOD0_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX0_TALPHA0_MOD0_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX0_TALPHA0_MOD0_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX0_TALPHA0_MOD0_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX0_TALPHA0_MOD1_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX0_TALPHA0_MOD1_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX0_TALPHA0_MOD1_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX0_TALPHA0_MOD1_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX0_TALPHA1_MOD0_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX0_TALPHA1_MOD0_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX0_TALPHA1_MOD0_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX0_TALPHA1_MOD0_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX0_TALPHA1_MOD1_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX0_TALPHA1_MOD1_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX0_TALPHA1_MOD1_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX0_TALPHA1_MOD1_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX1_TALPHA0_MOD0_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX1_TALPHA0_MOD0_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX1_TALPHA0_MOD0_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX1_TALPHA0_MOD0_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX1_TALPHA0_MOD1_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX1_TALPHA0_MOD1_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX1_TALPHA0_MOD1_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX1_TALPHA0_MOD1_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX1_TALPHA1_MOD0_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX1_TALPHA1_MOD0_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX1_TALPHA1_MOD0_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX1_TALPHA1_MOD0_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX1_TALPHA1_MOD1_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX1_TALPHA1_MOD1_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX1_TALPHA1_MOD1_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0888_TEX1_TALPHA1_MOD1_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX0_TALPHA0_MOD0_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX0_TALPHA0_MOD0_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX0_TALPHA0_MOD0_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX0_TALPHA0_MOD0_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX0_TALPHA0_MOD1_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX0_TALPHA0_MOD1_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX0_TALPHA0_MOD1_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX0_TALPHA0_MOD1_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX0_TALPHA1_MOD0_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX0_TALPHA1_MOD0_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX0_TALPHA1_MOD0_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX0_TALPHA1_MOD0_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX0_TALPHA1_MOD1_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX0_TALPHA1_MOD1_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX0_TALPHA1_MOD1_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX0_TALPHA1_MOD1_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX1_TALPHA0_MOD0_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX1_TALPHA0_MOD0_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX1_TALPHA0_MOD0_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX1_TALPHA0_MOD0_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX1_TALPHA0_MOD1_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX1_TALPHA0_MOD1_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX1_TALPHA0_MOD1_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX1_TALPHA0_MOD1_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX1_TALPHA1_MOD0_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX1_TALPHA1_MOD0_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX1_TALPHA1_MOD0_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX1_TALPHA1_MOD0_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX1_TALPHA1_MOD1_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX1_TALPHA1_MOD1_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX1_TALPHA1_MOD1_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0565_TEX1_TALPHA1_MOD1_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX0_TALPHA0_MOD0_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX0_TALPHA0_MOD0_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX0_TALPHA0_MOD0_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX0_TALPHA0_MOD0_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX0_TALPHA0_MOD1_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX0_TALPHA0_MOD1_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX0_TALPHA0_MOD1_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX0_TALPHA0_MOD1_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX0_TALPHA1_MOD0_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX0_TALPHA1_MOD0_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX0_TALPHA1_MOD0_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX0_TALPHA1_MOD0_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX0_TALPHA1_MOD1_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX0_TALPHA1_MOD1_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX0_TALPHA1_MOD1_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX0_TALPHA1_MOD1_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX1_TALPHA0_MOD0_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX1_TALPHA0_MOD0_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX1_TALPHA0_MOD0_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX1_TALPHA0_MOD0_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX1_TALPHA0_MOD1_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX1_TALPHA0_MOD1_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX1_TALPHA0_MOD1_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX1_TALPHA0_MOD1_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX1_TALPHA1_MOD0_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX1_TALPHA1_MOD0_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX1_TALPHA1_MOD0_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX1_TALPHA1_MOD0_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX1_TALPHA1_MOD1_GLOB0_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX1_TALPHA1_MOD1_GLOB0_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX1_TALPHA1_MOD1_GLOB1_BLEND0(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);
extern void DrawTriangle_0555_TEX1_TALPHA1_MOD1_GLOB1_BLEND1(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse);

} // namespace Sexy
