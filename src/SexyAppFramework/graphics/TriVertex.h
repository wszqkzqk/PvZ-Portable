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

#ifndef __TRIVERTEX_H__
#define __TRIVERTEX_H__

#include <cstdint>

namespace Sexy
{

class TriVertex
{
public:
	float x,y,u,v;
	uint32_t color; //ARGB (0 = use color specified in function call)

public:
	TriVertex() { color = 0; }
	TriVertex(float theX, float theY) : x(theX), y(theY) { color = 0; }
	TriVertex(float theX, float theY, float theU, float theV) : x(theX), y(theY), u(theU), v(theV) { color = 0; }
	TriVertex(float theX, float theY, float theU, float theV, uint32_t theColor) : x(theX), y(theY), u(theU), v(theV), color(theColor) { }
};

} // namespace Sexy


#endif
