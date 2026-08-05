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

#ifndef __TRAIL_H__
#define __TRAIL_H__

#include <cstdint>
#include "PvzpParticle.h"

#define MAX_TRAIL_TRIANGLES 38

enum TrailType : int32_t
{
	TRAIL_NONE = -1,
	TRAIL_ICE,
	NUM_TRAILS
};

enum TrailTracks : int32_t
{
	TRACK_WIDTH_OVER_LENGTH,
	TRACK_WIDTH_OVER_TIME,
	TRACK_ALPHA_OVER_LENGTH,
	TRACK_ALPHA_OVER_TIME,
	NUM_TRAIL_TRACKS
};

enum TrailFlags : int32_t
{
	TRAIL_FLAG_LOOPS = 0
};

class TrailParams
{
public:
	TrailType				mTrailType;
	const char*				mTrailFileName;
};

extern int gTrailParamArraySize;
extern TrailParams* gTrailParamArray;

extern TrailParams gLawnTrailArray[static_cast<int>(TrailType::NUM_TRAILS)];

class TrailDefinition
{
public:
	Image*					mImage = nullptr;
	int						mMaxPoints = 2;
	float					mMinPointDistance = 1.0f;
	int						mTrailFlags = 0;
	FloatParameterTrack		mTrailDuration{};
	FloatParameterTrack		mWidthOverLength{};
	FloatParameterTrack		mWidthOverTime{};
	FloatParameterTrack		mAlphaOverLength{};
	FloatParameterTrack		mAlphaOverTime{};

public:
	TrailDefinition() = default;
	~TrailDefinition() = default;
};
bool						TrailLoadADef(TrailDefinition* theTrailDef, const char* theTrailFileName);
void						TrailLoadDefinitions(TrailParams* theTrailParamArray, int theTrailParamArraySize);
void						TrailFreeDefinitions();

extern int gTrailDefCount;
extern TrailDefinition* gTrailDefArray;

class TrailPoint
{
public:
	SexyVector2				aPos;

public:
	TrailPoint();
};

class TrailHolder;

class Trail
{
public:
	TrailPoint				mTrailPoints[20];
	int32_t					mNumTrailPoints;
	bool					mDead;
	int32_t					mRenderOrder;
	int32_t					mTrailAge;
	int32_t					mTrailDuration;
	TrailDefinition*		mDefinition;
	TrailHolder*			mTrailHolder;
	float					mTrailInterp[4];
	SexyVector2				mTrailCenter;
	bool					mIsAttachment;
	Color					mColorOverride;

public:
	Trail();

	void					Update();
	void					Draw(Graphics* g);
	void					AddPoint(float x, float y);
	bool					GetNormalAtPoint(int nIndex, SexyVector2& theNormal);
};

class TrailHolder
{
public:
	DataArray<Trail>		mTrails;

public:
	TrailHolder() { ; }
	~TrailHolder() { DisposeHolder(); }

	void					InitializeHolder();
	void					DisposeHolder();
	Trail*					AllocTrail(int theRenderOrder, TrailType theTrailType);
	Trail*					AllocTrailFromDef(int theRenderOrder, TrailDefinition* theDefinition);
};

#endif
