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

class PvzpHesitationBracket
{
public:
	char			mMessage[256];
	int				mBracketStartTime;

public:
	PvzpHesitationBracket(const char* /*theFormat*/, ...) { ; }
	~PvzpHesitationBracket() { ; }

	inline void		EndBracket() { ; }
};

void				PvzpLogLn(const char* theFormat, ...);
void				PvzpLogStringLn(const char* theMsg);
void				PvzpTrace(const char* theFormat, ...);
void				PvzpTraceMemory();
void				PvzpTraceAndLogLn(const char* theFormat, ...);
void				PvzpTraceWithoutSpamming(const char* theFormat, ...);
void				PvzpHesitationTrace(...);
void				PvzpAssertFailed(const char* theCondition, const char* theFile, int theLine, const char* theMsg = "", ...);
void		PvzpErrorMessageBox(const char* theMessage, const char* theTitle);

void*	PvzpMalloc(int theSize);
void		PvzpFree(void* theBlock);
void				PvzpAssertInitForApp();

#ifdef PVZ_DEBUG
#define PVZP_ASSERT(condition, ...) { \
if (!bool(condition)) { PvzpAssertFailed(""#condition, __FILE__, __LINE__, ##__VA_ARGS__); \
PvzpTraceMemory(); }\
}
#else
#define PVZP_ASSERT(condition, ...)
#endif

#endif
