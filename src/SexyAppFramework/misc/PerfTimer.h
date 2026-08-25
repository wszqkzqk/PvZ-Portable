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

#ifndef __SEXY_PERFTIMER_H__
#define __SEXY_PERFTIMER_H__

#include "Common.h"

namespace Sexy
{

class PerfTimer
{
protected:
	int64_t mStart;
	double mDuration;
	bool mRunning;

	void CalcDuration();

public:
	PerfTimer();
	void Start();
	void Stop();
	void SetStartTime(int theTimeMillisecondsAgo);

	double GetDuration();
};

class SexyPerf
{
public:
	static void BeginPerf(bool measurePerfOverhead = false);
	static void EndPerf();
	static bool IsPerfOn();

	static void StartTiming(const char *theName);
	static void StopTiming(const char *theName);

	static std::string GetResults();
};

class SexyAutoPerf
{
public:
	const char *mName;
	bool mIsStarted;

	SexyAutoPerf(const char *theName) : mName(theName), mIsStarted(true) { SexyPerf::StartTiming(theName); }
	SexyAutoPerf(const char *theName, bool doStart) : mName(theName), mIsStarted(doStart)
	{
		if(doStart)
			SexyPerf::StartTiming(theName);
	}

	~SexyAutoPerf() { Stop(); }

	void Start()
	{
		if(!mIsStarted)
		{
			mIsStarted = true;
			SexyPerf::StartTiming(mName);
		}
	}

	void Stop()
	{
		if(mIsStarted)
		{
			SexyPerf::StopTiming(mName);
			mIsStarted = false;
		}
	}
};

} // namespace Sexy

//#define SEXY_PERF_ENABLED
#if defined(SEXY_PERF_ENABLED) && !defined(RELEASEFINAL)

#define SEXY_PERF_BEGIN(theName) SexyPerf::StartTiming(theName)
#define SEXY_PERF_END(theName) SexyPerf::StopTiming(theName)
#define SEXY_AUTO_PERF_MULTI(theName,theSuffix) SexyAutoPerf anAutoPerf##theSuffix(theName)
#define SEXY_AUTO_PERF_2(theName,theSuffix) SEXY_AUTO_PERF_MULTI(theName,theSuffix)
#define SEXY_AUTO_PERFL(theName) SEXY_AUTO_PERF_2(theName,__LINE__) // __LINE__ doesn't work correctly if Edit-and-Continue (/ZI) is enabled
#define SEXY_AUTO_PERF(theName) SEXY_AUTO_PERF_2(theName,UNIQUE)

#define SEXY_PERF_BEGIN_COND(theName,theCond) if(theCond) SexyPerf::StartTiming(theName)
#define SEXY_PERF_END_COND(theName,theCond) if(theCond) SexyPerf::StopTiming(theName)
#define SEXY_AUTO_PERF_MULTI_COND(theName,theSuffix,theCond) SexyAutoPerf anAutoPerf##theSuffix(theName,theCond);
#define SEXY_AUTO_PERF_COND_2(theName,theSuffix,theCond) SEXY_AUTO_PERF_MULTI_COND(theName,theSuffix,theCond);
#define SEXY_AUTO_PERF_CONDL(theName) SEXY_AUTO_PERF_COND_2(theName,__LINE__,theCond)
#define SEXY_AUTO_PERF_COND(theName) SEXY_AUTO_PERF_COND_2(theName,UNIQUE,theCond)

#else

#define SEXY_PERF_BEGIN(theName)
#define SEXY_PERF_END(theName)
#define SEXY_AUTO_PERF_MULTI(theName,theSuffix)
#define SEXY_AUTO_PERF(theName)

#define SEXY_PERF_BEGIN_COND(theName,theCond)
#define SEXY_PERF_END_COND(theName,theCond)
#define SEXY_AUTO_PERF_MULTI_COND(theName,theSuffix,theCond)
#define SEXY_AUTO_PERF_COND(theName)

#endif

#endif
