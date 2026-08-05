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

#include "MusicInterface.h"
#include "SexyAppBase.h"

using namespace Sexy;

class DummyMusicInterface : public MusicInterface
{
public:
	DummyMusicInterface() {}
	~DummyMusicInterface() override = default;

	bool					LoadMusic(int, const std::string&) override{return false;}
	void 					PlayMusic(int, int, bool) override{}
	void 					StopMusic(int) override{}
	void 					PauseMusic(int) override{}
	void 					ResumeMusic(int) override{}
	void 					StopAllMusic() override{}
	void 					UnloadMusic(int) override{}
	void 					UnloadAllMusic() override{}
	void 					PauseAllMusic() override{}
	void 					ResumeAllMusic() override{}
	void 					FadeIn(int, int, double, bool) override{}
	void 					FadeOut(int, bool, double) override{}
	void 					FadeOutAll(bool, double) override{}
	void 					SetSongVolume(int, double) override{}
	void 					SetSongMaxVolume(int, double) override{}
	bool					IsPlaying(int) override{return false;};

	void					SetVolume(double) override{}
	void					SetMusicAmplify(int, double) override{}
	void					Update() override{}
};
