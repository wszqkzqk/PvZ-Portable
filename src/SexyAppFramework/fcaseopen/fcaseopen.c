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

#include "fcaseopen.h"

#if defined(_WIN32)
#include <direct.h> // chdir on Windows
#include <malloc.h> // alloca on Windows (MSYS2/UCRT)
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#else
#include <unistd.h> // fix "implicit declaration of function chdir"
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <errno.h>
#endif
#ifdef __HAIKU__
// this function seems to not exist under haiku??
char *strsep(char **stringp, const char *delim)
{
	char *begin, *end;
	begin = *stringp;
	if (begin == NULL) return NULL;

	if (delim[0] == '\0' || begin[0] == '\0')
	{
		*stringp = NULL;
		return begin;
	}

	end = strpbrk(begin, delim);
	if (end)
	{
		*end = '\0';
		*stringp = end + 1;
	}
	else
	{
		*stringp = NULL;
	}
	return begin;
}
#endif

#if !defined(_WIN32)
// r must have strlen(path) + 3 bytes
static int casepath(char const *path, char *r)
{
	size_t l = strlen(path);
	char *p = (char *)alloca(l + 1);
	strcpy(p, path);
	size_t rl = 0;

	DIR *d;
	if (p[0] == '/')
	{
		d = opendir("/");
		p = p + 1;
	}
	else
	{
		d = opendir(".");
		r[0] = '.';
		r[1] = 0;
		rl = 1;
	}

	int last = 0;
	char *c = strsep(&p, "/");
	while (c)
	{
		if (!d)
		{
			return 0;
		}

		if (last)
		{
			closedir(d);
			return 0;
		}

		r[rl] = '/';
		rl += 1;
		r[rl] = 0;

		struct dirent *e = readdir(d);
		while (e)
		{
			if (strcasecmp(c, e->d_name) == 0)
			{
				strcpy(r + rl, e->d_name);
				rl += strlen(e->d_name);

				closedir(d);
				d = opendir(r);

				break;
			}

			e = readdir(d);
		}

		if (!e)
		{
			strcpy(r + rl, c);
			rl += strlen(c);
			last = 1;
		}

		c = strsep(&p, "/");
	}

	if (d) closedir(d);
	return 1;
}

// r must have strlen(base) + strlen(path) + 3 bytes
static int casepathat(char const *base, char const *path, char *r)
{
	if (!base || base[0] == '\0')
		base = ".";

	// Copy base directly - it's already case-correct
	strcpy(r, base);
	size_t rl = strlen(r);
	if (rl == 0)
	{
		r[0] = '.';
		r[1] = 0;
		rl = 1;
	}

	// Ensure trailing separator
	if (r[rl - 1] != '/' && r[rl - 1] != '\\')
	{
		r[rl++] = '/';
		r[rl] = 0;
	}

	// Open base directory directly (no case correction needed)
	DIR *d = opendir(base);
	if (!d)
		return 0;

	// Prepare to parse the relative path
	char *p = (char *)alloca(strlen(path) + 1);
	strcpy(p, path);

	int last = 0;
	char *c = strsep(&p, "/\\");
	while (c)
	{
		if (!d)
			return 0;

		if (last)
		{
			closedir(d);
			return 0;
		}

		// Skip empty components (e.g., from leading or double slashes)
		if (c[0] == 0)
		{
			c = strsep(&p, "/\\");
			continue;
		}

		r[rl] = '/';
		rl += 1;
		r[rl] = 0;

		struct dirent *e = readdir(d);
		while (e)
		{
			if (strcasecmp(c, e->d_name) == 0)
			{
				strcpy(r + rl, e->d_name);
				rl += strlen(e->d_name);

				closedir(d);
				d = opendir(r);
				break;
			}
			e = readdir(d);
		}

		if (!e)
		{
			strcpy(r + rl, c);
			rl += strlen(c);
			last = 1;
		}

		c = strsep(&p, "/\\");
	}

	if (d) closedir(d);
	return 1;
}
#endif

#if defined(_WIN32)
static wchar_t *utf8_to_wide_alloc(const char *utf8)
{
	if (!utf8)
		return NULL;
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	if (len <= 0)
		return NULL;
	wchar_t *wide = (wchar_t *)malloc(sizeof(wchar_t) * (size_t)len);
	if (!wide)
		return NULL;
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wide, len);
	return wide;
}
#endif

FILE *fcaseopen(char const *path, char const *mode)
{
#if defined(_WIN32)
	wchar_t *wpath = utf8_to_wide_alloc(path);
	wchar_t *wmode = utf8_to_wide_alloc(mode);
	if (!wpath || !wmode)
	{
		if (wpath) free(wpath);
		if (wmode) free(wmode);
		return NULL;
	}
	FILE *f = _wfopen(wpath, wmode);
	free(wpath);
	free(wmode);
#else
	FILE *f = fopen(path, mode);
#if !defined(_WIN32)
	if (!f)
	{
		char *r = (char *)alloca(strlen(path) + 3);
		if (casepath(path, r))
		{
			f = fopen(r, mode);
		}
	}
#endif
#endif
	return f;
}

int casechdir(char const *path)
{
#if !defined(_WIN32)
	char *r = (char *)alloca(strlen(path) + 3);
	if (casepath(path, r))
	{
		return chdir(r);
	}
	else
	{
		errno = ENOENT;
		return -1;
	}
#else
	wchar_t *wpath = utf8_to_wide_alloc(path);
	if (!wpath)
		return -1;
	int ret = _wchdir(wpath);
	free(wpath);
	return ret;
#endif
}

FILE *fcaseopenat(char const *base, char const *path, char const *mode)
{
#if !defined(_WIN32)
	if (!base || base[0] == '\0')
		base = ".";

	size_t baseLen = strlen(base);
	size_t pathLen = strlen(path);
	char *full = (char *)alloca(baseLen + pathLen + 3);

	strcpy(full, base);
	size_t fl = strlen(full);
	if (fl > 0 && full[fl - 1] != '/' && full[fl - 1] != '\\')
	{
		full[fl++] = '/';
		full[fl] = 0;
	}
	strcpy(full + fl, path);

	FILE *f = fopen(full, mode);
	if (!f)
	{
		char *r = (char *)alloca(baseLen + pathLen + 3);
		if (casepathat(base, path, r))
		{
			f = fopen(r, mode);
		}
	}
	return f;
#else
	if (!base || base[0] == '\0')
		return fcaseopen(path, mode);

	size_t baseLen = strlen(base);
	size_t pathLen = strlen(path);
	char *full = (char *)alloca(baseLen + pathLen + 3);
	strcpy(full, base);
	size_t fl = strlen(full);
	if (fl > 0 && full[fl - 1] != '/' && full[fl - 1] != '\\')
	{
		full[fl++] = '/';
		full[fl] = 0;
	}
	strcpy(full + fl, path);
	return fcaseopen(full, mode);
#endif
}
