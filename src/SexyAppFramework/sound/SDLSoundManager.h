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

#ifndef __SDLSOUNDMANAGER_H__
#define __SDLSOUNDMANAGER_H__

#include "SoundManager.h"
#include <memory>
#include <SDL.h>
#include <SDL_mixer_ext/SDL_mixer_ext.h>

namespace Sexy
{

class SDLSoundInstance;

class SDLSoundManager : public SoundManager
{
	friend class SDLSoundInstance;

protected:
	bool					mInitializedMixer;
	Mix_Chunk*				mSourceSounds[MAX_SOURCE_SOUNDS];
	std::string				mSourceFileNames[MAX_SOURCE_SOUNDS];
	double					mBaseVolumes[MAX_SOURCE_SOUNDS];
	int						mBasePans[MAX_SOURCE_SOUNDS];
	std::unique_ptr<SDLSoundInstance>	mPlayingSounds[MAX_CHANNELS];
	double					mMasterVolume;
	uint64_t				mLastReleaseTick;
	int						mMixerFreq;
	uint16_t				mMixerFormat;
	int						mMixerChannels;

protected:
	int						FindFreeChannel();
	bool					LoadAUSound(intptr_t theSfxID, const std::string& theFilename);
	void					ReleaseFreeChannels();

public:
	SDLSoundManager();
	~SDLSoundManager() override;

	bool					Initialized() override;

	bool					LoadSound(intptr_t theSfxID, const std::string& theFilename) override;
	intptr_t				LoadSound(const std::string& theFilename) override;
	void					ReleaseSound(intptr_t theSfxID) override;

	void					SetVolume(double theVolume) override;
	bool					SetBaseVolume(intptr_t theSfxID, double theBaseVolume) override;
	bool					SetBasePan(intptr_t theSfxID, int theBasePan) override;

	SoundInstance*			GetSoundInstance(intptr_t theSfxID) override;

	void					ReleaseSounds() override;
	void					ReleaseChannels() override;

	double					GetMasterVolume() override;
	void					SetMasterVolume(double theVolume) override;

	void					Flush() override;
	void					StopAllSounds() override;
	intptr_t				GetFreeSoundId() override;
	int						GetNumSounds() override;
};

}

#endif //__SDLSOUNDMANAGER_H__
