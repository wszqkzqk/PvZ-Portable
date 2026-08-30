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

#ifndef __PVZPDEBUG_H__
#define __PVZPDEBUG_H__

#include <format>

#include "../SexyAppFramework/Common.h"

class PvzpHesitationBracket
{
public:
	char			mMessage[256];
	int				mBracketStartTime;

public:
	template<typename... Args>
	PvzpHesitationBracket(std::string_view, Args&&...) { ; }
	~PvzpHesitationBracket() { ; }

	inline void		EndBracket() { ; }
};

template<typename... Args>
void				PvzpHesitationTrace(std::string_view, Args&&...) { ; }

void				PvzpTraceMemory();
void				PvzpAssertFailed(const char* theCondition, const char* theFile, int theLine);
void				PvzpAssertReport(const char* theCondition, const char* theFile, int theLine, std::string_view theMsg);
void				PvzpErrorMessageBox(std::string_view theMessage, std::string_view theTitle);

void*				PvzpMalloc(int theSize);
void				PvzpFree(void* theBlock);

template<typename... Args>
void				PvzpLogLn(std::format_string<Args...> theFmt, Args&&... theArgs)
{
	Sexy::DispatchLogLn(Sexy::SexyLogPriority::Info, std::vformat(theFmt.get(), std::make_format_args(theArgs...)));
}

template<typename... Args>
void				PvzpTraceWithoutSpamming(std::format_string<Args...> theFmt, Args&&... theArgs)
{
	static uint64_t gLastTraceTime = 0LL;
	uint64_t aTime = std::time(nullptr);
	if (aTime <= gLastTraceTime) // at most one trace per second
		return;

	gLastTraceTime = aTime;
	Sexy::DispatchLogLn(Sexy::SexyLogPriority::Info, std::vformat(theFmt.get(), std::make_format_args(theArgs...)));
}

template<typename... Args>
void				PvzpAssertFailed(const char* theCondition, const char* theFile, int theLine, std::format_string<Args...> theMsg, Args&&... theArgs)
{
	PvzpAssertReport(theCondition, theFile, theLine, std::vformat(theMsg.get(), std::make_format_args(theArgs...)));
}

#ifdef PVZ_DEBUG
#define PVZP_ASSERT(condition, ...) { \
if (!bool(condition)) { PvzpAssertFailed(""#condition, __FILE__, __LINE__, ##__VA_ARGS__); \
PvzpTraceMemory(); }\
}
#else
#define PVZP_ASSERT(condition, ...)
#endif

#endif
