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

#ifndef __SDLMUSICINTERFACE_H__
#define __SDLMUSICINTERFACE_H__

#include "MusicInterface.h"
#include "OpenMptMusic.h"
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <memory>
#include <mutex>

namespace Sexy
{

class SDLSoundManager;

class SDLMusicInfo
{
public:
	std::unique_ptr<OpenMptMusic> mOpenMpt;
	std::unique_ptr<MIX_Audio, decltype(&MIX_DestroyAudio)> mAudio{nullptr, &MIX_DestroyAudio};
	std::unique_ptr<MIX_Track, decltype(&MIX_DestroyTrack)> mTrack{nullptr, &MIX_DestroyTrack};
	double mVolume;
	double mVolumeAdd;
	double mVolumeCap;
	bool mStopOnFade;

	SDLMusicInfo();
};

using SDLMusicMap = std::map<int, SDLMusicInfo>;

class SDLMusicInterface : public MusicInterface
{
protected:
	MIX_Mixer* mMixer;
	bool StartMusic(SDLMusicInfo& theMusic, int theOffset, bool noLoop);

public:
	SDLMusicMap mMusicMap;
	std::recursive_mutex mMusicMapMutex;
	float mGlobalVolume;

	SDLMusicInterface(SDLSoundManager* theSoundManager);
	~SDLMusicInterface() override;

	bool LoadMusic(int theSongId, const std::string& theFileName) override;
	bool LoadMusicFromMemory(int theSongId, const void* theData, size_t theSize, std::string_view theExtension) override;
	void PlayMusic(int theSongId, int theOffset = 0, bool noLoop = false) override;
	void StopMusic(int theSongId) override;
	void PauseMusic(int theSongId) override;
	void ResumeMusic(int theSongId) override;
	void StopAllMusic() override;

	void UnloadMusic(int theSongId) override;
	void UnloadAllMusic() override;
	void PauseAllMusic() override;
	void ResumeAllMusic() override;

	void FadeIn(int theSongId, int theOffset = -1, double theSpeed = 0.002, bool noLoop = false) override;
	void FadeOut(int theSongId, bool stopSong = true, double theSpeed = 0.004) override;
	void FadeOutAll(bool stopSong = true, double theSpeed = 0.004) override;
	void SetSongVolume(int theSongId, double theVolume) override;
	void SetSongMaxVolume(int theSongId, double theMaxVolume) override;
	bool IsPlaying(int theSongId) override;

	void SetVolume(double theVolume) override;
	void SetMusicAmplify(int theSongId, double theAmp) override;
	void Update() override;

	int GetMusicOrder(int theSongId) override;
	bool JumpToOrder(int theSongId, int thePackedOrder) override;
	bool SetChannelVolume(int theSongId, int theChannel, double theVolume) override;
};

}

#endif //__SDLMUSICINTERFACE_H__
