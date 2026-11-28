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

#include "FileSystem.h"

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <optional>
#include <unordered_map>

static std::filesystem::path gAppDataFolder;
static std::filesystem::path gResourceFolder;
static std::string gResourceFolderStr;

struct DirSnapshot
{
	bool mIsDirectory = false;
	std::unordered_map<std::string, bool, Sexy::CaseInsensitiveHash, Sexy::CaseInsensitiveEqual> mEntries;
};

static std::mutex gDirCacheMutex;
static std::unordered_map<std::string, DirSnapshot> gDirCache;

#if defined(__SWITCH__)
static constexpr bool IsAsciiAlphaNum(char c) noexcept
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}
#endif

static size_t SwitchMountLen(std::string_view thePath) noexcept
{
#if defined(__SWITCH__)
	const size_t aColonPos = thePath.find(':');
	if (aColonPos == std::string_view::npos || aColonPos == 0 || aColonPos + 1 >= thePath.size())
		return 0;

	const char aSeparator = thePath[aColonPos + 1];
	if (aSeparator != '/' && aSeparator != '\\')
		return 0;

	for (size_t i = 0; i < aColonPos; ++i)
	{
		if (!IsAsciiAlphaNum(thePath[i]) && thePath[i] != '_')
			return 0;
	}

	return aColonPos + 2;
#else
	(void)thePath;
	return 0;
#endif
}

static FILE* OpenFileRaw(const std::filesystem::path& thePath, const char* theMode)
{
#ifdef _WIN32
	wchar_t aWideMode[16]{};
	size_t aLen = 0;
	while (theMode[aLen] != '\0' && aLen + 1 < sizeof(aWideMode) / sizeof(aWideMode[0]))
	{
		aWideMode[aLen] = static_cast<unsigned char>(theMode[aLen]);
		++aLen;
	}
	return _wfopen(thePath.c_str(), aWideMode);
#else
	return std::fopen(thePath.c_str(), theMode);
#endif
}

static bool CanCreateFile(const char* theMode) noexcept
{
	return theMode != nullptr &&
		(theMode[0] == 'w' || theMode[0] == 'a' || std::strchr(theMode, '+') != nullptr);
}

static void InvalidateDirectoryLocked(std::filesystem::path theDir)
{
	if (theDir.empty())
	{
		std::error_code anError;
		theDir = std::filesystem::current_path(anError);
		if (anError)
		{
			gDirCache.clear();
			return;
		}
	}
	gDirCache.erase(Sexy::PathToU8(theDir.lexically_normal()));
}

static void InvalidateDirectory(const std::filesystem::path& theDir)
{
	std::scoped_lock aLock(gDirCacheMutex);
	InvalidateDirectoryLocked(theDir);
}

static void InvalidateFileCache()
{
	std::scoped_lock aLock(gDirCacheMutex);
	gDirCache.clear();
}

static std::filesystem::path NormalizeRootFolder(std::string_view thePath)
{
	if (thePath.empty())
		return {};

	std::filesystem::path aPath = Sexy::PathFromU8(thePath);
	if (!Sexy::IsPathRooted(thePath))
	{
		std::error_code anError;
		auto aAbs = std::filesystem::absolute(aPath, anError);
		if (!anError)
			aPath = std::move(aAbs);
	}
	aPath = aPath.lexically_normal();

	const size_t aMountLen = SwitchMountLen(Sexy::PathToU8(aPath));
	while (!aPath.empty() && aPath != aPath.root_path() && !aPath.has_filename())
	{
		auto aParent = aPath.parent_path();
		if (Sexy::PathToU8(aParent).size() < aMountLen)
			break;
		aPath = std::move(aParent);
	}
	return aPath;
}

#if !defined(_WIN32)
// Lazily scans a directory once; callers invalidate the entry on local mutations or failed opens.
static DirSnapshot& GetDirSnapshotLocked(const std::filesystem::path& theDir)
{
	auto [anItr, anInserted] = gDirCache.try_emplace(Sexy::PathToU8(theDir));
	DirSnapshot& aSnap = anItr->second;
	if (!anInserted)
		return aSnap;

	std::error_code aIterError;
	std::filesystem::directory_iterator aDirItr(theDir, std::filesystem::directory_options::skip_permission_denied, aIterError);
	if (aIterError)
		return aSnap;

	aSnap.mIsDirectory = true;
	for (const std::filesystem::directory_entry& aEntry : aDirItr)
	{
		std::error_code aTypeError;
		const bool aIsDir = aEntry.is_directory(aTypeError);
		std::string aName = Sexy::PathToU8(aEntry.path().filename());
		auto [anEntryItr, anEntryInserted] = aSnap.mEntries.try_emplace(aName, aIsDir);
		if (!anEntryInserted && aName < anEntryItr->first)
		{
			auto aNode = aSnap.mEntries.extract(anEntryItr);
			aNode.key() = std::move(aName);
			aNode.mapped() = aIsDir;
			aSnap.mEntries.insert(std::move(aNode));
		}
	}
	return aSnap;
}

static std::optional<std::filesystem::path> ResolveFromBase(
	std::filesystem::path theBase, std::string_view theRelPath, bool theAllowMissingTail)
{
	size_t aPos = 0;
	while (aPos < theRelPath.size())
	{
		while (aPos < theRelPath.size() && (theRelPath[aPos] == '/' || theRelPath[aPos] == '\\'))
			++aPos;
		if (aPos == theRelPath.size())
			break;

		const size_t aSegStart = aPos;
		size_t aEnd = theRelPath.find_first_of("/\\", aPos);
		const bool aIsLeaf = (aEnd == std::string_view::npos);
		if (aIsLeaf)
			aEnd = theRelPath.size();
		const std::string_view aSeg = theRelPath.substr(aSegStart, aEnd - aSegStart);
		aPos = aEnd;

		if (aSeg == ".")
			continue;
		if (aSeg == "..")
		{
			theBase = theBase.parent_path();
			continue;
		}

		std::scoped_lock aLock(gDirCacheMutex);
		DirSnapshot& aSnap = GetDirSnapshotLocked(theBase);
		if (!aSnap.mIsDirectory)
			return std::nullopt;

		auto anItr = aSnap.mEntries.find(aSeg);
		if (anItr != aSnap.mEntries.end())
		{
			if (!aIsLeaf && !anItr->second)
				return std::nullopt;
			theBase /= Sexy::PathFromU8(anItr->first);
		}
		else if (theAllowMissingTail)
		{
			theBase /= Sexy::PathFromU8(theRelPath.substr(aSegStart));
			return theBase;
		}
		else
		{
			return std::nullopt;
		}
	}
	return theBase;
}

static std::optional<std::filesystem::path> ResolveResourcePath(
	const std::filesystem::path& theFullPath, bool theAllowMissingTail)
{
	if (theFullPath.empty())
		return std::nullopt;

	const std::filesystem::path aNorm = theFullPath.lexically_normal();
	if (!gResourceFolder.empty())
	{
		const auto aRel = aNorm.lexically_relative(gResourceFolder);
		if (aRel.empty() || *aRel.begin() == "..")
			return std::nullopt;
		return ResolveFromBase(gResourceFolder, Sexy::PathToU8(aRel), theAllowMissingTail);
	}

	if (aNorm.has_root_path())
		return std::nullopt;

	std::error_code anError;
	auto aCwd = std::filesystem::current_path(anError);
	if (anError)
		return std::nullopt;
	return ResolveFromBase(std::move(aCwd), Sexy::PathToU8(aNorm), theAllowMissingTail);
}
#endif

size_t Sexy::CaseInsensitiveHash::operator()(std::string_view theStr) const noexcept
{
	uint64_t aHash = 14695981039346656037ull;
	for (char c : theStr)
		aHash = (aHash ^ static_cast<unsigned char>(ToLowerAscii(c))) * 1099511628211ull;
	return static_cast<size_t>(aHash);
}

bool Sexy::CaseInsensitiveEqual::operator()(std::string_view theLeft, std::string_view theRight) const noexcept
{
	return theLeft.size() == theRight.size() &&
		std::equal(theLeft.begin(), theLeft.end(), theRight.begin(),
			[](char a, char b) { return ToLowerAscii(a) == ToLowerAscii(b); });
}

bool Sexy::IsPathRooted(std::string_view thePath)
{
	if (thePath.empty())
		return false;
#ifdef _WIN32
	return PathFromU8(thePath).has_root_path();
#else
	return thePath.front() == '/' || SwitchMountLen(thePath) != 0;
#endif
}

bool Sexy::IsCanonicalRelativePath(std::string_view thePath) noexcept
{
	if (thePath.empty() || thePath.front() == '/' || thePath.front() == '\\' ||
		thePath.back() == '/' || thePath.back() == '\\' || SwitchMountLen(thePath) != 0)
	{
		return false;
	}
#ifdef _WIN32
	if (thePath.size() >= 2 && thePath[1] == ':')
		return false;
#endif

	while (!thePath.empty())
	{
		const size_t aSep = thePath.find_first_of("/\\");
		const std::string_view aSeg = (aSep == std::string_view::npos) ? thePath : thePath.substr(0, aSep);
		if (aSeg.empty() || aSeg == "." || aSeg == ".." || (aSep != std::string_view::npos && thePath[aSep] == '\\'))
			return false;
		if (aSep == std::string_view::npos)
			return true;
		thePath.remove_prefix(aSep + 1);
	}
	return false;
}

std::string Sexy::CanonicalizePath(std::string_view thePath)
{
	if (IsCanonicalRelativePath(thePath))
		return std::string(thePath);

	std::string aResult;
	aResult.reserve(thePath.size());

	size_t aPos = 0;
	bool aRooted = false;
	if (!thePath.empty() && (thePath.front() == '/' || thePath.front() == '\\'))
	{
		aResult = '/';
		aPos = 1;
		aRooted = true;
	}
	else if (const size_t aMountLen = SwitchMountLen(thePath); aMountLen != 0)
	{
		aResult.assign(thePath.substr(0, aMountLen));
		aResult.back() = '/';
		aPos = aMountLen;
		aRooted = true;
	}
#ifdef _WIN32
	else if (thePath.size() >= 2 && thePath[1] == ':')
	{
		aResult.assign(thePath.substr(0, 2));
		aPos = 2;
		if (aPos < thePath.size() && (thePath[aPos] == '/' || thePath[aPos] == '\\'))
		{
			aResult += '/';
			++aPos;
			aRooted = true;
		}
	}
#endif

	const size_t aRootLen = aResult.size();
	for (;;)
	{
		size_t aEnd = thePath.find_first_of("/\\", aPos);
		if (aEnd == std::string_view::npos)
			aEnd = thePath.size();
		const std::string_view aSeg = thePath.substr(aPos, aEnd - aPos);

		if (!aSeg.empty() && aSeg != ".")
		{
			bool aCanPop = (aSeg == "..") && aResult.size() > aRootLen;
			if (aCanPop && !aRooted)
			{
				const size_t aLastSlash = aResult.rfind('/');
				const std::string_view aLast = (aLastSlash == std::string::npos)
					? std::string_view(aResult)
					: std::string_view(aResult).substr(aLastSlash + 1);
				aCanPop = (aLast != "..");
			}

			if (aCanPop)
			{
				const size_t aSlash = aResult.rfind('/');
				aResult.resize((aSlash == std::string::npos || aSlash < aRootLen) ? aRootLen : aSlash);
			}
			else if (aSeg != ".." || !aRooted)
			{
				if (aResult.size() > aRootLen)
					aResult += '/';
				aResult += aSeg;
			}
		}

		if (aEnd == thePath.size())
			break;
		aPos = aEnd + 1;
	}

	return aResult;
}

std::string Sexy::GetAppDataFolder()
{
	return PathToU8(gAppDataFolder);
}

void Sexy::SetAppDataFolder(std::string_view thePath)
{
	gAppDataFolder = NormalizeRootFolder(thePath);
}

std::string Sexy::GetAppDataPath(std::string_view theRelativePath)
{
	return PathToU8(gAppDataFolder / PathFromU8(theRelativePath));
}

const std::string& Sexy::GetResourceFolder()
{
	return gResourceFolderStr;
}

void Sexy::SetResourceFolder(std::string_view thePath)
{
	auto aFolder = NormalizeRootFolder(thePath);
	std::scoped_lock aLock(gDirCacheMutex);
	gResourceFolder = std::move(aFolder);
	gResourceFolderStr = PathToU8(gResourceFolder);
	gDirCache.clear();
}

std::string Sexy::GetResourcePath(std::string_view theRelativePath)
{
	return PathToU8(gResourceFolder / PathFromU8(theRelativePath));
}

FILE* Sexy::OpenDiskFile(const std::filesystem::path& thePath, const char* theMode)
{
	if (thePath.empty() || theMode == nullptr)
		return nullptr;

	FILE* aFile = OpenFileRaw(thePath, theMode);
	if (aFile && CanCreateFile(theMode))
		InvalidateDirectory(thePath.parent_path());
	return aFile;
}

bool Sexy::EraseDiskFile(const std::filesystem::path& thePath)
{
	if (thePath.empty())
		return false;

	std::error_code anError;
	const bool aRemoved = std::filesystem::remove(thePath, anError);
	if (aRemoved)
		InvalidateDirectory(thePath.parent_path());
	return aRemoved;
}

std::filesystem::path Sexy::MakeResourcePath(std::string_view thePath)
{
	if (IsCanonicalRelativePath(thePath))
		return gResourceFolder / PathFromU8(thePath);
	if (thePath.empty())
		return gResourceFolder;
#ifdef _WIN32
	// Windows natively accepts both separators and dot segments; keep UNC/drive paths intact.
	if (IsPathRooted(thePath))
		return PathFromU8(thePath);
#endif

	std::string aCanonical = CanonicalizePath(thePath);
	if (IsPathRooted(aCanonical))
		return PathFromU8(aCanonical);
	return gResourceFolder / PathFromU8(aCanonical);
}

FILE* Sexy::OpenResourceFile(std::string_view thePath, const char* theMode)
{
	if (thePath.empty() || theMode == nullptr)
		return nullptr;

	const auto aFullPath = MakeResourcePath(thePath);
	if (CanCreateFile(theMode))
	{
		auto aTarget = aFullPath;
		std::error_code anError;
#if !defined(_WIN32)
		if (!std::filesystem::exists(aFullPath, anError))
		{
			if (auto aResolved = ResolveResourcePath(aFullPath, true))
				aTarget = std::move(*aResolved);
		}
#endif
		FILE* aFile = OpenFileRaw(aTarget, theMode);
		if (aFile)
			InvalidateDirectory(aTarget.parent_path());
		return aFile;
	}

	FILE* aFile = OpenFileRaw(aFullPath, theMode);
#if !defined(_WIN32)
	if (!aFile && (errno == ENOENT || errno == ENOTDIR))
	{
		if (auto aResolved = ResolveResourcePath(aFullPath, false))
		{
			aFile = OpenFileRaw(*aResolved, theMode);
			if (!aFile && (errno == ENOENT || errno == ENOTDIR) && *aResolved != aFullPath)
			{
				InvalidateDirectory(aResolved->parent_path());
				if (auto aRetry = ResolveResourcePath(aFullPath, false))
					aFile = OpenFileRaw(*aRetry, theMode);
			}
		}
	}
#endif
	return aFile;
}

bool Sexy::ResourceFileExists(std::string_view thePath)
{
	if (thePath.empty())
		return false;

	const auto aFullPath = MakeResourcePath(thePath);
	std::error_code anError;
	if (std::filesystem::exists(aFullPath, anError))
		return true;

#if !defined(_WIN32)
	if (!anError || anError == std::errc::no_such_file_or_directory || anError == std::errc::not_a_directory)
	{
		if (auto aResolved = ResolveResourcePath(aFullPath, false))
		{
			if (std::filesystem::exists(*aResolved, anError))
				return true;
			if (*aResolved != aFullPath)
			{
				InvalidateDirectory(aResolved->parent_path());
				if (auto aRetry = ResolveResourcePath(aFullPath, false))
					return std::filesystem::exists(*aRetry, anError);
			}
		}
	}
#endif
	return false;
}

std::string Sexy::GetPathFrom(std::string_view theRelPath, std::string_view theDir)
{
	std::filesystem::path relPath = PathFromU8(theRelPath);
	if (IsPathRooted(theRelPath))
		return PathToU8(relPath.lexically_normal());

	std::filesystem::path baseDir;
	if (!theDir.empty())
		baseDir = PathFromU8(theDir);
	else
	{
		std::error_code ec;
		baseDir = std::filesystem::current_path(ec);
		if (ec)
			return PathToU8(relPath.lexically_normal());
	}

	return PathToU8((baseDir / relPath).lexically_normal());
}

void Sexy::MkDir(std::string_view theDir)
{
	if (theDir.empty())
		return;

	std::error_code ec;
	if (std::filesystem::create_directories(PathFromU8(theDir), ec))
		InvalidateFileCache();
}

std::string Sexy::GetFileName(std::string_view thePath, bool noExtension)
{
	std::filesystem::path path = PathFromU8(thePath);
	if (!path.has_filename())
		return "";

	if (noExtension)
		return PathToU8(path.stem());

	return PathToU8(path.filename());
}

std::string Sexy::GetFileDir(std::string_view thePath, bool withSlash)
{
	std::filesystem::path path = PathFromU8(thePath);
	std::filesystem::path parent = path.parent_path();
	if (parent.empty())
		return "";

	std::string result = PathToU8(parent);
	if (withSlash && !result.empty())
		result += '/';

	return result;
}

std::string Sexy::RemoveTrailingSlash(std::string_view theDirectory)
{
	if (theDirectory.empty())
		return std::string(theDirectory);

	return PathToU8(PathFromU8(theDirectory).lexically_normal());
}
