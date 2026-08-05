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

#ifndef __SOUNDMANAGER_H__
#define __SOUNDMANAGER_H__

#include "Common.h"

namespace Sexy
{

class SoundInstance;

#define MAX_SOURCE_SOUNDS	256
#define MAX_CHANNELS		32

class SoundManager
{
public:
	SoundManager() {}
	virtual ~SoundManager() = default;

	virtual bool			Initialized() = 0;

	virtual bool			LoadSound(intptr_t theSfxID, const std::string& theFilename) = 0;
	virtual intptr_t		LoadSound(const std::string& theFilename) = 0;
	virtual void			ReleaseSound(intptr_t theSfxID) = 0;

	virtual void			SetVolume(double theVolume) = 0;
	virtual bool			SetBaseVolume(intptr_t theSfxID, double theBaseVolume) = 0;
	virtual bool			SetBasePan(intptr_t theSfxID, int theBasePan) = 0;

	virtual SoundInstance*	GetSoundInstance(intptr_t theSfxID) = 0;

	virtual void			ReleaseSounds() = 0;
	virtual void			ReleaseChannels() = 0;

	virtual double			GetMasterVolume() = 0;
	virtual void			SetMasterVolume(double theVolume) = 0;

	virtual void			Flush() = 0;
	virtual void			StopAllSounds() = 0;
	virtual intptr_t		GetFreeSoundId() = 0;
	virtual int				GetNumSounds() = 0;
};


}

#endif //__SOUNDMANAGER_H__
