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
{
	int premult; (void)premult;
	#if defined(MOD_ARGB) && defined(GLOBAL_ARGB)
	{
		premult = ((globalDiffuse.a*a)>>24);
		alpha = (alpha * premult) >> 8;
		tex =	((((tex&0xff0000)*((globalDiffuse.r*r)>>24))>>8)&0xff0000)|
			((((tex&0x00ff00)*((globalDiffuse.g*g)>>24))>>8)&0x00ff00)|
			((((tex&0x0000ff)*((globalDiffuse.b*b)>>24))>>8)&0x0000ff);
	}
	#elif !defined(MOD_ARGB) && defined(GLOBAL_ARGB)
	{
		premult = globalDiffuse.a;
		alpha = (alpha * premult) >> 8;
		tex =	((((tex&0xff0000)*globalDiffuse.r)>>8)&0xff0000)|
			((((tex&0x00ff00)*globalDiffuse.g)>>8)&0x00ff00)|
			((((tex&0x0000ff)*globalDiffuse.b)>>8)&0x0000ff);
	}
	#elif defined(MOD_ARGB) && !defined(GLOBAL_ARGB)
	{
		premult = a>>16;
		alpha = (alpha * premult) >> 8;
		tex =	((((tex&0xff0000)*(r>>16))>>8)&0xff0000)|
			((((tex&0x00ff00)*(g>>16))>>8)&0x00ff00)|
			((((tex&0x0000ff)*(b>>16))>>8)&0x0000ff);
	}
	#endif

	// linear blend expects pixel to already be premultiplied by alpha
	#if defined(LINEAR_BLEND) && (defined(MOD_ARGB) || defined(GLOBAL_ARGB))
	{
		int r = (((tex&0xff0000)*premult)>>8)&0xff0000;
		int g = (((tex&0x00ff00)*premult)>>8)&0x00ff00;
		int b = (((tex&0x0000ff)*premult)>>8)&0x0000ff;
		tex = r|g|b;
	}	
	#endif
}
