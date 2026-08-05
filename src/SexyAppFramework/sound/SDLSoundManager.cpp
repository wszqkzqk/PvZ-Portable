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

#include "SDLSoundManager.h"
#include "SDLSoundInstance.h"
#include "paklib/PakInterface.h"
#include <vector>

using namespace Sexy;

SDLSoundManager::SDLSoundManager()
{
	mInitializedMixer = false;
	mLastReleaseTick = 0;
	mMasterVolume = 1.0;
	mMixerFreq = 0;
	mMixerFormat = 0;
	mMixerChannels = 0;

	int i;

	for (i = 0; i < MAX_SOURCE_SOUNDS; i++)
	{
		mSourceSounds[i] = nullptr;
		mBaseVolumes[i] = 1;
		mBasePans[i] = 0;
	}

	for (i = 0; i < MAX_CHANNELS; i++)
		mPlayingSounds[i] = nullptr;

	if (SDL_InitSubSystem(SDL_INIT_AUDIO))
	{
		Sexy::PrintF("Failed to initialize SDL audio subsystem\n");
		return;
	}

	if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 2048))
	{
		Sexy::PrintF("Failed to initialize SDL mixer\n");
		return;
	}
	mInitializedMixer = true;

	Mix_QuerySpec(&mMixerFreq, &mMixerFormat, &mMixerChannels);
	Mix_AllocateChannels(MAX_CHANNELS);
}

SDLSoundManager::~SDLSoundManager()
{
	if (mInitializedMixer)
		Mix_CloseAudio();

	if (SDL_WasInit(SDL_INIT_AUDIO))
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

bool SDLSoundManager::Initialized()
{
	return SDL_WasInit(SDL_INIT_AUDIO) && mInitializedMixer;
}

bool SDLSoundManager::LoadAUSound(intptr_t theSfxID, const std::string& theFilename)
{
	PFILE* fp;

	fp = p_fopen(theFilename.c_str(), "rb");

	if (fp == nullptr)
		return false;

	char aHeaderId[5];
	aHeaderId[4] = '\0';
	p_fread(aHeaderId, 1, 4, fp);
	if ((!strcmp(aHeaderId, ".snd")) == 0)
		return false;

	uint32_t aHeaderSize;
	p_fread(&aHeaderSize, 4, 1, fp);
	aHeaderSize = FromBE32(aHeaderSize);

	uint32_t aDataSize;
	p_fread(&aDataSize, 4, 1, fp);
	aDataSize = FromBE32(aDataSize);

	uint32_t anEncoding;
	p_fread(&anEncoding, 4, 1, fp);
	anEncoding = FromBE32(anEncoding);

	uint32_t aSampleRate;
	p_fread(&aSampleRate, 4, 1, fp);
	aSampleRate = FromBE32(aSampleRate);

	uint32_t aChannelCount;
	p_fread(&aChannelCount, 4, 1, fp);
	aChannelCount = FromBE32(aChannelCount);

	p_fseek(fp, aHeaderSize, SEEK_SET);

	bool ulaw = false;

	uint32_t aSrcBitCount = 8;
	uint32_t aBitCount = 16;
	switch (anEncoding)
	{
	case 1:
		aSrcBitCount = 8;
		aBitCount = 16;
		ulaw = true;
		break;
	case 2:
		aSrcBitCount = 8;
		aBitCount = 8;
		break;

	/*
	Support these formats?

	case 3:
		aBitCount = 16;
		break;
	case 4:
		aBitCount = 24;
		break;
	case 5:
		aBitCount = 32;
		break;*/

	default:
		return false;
	}

	uint32_t aDestSize = aDataSize * aBitCount/aSrcBitCount;

	Mix_Chunk* aMixChunk = (Mix_Chunk *)SDL_malloc(sizeof(Mix_Chunk));
	uint8_t* aDest = (uint8_t*)SDL_malloc(aDestSize);
	uint8_t* aSrcBuffer = (uint8_t*)malloc(aDataSize);

	uint32_t aReadSize = p_fread(aSrcBuffer, 1, aDataSize, fp);
	p_fclose(fp);

	if (ulaw)
	{
		short* aDestBuffer = (short*)aDest;

		for (ulong i = 0; i < aDataSize; i++)
		{
			int ch = aSrcBuffer[i];

			int sign = (ch < 128) ? -1 : 1;
			ch = ch | 0x80;
			if (ch > 239)
				ch = ((0xF0 | 15) - ch) * 2;
			else if (ch > 223)
				ch = (((0xE0 | 15) - ch) * 4) + 32;
			else if (ch > 207)
				ch = (((0xD0 | 15) - ch) * 8) + 96;
			else if (ch > 191)
				ch = (((0xC0 | 15) - ch) * 16) + 224;
			else if (ch > 175)
				ch = (((0xB0 | 15) - ch) * 32) + 480;
			else if (ch > 159)
				ch = (((0xA0 | 15) - ch) * 64) + 992;
			else if (ch > 143)
				ch = (((0x90 | 15) - ch) * 128) + 2016;
			else if (ch > 128)
				ch = (((0x80 | 15) - ch) * 256) + 4064;
			else
				ch = 0xff;

			aDestBuffer[i] = sign * ch * 4;
		}
	}
	else
		memcpy(aDest, aSrcBuffer, aDataSize);

	free(aSrcBuffer);

	if (aReadSize != aDataSize)
	{
		SDL_free(aMixChunk);
		SDL_free(aDest);
		return false;
	}

	int freq; uint16_t format; int channels;
	int srcfreq; uint16_t srcformat; int srcchannels;
	Mix_QuerySpec(&freq, &format, &channels);
	srcfreq = 8000; srcformat = AUDIO_S16SYS; srcchannels = 1;

	// https://github.com/libsdl-org/SDL_mixer/blob/SDL2/src/mixer.c#L852
	// Build the audio converter and create conversion buffers
	SDL_AudioCVT wavecvt;
	if (SDL_BuildAudioCVT(&wavecvt,
			srcformat, srcchannels, srcfreq,
			format, channels, freq) < 0)
	{
		SDL_free(aMixChunk);
		SDL_free(aDest);
		return false;
	}
	uint32_t samplesize = ((srcfreq & 0xFF)/8)*srcchannels;
	wavecvt.len = aDestSize & ~(samplesize - 1);
	wavecvt.buf = (uint8_t*)SDL_calloc(1, wavecvt.len*wavecvt.len_mult);
	if (wavecvt.buf == nullptr)
	{
		Mix_OutOfMemory();
		SDL_free(aMixChunk);
		SDL_free(aDest);
		return false;
	}
	SDL_memcpy(wavecvt.buf, aDest, wavecvt.len);
	SDL_free(aDest);

	// Run the audio converter
	if (SDL_ConvertAudio(&wavecvt) < 0) {
		SDL_free(wavecvt.buf);
		SDL_free(aMixChunk);
		return false;
	}

	aDest = (uint8_t*)SDL_realloc(wavecvt.buf, wavecvt.len_cvt);
	if (aDest == nullptr) {
		aMixChunk->abuf = wavecvt.buf;
	} else {
		aMixChunk->abuf = aDest;
	}
	aMixChunk->alen = wavecvt.len_cvt;
	aMixChunk->allocated = 1;
	aMixChunk->volume = 128;

	mSourceSounds[theSfxID] = aMixChunk;
	return true;
}

bool SDLSoundManager::LoadSound(intptr_t theSfxID, const std::string& theFilename)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	ReleaseSound(theSfxID);

	if (!Initialized())
		return true;

	mSourceFileNames[theSfxID] = theFilename;
	const char* formats[] = {".wav", ".mp3", ".ogg"};
	for (int i=0; i<3; i++)
	{
		std::string aFilename = theFilename + formats[i];

		PFILE *fp = p_fopen(aFilename.c_str(), "rb");
		if (!fp)
			continue;

		p_fseek(fp, 0, SEEK_END);
		size_t fileSize = p_ftell(fp);
		p_fseek(fp, 0, SEEK_SET);
		std::vector<uint8_t> data(fileSize);
		p_fread(data.data(), 1, fileSize, fp);
		p_fclose(fp);

		mSourceSounds[theSfxID] = Mix_LoadWAV_RW(SDL_RWFromConstMem(data.data(), fileSize), 1);

		if (mSourceSounds[theSfxID]) break;
	}

	if (!mSourceSounds[theSfxID])
		LoadAUSound(theSfxID, theFilename + ".au");

	return !!mSourceSounds[theSfxID];
}

intptr_t SDLSoundManager::LoadSound(const std::string& theFilename)
{
	intptr_t i;
	for (i = 0; i < MAX_SOURCE_SOUNDS; i++)
		if (mSourceFileNames[i] == theFilename)
			return i;

	for (i = MAX_SOURCE_SOUNDS-1; i >= 0; i--)
	{
		if (mSourceSounds[i] == nullptr)
		{
			if (!LoadSound(i, theFilename))
				return -1;
			else
				return i;
		}
	}

	return -1;
}

void SDLSoundManager::ReleaseSound(intptr_t theSfxID)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return;

	if (mSourceSounds[theSfxID] != nullptr)
	{
		Mix_FreeChunk(mSourceSounds[theSfxID]);
		mSourceSounds[theSfxID] = nullptr;
		mSourceFileNames[theSfxID] = "";
	}
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
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	mBaseVolumes[theSfxID] = theBaseVolume;
	return true;
}

bool SDLSoundManager::SetBasePan(intptr_t theSfxID, int theBasePan)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return false;

	mBasePans[theSfxID] = theBasePan;
	return true;
}

SoundInstance* SDLSoundManager::GetSoundInstance(intptr_t theSfxID)
{
	if ((theSfxID < 0) || (theSfxID >= MAX_SOURCE_SOUNDS))
		return nullptr;

	int aFreeChannel = FindFreeChannel();
	if (aFreeChannel < 0)
		return nullptr;

	if (mSourceSounds[theSfxID] == nullptr)
		return nullptr;

	mPlayingSounds[aFreeChannel] = new SDLSoundInstance(this, mSourceSounds[theSfxID], aFreeChannel);

	mPlayingSounds[aFreeChannel]->SetBasePan(mBasePans[theSfxID]);
	mPlayingSounds[aFreeChannel]->SetBaseVolume(mBaseVolumes[theSfxID]);

	return mPlayingSounds[aFreeChannel];
}

void SDLSoundManager::ReleaseSounds()
{
	for (int i = 0; i < MAX_SOURCE_SOUNDS; i++)
	{
		if (mSourceSounds[i] != nullptr)
		{
			Mix_FreeChunk(mSourceSounds[i]);
			mSourceSounds[i] = nullptr;
		}
	}
}

void SDLSoundManager::ReleaseChannels()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
	{
		if (mPlayingSounds[i] != nullptr)
		{
			delete mPlayingSounds[i];
			mPlayingSounds[i] = nullptr;
		}
	}
}

double SDLSoundManager::GetMasterVolume()
{
	return Mix_MasterVolume(-1) / 128.;
}

void SDLSoundManager::SetMasterVolume(double theVolume)
{
	Mix_MasterVolume((int)(theVolume * 128));
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
			bool isAutoRelease = mPlayingSounds[i]->mAutoRelease;
			mPlayingSounds[i]->Stop();
			mPlayingSounds[i]->mAutoRelease = isAutoRelease;
		}
	}
}

intptr_t SDLSoundManager::GetFreeSoundId()
{
	for (intptr_t i=0; i<MAX_SOURCE_SOUNDS; i++)
	{
		if (mSourceSounds[i]==nullptr)
			return i;
	}

	return -1;
}

int SDLSoundManager::GetNumSounds()
{
	int aCount = 0;
	for (int i=0; i<MAX_SOURCE_SOUNDS; i++)
	{
		if (mSourceSounds[i]!=nullptr)
			aCount++;
	}

	return aCount;
}

int SDLSoundManager::FindFreeChannel()
{
	uint64_t aTick = SDL_GetTicks();
	if (aTick-mLastReleaseTick > 1000)
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
			delete mPlayingSounds[i];
			mPlayingSounds[i] = nullptr;
			return i;
		}
	}

	return -1;
}

void SDLSoundManager::ReleaseFreeChannels()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
	{
		if (mPlayingSounds[i] != nullptr && mPlayingSounds[i]->IsReleased())
		{
			delete mPlayingSounds[i];
			mPlayingSounds[i] = nullptr;
		}
	}
}
