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
					unsigned int trb, tg;
					#if !defined(LINEAR_BLEND)
					{
						trb = (((tex&0xff00ff) * alpha) >> 8) & 0xff00ff;
						tg  = (((tex&0x00ff00) * alpha) >> 8) & 0x00ff00;
					}
					#else
					{
						trb = tex&0xff00ff;
						tg = tex&0x00ff00;
					}
					#endif

					trb = ((trb>>9)&0x7c00)|((trb>>3)&0x001f);
					tg = ((tg>>6)&0x03e0);
					tex = *pix;
					alpha = (0xff - alpha)>>3;
					unsigned int	prb = (((tex&0x7c1f) * alpha) >> 5) & 0x7c1f;
					unsigned int	pg  = (((tex&0x03e0) * alpha) >> 5) & 0x03e0;
					*pix = (trb|tg) + (prb|pg);
				}
				else
				{
					*pix = ((tex>>9)&0x7c00)|((tex>>6)&0x03e0)|((tex>>3)&0x001f);
				}
			}
			#else
			{
				*pix = ((tex>>9)&0x7c00)|((tex>>6)&0x03e0)|((tex>>3)&0x001f);
			}
			#endif
		}
	}
	#elif defined(MOD_ARGB)
	{
		if (a > 0xf00000)
		{
			*pix = ((r>>9)&0x7c00)|((g>>14)&0x03e0)|((b>>19)&0x001f);
		}
		else if (a > 0x080000)
		{
			unsigned int	alpha = a >> 16;
			unsigned int	_rb = ((((r&0xff0000) | (b>>16)) * alpha)>> 8)&0xff00ff;
			unsigned int	_g  =  (((g&0xff0000)            * alpha)>>16)&0x00ff00;
					_rb = ((_rb>>9)&0x7c00)|((_rb>>3)&0x001f);
					_g = ((_g>>6)&0x03e0);
			unsigned int	p = *pix;
					alpha = (0xff - alpha)>>3;
			unsigned int	prb = (((p&0x7c1f) * alpha) >> 5) & 0x7c1f;
			unsigned int	pg  = (((p&0x03e0) * alpha) >> 5) & 0x03e0;
			*pix = (_rb|_g)+(prb|pg);
		}
	}
	#endif
