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

#ifndef __PVZPFOLEY_H__
#define __PVZPFOLEY_H__

#include <cstdint>
#include "../Resources.h"
#include "sound/SDLSoundInstance.h"
using namespace Sexy;

#define MAX_FOLEY_TYPES 110
#define MAX_FOLEY_INSTANCES 8

// Foley sound definitions

enum FoleyFlags : int32_t
{
	FOLEYFLAGS_LOOP,
	FOLEYFLAGS_ONE_AT_A_TIME,       // at most 1 instance of this sound; replaying only bumps the refcount and refreshes the start time
	FOLEYFLAGS_MUTE_ON_PAUSE,
	FOLEYFLAGS_USES_MUSIC_VOLUME,
	FOLEYFLAGS_DONT_REPEAT          // the variation played must differ from the previous one
};

enum FoleyType : int32_t
{
	FOLEY_SUN,
	FOLEY_SPLAT,
	FOLEY_LAWNMOWER,
	FOLEY_THROW,
	FOLEY_SPAWN_SUN,
	FOLEY_CHOMP,
	FOLEY_CHOMP_SOFT,
	FOLEY_PLANT,
	FOLEY_USE_SHOVEL,
	FOLEY_DROP,
	FOLEY_BLEEP,
	FOLEY_GROAN,
	FOLEY_BRAINS,
	FOLEY_SUKHBIR,
	FOLEY_JACKINTHEBOX,
	FOLEY_ART_CHALLENGE,
	FOLEY_ZAMBONI,
	FOLEY_THUNDER,
	FOLEY_FROZEN,
	FOLEY_ZOMBIESPLASH,
	FOLEY_BOWLINGIMPACT,
	FOLEY_SQUISH,
	FOLEY_TIRE_POP,
	FOLEY_EXPLOSION,
	FOLEY_SLURP,
	FOLEY_LIMBS_POP,
	FOLEY_POGO_ZOMBIE,
	FOLEY_SNOW_PEA_SPARKLES,
	FOLEY_ZOMBIE_FALLING,
	FOLEY_PUFF,
	FOLEY_FUME,
	FOLEY_COIN,
	FOLEY_KERNEL_SPLAT,
	FOLEY_DIGGER,
	FOLEY_JACK_SURPRISE,
	FOLEY_VASE_BREAKING,
	FOLEY_POOL_CLEANER,
	FOLEY_BASKETBALL,
	FOLEY_IGNITE,
	FOLEY_FIREPEA,
	FOLEY_THUMP,
	FOLEY_SQUASH_HMM,
	FOLEY_MAGNETSHROOM,
	FOLEY_BUTTER,
	FOLEY_BUNGEE_SCREAM,
	FOLEY_BOSS_EXPLOSION_SMALL,
	FOLEY_SHIELD_HIT,
	FOLEY_SWING,
	FOLEY_BONK,
	FOLEY_RAIN,
	FOLEY_DOLPHIN_BEFORE_JUMPING,
	FOLEY_DOLPHIN_APPEARS,
	FOLEY_PLANT_WATER,
	FOLEY_ZOMBIE_ENTERING_WATER,
	FOLEY_GRAVEBUSTERCHOMP,
	FOLEY_CHERRYBOMB,
	FOLEY_JALAPENO_IGNITE,
	FOLEY_REVERSE_EXPLOSION,
	FOLEY_PLASTIC_HIT,
	FOLEY_WINMUSIC,
	FOLEY_BALLOONINFLATE,
	FOLEY_BIGCHOMP,
	FOLEY_MELONIMPACT,
	FOLEY_PLANTGROW,
	FOLEY_SHOOP,
	FOLEY_JUICY,
	FOLEY_NEWSPAPER_RARRGH,
	FOLEY_NEWSPAPER_RIP,
	FOLEY_FLOOP,
	FOLEY_COFFEE,
	FOLEY_LOW_GROAN,
	FOLEY_PRIZE,
	FOLEY_YUCK,
	FOLEY_UMBRELLA,
	FOLEY_GRASSSTEP,
	FOLEY_SHOVEL,
	FOLEY_COB_LAUNCH,
	FOLEY_WATERING,
	FOLEY_POLEVAULT,
	FOLEY_GRAVESTONE_RUMBLE,
	FOLEY_DIRT_RISE,
	FOLEY_FERTILIZER,
	FOLEY_PORTAL,
	FOLEY_WAKEUP,
	FOLEY_BUGSPRAY,
	FOLEY_SCREAM,
	FOLEY_PAPER,
	FOLEY_MONEYFALLS,
	FOLEY_IMP,
	FOLEY_HYDRAULIC_SHORT,
	FOLEY_HYDRAULIC,
	FOLEY_GARGANTUDEATH,
	FOLEY_CERAMIC,
	FOLEY_BOSS_BOULDER_ATTACK,
	FOLEY_CHIME,
	FOLEY_CRAZY_DAVE_SHORT,
	FOLEY_CRAZY_DAVE_LONG,
	FOLEY_CRAZY_DAVE_EXTRA_LONG,
	FOLEY_CRAZY_DAVE_CRAZY,
	FOLEY_PHONOGRAPH,
	FOLEY_DANCER,
	FOLEY_FINAL_FANFARE,
	FOLEY_CRAZY_DAVE_SCREAM,
	FOLEY_CRAZY_DAVE_SCREAM_2,
	NUM_FOLEY
};

class FoleyParams
{
public:
	FoleyType           mFoleyType;
	float               mPitchRange;
	intptr_t*           mSfxID[10];
	unsigned int        mFoleyFlags;
};

void         PvzpFoleyInitialize(const FoleyParams* theFoleyParamArray, int theFoleyParamArraySize);
void         PvzpFoleyDispose();
const FoleyParams*      LookupFoley(FoleyType theFoleyType);

extern int gFoleyParamArraySize;
extern const FoleyParams* gFoleyParamArray;

extern const FoleyParams gLawnFoleyParamArray[static_cast<int>(FoleyType::NUM_FOLEY)];

// Foley sound declarations

class PvzpDSoundInstance : public SDLSoundInstance
{
	friend class PvzpFoley;

public:
	PvzpDSoundInstance(SDLSoundManager* theSoundManager, Mix_Chunk* theSourceSound) : SDLSoundInstance(theSoundManager, theSourceSound) { }

	int      GetSoundPosition();
	void     SetSoundPosition(int thePosition);
};

class FoleyInstance
{
public:
	SoundInstance*      mInstance;
	int                 mRefCount;
	bool                mPaused;
	uint                mStartTime;
	int                 mPauseOffset;

public:
	FoleyInstance();
};

class FoleyTypeData
{
public:
	FoleyInstance       mFoleyInstances[MAX_FOLEY_INSTANCES];
	int                 mLastVariationPlayed;

public:
	FoleyTypeData();
};

class PvzpFoley
{
public:
	FoleyTypeData	    mFoleyTypeData[MAX_FOLEY_TYPES];

public:
	void                PlayFoley(FoleyType theFoleyType);
	void                StopFoley(FoleyType theFoleyType);
	bool                IsFoleyPlaying(FoleyType theFoleyType);
	void                GamePause(bool theEnteringPause);
	void                PlayFoleyPitch(FoleyType theFoleyType, float thePitch);
	void                CancelPausedFoley();
	void     ApplyMusicVolume(FoleyInstance* theFoleyInstance);
	void                RehookupSoundWithMusicVolume();
};

void                    SoundSystemReleaseFinishedInstances(PvzpFoley* theSoundSystem);
bool                    SoundSystemHasFoleyPlayedTooRecently(PvzpFoley* theSoundSystem, FoleyType theFoleyType);
FoleyInstance*          SoundSystemFindInstance(PvzpFoley* theSoundSystem, FoleyType theFoleyType);
FoleyInstance*          SoundSystemGetFreeInstanceIndex(PvzpFoley* theSoundSystem, FoleyType theFoleyType);

#endif
