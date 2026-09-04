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

#ifndef __SDLMUSICINTERFACE_H__
#define __SDLMUSICINTERFACE_H__

#include "MusicInterface.h"

#include <SDL.h>
#include <SDL_mixer_ext/SDL_mixer_ext.h>

#include <mutex>

namespace Sexy
{

class SexyAppBase;

class SDLMusicInfo
{
public:
	Mix_Music*				mHMusic;
	double					mVolume;
	double					mVolumeAdd;
	double					mVolumeCap;
	bool					mStopOnFade;

public:
	SDLMusicInfo();

	Mix_Music* GetHandle() { return mHMusic; }
};

typedef std::map<int, SDLMusicInfo> SDLMusicMap;

class SDLMusicInterface : public MusicInterface
{
public:
	SDLMusicMap				mMusicMap;
	std::recursive_mutex	mMusicMapMutex;
	int						mGlobalVolume;
	int						mMusicLoadFlags;

public:
	SDLMusicInterface();
	~SDLMusicInterface() override;

	bool					LoadMusic(int theSongId, const std::string& theFileName) override;
	void					PlayMusic(int theSongId, int theOffset = 0, bool noLoop = false) override;
	void					StopMusic(int theSongId) override;
	void					PauseMusic(int theSongId) override;
	void					ResumeMusic(int theSongId) override;
	void					StopAllMusic() override;

	void					UnloadMusic(int theSongId) override;
	void					UnloadAllMusic() override;
	void					PauseAllMusic() override;
	void					ResumeAllMusic() override;

	void					FadeIn(int theSongId, int theOffset = -1, double theSpeed = 0.002, bool noLoop = false) override;
	void					FadeOut(int theSongId, bool stopSong = true, double theSpeed = 0.004) override;
	void					FadeOutAll(bool stopSong = true, double theSpeed = 0.004) override;
	void					SetSongVolume(int theSongId, double theVolume) override;
	void					SetSongMaxVolume(int theSongId, double theMaxVolume) override;
	bool					IsPlaying(int theSongId) override;

	void					SetVolume(double theVolume) override;
	void					SetMusicAmplify(int theSongId, double theAmp) override;
	void					Update() override;

	// functions for dealing with MODs
	int						GetMusicOrder(int theSongId);
};
}

#endif //__SDLMUSICINTERFACE_H__
