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

#include "SDLMusicInterface.h"
#include "paklib/PakInterface.h"

using namespace Sexy;

SDLMusicInfo::SDLMusicInfo()
{
	mVolume = 0.0;
	mVolumeAdd = 0.0;
	mVolumeCap = 1.0;
	mStopOnFade = false;
	mHMusic = 0;
}

SDLMusicInterface::SDLMusicInterface()
{
	mGlobalVolume = MIX_MAX_VOLUME;
}

SDLMusicInterface::~SDLMusicInterface()
{
	Mix_HaltMusic();
}

bool SDLMusicInterface::LoadMusic(int theSongId, const std::string& theFileName)
{
	Mix_Music* aHMusic = 0;

	std::string anExt;
	size_t aDotPos = theFileName.find_last_of('.');
	if (aDotPos!=std::string::npos)
		anExt = StringToLower(theFileName.substr(aDotPos+1));

	aHMusic = Mix_LoadMUS(theFileName.c_str());

	if (aHMusic==0)
		return false;

	SDLMusicInfo aMusicInfo;
	aMusicInfo.mHMusic = aHMusic;

	std::scoped_lock anAutoCrit(mMusicMapMutex);
	mMusicMap.insert(SDLMusicMap::value_type(theSongId, aMusicInfo));

	return true;
}

void SDLMusicInterface::PlayMusic(int theSongId, int theOffset, bool noLoop)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);

	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		aMusicInfo->mVolume = aMusicInfo->mVolumeCap;
		aMusicInfo->mVolumeAdd = 0.0;
		aMusicInfo->mStopOnFade = noLoop;

		Mix_HaltMusicStream(aMusicInfo->mHMusic);
		Mix_PlayMusicStream(aMusicInfo->mHMusic, (noLoop) ? 0 : -1);
		if (theOffset > 0)
			Mix_ModMusicStreamJumpToOrder(aMusicInfo->mHMusic, theOffset);
	}
}

void SDLMusicInterface::StopMusic(int theSongId)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);

	Mix_HaltMusic();

	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		aMusicInfo->mVolume = 0.0;
		Mix_HaltMusicStream(aMusicInfo->mHMusic);
	}
}

void SDLMusicInterface::PauseMusic(int theSongId)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);

	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		Mix_PauseMusicStream(aMusicInfo->mHMusic);
	}
}

void SDLMusicInterface::ResumeMusic(int theSongId)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);

	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		Mix_ResumeMusicStream(aMusicInfo->mHMusic);
	}
}

void SDLMusicInterface::StopAllMusic()
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);

	SDLMusicMap::iterator anItr = mMusicMap.begin();
	while (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		aMusicInfo->mVolume = 0.0;
		Mix_HaltMusicStream(aMusicInfo->mHMusic);
		++anItr;
	}
}

void SDLMusicInterface::UnloadMusic(int theSongId)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);

	StopMusic(theSongId);

	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		Mix_FreeMusic(aMusicInfo->mHMusic);

		mMusicMap.erase(anItr);
	}
}

void SDLMusicInterface::UnloadAllMusic()
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);

	StopAllMusic();
	for (SDLMusicMap::iterator anItr = mMusicMap.begin(); anItr != mMusicMap.end(); ++anItr)
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		Mix_FreeMusic(aMusicInfo->mHMusic);
	}
	mMusicMap.clear();
}

void SDLMusicInterface::PauseAllMusic()
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);

	for (SDLMusicMap::iterator anItr = mMusicMap.begin(); anItr != mMusicMap.end(); ++anItr)
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		if (Mix_PlayingMusicStream(aMusicInfo->mHMusic) && !Mix_PausedMusicStream(aMusicInfo->mHMusic))
			Mix_PauseMusicStream(aMusicInfo->mHMusic);
	}
}

void SDLMusicInterface::ResumeAllMusic()
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);

	for (SDLMusicMap::iterator anItr = mMusicMap.begin(); anItr != mMusicMap.end(); ++anItr)
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		if (Mix_PausedMusicStream(aMusicInfo->mHMusic))
			Mix_ResumeMusicStream(aMusicInfo->mHMusic);
	}
}

void SDLMusicInterface::FadeIn(int theSongId, int theOffset, double theSpeed, bool noLoop)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);

	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;

		aMusicInfo->mVolumeAdd = theSpeed;
		aMusicInfo->mStopOnFade = noLoop;

		Mix_HaltMusicStream(aMusicInfo->mHMusic);
		Mix_PlayMusicStream(aMusicInfo->mHMusic, (noLoop) ? 0 : -1);
		if (theOffset > 0)
			Mix_ModMusicStreamJumpToOrder(aMusicInfo->mHMusic, theOffset);

	}
}

void SDLMusicInterface::FadeOut(int theSongId, bool stopSong, double theSpeed)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);

	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;

		if (aMusicInfo->mVolume != 0.0)
		{
			aMusicInfo->mVolumeAdd = -theSpeed;
		}

		aMusicInfo->mStopOnFade = stopSong;
	}
}

void SDLMusicInterface::FadeOutAll(bool stopSong, double theSpeed)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);

	SDLMusicMap::iterator anItr = mMusicMap.begin();
	while (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;

		aMusicInfo->mVolumeAdd = -theSpeed;
		aMusicInfo->mStopOnFade = stopSong;

		++anItr;
	}
}

void SDLMusicInterface::SetSongVolume(int theSongId, double theVolume)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);

	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;

		aMusicInfo->mVolume = theVolume;
		Mix_VolumeMusicStream(aMusicInfo->mHMusic, (int)(aMusicInfo->mVolume*128));
	}
}

void SDLMusicInterface::SetSongMaxVolume(int theSongId, double theMaxVolume)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);

	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;

		aMusicInfo->mVolumeCap = theMaxVolume;
		aMusicInfo->mVolume = std::min(aMusicInfo->mVolume, theMaxVolume);
		Mix_VolumeMusicStream(aMusicInfo->mHMusic, (int)(aMusicInfo->mVolume*128));
	}
}

bool SDLMusicInterface::IsPlaying(int theSongId)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);

	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		return Mix_PlayingMusicStream(aMusicInfo->mHMusic);
	}

	return false;
}

void SDLMusicInterface::SetVolume(double theVolume)
{
	mGlobalVolume = (int)(theVolume*80);
}

void SDLMusicInterface::SetMusicAmplify(int theSongId, double theAmp)
{

}

void SDLMusicInterface::Update()
{
	Mix_VolumeMusic(mGlobalVolume);
	Mix_VolumeMusicGeneral(mGlobalVolume);

	std::scoped_lock anAutoCrit(mMusicMapMutex);

	SDLMusicMap::iterator anItr = mMusicMap.begin();
	while (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;

		if (aMusicInfo->mVolumeAdd != 0.0)
		{
			aMusicInfo->mVolume += aMusicInfo->mVolumeAdd;

			if (aMusicInfo->mVolume > aMusicInfo->mVolumeCap)
			{
				aMusicInfo->mVolume = aMusicInfo->mVolumeCap;
				aMusicInfo->mVolumeAdd = 0.0;
			}
			else if (aMusicInfo->mVolume < 0.0)
			{
				aMusicInfo->mVolume = 0.0;
				aMusicInfo->mVolumeAdd = 0.0;

				if (aMusicInfo->mStopOnFade)
					Mix_HaltMusicStream(aMusicInfo->mHMusic);
			}

			Mix_VolumeMusicStream(aMusicInfo->mHMusic, (int)(aMusicInfo->mVolume*128));
		}

		++anItr;
	}
}

// functions for dealing with MODs
int SDLMusicInterface::GetMusicOrder(int theSongId)
{
	std::scoped_lock anAutoCrit(mMusicMapMutex);

	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		int aPosition = -1;
		Mix_ModMusicStreamGetOrder(aMusicInfo->mHMusic, &aPosition);
		return aPosition;
	}
	return -1;
}
