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

#include "OpenMptMusic.h"
#include "Common.h"
#include <algorithm>

using namespace Sexy;

OpenMptMusic::~OpenMptMusic()
{
	SDL_DestroyAudioStream(mStream);
	if (mModuleExt != nullptr)
		openmpt_module_ext_destroy(mModuleExt);
}

bool OpenMptMusic::Load(const void* theData, size_t theSize, int theSampleRate)
{
	if (theData == nullptr || theSize == 0)
		return false;

	int aError = 0;
	const char* aErrorMessage = nullptr;
	mModuleExt = openmpt_module_ext_create_from_memory(
		theData, theSize, nullptr, nullptr, nullptr, nullptr, &aError, &aErrorMessage, nullptr);
	if (mModuleExt == nullptr)
	{
		LogInfoLn("Failed to load MOD music: {}", aErrorMessage != nullptr ? aErrorMessage : "unknown error");
		return false;
	}

	mModule = openmpt_module_ext_get_module(mModuleExt);
	if (mModule == nullptr || !openmpt_module_ext_get_interface(
		mModuleExt, LIBOPENMPT_EXT_C_INTERFACE_INTERACTIVE, &mInteractive, sizeof(mInteractive)))
	{
		return false;
	}

	mSampleRate = theSampleRate;
	openmpt_module_set_repeat_count(mModule, -1);
	openmpt_module_ctl_set_boolean(mModule, "seek.sync_samples", 0);

	SDL_AudioSpec aSpec = { SDL_AUDIO_F32, 2, mSampleRate };
	mStream = SDL_CreateAudioStream(&aSpec, &aSpec);
	if (mStream == nullptr)
		return false;

	return SDL_SetAudioStreamGetCallback(mStream, &OpenMptMusic::RenderCallback, this);
}

void OpenMptMusic::Rewind(int thePackedOrder)
{
	SDL_ClearAudioStream(mStream);
	openmpt_module_set_position_seconds(mModule, 0.0);
	if (thePackedOrder > 0)
		JumpToOrder(thePackedOrder);
}

void OpenMptMusic::SetLooping(bool theLooping)
{
	openmpt_module_set_repeat_count(mModule, theLooping ? -1 : 0);
}

void OpenMptMusic::JumpToOrder(int thePackedOrder)
{
	const auto aValue = static_cast<uint32_t>(thePackedOrder);
	openmpt_module_set_position_order_row(
		mModule,
		static_cast<int32_t>(aValue & 0xFFFF),
		static_cast<int32_t>((aValue >> 16) & 0xFFFF));
}

int OpenMptMusic::GetOrder() const
{
	const uint32_t aOrder = static_cast<uint32_t>(openmpt_module_get_current_order(mModule));
	const uint32_t aRow = static_cast<uint32_t>(openmpt_module_get_current_row(mModule));
	return static_cast<int>((aRow << 16) | (aOrder & 0xFFFF));
}

void OpenMptMusic::SetChannelVolume(int theChannel, double theVolume)
{
	mInteractive.set_channel_volume(mModuleExt, theChannel, theVolume);
}

void OpenMptMusic::RenderCallback(void* theUserData, SDL_AudioStream*, int theAdditionalAmount, int)
{
	static_cast<OpenMptMusic*>(theUserData)->Render(theAdditionalAmount);
}

void OpenMptMusic::Render(int theByteCount)
{
	while (theByteCount > 0)
	{
		const int aFrames = std::min(theByteCount / static_cast<int>(sizeof(float) * 2), 2048);
		if (aFrames <= 0)
			return;

		const size_t aRendered = openmpt_module_read_interleaved_float_stereo(
			mModule, mSampleRate, static_cast<size_t>(aFrames), mBuffer.data());
		if (aRendered == 0)
			return;

		const int aBytes = static_cast<int>(aRendered * sizeof(float) * 2);
		if (!SDL_PutAudioStreamData(mStream, mBuffer.data(), aBytes))
			return;
		theByteCount -= aBytes;
	}
}
