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

#include <time.h>
#include <cinttypes>
#include <stdarg.h>
#include <stdexcept>
#include <fstream>

#ifdef __SWITCH__
#include <switch.h>
#endif

#include "PvzpDebug.h"
#include "PvzpCommon.h"
#include "misc/Debug.h"
#include "../SexyAppFramework/Common.h"
#include "../SexyAppFramework/SexyAppBase.h"

using namespace Sexy;

static char gLogFileName[512];
static char gDebugDataFolder[512];

void PvzpErrorMessageBox(const char* theMessage, const char* theTitle)
{
#ifdef __SWITCH__
	ErrorApplicationConfig c;
	errorApplicationCreate(&c, theTitle, theMessage);
	errorApplicationShow(&c);
#else
	throw std::runtime_error("Error Box\n--" + std::string(theTitle) + "--\n" + theMessage);
#endif
}

void PvzpTraceMemory()
{
}

void* PvzpMalloc(int theSize)
{
	PVZP_ASSERT(theSize > 0);
	return malloc(theSize);
}

void PvzpFree(void* theBlock)
{
	if (theBlock != nullptr)
	{
		free(theBlock);
	}
}

void PvzpAssertFailed(const char* theCondition, const char* theFile, int theLine)
{
	PvzpAssertFailed(theCondition, theFile, theLine, "%s", "");
}

void PvzpAssertFailed(const char* theCondition, const char* theFile, int theLine, const char* theMsg, ...)
{
	va_list argList;
	va_start(argList, theMsg);
	std::string aFormattedMsg = Sexy::VFormat(theMsg, argList);
	va_end(argList);

	std::string aBuffer;
	if (*theCondition != '\0')
		aBuffer = Sexy::StrFormat("\n%s(%d)\nassertion failed: '%s'\n%s", theFile, theLine, theCondition, aFormattedMsg.c_str());
	else
		aBuffer = Sexy::StrFormat("\n%s(%d)\nassertion failed: %s", theFile, theLine, aFormattedMsg.c_str());

	PvzpTrace("%s", aBuffer.c_str());
	PvzpErrorMessageBox(aBuffer.c_str(), "Assertion failed");
	exit(0);
}

void PvzpLogLn(const char* theFormat, ...)
{
	va_list argList;
	va_start(argList, theFormat);
	std::string aBuffer = Sexy::VFormat(theFormat, argList);
	va_end(argList);

	if (!aBuffer.empty())
		PvzpLogStringLn(aBuffer.c_str());
}

void PvzpLogStringLn(const char* theMsg)
{
#ifdef PVZ_DEBUG
	std::ofstream f(Sexy::PathFromU8(gLogFileName), std::ios::app | std::ios::binary);
	if (!f)
	{
		Sexy::LogError("Failed to open log file '%s'", gLogFileName);
		return;
	}

	f << theMsg << '\n';
	if (!f)
	{
		Sexy::LogError("Failed to write to log file");
	}
#endif
}

void PvzpTrace(const char* theFormat, ...)
{
	va_list argList;
	va_start(argList, theFormat);
	std::string aBuffer = Sexy::VFormat(theFormat, argList);
	va_end(argList);

	if (!aBuffer.empty())
		Sexy::PrintF("%s", aBuffer.c_str());
}

void PvzpHesitationTrace(...)
{
}

void PvzpTraceAndLogLn(const char* theFormat, ...)
{
	va_list argList;
	va_start(argList, theFormat);
	std::string aBuffer = Sexy::VFormat(theFormat, argList);
	va_end(argList);

	if (aBuffer.empty())
		return;

	Sexy::PrintF("%s", aBuffer.c_str());
	PvzpLogStringLn(aBuffer.c_str());
}

void PvzpTraceWithoutSpamming(const char* theFormat, ...)
{
	static uint64_t gLastTraceTime = 0LL;
	uint64_t aTime = time(nullptr);
	if (aTime <= gLastTraceTime) // at most one trace per second
		return;

	gLastTraceTime = aTime;

	va_list argList;
	va_start(argList, theFormat);
	std::string aBuffer = Sexy::VFormat(theFormat, argList);
	va_end(argList);

	if (!aBuffer.empty())
		Sexy::PrintF("%s", aBuffer.c_str());
}

void PvzpAssertInitForApp()
{
	MkDir(GetAppDataPath("userdata"));
	std::string aRelativeUserPath = GetAppDataPath("userdata/");
	strcpy(gDebugDataFolder, aRelativeUserPath.c_str());
	strcpy(gLogFileName, gDebugDataFolder);
	strcpy(gLogFileName + strlen(gLogFileName), "log.txt");
	PVZP_ASSERT(strlen(gLogFileName) < 512);

	PvzpLogLn("Started %" PRIu64, static_cast<uint64_t>(time(nullptr)));
}
