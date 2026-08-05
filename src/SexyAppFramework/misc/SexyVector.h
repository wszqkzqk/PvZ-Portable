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

#ifndef __SEXYVECTOR_H__
#define __SEXYVECTOR_H__

#include <math.h>

namespace Sexy
{

class SexyVector2
{
public:
	float x,y;

public:
	SexyVector2() : x(0), y(0) { }
	SexyVector2(float theX, float theY) : x(theX), y(theY) { }

	float Dot(const SexyVector2 &v) const { return x*v.x + y*v.y; }
	SexyVector2 operator+(const SexyVector2 &v) const { return SexyVector2(x+v.x, y+v.y); }
	SexyVector2 operator-(const SexyVector2 &v) const { return SexyVector2(x-v.x, y-v.y); }
	SexyVector2 operator-() const { return SexyVector2(-x, -y); }
	SexyVector2 operator*(float t) const { return SexyVector2(t*x, t*y); }
	SexyVector2 operator/(float t) const { return SexyVector2(x/t, y/t); }
	void operator+=(const SexyVector2 &v) { x+=v.x; y+=v.y; }
	void operator-=(const SexyVector2 &v) { x-=v.x; y-=v.y; }
	void operator*=(float t) { x*=t; y*=t; }
	void operator/=(float t) { x/=t; y/=t; }

	bool operator==(const SexyVector2 &v) const = default;

	float Magnitude() const { return sqrtf(x*x + y*y); }
	float MagnitudeSquared() const { return x*x+y*y; }

	SexyVector2 Normalize() const
	{
		float aMag = Magnitude();
		return aMag!=0 ? (*this)/aMag : *this;
	}

	SexyVector2 Perp() const
	{
		return SexyVector2(-y, x);
	}
};

class SexyVector3
{
public:
	float x,y,z;

public:
	SexyVector3() : x(0), y(0), z(0) { }
	SexyVector3(float theX, float theY, float theZ) : x(theX), y(theY), z(theZ) { }

	float Dot(const SexyVector3 &v) const { return x*v.x + y*v.y + z*v.z; }
	SexyVector3 Cross(const SexyVector3 &v) const { return SexyVector3(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x); }
	SexyVector3 operator+(const SexyVector3 &v) const { return SexyVector3(x+v.x, y+v.y, z+v.z); }
	SexyVector3 operator-(const SexyVector3 &v) const { return SexyVector3(x-v.x, y-v.y, z-v.z); }
	SexyVector3 operator*(float t) const { return SexyVector3(t*x, t*y, t*z); }
	SexyVector3 operator/(float t) const { return SexyVector3(x/t, y/t, z/t); }
	bool operator==(const SexyVector3 &v) const = default;
	float Magnitude() const { return sqrtf(x*x + y*y + z*z); }

	SexyVector3 Normalize() const
	{
		float aMag = Magnitude();
		return aMag!=0 ? (*this)/aMag : *this;
	}
};

};

#endif
