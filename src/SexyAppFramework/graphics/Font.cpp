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

#include "Font.h"

using namespace Sexy;

_Font::_Font()
{
	mAscent = 0;
	mAscentPadding = 0;
	mHeight = 0;
	mLineSpacingOffset = 0;
}

_Font::_Font(const _Font& theFont) :
	mAscent(theFont.mAscent),
	mAscentPadding(theFont.mAscentPadding),
	mHeight(theFont.mHeight),
	mLineSpacingOffset(theFont.mLineSpacingOffset)
{
}

_Font::~_Font()
{
}

int	_Font::GetAscent()
{
	return mAscent;
}

int	_Font::GetAscentPadding()
{
	return mAscentPadding;
}

int	_Font::GetDescent()
{
	return mHeight - mAscent;
}

int	_Font::GetHeight()
{
	return mHeight;
}

int _Font::GetLineSpacingOffset()
{
	return mLineSpacingOffset;
}

int _Font::GetLineSpacing()
{
	return mHeight + mLineSpacingOffset;
}
