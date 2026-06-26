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

#include "SexyAppBase.h"
#include "TodList.h"
#include <algorithm>
#include "TodDebug.h"
#include "TodCommon.h"
#include "../LawnApp.h"
#include "EffectSystem.h"
#include "../Resources.h"
#include "TodStringFile.h"
#include "../GameConstants.h"
#include "graphics/Font.h"
#include "misc/Debug.h"
#include "graphics/GLImage.h"
#include "graphics/Graphics.h"
#include "graphics/ImageFont.h"
#include "misc/PerfTimer.h"
#include "misc/SexyMatrix.h"
#include "graphics/GLInterface.h"

void Tod_SWTri_AddAllDrawTriFuncs()
{
	SWTri_AddDrawTriFunc(true, false, false, false, 0x8888, false, TodDrawTriangle_8888_TEX1_TALPHA0_MOD0_GLOB0_BLEND0);
	SWTri_AddDrawTriFunc(true, false, false, false, 0x8888, true, TodDrawTriangle_8888_TEX1_TALPHA0_MOD0_GLOB0_BLEND1);
	SWTri_AddDrawTriFunc(true, false, false, true, 0x8888, false, TodDrawTriangle_8888_TEX1_TALPHA0_MOD0_GLOB1_BLEND0);
	SWTri_AddDrawTriFunc(true, false, false, true, 0x8888, true, TodDrawTriangle_8888_TEX1_TALPHA0_MOD0_GLOB1_BLEND1);
	SWTri_AddDrawTriFunc(true, false, true, false, 0x8888, false, TodDrawTriangle_8888_TEX1_TALPHA0_MOD1_GLOB0_BLEND0);
	SWTri_AddDrawTriFunc(true, false, true, false, 0x8888, true, TodDrawTriangle_8888_TEX1_TALPHA0_MOD1_GLOB0_BLEND1);
	SWTri_AddDrawTriFunc(true, false, true, true, 0x8888, false, TodDrawTriangle_8888_TEX1_TALPHA0_MOD1_GLOB1_BLEND0);
	SWTri_AddDrawTriFunc(true, false, true, true, 0x8888, true, TodDrawTriangle_8888_TEX1_TALPHA0_MOD1_GLOB1_BLEND1);

	SWTri_AddDrawTriFunc(true, true, false, false, 0x8888, false, TodDrawTriangle_8888_TEX1_TALPHA1_MOD0_GLOB0_BLEND0);
	SWTri_AddDrawTriFunc(true, true, false, false, 0x8888, true, TodDrawTriangle_8888_TEX1_TALPHA1_MOD0_GLOB0_BLEND1);
	SWTri_AddDrawTriFunc(true, true, false, true, 0x8888, false, TodDrawTriangle_8888_TEX1_TALPHA1_MOD0_GLOB1_BLEND0);
	SWTri_AddDrawTriFunc(true, true, false, true, 0x8888, true, TodDrawTriangle_8888_TEX1_TALPHA1_MOD0_GLOB1_BLEND1);
	SWTri_AddDrawTriFunc(true, true, true, false, 0x8888, false, TodDrawTriangle_8888_TEX1_TALPHA1_MOD1_GLOB0_BLEND0);
	SWTri_AddDrawTriFunc(true, true, true, false, 0x8888, true, TodDrawTriangle_8888_TEX1_TALPHA1_MOD1_GLOB0_BLEND1);
	SWTri_AddDrawTriFunc(true, true, true, true, 0x8888, false, TodDrawTriangle_8888_TEX1_TALPHA1_MOD1_GLOB1_BLEND0);
	SWTri_AddDrawTriFunc(true, true, true, true, 0x8888, true, TodDrawTriangle_8888_TEX1_TALPHA1_MOD1_GLOB1_BLEND1);

	SWTri_AddDrawTriFunc(true, false, false, false, 0x0888, false, TodDrawTriangle_0888_TEX1_TALPHA0_MOD0_GLOB0_BLEND0);
	SWTri_AddDrawTriFunc(true, false, false, false, 0x0888, true, TodDrawTriangle_0888_TEX1_TALPHA0_MOD0_GLOB0_BLEND1);
	SWTri_AddDrawTriFunc(true, false, false, true, 0x0888, false, TodDrawTriangle_0888_TEX1_TALPHA0_MOD0_GLOB1_BLEND0);
	SWTri_AddDrawTriFunc(true, false, false, true, 0x0888, true, TodDrawTriangle_0888_TEX1_TALPHA0_MOD0_GLOB1_BLEND1);
	SWTri_AddDrawTriFunc(true, false, true, false, 0x0888, false, TodDrawTriangle_0888_TEX1_TALPHA0_MOD1_GLOB0_BLEND0);
	SWTri_AddDrawTriFunc(true, false, true, false, 0x0888, true, TodDrawTriangle_0888_TEX1_TALPHA0_MOD1_GLOB0_BLEND1);
	SWTri_AddDrawTriFunc(true, false, true, true, 0x0888, false, TodDrawTriangle_0888_TEX1_TALPHA0_MOD1_GLOB1_BLEND0);
	SWTri_AddDrawTriFunc(true, false, true, true, 0x0888, true, TodDrawTriangle_0888_TEX1_TALPHA0_MOD1_GLOB1_BLEND1);

	SWTri_AddDrawTriFunc(true, true, false, false, 0x0888, false, TodDrawTriangle_0888_TEX1_TALPHA1_MOD0_GLOB0_BLEND0);
	SWTri_AddDrawTriFunc(true, true, false, false, 0x0888, true, TodDrawTriangle_0888_TEX1_TALPHA1_MOD0_GLOB0_BLEND1);
	SWTri_AddDrawTriFunc(true, true, false, true, 0x0888, false, TodDrawTriangle_0888_TEX1_TALPHA1_MOD0_GLOB1_BLEND0);
	SWTri_AddDrawTriFunc(true, true, false, true, 0x0888, true, TodDrawTriangle_0888_TEX1_TALPHA1_MOD0_GLOB1_BLEND1);
	SWTri_AddDrawTriFunc(true, true, true, false, 0x0888, false, TodDrawTriangle_0888_TEX1_TALPHA1_MOD1_GLOB0_BLEND0);
	SWTri_AddDrawTriFunc(true, true, true, false, 0x0888, true, TodDrawTriangle_0888_TEX1_TALPHA1_MOD1_GLOB0_BLEND1);
	SWTri_AddDrawTriFunc(true, true, true, true, 0x0888, false, TodDrawTriangle_0888_TEX1_TALPHA1_MOD1_GLOB1_BLEND0);
	SWTri_AddDrawTriFunc(true, true, true, true, 0x0888, true, TodDrawTriangle_0888_TEX1_TALPHA1_MOD1_GLOB1_BLEND1);

	SWTri_AddDrawTriFunc(true, false, false, false, 0x0565, false, TodDrawTriangle_0565_TEX1_TALPHA0_MOD0_GLOB0_BLEND0);
	SWTri_AddDrawTriFunc(true, false, false, false, 0x0565, true, TodDrawTriangle_0565_TEX1_TALPHA0_MOD0_GLOB0_BLEND1);
	SWTri_AddDrawTriFunc(true, false, false, true, 0x0565, false, TodDrawTriangle_0565_TEX1_TALPHA0_MOD0_GLOB1_BLEND0);
	SWTri_AddDrawTriFunc(true, false, false, true, 0x0565, true, TodDrawTriangle_0565_TEX1_TALPHA0_MOD0_GLOB1_BLEND1);
	SWTri_AddDrawTriFunc(true, false, true, false, 0x0565, false, TodDrawTriangle_0565_TEX1_TALPHA0_MOD1_GLOB0_BLEND0);
	SWTri_AddDrawTriFunc(true, false, true, false, 0x0565, true, TodDrawTriangle_0565_TEX1_TALPHA0_MOD1_GLOB0_BLEND1);
	SWTri_AddDrawTriFunc(true, false, true, true, 0x0565, false, TodDrawTriangle_0565_TEX1_TALPHA0_MOD1_GLOB1_BLEND0);
	SWTri_AddDrawTriFunc(true, false, true, true, 0x0565, true, TodDrawTriangle_0565_TEX1_TALPHA0_MOD1_GLOB1_BLEND1);

	SWTri_AddDrawTriFunc(true, true, false, false, 0x0565, false, TodDrawTriangle_0565_TEX1_TALPHA1_MOD0_GLOB0_BLEND0);
	SWTri_AddDrawTriFunc(true, true, false, false, 0x0565, true, TodDrawTriangle_0565_TEX1_TALPHA1_MOD0_GLOB0_BLEND1);
	SWTri_AddDrawTriFunc(true, true, false, true, 0x0565, false, TodDrawTriangle_0565_TEX1_TALPHA1_MOD0_GLOB1_BLEND0);
	SWTri_AddDrawTriFunc(true, true, false, true, 0x0565, true, TodDrawTriangle_0565_TEX1_TALPHA1_MOD0_GLOB1_BLEND1);
	SWTri_AddDrawTriFunc(true, true, true, false, 0x0565, false, TodDrawTriangle_0565_TEX1_TALPHA1_MOD1_GLOB0_BLEND0);
	SWTri_AddDrawTriFunc(true, true, true, false, 0x0565, true, TodDrawTriangle_0565_TEX1_TALPHA1_MOD1_GLOB0_BLEND1);
	SWTri_AddDrawTriFunc(true, true, true, true, 0x0565, false, TodDrawTriangle_0565_TEX1_TALPHA1_MOD1_GLOB1_BLEND0);
	SWTri_AddDrawTriFunc(true, true, true, true, 0x0565, true, TodDrawTriangle_0565_TEX1_TALPHA1_MOD1_GLOB1_BLEND1);

	SWTri_AddDrawTriFunc(true, false, false, false, 0x0555, false, TodDrawTriangle_0555_TEX1_TALPHA0_MOD0_GLOB0_BLEND0);
	SWTri_AddDrawTriFunc(true, false, false, false, 0x0555, true, TodDrawTriangle_0555_TEX1_TALPHA0_MOD0_GLOB0_BLEND1);
	SWTri_AddDrawTriFunc(true, false, false, true, 0x0555, false, TodDrawTriangle_0555_TEX1_TALPHA0_MOD0_GLOB1_BLEND0);
	SWTri_AddDrawTriFunc(true, false, false, true, 0x0555, true, TodDrawTriangle_0555_TEX1_TALPHA0_MOD0_GLOB1_BLEND1);
	SWTri_AddDrawTriFunc(true, false, true, false, 0x0555, false, TodDrawTriangle_0555_TEX1_TALPHA0_MOD1_GLOB0_BLEND0);
	SWTri_AddDrawTriFunc(true, false, true, false, 0x0555, true, TodDrawTriangle_0555_TEX1_TALPHA0_MOD1_GLOB0_BLEND1);
	SWTri_AddDrawTriFunc(true, false, true, true, 0x0555, false, TodDrawTriangle_0555_TEX1_TALPHA0_MOD1_GLOB1_BLEND0);
	SWTri_AddDrawTriFunc(true, false, true, true, 0x0555, true, TodDrawTriangle_0555_TEX1_TALPHA0_MOD1_GLOB1_BLEND1);

	SWTri_AddDrawTriFunc(true, true, false, false, 0x0555, false, TodDrawTriangle_0555_TEX1_TALPHA1_MOD0_GLOB0_BLEND0);
	SWTri_AddDrawTriFunc(true, true, false, false, 0x0555, true, TodDrawTriangle_0555_TEX1_TALPHA1_MOD0_GLOB0_BLEND1);
	SWTri_AddDrawTriFunc(true, true, false, true, 0x0555, false, TodDrawTriangle_0555_TEX1_TALPHA1_MOD0_GLOB1_BLEND0);
	SWTri_AddDrawTriFunc(true, true, false, true, 0x0555, true, TodDrawTriangle_0555_TEX1_TALPHA1_MOD0_GLOB1_BLEND1);
	SWTri_AddDrawTriFunc(true, true, true, false, 0x0555, false, TodDrawTriangle_0555_TEX1_TALPHA1_MOD1_GLOB0_BLEND0);
	SWTri_AddDrawTriFunc(true, true, true, false, 0x0555, true, TodDrawTriangle_0555_TEX1_TALPHA1_MOD1_GLOB0_BLEND1);
	SWTri_AddDrawTriFunc(true, true, true, true, 0x0555, false, TodDrawTriangle_0555_TEX1_TALPHA1_MOD1_GLOB1_BLEND0);
	SWTri_AddDrawTriFunc(true, true, true, true, 0x0555, true, TodDrawTriangle_0555_TEX1_TALPHA1_MOD1_GLOB1_BLEND1);
}

std::string TodGetCurrentLevelName()
{
	return "Unknown level";
}

bool TodHasUsedCheatKeys()
{
	return false;
}

bool TodAppCloseRequest()
{
	return false;
}

intptr_t TodPickFromWeightedArray(const TodWeightedArray* theArray, int theCount)
{
	return TodPickArrayItemFromWeightedArray(theArray, theCount)->mItem;
}

TodWeightedArray* TodPickArrayItemFromWeightedArray(const TodWeightedArray* theArray, int theCount)
{
	if (theCount <= 0)
		return nullptr;

	int aTotalWeight = 0;
	for (int i = 0; i < theCount; i++)
	{
		aTotalWeight += theArray[i].mWeight;
	}
	TOD_ASSERT(aTotalWeight > 0);

	aTotalWeight = Sexy::Rand(aTotalWeight);

	for (int i = 0; i < theCount; i++)
	{
		aTotalWeight -= theArray[i].mWeight;
		if (aTotalWeight < 0)
		{
			return (TodWeightedArray*)&theArray[i];
		}
	}

	TOD_ASSERT(false);
	return nullptr;
}

TodWeightedGridArray* TodPickFromWeightedGridArray(const TodWeightedGridArray* theArray, int theCount)
{
	if (theCount <= 0)
		return nullptr;

	int aTotalWeight = 0;
	for (int i = 0; i < theCount; i++)
	{
		aTotalWeight += theArray[i].mWeight;
	}
	TOD_ASSERT(aTotalWeight > 0);

	aTotalWeight = Sexy::Rand(aTotalWeight);

	for (int i = 0; i < theCount; i++)
	{
		aTotalWeight -= theArray[i].mWeight;
		if (aTotalWeight < 0)
		{
			return (TodWeightedGridArray*)&theArray[i];
		}
	}

	TOD_ASSERT(false);
	return nullptr;
}

float TodCalcSmoothWeight(float aWeight, float aLastPicked, float aSecondLastPicked)
{
	if (aWeight < 1E-6f)
	{
		return 0.0f;
	}

	float aExpectedLength1 = 1.0f / aWeight;								// theLastPicked 的期望值
	float aExpectedLength2 = aExpectedLength1 * 2.0f;						// theSecondLastPicked 的期望值
	float aAdvancedLength1 = aLastPicked + 1.0f - aExpectedLength1;			// 相较于 theLastPicked 的期望值，提前的轮数
	float aAdvancedLength2 = aSecondLastPicked + 1.0f - aExpectedLength2;	// 相较于 theSecondLastPicked 的期望值，提前的轮数
	float aFactor1 = 1.0f + aAdvancedLength1 / aExpectedLength1 * 2.0f;		// = aWeight * aLastPicked * 2 + aWeight * 2 - 1
	float aFactor2 = 1.0f + aAdvancedLength2 / aExpectedLength2 * 2.0f;		// = aSecondLastPicked * aWeight + aWeight - 1
	float aFactorFinal = ClampFloat(aFactor1 * 0.75f + aFactor2 * 0.25f, 0.01f, 100.0f);
	return aWeight * aFactorFinal;
}

int TodPickFromSmoothArray(TodSmoothArray* theArray, int theCount)
{
	float aTotalWeight = 0.0f;
	for (int i = 0; i < theCount; i++)
	{
		aTotalWeight += theArray[i].mWeight;
	}
	TOD_ASSERT(aTotalWeight > 0.0f);

	float aNormalizeFactor = 1.0f / aTotalWeight;
	float aTotalAdjustedWeight = 0.0f;
	for (int j = 0; j < theCount; j++)
	{
		aTotalAdjustedWeight += TodCalcSmoothWeight(theArray[j].mWeight * aNormalizeFactor, theArray[j].mLastPicked, theArray[j].mSecondLastPicked);
	}
	TOD_ASSERT(aTotalAdjustedWeight > 0.0f);

	float aRandWeight = Rand(aTotalAdjustedWeight);
	float aAccumulatedWeight = 0.0f;
	int k;
	for (k = 0; k < theCount - 1; k++)
	{
		aAccumulatedWeight += TodCalcSmoothWeight(theArray[k].mWeight * aNormalizeFactor, theArray[k].mLastPicked, theArray[k].mSecondLastPicked);
		if (aRandWeight <= aAccumulatedWeight)
		{
			break;
		}
	}

	TodUpdateSmoothArrayPick(theArray, theCount, k);
	return theArray[k].mItem;
}

void TodUpdateSmoothArrayPick(TodSmoothArray* theArray, int theCount, int thePickIndex)
{
	for (int i = 0; i < theCount; i++)
	{
		if (theArray[i].mWeight > 0.0f)
		{
			theArray[i].mLastPicked += 1.0f;
			theArray[i].mSecondLastPicked += 1.0f;
		}
	}

	theArray[thePickIndex].mSecondLastPicked = theArray[thePickIndex].mLastPicked;
	theArray[thePickIndex].mLastPicked = 0.0f;
}

float TodCurveQuad(float theTime)
{
	return theTime * theTime;
}

float TodCurveInvQuad(float theTime)
{
	return 2 * theTime - theTime * theTime;
}

float TodCurveS(float theTime)
{
	return 3 * theTime * theTime - 2 * theTime * theTime * theTime;
}

float TodCurveInvQuadS(float theTime)
{
	//float aVal = 2 * (theTime - theTime * theTime);
	//return theTime <= 0.5 ? aVal : 1 - aVal;
	if (theTime <= 0.5f)
	{
		return TodCurveInvQuad(theTime * 2.0f) * 0.5f;
	}
	return TodCurveQuad((theTime - 0.5f) * 2.0f) * 0.5f + 0.5f;
}

float TodCurveBounce(float theTime)
{
	return 1 - fabs(2 * theTime - 1);
}

float TodCurveQuadS(float theTime)
{
	if (theTime <= 0.5f)
	{
		return TodCurveQuad(theTime * 2.0f) * 0.5f;
	}
	return TodCurveInvQuad((theTime - 0.5f) * 2.0f) * 0.5f + 0.5f;
}

float TodCurveCubic(float theTime)
{
	return theTime * theTime * theTime;
}

float TodCurveInvCubic(float theTime)
{
	return (theTime - 1.0f) * (theTime - 1.0f) * (theTime - 1.0f) + 1.0f;
}

float TodCurveCubicS(float theTime)
{
	if (theTime <= 0.5f)
	{
		return TodCurveCubic(theTime * 2.0f) * 0.5f;
	}
	return TodCurveInvCubic((theTime - 0.5f) * 2.0f) * 0.5f + 0.5f;
}

float TodCurvePoly(float theTime, float thePoly)
{
	return static_cast<float>(pow(theTime, thePoly));
}

float TodCurveInvPoly(float theTime, float thePoly)
{
	return static_cast<float>(pow(theTime - 1.0f, thePoly)) + 1.0f;
}

float TodCurvePolyS(float theTime, float thePoly)
{
	if (theTime <= 0.5f)
	{
		return TodCurvePoly(theTime * 2.0f, thePoly) * 0.5f;
	}
	return TodCurveInvPoly((theTime - 0.5f) * 2.0f, thePoly) * 0.5f + 0.5f;
}

float TodCurveCircle(float theTime)
{
	if (theTime > 1 - 1E-6f)
	{
		return 1.0f;
	}
	return 1.0f - static_cast<float>(sqrt(1.0f - theTime * theTime));
}

float TodCurveInvCircle(float theTime)
{
	if (theTime < 1E-06f)
	{
		return 0.0f;
	}
	return static_cast<float>(sqrt(1.0f - (theTime - 1.0f) * (theTime - 1.0f)));
}

float TodCurveEvaluate(float theTime, float thePositionStart, float thePositionEnd, TodCurves theCurve)
{
	float aWarpedTime = 0;
	switch (theCurve)
	{
	case TodCurves::CURVE_CONSTANT:				aWarpedTime = 0;													break;
	case TodCurves::CURVE_LINEAR:				aWarpedTime = theTime;												break;
	case TodCurves::CURVE_EASE_IN:				aWarpedTime = TodCurveQuad(theTime);								break;
	case TodCurves::CURVE_EASE_OUT:				aWarpedTime = TodCurveInvQuad(theTime);								break;
	case TodCurves::CURVE_EASE_IN_OUT:			aWarpedTime = TodCurveS(TodCurveS(theTime));						break;
	case TodCurves::CURVE_EASE_IN_OUT_WEAK:		aWarpedTime = TodCurveS(theTime);									break;
	case TodCurves::CURVE_FAST_IN_OUT:			aWarpedTime = TodCurveInvQuadS(TodCurveInvQuadS(theTime));			break;
	case TodCurves::CURVE_FAST_IN_OUT_WEAK:		aWarpedTime = TodCurveInvQuadS(theTime);							break;
	case TodCurves::CURVE_BOUNCE:				aWarpedTime = TodCurveBounce(theTime);								break;
	case TodCurves::CURVE_BOUNCE_FAST_MIDDLE:	aWarpedTime = TodCurveQuad(TodCurveBounce(theTime));				break;
	case TodCurves::CURVE_BOUNCE_SLOW_MIDDLE:	aWarpedTime = TodCurveInvQuad(TodCurveBounce(theTime));				break;
	case TodCurves::CURVE_SIN_WAVE:				aWarpedTime = sinf(2 * PI * theTime);								break;
	case TodCurves::CURVE_EASE_SIN_WAVE:		aWarpedTime = sinf(2 * PI * TodCurveS(theTime));					break;
	default:									TOD_ASSERT(false);														break;
	}
	return (thePositionEnd - thePositionStart) * aWarpedTime + thePositionStart;
}

float TodCurveEvaluateClamped(float theTime, float thePositionStart, float thePositionEnd, TodCurves theCurve)
{
	if (theTime <= 0.0f)
	{
		return thePositionStart;
	}

	if (theTime >= 1.0f)
	{
		if (theCurve == TodCurves::CURVE_BOUNCE ||
			theCurve == TodCurves::CURVE_BOUNCE_FAST_MIDDLE ||
			theCurve == TodCurves::CURVE_BOUNCE_SLOW_MIDDLE ||
			theCurve == TodCurves::CURVE_SIN_WAVE ||
			theCurve == TodCurves::CURVE_EASE_SIN_WAVE)
		{
			return thePositionStart;
		}
		else
		{
			return thePositionEnd;
		}
	}

	return TodCurveEvaluate(theTime, thePositionStart, thePositionEnd, theCurve);
}

float TodAnimateCurveFloatTime(float theTimeStart, float theTimeEnd, float theTimeAge, float thePositionStart, float thePositionEnd, TodCurves theCurve)
{
	float aWarpedAge = (theTimeAge - theTimeStart) / (theTimeEnd - theTimeStart);
	return TodCurveEvaluateClamped(aWarpedAge, thePositionStart, thePositionEnd, theCurve);
}

float TodAnimateCurveFloat(int theTimeStart, int theTimeEnd, int theTimeAge, float thePositionStart, float thePositionEnd, TodCurves theCurve)
{
	//return TodAnimateCurveFloatTime(theTimeStart, theTimeEnd, theTimeAge, thePositionStart, thePositionEnd, theCurve);

	float aWarpedAge = (theTimeAge - theTimeStart) / static_cast<float>(theTimeEnd - theTimeStart);
	return TodCurveEvaluateClamped(aWarpedAge, thePositionStart, thePositionEnd, theCurve);
}

// GOTY @Patoke: 0x51BEA0
int TodAnimateCurve(int theTimeStart, int theTimeEnd, int theTimeAge, int thePositionStart, int thePositionEnd, TodCurves theCurve)
{
	return FloatRoundToInt(TodAnimateCurveFloat(theTimeStart, theTimeEnd, theTimeAge, thePositionStart, thePositionEnd, theCurve));
}

int RandRangeInt(int theMin, int theMax)
{
	TOD_ASSERT(theMin <= theMax);
	return Rand(theMax - theMin + 1) + theMin;
}

float RandRangeFloat(float theMin, float theMax)
{
	TOD_ASSERT(theMin <= theMax);
	return Rand(theMax - theMin) + theMin;
}

void TodDrawString(Graphics* g, const std::string& theText, int thePosX, int thePosY, _Font* theFont, const Color& theColor, DrawStringJustification theJustification)
{
	std::string aFinalString = TodStringTranslate(theText);

	int aPosX = thePosX;
	if (theJustification == DrawStringJustification::DS_ALIGN_RIGHT || theJustification == DrawStringJustification::DS_ALIGN_RIGHT_VERTICAL_MIDDLE)
	{
		aPosX -= theFont->StringWidth(aFinalString);
	}
	else if (theJustification == DrawStringJustification::DS_ALIGN_CENTER || theJustification == DrawStringJustification::DS_ALIGN_CENTER_VERTICAL_MIDDLE)
	{
		aPosX -= theFont->StringWidth(aFinalString) / 2;
	}

	theFont->DrawString(g, aPosX, thePosY, aFinalString, theColor, g->mClipRect);
}

void TodDrawImageCelScaled(Graphics* g, Image* theImageStrip, int thePosX, int thePosY, int theCelCol, int theCelRow, float theScaleX, float theScaleY)
{
	TOD_ASSERT(theCelCol >= 0 && theCelCol < theImageStrip->mNumCols);
	TOD_ASSERT(theCelRow >= 0 && theCelRow < theImageStrip->mNumRows);

	int aCelWidth = theImageStrip->GetCelWidth();
	int aCelHeight = theImageStrip->GetCelHeight();
	Rect aSrcRect(aCelWidth * theCelCol, aCelHeight * theCelRow, aCelWidth, aCelHeight);
	Rect aDestRect(thePosX, thePosY, FloatRoundToInt(aCelWidth * theScaleX), FloatRoundToInt(aCelHeight * theScaleY));
	g->DrawImage(theImageStrip, aDestRect, aSrcRect);
}

static const int POOL_SIZE = 4096;
static RenderCommand gRenderCommandPool[POOL_SIZE];
static RenderCommand* gRenderTail[256];
static RenderCommand* gRenderHead[256];

void TodDrawStringMatrix(Graphics* g, const _Font* theFont, const SexyMatrix3& theMatrix, const std::string& theString, const Color& theColor)
{
	std::string aFinalString = TodStringTranslate(theString);

	memset(gRenderTail, 0, sizeof(gRenderTail));
	memset(gRenderHead, 0, sizeof(gRenderHead));
	ImageFont* aFont = reinterpret_cast<ImageFont*>(const_cast<_Font*>(theFont));
	if (!aFont->mFontData->mInitialized)
		return;

	aFont->Prepare();
	int aCurXPos = 0;
	int aCurPoolIdx = 0;
	for (int aCharNum = 0; aCharNum < static_cast<int>(aFinalString.size()); aCharNum++)
	{
		char aChar = aFont->GetMappedChar(aFinalString[aCharNum]);
		char aNextChar = '\0';
		if (aCharNum < static_cast<int>(aFinalString.size()) - 1)
		{
			aNextChar = aFont->GetMappedChar(aFinalString[aCharNum + 1]);
		}

		int aMaxXPos = aCurXPos;
		for (auto aKernItr = aFont->mActiveLayerList.begin(); aKernItr != aFont->mActiveLayerList.end(); aKernItr++)
		{
			FontLayer* aLayer = aKernItr->mBaseFontLayer;
			CharData* aCharData = aLayer->GetCharData(aChar);
			double aScale = aFont->mScale;
			int aLayerPointSize = aLayer->mPointSize;
			if (aLayerPointSize)
			{
				aScale *= static_cast<float>(aFont->mPointSize) / static_cast<float>(aLayerPointSize);
			}

			int anImageX, anImageY, aCharWidth, aSpacing;
			if (aScale == 1.0f)
			{
				anImageX = aCharData->mOffset.mX + aLayer->mOffset.mX + aCurXPos;
				anImageY = aCharData->mOffset.mY + aLayer->mOffset.mY - aLayer->mAscent;
				aCharWidth = aCharData->mWidth;

				if (aNextChar == '\0')
				{
					aSpacing = 0;
				}
				else
				{
					aSpacing = aLayer->mSpacing;

					//aSpacing += aCharData->mKerningOffsets[aNextChar];
					auto anItr = aCharData->mKerningOffsets.find(aNextChar);
					if (anItr != aCharData->mKerningOffsets.end())
					{
						aSpacing += anItr->second;
					}
				}
			}
			else
			{
				anImageX = aCurXPos + floor((aCharData->mOffset.mX + aLayer->mOffset.mX) * aScale);
				anImageY = -floor((aLayer->mAscent - aLayer->mOffset.mY - aCharData->mOffset.mY) * aScale);
				aCharWidth = aCharData->mWidth * aScale;

				if (aNextChar == '\0')
				{
					aSpacing = 0;
				}
				else
				{
					aSpacing = aLayer->mSpacing;

					//aSpacing += aCharData->mKerningOffsets[aNextChar] * aScale;
					auto anItr = aCharData->mKerningOffsets.find(aNextChar);
					if (anItr != aCharData->mKerningOffsets.end())
					{
						aSpacing += anItr->second * aScale;
					}
				}
			}

			Color aColor;
			aColor.mRed = std::min(aLayer->mColorAdd.mRed + theColor.mRed * aLayer->mColorMult.mRed / 255, 255);
			aColor.mGreen = std::min(aLayer->mColorAdd.mGreen + theColor.mGreen * aLayer->mColorMult.mGreen / 255, 255);
			aColor.mBlue = std::min(aLayer->mColorAdd.mBlue + theColor.mBlue * aLayer->mColorMult.mBlue / 255, 255);
			aColor.mAlpha = std::min(aLayer->mColorAdd.mAlpha + theColor.mAlpha * aLayer->mColorMult.mAlpha / 255, 255);
			int anOrder = aCharData->mOrder + aLayer->mBaseOrder;

			if (aCurPoolIdx >= POOL_SIZE)
				break;

			RenderCommand* aRenderCommand = &gRenderCommandPool[aCurPoolIdx++];
			aRenderCommand->mImage = aKernItr->mScaledImage;
			aRenderCommand->mColor = aColor;
			aRenderCommand->mDest[0] = anImageX;
			aRenderCommand->mDest[1] = anImageY;
			//aRenderCommand->mSrc[0] = aKernItr->mScaledCharImageRects[aChar].mX;
			//aRenderCommand->mSrc[1] = aKernItr->mScaledCharImageRects[aChar].mY;
			//aRenderCommand->mSrc[2] = aKernItr->mScaledCharImageRects[aChar].mWidth;
			//aRenderCommand->mSrc[3] = aKernItr->mScaledCharImageRects[aChar].mHeight;
			aRenderCommand->mSrc[0] = aKernItr->mScaledCharImageRects.find(aChar)->second.mX;
			aRenderCommand->mSrc[1] = aKernItr->mScaledCharImageRects.find(aChar)->second.mY;
			aRenderCommand->mSrc[2] = aKernItr->mScaledCharImageRects.find(aChar)->second.mWidth;
			aRenderCommand->mSrc[3] = aKernItr->mScaledCharImageRects.find(aChar)->second.mHeight;
			aRenderCommand->mMode = aLayer->mDrawMode;
			aRenderCommand->mUseAlphaCorrection = aLayer->mUseAlphaCorrection;
			aRenderCommand->mNext = nullptr;

			int anOrderIdx = std::clamp(anOrder + 128, 0, 255);
			if (gRenderTail[anOrderIdx])
			{
				gRenderTail[anOrderIdx]->mNext = aRenderCommand;
				gRenderTail[anOrderIdx] = aRenderCommand;
			}
			else
			{
				gRenderHead[anOrderIdx] = aRenderCommand;
				gRenderTail[anOrderIdx] = aRenderCommand;
			}

			//aCurXPos += aSpacing + aCharWidth;
			//if (aCurXPos > aMaxXPos)
			//{
			//	aMaxXPos = aCurXPos;
			//}
			aMaxXPos = std::max(aMaxXPos, aCurXPos + aSpacing + aCharWidth);
		}

		aCurXPos = aMaxXPos;
	}

	for (int aPoolIdx = 0; aPoolIdx < 256; aPoolIdx++)
	{
		RenderCommand* aRenderCommand = gRenderHead[aPoolIdx];

		while (aRenderCommand)
		{
			int aDrawMode = g->GetDrawMode();
			if (aRenderCommand->mMode != -1)
			{
				aDrawMode = aRenderCommand->mMode;
			}

			if (aRenderCommand->mImage)
			{
				Rect aSrcRect(aRenderCommand->mSrc[0], aRenderCommand->mSrc[1], aRenderCommand->mSrc[2], aRenderCommand->mSrc[3]);
				SexyTransform2D aTransform;
				float aPosX = aSrcRect.mWidth * 0.5f + aRenderCommand->mDest[0];
				float aPosY = aSrcRect.mHeight * 0.5f + aRenderCommand->mDest[1];
				SexyMatrix3Translation(aTransform, aPosX, aPosY);
				SexyMatrix3Multiply(aTransform, theMatrix, aTransform);
				TodBltMatrix(g, aRenderCommand->mImage, aTransform, g->mClipRect, aRenderCommand->mColor, aDrawMode, aSrcRect);
			}
			
			aRenderCommand = aRenderCommand->mNext;
		}
	}
}

// GOTY @Patoke: 0x51C863
void TodDrawImageCelF(Graphics* g, Image* theImageStrip, float thePosX, float thePosY, int theCelCol, int theCelRow)
{
	TOD_ASSERT(theCelCol >= 0 && theCelCol < theImageStrip->mNumCols);
	TOD_ASSERT(theCelRow >= 0 && theCelRow < theImageStrip->mNumRows);

	int aCelWidth = theImageStrip->GetCelWidth();
	int aCelHeight = theImageStrip->GetCelHeight();
	Rect theSrcRect(aCelWidth * theCelCol, aCelHeight * theCelRow, aCelWidth, aCelHeight);
	g->DrawImageF(theImageStrip, thePosX, thePosY, theSrcRect);
}

void SexyMatrix3Translation(SexyMatrix3& m, float x, float y)
{
	m.m02 += x;
	m.m12 += y;
}

void TodScaleTransformMatrix(SexyMatrix3& m, float x, float y, float theScaleX, float theScaleY)
{
	m.m00 = theScaleX;
	m.m10 = 0.0f;
	m.m20 = 0.0f;
	m.m01 = 0.0f;
	m.m11 = theScaleY;
	m.m21 = 0.0f;
	m.m02 = x;
	m.m12 = y;
	m.m22 = 1.0f;
}

void TodScaleRotateTransformMatrix(SexyMatrix3& m, float x, float y, float rad, float theScaleX, float theScaleY)
{
	m.m00 = cos(rad) * theScaleX;
	m.m10 = -sin(rad) * theScaleX;
	m.m20 = 0.0f;
	m.m01 = sin(rad) * theScaleY;
	m.m11 = cos(rad) * theScaleY;
	m.m21 = 0.0f;
	m.m02 = x;
	m.m12 = y;
	m.m22 = 1.0f;
}

void SexyMatrix3ExtractScale(const SexyMatrix3& m, float& theScaleX, float& theScaleY)
{
	float kx = atan2(m.m00, m.m10);
	if (abs(kx) < PI / 4 || abs(kx) > 4 * PI / 3)
	{
		theScaleX = m.m10 / cos(kx);
	}
	else
	{
		theScaleX = m.m00 / sin(kx);
	}

	float ky = atan2(m.m11, m.m01);
	if (abs(ky) < PI / 4 || abs(ky) > 4 * PI / 3)
	{
		theScaleY = m.m01 / cos(ky);
	}
	else
	{
		theScaleY = m.m11 / sin(ky);
	}
}

void TodMarkImageForSanding(Image* theImage)
{
	((MemoryImage*)theImage)->mRenderFlags |= RENDERIMAGEFLAG_SANDING;
}

void TodSandImageIfNeeded(Image* theImage)
{
	MemoryImage* aImage = (MemoryImage*)theImage;
	/*if (TestBit(aImage->mRenderFlags, RENDERIMAGEFLAG_SANDING))*/ // UB shift by a billion
	if (aImage->mRenderFlags & RENDERIMAGEFLAG_SANDING)
	{
		FixPixelsOnAlphaEdgeForBlending(theImage);
		((MemoryImage*)theImage)->mRenderFlags &= ~RENDERIMAGEFLAG_SANDING; // Unset the sanding flag
		//SetBit((unsigned int&)aImage->mRenderFlags, RENDERIMAGEFLAG_SANDING, false);  // 清除标记 Also UB!?!
	}
}

void TodBltMatrix(Graphics* g, Image* theImage, const SexyMatrix3& theTransform, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect& theSrcRect)
{
	float aOffsetX = 0.0f;
	float aOffsetY = 0.0f;
	if (gSexyAppBase->Is3DAccelerated())
	{
		aOffsetX -= 0.5f;
		aOffsetY -= 0.5f;
	}
	else if (theDrawMode == Graphics::DRAWMODE_ADDITIVE)
	{
		gTodTriangleDrawAdditive = true;
	}

	TodSandImageIfNeeded(theImage);

	if (theClipRect.mX != 0 || theClipRect.mY != 0 || theClipRect.mWidth != BOARD_WIDTH || theClipRect.mHeight != BOARD_HEIGHT)
	{
		g->mDestImage->BltMatrix(theImage, aOffsetX, aOffsetY, theTransform, theClipRect, theColor, theDrawMode, theSrcRect, g->mLinearBlend);
	}
	else if (GLImage::Check3D(g->mDestImage))
	{
		theImage->mDrawn = true;
		GLInterface* aInterface = ((GLImage*)g->mDestImage)->mGLInterface;
		aInterface->BltTransformed(theImage, nullptr, theColor, theDrawMode, theSrcRect, theTransform, g->mLinearBlend, aOffsetX, aOffsetY, true);
	}
	else
	{
		Rect aBufFixClipRect(0, 0, BOARD_WIDTH + 1, BOARD_HEIGHT + 1);
		g->mDestImage->BltMatrix(theImage, aOffsetX, aOffsetY, theTransform, aBufFixClipRect, theColor, theDrawMode, theSrcRect, g->mLinearBlend);
	}

	gTodTriangleDrawAdditive = false;
}

void TodDrawImageCelCenterScaledF(Graphics* g, Image* theImageStrip, float thePosX, float thePosY, int theCelCol, float theScaleX, float theScaleY)
{
	TOD_ASSERT(theCelCol >= 0 && theCelCol < theImageStrip->mNumCols);

	int aCelWidth = theImageStrip->GetCelWidth();
	int aCelHeight = theImageStrip->GetCelHeight();
	Rect aSrcRect(aCelWidth * theCelCol, 0, aCelWidth, aCelHeight);
	if (theScaleX == 1.0f && theScaleY == 1.0f)
	{
		g->DrawImageF(theImageStrip, thePosX, thePosY, aSrcRect);
		return;
	}

	float aTransX = aCelWidth * 0.5f + thePosX + g->mTransX;
	float aTransY = aCelHeight * 0.5f + thePosY + g->mTransY;

	SexyMatrix3 aTransform;
	aTransform.m00 = theScaleX;
	aTransform.m10 = 0.0f;
	aTransform.m20 = 0.0f;
	aTransform.m01 = 0.0f;
	aTransform.m11 = theScaleY;
	aTransform.m21 = 0.0f;
	aTransform.m02 = aTransX;
	aTransform.m12 = aTransY;
	aTransform.m22 = 1.0f;

	const Color& aColor = g->mColorizeImages ? g->mColor : Color::White;
	TodBltMatrix(g, theImageStrip, aTransform, g->mClipRect, aColor, g->mDrawMode, aSrcRect);
}

void TodDrawImageCelScaledF(Graphics* g, Image* theImageStrip, float thePosX, float thePosY, int theCelCol, int theCelRow, float theScaleX, float theScaleY)
{
	//(void)theCelRow;
	TOD_ASSERT(theCelCol >= 0 && theCelCol < theImageStrip->mNumCols);

	int aCelWidth = theImageStrip->GetCelWidth();
	int aCelHeight = theImageStrip->GetCelHeight();
	Rect aSrcRect(aCelWidth * theCelCol, aCelHeight * theCelRow, aCelWidth, aCelHeight);
	if (theScaleX == 1.0f && theScaleY == 1.0f)
	{
		g->DrawImageF(theImageStrip, thePosX, thePosY, aSrcRect);
		return;
	}

	float aTransX = aCelWidth * 0.5f * theScaleX + thePosX + g->mTransX;
	float aTransY = aCelHeight * 0.5f * theScaleY + thePosY + g->mTransY;

	SexyMatrix3 aTransform;
	aTransform.m00 = theScaleX;
	aTransform.m10 = 0.0f;
	aTransform.m20 = 0.0f;
	aTransform.m01 = 0.0f;
	aTransform.m11 = theScaleY;
	aTransform.m21 = 0.0f;
	aTransform.m02 = aTransX;
	aTransform.m12 = aTransY;
	aTransform.m22 = 1.0f;

	const Color& aColor = g->mColorizeImages ? g->mColor : Color::White;
	TodBltMatrix(g, theImageStrip, aTransform, g->mClipRect, aColor, g->mDrawMode, aSrcRect);
}

// GOTY @Patoke: 0x51CC90
void TodDrawImageScaledF(Graphics* g, Image* theImage, float thePosX, float thePosY, float theScaleX, float theScaleY)
{
	if (theScaleX == 1.0f && theScaleY == 1.0f)
	{
		g->DrawImageF(theImage, thePosX, thePosY);
		return;
	}

	Rect aSrcRect(0, 0, theImage->mWidth, theImage->mHeight);
	float aTransX = theImage->mWidth * 0.5f * theScaleX + thePosX + g->mTransX;
	float aTransY = theImage->mHeight * 0.5f * theScaleY + thePosY + g->mTransY;

	SexyMatrix3 aTransform;
	aTransform.m00 = theScaleX;
	aTransform.m10 = 0.0f;
	aTransform.m20 = 0.0f;
	aTransform.m01 = 0.0f;
	aTransform.m11 = theScaleY;
	aTransform.m21 = 0.0f;
	aTransform.m02 = aTransX;
	aTransform.m12 = aTransY;
	aTransform.m22 = 1.0f;

	const Color& aColor = g->mColorizeImages ? g->mColor : Color::White;
	TodBltMatrix(g, theImage, aTransform, g->mClipRect, aColor, g->mDrawMode, aSrcRect);
}

void TodDrawImageCenterScaledF(Graphics* g, Image* theImage, float thePosX, float thePosY, float theScaleX, float theScaleY)
{
	if (theScaleX == 1.0f && theScaleY == 1.0f)
	{
		g->DrawImageF(theImage, thePosX, thePosY);
		return;
	}

	Rect aSrcRect(0, 0, theImage->mWidth, theImage->mHeight);
	float aTransX = theImage->mWidth * 0.5f + thePosX + g->mTransX;
	float aTransY = theImage->mHeight * 0.5f + thePosY + g->mTransY;

	SexyMatrix3 aTransform;
	aTransform.m00 = theScaleX;
	aTransform.m10 = 0.0f;
	aTransform.m20 = 0.0f;
	aTransform.m01 = 0.0f;
	aTransform.m11 = theScaleY;
	aTransform.m21 = 0.0f;
	aTransform.m02 = aTransX;
	aTransform.m12 = aTransY;
	aTransform.m22 = 1.0f;

	const Color& aColor = g->mColorizeImages ? g->mColor : Color::White;
	TodBltMatrix(g, theImage, aTransform, g->mClipRect, aColor, g->mDrawMode, aSrcRect);
}

uint32_t AverageNearByPixels(MemoryImage* theImage, uint32_t* thePixel, int x, int y)
{
	int aRed = 0;
	int aGreen = 0;
	int aBlue = 0;
	int aBitsCount = 0;

	for (int i = -1; i <= 1; i++)  // 依次循环上方、当前、下方的一行
	{
		if (i == 0)  // 排除当前行
		{
			continue;
		}

		for (int j = -1; j <= 1; j++)  // 依次循环左方、当前、右方的一列
		{
			if ((x != 0 || j != -1) && (x != theImage->mWidth - 1 || j != 1) && (y != 0 || i != -1) && (y != theImage->mHeight - 1 || i != 1))
			{
				uint32_t aPixel = *(thePixel + i * theImage->mWidth + j);
				if (aPixel & 0xFF000000UL)  // 如果不是透明像素
				{
					aRed += (aPixel >> 16) & 0x000000FFUL;
					aGreen += (aPixel >> 8) & 0x000000FFUL;
					aBlue += aPixel & 0x000000FFUL;
					aBitsCount++;
				}
			}
		}
	}

	if (aBitsCount == 0)
		return 0;

	aRed /= aBitsCount;
	aRed = std::min(aRed, 255);
	aGreen /= aBitsCount;
	aGreen = std::min(aGreen, 255);
	aBlue /= aBitsCount;
	aBlue = std::min(aBlue, 255);
	return (aRed << 16) | (aGreen << 8) | (aBlue);
}

void FixPixelsOnAlphaEdgeForBlending(Image* theImage)
{
	MemoryImage* aImage = (MemoryImage*)theImage;
	if (aImage->mBits == nullptr)
		return;

	aImage->CommitBits();  // 分析 mHasTrans 和 mHasAlpha
	if (!aImage->mHasTrans)
		return;

	PerfTimer aTimer;
	aTimer.Start();

	uint32_t* aBitsPtr = aImage->mBits;
	for (int y = 0; y < theImage->mHeight; y++)
	{
		for (int x = 0; x < theImage->mWidth; x++)
		{
			if ((*aBitsPtr & 0xFF000000UL) == 0)  // 如果像素的不透明度为 0
			{
				*aBitsPtr = AverageNearByPixels(aImage, aBitsPtr, x, y);  // 计算该点周围非透明像素的平均颜色
			}

			aBitsPtr++;
		}
	}
	aImage->mBitsChangedCount++;

	int aDuration = std::max(aTimer.GetDuration(), 0.0);
	if (aDuration > 20)
	{
		TodTraceAndLog("LOADING:Long sanding '%s' %d ms on %s", theImage->mFilePath.c_str(), aDuration, gGetCurrentLevelName().c_str());
	}
}

void SexyMatrix3Transpose(const SexyMatrix3& m, SexyMatrix3 &r)
{
	SexyMatrix3 temp;
	temp.m00 = m.m00;
	temp.m01 = m.m10;
	temp.m02 = m.m20;
	temp.m10 = m.m01;
	temp.m11 = m.m11;
	temp.m12 = m.m21;
	temp.m20 = m.m02;
	temp.m21 = m.m12;
	temp.m22 = m.m22;
	
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			r.m[i][j] = temp.m[i][j];
		}
	}
}

void SexyMatrix3Inverse(const SexyMatrix3& m, SexyMatrix3 &r)
{
	float aDet = (m.m22 * m.m11 - m.m21 * m.m12) * m.m00- (m.m22 * m.m10 - m.m20 * m.m12) * m.m01 + (m.m21 * m.m10 - m.m20 * m.m11) * m.m02;
	float aInvDet = 1.0f / aDet;

	SexyMatrix3 temp;
	temp.m00 = (m.m22 * m.m11 - m.m21 * m.m12) * aInvDet;
	temp.m01 = (m.m02 * m.m21 - m.m22 * m.m01) * aInvDet;
	temp.m02 = (m.m12 * m.m01 - m.m02 * m.m11) * aInvDet;
	temp.m10 = (m.m20 * m.m12 - m.m22 * m.m10) * aInvDet;
	temp.m11 = (m.m00 * m.m22 - m.m02 * m.m20) * aInvDet;
	temp.m12 = (m.m02 * m.m10 - m.m12 * m.m00) * aInvDet;
	temp.m20 = (m.m21 * m.m10 - m.m20 * m.m11) * aInvDet;
	temp.m21 = (m.m20 * m.m01 - m.m21 * m.m00) * aInvDet;
	temp.m22 = (m.m00 * m.m11 - m.m10 * m.m01) * aInvDet;

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			r.m[i][j] = temp.m[i][j];
		}
	}
}

void SexyMatrix3Multiply(SexyMatrix3& m, const SexyMatrix3& l, const SexyMatrix3& r)
{
	//SexyMatrix3 temp = l * r;
	SexyMatrix3 temp;
	temp.m00 = l.m00 * r.m00 + l.m01 * r.m10 + l.m02 * r.m20;
	temp.m01 = l.m00 * r.m01 + l.m01 * r.m11 + l.m02 * r.m21;
	temp.m02 = l.m00 * r.m02 + l.m01 * r.m12 + l.m02 * r.m22;
	temp.m10 = l.m10 * r.m00 + l.m11 * r.m10 + l.m12 * r.m20;
	temp.m11 = l.m10 * r.m01 + l.m11 * r.m11 + l.m12 * r.m21;
	temp.m12 = l.m10 * r.m02 + l.m11 * r.m12 + l.m12 * r.m22;
	temp.m20 = l.m20 * r.m00 + l.m21 * r.m10 + l.m22 * r.m20;
	temp.m21 = l.m20 * r.m01 + l.m21 * r.m11 + l.m22 * r.m21;
	temp.m22 = l.m20 * r.m02 + l.m21 * r.m12 + l.m22 * r.m22;

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			m.m[i][j] = temp.m[i][j];
		}
	}
}

// GOTY @Patoke: 0x51D2C0
Color GetFlashingColor(uint32_t theCounter, int theFlashTime)
{
	int aTimeAge = static_cast<int>(theCounter % static_cast<uint32_t>(theFlashTime));
	int aTimeInf = theFlashTime / 2;
	//int aTimeDel = abs(aTimeInf - aTimeAge) / aTimeInf;
	// @Patoke: order wasn't like in binaries
	int aGrayness = ClampInt(200 * abs(aTimeInf - aTimeAge) / aTimeInf + 55, 0, 255);
	//int aGrayness = ClampInt(55 + 200 * abs(aTimeInf - aTimeAge)/ aTimeInf, 0, 255);
	return Color(aGrayness, aGrayness, aGrayness, 255);
}

Color ColorAdd(const Color& theColor1, const Color& theColor2)
{
	int r = theColor1.mRed + theColor2.mRed;
	int g = theColor1.mGreen + theColor2.mGreen;
	int b = theColor1.mBlue + theColor2.mBlue;
	int a = theColor1.mAlpha + theColor2.mAlpha;

	return Color(ClampInt(r, 0, 255), ClampInt(g, 0, 255), ClampInt(b, 0, 255), ClampInt(a, 0, 255));  // 线性减淡
}

// GOTY @Patoke: 0x51D3C0
int ColorComponentMultiply(int theColor1, int theColor2)
{
	return ClampInt(theColor1 * theColor2 / 255, 0, 255);  // 正片叠底
}

Color ColorsMultiply(const Color& theColor1, const Color& theColor2)
{
	return Color(
		ColorComponentMultiply(theColor1.mRed, theColor2.mRed), 
		ColorComponentMultiply(theColor1.mGreen, theColor2.mGreen),
		ColorComponentMultiply(theColor1.mBlue, theColor2.mBlue),
		ColorComponentMultiply(theColor1.mAlpha, theColor2.mAlpha)
	);  // 正片叠底
}

// GOTY @Patoke: inlined 0x51D4C0
bool TodLoadResources(const std::string& theGroup)
{
	return ((TodResourceManager*)gSexyAppBase->mResourceManager)->TodLoadResources(theGroup);
}

// GOTY @Patoke: 0x51D4C0
bool TodResourceManager::TodLoadResources(const std::string& theGroup)
{
	if (IsGroupLoaded(theGroup))
		return true;

	PerfTimer aTimer;
	aTimer.Start();

	StartLoadResources(theGroup);
	while (!gSexyAppBase->mShutdown && TodLoadNextResource());
	if (gSexyAppBase->mShutdown)
		return false;

	if (HadError())
	{
		gSexyAppBase->ShowResourceError(true);
		return false;
	}

	if (gExtractResourcesByName && !gExtractResourcesByName(this, theGroup.c_str()))
	{
		gSexyAppBase->ShowResourceError(true);
		return false;
	}

	mLoadedGroups.insert(theGroup);

	int aDuration = std::max(aTimer.GetDuration(), 0.0);
	if (aDuration > 20)
	{
		TodTraceAndLog("LOADED: '%s' %d ms on %s", theGroup.c_str(), aDuration, gGetCurrentLevelName().c_str());
	}

	return true;
}

void TodAddImageToMap(SharedImageRef* theImage, const std::string& thePath)
{ 
	((TodResourceManager*)gSexyAppBase->mResourceManager)->AddImageToMap(theImage, thePath);
}

void TodResourceManager::AddImageToMap(SharedImageRef* theImage, const std::string& thePath)
{
	TOD_ASSERT(mImageMap.find(thePath) == mImageMap.end());

	ImageRes* aImageRes = new ImageRes();
	aImageRes->mImage = *theImage;
	aImageRes->mPath = thePath;
	mImageMap.insert(ResMap::value_type(thePath, aImageRes));
}

bool TodLoadNextResource()
{
	return ((TodResourceManager*)gSexyAppBase->mResourceManager)->TodLoadNextResource();
}

bool TodResourceManager::TodLoadNextResource()
{
	//GetTickCount();
	TodHesitationTrace("preres");

	while (mCurResGroupListItr != mCurResGroupList->end())
	{
		BaseRes* aRes = *mCurResGroupListItr;
		if (aRes->mFromProgram)
			continue;

		switch (aRes->mType)
		{
		case ResType_Image:
		{
			ImageRes* anImageRes = (ImageRes*)aRes;
			if ((GLImage*)anImageRes->mImage != nullptr)
			{
				mCurResGroupListItr++;
				continue;
			}

			break;
		}

		case ResType_Sound:
		{
			SoundRes* aSoundRes = (SoundRes*)aRes;
			if (aSoundRes->mSoundId != -1)
			{
				mCurResGroupListItr++;
				continue;
			}

			break;
		}

		case ResType_Font:
		{
			FontRes* aFontRes = (FontRes*)aRes;
			if (aFontRes->mFont != nullptr)
			{
				mCurResGroupListItr++;
				continue;
			}

			break;
		}
		}

		if (!LoadNextResource())
			break;

		if (aRes->mType == ResType::ResType_Image)
		{
			ImageRes* anImageRes = (ImageRes*)aRes;
			Image* aImage = (Image*)anImageRes->mImage;
			if (aImage != nullptr)
			{
				TodMarkImageForSanding(aImage);
			}
		}

		//GetTickCount();
		TodHesitationTrace("Loading: '%s'", aRes->mPath.c_str());
		TodHesitationTrace("resource '%s'", aRes->mPath.c_str());
		return true;
	}

	return false;
}

bool TodFindImagePath(Image* theImage, std::string* thePath)
{
	return ((TodResourceManager*)gSexyAppBase->mResourceManager)->FindImagePath(theImage, thePath);
}

// @Patoke implemented
bool TodFindFontPath(_Font* theFont, std::string* thePath) {
	return ((TodResourceManager*)gSexyAppBase->mResourceManager)->FindFontPath(theFont, thePath);
}

bool TodResourceManager::FindFontPath(_Font* theFont, std::string* thePath)
{
	for (auto anItr = mFontMap.begin(); anItr != mFontMap.end(); anItr++)
	{
		FontRes* aFontRes = (FontRes*)anItr->second;
		_Font* aFont = (_Font*)aFontRes->mFont;
		if (aFont == theFont)
		{
			*thePath = anItr->first;
			return true;
		}
	}
	return false;
}

bool TodResourceManager::FindImagePath(Image* theImage, std::string* thePath)
{
	for (auto anItr = mImageMap.begin(); anItr != mImageMap.end(); anItr++)
	{
		ImageRes* aImageRes = (ImageRes*)anItr->second;
		Image* aImage = (Image*)aImageRes->mImage;
		if (aImage == theImage)
		{
			*thePath = anItr->first;
			return true;
		}
	}
	return false;
}

TodAllocator gGlobalAllocators[MAX_GLOBAL_ALLOCATORS];
int gNumGlobalAllocators = 0;

TodAllocator* FindGlobalAllocator(int theSize)
{
	for (int i = 0; i < gNumGlobalAllocators; i++)
	{
		if (gGlobalAllocators[i].mItemSize == theSize)
		{
			return &gGlobalAllocators[i];
		}
	}

	TOD_ASSERT(gNumGlobalAllocators < MAX_GLOBAL_ALLOCATORS - 1);

	TodAllocator* pAllocator = &gGlobalAllocators[gNumGlobalAllocators++];
	pAllocator->Initialize(16, theSize);
	return pAllocator;
}

void FreeGlobalAllocators()
{
	for (int i = 0; i < gNumGlobalAllocators; i++)
	{
		gGlobalAllocators[i].FreeAll();
	}

	gNumGlobalAllocators = 0;
}

std::string TodReplaceString(const std::string& theText, const char* theStringToFind, const std::string& theStringToSubstitute)
{
	std::string aFinalString = TodStringTranslate(theText);
	size_t aPos = aFinalString.find(theStringToFind);
	if (aPos != std::string::npos)
	{
		std::string aFinalStringToSubstitute = TodStringTranslate(theStringToSubstitute);
		aFinalString.replace(aPos, strlen(theStringToFind), aFinalStringToSubstitute);
	}

	return aFinalString;
}

std::string TodReplaceNumberString(const std::string& theText, const char* theStringToFind, int theNumber)
{
	std::string aFinalString = TodStringTranslate(theText);
	size_t aPos = aFinalString.find(theStringToFind);
	if (aPos != std::string::npos)
	{
		std::string aNumberString = StrFormat("%d", theNumber);
		aFinalString.replace(aPos, strlen(theStringToFind), aNumberString);
	}

	return aFinalString;
}

// GOTY @Patoke: 0x51DB00
bool TodIsPointInPolygon(const SexyVector2* thePolygonPoint, int theNumberPolygonPoints, const SexyVector2& theCheckPoint)
{
	TOD_ASSERT(theNumberPolygonPoints >= 3);

	for (int i = 0; i < theNumberPolygonPoints; i++)
	{
		const SexyVector2& cur = thePolygonPoint[i];
		const SexyVector2& nex = thePolygonPoint[i == theNumberPolygonPoints - 1 ? 0 : i + 1];

		SexyVector2 u = (nex - cur).Perp();
		SexyVector2 v = theCheckPoint - cur;
		if (u.Dot(v) < 0)
			return false;
	}
	return true;
}

int TodVsnprintf(char* theBuffer, int theSize, const char* theFormat, va_list theArgList)
{
	try
	{
		int aCount = vsnprintf(theBuffer, theSize, theFormat, theArgList);
		if (aCount == -1)
		{
			theBuffer[theSize - 1] = '\0';
			aCount = theSize - 1;
		}
		return aCount;
	}
	catch (std::exception&)
	{
		TOD_ASSERT(, "bad format string");
		return 1;
	}
}

int TodSnprintf(char* theBuffer, int theSize, const char* theFormat, ...)
{
	va_list argList;
	va_start(argList, theFormat);
	int aCount = TodVsnprintf(theBuffer, theSize, theFormat, argList);
	va_end(argList);

	return aCount;
}
