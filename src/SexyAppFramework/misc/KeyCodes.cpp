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

#include "KeyCodes.h"

#include <stdio.h>
#include <string.h>

using namespace Sexy;

#define MAX_KEYNAME_LEN 12

typedef struct
{
	char mKeyName[MAX_KEYNAME_LEN];
	KeyCode mKeyCode;
} KeyNameEntry;

static constexpr KeyNameEntry aKeyCodeArray[] =
{
	{ .mKeyName = "UNKNOWN", .mKeyCode = KEYCODE_UNKNOWN },
	{ .mKeyName = "LBUTTON", .mKeyCode = KEYCODE_LBUTTON },
	{ .mKeyName = "RBUTTON", .mKeyCode = KEYCODE_RBUTTON },
	{ .mKeyName = "CANCEL", .mKeyCode = KEYCODE_CANCEL },
	{ .mKeyName = "MBUTTON", .mKeyCode = KEYCODE_MBUTTON },
	{ .mKeyName = "BACK", .mKeyCode = KEYCODE_BACK },
	{ .mKeyName = "TAB", .mKeyCode = KEYCODE_TAB },
	{ .mKeyName = "CLEAR", .mKeyCode = KEYCODE_CLEAR },
	{ .mKeyName = "RETURN", .mKeyCode = KEYCODE_RETURN },
	{ .mKeyName = "SHIFT", .mKeyCode = KEYCODE_SHIFT },
	{ .mKeyName = "CONTROL", .mKeyCode = KEYCODE_CONTROL },
	{ .mKeyName = "MENU", .mKeyCode = KEYCODE_MENU },
	{ .mKeyName = "PAUSE", .mKeyCode = KEYCODE_PAUSE },
	{ .mKeyName = "CAPITAL", .mKeyCode = KEYCODE_CAPITAL },
	{ .mKeyName = "KANA", .mKeyCode = KEYCODE_KANA },
	{ .mKeyName = "HANGEUL", .mKeyCode = KEYCODE_HANGEUL },
	{ .mKeyName = "HANGUL", .mKeyCode = KEYCODE_HANGUL },
	{ .mKeyName = "JUNJA", .mKeyCode = KEYCODE_JUNJA },
	{ .mKeyName = "FINAL", .mKeyCode = KEYCODE_FINAL },
	{ .mKeyName = "HANJA", .mKeyCode = KEYCODE_HANJA },
	{ .mKeyName = "KANJI", .mKeyCode = KEYCODE_KANJI },
	{ .mKeyName = "ESCAPE", .mKeyCode = KEYCODE_ESCAPE },
	{ .mKeyName = "CONVERT", .mKeyCode = KEYCODE_CONVERT },
	{ .mKeyName = "NONCONVERT", .mKeyCode = KEYCODE_NONCONVERT },
	{ .mKeyName = "ACCEPT", .mKeyCode = KEYCODE_ACCEPT },
	{ .mKeyName = "MODECHANGE", .mKeyCode = KEYCODE_MODECHANGE },
	{ .mKeyName = "SPACE", .mKeyCode = KEYCODE_SPACE },
	{ .mKeyName = "PRIOR", .mKeyCode = KEYCODE_PRIOR },
	{ .mKeyName = "NEXT", .mKeyCode = KEYCODE_NEXT },
	{ .mKeyName = "END", .mKeyCode = KEYCODE_END },
	{ .mKeyName = "HOME", .mKeyCode = KEYCODE_HOME },
	{ .mKeyName = "LEFT", .mKeyCode = KEYCODE_LEFT },
	{ .mKeyName = "UP", .mKeyCode = KEYCODE_UP },
	{ .mKeyName = "RIGHT", .mKeyCode = KEYCODE_RIGHT },
	{ .mKeyName = "DOWN", .mKeyCode = KEYCODE_DOWN },
	{ .mKeyName = "SELECT", .mKeyCode = KEYCODE_SELECT },
	{ .mKeyName = "PRINT", .mKeyCode = KEYCODE_PRINT },
	{ .mKeyName = "EXECUTE", .mKeyCode = KEYCODE_EXECUTE },
	{ .mKeyName = "SNAPSHOT", .mKeyCode = KEYCODE_SNAPSHOT },
	{ .mKeyName = "INSERT", .mKeyCode = KEYCODE_INSERT },
	{ .mKeyName = "DELETE", .mKeyCode = KEYCODE_DELETE },
	{ .mKeyName = "HELP", .mKeyCode = KEYCODE_HELP },
	{ .mKeyName = "LWIN", .mKeyCode = KEYCODE_LWIN },
	{ .mKeyName = "RWIN", .mKeyCode = KEYCODE_RWIN },
	{ .mKeyName = "APPS", .mKeyCode = KEYCODE_APPS },
	{ .mKeyName = "NUMPAD0", .mKeyCode = KEYCODE_NUMPAD0 },
	{ .mKeyName = "NUMPAD1", .mKeyCode = KEYCODE_NUMPAD1 },
	{ .mKeyName = "NUMPAD2", .mKeyCode = KEYCODE_NUMPAD2 },
	{ .mKeyName = "NUMPAD3", .mKeyCode = KEYCODE_NUMPAD3 },
	{ .mKeyName = "NUMPAD4", .mKeyCode = KEYCODE_NUMPAD4 },
	{ .mKeyName = "NUMPAD5", .mKeyCode = KEYCODE_NUMPAD5 },
	{ .mKeyName = "NUMPAD6", .mKeyCode = KEYCODE_NUMPAD6 },
	{ .mKeyName = "NUMPAD7", .mKeyCode = KEYCODE_NUMPAD7 },
	{ .mKeyName = "NUMPAD8", .mKeyCode = KEYCODE_NUMPAD8 },
	{ .mKeyName = "NUMPAD9", .mKeyCode = KEYCODE_NUMPAD9 },
	{ .mKeyName = "MULTIPLY", .mKeyCode = KEYCODE_MULTIPLY },
	{ .mKeyName = "ADD", .mKeyCode = KEYCODE_ADD },
	{ .mKeyName = "SEPARATOR", .mKeyCode = KEYCODE_SEPARATOR },
	{ .mKeyName = "SUBTRACT", .mKeyCode = KEYCODE_SUBTRACT },
	{ .mKeyName = "DECIMAL", .mKeyCode = KEYCODE_DECIMAL },
	{ .mKeyName = "DIVIDE", .mKeyCode = KEYCODE_DIVIDE },
	{ .mKeyName = "F1", .mKeyCode = KEYCODE_F1 },
	{ .mKeyName = "F2", .mKeyCode = KEYCODE_F2 },
	{ .mKeyName = "F3", .mKeyCode = KEYCODE_F3 },
	{ .mKeyName = "F4", .mKeyCode = KEYCODE_F4 },
	{ .mKeyName = "F5", .mKeyCode = KEYCODE_F5 },
	{ .mKeyName = "F6", .mKeyCode = KEYCODE_F6 },
	{ .mKeyName = "F7", .mKeyCode = KEYCODE_F7 },
	{ .mKeyName = "F8", .mKeyCode = KEYCODE_F8 },
	{ .mKeyName = "F9", .mKeyCode = KEYCODE_F9 },
	{ .mKeyName = "F10", .mKeyCode = KEYCODE_F10 },
	{ .mKeyName = "F11", .mKeyCode = KEYCODE_F11 },
	{ .mKeyName = "F12", .mKeyCode = KEYCODE_F12 },
	{ .mKeyName = "F13", .mKeyCode = KEYCODE_F13 },
	{ .mKeyName = "F14", .mKeyCode = KEYCODE_F14 },
	{ .mKeyName = "F15", .mKeyCode = KEYCODE_F15 },
	{ .mKeyName = "F16", .mKeyCode = KEYCODE_F16 },
	{ .mKeyName = "F17", .mKeyCode = KEYCODE_F17 },
	{ .mKeyName = "F18", .mKeyCode = KEYCODE_F18 },
	{ .mKeyName = "F19", .mKeyCode = KEYCODE_F19 },
	{ .mKeyName = "F20", .mKeyCode = KEYCODE_F20 },
	{ .mKeyName = "F21", .mKeyCode = KEYCODE_F21 },
	{ .mKeyName = "F22", .mKeyCode = KEYCODE_F22 },
	{ .mKeyName = "F23", .mKeyCode = KEYCODE_F23 },
	{ .mKeyName = "F24", .mKeyCode = KEYCODE_F24 },
	{ .mKeyName = "NUMLOCK", .mKeyCode = KEYCODE_NUMLOCK },
	{ .mKeyName = "SCROLL", .mKeyCode = KEYCODE_SCROLL }
};

KeyCode	Sexy::GetKeyCodeFromName(const std::string& theKeyName)
{
	char aKeyName[MAX_KEYNAME_LEN];

	if (theKeyName.length() >= MAX_KEYNAME_LEN-1)
		return KEYCODE_UNKNOWN;

	strcpy(aKeyName, theKeyName.c_str());
	//strupr(aKeyName);
	char *s = aKeyName;
	while (*s)
	{
		*s = toupper(static_cast<unsigned char>(*s));
		s++;
	}

	if (theKeyName.length() == 1)
	{
		unsigned char aKeyNameChar = aKeyName[0];

		if ((aKeyNameChar >= (unsigned char) KEYCODE_ASCIIBEGIN) && (aKeyNameChar <= (unsigned char) KEYCODE_ASCIIEND))
			return (KeyCode) aKeyNameChar;

		if ((aKeyNameChar >= ((unsigned char) KEYCODE_ASCIIBEGIN2) - 0x80) && (aKeyNameChar <= ((unsigned char) KEYCODE_ASCIIEND2) - 0x80))
			return (KeyCode) (aKeyNameChar + 0x80);
	}	

	for (size_t i = 0; i < sizeof(aKeyCodeArray)/sizeof(aKeyCodeArray[0]); i++)	
		if (strcmp(aKeyName, aKeyCodeArray[i].mKeyName) == 0)
			return aKeyCodeArray[i].mKeyCode;	

	return KEYCODE_UNKNOWN;
}

const std::string Sexy::GetKeyNameFromCode(const KeyCode& theKeyCode)
{
	if ((theKeyCode >= KEYCODE_ASCIIBEGIN) && (theKeyCode <= KEYCODE_ASCIIEND))
	{
		char aStr[2] = {(char) theKeyCode, 0};
		return aStr;
	}

	if ((theKeyCode >= KEYCODE_ASCIIBEGIN2) && (theKeyCode <= KEYCODE_ASCIIEND2))
	{
		char aStr[2] = {(char)(((unsigned char) theKeyCode) - 0x80), 0};
		return aStr;
	}

	for (size_t i = 0; i < sizeof(aKeyCodeArray)/sizeof(aKeyCodeArray[0]); i++)	
		if (theKeyCode == aKeyCodeArray[i].mKeyCode)
			return aKeyCodeArray[i].mKeyName;	

	return "UNKNOWN";
}
