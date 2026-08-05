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

#ifndef __MUSICINTERFACE_H__
#define __MUSICINTERFACE_H__

#include "Common.h"

namespace Sexy
{

class MusicInterface
{
public:
	MusicInterface() {}
	virtual ~MusicInterface() = default;

	virtual bool			LoadMusic(int theSongId, const std::string& theFileName) = 0;
	virtual void			PlayMusic(int theSongId, int theOffset = 0, bool noLoop = false) = 0;
	virtual void			StopMusic(int theSongId) = 0;
	virtual void			PauseMusic(int theSongId) = 0;
	virtual void			ResumeMusic(int theSongId) = 0;
	virtual void			StopAllMusic() = 0;

	virtual void			UnloadMusic(int theSongId) = 0;
	virtual void			UnloadAllMusic() = 0;
	virtual void			PauseAllMusic() = 0;
	virtual void			ResumeAllMusic() = 0;

	virtual void			FadeIn(int theSongId, int theOffset = -1, double theSpeed = 0.002, bool noLoop = false) = 0;
	virtual void			FadeOut(int theSongId, bool stopSong = true, double theSpeed = 0.004) = 0;
	virtual void			FadeOutAll(bool stopSong = true, double theSpeed = 0.004) = 0;
	virtual void			SetSongVolume(int theSongId, double theVolume) = 0;
	virtual void			SetSongMaxVolume(int theSongId, double theMaxVolume) = 0;
	virtual bool			IsPlaying(int theSongId) = 0;

	virtual void			SetVolume(double theVolume) = 0;
	virtual void			SetMusicAmplify(int theSongId, double theAmp) = 0;
	virtual void			Update() = 0;
};
}

#endif //__MUSICINTERFACE_H__
