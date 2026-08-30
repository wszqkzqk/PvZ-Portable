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

#include <stdexcept>
#include <format>

#ifdef __SWITCH__
#include <switch.h>
#endif

#include "PvzpDebug.h"
#include "PvzpCommon.h"
#include "misc/Debug.h"
#include "../SexyAppFramework/Common.h"
#include "../SexyAppFramework/SexyAppBase.h"

using namespace Sexy;

void PvzpErrorMessageBox(std::string_view theMessage, std::string_view theTitle)
{
#ifdef __SWITCH__
	ErrorApplicationConfig c;
	errorApplicationCreate(&c, std::string(theTitle).c_str(), std::string(theMessage).c_str());
	errorApplicationShow(&c);
#else
	throw std::runtime_error("Error Box\n--" + std::string(theTitle) + "--\n" + std::string(theMessage));
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
	PvzpAssertReport(theCondition, theFile, theLine, "");
}

void PvzpAssertReport(const char* theCondition, const char* theFile, int theLine, std::string_view theMsg)
{
	std::string aBuffer;
	if (*theCondition != '\0')
		aBuffer = std::format("\n{}({})\nassertion failed: '{}'\n{}", theFile, theLine, theCondition, theMsg);
	else
		aBuffer = std::format("\n{}({})\nassertion failed: {}", theFile, theLine, theMsg);

	Sexy::DispatchLogLn(Sexy::SexyLogPriority::Error, aBuffer);
	PvzpErrorMessageBox(aBuffer, "Assertion failed");
	exit(1);
}
