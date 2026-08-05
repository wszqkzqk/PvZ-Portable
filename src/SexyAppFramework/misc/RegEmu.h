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

// simple Windows Registry emulator

#ifndef __REGEMU_H__
#define __REGEMU_H__

#include <string>
#include <cstdint>

namespace regemu
{
	enum
	{
		REGEMU_NONE,
		REGEMU_SZ,
		REGEMU_EXPAND_SZ,
		REGEMU_BINARY,
		REGEMU_DWORD,
		REGEMU_DWORD_LITTLE_ENDIAN=4,
		REGEMU_DWORD_BIG_ENDIAN,
		REGEMU_MULTI_SZ=7,
		REGEMU_QWORD=11,
		REGEMU_QWORD_LITTLE_ENDIAN=11
	};

	void SetRegFile(const std::string& fileName);
	bool RegistryRead(const std::string& keyName, const std::string& valueName, uint32_t* type, uint8_t* value, uint32_t* length);
	bool RegistryWrite(const std::string& keyName, const std::string& valueName, uint32_t type, const uint8_t* value, uint32_t length);
	bool RegistryEraseKey(const std::string& keyName);
	bool RegistryEraseValue(const std::string& keyName, const std::string& valueName);
}

#endif
