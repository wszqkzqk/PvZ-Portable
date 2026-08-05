/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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

#ifndef __EFFECTSYSTEM_H__
#define __EFFECTSYSTEM_H__

#include <memory>

#include "DataArray.h"
#include "../ConstEnums.h"
#include "graphics/SWTri.h"
#include "graphics/Graphics.h"
using namespace Sexy;

#define MAX_TRIANGLES 256

class PvzpTriVertex
{
public:
	float                       x;
	float                       y;
	float                       u;
	float                       v;
	unsigned long               color;
};

class PvzpTriangleGroup
{
public:
	Image*                      mImage;
	TriVertex                   mVertArray[MAX_TRIANGLES][3];
	int                         mTriangleCount;
	int                         mDrawMode;

	PvzpTriangleGroup();
	void                        DrawGroup(Graphics* g);
	void                        AddTriangle(Graphics* g, Image* theImage, const SexyMatrix3& theMatrix, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect& theSrcRect);
};

extern bool gPvzpTriangleDrawAdditive;

extern void PvzpDrawTriangle_8888_TEX1_TALPHA0_MOD0_GLOB0_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_8888_TEX1_TALPHA0_MOD0_GLOB0_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_8888_TEX1_TALPHA0_MOD0_GLOB1_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_8888_TEX1_TALPHA0_MOD0_GLOB1_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_8888_TEX1_TALPHA0_MOD1_GLOB0_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_8888_TEX1_TALPHA0_MOD1_GLOB0_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_8888_TEX1_TALPHA0_MOD1_GLOB1_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_8888_TEX1_TALPHA0_MOD1_GLOB1_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_8888_TEX1_TALPHA1_MOD0_GLOB0_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_8888_TEX1_TALPHA1_MOD0_GLOB0_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_8888_TEX1_TALPHA1_MOD0_GLOB1_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_8888_TEX1_TALPHA1_MOD0_GLOB1_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_8888_TEX1_TALPHA1_MOD1_GLOB0_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_8888_TEX1_TALPHA1_MOD1_GLOB0_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_8888_TEX1_TALPHA1_MOD1_GLOB1_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_8888_TEX1_TALPHA1_MOD1_GLOB1_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA0_MOD0_GLOB0_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA0_MOD0_GLOB0_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA0_MOD0_GLOB1_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA0_MOD0_GLOB1_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA0_MOD1_GLOB0_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA0_MOD1_GLOB0_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA0_MOD1_GLOB1_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA0_MOD1_GLOB1_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA1_MOD0_GLOB0_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA1_MOD0_GLOB0_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA1_MOD0_GLOB1_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA1_MOD0_GLOB1_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA1_MOD1_GLOB0_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA1_MOD1_GLOB0_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA1_MOD1_GLOB1_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA1_MOD1_GLOB1_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA0_MOD0_GLOB0_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA0_MOD0_GLOB0_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA0_MOD0_GLOB1_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA0_MOD0_GLOB1_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA0_MOD1_GLOB0_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA0_MOD1_GLOB0_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA0_MOD1_GLOB1_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA0_MOD1_GLOB1_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA1_MOD0_GLOB0_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA1_MOD0_GLOB0_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA1_MOD0_GLOB1_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA1_MOD0_GLOB1_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA1_MOD1_GLOB0_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA1_MOD1_GLOB0_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA1_MOD1_GLOB1_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA1_MOD1_GLOB1_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0555_TEX1_TALPHA0_MOD0_GLOB0_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0555_TEX1_TALPHA0_MOD0_GLOB0_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0555_TEX1_TALPHA0_MOD0_GLOB1_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0555_TEX1_TALPHA0_MOD0_GLOB1_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0555_TEX1_TALPHA0_MOD1_GLOB0_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0555_TEX1_TALPHA0_MOD1_GLOB0_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0555_TEX1_TALPHA0_MOD1_GLOB1_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0555_TEX1_TALPHA0_MOD1_GLOB1_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0555_TEX1_TALPHA1_MOD0_GLOB0_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0555_TEX1_TALPHA1_MOD0_GLOB0_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0555_TEX1_TALPHA1_MOD0_GLOB1_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0555_TEX1_TALPHA1_MOD0_GLOB1_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0555_TEX1_TALPHA1_MOD1_GLOB0_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0555_TEX1_TALPHA1_MOD1_GLOB0_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0555_TEX1_TALPHA1_MOD1_GLOB1_BLEND0(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0555_TEX1_TALPHA1_MOD1_GLOB1_BLEND1(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA0_MOD0_GLOB1_BLEND0_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA0_MOD0_GLOB1_BLEND1_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA0_MOD1_GLOB0_BLEND0_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA0_MOD1_GLOB0_BLEND1_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA0_MOD1_GLOB1_BLEND0_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA0_MOD1_GLOB1_BLEND1_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA1_MOD0_GLOB0_BLEND0_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA1_MOD0_GLOB0_BLEND1_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA1_MOD0_GLOB1_BLEND0_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA1_MOD0_GLOB1_BLEND1_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA1_MOD1_GLOB0_BLEND0_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA1_MOD1_GLOB0_BLEND1_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA1_MOD1_GLOB1_BLEND0_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0888_TEX1_TALPHA1_MOD1_GLOB1_BLEND1_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA0_MOD0_GLOB1_BLEND0_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA0_MOD0_GLOB1_BLEND1_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA0_MOD1_GLOB0_BLEND0_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA0_MOD1_GLOB0_BLEND1_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA0_MOD1_GLOB1_BLEND0_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA0_MOD1_GLOB1_BLEND1_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA1_MOD0_GLOB0_BLEND0_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA1_MOD0_GLOB0_BLEND1_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA1_MOD0_GLOB1_BLEND0_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA1_MOD0_GLOB1_BLEND1_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA1_MOD1_GLOB0_BLEND0_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA1_MOD1_GLOB0_BLEND1_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA1_MOD1_GLOB1_BLEND0_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);
extern void PvzpDrawTriangle_0565_TEX1_TALPHA1_MOD1_GLOB1_BLEND1_ADDITIVE(SWHelper::SWVertex* pVerts, void* pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo* textureInfo, SWHelper::SWDiffuse& globalDiffuse);

class Reanimation;
class PvzpParticleHolder;
class TrailHolder;
class ReanimationHolder;
class AttachmentHolder;
class EffectSystem
{
public:
	std::unique_ptr<PvzpParticleHolder>      mParticleHolder;
	std::unique_ptr<TrailHolder>            mTrailHolder;
	std::unique_ptr<ReanimationHolder>      mReanimationHolder;
	std::unique_ptr<AttachmentHolder>       mAttachmentHolder;

public:
	EffectSystem() = default;
	~EffectSystem();

	void                        EffectSystemInitialize();
	void                        EffectSystemDispose();
	void                        EffectSystemFreeAll();
	void                        ProcessDeleteQueue();
	void                        Update();
};
extern EffectSystem* gEffectSystem;

#endif
