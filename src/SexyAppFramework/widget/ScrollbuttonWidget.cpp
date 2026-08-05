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

#include "ScrollbuttonWidget.h"
#include "ButtonListener.h"
#include "graphics/Graphics.h"

using namespace Sexy;

ScrollbuttonWidget::ScrollbuttonWidget(int theId, ButtonListener *theButtonListener, int theType) : ButtonWidget(theId, theButtonListener)
{
	mHorizontal = false;
	mType = theType;
}

ScrollbuttonWidget::~ScrollbuttonWidget() = default;

void ScrollbuttonWidget::Draw(Graphics *g)
{
	int anArrowOffset = 0;

	g->SetColor(Color(212, 212, 212));
	g->FillRect(0, 0, mWidth, mHeight);

	if (mIsDown && mIsOver && !mDisabled)
	{
		anArrowOffset = 1;
		g->SetColor(Color(132, 132, 132));
		g->DrawRect(0, 0, mWidth-1, mHeight-1);
	}
	else
	{
		g->SetColor(Color(255, 255, 255));
		g->FillRect(1, 1, mWidth-2, 1);
		g->FillRect(1, 1, 1, mHeight-2);

		g->SetColor(Color::Black);
		g->FillRect(0, mHeight - 1, mWidth, 1);
		g->FillRect(mWidth - 1, 0, 1, mHeight);

		g->SetColor(Color(132, 132, 132));
		g->FillRect(1, mHeight - 2, mWidth - 2, 1);
		g->FillRect(mWidth - 2, 1, 1, mHeight - 2);
	}

	if (!mDisabled)
		g->SetColor(Color::Black);
	else
		g->SetColor(Color(132, 132, 132));

	if (mHorizontal || (mType==3 || mType==4))
	{
		for (int i = 0; i < 4; i++)
		{
			if (mId == 0 || mType==3)
				g->FillRect(i + (mWidth-4)/2 + anArrowOffset, mHeight/2 - i - 1 + anArrowOffset, 1, 1 + i*2);
			else
				g->FillRect((3 - i) + (mWidth-4)/2 + anArrowOffset, mHeight/2 - i - 1 + anArrowOffset, 1, 1 + i*2);
		}
	}
	else
	{
		for (int i = 0; i < 4; i++)
		{
			if (mId == 0 || mType==1)
				g->FillRect(mWidth/2 - i - 1 + anArrowOffset, i + (mHeight-4)/2 + anArrowOffset, 1 + i*2, 1);
			else
				g->FillRect(mWidth/2 - i - 1 + anArrowOffset, (3 - i) + (mHeight-4)/2 + anArrowOffset, 1 + i*2, 1);
		}
	}
}
