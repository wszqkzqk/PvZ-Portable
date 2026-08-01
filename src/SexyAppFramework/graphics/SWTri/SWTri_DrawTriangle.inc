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

// This file is included by SWTri.cpp and should not be built directly by the project.

#if (TRI0==0)
	#define PTYPE uint32_t
	#define PIXEL_INCLUDE "SWTri_Pixel8888.inc"
	#define NAME0 8888
#elif (TRI0==1)
	#define PTYPE uint32_t
	#define PIXEL_INCLUDE "SWTri_Pixel888.inc"
	#define NAME0 0888
#elif (TRI0==2)
	#define PTYPE uint16_t
	#define PIXEL_INCLUDE "SWTri_Pixel565.inc"
	#define NAME0 0565
#elif (TRI0==3)
	#define PTYPE uint16_t
	#define PIXEL_INCLUDE "SWTri_Pixel555.inc"
	#define NAME0 0555
#endif

#if (TRI1==1)
	#define TEXTURED
	#define NAME1 TEX1
#else
	#define NAME1 TEX0
#endif


#if (TRI2==1)
	#define TEX_ALPHA
	#define NAME2 TALPHA1
#else
	#define NAME2 TALPHA0
#endif

#if (TRI3==1)
	#define MOD_ARGB
	#define NAME3 MOD1
#else
	#define NAME3 MOD0
#endif

#if (TRI4==1)
	#define GLOBAL_ARGB
	#define NAME4 GLOB1
#else
	#define NAME4 GLOB0
#endif

#if (TRI5==1)
	#define LINEAR_BLEND
	#define NAME5 BLEND1
#else
	#define NAME5 BLEND0
#endif

#define funcname2(t0,t1,t2,t3,t4,t5) DrawTriangle_##t0##_##t1##_##t2##_##t3##_##t4##_##t5
#define funcname1(t0,t1,t2,t3,t4,t5) funcname2(t0,t1,t2,t3,t4,t5)
#define funcname funcname1(NAME0,NAME1,NAME2,NAME3,NAME4,NAME5)
void	Sexy::funcname(SWHelper::SWVertex * pVerts, void * pFrameBuffer, const unsigned int bytepitch, const SWHelper::SWTextureInfo * textureInfo, SWHelper::SWDiffuse & globalDiffuse)
{
	(void)globalDiffuse;
	const int pitch = bytepitch/sizeof(PTYPE);
	const int tex_pitch = textureInfo->pitch; (void)tex_pitch;
	const int tex_height = textureInfo->height; (void)tex_height;
	const unsigned int tex_endpos = textureInfo->endpos; (void)tex_endpos;

	const SWHelper::signed64 bigOne = static_cast<SWHelper::signed64>(1) << 48;
	#define  swap(a,b,type) {type tmp = a; a = b; b = tmp;}

	const unsigned int *	pTexture; (void)pTexture;
	unsigned int		vShift, uMask, vMask; (void)vShift;(void)uMask;(void)vMask;
	
	#if defined(TEXTURED)
	{
		pTexture = textureInfo->pTexture;
		vShift = textureInfo->vShift;
		vMask = textureInfo->vMask;
		uMask = textureInfo->uMask;
	}
	#endif

	// Sort vertices by Y component

	SWHelper::SWVertex *	v0 = pVerts+0;
	SWHelper::SWVertex *	v1 = pVerts+1;
	SWHelper::SWVertex *	v2 = pVerts+2;

	if (v0->y > v1->y) swap(v0,v1,SWHelper::SWVertex *);
	if (v1->y > v2->y) swap(v1,v2,SWHelper::SWVertex *);
	if (v0->y > v1->y) swap(v0,v1,SWHelper::SWVertex *);

	#if defined(MOD_ARGB) && defined(GLOBAL_ARGB)
	{
		v0->a = (v0->a * globalDiffuse.a) >> 8;
		v0->r = (v0->r * globalDiffuse.r) >> 8;
		v0->g = (v0->g * globalDiffuse.g) >> 8;
		v0->b = (v0->b * globalDiffuse.b) >> 8;
		v1->a = (v1->a * globalDiffuse.a) >> 8;
		v1->r = (v1->r * globalDiffuse.r) >> 8;
		v1->g = (v1->g * globalDiffuse.g) >> 8;
		v1->b = (v1->b * globalDiffuse.b) >> 8;
		v2->a = (v2->a * globalDiffuse.a) >> 8;
		v2->r = (v2->r * globalDiffuse.r) >> 8;
		v2->g = (v2->g * globalDiffuse.g) >> 8;
		v2->b = (v2->b * globalDiffuse.b) >> 8;
	}
	#endif

	// Integer Y values (using a quick form of ceil() for positive values)

	int	y0 = (v0->y + 0xffff) >> 16;
	int	y2 = (v2->y + 0xffff) >> 16;
	if (y0 == y2) return;   // Null polygon (no height)?
	int	y1 = (v1->y + 0xffff) >> 16;

	// Calculate long-edge deltas

	SWHelper::signed64	oneOverHeight = bigOne / (v2->y - v0->y);
	int		ldx, ldr, ldg, ldb, lda, ldu, ldv;
	(void)ldr;(void)ldg;(void)ldb;(void)lda;(void)ldu;(void)ldv;
			ldx = static_cast<int>(((v2->x - v0->x) * oneOverHeight) >> 32);

	#if defined(MOD_ARGB)		
		lda = static_cast<int>(((v2->a - v0->a) * oneOverHeight) >> 32);
		ldr = static_cast<int>(((v2->r - v0->r) * oneOverHeight) >> 32);
		ldg = static_cast<int>(((v2->g - v0->g) * oneOverHeight) >> 32);
		ldb = static_cast<int>(((v2->b - v0->b) * oneOverHeight) >> 32);
	#endif
	
	#if defined(TEXTURED)
		ldu = static_cast<int>(((v2->u - v0->u) * oneOverHeight) >> 32);
		ldv = static_cast<int>(((v2->v - v0->v) * oneOverHeight) >> 32);
	#endif

	// Long-edge midpoint

	SWHelper::signed64	topHeight = v1->y - v0->y;
	int		mid = v0->x + static_cast<int>((topHeight * ldx)>>16);

	if (v1->x == mid) return;   // Null polygon (no width)?

	// Edge variables (long)

	SWHelper::signed64	subPix = (y0<<16) - v0->y;
	int		lx, lr, lg, lb, la, lu, lv;
	(void)lr;(void)lg;(void)lb;(void)la;(void)lu;(void)lv;
			lx = v0->x + static_cast<int>((ldx * subPix)>>16);

	#if defined(MOD_ARGB)
		la = v0->a + static_cast<int>((lda * subPix)>>16);
		lr = v0->r + static_cast<int>((ldr * subPix)>>16);
		lg = v0->g + static_cast<int>((ldg * subPix)>>16);
		lb = v0->b + static_cast<int>((ldb * subPix)>>16);
	#endif
	
	#if defined(TEXTURED)
		lu = v0->u + static_cast<int>((ldu * subPix)>>16);
		lv = v0->v + static_cast<int>((ldv * subPix)>>16);
	#endif

	// Scanline deltas

	SWHelper::signed64	oneOverWidth; (void)oneOverWidth;
	int		dr, dg, db, da, du, dv;
	(void)dr;(void)dg;(void)db;(void)da;(void)du;(void)dv;
	#if defined(TEXTURED) || defined(MOD_ARGB)	
		oneOverWidth = bigOne / (v1->x - mid);
	#endif
	
	#if defined (MOD_ARGB)
		da = static_cast<int>(((v1->a - (v0->a + ((topHeight * lda)>>16))) * oneOverWidth)>>32);
		dr = static_cast<int>(((v1->r - (v0->r + ((topHeight * ldr)>>16))) * oneOverWidth)>>32);
		dg = static_cast<int>(((v1->g - (v0->g + ((topHeight * ldg)>>16))) * oneOverWidth)>>32);
		db = static_cast<int>(((v1->b - (v0->b + ((topHeight * ldb)>>16))) * oneOverWidth)>>32);
	#endif
	
	#if defined (TEXTURED)
		du = static_cast<int>(((v1->u - (v0->u + ((topHeight * ldu)>>16))) * oneOverWidth)>>32);
		dv = static_cast<int>(((v1->v - (v0->v + ((topHeight * ldv)>>16))) * oneOverWidth)>>32);
	#endif

	// Screen info

	unsigned int	offset = y0 * pitch;
	PTYPE *		fb = reinterpret_cast<PTYPE *>(pFrameBuffer) + offset;
	int		iHeight = y1 - y0;

	if (iHeight)
	{
		// Short edge delta X

		oneOverHeight = bigOne / topHeight;
		int	sdx = static_cast<int>(((v1->x - v0->x) * oneOverHeight) >> 32);

		// Edge variables (short)

		int	sx = v0->x + static_cast<int>((sdx * subPix)>>16);

		// Scan-convert the top half

		if (mid < v1->x)
		{
			while(iHeight-- > 0)
			{
				// Integer (ceil()) left and right X components

				int		x0 = (lx + 0xffff) & 0xffff0000;
				int		x1 = (sx + 0xffff) & 0xffff0000;
				#include "SWTri_Loop.inc"
			}
		}
		else if (mid > v1->x)
		{
			while(iHeight-- > 0)
			{
				// Integer (ceil()) left and right X components

				int	x0 = (sx + 0xffff) & 0xffff0000;
				int	x1 = (lx + 0xffff) & 0xffff0000;
				#include "SWTri_Loop.inc"
			}
		}
	}

	// Done?

	iHeight = y2 - y1;
	if (!iHeight) return;

	// Short edge along bottom half

	oneOverHeight = bigOne / (v2->y - v1->y);
	int	sdx = static_cast<int>(((v2->x - v1->x) * oneOverHeight) >> 32);

	subPix = (y1<<16) - v1->y;
	int	sx = v1->x + static_cast<int>((sdx * subPix)>>16);

	// Scan-convert the bottom half

	if (mid < v1->x)
	{
		while(iHeight-- > 0)
		{
			// Integer (ceil()) left and right X components

			int	x0 = (lx + 0xffff) & 0xffff0000;
			int	x1 = (sx + 0xffff) & 0xffff0000;
			#include "SWTri_Loop.inc"
		}
	}
	else if (mid > v1->x)
	{
		while(iHeight-- > 0)
		{
			// Integer (ceil()) left and right X components

			int	x0 = (sx + 0xffff) & 0xffff0000;
			int	x1 = (lx + 0xffff) & 0xffff0000;
			#include "SWTri_Loop.inc"
		}
	}
}
#undef funcname2
#undef funcname1
#undef funcname
#undef PTYPE
#undef PIXEL_INCLUDE
#undef TEXTURED
#undef TEX_ALPHA
#undef MOD_ARGB
#undef GLOBAL_ARGB
#undef LINEAR_BLEND
#undef NAME0
#undef NAME1
#undef NAME2
#undef NAME3
#undef NAME4
#undef NAME5
