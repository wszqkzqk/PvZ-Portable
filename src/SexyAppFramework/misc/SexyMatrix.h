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

#ifndef __SEXYMATRIX_H__
#define __SEXYMATRIX_H__

#include "SexyVector.h"

namespace Sexy
{

class SexyMatrix3
{
public:
	union
	{
		float m[3][3];
		struct
		{
			float m00, m01, m02; // scaleX*cos,	skewX*sin,	translateX
			float m10, m11, m12; // skewY*-sin,	scaleY*cos,	translateY
			float m20, m21, m22; // projective;	in this project always (0, 0, 1)
		};
	};

public:
	SexyMatrix3();
	void ZeroMatrix();
	void LoadIdentity();

	SexyVector2 operator*(const SexyVector2 &theVec) const;
	SexyVector3 operator*(const SexyVector3 &theVec) const;
	SexyMatrix3 operator*(const SexyMatrix3 &theMat) const;
	const SexyMatrix3& operator*=(const SexyMatrix3 &theMat);
};

class SexyTransform2D : public SexyMatrix3
{
public:
	SexyTransform2D();
	SexyTransform2D(bool loadIdentity);
	SexyTransform2D(const SexyMatrix3& theMatrix);

	const SexyTransform2D& operator=(const SexyMatrix3 &theMat);


	void Translate(float tx, float ty);

	// Rotate has been replaced by RotateRad.
	// NOTE:  If you had Rotate(angle) you should now use RotateRad(-angle).
	// This is to make positive rotations go counter-clockwise when using screen coordinates.
	void RotateRad(float rot);
	void RotateDeg(float rot);
	void Scale(float sx, float sy);
};

class Transform
{
protected:
	mutable SexyTransform2D mMatrix;
	mutable bool mNeedCalcMatrix;
	void MakeComplex();
	void CalcMatrix() const;

public:
	bool mComplex, mHaveRot, mHaveScale;
	float mTransX1, mTransY1, mTransX2, mTransY2;
	float mScaleX, mScaleY;
	float mRot;

public:
	Transform();

	void Reset();

	void Translate(float tx, float ty);
	void RotateRad(float rot);
	void RotateDeg(float rot);
	void Scale(float sx, float sy);

	const SexyTransform2D& GetMatrix() const;
};


} // namespace Sexy

#endif
