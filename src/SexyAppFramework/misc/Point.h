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

#ifndef __POINT_H__
#define __POINT_H__

#include "Common.h"

namespace Sexy
{

template<class _T> class TPoint
{
public:
	_T						mX;
	_T						mY;

public:
	TPoint(_T theX, _T theY) :
		mX(theX),
		mY(theY)
	{
	}

	/*
	TPoint(const TPoint<_T>& theTPoint) :
		mX(theTPoint.mX),
		mY(theTPoint.mY)
	{
	}
	*/

	TPoint() :
		mX(0),
		mY(0)
	{
	}

	bool operator==(const TPoint& p) const = default;

	TPoint operator+(const TPoint& p) const {return TPoint(mX+p.mX, mY+p.mY);}
	TPoint operator-(const TPoint& p) const {return TPoint(mX-p.mX, mY-p.mY);}
	TPoint operator*(const TPoint& p) const {return TPoint(mX*p.mX, mY*p.mY);}
	TPoint operator/(const TPoint& p) const {return TPoint(mX/p.mX, mY/p.mY);}
	TPoint& operator+=(const TPoint& p)  {mX+=p.mX; mY+=p.mY; return *this;}
	TPoint& operator-=(const TPoint& p)  {mX-=p.mX; mY-=p.mY; return *this;}
	TPoint& operator*=(const TPoint& p)  {mX*=p.mX; mY*=p.mY; return *this;}
	TPoint& operator/=(const TPoint& p)  {mX/=p.mX; mY/=p.mY; return *this;}
	TPoint operator*(_T s) const {return TPoint(mX*s, mY*s);}
	TPoint operator/(_T s) const {return TPoint(mX/s, mY/s);}
};

typedef TPoint<int> Point;
typedef TPoint<double> FPoint;

};

#endif //__POINT_H__
