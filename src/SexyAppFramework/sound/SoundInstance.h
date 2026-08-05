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

#ifndef __SOUNDINSTANCE_H__
#define __SOUNDINSTANCE_H__

#include "Common.h"

namespace Sexy
{

class SoundInstance
{
public:
	SoundInstance() {}
	virtual ~SoundInstance() = default;
	virtual void			Release() = 0;

	virtual void			SetBaseVolume(double theBaseVolume) = 0;
	virtual void			SetBasePan(int theBasePan) = 0;

	virtual void			AdjustPitch(double theNumSteps) = 0;

	virtual void			SetVolume(double theVolume) = 0;
	virtual void			SetPan(int thePosition) = 0; //-hundredth db to +hundredth db = left to right

	virtual bool			Play(bool looping, bool autoRelease) = 0;
	virtual void			Stop() = 0;
	virtual bool			IsPlaying() = 0;
	virtual bool			IsReleased() = 0;
	virtual double			GetVolume() = 0;
};

}

#endif //__SOUNDINSTANCE_H__
