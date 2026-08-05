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

#include "Checkbox.h"
#include "CheckboxListener.h"
#include "graphics/Graphics.h"

using namespace Sexy;

Checkbox::Checkbox(Image* theUncheckedImage, Image* theCheckedImage, int theId, CheckboxListener* theCheckboxListener) :
	mListener(theCheckboxListener),
	mId(theId),
	mChecked(false),
	mUncheckedImage(theUncheckedImage),
	mCheckedImage(theCheckedImage),
	mOutlineColor(Color::White),
	mBkgColor(Color(80, 80, 80)),
	mCheckColor(Color(255, 255, 0))
{
	mDoFinger = true;
}

void Checkbox::SetChecked(bool checked, bool tellListener)
{
	mChecked = checked;
	if (tellListener && mListener)
		mListener->CheckboxChecked(mId, mChecked);
	MarkDirty();
}

bool Checkbox::IsChecked()
{
	return mChecked;
}

void Checkbox::Draw(Graphics* g)
{
	Widget::Draw(g);

	if ((mCheckedRect.mWidth == 0) && (mCheckedImage != nullptr) && (mUncheckedImage != nullptr))
	{
		if (mChecked)
			g->DrawImage(mCheckedImage, 0, 0);
		else
			g->DrawImage(mUncheckedImage, 0, 0);
	}
	else if ((mCheckedRect.mWidth != 0) && (mUncheckedImage != nullptr))
	{
		if (mChecked)
			g->DrawImage(mUncheckedImage, 0, 0, mCheckedRect);
		else
			g->DrawImage(mUncheckedImage, 0, 0, mUncheckedRect);
	}
	else if ((mUncheckedImage == nullptr) && (mCheckedImage == nullptr))
	{
		// No image, default draw method
		g->SetColor(mOutlineColor);
		g->FillRect(0, 0, mWidth, mHeight);
		g->SetColor(mBkgColor);
		g->FillRect(1, 1, mWidth - 2, mHeight - 2);

		if (mChecked)
		{
			g->SetColor(mCheckColor);
			g->DrawLine(1, 1, mWidth - 2, mHeight - 2);
			g->DrawLine(mWidth - 1, 1, 1, mHeight - 2);
		}
	}
}

void Checkbox::MouseDown(int x, int y, int theBtnNum, int theClickCount)
{
	Widget::MouseDown(x, y, theBtnNum, theClickCount);

	mChecked = !mChecked;
	if (mListener)
		mListener->CheckboxChecked(mId, mChecked);
	MarkDirty();
}
