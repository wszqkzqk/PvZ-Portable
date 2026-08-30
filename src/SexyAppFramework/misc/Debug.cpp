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

#include "Common.h"
#include "Debug.h"

#include <format>
#include <mutex>

#include <time.h>
#include <SDL.h>

#include "memmgr.h"

bool gInAssert = false;

using namespace Sexy;

struct SEXY_ALLOC_INFO
{
	int		size;
	char	file[512];
	int		line;
};
static bool gShowLeaks = false;
static bool gSexyAllocMapValid = false;
class SexyAllocMap : public std::map<void*,SEXY_ALLOC_INFO>
{
public:
	std::mutex mCrit;

public:
	SexyAllocMap() { gSexyAllocMapValid = true; }
	~SexyAllocMap()
	{
		if (gShowLeaks)
			SexyDumpUnfreed();

		gSexyAllocMapValid = false;
	}
};
static SexyAllocMap gSexyAllocMap;

void SexyMemAddTrack(void *addr,  int asize,  const char* fname, int lnum)
{
	if (!gSexyAllocMapValid)
		return;

	std::scoped_lock aCrit(gSexyAllocMap.mCrit);
	gShowLeaks = true;

	SEXY_ALLOC_INFO &info = gSexyAllocMap[addr];
	SDL_strlcpy(info.file, fname, sizeof(info.file));
	info.line = lnum;
	info.size = asize;
};

void SexyMemRemoveTrack(void* addr)
{
	if (!gSexyAllocMapValid)
		return;

	std::scoped_lock aCrit(gSexyAllocMap.mCrit);
	SexyAllocMap::iterator anItr = gSexyAllocMap.find(addr);
	if (anItr != gSexyAllocMap.end())
		gSexyAllocMap.erase(anItr);
};

void SexyDumpUnfreed()
{
	if (!gSexyAllocMapValid)
		return;

	std::scoped_lock aCrit(gSexyAllocMap.mCrit);
	int totalSize = 0;

#ifdef SEXY_DUMP_LEAKED_MEM
	int count = 0;
	int index = 0;
#endif

	FILE* f = fopen("mem_leaks.txt", "wt");
	if (!f)
		return;

	time_t aTime = time(nullptr);
	std::string aHeader = std::format("Memory Leak Report for {}", asctime(localtime(&aTime)));
	aHeader.pop_back();
	fprintf(f, "%s\n\n", aHeader.c_str());
	Sexy::LogInfoLn("\n{}", aHeader);
	for (SexyAllocMap::iterator i = gSexyAllocMap.begin(); i != gSexyAllocMap.end(); i++)
	{
		std::string aLine = std::format("{}({}) : Leak {} byte{}", i->second.file, i->second.line, i->second.size, i->second.size > 1 ? "s" : "");
		Sexy::LogInfoLn("{}", aLine);
		fprintf(f, "%s\n", aLine.c_str());

#ifdef SEXY_DUMP_LEAKED_MEM
		unsigned char* data = (unsigned char*)i->first;
		std::string aHexDump;
		std::string aAsciiDump;

		for (index = 0; index < i->second.size; index++)
		{
			unsigned char _c = *data;

			if (count == 0)
				aHexDump += '\t';
			aHexDump += std::format("{:02X} ", (unsigned int)_c);

			if ((_c < 32) || (_c > 126))
				_c = '.';

			char aPrintChar = static_cast<char>(_c);
			if (count == 0)
				aAsciiDump += '\t';
			aAsciiDump += aPrintChar;
			if (count == 7)
				aAsciiDump += ' ';

			if (++count == 16)
			{
				count = 0;
				std::string aDumpLine = std::format("{}\t{}\n", aHexDump, aAsciiDump);
				fprintf(f, "%s", aDumpLine.c_str());

				aHexDump.clear();
				aAsciiDump.clear();
			}

			data++;
		}

		if (count != 0)
		{
			fprintf(f, "%s", aHexDump.c_str());
			for (index = 0; index < 16 - count; index++)
				fprintf(f, "\t");

			fprintf(f, "%s", aAsciiDump.c_str());

			for (index = 0; index < 16 - count; index++)
				fprintf(f, ".");
		}

		count = 0;
		fprintf(f, "\n\n");

#endif // SEXY_DUMP_LEAKED_MEM

		totalSize += i->second.size;
	}


	std::string aSeparator = "-----------------------------------------------------------";
	fprintf(f, "%s\n", aSeparator.c_str());
	Sexy::LogInfoLn("{}", aSeparator);
	std::string aTotal = std::format("Total Unfreed: {} bytes ({}KB)", totalSize, totalSize / 1024);
	Sexy::LogInfoLn("{}", aTotal);
	fprintf(f, "%s\n\n", aTotal.c_str());
	fclose(f);
}
