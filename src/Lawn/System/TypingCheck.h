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

#ifndef __TYPINGCHECK_H__
#define __TYPINGCHECK_H__

#include <string>
#include "misc/KeyCodes.h"

class TypingCheck
{
protected:
	std::string		mPhrase;
	std::string		mRecentTyping;

public:
	TypingCheck() : mPhrase() { }
	TypingCheck(std::string_view thePhrase);

	void			SetPhrase(std::string_view thePhrase);
	void	AddKeyCode(Sexy::KeyCode theKeyCode);
	void			AddChar(char theChar);
	bool	Check();
	bool			Check(Sexy::KeyCode theKeyCode);
};

#endif
