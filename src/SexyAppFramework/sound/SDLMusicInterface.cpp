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

#include "SDLMusicInterface.h"
#include "SDLSoundManager.h"
#include "paklib/PakInterface.h"
#include <vector>

using namespace Sexy;

SDLMusicInfo::SDLMusicInfo()
{
	mVolume = 0.0;
	mVolumeAdd = 0.0;
	mVolumeCap = 1.0;
	mStopOnFade = false;
}

static void ApplyMusicGain(const SDLMusicInfo& theMusic, float theGlobalVolume)
{
	MIX_SetTrackGain(theMusic.mTrack.get(), theMusic.mVolume * theGlobalVolume);
}

static bool InsertMusic(MIX_Mixer* theMixer, int theSongId, SDLMusicInfo&& theMusic, SDLMusicMap& theMap, float theGlobalVolume)
{
	theMusic.mTrack.reset(MIX_CreateTrack(theMixer));
	if (theMusic.mTrack == nullptr)
		return false;

	const bool aHasInput = theMusic.mOpenMpt != nullptr
		? MIX_SetTrackAudioStream(theMusic.mTrack.get(), theMusic.mOpenMpt->GetStream())
		: MIX_SetTrackAudio(theMusic.mTrack.get(), theMusic.mAudio.get());
	if (!aHasInput || !MIX_TagTrack(theMusic.mTrack.get(), "music"))
		return false;

	theMap.emplace(theSongId, std::move(theMusic));
	ApplyMusicGain(theMap.at(theSongId), theGlobalVolume);
	return true;
}

static bool ReadMusicFile(const std::string& theFileName, std::vector<uint8_t>& theData)
{
	std::unique_ptr<PFILE, decltype(&p_fclose)> aFile(p_fopen(theFileName.c_str(), "rb"), &p_fclose);
	if (aFile == nullptr)
		return false;

	p_fseek(aFile.get(), 0, SEEK_END);
	const size_t aSize = p_ftell(aFile.get());
	p_fseek(aFile.get(), 0, SEEK_SET);
	theData.resize(aSize);
	return p_fread(theData.data(), 1, aSize, aFile.get()) == aSize;
}

SDLMusicInterface::SDLMusicInterface(SDLSoundManager* theSoundManager)
{
	mMixer = theSoundManager->GetMixer();
	mGlobalVolume = 1.0f;
}

SDLMusicInterface::~SDLMusicInterface()
{
	UnloadAllMusic();
}

bool SDLMusicInterface::LoadMusic(int theSongId, const std::string& theFileName)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	if (mMixer == nullptr || mMusicMap.contains(theSongId))
		return mMixer != nullptr;

	std::string anExt;
	const size_t aDotPos = theFileName.find_last_of('.');
	if (aDotPos != std::string::npos)
		anExt = StringToLower(theFileName.substr(aDotPos + 1));

	if (anExt == "mo3")
	{
		std::vector<uint8_t> aData;
		return ReadMusicFile(theFileName, aData) && LoadMusicFromMemory(theSongId, aData.data(), aData.size(), anExt);
	}

	SDLMusicInfo aMusicInfo;
	aMusicInfo.mAudio.reset(MIX_LoadAudio(mMixer, theFileName.c_str(), false));
	if (aMusicInfo.mAudio == nullptr)
		return false;
	return InsertMusic(mMixer, theSongId, std::move(aMusicInfo), mMusicMap, mGlobalVolume);
}

bool SDLMusicInterface::LoadMusicFromMemory(int theSongId, const void* theData, size_t theSize, std::string_view theExtension)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	if (mMixer == nullptr || theData == nullptr || theSize == 0)
		return false;
	if (mMusicMap.contains(theSongId))
		return true;

	SDLMusicInfo aMusicInfo;
	if (StringToLower(std::string(theExtension)) == "mo3")
	{
		SDL_AudioSpec aSpec = {};
		int aSampleRate = 44100;
		if (MIX_GetMixerFormat(mMixer, &aSpec))
			aSampleRate = aSpec.freq;

		aMusicInfo.mOpenMpt = std::make_unique<OpenMptMusic>();
		if (!aMusicInfo.mOpenMpt->Load(theData, theSize, aSampleRate))
			return false;
	}
	else
	{
		aMusicInfo.mAudio.reset(MIX_LoadAudio_IO(
			mMixer, SDL_IOFromConstMem(theData, theSize), false, true));
		if (aMusicInfo.mAudio == nullptr)
			return false;
	}

	return InsertMusic(mMixer, theSongId, std::move(aMusicInfo), mMusicMap, mGlobalVolume);
}

bool SDLMusicInterface::StartMusic(SDLMusicInfo& theMusic, int theOffset, bool noLoop)
{
	MIX_LockMixer(mMixer);
	MIX_StopTrack(theMusic.mTrack.get(), 0);
	if (theMusic.mOpenMpt != nullptr)
	{
		theMusic.mOpenMpt->SetLooping(!noLoop);
		theMusic.mOpenMpt->Rewind(theOffset);
	}

	SDL_PropertiesID aOptions = SDL_CreateProperties();
	if (aOptions == 0)
	{
		MIX_UnlockMixer(mMixer);
		return false;
	}
	SDL_SetNumberProperty(aOptions, MIX_PROP_PLAY_LOOPS_NUMBER, noLoop ? 0 : -1);
	const bool aPlayed = MIX_PlayTrack(theMusic.mTrack.get(), aOptions);
	SDL_DestroyProperties(aOptions);
	MIX_UnlockMixer(mMixer);
	return aPlayed;
}

void SDLMusicInterface::PlayMusic(int theSongId, int theOffset, bool noLoop)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	auto anItr = mMusicMap.find(theSongId);
	if (anItr == mMusicMap.end())
		return;

	SDLMusicInfo& aMusicInfo = anItr->second;
	aMusicInfo.mVolume = aMusicInfo.mVolumeCap;
	aMusicInfo.mVolumeAdd = 0.0;
	aMusicInfo.mStopOnFade = noLoop;
	ApplyMusicGain(aMusicInfo, mGlobalVolume);
	StartMusic(aMusicInfo, theOffset, noLoop);
}

void SDLMusicInterface::StopMusic(int theSongId)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	auto anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		anItr->second.mVolume = 0.0;
		MIX_StopTrack(anItr->second.mTrack.get(), 0);
	}
}

void SDLMusicInterface::PauseMusic(int theSongId)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	auto anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
		MIX_PauseTrack(anItr->second.mTrack.get());
}

void SDLMusicInterface::ResumeMusic(int theSongId)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	auto anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
		MIX_ResumeTrack(anItr->second.mTrack.get());
}

void SDLMusicInterface::StopAllMusic()
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	for (auto& [aId, aMusicInfo] : mMusicMap)
	{
		aMusicInfo.mVolume = 0.0;
		MIX_StopTrack(aMusicInfo.mTrack.get(), 0);
	}
}

void SDLMusicInterface::UnloadMusic(int theSongId)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	StopMusic(theSongId);
	mMusicMap.erase(theSongId);
}

void SDLMusicInterface::UnloadAllMusic()
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	StopAllMusic();
	mMusicMap.clear();
}

void SDLMusicInterface::PauseAllMusic()
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	for (auto& [aId, aMusicInfo] : mMusicMap)
		if (MIX_TrackPlaying(aMusicInfo.mTrack.get()))
			MIX_PauseTrack(aMusicInfo.mTrack.get());
}

void SDLMusicInterface::ResumeAllMusic()
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	for (auto& [aId, aMusicInfo] : mMusicMap)
		if (MIX_TrackPaused(aMusicInfo.mTrack.get()))
			MIX_ResumeTrack(aMusicInfo.mTrack.get());
}

void SDLMusicInterface::FadeIn(int theSongId, int theOffset, double theSpeed, bool noLoop)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	auto anItr = mMusicMap.find(theSongId);
	if (anItr == mMusicMap.end())
		return;

	SDLMusicInfo& aMusicInfo = anItr->second;
	aMusicInfo.mVolumeAdd = theSpeed;
	aMusicInfo.mStopOnFade = noLoop;
	StartMusic(aMusicInfo, theOffset, noLoop);
}

void SDLMusicInterface::FadeOut(int theSongId, bool stopSong, double theSpeed)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	auto anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo& aMusicInfo = anItr->second;
		if (aMusicInfo.mVolume != 0.0)
			aMusicInfo.mVolumeAdd = -theSpeed;
		aMusicInfo.mStopOnFade = stopSong;
	}
}

void SDLMusicInterface::FadeOutAll(bool stopSong, double theSpeed)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	for (auto& [aId, aMusicInfo] : mMusicMap)
	{
		aMusicInfo.mVolumeAdd = -theSpeed;
		aMusicInfo.mStopOnFade = stopSong;
	}
}

void SDLMusicInterface::SetSongVolume(int theSongId, double theVolume)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	auto anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		anItr->second.mVolume = theVolume;
		ApplyMusicGain(anItr->second, mGlobalVolume);
	}
}

void SDLMusicInterface::SetSongMaxVolume(int theSongId, double theMaxVolume)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	auto anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo& aMusicInfo = anItr->second;
		aMusicInfo.mVolumeCap = theMaxVolume;
		aMusicInfo.mVolume = std::min(aMusicInfo.mVolume, theMaxVolume);
		ApplyMusicGain(aMusicInfo, mGlobalVolume);
	}
}

bool SDLMusicInterface::IsPlaying(int theSongId)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	auto anItr = mMusicMap.find(theSongId);
	return anItr != mMusicMap.end() &&
		(MIX_TrackPlaying(anItr->second.mTrack.get()) || MIX_TrackPaused(anItr->second.mTrack.get()));
}

void SDLMusicInterface::SetVolume(double theVolume)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	mGlobalVolume = static_cast<float>(theVolume);
	for (auto& [aId, aMusicInfo] : mMusicMap)
		ApplyMusicGain(aMusicInfo, mGlobalVolume);
}

void SDLMusicInterface::SetMusicAmplify(int, double)
{
}

void SDLMusicInterface::Update()
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	for (auto& [aId, aMusicInfo] : mMusicMap)
	{
		if (aMusicInfo.mVolumeAdd == 0.0)
			continue;

		aMusicInfo.mVolume += aMusicInfo.mVolumeAdd;
		if (aMusicInfo.mVolume > aMusicInfo.mVolumeCap)
		{
			aMusicInfo.mVolume = aMusicInfo.mVolumeCap;
			aMusicInfo.mVolumeAdd = 0.0;
		}
		else if (aMusicInfo.mVolume < 0.0)
		{
			aMusicInfo.mVolume = 0.0;
			aMusicInfo.mVolumeAdd = 0.0;
			if (aMusicInfo.mStopOnFade)
				MIX_StopTrack(aMusicInfo.mTrack.get(), 0);
		}
		ApplyMusicGain(aMusicInfo, mGlobalVolume);
	}
}

int SDLMusicInterface::GetMusicOrder(int theSongId)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	auto anItr = mMusicMap.find(theSongId);
	if (anItr == mMusicMap.end() || anItr->second.mOpenMpt == nullptr)
		return -1;

	MIX_LockMixer(mMixer);
	const int aPosition = anItr->second.mOpenMpt->GetOrder();
	MIX_UnlockMixer(mMixer);
	return aPosition;
}

bool SDLMusicInterface::JumpToOrder(int theSongId, int thePackedOrder)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	auto anItr = mMusicMap.find(theSongId);
	if (anItr == mMusicMap.end() || anItr->second.mOpenMpt == nullptr)
		return false;

	MIX_LockMixer(mMixer);
	anItr->second.mOpenMpt->JumpToOrder(thePackedOrder);
	MIX_UnlockMixer(mMixer);
	return true;
}

bool SDLMusicInterface::SetChannelVolume(int theSongId, int theChannel, double theVolume)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);
	auto anItr = mMusicMap.find(theSongId);
	if (anItr == mMusicMap.end() || anItr->second.mOpenMpt == nullptr)
		return false;

	MIX_LockMixer(mMixer);
	anItr->second.mOpenMpt->SetChannelVolume(theChannel, theVolume);
	MIX_UnlockMixer(mMixer);
	return true;
}
