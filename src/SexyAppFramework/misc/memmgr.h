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

#ifndef __SEXYMEMMGR_H__
#define __SEXYMEMMGR_H__

//						HOW TO USE THIS FILE
//
//			In the desired .CPP file (NOT header file), AFTER ALL of your
//	#include declarations, do a #include "memmgr.h" or whatever you renamed
//	this file to. It's very important that you do it only in the .cpp and
//	after every other include file, otherwise it won't compile.  The memory leaks
//  will appear in a file called mem_leaks.txt and they will also be printed out
//  in the output window when the program exits.
//


#include <list>
#include <stdio.h>
#include <stdlib.h>

extern void SexyDumpUnfreed();

#if defined(SEXY_MEMTRACE) && !defined(RELEASEFINAL)

/************************************************************************/
/* DO NOT CALL THESE TWO METHODS DIRECTLY								*/
/************************************************************************/
void SexyMemAddTrack(void* addr,  int asize,  const char *fname, int lnum);
void SexyMemRemoveTrack(void *addr);


//Replacement for the standard "new" operator, records size of allocation and
//the file/line number it was on
inline void* __cdecl operator new(unsigned int size, const char* file, int line)
{
	void* ptr = (void*)malloc(size);
	SexyMemAddTrack(ptr, size, file, line);
	return(ptr);
}

//Same as above, but for arrays
inline void* __cdecl operator new[](unsigned int size, const char* file, int line)
{
	void* ptr = (void*)malloc(size);
	SexyMemAddTrack(ptr, size, file, line);
	return(ptr);
}


// These single argument new operators allow vc6 apps to compile without errors
inline void* __cdecl operator new(unsigned int size)
{
	void* ptr = (void*)malloc(size);
	return(ptr);
}

inline void* __cdecl operator new[](unsigned int size)
{
	void* ptr = (void*)malloc(size);
	return(ptr);
}


//custom delete operators
inline void __cdecl operator delete(void* p)
{
	SexyMemRemoveTrack(p);
	free(p);
}

inline void __cdecl operator delete[](void* p)
{
	SexyMemRemoveTrack(p);
	free(p);
}

//needed in case in the constructor of the class we're newing, it throws an exception
inline void __cdecl operator delete(void* pMem, const char *file, int line)
{
	free(pMem);
}

inline void __cdecl operator delete[](void* pMem, const char *file, int line)
{
	free(pMem);
}


#define DEBUG_NEW new(__FILE__, __LINE__)
#define new DEBUG_NEW


#endif // SEXY_MEMTRACE


#endif
