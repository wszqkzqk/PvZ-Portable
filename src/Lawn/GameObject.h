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

#pragma once

#include <cstdint>

#include "../ConstEnums.h"
#include "../SexyAppFramework/graphics/Graphics.h"

using namespace Sexy;

class LawnApp;
class Board;

class GameObject
{
public:
	LawnApp*                        mApp;
	Board*                          mBoard;
	int32_t                         mX;
	int32_t                         mY;
	int32_t                         mWidth;
	int32_t                         mHeight;
	bool                            mVisible;
	int32_t                         mRow;
	int32_t                         mRenderOrder;

public:
	GameObject();
	bool                 BeginDraw(Graphics* g);
	void                 EndDraw(Graphics* g);
	void                 MakeParentGraphicsFrame(Graphics* g);
};
