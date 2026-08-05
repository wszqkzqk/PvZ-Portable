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

#ifndef __KEYCODES_INCLUDED__
#define __KEYCODES_INCLUDED__

#include "Common.h"

namespace Sexy
{

// Don't need typedef
/*typedef*/ enum KeyCode
{
	KEYCODE_UNKNOWN		  = 0x00,
	KEYCODE_LBUTTON		  = 0x01,
	KEYCODE_RBUTTON       = 0x02,
	KEYCODE_CANCEL        = 0x03,
	KEYCODE_MBUTTON       = 0x04,
	KEYCODE_BACK          = 0x08,
	KEYCODE_TAB           = 0x09,
	KEYCODE_CLEAR         = 0x0C,
	KEYCODE_RETURN        = 0x0D,
	KEYCODE_SHIFT         = 0x10,
	KEYCODE_CONTROL       = 0x11,
	KEYCODE_MENU          = 0x12,
	KEYCODE_PAUSE         = 0x13,
	KEYCODE_CAPITAL       = 0x14,
	KEYCODE_KANA          = 0x15,
	KEYCODE_HANGEUL       = 0x15,
	KEYCODE_HANGUL        = 0x15,
	KEYCODE_JUNJA         = 0x17,
	KEYCODE_FINAL         = 0x18,
	KEYCODE_HANJA         = 0x19,
	KEYCODE_KANJI         = 0x19,
	KEYCODE_ESCAPE        = 0x1B,
	KEYCODE_CONVERT       = 0x1C,
	KEYCODE_NONCONVERT    = 0x1D,
	KEYCODE_ACCEPT        = 0x1E,
	KEYCODE_MODECHANGE    = 0x1F,
	KEYCODE_SPACE         = 0x20,
	KEYCODE_PRIOR         = 0x21,
	KEYCODE_NEXT          = 0x22,
	KEYCODE_END           = 0x23,
	KEYCODE_HOME          = 0x24,
	KEYCODE_LEFT          = 0x25,
	KEYCODE_UP            = 0x26,
	KEYCODE_RIGHT         = 0x27,
	KEYCODE_DOWN          = 0x28,
	KEYCODE_SELECT        = 0x29,
	KEYCODE_PRINT         = 0x2A,
	KEYCODE_EXECUTE       = 0x2B,
	KEYCODE_SNAPSHOT      = 0x2C,
	KEYCODE_INSERT        = 0x2D,
	KEYCODE_DELETE        = 0x2E,
	KEYCODE_HELP          = 0x2F,
	KEYCODE_ASCIIBEGIN	  = 0x30,
	KEYCODE_ASCIIEND	  = 0x5A,
	KEYCODE_LWIN          = 0x5B,
	KEYCODE_RWIN          = 0x5C,
	KEYCODE_APPS          = 0x5D,
	KEYCODE_NUMPAD0       = 0x60,
	KEYCODE_NUMPAD1       = 0x61,
	KEYCODE_NUMPAD2       = 0x62,
	KEYCODE_NUMPAD3       = 0x63,
	KEYCODE_NUMPAD4       = 0x64,
	KEYCODE_NUMPAD5       = 0x65,
	KEYCODE_NUMPAD6       = 0x66,
	KEYCODE_NUMPAD7       = 0x67,
	KEYCODE_NUMPAD8       = 0x68,
	KEYCODE_NUMPAD9       = 0x69,
	KEYCODE_MULTIPLY      = 0x6A,
	KEYCODE_ADD           = 0x6B,
	KEYCODE_SEPARATOR     = 0x6C,
	KEYCODE_SUBTRACT      = 0x6D,
	KEYCODE_DECIMAL       = 0x6E,
	KEYCODE_DIVIDE        = 0x6F,
	KEYCODE_F1            = 0x70,
	KEYCODE_F2            = 0x71,
	KEYCODE_F3            = 0x72,
	KEYCODE_F4            = 0x73,
	KEYCODE_F5            = 0x74,
	KEYCODE_F6            = 0x75,
	KEYCODE_F7            = 0x76,
	KEYCODE_F8            = 0x77,
	KEYCODE_F9            = 0x78,
	KEYCODE_F10           = 0x79,
	KEYCODE_F11           = 0x7A,
	KEYCODE_F12           = 0x7B,
	KEYCODE_F13           = 0x7C,
	KEYCODE_F14           = 0x7D,
	KEYCODE_F15           = 0x7E,
	KEYCODE_F16           = 0x7F,
	KEYCODE_F17           = 0x80,
	KEYCODE_F18           = 0x81,
	KEYCODE_F19           = 0x82,
	KEYCODE_F20           = 0x83,
	KEYCODE_F21           = 0x84,
	KEYCODE_F22           = 0x85,
	KEYCODE_F23           = 0x86,
	KEYCODE_F24           = 0x87,
	KEYCODE_NUMLOCK       = 0x90,
	KEYCODE_SCROLL        = 0x91,
	KEYCODE_ASCIIBEGIN2	  = 0xB3, //ASCII + 0x80
	KEYCODE_ASCIIEND2	  = 0xE0
};

KeyCode	GetKeyCodeFromName(const std::string& theKeyName);
const std::string GetKeyNameFromCode(const KeyCode& theKeyCode);

}

#endif //__KEYCODES_INCLUDED__
