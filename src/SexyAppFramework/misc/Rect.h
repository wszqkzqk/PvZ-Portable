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

#ifndef __TRectANGLE_H__
#define __TRectANGLE_H__

#include "Common.h"
#include "Point.h"

#include <list>

namespace Sexy
{

template<class _T> class TRect
{
public:
	_T mX;
	_T mY;
	_T mWidth;
	_T mHeight;

public:
	TRect(_T theX, _T theY, _T theWidth, _T theHeight) :
	  mX(theX), mY(theY), mWidth(theWidth), mHeight(theHeight)
	{
	}

	/*
	TRect(const TRect<_T>& theTRect) :
	  mX(theTRect.mX), mY(theTRect.mY), mWidth(theTRect.mWidth), mHeight(theTRect.mHeight)
	{
	}
	*/

	TRect() :
	  mX(0), mY(0), mWidth(0), mHeight(0)
	{
	}

	bool					Intersects(const TRect<_T>& theTRect) const
	{
		return !((theTRect.mX + theTRect.mWidth <= mX) ||
			(theTRect.mY + theTRect.mHeight <= mY) ||
			(theTRect.mX >= mX + mWidth) ||
			(theTRect.mY >= mY + mHeight));
	}

	TRect<_T>				Intersection(const TRect<_T>& theTRect)	const
	{
		_T x1 = std::max(mX, theTRect.mX);
		_T x2 = std::min(mX + mWidth, theTRect.mX + theTRect.mWidth);
		_T y1 = std::max(mY, theTRect.mY);
		_T y2 = std::min(mY + mHeight, theTRect.mY + theTRect.mHeight);
		if (((x2 - x1) < 0) || ((y2 - y1) < 0))
			return TRect<_T>(0,0,0,0);
		else
			return TRect<_T>(x1, y1, x2 - x1, y2 - y1);
	}

	TRect<_T>				Union(const TRect<_T>& theTRect)
	{
		_T x1 = std::min(mX, theTRect.mX);
		_T x2 = std::max(mX + mWidth, theTRect.mX + theTRect.mWidth);
		_T y1 = std::min(mY, theTRect.mY);
		_T y2 = std::max(mY + mHeight, theTRect.mY + theTRect.mHeight);
			return TRect<_T>(x1, y1, x2 - x1, y2 - y1);
	}

	bool					Contains(_T theX, _T theY) const
	{
		return ((theX >= mX) && (theX < mX + mWidth) &&
			(theY >= mY) && (theY < mY + mHeight));
	}

	bool					Contains(const TPoint<_T>& thePoint) const
	{
		return ((thePoint.mX >= mX) && (thePoint.mX < mX + mWidth) &&
			(thePoint.mY >= mY) && (thePoint.mY < mY + mHeight));
	}

	void					Offset(_T theX, _T theY)
	{
		mX += theX;
		mY += theY;
	}

	void					Offset(const TPoint<_T>& thePoint)
	{
		mX += thePoint.mX;
		mY += thePoint.mY;
	}

	TRect					Inflate(_T theX, _T theY)
	{
		mX -= theX;
		mWidth += theX*2;
		mY -= theY;
		mHeight += theY*2;

		return *this;
	}

	bool operator==(const TRect<_T>& theRect) const = default;
};

typedef TRect<int32_t> Rect;
typedef TRect<double> FRect;
}

#endif //__TRectANGLE_H__
