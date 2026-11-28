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

#ifndef __SEXYAPPFRAMEWORK_FILESYSTEM_H__
#define __SEXYAPPFRAMEWORK_FILESYSTEM_H__

#include <cstdio>
#include <filesystem>
#include <string>
#include <string_view>

namespace Sexy
{

#ifdef _WIN32
inline std::filesystem::path PathFromU8(std::string_view s)
{
	return std::filesystem::path(std::u8string_view(reinterpret_cast<const char8_t*>(s.data()), s.size()));
}

inline std::string PathToU8(const std::filesystem::path& p)
{
	auto u8 = p.generic_u8string();
	return std::string(u8.begin(), u8.end());
}
#else
inline std::filesystem::path PathFromU8(std::string_view s) { return std::filesystem::path(s); }
inline std::string PathToU8(const std::filesystem::path& p) { return p.string(); }
#endif

constexpr char ToLowerAscii(char c) noexcept
{
	return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + ('a' - 'A')) : c;
}

// Transparent ASCII case-insensitive hash/equality for zero-allocation C++20 heterogeneous lookup.
struct CaseInsensitiveHash
{
	using is_transparent = void;
	size_t operator()(std::string_view theStr) const noexcept;
};

struct CaseInsensitiveEqual
{
	using is_transparent = void;
	bool operator()(std::string_view theLeft, std::string_view theRight) const noexcept;
};

// Lexical path helpers. Resource paths treat both '/' and '\' as separators on every platform.
bool				IsPathRooted(std::string_view thePath);
bool				IsCanonicalRelativePath(std::string_view thePath) noexcept;
std::string			CanonicalizePath(std::string_view thePath);

std::string			GetAppDataFolder();
void				SetAppDataFolder(std::string_view thePath);
std::string			GetAppDataPath(std::string_view theRelativePath);
const std::string&	GetResourceFolder();
void				SetResourceFolder(std::string_view thePath);
std::string			GetResourcePath(std::string_view theRelativePath);

// General disk access: UTF-8/Unicode-safe on all platforms, exact-case semantics on case-sensitive hosts.
FILE*				OpenDiskFile(const std::filesystem::path& thePath, const char* theMode);
bool				EraseDiskFile(const std::filesystem::path& thePath);

// Resource access: resolves paths against GetResourceFolder() with cached case-insensitive fallback
// for compatibility with Windows-authored resource packs and mods on case-sensitive filesystems.
std::filesystem::path	MakeResourcePath(std::string_view thePath);
FILE*					OpenResourceFile(std::string_view thePath, const char* theMode);
bool					ResourceFileExists(std::string_view thePath);

std::string			GetPathFrom(std::string_view theRelPath, std::string_view theDir);
void				MkDir(std::string_view theDir);
std::string			GetFileName(std::string_view thePath, bool noExtension = false);
std::string			GetFileDir(std::string_view thePath, bool withSlash = false);
std::string			RemoveTrailingSlash(std::string_view theDirectory);

}

#endif
