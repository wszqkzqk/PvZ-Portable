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

#ifndef __DESCPARSER_H__
#define __DESCPARSER_H__

#include "Common.h"
#include <memory>

namespace Sexy
{

class DataElement
{
public:
	bool					mIsList;

public:
	DataElement();
	virtual ~DataElement();

	virtual DataElement*	Duplicate() = 0;
};

class SingleDataElement : public DataElement
{
public:
	std::string				mString;

public:
	SingleDataElement();
	SingleDataElement(const std::string theString);
	~SingleDataElement() override;

	DataElement*			Duplicate() override;
};

typedef std::vector<std::unique_ptr<DataElement>> ElementVector;

class ListDataElement : public DataElement
{
public:
	ElementVector			mElementVector;

public:
	ListDataElement();
	ListDataElement(const ListDataElement& theListDataElement);
	~ListDataElement() override;

	ListDataElement&		operator=(const ListDataElement& theListDataElement);

	DataElement*			Duplicate() override;
};

typedef std::map<std::string, std::unique_ptr<DataElement>> DataElementMap;
typedef std::vector<double> DoubleVector;

class DescParser
{
public:
	enum
	{
		CMDSEP_SEMICOLON = 1,
		CMDSEP_NO_INDENT = 2
	};

public:
	int						mCmdSep;

	std::string				mError;
	int						mCurrentLineNum;
	std::string				mCurrentLine;
	DataElementMap			mDefineMap;

public:
	virtual bool			Error(const std::string& theError);
	virtual DataElement*	Dereference(const std::string& theString);
	bool					IsImmediate(const std::string& theString);
	std::string				Unquote(const std::string& theQuotedString);
	bool					GetValues(ListDataElement* theSource, ListDataElement* theValues);
	std::string				DataElementToString(DataElement* theDataElement);
	bool					DataToString(DataElement* theSource, std::string* theString);
	bool					DataToInt(DataElement* theSource, int* theInt);
	bool					DataToStringVector(DataElement* theSource, std::vector<std::string>* theStringVector);
	bool					DataToList(DataElement* theSource, ListDataElement* theValues);
	bool					DataToIntVector(DataElement* theSource, std::vector<int>* theIntVector);
	bool					DataToDoubleVector(DataElement* theSource, DoubleVector* theDoubleVector);
	bool					ParseToList(const std::string& theString, ListDataElement* theList, bool expectListEnd, int* theStringPos);
	bool					ParseDescriptorLine(const std::string& theDescriptorLine);

	// You must implement this one
	virtual bool			HandleCommand(const ListDataElement& theParams) = 0;

public:
	DescParser();
	virtual ~DescParser();

	bool					LoadDescriptor(const std::string& theFileName);
};

}

#endif //__DESCPARSER_H__
