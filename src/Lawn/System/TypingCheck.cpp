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

#include "TypingCheck.h"
using namespace Sexy;

TypingCheck::TypingCheck(std::string_view thePhrase)
{
	SetPhrase(thePhrase);
}

void TypingCheck::SetPhrase(std::string_view thePhrase)
{
	for (size_t i = 0; i < thePhrase.size(); i++)
		AddChar(thePhrase[i]);
}

void TypingCheck::AddKeyCode(Sexy::KeyCode theKeyCode)
{
	mPhrase.append(1, static_cast<char>(theKeyCode));
}

void TypingCheck::AddChar(char theChar)
{
	theChar = static_cast<char>(tolower(static_cast<unsigned char>(theChar)));
	std::string aCharString(1, theChar);
	AddKeyCode(GetKeyCodeFromName(aCharString));
}

bool TypingCheck::Check()
{
	if (mRecentTyping.compare(mPhrase) == 0)
	{
		mRecentTyping.clear();
		return true;
	}
	return false;
}

bool TypingCheck::Check(Sexy::KeyCode theKeyCode)
{
	mRecentTyping.append(1, static_cast<char>(theKeyCode));
	size_t aLength = mPhrase.size();
	if (aLength == 0)
		return false;

	if (mRecentTyping.size() > aLength)
		mRecentTyping = mRecentTyping.substr(1, aLength);

	return Check();
}
