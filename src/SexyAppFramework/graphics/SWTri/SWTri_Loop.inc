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

	SWHelper::signed64	subTex = x0 - lx;
	(void)subTex; // unused
	unsigned int	u, v, r, g, b, a;
	(void)u;(void)v;(void)r;(void)g;(void)b;(void)a; // unused
	
	#if defined(MOD_ARGB)
		a = la + static_cast<int>((da * subTex)>>16);
		r = lr + static_cast<int>((dr * subTex)>>16);
		g = lg + static_cast<int>((dg * subTex)>>16);
		b = lb + static_cast<int>((db * subTex)>>16);
	#endif
	
	#if defined(TEXTURED)
		u = lu + static_cast<int>((du * subTex)>>16);
		v = lv + static_cast<int>((dv * subTex)>>16);
	#endif
	
	PTYPE *		pix = fb + (x0>>16);
	(void)pix; // unused
	int		width = ((x1-x0)>>16);
	
	while(width-- > 0)
	{
		#include PIXEL_INCLUDE
//		if (bit_format == 0x888) PIXEL888()
//		if (bit_format == 0x565) PIXEL565()
//		if (bit_format == 0x555) PIXEL555()
//		if (bit_format == 0x8888) PIXEL8888()
		++pix;
		#if defined(MOD_ARGB)
			a += da;
			r += dr;
			g += dg;
			b += db;
		#endif
		
		#if defined(TEXTURED)
			u += du;
			v += dv;
		#endif
	}
	
	lx += ldx;
	sx += sdx;
	fb += pitch;

	#if defined (MOD_ARGB)
		la += lda;
		lr += ldr;
		lg += ldg;
		lb += ldb;
	#endif
		
	#if defined (TEXTURED)	
		lu += ldu;
		lv += ldv;
	#endif
