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

#ifndef __MUSIC_H__
#define __MUSIC_H__

#include <cstdint>
#include <string>
#include <string_view>
#include <SDL_mixer_ext/SDL_mixer_ext.h>

class LawnApp;
namespace Sexy
{
	class MusicInterface;
};

enum MusicTune : int32_t
{
	MUSIC_TUNE_NONE = -1,
	MUSIC_TUNE_DAY_GRASSWALK = 1,
	MUSIC_TUNE_NIGHT_MOONGRAINS,
	MUSIC_TUNE_POOL_WATERYGRAVES,
	MUSIC_TUNE_FOG_RIGORMORMIST,
	MUSIC_TUNE_ROOF_GRAZETHEROOF,
	MUSIC_TUNE_CHOOSE_YOUR_SEEDS,				// seed chooser and minigame screens
	MUSIC_TUNE_TITLE_CRAZY_DAVE_MAIN_THEME,
	MUSIC_TUNE_ZEN_GARDEN,
	MUSIC_TUNE_PUZZLE_CEREBRAWL,
	MUSIC_TUNE_MINIGAME_LOONBOON,
	MUSIC_TUNE_CONVEYER,
	MUSIC_TUNE_FINAL_BOSS_BRAINIAC_MANIAC,
	MUSIC_TUNE_CREDITS_ZOMBIES_ON_YOUR_LAWN,
	NUM_MUSIC_TUNES
};

enum MusicFile : int32_t
{
	MUSIC_FILE_NONE = -1,
	MUSIC_FILE_MAIN_MUSIC = 1,
	MUSIC_FILE_DRUMS,
	MUSIC_FILE_HIHATS,
	MUSIC_FILE_CREDITS_ZOMBIES_ON_YOUR_LAWN,
	NUM_MUSIC_FILES
};

enum MusicBurstState : int32_t
{
	MUSIC_BURST_OFF,
	MUSIC_BURST_STARTING,
	MUSIC_BURST_ON,
	MUSIC_BURST_FINISHING
};

enum MusicDrumsState : int32_t
{
	MUSIC_DRUMS_OFF,
	MUSIC_DRUMS_ON_QUEUED,
	MUSIC_DRUMS_ON,
	MUSIC_DRUMS_OFF_QUEUED,
	MUSIC_DRUMS_FADING
};

class MusicFileData
{
public:
	unsigned int*				mFileData;
};
extern MusicFileData gMusicFileData[MusicFile::NUM_MUSIC_FILES];

class Music
{
public:
	LawnApp*					mApp;
	Sexy::MusicInterface*		mMusicInterface;
	MusicTune					mCurMusicTune;
	MusicFile					mCurMusicFileMain;
	MusicFile					mCurMusicFileDrums;
	MusicFile					mCurMusicFileHihats;
	int32_t						mBurstOverride;
	float						mBaseBPM;
	float						mBaseModSpeed;
	MusicBurstState				mMusicBurstState;
	int32_t						mBurstStateCounter;
	MusicDrumsState				mMusicDrumsState;
	int32_t						mQueuedDrumTrackPackedOrder;
	int32_t						mDrumsStateCounter;
	int32_t						mPauseOffset;
	int32_t						mPauseOffsetDrums;
	bool						mPaused;
	bool						mMusicDisabled;
	int32_t						mFadeOutCounter;
	int32_t						mFadeOutDuration;

public:
	Music();

	static const int			MUSIC_LOADING_TASKS;

	void						MusicInit();
	void						MusicDispose() { ; }
	void						MusicUpdate();
	void						StopAllMusic();
	/*inline*/ void				PlayMusic(MusicTune theMusicTune, int theOffset = -1, int theDrumsOffset = -1);
	/*inline*/ Mix_Music*		GetMusicHandle(MusicFile theMusicFile);
	void						StartGameMusic();
	/*inline*/ void				LoadSong(MusicFile theMusicFile, std::string_view theFileName);
	void						MusicResync();
	void						UpdateMusicBurst();
	/*inline*/ void				StartBurst();
	void						GameMusicPause(bool thePause);
	void						PlayFromOffset(MusicFile theMusicFile, int theOffset, double theVolume);
	void						MusicResyncChannel(MusicFile theMusicFileToMatch, MusicFile theMusicFileToSync);
	bool						PvzpLoadMusic(MusicFile theMusicFile, std::string_view theFileName);
	void						MusicTitleScreenInit();
	/*inline*/ void				MakeSureMusicIsPlaying(MusicTune theMusicTune);
	/*inline*/ void				FadeOut(int theFadeOutDuration);
	void						SetupVolumeForTune(MusicTune theMusicTune, float theDrumsVolume, float theHihatsVolume);
	unsigned long				GetMusicOrder(MusicFile theMusicFile);
	void						MusicCreditScreenInit();
};

#endif
