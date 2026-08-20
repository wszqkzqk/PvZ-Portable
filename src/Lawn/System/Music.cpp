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

#include "Music.h"
#include "../Board.h"
#include "PlayerInfo.h"
#include "../../LawnApp.h"
#include "paklib/PakInterface.h"
#include "../../PvzpLib/PvzpDebug.h"
#include "../../PvzpLib/PvzpCommon.h"
#include "sound/SDLMusicInterface.h"

using namespace Sexy;

Music::Music()
{
	mApp = (LawnApp*)gSexyAppBase;
	mMusicInterface = gSexyAppBase->mMusicInterface.get();
	mCurMusicTune = MusicTune::MUSIC_TUNE_NONE;
	mCurMusicFileMain = MusicFile::MUSIC_FILE_NONE;
	mCurMusicFileDrums = MusicFile::MUSIC_FILE_NONE;
	mCurMusicFileHihats = MusicFile::MUSIC_FILE_NONE;
	mBurstOverride = -1;
	mMusicDrumsState = MusicDrumsState::MUSIC_DRUMS_OFF;
	mQueuedDrumTrackPackedOrder = -1;
	mBaseBPM = 155;
	mBaseModSpeed = 3;
	mMusicBurstState = MusicBurstState::MUSIC_BURST_OFF;
	mPauseOffset = 0;
	mPauseOffsetDrums = 0;
	mPaused = false;
	mMusicDisabled = false;
	mFadeOutCounter = 0;
	mFadeOutDuration = 0;
}

MusicFileData gMusicFileData[MusicFile::NUM_MUSIC_FILES];

struct MusicLoadEntry
{
	MusicFile					mMusicFile;
	std::string_view			mFileName;
};

static constexpr int MUSIC_LOADING_TASK_WEIGHT = 3500;
static constexpr MusicLoadEntry MUSIC_LOADING_FILES[] = {
	{MusicFile::MUSIC_FILE_DRUMS, "sounds/mainmusic.mo3"},
	{MusicFile::MUSIC_FILE_CREDITS_ZOMBIES_ON_YOUR_LAWN, "sounds/ZombiesOnYourLawn.ogg"}
};

const int Music::MUSIC_LOADING_TASKS = MUSIC_LOADING_TASK_WEIGHT * static_cast<int>(sizeof(MUSIC_LOADING_FILES) / sizeof(MUSIC_LOADING_FILES[0]));

bool Music::PvzpLoadMusic(MusicFile theMusicFile, std::string_view theFileName)
{
	Mix_Music* aHMusic = 0;
	SDLMusicInterface* anSDL = (SDLMusicInterface*)mApp->mMusicInterface.get();
	std::string aFileName(theFileName);
	std::string anExt;

	size_t aDot = aFileName.rfind('.');
	if (aDot != std::string::npos)
		anExt = StringToLower(aFileName.substr(aDot + 1));

	PFILE* pFile = p_fopen(aFileName.c_str(), "rb");
	if (pFile == nullptr)
		return false;

	p_fseek(pFile, 0, SEEK_END);
	int aSize = p_ftell(pFile);
	p_fseek(pFile, 0, SEEK_SET);
	void* aData = operator new[](aSize);
	p_fread(aData, sizeof(char), aSize, pFile);
	p_fclose(pFile);

	aHMusic = Mix_LoadMUS_RW(SDL_RWFromMem(aData, aSize), 1);
	if (theMusicFile == MusicFile::MUSIC_FILE_CREDITS_ZOMBIES_ON_YOUR_LAWN)
	{
		gMusicFileData[theMusicFile].mFileData = (unsigned int*)aData;
	}
	else
	{
		delete[] (char *)aData;
	}

	if (aHMusic == 0)
		return false;

	SDLMusicInfo aMusicInfo;
	aMusicInfo.mHMusic = aHMusic;
	anSDL->mMusicMap.insert(SDLMusicMap::value_type(theMusicFile, aMusicInfo));
	return true;
}

void Music::SetupVolumeForTune(MusicTune theMusicTune, float theDrumsVolume, float theHihatsVolume)
{
	constexpr const int TRACK_COUNT = 30;
	int aMainEnd = 29;
	int aDrumsStart = -1, aDrumsEnd = -1;
	int aHihatsStart1 = -1, aHihatsEnd1 = -1, aHihatsStart2 = -1, aHihatsEnd2 = -1;

	switch (theMusicTune)
	{
	case MusicTune::MUSIC_TUNE_DAY_GRASSWALK:
		aMainEnd = 23;
		aDrumsStart = 24;	aDrumsEnd = 26;
		aHihatsStart1 = 27;	aHihatsEnd1 = 27;
		break;
	case MusicTune::MUSIC_TUNE_POOL_WATERYGRAVES:
		aMainEnd = 17;
		aDrumsStart = 18;	aDrumsEnd = 28;
		aHihatsStart1 = 18;	aHihatsEnd1 = 24;	aHihatsStart2 = 29;	aHihatsEnd2 = 29;
		break;
	case MusicTune::MUSIC_TUNE_FOG_RIGORMORMIST:
		aMainEnd = 15;
		aDrumsStart = 16;	aDrumsEnd = 22;
		aHihatsStart1 = 23;	aHihatsEnd1 = 23;
		break;
	case MusicTune::MUSIC_TUNE_ROOF_GRAZETHEROOF:
		aMainEnd = 17;
		aDrumsStart = 18;	aDrumsEnd = 20;
		aHihatsStart1 = 21;	aHihatsEnd1 = 21;
		break;
	default:
		break;
	}

	Mix_Music* aHMusic = GetMusicHandle(MusicFile::MUSIC_FILE_MAIN_MUSIC);
	for (int aTrack = 0; aTrack < TRACK_COUNT; aTrack++)
	{
		float aVolume;
		if (aTrack <= aMainEnd)
			aVolume = 1.0f;
		else
		{
			bool isDrums = (aTrack >= aDrumsStart && aTrack <= aDrumsEnd);
			bool isHihats = (aTrack >= aHihatsStart1 && aTrack <= aHihatsEnd1) ||
							(aTrack >= aHihatsStart2 && aTrack <= aHihatsEnd2);
			if (isDrums && isHihats)
				aVolume = std::max(theDrumsVolume, theHihatsVolume);
			else if (isDrums)
				aVolume = theDrumsVolume;
			else if (isHihats)
				aVolume = theHihatsVolume;
			else
				aVolume = 0.0f;
		}
		Mix_ModMusicStreamSetChannelVolume(aHMusic, aTrack, (int)(aVolume * 128));
	}
}

void Music::LoadSong(MusicFile theMusicFile, std::string_view theFileName)
{
	PvzpHesitationTrace("preloadsong");
	if (!PvzpLoadMusic(theMusicFile, theFileName))
	{
		PvzpTrace("music failed to load\n");
		mMusicDisabled = true;
	}
	else
	{
		PvzpHesitationTrace("song '%.*s'", static_cast<int>(theFileName.size()), theFileName.data());
	}
}

void Music::MusicTitleScreenInit()
{
	LoadSong(MusicFile::MUSIC_FILE_MAIN_MUSIC, "sounds/mainmusic.mo3");
	MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_TITLE_CRAZY_DAVE_MAIN_THEME);
}

void Music::MusicInit()
{
	for (const auto& aMusic : MUSIC_LOADING_FILES)
	{
		LoadSong(aMusic.mMusicFile, aMusic.mFileName);
		mApp->mCompletedLoadingThreadTasks += MUSIC_LOADING_TASK_WEIGHT;
	}
}

void Music::MusicCreditScreenInit()
{
	SDLMusicInterface* anSDL = (SDLMusicInterface*)mApp->mMusicInterface.get();
	if (anSDL->mMusicMap.find((int)MusicFile::MUSIC_FILE_CREDITS_ZOMBIES_ON_YOUR_LAWN) == anSDL->mMusicMap.end())
		LoadSong(MusicFile::MUSIC_FILE_CREDITS_ZOMBIES_ON_YOUR_LAWN, "sounds/ZombiesOnYourLawn.ogg");
}

void Music::StopAllMusic()
{
	if (mMusicInterface != nullptr)
	{
		if (mCurMusicFileMain != MusicFile::MUSIC_FILE_NONE)
			mMusicInterface->StopMusic(mCurMusicFileMain);
		if (mCurMusicFileDrums != MusicFile::MUSIC_FILE_NONE)
			mMusicInterface->StopMusic(mCurMusicFileDrums);
	}

	mCurMusicTune = MusicTune::MUSIC_TUNE_NONE;
	mCurMusicFileMain = MusicFile::MUSIC_FILE_NONE;
	mCurMusicFileDrums = MusicFile::MUSIC_FILE_NONE;
	mCurMusicFileHihats = MusicFile::MUSIC_FILE_NONE;
	mQueuedDrumTrackPackedOrder = -1;
	mMusicDrumsState = MusicDrumsState::MUSIC_DRUMS_OFF;
	mMusicBurstState = MusicBurstState::MUSIC_BURST_OFF;
	mPauseOffset = 0;
	mPauseOffsetDrums = 0;
	mPaused = false;
	mFadeOutCounter = 0;
}

Mix_Music* Music::GetMusicHandle(MusicFile theMusicFile)
{
	SDLMusicInterface* anSDL = (SDLMusicInterface*)mApp->mMusicInterface.get();
	auto anItr = anSDL->mMusicMap.find((int)theMusicFile);
	PVZP_ASSERT(anItr != anSDL->mMusicMap.end());
	return anItr->second.mHMusic;
}

void Music::PlayFromOffset(MusicFile theMusicFile, int theOffset, double theVolume)
{
	SDLMusicInterface* anSDL = (SDLMusicInterface*)mApp->mMusicInterface.get();
	auto anItr = anSDL->mMusicMap.find((int)theMusicFile);
	PVZP_ASSERT(anItr != anSDL->mMusicMap.end());
	SDLMusicInfo* aMusicInfo = &anItr->second;

	if (mCurMusicTune == MusicTune::MUSIC_TUNE_CREDITS_ZOMBIES_ON_YOUR_LAWN)
	{
		bool aNoLoop = theMusicFile == MusicFile::MUSIC_FILE_CREDITS_ZOMBIES_ON_YOUR_LAWN;
		mMusicInterface->PlayMusic(theMusicFile, theOffset, aNoLoop);
	}
	else
	{
		Mix_HaltMusicStream(aMusicInfo->mHMusic);
		aMusicInfo->mStopOnFade = false;
		aMusicInfo->mVolume = aMusicInfo->mVolumeCap * theVolume;
		aMusicInfo->mVolumeAdd = 0.0;
		Mix_PlayMusicStream(aMusicInfo->mHMusic, -1);
		Mix_ModMusicStreamJumpToOrder(aMusicInfo->mHMusic, theOffset);
		Mix_VolumeMusicStream(aMusicInfo->mHMusic, (int)(aMusicInfo->mVolume*128));
		SetupVolumeForTune(mCurMusicTune, 0, 0);
	}
}

void Music::PlayMusic(MusicTune theMusicTune, int theOffset, int theDrumsOffset)
{
	if (mMusicDisabled)
		return;

	mCurMusicTune = theMusicTune;
	mCurMusicFileMain = MusicFile::MUSIC_FILE_NONE;
	mCurMusicFileDrums = MusicFile::MUSIC_FILE_NONE;
	mCurMusicFileHihats = MusicFile::MUSIC_FILE_NONE;

	switch (theMusicTune)
	{
	case MusicTune::MUSIC_TUNE_DAY_GRASSWALK:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_MAIN_MUSIC;
		if (theOffset == -1)
			theOffset = 0;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		break;

	case MusicTune::MUSIC_TUNE_NIGHT_MOONGRAINS:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_MAIN_MUSIC;
		mCurMusicFileDrums = MusicFile::MUSIC_FILE_DRUMS;
		if (theOffset == -1)
		{
			theOffset = 0x30;
			theDrumsOffset = 0x5C;
		}
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		PlayFromOffset(mCurMusicFileDrums, theDrumsOffset, 0.0);
		break;

	case MusicTune::MUSIC_TUNE_POOL_WATERYGRAVES:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_MAIN_MUSIC;
		if (theOffset == -1)
			theOffset = 0x5E;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		break;

	case MusicTune::MUSIC_TUNE_FOG_RIGORMORMIST:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_MAIN_MUSIC;
		if (theOffset == -1)
			theOffset = 0x7D;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		break;

	case MusicTune::MUSIC_TUNE_ROOF_GRAZETHEROOF:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_MAIN_MUSIC;
		if (theOffset == -1)
			theOffset = 0xB8;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		break;

	case MusicTune::MUSIC_TUNE_CHOOSE_YOUR_SEEDS:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_MAIN_MUSIC;
		if (theOffset == -1)
			theOffset = 0x7A;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		break;

	case MusicTune::MUSIC_TUNE_TITLE_CRAZY_DAVE_MAIN_THEME:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_MAIN_MUSIC;
		if (theOffset == -1)
			theOffset = 0x98;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		break;

	case MusicTune::MUSIC_TUNE_ZEN_GARDEN:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_MAIN_MUSIC;
		if (theOffset == -1)
			theOffset = 0xDD;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		break;

	case MusicTune::MUSIC_TUNE_PUZZLE_CEREBRAWL:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_MAIN_MUSIC;
		if (theOffset == -1)
			theOffset = 0xB1;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		break;

	case MusicTune::MUSIC_TUNE_MINIGAME_LOONBOON:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_MAIN_MUSIC;
		if (theOffset == -1)
			theOffset = 0xA6;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		break;

	case MusicTune::MUSIC_TUNE_CONVEYER:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_MAIN_MUSIC;
		if (theOffset == -1)
			theOffset = 0xD4;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		break;

	case MusicTune::MUSIC_TUNE_FINAL_BOSS_BRAINIAC_MANIAC:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_MAIN_MUSIC;
		if (theOffset == -1)
			theOffset = 0x9E;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		break;

	case MusicTune::MUSIC_TUNE_CREDITS_ZOMBIES_ON_YOUR_LAWN:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_CREDITS_ZOMBIES_ON_YOUR_LAWN;
		if (theOffset == -1)
			theOffset = 0;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		break;

	default:
		PVZP_ASSERT(false);
		break;
	}
}

unsigned long Music::GetMusicOrder(MusicFile theMusicFile)
{
	PVZP_ASSERT(theMusicFile != MusicFile::MUSIC_FILE_NONE);
	return ((SDLMusicInterface*)mApp->mMusicInterface.get())->GetMusicOrder((int)theMusicFile);
}

void Music::StartBurst()
{
	if (mMusicBurstState == MusicBurstState::MUSIC_BURST_OFF)
	{
		mMusicBurstState = MusicBurstState::MUSIC_BURST_STARTING;
		mBurstStateCounter = 400;
	}
}

void Music::FadeOut(int theFadeOutDuration)
{
	if (mCurMusicTune != MusicTune::MUSIC_TUNE_NONE)
	{
		mFadeOutCounter = theFadeOutDuration;
		mFadeOutDuration = theFadeOutDuration;
	}
}

void Music::UpdateMusicBurst()
{
	if (mApp->mBoard == nullptr)
		return;
	if (mApp->mGameMode == GameMode::GAMEMODE_INTRO)
		return;

	int aBurstScheme;
	if (mCurMusicTune == MusicTune::MUSIC_TUNE_DAY_GRASSWALK || mCurMusicTune == MusicTune::MUSIC_TUNE_POOL_WATERYGRAVES ||
		mCurMusicTune == MusicTune::MUSIC_TUNE_FOG_RIGORMORMIST || mCurMusicTune == MusicTune::MUSIC_TUNE_ROOF_GRAZETHEROOF)
		aBurstScheme = 1;
	else if (mCurMusicTune == MusicTune::MUSIC_TUNE_NIGHT_MOONGRAINS)
		aBurstScheme = 2;
	else
		return;

	int aPackedOrderMain = GetMusicOrder(mCurMusicFileMain);
	if (mBurstStateCounter > 0)
		mBurstStateCounter--;
	if (mDrumsStateCounter > 0)
		mDrumsStateCounter--;

	float aFadeTrackVolume = 0.0f;
	float aDrumsVolume = 0.0f;
	float aMainTrackVolume = 1.0f;
	switch (mMusicBurstState)
	{
		case MusicBurstState::MUSIC_BURST_OFF:
			if (mApp->mBoard->CountZombiesOnScreen() >= 10 || mBurstOverride == 1)
				StartBurst();
			break;
		case MusicBurstState::MUSIC_BURST_STARTING:
			if (aBurstScheme == 1)
			{
				aFadeTrackVolume = PvzpAnimateCurveFloat(400, 0, mBurstStateCounter, 0.0f, 1.0f, PvzpCurves::CURVE_LINEAR);
				if (mBurstStateCounter == 100)
				{
					mMusicDrumsState = MusicDrumsState::MUSIC_DRUMS_ON_QUEUED;
					mQueuedDrumTrackPackedOrder = aPackedOrderMain;
				}
				else if (mBurstStateCounter == 0)
				{
					mMusicBurstState = MusicBurstState::MUSIC_BURST_ON;
					mBurstStateCounter = 800;
				}
			}
			else if (aBurstScheme == 2)
			{
				if (mMusicDrumsState == MusicDrumsState::MUSIC_DRUMS_OFF)
				{
					mMusicDrumsState = MusicDrumsState::MUSIC_DRUMS_ON_QUEUED;
					mQueuedDrumTrackPackedOrder = aPackedOrderMain;
					mBurstStateCounter = 400;
				}
				else if (mMusicDrumsState == MusicDrumsState::MUSIC_DRUMS_ON_QUEUED)
					mBurstStateCounter = 400;
				else
				{
					aMainTrackVolume = PvzpAnimateCurveFloat(400, 0, mBurstStateCounter, 1.0f, 0.0f, PvzpCurves::CURVE_LINEAR);
					if (mBurstStateCounter == 0)
					{
						mMusicBurstState = MusicBurstState::MUSIC_BURST_ON;
						mBurstStateCounter = 800;
					}
				}
			}
			break;
		case MusicBurstState::MUSIC_BURST_ON:
			aFadeTrackVolume = 1.0f;
			if (aBurstScheme == 2)
				aMainTrackVolume = 0.0f;
			if (mBurstStateCounter == 0 && ((mApp->mBoard->CountZombiesOnScreen() < 4 && mBurstOverride == -1) || mBurstOverride == 2))
			{
				if (aBurstScheme == 1)
				{
					mMusicBurstState = MusicBurstState::MUSIC_BURST_FINISHING;
					mBurstStateCounter = 800;
					mMusicDrumsState = MusicDrumsState::MUSIC_DRUMS_OFF_QUEUED;
					mQueuedDrumTrackPackedOrder = aPackedOrderMain;
				}
				else if (aBurstScheme == 2)
				{
					mMusicBurstState = MusicBurstState::MUSIC_BURST_FINISHING;
					mBurstStateCounter = 1100;
					mMusicDrumsState = MusicDrumsState::MUSIC_DRUMS_FADING;
					mDrumsStateCounter = 800;
				}
			}
			break;
		case MusicBurstState::MUSIC_BURST_FINISHING:
			if (aBurstScheme == 1)
				aFadeTrackVolume = PvzpAnimateCurveFloat(800, 0, mBurstStateCounter, 1.0f, 0.0f, PvzpCurves::CURVE_LINEAR);
			else
				aMainTrackVolume = PvzpAnimateCurveFloat(400, 0, mBurstStateCounter, 0.0f, 1.0f, PvzpCurves::CURVE_LINEAR);
			if (mBurstStateCounter == 0 && mMusicDrumsState == MusicDrumsState::MUSIC_DRUMS_OFF)
				mMusicBurstState = MusicBurstState::MUSIC_BURST_OFF;
			break;
	}

	int aDrumsJumpOrder = -1;
	int aOrderMain = 0, aOrderDrum = 0;
	if (aBurstScheme == 1)
	{
		aOrderMain = aPackedOrderMain & 0xFFFF;
		aOrderDrum = mQueuedDrumTrackPackedOrder & 0xFFFF;
	}
	else if (aBurstScheme == 2)
	{
		aOrderMain = aPackedOrderMain & 0xFFFF;
		aOrderDrum = mQueuedDrumTrackPackedOrder & 0xFFFF;
	}

	switch (mMusicDrumsState)
	{
		case MusicDrumsState::MUSIC_DRUMS_ON_QUEUED:
			if (aOrderMain != aOrderDrum)
			{
				aDrumsVolume = 1.0f;
				mMusicDrumsState = MusicDrumsState::MUSIC_DRUMS_ON;
				if (aBurstScheme == 2)
					aDrumsJumpOrder = (aOrderMain % 2 == 0) ? 76 : 77;
			}
			break;
		case MusicDrumsState::MUSIC_DRUMS_ON:
			aDrumsVolume = 1.0f;
			break;
		case MusicDrumsState::MUSIC_DRUMS_OFF_QUEUED:
			aDrumsVolume = 1.0f;
			if (aOrderMain != aOrderDrum && aBurstScheme == 1)
			{
				mMusicDrumsState = MusicDrumsState::MUSIC_DRUMS_FADING;
				mDrumsStateCounter = 50;
			}
			break;
		case MusicDrumsState::MUSIC_DRUMS_FADING:
			if (aBurstScheme == 2)
				aDrumsVolume = PvzpAnimateCurveFloat(800, 0, mDrumsStateCounter, 1.0f, 0.0f, PvzpCurves::CURVE_LINEAR);
			else
				aDrumsVolume = PvzpAnimateCurveFloat(50, 0, mDrumsStateCounter, 1.0f, 0.0f, PvzpCurves::CURVE_LINEAR);
			if (mDrumsStateCounter == 0)
				mMusicDrumsState = MusicDrumsState::MUSIC_DRUMS_OFF;
			break;
		case MusicDrumsState::MUSIC_DRUMS_OFF:
			break;
	}

	if (aBurstScheme == 1)
	{
		SetupVolumeForTune(mCurMusicTune, aDrumsVolume, aFadeTrackVolume);
	}
	else if (aBurstScheme == 2)
	{
		mMusicInterface->SetSongVolume(mCurMusicFileMain, aMainTrackVolume);
		mMusicInterface->SetSongVolume(mCurMusicFileDrums, aDrumsVolume);
		if (aDrumsJumpOrder != -1)
			Mix_ModMusicStreamJumpToOrder(GetMusicHandle(mCurMusicFileDrums), aDrumsJumpOrder);
	}
}

void Music::MusicUpdate()
{
	if (mFadeOutCounter > 0)
	{
		mFadeOutCounter--;
		if (mFadeOutCounter == 0)
			StopAllMusic();
		else
		{
			float aFadeLevel = PvzpAnimateCurveFloat(mFadeOutDuration, 0, mFadeOutCounter, 1.0f, 0.0f, PvzpCurves::CURVE_LINEAR);
			mMusicInterface->SetSongVolume(mCurMusicFileMain, aFadeLevel);
		}
	}

	if (mApp->mBoard == nullptr || !mApp->mBoard->mPaused)
	{
		UpdateMusicBurst();
	}
}

void Music::MakeSureMusicIsPlaying(MusicTune theMusicTune)
{
	if (mCurMusicTune != theMusicTune)
	{
		StopAllMusic();
		PlayMusic(theMusicTune, -1, -1);
	}
}

void Music::StartGameMusic()
{
	PVZP_ASSERT(mApp->mBoard);

	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN || mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM)
		MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_ZEN_GARDEN);
	else if (mApp->IsFinalBossLevel())
		MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_FINAL_BOSS_BRAINIAC_MANIAC);
	else if (mApp->IsWallnutBowlingLevel() || mApp->IsWhackAZombieLevel() || mApp->IsLittleTroubleLevel() || mApp->IsBungeeBlitzLevel() ||
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_SPEED)
		MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_MINIGAME_LOONBOON);
	else if ((mApp->IsAdventureMode() && (mApp->mPlayerInfo->GetLevel() == 10 || mApp->mPlayerInfo->GetLevel() == 20 || mApp->mPlayerInfo->GetLevel() == 30)) ||
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_COLUMN)
		MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_CONVEYER);
	else if (mApp->IsStormyNightLevel())
		StopAllMusic();
	else if (mApp->IsScaryPotterLevel() || mApp->IsIZombieLevel())
		MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_PUZZLE_CEREBRAWL);
	else if (mApp->mBoard->StageIsNight())
	{
		if (mApp->mBoard->StageHasPool())
			MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_FOG_RIGORMORMIST);
		else
			MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_NIGHT_MOONGRAINS);
	}
	else if (mApp->mBoard->StageHas6Rows())
		MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_POOL_WATERYGRAVES);
	else if (mApp->mBoard->StageHasRoof())
		MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_ROOF_GRAZETHEROOF);
	else
		MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_DAY_GRASSWALK);
}

void Music::GameMusicPause(bool thePause)
{
	if (thePause)
	{
		if (!mPaused && mCurMusicTune != MusicTune::MUSIC_TUNE_NONE)
		{
			if (mCurMusicFileMain != MusicFile::MUSIC_FILE_NONE &&
				mCurMusicTune != MusicTune::MUSIC_TUNE_CREDITS_ZOMBIES_ON_YOUR_LAWN)
			{
				mPauseOffset = GetMusicOrder(mCurMusicFileMain);
			}
			if (mCurMusicTune == MusicTune::MUSIC_TUNE_NIGHT_MOONGRAINS &&
				mCurMusicFileDrums != MusicFile::MUSIC_FILE_NONE)
			{
				mPauseOffsetDrums = GetMusicOrder(mCurMusicFileDrums);
			}

			if (mCurMusicFileMain != MusicFile::MUSIC_FILE_NONE)
				mMusicInterface->PauseMusic(mCurMusicFileMain);
			if (mCurMusicFileDrums != MusicFile::MUSIC_FILE_NONE)
				mMusicInterface->PauseMusic(mCurMusicFileDrums);

			mPaused = true;
		}
	}
	else if (mPaused)
	{
		if (mCurMusicTune != MusicTune::MUSIC_TUNE_NONE)
		{
			Mix_Music* aHandle = (mCurMusicFileMain != MusicFile::MUSIC_FILE_NONE) ?
								  GetMusicHandle(mCurMusicFileMain) : nullptr;
			if (mCurMusicTune == MusicTune::MUSIC_TUNE_CREDITS_ZOMBIES_ON_YOUR_LAWN ||
				(aHandle && Mix_PlayingMusicStream(aHandle)))
			{
				if (mCurMusicFileMain != MusicFile::MUSIC_FILE_NONE)
					mMusicInterface->ResumeMusic(mCurMusicFileMain);
				if (mCurMusicFileDrums != MusicFile::MUSIC_FILE_NONE)
					mMusicInterface->ResumeMusic(mCurMusicFileDrums);
			}
			else
			{
				PlayMusic(mCurMusicTune, mPauseOffset, mPauseOffsetDrums);
			}
		}
		mPaused = false;
	}
}
