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

#include "SDLSoundInstance.h"
#include "SDLSoundManager.h"
#include "SexyAppBase.h"
#include <cmath>

using namespace Sexy;

SDLSoundInstance::SDLSoundInstance(SDLSoundManager* theSoundManager, MIX_Audio* theAudio, int theDurationMs)
{
	mSoundManagerP = theSoundManager;
	mAudio = theAudio;
	mTrack = MIX_CreateTrack(theSoundManager->mMixer);
	mDurationMs = theDurationMs;
	mReleased = false;
	mAutoRelease = false;
	mHasPlayed = false;

	mBasePan = 0;
	mBaseVolume = 1.0;
	mVolume = 1.0;
	mPan = 0;
	mPitch = 1.0f;

	mDemoEndUpdateCount = 0;
	mDemoLooping = false;

	RehupVolume();
}

SDLSoundInstance::~SDLSoundInstance()
{
	Stop();
	MIX_DestroyTrack(mTrack);
}

void SDLSoundInstance::RehupVolume()
{
	if (mTrack != nullptr)
		MIX_SetTrackGain(mTrack, static_cast<float>(mBaseVolume * mVolume * mSoundManagerP->mMasterVolume));
}

void SDLSoundInstance::RehupPan()
{
}

void SDLSoundInstance::Release()
{
	Stop();
	mReleased = true;
}

void SDLSoundInstance::SetBaseVolume(double theBaseVolume)
{
	mBaseVolume = theBaseVolume;
	RehupVolume();
}

void SDLSoundInstance::SetBasePan(int theBasePan)
{
	mBasePan = theBasePan;
	RehupPan();
}

void SDLSoundInstance::AdjustPitch(double theNumSteps)
{
	mPitch = powf(1.0594630943592952645618252949463, theNumSteps);
	if (mTrack != nullptr)
		MIX_SetTrackFrequencyRatio(mTrack, mPitch);
}

void SDLSoundInstance::SetVolume(double theVolume)
{
	mVolume = theVolume;
	RehupVolume();
}

void SDLSoundInstance::SetPan(int thePosition)
{
	mPan = thePosition;
	RehupPan();
}

bool SDLSoundInstance::Play(bool looping, bool autoRelease)
{
	Stop();

	mHasPlayed = true;
	mAutoRelease = autoRelease;

	if (gSexyAppBase->IsInDemoMode())
	{
		mDemoLooping = looping;
		mDemoEndUpdateCount = gSexyAppBase->mUpdateCount +
			static_cast<uint>(mDurationMs / (10.0 * mPitch)) + 1;
	}

	if (mAudio == nullptr || mTrack == nullptr)
		return false;

	if (!MIX_SetTrackAudio(mTrack, mAudio))
		return false;

	RehupVolume();
	MIX_SetTrackFrequencyRatio(mTrack, mPitch);

	SDL_PropertiesID aOptions = SDL_CreateProperties();
	if (aOptions == 0)
		return false;
	SDL_SetNumberProperty(aOptions, MIX_PROP_PLAY_LOOPS_NUMBER, looping ? -1 : 0);
	const bool aPlayed = MIX_PlayTrack(mTrack, aOptions);
	SDL_DestroyProperties(aOptions);
	return aPlayed;
}

void SDLSoundInstance::Stop()
{
	mDemoEndUpdateCount = 0;
	if (mTrack != nullptr)
		MIX_StopTrack(mTrack, 0);
	mAutoRelease = false;
}

bool SDLSoundInstance::IsPlaying()
{
	if (gSexyAppBase->IsInDemoMode())
	{
		if (mAudio == nullptr || !mHasPlayed || mDemoEndUpdateCount == 0)
			return false;
		if (mDemoLooping)
			return true;
		return gSexyAppBase->mUpdateCount < mDemoEndUpdateCount;
	}
	return mAudio != nullptr && mHasPlayed && mTrack != nullptr && MIX_TrackPlaying(mTrack);
}

bool SDLSoundInstance::IsReleased()
{
	if (!mReleased && mAutoRelease && mHasPlayed && !IsPlaying())
		Release();

	return mReleased;
}

double SDLSoundInstance::GetVolume()
{
	return mVolume;
}
