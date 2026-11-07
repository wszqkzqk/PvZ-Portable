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

#ifndef __SDLSOUNDINSTANCE_H__
#define __SDLSOUNDINSTANCE_H__

#include "SoundInstance.h"
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

namespace Sexy
{

class SDLSoundManager;

class SDLSoundInstance : public SoundInstance
{
	friend class SDLSoundManager;

protected:
	SDLSoundManager* mSoundManagerP;
	MIX_Audio* mAudio;
	MIX_Track* mTrack;
	int mDurationMs;
	bool mAutoRelease;
	bool mHasPlayed;
	bool mReleased;

	int mBasePan;
	double mBaseVolume;
	int mPan;
	double mVolume;
	float mPitch;

	uint mDemoEndUpdateCount;
	bool mDemoLooping;

protected:
	void RehupVolume();
	void RehupPan();

public:
	SDLSoundInstance(SDLSoundManager* theSoundManager, MIX_Audio* theAudio, int theDurationMs = 0);
	~SDLSoundInstance() override;
	void Release() override;

	void SetBaseVolume(double theBaseVolume) override;
	void SetBasePan(int theBasePan) override;
	void AdjustPitch(double theNumSteps) override;
	void SetVolume(double theVolume) override;
	void SetPan(int thePosition) override;

	bool Play(bool looping, bool autoRelease) override;
	void Stop() override;
	bool IsPlaying() override;
	bool IsReleased() override;
	double GetVolume() override;
};

}

#endif //__SDLSOUNDINSTANCE_H__
