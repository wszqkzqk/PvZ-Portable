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

#include "PropertiesParser.h"
#include "XMLParser.h"
#include <stdlib.h>
#include <format>

using namespace Sexy;

PropertiesParser::PropertiesParser(SexyAppBase* theApp)
{
	mApp = theApp;
	mHasFailed = false;
}

void PropertiesParser::Fail(const std::string& theErrorText)
{
	if (!mHasFailed)
	{
		mHasFailed = true;
		int aLineNum = mXMLParser->GetCurrentLineNum();

		mError = theErrorText;
		if (aLineNum > 0) mError += std::format(" on Line {}", aLineNum);
		if (!mXMLParser->GetFileName().empty()) mError += std::format(" in File '{}'", mXMLParser->GetFileName());
	}
}


PropertiesParser::~PropertiesParser()
{
}


bool PropertiesParser::ParseSingleElement(std::string* aString)
{
	*aString = "";

	for (;;)
	{
		XMLElement aXMLElement;
		if (!mXMLParser->NextElement(&aXMLElement))
			return false;

		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			Fail("Unexpected Section: '" + aXMLElement.mValue + "'");
			return false;
		}
		else if (aXMLElement.mType == XMLElement::TYPE_ELEMENT)
		{
			*aString = aXMLElement.mValue;
		}
		else if (aXMLElement.mType == XMLElement::TYPE_END)
		{
			return true;
		}
	}
}

bool PropertiesParser::ParseStringArray(std::vector<std::string>* theStringVector)
{
	theStringVector->clear();

	for (;;)
	{
		XMLElement aXMLElement;
		if (!mXMLParser->NextElement(&aXMLElement))
			return false;

		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			if (aXMLElement.mValue == "String")
			{
				std::string aString;

				if (!ParseSingleElement(&aString))
					return false;

				theStringVector->push_back(aString);
			}
			else
			{
				Fail("Invalid Section '" + aXMLElement.mValue + "'");
				return false;
			}
		}
		else if (aXMLElement.mType == XMLElement::TYPE_ELEMENT)
		{
			Fail("Element Not Expected '" + aXMLElement.mValue + "'");
			return false;
		}
		else if (aXMLElement.mType == XMLElement::TYPE_END)
		{
			return true;
		}
	}
}


bool PropertiesParser::ParseProperties()
{
	for (;;)
	{
		XMLElement aXMLElement;
		if (!mXMLParser->NextElement(&aXMLElement))
			return false;

		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			if (aXMLElement.mValue == "String")
			{
				std::string aDef;
				if (!ParseSingleElement(&aDef))
					return false;

				std::string anId = aXMLElement.mAttributes["id"];
				mApp->SetString(anId, aDef);
			}
			else if (aXMLElement.mValue == "StringArray")
			{
				std::vector<std::string> aDef;

				if (!ParseStringArray(&aDef))
					return false;

				std::string anId = aXMLElement.mAttributes["id"];

				mApp->mStringVectorProperties.insert(StringStringVectorMap::value_type(anId, aDef));
			}
			else if (aXMLElement.mValue == "Boolean")
			{
				std::string aVal;

				if (!ParseSingleElement(&aVal))
					return false;

				aVal = Upper(aVal);

				bool boolVal;
				if ((aVal == "1") || (aVal == "YES") || (aVal == "ON") || (aVal == "TRUE"))
					boolVal = true;
				else if ((aVal == "0") || (aVal == "NO") || (aVal == "OFF") || (aVal == "FALSE"))
					boolVal = false;
				else
				{
					Fail("Invalid Boolean Value: '" + aVal + "'");
					return false;
				}

				std::string anId = aXMLElement.mAttributes["id"];

				mApp->SetBoolean(anId, boolVal);
			}
			else if (aXMLElement.mValue == "Integer")
			{
				std::string aVal;

				if (!ParseSingleElement(&aVal))
					return false;

				int anInt;
				if (!StringToInt(aVal, &anInt))
				{
					Fail("Invalid Integer Value: '" + aVal + "'");
					return false;
				}

				std::string anId = aXMLElement.mAttributes["id"];

				mApp->SetInteger(anId, anInt);
			}
			else if (aXMLElement.mValue == "Double")
			{
				std::string aVal;

				if (!ParseSingleElement(&aVal))
					return false;

				double aDouble;
				if (!StringToDouble(aVal, &aDouble))
				{
					Fail("Invalid Double Value: '" + aVal + "'");
					return false;
				}

				std::string anId = aXMLElement.mAttributes["id"];

				mApp->SetDouble(anId, aDouble);
			}
			else
			{
				Fail("Invalid Section '" + aXMLElement.mValue + "'");
				return false;
			}
		}
		else if (aXMLElement.mType == XMLElement::TYPE_ELEMENT)
		{
			Fail("Element Not Expected '" + aXMLElement.mValue + "'");
			return false;
		}
		else if (aXMLElement.mType == XMLElement::TYPE_END)
		{
			return true;
		}
	}
}

bool PropertiesParser::DoParseProperties()
{
	if (!mXMLParser->HasFailed())
	{
		for (;;)
		{
			XMLElement aXMLElement;
			if (!mXMLParser->NextElement(&aXMLElement))
				break;

			if (aXMLElement.mType == XMLElement::TYPE_START)
			{
				if (aXMLElement.mValue == "Properties")
				{
					if (!ParseProperties())
						break;
				}
				else
				{
					Fail("Invalid Section '" + aXMLElement.mValue + "'");
					break;
				}
			}
			else if (aXMLElement.mType == XMLElement::TYPE_ELEMENT)
			{
				Fail("Element Not Expected '" + aXMLElement.mValue + "'");
				break;
			}
		}
	}

	if (mXMLParser->HasFailed())
		Fail(mXMLParser->GetErrorText());

	mXMLParser.reset();

	return !mHasFailed;
}

bool PropertiesParser::ParsePropertiesBuffer(const Buffer& theBuffer)
{
	mXMLParser = std::make_unique<XMLParser>();

	std::string aString;
	if (!theBuffer.ToUTF8String(&aString))
	{
		Fail("Failed to convert properties buffer to UTF-8 string");
		return false;
	}

	mXMLParser->SetStringSource(aString);
	return DoParseProperties();
}

bool PropertiesParser::ParsePropertiesFile(const std::string& theFilename)
{
	mXMLParser = std::make_unique<XMLParser>();
	mXMLParser->OpenFile(theFilename);
	return DoParseProperties();
}

std::string PropertiesParser::GetErrorText()
{
	return mError;
}
