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

#include "ModVal.h"
#include "Common.h"
#include <algorithm>
#include <fstream>

struct ModStorage
{		
	bool					mChanged;
	int						mInt; 
	double					mDouble;	
	std::string				mString;
};

struct ModPointer
{
	const char *mStrPtr;
	int mLineNum;

	ModPointer() : mStrPtr(nullptr), mLineNum(0) {}
	ModPointer(const char *theStrPtr, int theLineNum) : mStrPtr(theStrPtr), mLineNum(theLineNum) {}
};

typedef std::map<int,ModPointer> ModStorageMap; // stores counters

struct FileMod
{
	bool mHasMods;
	ModStorageMap mMap;

	FileMod(bool hasMods = false) { mHasMods = hasMods; }
};

typedef std::map<std::string, int> StringToIntMap;
typedef std::map<std::string, FileMod> FileModMap;

static StringToIntMap gStringToIntMap;
time_t gLastFileTime = 0;
const char *gSampleString = nullptr; // for finding the others

static FileModMap& GetFileModMap()
{
	static FileModMap aMap;
	return aMap;
}

static const char* FindFileInStringTable(const std::string &theSearch, const char *theMem, uint32_t theLength, const char *theStartPos)
{
	const char *aFind = nullptr;
	try
	{
		aFind = std::search(theStartPos,theMem+theLength,theSearch.c_str(),theSearch.c_str()+theSearch.length());
		if (aFind>=theMem+theLength)
			return nullptr;
		else
			return aFind;
	}
	catch(...)
	{
		return nullptr;
	}

	return nullptr;
}

static bool ParseModValString(std::string &theStr, int *theCounter = nullptr, int *theLineNum = nullptr)
{
	size_t aPos = theStr.length()-1;
	bool foundComma = false;
	while (aPos>0)
	{
		if (!foundComma && theStr[aPos]==',')
		{
			aPos--;
			foundComma = true;
		}
		else if (isdigit(static_cast<unsigned char>(theStr[aPos])))
			aPos--;
		else
			break;
	}

	if (aPos==theStr.length()-1 || aPos==0) // no number,number to erase... or empty file
		return false;

	aPos++;
	int aCounterVal = -1, aLineNum = -1;
	if (sscanf(theStr.c_str()+aPos,"%d,%d",&aCounterVal,&aLineNum)!=2) // couldn't parse out the numbers
		return false;

	theStr.resize(aPos);
	if (theCounter) *theCounter = aCounterVal;
	if (theLineNum) *theLineNum = aLineNum;
	return true;
}

static bool FindModValsInMemoryHelper(const char *theMem, uint32_t theLength)
{
	std::string aSearchStr = "SEXYMODVAL";

	FileModMap &aMap = GetFileModMap();

	bool foundOne = false;
	const char *aPtr = theMem;
	while (true)
	{
		aPtr = FindFileInStringTable(aSearchStr,theMem,theLength,aPtr);
		if (aPtr==nullptr)
			break;

		int aCounter, aLineNum;
		std::string aFileName = aPtr+10; // skip SEXYMODVAL
		if (ParseModValString(aFileName,&aCounter,&aLineNum))
		{
			FileMod &aFileMod = aMap[aFileName];
			aFileMod.mMap[aCounter] = ModPointer(aPtr-5,aLineNum);
			foundOne = true;
		}
		aPtr++;
	}

	return foundOne;
}

static void FindModValsInMemory()
{
	// Not implemented
}

static ModStorage* CreateFileModsHelper(const char* theFileName)
{
	ModStorage *aModStorage = new ModStorage;
	aModStorage->mChanged = false;
	(void)theFileName;
	return aModStorage;
}


static ModStorage* CreateFileMods(const char* theFileName)
{	
	if (gSampleString==nullptr)
		gSampleString = theFileName;

	std::string aFileName = theFileName+15; // skip SEXY_SEXYMODVAL
	ParseModValString(aFileName);

	FileModMap &aMap = GetFileModMap();
	aMap[aFileName].mHasMods = true; 

	return CreateFileModsHelper(theFileName);
}

int Sexy::ModVal(const char* theFileName, int theInt)
{
	if (*theFileName != 0)
		CreateFileMods(theFileName);	

	ModStorage* aModStorage = *(ModStorage**)(theFileName+1);
	if (aModStorage->mChanged)
		return aModStorage->mInt;
	else
		return theInt;
}

double Sexy::ModVal(const char* theFileName, double theDouble)
{
	if (*theFileName != 0)
		CreateFileMods(theFileName);	
			
	ModStorage* aModStorage = *(ModStorage**)(theFileName+1);
	if (aModStorage->mChanged)
		return aModStorage->mDouble;
	else
		return theDouble;
}

float Sexy::ModVal(const char* theFileName, float theFloat)
{
	return (float) ModVal(theFileName, (double) theFloat);
}

const char*	Sexy::ModVal(const char* theFileName, const char *theStr)
{
	if (*theFileName != 0)
		CreateFileMods(theFileName);	

	ModStorage* aModStorage = *(ModStorage**)(theFileName+1);
	if (aModStorage->mChanged)
		return aModStorage->mString.c_str();
	else
		return theStr;
}


void Sexy::AddModValEnum(const std::string &theEnumName, int theVal)
{
	gStringToIntMap[theEnumName] = theVal;
}

static bool ModStringToInteger(const char* theString, int* theIntVal)
{
	*theIntVal = 0;

	int theRadix = 10;
	bool isNeg = false;

	unsigned i = 0;

	if (isalpha((unsigned char)theString[i]) || theString[i]=='_') // enum
	{
		
		std::string aStr;
		while (isalnum((unsigned char)theString[i]) || theString[i]=='_')
		{
			aStr += theString[i];
			i++;
		}

		StringToIntMap::iterator anItr = gStringToIntMap.find(aStr);
		if (anItr!=gStringToIntMap.end())
		{
			*theIntVal = anItr->second;
			return true;
		}

		i = 0;
	}
		
	if (theString[i] == '-')
	{
		isNeg = true;
		i++;
	}

	for (;;)
	{
		char aChar = theString[i];
		
		if ((theRadix == 10) && (aChar >= '0') && (aChar <= '9'))
			*theIntVal = (*theIntVal * 10) + (aChar - '0');
		else if ((theRadix == 0x10) && 
			(((aChar >= '0') && (aChar <= '9')) || 
			 ((aChar >= 'A') && (aChar <= 'F')) || 
			 ((aChar >= 'a') && (aChar <= 'f'))))
		{			
			if (aChar <= '9')
				*theIntVal = (*theIntVal * 0x10) + (aChar - '0');
			else if (aChar <= 'F')
				*theIntVal = (*theIntVal * 0x10) + (aChar - 'A') + 0x0A;
			else
				*theIntVal = (*theIntVal * 0x10) + (aChar - 'a') + 0x0A;
		}
		else if (((aChar == 'x') || (aChar == 'X')) && (i == 1) && (*theIntVal == 0))
		{
			theRadix = 0x10;
		}
		else if (aChar == ')')
		{
			if (isNeg)
				*theIntVal = -*theIntVal;
			return true;
		}
		else
		{
			*theIntVal = 0;
			return false;
		}

		i++;
	}		
}

static bool ModStringToDouble(const char* theString, double* theDoubleVal)
{
	*theDoubleVal = 0.0;

	bool isNeg = false;

	unsigned i = 0;
	if (theString[i] == '-')
	{
		isNeg = true;
		i++;
	}

	for (;;)
	{
		char aChar = theString[i];

		if ((aChar >= '0') && (aChar <= '9'))
			*theDoubleVal = (*theDoubleVal * 10) + (aChar - '0');
		else if (aChar == '.')
		{
			i++;
			break;
		}		
		else if ((aChar == ')') || ((aChar == 'f') && (theString[i+1] == ')'))) // At end
		{
			if (isNeg)
				*theDoubleVal = -*theDoubleVal;
			return true;
		}
		else
		{
			*theDoubleVal = 0.0;
			return false;
		}

		i++;
	}

	double aMult = 0.1;
	for (;;)
	{
		char aChar = theString[i];

		if ((aChar >= '0') && (aChar <= '9'))
		{
			*theDoubleVal += (aChar - '0') * aMult;	
			aMult /= 10.0;
		}
		else if ((aChar == ')') || ((aChar == 'f') && (theString[i+1] == ')'))) // At end
		{
			if (isNeg)
				*theDoubleVal = -*theDoubleVal;
			return true;
		}
		else
		{
			*theDoubleVal = 0.0;
			return false;
		}

		i++;
	}
}

static bool ModStringToString(const char* theString, std::string &theStrVal)
{
	if (theString[0]!='"')
		return false;

	std::string &aStr = theStrVal;
	aStr.erase();

	int i=1;
	while (true)
	{
		if (theString[i]=='\\')
		{
			i++;
			switch (theString[i++])
			{
			case 'n': aStr += '\n'; break;
			case 't': aStr += '\t'; break;
			case '\\': aStr += '\\'; break;
			case '"': aStr += '\"'; break;
			default: return false;
			}
		}
		else if (theString[i]=='"')
		{
			i++;
			while (isspace((unsigned char)theString[i]))
				i++;

			if (theString[i]!='"') // continued string
				return true;
			else
				break;
		}
		else if (theString[i]=='\0')
			return false;
		else
			aStr += theString[i++];
	}

	return true;
}

bool Sexy::ReparseModValues()
{
	return false;
}
