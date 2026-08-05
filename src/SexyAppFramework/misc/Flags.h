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

#ifndef __FLAGS_H__
#define __FLAGS_H__

namespace Sexy
{

class FlagsMod
{
public:
	int						mAddFlags;
	int						mRemoveFlags;

public:
	FlagsMod()
	{
		mAddFlags = 0;
		mRemoveFlags = 0;
	}
};

inline void ModFlags(int& theFlags, const FlagsMod& theFlagMod)
{
	theFlags = (theFlags | theFlagMod.mAddFlags) & ~theFlagMod.mRemoveFlags;
}

inline int GetModFlags(int theFlags, const FlagsMod& theFlagMod)
{
	return (theFlags | theFlagMod.mAddFlags) & ~theFlagMod.mRemoveFlags;
}

class ModalFlags
{
public:
	int						mOverFlags;
	int						mUnderFlags;
	bool					mIsOver;

public:
	void					ModFlags(const FlagsMod& theFlagsMod)
	{
		Sexy::ModFlags(mOverFlags, theFlagsMod);
		Sexy::ModFlags(mUnderFlags, theFlagsMod);
	}

	int						GetFlags()
	{
		return mIsOver ? mOverFlags : mUnderFlags;
	}
};

class AutoModalFlags
{
public:
	ModalFlags*				mModalFlags;
	int						mOldOverFlags;
	int						mOldUnderFlags;

public:
	AutoModalFlags(ModalFlags* theModalFlags, const FlagsMod& theFlagMod)
	{
		mModalFlags = theModalFlags;
		mOldOverFlags = theModalFlags->mOverFlags;
		mOldUnderFlags = theModalFlags->mUnderFlags;
		theModalFlags->ModFlags(theFlagMod);
	}

	~AutoModalFlags()
	{
		mModalFlags->mOverFlags = mOldOverFlags;
		mModalFlags->mUnderFlags = mOldUnderFlags;
	}
};

}

#endif //__FLAGS_H__
