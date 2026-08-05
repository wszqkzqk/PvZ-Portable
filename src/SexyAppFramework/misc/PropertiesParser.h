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

#ifndef __SEXYAPPFRAMEWORK_NO_PROPERTIES_PARSER__
#ifndef __PROPERTIESPARSER_H__
#define __PROPERTIESPARSER_H__

#include "SexyAppBase.h"

namespace Sexy
{

class XMLParser;

class PropertiesParser
{
public:
	SexyAppBase*			mApp;
	XMLParser*				mXMLParser;
	std::string				mError;
	bool					mHasFailed;

protected:
	void					Fail(const std::string& theErrorText);

	bool					ParseSingleElement(std::string* theString);
	bool					ParseStringArray(std::vector<std::string>* theStringVector);
	bool					ParseProperties();
	bool					DoParseProperties();

public:
	PropertiesParser(SexyAppBase* theApp);
	virtual ~PropertiesParser();

	bool					ParsePropertiesFile(const std::string& theFilename);
	bool					ParsePropertiesBuffer(const Buffer& theBuffer);
	std::string				GetErrorText();
};

}

#endif //__PROPERTIESPARSER_H__

#endif // __SEXYAPPFRAMEWORK_NO_PROPERTIES_PARSER__
