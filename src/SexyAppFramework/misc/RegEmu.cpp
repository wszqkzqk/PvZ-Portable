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

#include "RegEmu.h"
#include "Common.h"

#include <map>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>

#define REGEMU_VERSION 1

struct RegValue
{
	uint32_t mType;
	uint32_t mLength;
	std::vector<uint8_t> mValue;
};
typedef std::map<std::string, std::map<std::string, RegValue> > RegContents;
static RegContents registry;
static std::string currFile;

static void SaveToFile()
{
	if (currFile.empty())
	{
		Sexy::PrintF("RegEmu: Filename not specified, can't save\n");
		return;
	}

	std::ofstream f(Sexy::PathFromU8(currFile), std::ios::binary);
	if (!f)
	{
		Sexy::PrintF("RegEmu: Couldn't open '%s' for writing\n", currFile.c_str());
		return;
	}

	f.write("REGEMU", 6);

	uint16_t aVersion = REGEMU_VERSION;
	f.write(reinterpret_cast<const char*>(&aVersion), sizeof(uint16_t));

	uint32_t aNumKeys = registry.size();
	f.write(reinterpret_cast<const char*>(&aNumKeys), sizeof(uint32_t));

	for (auto& keyPair : registry)
	{
		uint32_t aKeyNameLen = keyPair.first.size()+1;
		f.write(reinterpret_cast<const char*>(&aKeyNameLen), sizeof(uint32_t));
		f.write(keyPair.first.c_str(), aKeyNameLen);

		uint32_t aNumValues = keyPair.second.size();
		f.write(reinterpret_cast<const char*>(&aNumValues), sizeof(uint32_t));

		for (auto& valuePair : registry[keyPair.first])
		{
			uint32_t aValueNameLen = valuePair.first.size()+1;
			f.write(reinterpret_cast<const char*>(&aValueNameLen), sizeof(uint32_t));
			f.write(valuePair.first.c_str(), aValueNameLen);

			RegValue& value = valuePair.second;
			f.write(reinterpret_cast<const char*>(&value.mType), sizeof(uint32_t));
			f.write(reinterpret_cast<const char*>(&value.mLength), sizeof(uint32_t));
			f.write(reinterpret_cast<const char*>(value.mValue.data()), value.mLength);
		}
	}
}

void regemu::SetRegFile(const std::string& fileName)
{
	currFile = fileName;
	registry.clear();

	std::ifstream f(Sexy::PathFromU8(currFile), std::ios::binary);
	if (!f)
	{
		Sexy::PrintF("RegEmu: Can't read '%s': File does not exist\n", currFile.c_str());
		return;
	}

	char aHeader[6];
	if (!f.read(aHeader, 6) || strncmp(aHeader, "REGEMU", 6))
	{
		Sexy::PrintF("RegEmu: Can't read '%s': Invalid header\n", currFile.c_str());
		return;
	}

	uint16_t aVersion;
	if (!f.read(reinterpret_cast<char*>(&aVersion), sizeof(uint16_t))) { return; }

	uint32_t aNumKeys;
	if (!f.read(reinterpret_cast<char*>(&aNumKeys), sizeof(uint32_t))) { return; }

	for (uint32_t i=0; i<aNumKeys; i++)
	{
		uint32_t aKeyNameLen;
		if (!f.read(reinterpret_cast<char*>(&aKeyNameLen), sizeof(uint32_t))) { return; }

		std::vector<char> aKeyName(aKeyNameLen);
		if (!f.read(aKeyName.data(), aKeyNameLen)) { return; }

		registry[aKeyName.data()] = {};

		uint32_t aNumValues;
		if (!f.read(reinterpret_cast<char*>(&aNumValues), sizeof(uint32_t))) { return; }

		for (uint32_t j=0; j<aNumValues; j++)
		{
			RegValue value;
			uint32_t aValueNameLen;
			if (!f.read(reinterpret_cast<char*>(&aValueNameLen), sizeof(uint32_t))) { return; }

			std::vector<char> aValueName(aValueNameLen);
			if (!f.read(aValueName.data(), aValueNameLen)) { return; }

			if (!f.read(reinterpret_cast<char*>(&value.mType), sizeof(uint32_t))) { return; }
			if (!f.read(reinterpret_cast<char*>(&value.mLength), sizeof(uint32_t))) { return; }
			value.mValue.resize(value.mLength);
			if (!f.read(reinterpret_cast<char*>(value.mValue.data()), value.mLength)) { return; }

			registry[aKeyName.data()][aValueName.data()] = value;
		}
	}

	Sexy::PrintF("RegEmu: Loaded from '%s': %zu total key(s)\n", currFile.c_str(), static_cast<size_t>(registry.size()));
}

bool regemu::RegistryRead(const std::string& keyName, const std::string& valueName, uint32_t* type, uint8_t* value, uint32_t* length)
{
	if (!registry.count(keyName))
	{
		Sexy::PrintF("RegEmu: Key '%s' does not exist\n", keyName.c_str());
		return false;
	}
	if (!registry[keyName].count(valueName))
	{
		Sexy::PrintF("RegEmu: Value '%s' does not exist\n", valueName.c_str());
		return false;
	}

	*type = registry[keyName][valueName].mType;
	*length = registry[keyName][valueName].mLength;
	memcpy(value, registry[keyName][valueName].mValue.data(), registry[keyName][valueName].mLength);
	return true;
}

bool regemu::RegistryWrite(const std::string& keyName, const std::string& valueName, uint32_t type, const uint8_t* value, uint32_t length)
{
	if (!registry.count(keyName))
		registry[keyName] = {}; // create

	RegValue regvalue;
	regvalue.mType = type;
	regvalue.mLength = length;
	regvalue.mValue.resize(length);
	memcpy(regvalue.mValue.data(), value, length);

	registry[keyName][valueName] = regvalue;

	SaveToFile();

	return true;
}

bool regemu::RegistryEraseKey(const std::string& keyName)
{
	if (!registry.count(keyName))
		return false;

	registry.erase(keyName);
	Sexy::PrintF("RegEmu: Erased key '%s'\n", keyName.c_str());

	SaveToFile();

	return true;
}

bool regemu::RegistryEraseValue(const std::string& keyName, const std::string& valueName)
{
	if (!registry.count(keyName) || !registry[keyName].count(valueName))
		return false;

	registry[keyName].erase(valueName);
	Sexy::PrintF("RegEmu: Erased value '%s' from key '%s'\n", valueName.c_str(), keyName.c_str());

	SaveToFile();

	return true;
}
