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

#include "DescParser.h"
#include "SexyAppBase.h"

using namespace Sexy;

DescParser::DescParser()
{
	mCmdSep = CMDSEP_SEMICOLON;
}

DescParser::~DescParser()
{
}

bool DescParser::Error(const std::string& theError)
{
	mError = theError;
	return false;
}

DataElement* DescParser::Dereference(const std::string& theString)
{
	std::string aDefineName = StringToUpper(theString);

	DataElementMap::iterator anItr = mDefineMap.find(aDefineName);
	if (anItr != mDefineMap.end())
		return anItr->second.get();
	else
		return nullptr;
}

bool DescParser::IsImmediate(const std::string& theString)
{
	return (((theString[0] >= '0') && (theString[0] <= '9')) || (theString[0] == '-') ||
		(theString[0] == '+') || (theString[0] == '\'') || (theString[0] == '"'));
}

std::string DescParser::Unquote(const std::string& theQuotedString)
{
	if ((theQuotedString[0] == '\'') || (theQuotedString[0] == '"'))
	{
		char aQuoteChar = theQuotedString[0];
		std::string aLiteralString;
		bool lastWasQuote = false;

		for (ulong i = 0; i < theQuotedString.length(); i++)
		{
			if (theQuotedString[i] == aQuoteChar)
			{
				if (lastWasQuote)
					aLiteralString += aQuoteChar;

				lastWasQuote = true;
			}
			else
			{
				aLiteralString += theQuotedString[i];
				lastWasQuote = false;
			}
		}

		return aLiteralString;
	}
	else
		return theQuotedString;
}

bool DescParser::GetValues(ListDataElement* theSource, ListDataElement* theValues)
{
	theValues->mElementVector.clear();

	for (ulong aSourceNum = 0; aSourceNum < theSource->mElementVector.size(); aSourceNum++)
	{
		if (theSource->mElementVector[aSourceNum]->mIsList)
		{
			ListDataElement* aChildList = new ListDataElement();
			theValues->mElementVector.emplace_back(aChildList);

			if (!GetValues((ListDataElement*) theSource->mElementVector[aSourceNum].get(), aChildList))
				return false;
		}
		else
		{
			std::string aString = ((SingleDataElement*) theSource->mElementVector[aSourceNum].get())->mString;

			if (aString.length() > 0)
			{
				if ((aString[0] == '\'') || (aString[0] == '"'))
				{
					SingleDataElement* aChildData = new SingleDataElement(Unquote(aString));
					theValues->mElementVector.emplace_back(aChildData);
				}
				else if (IsImmediate(aString))
				{
					theValues->mElementVector.push_back(std::make_unique<SingleDataElement>(aString));
				}
				else
				{
					std::string aDefineName = StringToUpper(aString);

					DataElementMap::iterator anItr = mDefineMap.find(aDefineName);

					if (anItr == mDefineMap.end())
					{
						Error("Unable to Dereference \"" + aString + "\"");
						return false;
					}

					theValues->mElementVector.emplace_back(anItr->second->Duplicate());
				}
			}


		}
	}

	return true;
}

std::string DescParser::DataElementToString(DataElement* theDataElement)
{
	if (theDataElement->mIsList)
	{
		ListDataElement* aListDataElement = (ListDataElement*) theDataElement;

		std::string aString = "(";

		for (ulong i = 0; i < aListDataElement->mElementVector.size(); i++)
		{
			if (i != 0)
				aString += ", ";

			aString += DataElementToString(aListDataElement->mElementVector[i].get());
		}

		aString += ")";

		return aString;
	}
	else
	{
		SingleDataElement* aSingleDataElement = (SingleDataElement*) theDataElement;
		return aSingleDataElement->mString;
	}
}

bool DescParser::DataToString(DataElement* theSource, std::string* theString)
{
	*theString = "";

	if (theSource->mIsList)
		return false;

	std::string aDefName = ((SingleDataElement*) theSource)->mString;

	DataElement* aDataElement = Dereference(aDefName);

	if (aDataElement != nullptr)
	{
		if (aDataElement->mIsList)
			return false;

		*theString = Unquote(((SingleDataElement*) aDataElement)->mString);
	}
	else
		*theString = Unquote(aDefName);

	return true;
}

bool DescParser::DataToInt(DataElement* theSource, int* theInt)
{
	*theInt = 0;

	std::string aTempString;
	if (!DataToString(theSource, &aTempString))
		return false;

	if (!StringToInt(aTempString, theInt))
		return false;

	return true;
}

bool DescParser::DataToStringVector(DataElement* theSource, std::vector<std::string>* theStringVector)
{
	theStringVector->clear();

	ListDataElement aStaticValues = ListDataElement();
	ListDataElement* aValues;

	if (theSource->mIsList)
	{
		if (!GetValues((ListDataElement*) theSource, &aStaticValues))
			return false;

		aValues = &aStaticValues;
	}
	else
	{
		std::string aDefName = ((SingleDataElement*) theSource)->mString;

		DataElement* aDataElement = Dereference(aDefName);

		if (aDataElement == nullptr)
		{
			Error("Unable to Dereference \"" + aDefName + "\"");
			return false;
		}

		if (!aDataElement->mIsList)
			return false;

		aValues = (ListDataElement*) aDataElement;
	}

	for (ulong i = 0; i < aValues->mElementVector.size(); i++)
	{
		if (aValues->mElementVector[i]->mIsList)
		{
			theStringVector->clear();
			return false;
		}

		SingleDataElement* aSingleDataElement = (SingleDataElement*) aValues->mElementVector[i].get();

		theStringVector->push_back(aSingleDataElement->mString);
	}

	return true;
}

bool DescParser::DataToList(DataElement* theSource, ListDataElement* theValues)
{
	if (theSource->mIsList)
	{
		return GetValues((ListDataElement*) theSource, theValues);
	}

	DataElement* aDataElement = Dereference(((SingleDataElement*) theSource)->mString);

	if ((aDataElement == nullptr) || (!aDataElement->mIsList))
		return false;

	ListDataElement* aListElement = (ListDataElement*) aDataElement;

	*theValues = *aListElement;

	return true;
}

bool DescParser::DataToIntVector(DataElement* theSource, std::vector<int>* theIntVector)
{
	theIntVector->clear();

	std::vector<std::string> aStringVector = std::vector<std::string>();
	if (!DataToStringVector(theSource, &aStringVector))
		return false;

	for (ulong i = 0; i < aStringVector.size(); i++)
	{
		int aIntVal;
		if (!StringToInt(aStringVector[i], &aIntVal))
			return false;

		theIntVector->push_back(aIntVal);
	}

	return true;
}

bool DescParser::DataToDoubleVector(DataElement* theSource, DoubleVector* theDoubleVector)
{
	theDoubleVector->clear();

	std::vector<std::string> aStringVector;
	if (!DataToStringVector(theSource, &aStringVector))
		return false;

	for (ulong i = 0; i < aStringVector.size(); i++)
	{
		double aDoubleVal;
		if (!StringToDouble(aStringVector[i], &aDoubleVal))
			return false;

		theDoubleVector->push_back(aDoubleVal);
	}

	return true;
}

bool DescParser::ParseToList(const std::string& theString, ListDataElement* theList, bool expectListEnd, int* theStringPos)
{
	bool inSingleQuotes = false;
	bool inDoubleQuotes = false;
	bool escaped = false;

	SingleDataElement* aCurSingleDataElement = nullptr;

	int aStringPos = 0;

	if (theStringPos == nullptr)
		theStringPos = &aStringPos;

	while (*theStringPos < (int) theString.length())
	{
		bool addSingleChar = false;
		char aChar = theString[(*theStringPos)++];

		bool isSeperator = (aChar == ' ') || (aChar == '\t') || (aChar == '\n') || (aChar == ',');

		if (escaped)
		{
			addSingleChar = true;
			escaped = false;
		}
		else
		{
			if ((aChar == '\'') && (!inDoubleQuotes))
				inSingleQuotes = !inSingleQuotes;
			else if ((aChar == '"') && (!inSingleQuotes))
				inDoubleQuotes = !inDoubleQuotes;

			if (aChar == '\\')
			{
				escaped = true;
			}
			else if ((!inSingleQuotes) && (!inDoubleQuotes))
			{
				if (aChar == ')')
				{
					if (expectListEnd)
						return true;
					else
					{
						Error("Unexpected List End");
						return false;
					}
				}
				else if (aChar == '(')
				{
					if (aCurSingleDataElement != nullptr)
					{
						Error("Unexpected List Start");
						return false;
					}
					else
					{
						auto aChildList = std::make_unique<ListDataElement>();

						if (!ParseToList(theString, aChildList.get(), true, theStringPos))
							return false;

						theList->mElementVector.push_back(std::move(aChildList));
					}
				}
				else if (isSeperator)
				{
					if (aCurSingleDataElement != nullptr)
						aCurSingleDataElement = nullptr;
				}
				else
					addSingleChar = true;
			}
			else
				addSingleChar = true;
		}

		if (addSingleChar)
		{
			if (aCurSingleDataElement == nullptr)
			{
				aCurSingleDataElement = new SingleDataElement();
				theList->mElementVector.emplace_back(aCurSingleDataElement);
			}

			aCurSingleDataElement->mString += aChar;
		}
	}

	if (inSingleQuotes)
	{
		Error("Unterminated Single Quotes");
		return false;
	}

	if (inDoubleQuotes)
	{
		Error("Unterminated Double Quotes");
		return false;
	}

	if (expectListEnd)
	{
		Error("Unterminated List");
		return false;
	}

	return true;
}

bool DescParser::ParseDescriptorLine(const std::string& theDescriptorLine)
{
	ListDataElement aParams;
	if (!ParseToList(theDescriptorLine, &aParams, false, nullptr))
		return false;

	if (aParams.mElementVector.size() > 0)
	{
		if (aParams.mElementVector[0]->mIsList)
		{
			Error("Missing Command");
			return false;
		}

		if (!HandleCommand(aParams))
			return false;
	}

	return true;
}

bool DescParser::LoadDescriptor(const std::string& theFileName)
{
	mCurrentLineNum = 0;
	int aLineCount = 0;
	bool hasErrors = false;

	mError.clear();

	std::string aFileContent;

	if (!gSexyAppBase->ReadUTF8StringFromFile(theFileName, &aFileContent))
	{
		Error("Failed to open file");
		return false;
	}

	size_t aIndex = 0;
	char aBuffChar = 0;

	while (aIndex < aFileContent.size())
	{
		int aChar;

		bool skipLine = false;
		bool atLineStart = true;
		bool inSingleQuotes = false;
		bool inDoubleQuotes = false;
		bool escaped = false;
		bool isIndented = false;

		for (;;)
		{
			if (aBuffChar != 0)
			{
				aChar = aBuffChar;
				aBuffChar = 0;
			}
			else
			{
				if (aIndex >= aFileContent.size())
					break;
				aChar = aFileContent[aIndex++];
			}

			if (aChar != '\r')
			{
				if (aChar == '\n')
					aLineCount++;

				if (((aChar == ' ') || (aChar == '\t')) && (atLineStart))
					isIndented = true;

				if ((!atLineStart) || ((aChar != ' ') && (aChar != '\t') && (aChar != '\n')))
				{
					if (atLineStart)
					{
						if ((mCmdSep & CMDSEP_NO_INDENT) && (!isIndented) && (mCurrentLine.size() > 0))
						{
							// Start a new non-indented line
							aBuffChar = aChar;
							break;
						}

						if (aChar == '#')
							skipLine = true;

						atLineStart = false;
					}

					if (aChar == '\n')
					{
						isIndented = false;
						atLineStart = true;
					}

					if ((aChar == '\n') && (skipLine))
					{
						skipLine = false;
					}
					else if (!skipLine)
					{
						if (aChar == '\\' && (inSingleQuotes || inDoubleQuotes) && !escaped)
							escaped = true;
						else
						{
							if ((aChar == '\'') && (!inDoubleQuotes) && (!escaped))
								inSingleQuotes = !inSingleQuotes;

							if ((aChar == '"') && (!inSingleQuotes) && (!escaped))
								inDoubleQuotes = !inDoubleQuotes;

							if ((aChar == ';') && (mCmdSep & CMDSEP_SEMICOLON) && (!inSingleQuotes) && (!inDoubleQuotes))
								break;

							if(escaped) // stay escaped for when this is actually parsed
							{
								mCurrentLine += '\\';
								escaped = false;
							}

							if (mCurrentLine.size() == 0)
								mCurrentLineNum = aLineCount + 1;

							mCurrentLine += aChar;
						}
					}
				}
			}
		}

		if (mCurrentLine.length() > 0)
		{
			if (!ParseDescriptorLine(mCurrentLine))
			{
				hasErrors = true;
				break;
			}

			mCurrentLine.clear();
		}
	}

	mCurrentLine.clear();
	mCurrentLineNum = 0;

	return !hasErrors;
}
