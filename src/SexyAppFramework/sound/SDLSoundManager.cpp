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

#include "SDLSoundManager.h"
#include "SDLSoundInstance.h"
#include "paklib/PakInterface.h"
#include <memory>
#include <vector>

using namespace Sexy;

SDLSoundManager::SDLSoundManager()
{
	mMixer = nullptr;
	mInitializedMixer = false;
	mLastReleaseTick = 0;
	mMasterVolume = 1.0;

	for (int i = 0; i < MAX_SOURCE_SOUNDS; i++)
	{
		mSourceSounds[i] = nullptr;
		mSourceDurationMs[i] = 0;
		mBaseVolumes[i] = 1;
		mBasePans[i] = 0;
	}

	if (!MIX_Init())
	{
		LogInfoLn("Failed to initialize SDL mixer: {}", SDL_GetError());
		return;
	}
	mInitializedMixer = true;

	SDL_AudioSpec aSpec = { SDL_AUDIO_F32, 2, 44100 };
	mMixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &aSpec);
	if (mMixer == nullptr)
	{
		LogInfoLn("Failed to open SDL audio device: {}", SDL_GetError());
		MIX_Quit();
		mInitializedMixer = false;
	}
}

SDLSoundManager::~SDLSoundManager()
{
	ReleaseChannels();
	ReleaseSounds();
	if (mMixer != nullptr)
		MIX_DestroyMixer(mMixer);
	if (mInitializedMixer)
		MIX_Quit();
}

bool SDLSoundManager::Initialized()
{
	return mMixer != nullptr;
}

bool SDLSoundManager::DecodeSound(intptr_t theSfxID, const std::string& theFilename)
{
	static constexpr const char* aFormats[] = { ".wav", ".mp3", ".ogg", ".au" };

	for (const char* aFormat : aFormats)
	{
		std::string aFilename = theFilename + aFormat;
		PFILE* fp = p_fopen(aFilename.c_str(), "rb");
		if (fp == nullptr)
			continue;

		p_fseek(fp, 0, SEEK_END);
		const size_t aFileSize = p_ftell(fp);
		p_fseek(fp, 0, SEEK_SET);
		std::vector<uint8_t> aData(aFileSize);
		const size_t aReadSize = p_fread(aData.data(), 1, aFileSize, fp);
		p_fclose(fp);
		if (aReadSize != aFileSize)
			continue;

#ifdef LOW_MEMORY
		constexpr bool aPredecode = false;
#else
		constexpr bool aPredecode = true;
#endif
		MIX_Audio* aAudio = MIX_LoadAudio_IO(
			mMixer, SDL_IOFromConstMem(aData.data(), aData.size()), aPredecode, true);
		if (aAudio == nullptr)
			continue;

		mSourceSounds[theSfxID] = aAudio;
		const Sint64 aDurationMs = MIX_AudioFramesToMS(aAudio, MIX_GetAudioDuration(aAudio));
		mSourceDurationMs[theSfxID] = aDurationMs > 0 ? static_cast<int>(aDurationMs) : 0;
		return true;
	}

	return false;
}

bool SDLSoundManager::LoadSound(intptr_t theSfxID, const std::string& theFilename)
{
	if (theSfxID < 0 || theSfxID >= MAX_SOURCE_SOUNDS)
		return false;

	ReleaseSound(theSfxID);
	mSourceFileNames[theSfxID] = theFilename;
	if (!Initialized())
		return true;

#ifdef LOW_MEMORY
	static constexpr const char* aFormats[] = { ".wav", ".mp3", ".ogg", ".au" };
	for (const char* aFormat : aFormats)
	{
		if (PFILE* fp = p_fopen((theFilename + aFormat).c_str(), "rb"))
		{
			p_fclose(fp);
			return true;
		}
	}
	mSourceFileNames[theSfxID].clear();
	return false;
#else
	return DecodeSound(theSfxID, theFilename);
#endif
}

intptr_t SDLSoundManager::LoadSound(const std::string& theFilename)
{
	for (intptr_t i = 0; i < MAX_SOURCE_SOUNDS; i++)
		if (mSourceFileNames[i] == theFilename)
			return i;

	for (intptr_t i = MAX_SOURCE_SOUNDS - 1; i >= 0; i--)
	{
#ifdef LOW_MEMORY
		if (mSourceSounds[i] == nullptr && mSourceFileNames[i].empty())
#else
		if (mSourceSounds[i] == nullptr)
#endif
		{
			if (!LoadSound(i, theFilename))
				return -1;
			return i;
		}
	}

	return -1;
}

void SDLSoundManager::ReleaseSound(intptr_t theSfxID)
{
	if (theSfxID < 0 || theSfxID >= MAX_SOURCE_SOUNDS)
		return;

	MIX_DestroyAudio(mSourceSounds[theSfxID].exchange(nullptr));
	mSourceDurationMs[theSfxID] = 0;
	mSourceFileNames[theSfxID].clear();
}

void SDLSoundManager::SetVolume(double theVolume)
{
	mMasterVolume = theVolume;
	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] != nullptr)
			mPlayingSounds[i]->RehupVolume();
}

bool SDLSoundManager::SetBaseVolume(intptr_t theSfxID, double theBaseVolume)
{
	if (theSfxID < 0 || theSfxID >= MAX_SOURCE_SOUNDS)
		return false;
	mBaseVolumes[theSfxID] = theBaseVolume;
	return true;
}

bool SDLSoundManager::SetBasePan(intptr_t theSfxID, int theBasePan)
{
	if (theSfxID < 0 || theSfxID >= MAX_SOURCE_SOUNDS)
		return false;
	mBasePans[theSfxID] = theBasePan;
	return true;
}

SoundInstance* SDLSoundManager::GetSoundInstance(intptr_t theSfxID)
{
	if (theSfxID < 0 || theSfxID >= MAX_SOURCE_SOUNDS)
		return nullptr;

	const int aFreeChannel = FindFreeChannel();
	if (aFreeChannel < 0)
		return nullptr;

	if (mSourceSounds[theSfxID] == nullptr)
	{
#ifdef LOW_MEMORY
		if (mSourceFileNames[theSfxID].empty() || !DecodeSound(theSfxID, mSourceFileNames[theSfxID]))
			return nullptr;
#else
		return nullptr;
#endif
	}

	mPlayingSounds[aFreeChannel] = std::make_unique<SDLSoundInstance>(
		this, mSourceSounds[theSfxID].load(), mSourceDurationMs[theSfxID].load());
	mPlayingSounds[aFreeChannel]->SetBasePan(mBasePans[theSfxID]);
	mPlayingSounds[aFreeChannel]->SetBaseVolume(mBaseVolumes[theSfxID]);
	return mPlayingSounds[aFreeChannel].get();
}

void SDLSoundManager::ReleaseSounds()
{
	for (int i = 0; i < MAX_SOURCE_SOUNDS; i++)
	{
		MIX_DestroyAudio(mSourceSounds[i].exchange(nullptr));
		mSourceDurationMs[i] = 0;
		mSourceFileNames[i].clear();
	}
}

void SDLSoundManager::ReleaseChannels()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
		mPlayingSounds[i].reset();
}

double SDLSoundManager::GetMasterVolume()
{
	return mMixer != nullptr ? MIX_GetMixerGain(mMixer) : 0.0;
}

void SDLSoundManager::SetMasterVolume(double theVolume)
{
	if (mMixer != nullptr)
		MIX_SetMixerGain(mMixer, static_cast<float>(theVolume));
}

void SDLSoundManager::Flush()
{
}

void SDLSoundManager::StopAllSounds()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
	{
		if (mPlayingSounds[i] != nullptr)
		{
			const bool isAutoRelease = mPlayingSounds[i]->mAutoRelease;
			mPlayingSounds[i]->Stop();
			mPlayingSounds[i]->mAutoRelease = isAutoRelease;
		}
	}
}

intptr_t SDLSoundManager::GetFreeSoundId()
{
	for (intptr_t i = 0; i < MAX_SOURCE_SOUNDS; i++)
	{
#ifdef LOW_MEMORY
		if (mSourceSounds[i] == nullptr && mSourceFileNames[i].empty())
#else
		if (mSourceSounds[i] == nullptr)
#endif
			return i;
	}
	return -1;
}

int SDLSoundManager::GetNumSounds()
{
	int aCount = 0;
	for (int i = 0; i < MAX_SOURCE_SOUNDS; i++)
		if (mSourceSounds[i] != nullptr)
			aCount++;
	return aCount;
}

int SDLSoundManager::FindFreeChannel()
{
	const uint64_t aTick = SDL_GetTicks();
	if (aTick - mLastReleaseTick > 1000)
	{
		ReleaseFreeChannels();
		mLastReleaseTick = aTick;
	}

	for (int i = 0; i < MAX_CHANNELS; i++)
	{
		if (mPlayingSounds[i] == nullptr)
			return i;
		if (mPlayingSounds[i]->IsReleased())
		{
			mPlayingSounds[i].reset();
			return i;
		}
	}
	return -1;
}

void SDLSoundManager::ReleaseFreeChannels()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
		if (mPlayingSounds[i] != nullptr && mPlayingSounds[i]->IsReleased())
			mPlayingSounds[i].reset();
}
