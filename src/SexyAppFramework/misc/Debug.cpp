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

#include <mutex>

#include <time.h>
#include <stdarg.h>

#include "memmgr.h"

bool gInAssert = false;
// Seemingly unused
//extern bool gSexyDumpLeakedMem = false;

using namespace Sexy;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SexyMemAddTrack(void *addr,  int asize,  const char* fname, int lnum)
{
	if (!gSexyAllocMapValid)
		return;

	std::scoped_lock aCrit(gSexyAllocMap.mCrit);
	gShowLeaks = true;

	SEXY_ALLOC_INFO &info = gSexyAllocMap[addr];
	strncpy(info.file, fname, sizeof(info.file)-1);
	info.line = lnum;
	info.size = asize;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SexyMemRemoveTrack(void* addr)
{
	if (!gSexyAllocMapValid)
		return;

	std::scoped_lock aCrit(gSexyAllocMap.mCrit);
	SexyAllocMap::iterator anItr = gSexyAllocMap.find(addr);
	if (anItr != gSexyAllocMap.end())
		gSexyAllocMap.erase(anItr);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SexyDumpUnfreed()
{
	if (!gSexyAllocMapValid)
		return;

	std::scoped_lock aCrit(gSexyAllocMap.mCrit);
	SexyAllocMap::iterator i;
	int totalSize = 0;
	char buf[8192];

#ifdef SEXY_DUMP_LEAKED_MEM
	char hex_dump[1024];
	char ascii_dump[1024];
	int count = 0;
	int index = 0;
#endif

	FILE* f = fopen("mem_leaks.txt", "wt");
	if (!f)
		return;

	time_t aTime = time(nullptr);
	snprintf(buf, sizeof(buf), "Memory Leak Report for %s\n", asctime(localtime(&aTime)));
	fprintf(f, "%s", buf);
	Sexy::PrintF("\n%s", buf);
	for(i = gSexyAllocMap.begin(); i != gSexyAllocMap.end(); i++) 
	{
		snprintf(buf, sizeof(buf), "%s(%d) : Leak %d byte%s\n", i->second.file, i->second.line, i->second.size,i->second.size>1?"s":"");
		Sexy::PrintF("%s", buf);
		fprintf(f, "%s", buf);

#ifdef SEXY_DUMP_LEAKED_MEM
		unsigned char* data = (unsigned char*)i->first;

		for (index = 0; index < i->second.size; index++)
		{			
			unsigned char _c = *data;
			
			if (count == 0)
				sprintf(hex_dump, "\t%02X ", _c);
			else
				sprintf(hex_dump, "%s%02X ", hex_dump, _c);
		
			if ((_c < 32) || (_c > 126))
				_c = '.';

			if (count == 7)
				sprintf(ascii_dump, "%s%c ", ascii_dump, _c);
			else
				sprintf(ascii_dump, "%s%c", count == 0 ? "\t" : ascii_dump, _c);
			

			if (++count == 16)
			{
				count = 0;
				sprintf(buf, "%s\t%s\n", hex_dump, ascii_dump);
				fprintf(f, buf);

				memset((void*)hex_dump, 0, 1024);
				memset((void*)ascii_dump, 0, 1024);
			}

			data++;
		}

		if (count != 0)
		{
			fprintf(f, hex_dump);
			for (index = 0; index < 16 - count; index++)
				fprintf(f, "\t");

			fprintf(f, ascii_dump);

			for (index = 0; index < 16 - count; index++)
				fprintf(f, ".");
		}

		count = 0;
		fprintf(f, "\n\n");
		memset((void*)hex_dump, 0, 1024);	
		memset((void*)ascii_dump, 0, 1024);

#endif // SEXY_DUMP_LEAKED_MEM

		totalSize += i->second.size;
	}


	snprintf(buf, sizeof(buf), "-----------------------------------------------------------\n");
	fprintf(f, "%s", buf);
	Sexy::PrintF("%s", buf);
	snprintf(buf, sizeof(buf), "Total Unfreed: %d bytes (%dKB)\n\n", totalSize, totalSize / 1024);
	Sexy::PrintF("%s", buf);
	fprintf(f, "%s", buf);
}
