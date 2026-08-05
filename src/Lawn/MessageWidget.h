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

#ifndef __MESSAGEWIDGET_H__
#define __MESSAGEWIDGET_H__

#include <cstdint>
#include "../ConstEnums.h"
#include "../SexyAppFramework/Common.h"
#include "../LawnApp.h"

#define MAX_MESSAGE_LENGTH 128
#define MAX_REANIM_LINES 5

class LawnApp;
namespace Sexy
{
	class _Font;
	class Graphics;
}

class MessageWidget
{
public:
	LawnApp*			mApp;
	char				mLabel[MAX_MESSAGE_LENGTH];
	int32_t				mDisplayTime;
	int32_t				mDuration;
	MessageStyle		mMessageStyle;
	ReanimationID		mTextReanimID[MAX_MESSAGE_LENGTH];
	int32_t				mTextReanimByteOffset[MAX_MESSAGE_LENGTH];
	int32_t				mTextReanimCount;
	ReanimationType		mReanimType;
	int32_t				mSlideOffTime;
	char				mLabelNext[MAX_MESSAGE_LENGTH];
	MessageStyle		mMessageStyleNext;

public:
	MessageWidget(LawnApp* theApp);
	~MessageWidget() { ClearReanim(); }

	void		SetLabel(std::string_view theNewLabel, MessageStyle theMessageStyle);
	void				Update();
	void				Draw(Sexy::Graphics* g);
	void				ClearReanim();
	void		ClearLabel();
	inline bool			IsBeingDisplayed() { return mDuration != 0; }
	_Font*	GetFont();
	void				DrawReanimatedText(Sexy::Graphics* g, Sexy::_Font* theFont, const Sexy::Color& theColor, float thePosY);
	void				LayoutReanimText();
};

#endif
