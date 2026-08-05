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

#ifndef __FONT_H__
#define __FONT_H__

#include "Common.h"
#include "misc/Rect.h"
#include "Color.h"
#include <string>

namespace Sexy
{

class Graphics;

class _Font
{
public:
	int						mAscent;
	int						mAscentPadding; // How much space is above the avg uppercase char
	int						mHeight;
	int						mLineSpacingOffset; // This plus height should get added between lines
public:
	_Font();
	_Font(const _Font& theFont);
	virtual ~_Font();

	virtual int				GetAscent();
	virtual int				GetAscentPadding();
	virtual int				GetDescent();
	virtual int				GetHeight();
	virtual int				GetLineSpacingOffset();
	virtual int				GetLineSpacing();
	virtual int				StringWidth(std::string_view theString) = 0;
	virtual int				CharWidth(char32_t theChar) = 0;
	virtual int				CharWidthKern(char32_t theChar, char32_t thePrevChar) = 0;

	virtual void			DrawString(Graphics* g, int theX, int theY, std::string_view theString, const Color& theColor, const Rect& theClipRect) = 0;

	virtual _Font*			Duplicate() = 0;
};

}

#endif //__FONT_H__
