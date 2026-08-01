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

	#if defined(TEXTURED)
	{
		#include "SWTri_GetTexel.inc"
		
		if (alpha > 0x08)
		{
			#include "SWTri_TexelARGB.inc"
				
			#if defined(GLOBAL_ARGB) || defined (TEX_ALPHA) || defined(MOD_ARGB)
			{
				if (alpha < 0xf0)
				{
					unsigned int p = *pix;
					unsigned int da = p >> 24;
					
					unsigned int tr,tg,tb;
					#if !defined(LINEAR_BLEND)
					{
						tr = ((tex&0xFF0000)*alpha)&0xFF000000;
						tg = ((tex&0x00FF00)*alpha)&0x00FF0000;
						tb = ((tex&0x0000FF)*alpha)&0x0000FF00;
					}
					#else
					{
						tr = (tex&0xFF0000)<<8;
						tg = (tex&0x00FF00)<<8;
						tb = (tex&0x0000FF)<<8;
					}
					#endif
					
					unsigned int dr = (((p&0xFF0000)*da)>>8) & 0xFF0000;
					unsigned int dg = (((p&0x00FF00)*da)>>8) & 0x00FF00;
					unsigned int db = (((p&0x0000FF)*da)>>8) & 0x0000FF;
					
					int finalAlpha = 256 - (((256 - alpha)*(256 - da))>>8);
					tr = ((tr + (256-alpha)*dr)/finalAlpha) & 0xFF0000;
					tg = ((tg + (256-alpha)*dg)/finalAlpha) & 0x00FF00;
					tb = ((tb + (256-alpha)*db)/finalAlpha) & 0x0000FF;
					
					*pix = ((finalAlpha-1)<<24) | tr | tg | tb; 
				}
				else
				{
					*pix = tex | 0xFF000000;
				}
			}
			#else
			{
				*pix = tex | 0xFF000000;
			}
			#endif
		}
	}
	#elif defined(MOD_ARGB)
	{
		if (a > 0xf00000)
		{
			*pix = 0xFF000000 | ((r)&0xff0000)|((g>>8)&0xff00)|((b>>16)&0xff);
		}
		else if (a > 0x080000)
		{
			unsigned int p = *pix;
			unsigned int da = p >> 24;
			
			int alpha = a>>16;

			unsigned int tr = ((r)*(alpha))&0xFF000000;
			unsigned int tg = ((g>>8)*(alpha))&0x00FF0000;
			unsigned int tb = ((b>>16)*(alpha))&0x0000FF00;
			
			unsigned int dr = (((p&0xFF0000)*da)>>8) & 0xFF0000;
			unsigned int dg = (((p&0x00FF00)*da)>>8) & 0x00FF00;
			unsigned int db = (((p&0x0000FF)*da)>>8) & 0x0000FF;
			
			int finalAlpha = 256 - (((256 - alpha)*(256 - da))>>8);
			tr = ((tr + (256-alpha)*dr)/finalAlpha) & 0xFF0000;
			tg = ((tg + (256-alpha)*dg)/finalAlpha) & 0x00FF00;
			tb = ((tb + (256-alpha)*db)/finalAlpha) & 0x0000FF;
			
			*pix = ((finalAlpha-1)<<24) | tr | tg | tb;
		}
	}
	#endif
