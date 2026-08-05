/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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

#pragma once
#include <cstdint>
#include <cstdarg>
#include <stdlib.h>
#include <cmath>
#include <cfloat>
#include "../ConstEnums.h"  // PvzpCurves, DrawStringJustification
#include "../SexyAppFramework/Common.h"
#include "PvzpDebug.h"
#include "misc/ResourceManager.h"

struct PvzpAllocator;
namespace Sexy
{
	class Graphics;
	class SexyMatrix;
	class SexyMatrix3;
	class SexyVector2;
	class MemoryImage;
};
//using namespace std;
using namespace Sexy;

#define RENDERIMAGEFLAG_SANDING 0x1000
#define DEG_TO_RAD(deg) ((deg) * 0.017453292f)
#define RAD_TO_DEG(rad) ((rad) * 57.29578f)

struct PvzpWeightedArray
{
	intptr_t mItem;
	int32_t mWeight;
};

struct PvzpWeightedGridArray
{
	int32_t mX;
	int32_t mY;
	int32_t mWeight;
};

class PvzpSmoothArray
{
public:
	int32_t		mItem;
	float		mWeight;
	float		mLastPicked;
	float		mSecondLastPicked;
};

// Modified to a portable inline function that supports 32/64 bit systems
template <typename T>
inline T PvzpPickFromArray(const T* theArray, int theCount)
{
	PVZP_ASSERT(theCount > 0);
	return theCount > 0 ? theArray[Sexy::Rand(theCount)] : T{};
}
intptr_t				PvzpPickFromWeightedArray(const PvzpWeightedArray* theArray, int theCount);
PvzpWeightedArray*		PvzpPickArrayItemFromWeightedArray(const PvzpWeightedArray* theArray, int theCount);
PvzpWeightedGridArray*	PvzpPickFromWeightedGridArray(const PvzpWeightedGridArray* theArray, int theCount);
float					PvzpCalcSmoothWeight(float aWeight, float aLastPicked, float aSecondLastPicked);
void					PvzpUpdateSmoothArrayPick(PvzpSmoothArray* theArray, int theCount, int thePickIndex);
int						PvzpPickFromSmoothArray(PvzpSmoothArray* theArray, int theCount);

class PvzpResourceManager : public ResourceManager
{
public:
	PvzpResourceManager(SexyAppBase* theApp) : ResourceManager(theApp) {}

	bool				FindImagePath(Image* theImage, std::string* thePath);
	bool 				FindFontPath(_Font* theFont, std::string* thePath);
	void				AddImageToMap(SharedImageRef* theImage, const std::string& thePath);
	bool				PvzpLoadNextResource();
	bool				PvzpLoadResources(const std::string& theGroup);
};

bool			PvzpLoadResources(const std::string& theGroup);
bool			PvzpLoadNextResource();
void					PvzpAddImageToMap(SharedImageRef* theImage, const std::string& thePath);
bool					PvzpFindImagePath(Image* theImage, std::string* thePath);
bool					PvzpFindFontPath(_Font* theFont, std::string* thePath);

float		PvzpCurveQuad(float theTime);
float		PvzpCurveInvQuad(float theTime);
float		PvzpCurveS(float theTime);
float		PvzpCurveInvQuadS(float theTime);
float		PvzpCurveBounce(float theTime);
float					PvzpCurveQuadS(float theTime);
float					PvzpCurveCubic(float theTime);
float					PvzpCurveInvCubic(float theTime);
float					PvzpCurveCubicS(float theTime);
float					PvzpCurvePoly(float theTime, float thePoly);
float					PvzpCurveInvPoly(float theTime, float thePoly);
float					PvzpCurvePolyS(float theTime, float thePoly);
float					PvzpCurveCircle(float theTime);
float					PvzpCurveInvCircle(float theTime);
float					PvzpCurveEvaluate(float theTime, float thePositionStart, float thePositionEnd, PvzpCurves theCurve);
float					PvzpCurveEvaluateClamped(float theTime, float thePositionStart, float thePositionEnd, PvzpCurves theCurve);
float					PvzpAnimateCurveFloatTime(float theTimeStart, float theTimeEnd, float theTimeAge, float thePositionStart, float thePositionEnd, PvzpCurves theCurve);
float					PvzpAnimateCurveFloat(int theTimeStart, int theTimeEnd, int theTimeAge, float thePositionStart, float thePositionEnd, PvzpCurves theCurve);
int						PvzpAnimateCurve(int theTimeStart, int theTimeEnd, int theTimeAge, int thePositionStart, int thePositionEnd, PvzpCurves theCurve);

void			PvzpScaleTransformMatrix(SexyMatrix3& m, float x, float y, float theScaleX, float theScaleY);
void					PvzpScaleRotateTransformMatrix(SexyMatrix3& m, float x, float y, float rad, float theScaleX, float theScaleY);
void					SexyMatrix3ExtractScale(const SexyMatrix3& m, float& theScaleX, float& theScaleY);
void			SexyMatrix3Translation(SexyMatrix3& m, float x, float y);
void					SexyMatrix3Transpose(const SexyMatrix3& m, SexyMatrix3& r);  // r = m ^ T
void					SexyMatrix3Inverse(const SexyMatrix3& m, SexyMatrix3& r);  // r = m ^ -1
void					SexyMatrix3Multiply(SexyMatrix3& m, const SexyMatrix3& l, const SexyMatrix3& r);  // m = l × r
bool					PvzpIsPointInPolygon(const SexyVector2* thePolygonPoint, int theNumberPolygonPoints, const SexyVector2& theCheckPoint);

void					PvzpDrawString(Graphics* g, std::string_view theText, int thePosX, int thePosY, _Font* theFont, const Color& theColor, DrawStringJustification theJustification);
void					PvzpDrawStringMatrix(Graphics* g, const _Font* theFont, const SexyMatrix3& theMatrix, std::string_view theString, const Color& theColor);
void					PvzpDrawImageScaledF(Graphics* g, Image* theImage, float thePosX, float thePosY, float theScaleX, float theScaleY);
void					PvzpDrawImageCenterScaledF(Graphics* g, Image* theImage, float thePosX, float thePosY, float theScaleX, float theScaleY);
void					PvzpDrawImageCelF(Graphics* g, Image* theImageStrip, float thePosX, float thePosY, int theCelCol, int theCelRow);
void					PvzpDrawImageCelScaled(Graphics* g, Image* theImageStrip, int thePosX, int thePosY, int theCelCol, int theCelRow, float theScaleX, float theScaleY);
void					PvzpDrawImageCelScaledF(Graphics* g, Image* theImageStrip, float thePosX, float thePosY, int theCelCol, int theCelRow, float theScaleX, float theScaleY);
void					PvzpDrawImageCelCenterScaledF(Graphics* g, Image* theImageStrip, float thePosX, float thePosY, int theCelCol, float theScaleX, float theScaleY);
void					PvzpBltMatrix(Graphics* g, Image* theImage, const SexyMatrix3& theTransform, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect& theSrcRect);
void					PvzpMarkImageForSanding(Image* theImage);
void					PvzpSandImageIfNeeded(Image* theImage);
void					FixPixelsOnAlphaEdgeForBlending(Image* theImage);
uint32_t				AverageNearByPixels(MemoryImage* theImage, uint32_t* thePixel, int x, int y);
void					Pvzp_SWTri_AddAllDrawTriFuncs();

std::string				PvzpReplaceString(std::string_view theText, const char* theStringToFind, std::string_view theStringToSubstitute);
std::string				PvzpReplaceNumberString(std::string_view theText, const char* theStringToFind, int theNumber);
int						PvzpSnprintf(char* theBuffer, int theSize, const char* theFormat, ...);
int						PvzpVsnprintf(char* theBuffer, int theSize, const char* theFormat, va_list theArgList);

PvzpAllocator*			FindGlobalAllocator(int theSize);
void                    FreeGlobalAllocators();

int			RandRangeInt(int theMin, int theMax);
float		RandRangeFloat(float theMin, float theMax);
inline float			Distance2D(float x1, float y1, float x2, float y2)			{ return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)); }
inline float			FloatLerp(float theStart, float theEnd, float theFactor)	{ return theStart + theFactor * (theEnd - theStart); }
inline int				FloatRoundToInt(float theFloatValue)						{ return theFloatValue > 0 ? theFloatValue + 0.5f : theFloatValue - 0.5f; }
inline bool				FloatApproxEqual(float theFloatVal1, float theFloatVal2)	{ return fabs(theFloatVal1 - theFloatVal2) < FLT_EPSILON; }

Color					GetFlashingColor(uint32_t theCounter, int theFlashTime);
int			ColorComponentMultiply(int theColor1, int theColor2);
Color					ColorsMultiply(const Color& theColor1, const Color& theColor2);
Color					ColorAdd(const Color& theColor1, const Color& theColor2);

inline void				SetBit(uint& theNum, int theIdx, bool theValue = true)		{ if (theValue) theNum |= 1 << theIdx; else theNum &= ~(1 << theIdx); }
inline bool				TestBit(uint theNum, int theIdx)							{ return theNum & (1 << theIdx); }
//#define SetBit(num, idx, val) { if (val) (num) |= 1 << (idx); else (num) &= ~(1 << (idx)); }
//#define TestBit(num, idx) ((num) & (1 - (idx)))
