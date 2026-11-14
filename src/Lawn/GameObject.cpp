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

#include "GameObject.h"
#include "../LawnApp.h"

GameObject::GameObject()
{
	mApp = gLawnApp;
	mBoard = nullptr;
	mX = 0;
	mY = 0;
	mWidth = 0;
	mHeight = 0;
	mVisible = true;
	mRow = -1;
	mRenderOrder = RenderLayer::RENDER_LAYER_TOP;
}

bool GameObject::BeginDraw(Graphics* g)
{
	if (!mVisible)
		return false;

	g->Translate(mX, mY);
	return true;
}

void GameObject::EndDraw(Graphics* g)
{
	g->Translate(-mX, -mY);
}

void GameObject::MakeParentGraphicsFrame(Graphics* g)
{
	g->Translate(-mX, -mY);
}
