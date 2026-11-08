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

#ifndef __OPENMPTMUSIC_H__
#define __OPENMPTMUSIC_H__

#include <SDL3/SDL.h>
#include <libopenmpt/libopenmpt.h>
#include <libopenmpt/libopenmpt_ext.h>
#include <array>

namespace Sexy
{

class OpenMptMusic
{
public:
	OpenMptMusic() = default;
	~OpenMptMusic();
	OpenMptMusic(const OpenMptMusic&) = delete;
	OpenMptMusic& operator=(const OpenMptMusic&) = delete;

	bool Load(const void* theData, size_t theSize, int theSampleRate);
	SDL_AudioStream* GetStream() const { return mStream; }

	void Rewind(int thePackedOrder);
	void SetLooping(bool theLooping);
	void JumpToOrder(int thePackedOrder);
	int GetOrder() const;
	void SetChannelVolume(int theChannel, double theVolume);

private:
	static void SDLCALL RenderCallback(void* theUserData, SDL_AudioStream* theStream, int theAdditionalAmount, int theTotalAmount);
	void Render(int theByteCount);

	openmpt_module_ext* mModuleExt = nullptr;
	openmpt_module* mModule = nullptr;
	openmpt_module_ext_interface_interactive mInteractive = {};
	SDL_AudioStream* mStream = nullptr;
	int mSampleRate = 44100;
	std::array<float, 2048 * 2> mBuffer = {};
};

}

#endif
