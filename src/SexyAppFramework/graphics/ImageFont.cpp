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

#include "ImageFont.h"
#include "Graphics.h"
#include "Image.h"
#include "SexyAppBase.h"
#include "MemoryImage.h"
#include "graphics/GLImage.h"
#include <algorithm>
#include <mutex>
#include "fcaseopen/fcaseopen.h"

using namespace Sexy;

DataElement::DataElement() :
	mIsList(false)
{
}

DataElement::~DataElement() = default;

SingleDataElement::SingleDataElement()
{
	mIsList = false;
}

SingleDataElement::SingleDataElement(const std::string theString) :
	mString(theString)
{
	mIsList = false;
}

SingleDataElement::~SingleDataElement() = default;

DataElement* SingleDataElement::Duplicate()
{
	SingleDataElement* aSingleDataElement = new SingleDataElement(*this);
	return aSingleDataElement;
}

ListDataElement::ListDataElement()
{
	mIsList = true;
}

ListDataElement::~ListDataElement()
{
	for (uint32_t i = 0; i < mElementVector.size(); i++)
		delete mElementVector[i];
}

ListDataElement::ListDataElement(const ListDataElement& theListDataElement)
{
	mIsList = true;
	for (uint32_t i = 0; i < theListDataElement.mElementVector.size(); i++)
		mElementVector.push_back(theListDataElement.mElementVector[i]->Duplicate());
}

ListDataElement& ListDataElement::operator=(const ListDataElement& theListDataElement)
{
	uint32_t i;

	for (i = 0; i < mElementVector.size(); i++)
		delete mElementVector[i];
	mElementVector.clear();

	for (i = 0; i < theListDataElement.mElementVector.size(); i++)
		mElementVector.push_back(theListDataElement.mElementVector[i]->Duplicate());

	return *this;
}

DataElement* ListDataElement::Duplicate()
{
	ListDataElement* aListDataElement = new ListDataElement(*this);
	return aListDataElement;
}

CharData::CharData()
{
	mWidth = 0;
	mOrder = 0;
}

FontLayer::FontLayer(FontData* theFontData)
{
	mFontData = theFontData;
	mDrawMode = -1;
	mSpacing = 0;
	mPointSize = 0;
	mAscent = 0;
	mAscentPadding = 0;
	mMinPointSize = -1;
	mMaxPointSize = -1;
	mHeight = 0;
	mDefaultHeight = 0;
	mColorMult = Color::White;
	mColorAdd = Color(0, 0, 0, 0);
	mLineSpacingOffset = 0;
	mBaseOrder = 0;
}

FontLayer::FontLayer(const FontLayer& theFontLayer) :
	mFontData(theFontLayer.mFontData),
	mRequiredTags(theFontLayer.mRequiredTags),
	mExcludedTags(theFontLayer.mExcludedTags),
	mCharDataMap(theFontLayer.mCharDataMap),
	mColorMult(theFontLayer.mColorMult),
	mColorAdd(theFontLayer.mColorAdd),
	mImage(theFontLayer.mImage),
	mDrawMode(theFontLayer.mDrawMode),
	mOffset(theFontLayer.mOffset),
	mSpacing(theFontLayer.mSpacing),
	mMinPointSize(theFontLayer.mMinPointSize),
	mMaxPointSize(theFontLayer.mMaxPointSize),
	mPointSize(theFontLayer.mPointSize),
	mAscent(theFontLayer.mAscent),
	mAscentPadding(theFontLayer.mAscentPadding),
	mHeight(theFontLayer.mHeight),
	mDefaultHeight(theFontLayer.mDefaultHeight),
	mLineSpacingOffset(theFontLayer.mLineSpacingOffset),
	mBaseOrder(theFontLayer.mBaseOrder),
	// @Minerscale mUseAlphaCorrection wasn't initialised in the copy constructor causing UB
	mUseAlphaCorrection(theFontLayer.mUseAlphaCorrection)
{
	for (auto anItr = theFontLayer.mCharDataMap.begin(); anItr != theFontLayer.mCharDataMap.end(); anItr++)
	{
		mCharDataMap.insert(CharDataMap::value_type(anItr->first, anItr->second));
	}
}

CharData* FontLayer::GetCharData(char32_t theChar)
{
	auto anItr = mCharDataMap.find(theChar);
	if (anItr == mCharDataMap.end())
	{
		anItr = mCharDataMap.insert(CharDataMap::value_type(theChar, CharData())).first;
	}

	return &anItr->second;
}

FontData::FontData()
{
	mInitialized = false;

	mApp = nullptr;
	mRefCount = 0;
	mDefaultPointSize = 0;
}

FontData::~FontData()
{
	DataElementMap::iterator anItr = mDefineMap.begin();
	while (anItr != mDefineMap.end())
	{
		std::string aDefineName = anItr->first;
		DataElement* aDataElement = anItr->second;

		delete aDataElement;
		anItr++;
	}
}

void FontData::Ref()
{
	mRefCount++;
}

void FontData::DeRef()
{
	if (--mRefCount == 0)
	{
		delete this;
	}
}

bool FontData::Error(const std::string& theError)
{
	if (mApp != nullptr)
	{
		std::string anErrorString = mFontErrorHeader + theError;

		if (mCurrentLine.length() > 0)
		{
			anErrorString += " on Line " + StrFormat("%d:\r\n\r\n", mCurrentLineNum) + mCurrentLine;
		}

		mApp->Popup(anErrorString);
	}

	return false;
}

bool FontData::DataToLayer(DataElement* theSource, FontLayer** theFontLayer)
{
	*theFontLayer = nullptr;

	if (theSource->mIsList)
		return false;

	std::string aLayerName = StringToUpper(((SingleDataElement*)theSource)->mString);

	FontLayerMap::iterator anItr = mFontLayerMap.find(aLayerName);
	if (anItr == mFontLayerMap.end())
	{
		Error("Undefined Layer");
		return false;
	}

	*theFontLayer = anItr->second;

	return true;
}

bool FontData::GetColorFromDataElement(DataElement* theElement, Color& theColor)
{
	if (theElement->mIsList)
	{
		DoubleVector aFactorVector;
		if (!DataToDoubleVector(theElement, &aFactorVector) && (aFactorVector.size() == 4))
			return false;

		theColor = Color(
			(int)(aFactorVector[0] * 255),
			(int)(aFactorVector[1] * 255),
			(int)(aFactorVector[2] * 255),
			(int)(aFactorVector[3] * 255));

		return true;
	}

	int aColor = 0;
	if (!StringToInt(((SingleDataElement*)theElement)->mString, &aColor))
		return false;

	theColor = aColor;
	return true;
}


static char32_t UTF8CharToUTF32Char(const std::string &theString)
{
	char32_t aChar;
	size_t aOffset = 0;
	if (UTF8DecodeNext(theString, aOffset, aChar) && aOffset == theString.size())
		return aChar;

	return 0; // Invalid character
}

static bool UTF8PairToUTF32Pair(const std::string& theString, char32_t& firstChar, char32_t& secondChar)
{
	size_t aOffset = 0;
	if (!UTF8DecodeNext(theString, aOffset, firstChar))
		return false;
	if (!UTF8DecodeNext(theString, aOffset, secondChar))
		return false;
	return aOffset == theString.size();
}

bool FontData::HandleCommand(const ListDataElement& theParams)
{
	std::string aCmd = ((SingleDataElement*)theParams.mElementVector[0])->mString;

	bool invalidNumParams = false;
	bool invalidParamFormat = false;
	bool literalError = false;
	bool sizeMismatch = false;

	if (strcasecmp(aCmd.c_str(), "Define") == 0)
	{
		if (theParams.mElementVector.size() == 3)
		{
			if (!theParams.mElementVector[1]->mIsList)
			{
				std::string aDefineName = StringToUpper(((SingleDataElement*)theParams.mElementVector[1])->mString);

				if (!IsImmediate(aDefineName))
				{
					DataElementMap::iterator anItr = mDefineMap.find(aDefineName);
					if (anItr != mDefineMap.end())
					{
						delete anItr->second;
						mDefineMap.erase(anItr);
					}

					if (theParams.mElementVector[2]->mIsList)
					{
						ListDataElement* aValues = new ListDataElement();
						if (!GetValues(((ListDataElement*)theParams.mElementVector[2]), aValues))
						{
							delete aValues;
							return false;
						}

						mDefineMap.insert(DataElementMap::value_type(aDefineName, aValues));
					}
					else
					{
						SingleDataElement* aDefParam = (SingleDataElement*)theParams.mElementVector[2];

						DataElement* aDerefVal = Dereference(aDefParam->mString);

						if (aDerefVal)
							mDefineMap.insert(DataElementMap::value_type(aDefineName, aDerefVal->Duplicate()));
						else
							mDefineMap.insert(DataElementMap::value_type(aDefineName, aDefParam->Duplicate()));
					}
				}
				else
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "CreateHorzSpanRectList") == 0)
	{
		if (theParams.mElementVector.size() == 4)
		{
			std::vector<int> aRectIntVector;
			std::vector<int> aWidthsVector;

			if ((!theParams.mElementVector[1]->mIsList) &&
				(DataToIntVector(theParams.mElementVector[2], &aRectIntVector)) &&
				(aRectIntVector.size() == 4) &&
				(DataToIntVector(theParams.mElementVector[3], &aWidthsVector)))
			{
				std::string aDefineName = StringToUpper(((SingleDataElement*)theParams.mElementVector[1])->mString);

				int aXPos = 0;

				ListDataElement* aRectList = new ListDataElement();

				for (uint32_t aWidthNum = 0; aWidthNum < aWidthsVector.size(); aWidthNum++)
				{
					ListDataElement* aRectElement = new ListDataElement();
					aRectList->mElementVector.push_back(aRectElement);

					char aStr[256];

					snprintf(aStr, sizeof(aStr), "%d", aRectIntVector[0] + aXPos);
					aRectElement->mElementVector.push_back(new SingleDataElement(aStr));

					snprintf(aStr, sizeof(aStr), "%d", aRectIntVector[1]);
					aRectElement->mElementVector.push_back(new SingleDataElement(aStr));

					snprintf(aStr, sizeof(aStr), "%d", aWidthsVector[aWidthNum]);
					aRectElement->mElementVector.push_back(new SingleDataElement(aStr));

					snprintf(aStr, sizeof(aStr), "%d", aRectIntVector[3]);
					aRectElement->mElementVector.push_back(new SingleDataElement(aStr));

					aXPos += aWidthsVector[aWidthNum];
				}

				DataElementMap::iterator anItr = mDefineMap.find(aDefineName);
				if (anItr != mDefineMap.end())
				{
					delete anItr->second;
					mDefineMap.erase(anItr);
				}

				mDefineMap.insert(DataElementMap::value_type(aDefineName, aRectList));
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "SetDefaultPointSize") == 0)
	{
		if (theParams.mElementVector.size() == 2)
		{
			int aPointSize;

			if ((!theParams.mElementVector[1]->mIsList) &&
				(StringToInt(((SingleDataElement*)theParams.mElementVector[1])->mString, &aPointSize)))
			{
				mDefaultPointSize = aPointSize;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "SetCharMap") == 0)
	{
		if (theParams.mElementVector.size() == 3)
		{
			std::vector<std::string> aFromVector;
			std::vector<std::string> aToVector;

			if ((DataToStringVector(theParams.mElementVector[1], &aFromVector)) &&
				(DataToStringVector(theParams.mElementVector[2], &aToVector)))
			{
				if (aFromVector.size() == aToVector.size())
				{
					for (uint32_t aMapIdx = 0; aMapIdx < aFromVector.size(); aMapIdx++)
					{
						if ((aFromVector[aMapIdx].length() == 1) && (aToVector[aMapIdx].length() == 1))
						{
							mCharMap.insert(CharMap::value_type(aFromVector[aMapIdx][0], aToVector[aMapIdx][0]));
						}
						else
							invalidParamFormat = true;
					}
				}
				else
					sizeMismatch = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "CreateLayer") == 0)
	{
		if (theParams.mElementVector.size() == 2)
		{
			if (!theParams.mElementVector[1]->mIsList)
			{
				std::string aLayerName = StringToUpper(((SingleDataElement*)theParams.mElementVector[1])->mString);

				mFontLayerList.push_back(FontLayer(this));
				FontLayer* aFontLayer = &mFontLayerList.back();

				if (!mFontLayerMap.insert(FontLayerMap::value_type(aLayerName, aFontLayer)).second)
				{
					Error("Layer Already Exists");
				}
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "CreateLayerFrom") == 0)
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aSourceLayer;

			if ((!theParams.mElementVector[1]->mIsList) && (DataToLayer(theParams.mElementVector[2], &aSourceLayer)))
			{
				std::string aLayerName = StringToUpper(((SingleDataElement*)theParams.mElementVector[1])->mString);

				mFontLayerList.push_back(FontLayer(*aSourceLayer));
				FontLayer* aFontLayer = &mFontLayerList.back();

				if (!mFontLayerMap.insert(FontLayerMap::value_type(aLayerName, aFontLayer)).second)
				{
					Error("Layer Already Exists");
				}
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "LayerRequireTags") == 0)
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			std::vector<std::string> aStringVector;

			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(DataToStringVector(theParams.mElementVector[2], &aStringVector)))
			{
				for (uint32_t i = 0; i < aStringVector.size(); i++)
					aLayer->mRequiredTags.push_back(StringToUpper(aStringVector[i]));
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "LayerExcludeTags") == 0)
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			std::vector<std::string> aStringVector;

			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(DataToStringVector(theParams.mElementVector[2], &aStringVector)))
			{
				for (uint32_t i = 0; i < aStringVector.size(); i++)
					aLayer->mExcludedTags.push_back(StringToUpper(aStringVector[i]));
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "LayerPointRange") == 0)
	{
		if (theParams.mElementVector.size() == 4)
		{
			FontLayer* aLayer;
			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(!theParams.mElementVector[2]->mIsList) &&
				(!theParams.mElementVector[3]->mIsList))
			{
				int aMinPointSize;
				int aMaxPointSize;

				if ((StringToInt(((SingleDataElement*)theParams.mElementVector[2])->mString, &aMinPointSize)) &&
					(StringToInt(((SingleDataElement*)theParams.mElementVector[3])->mString, &aMaxPointSize)))
				{
					aLayer->mMinPointSize = aMinPointSize;
					aLayer->mMaxPointSize = aMaxPointSize;
				}
				else
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "LayerSetPointSize") == 0)
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(!theParams.mElementVector[2]->mIsList))
			{
				int aPointSize;
				if (StringToInt(((SingleDataElement*)theParams.mElementVector[2])->mString, &aPointSize))
				{
					aLayer->mPointSize = aPointSize;
				}
				else
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "LayerSetHeight") == 0)
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(!theParams.mElementVector[2]->mIsList))
			{
				int aHeight;
				if (StringToInt(((SingleDataElement*)theParams.mElementVector[2])->mString, &aHeight))
				{
					aLayer->mHeight = aHeight;
				}
				else
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "LayerSetImage") == 0)
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			std::string aFileNameString;

			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(DataToString(theParams.mElementVector[2], &aFileNameString)))
			{
				std::string aFileName = GetPathFrom(aFileNameString, GetFileDir(mSourceFile));

				bool isNew;
				SharedImageRef anImage = mApp->GetSharedImage(aFileName, "", &isNew);

				if ((Image*)anImage != nullptr)
				{
					if (isNew)
						anImage->Palletize();
					aLayer->mImage = anImage;
				}
				else
				{
					Error("Failed to Load Image");
					return false;
				}
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "LayerSetDrawMode") == 0)
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) && (!theParams.mElementVector[2]->mIsList))
			{
				int anDrawMode;
				if ((StringToInt(((SingleDataElement*)theParams.mElementVector[2])->mString, &anDrawMode)) &&
					(anDrawMode >= 0) && (anDrawMode <= 1))
				{
					aLayer->mDrawMode = anDrawMode;
				}
				else
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "LayerSetColorMult") == 0)
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			if (DataToLayer(theParams.mElementVector[1], &aLayer))
			{
				if (!GetColorFromDataElement(theParams.mElementVector[2], aLayer->mColorMult))
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "LayerSetColorAdd") == 0)
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			if (DataToLayer(theParams.mElementVector[1], &aLayer))
			{
				if (!GetColorFromDataElement(theParams.mElementVector[2], aLayer->mColorAdd))
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "LayerSetAscent") == 0)
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(!theParams.mElementVector[2]->mIsList))
			{
				int anAscent;
				if (StringToInt(((SingleDataElement*)theParams.mElementVector[2])->mString, &anAscent))
				{
					aLayer->mAscent = anAscent;
				}
				else
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "LayerSetAscentPadding") == 0)
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(!theParams.mElementVector[2]->mIsList))
			{
				int anAscent;
				if (StringToInt(((SingleDataElement*)theParams.mElementVector[2])->mString, &anAscent))
				{
					aLayer->mAscentPadding = anAscent;
				}
				else
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "LayerSetLineSpacingOffset") == 0)
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(!theParams.mElementVector[2]->mIsList))
			{
				int anAscent;
				if (StringToInt(((SingleDataElement*)theParams.mElementVector[2])->mString, &anAscent))
				{
					aLayer->mLineSpacingOffset = anAscent;
				}
				else
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "LayerSetOffset") == 0)
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			std::vector<int> anOffset;

			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) && (DataToIntVector(theParams.mElementVector[2], &anOffset)) && (anOffset.size() == 2))
			{
				aLayer->mOffset.mX = anOffset[0];
				aLayer->mOffset.mY = anOffset[1];
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "LayerSetCharWidths") == 0)
	{
		if (theParams.mElementVector.size() == 4)
		{
			FontLayer* aLayer;
			std::vector<std::string> aCharsVector;
			std::vector<int> aCharWidthsVector;

			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(DataToStringVector(theParams.mElementVector[2], &aCharsVector)) &&
				(DataToIntVector(theParams.mElementVector[3], &aCharWidthsVector)))
			{
				if (aCharsVector.size() == aCharWidthsVector.size())
				{
					for (uint32_t i = 0; i < aCharsVector.size(); i++)
					{
						char32_t first_char = UTF8CharToUTF32Char(aCharsVector[i]);
						aLayer->GetCharData(first_char)->mWidth = aCharWidthsVector[i];
					}
				}
				else
					sizeMismatch = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "LayerSetSpacing") == 0)
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			std::vector<int> anOffset;

			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(!theParams.mElementVector[2]->mIsList))
			{
				int aSpacing;

				if (StringToInt(((SingleDataElement*)theParams.mElementVector[2])->mString, &aSpacing))
				{
					aLayer->mSpacing = aSpacing;
				}
				else
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "LayerSetImageMap") == 0)
	{
		if (theParams.mElementVector.size() == 4)
		{
			FontLayer* aLayer;
			std::vector<std::string> aCharsVector;
			ListDataElement aRectList;

			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(DataToStringVector(theParams.mElementVector[2], &aCharsVector)) &&
				(DataToList(theParams.mElementVector[3], &aRectList)))
			{
				if (aCharsVector.size() == aRectList.mElementVector.size())
				{
					if ((Image*)aLayer->mImage != nullptr)
					{
						int anImageWidth = aLayer->mImage->GetWidth();
						int anImageHeight = aLayer->mImage->GetHeight();

						for (uint32_t i = 0; i < aCharsVector.size(); i++)
						{
							std::vector<int> aRectElement;
							//std::wstring aWString = UTF8StringToWString(aCharsVector[i]);
							char32_t first_char = UTF8CharToUTF32Char(aCharsVector[i]);

							if ((DataToIntVector(aRectList.mElementVector[i], &aRectElement)) &&
								(aRectElement.size() == 4))

							{
								Rect aRect = Rect(aRectElement[0]/IMG_DOWNSCALE, aRectElement[1]/IMG_DOWNSCALE, aRectElement[2]/IMG_DOWNSCALE, aRectElement[3]/IMG_DOWNSCALE);

								if ((aRect.mX < 0) || (aRect.mY < 0) ||
									(aRect.mX + aRect.mWidth > anImageWidth) || (aRect.mY + aRect.mHeight > anImageHeight))
								{
									Error("Image rectangle out of bounds " + std::to_string(aRect.mX) + " " + std::to_string(aRect.mY) + " " + std::to_string(aRect.mWidth) + " " + std::to_string(aRect.mHeight) + " - " + std::to_string(anImageWidth) + " " + std::to_string(anImageHeight));
									return false;
								}

								aLayer->GetCharData(first_char)->mImageRect = aRect;
							}
							else
								invalidParamFormat = true;
						}

						aLayer->mDefaultHeight = 0;
						for (auto anItr = aLayer->mCharDataMap.begin(); anItr != aLayer->mCharDataMap.end(); anItr++)
						{
							CharData& aCharData = anItr->second;
							aLayer->mDefaultHeight = std::max(aLayer->mDefaultHeight, aCharData.mImageRect.mHeight + aCharData.mOffset.mY);
						}
					}
					else
					{
						Error("Layer image not set");
						return false;
					}
				}
				else
					sizeMismatch = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "LayerSetCharOffsets") == 0)
	{
		if (theParams.mElementVector.size() == 4)
		{
			FontLayer* aLayer;
			std::vector<std::string> aCharsVector = std::vector<std::string>();
			ListDataElement aRectList = ListDataElement();

			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(DataToStringVector(theParams.mElementVector[2], &aCharsVector)) &&
				(DataToList(theParams.mElementVector[3], &aRectList)))
			{
				if (aCharsVector.size() == aRectList.mElementVector.size())
				{
					for (uint32_t i = 0; i < aCharsVector.size(); i++)
					{
						std::vector<int> aRectElement = std::vector<int>();
						char32_t first_char = UTF8CharToUTF32Char(aCharsVector[i]);

						if ((DataToIntVector(aRectList.mElementVector[i], &aRectElement)) &&
							(aRectElement.size() == 2))
						{
							aLayer->GetCharData(first_char)->mOffset = Point(aRectElement[0], aRectElement[1]);
						}
						else {
							invalidParamFormat = true;
				 		}
					}
				}
				else
					sizeMismatch = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "LayerSetKerningPairs") == 0)
	{
		if (theParams.mElementVector.size() == 4)
		{
			FontLayer* aLayer;
			std::vector<std::string> aPairsVector;
			std::vector<int> anOffsetsVector;

			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(DataToStringVector(theParams.mElementVector[2], &aPairsVector)) &&
				(DataToIntVector(theParams.mElementVector[3], &anOffsetsVector)))
			{
				if (aPairsVector.size() == anOffsetsVector.size())
				{
					for (uint32_t i = 0; i < aPairsVector.size(); i++)
					{
						char32_t firstChar = 0;
						char32_t secondChar = 0;
						if (UTF8PairToUTF32Pair(aPairsVector[i], firstChar, secondChar))
						{
							aLayer->GetCharData(firstChar)->mKerningOffsets[secondChar] = anOffsetsVector[i];
						}
						else
							invalidParamFormat = true;
					}
				}
				else
					sizeMismatch = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "LayerSetBaseOrder") == 0)
	{
		if (theParams.mElementVector.size() == 3)
		{
			FontLayer* aLayer;
			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(!theParams.mElementVector[2]->mIsList))
			{
				int aBaseOrder;
				if (StringToInt(((SingleDataElement*)theParams.mElementVector[2])->mString, &aBaseOrder))
				{
					aLayer->mBaseOrder = aBaseOrder;
				}
				else
					invalidParamFormat = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "LayerSetCharOrders") == 0)
	{
		if (theParams.mElementVector.size() == 4)
		{
			FontLayer* aLayer;
			std::vector<std::string> aCharsVector;
			std::vector<int> aCharOrdersVector;

			if ((DataToLayer(theParams.mElementVector[1], &aLayer)) &&
				(DataToStringVector(theParams.mElementVector[2], &aCharsVector)) &&
				(DataToIntVector(theParams.mElementVector[3], &aCharOrdersVector)))
			{
				if (aCharsVector.size() == aCharOrdersVector.size())
				{
					for (uint32_t i = 0; i < aCharsVector.size(); i++)
					{
						if (aCharsVector[i].length() == 1)
						{
							//aLayer->mCharData[(uchar) aCharsVector[i][0]].mOrder = aCharOrdersVector[i];
							aLayer->GetCharData(aCharsVector[i][0])->mOrder = aCharOrdersVector[i];
						}
						else
							invalidParamFormat = true;
					}
				}
				else
					sizeMismatch = true;
			}
			else
				invalidParamFormat = true;
		}
		else
			invalidNumParams = true;
	}
	else if (strcasecmp(aCmd.c_str(), "LayerSetExInfo") == 0) {}
	else
	{
		Error("Unknown Command");
		return false;
	}

	if (invalidNumParams)
	{
		Error("Invalid Number of Parameters");
		return false;
	}

	if (invalidParamFormat)
	{
		Error("Invalid Paramater Type");
		return false;
	}

	if (literalError)
	{
		Error("Undefined Value");
		return false;
	}

	if (sizeMismatch)
	{
		Error("List Size Mismatch");
		return false;
	}

	return true;
}

bool FontData::Load(SexyAppBase* theSexyApp, const std::string& theFontDescFileName)
{
	if (mInitialized)
		return false;

	bool hasErrors = false;

	mApp = theSexyApp;
	mCurrentLine = "";

	mFontErrorHeader = "Font Descriptor Error in " + theFontDescFileName + "\r\n";

	mSourceFile = theFontDescFileName;

	mInitialized = LoadDescriptor(theFontDescFileName); ;

	return !hasErrors;
}

bool FontData::LoadLegacy(Image* theFontImage, const std::string& theFontDescFileName)
{
	if (mInitialized)
		return false;

	mFontLayerList.push_back(FontLayer(this));
	FontLayer* aFontLayer = &mFontLayerList.back();

	FontLayerMap::iterator anItr = mFontLayerMap.insert(FontLayerMap::value_type("", aFontLayer)).first;
	if (anItr == mFontLayerMap.end())
		return false;

	aFontLayer->mImage = (MemoryImage*)theFontImage;
	aFontLayer->mDefaultHeight = aFontLayer->mImage->GetHeight();
	aFontLayer->mAscent = aFontLayer->mImage->GetHeight();

	int aCharPos = 0;
	
	// Use GetResourcePath to handle resource folder prefix
	std::string aFilePath = GetResourcePath(theFontDescFileName);
	FILE* aStream = fcaseopen(aFilePath.c_str(), "r");

	if (aStream == nullptr)
		return false;

	mSourceFile = theFontDescFileName;

	if (fscanf(aStream, "%d%d", &aFontLayer->GetCharData(' ')->mWidth, &aFontLayer->mAscent) != 2)
	{
		fclose(aStream);
		return false;
	}

	while (!feof(aStream))
	{
		char aBuf[2] = { 0, 0 }; // needed because fscanf will null terminate the string it reads
		char aChar = 0;
		int aWidth = 0;

		if (fscanf(aStream, "%1s%d", aBuf, &aWidth) != 2)
			break;
		aChar = aBuf[0];


		if (aChar == 0)
			break;

		aFontLayer->GetCharData(aChar)->mImageRect = Rect(aCharPos, 0, aWidth, aFontLayer->mImage->GetHeight());
		aFontLayer->GetCharData(aChar)->mWidth = aWidth;

		aCharPos += aWidth;
	}

	char c;

	for (c = 'A'; c <= 'Z'; c++)
		if ((aFontLayer->mCharDataMap[c].mWidth == 0) && (aFontLayer->mCharDataMap[c - 'A' + 'a'].mWidth != 0))
			mCharMap.insert(CharMap::value_type(c, c - 'A' + 'a'));

	for (c = 'a'; c <= 'z'; c++)
		if ((aFontLayer->mCharDataMap[c].mWidth == 0) && (aFontLayer->mCharDataMap[c - 'a' + 'A'].mWidth != 0))
			mCharMap.insert(CharMap::value_type(c, c - 'a' + 'A'));

	mInitialized = true;
	fclose(aStream);

	return true;
}

ActiveFontLayer::ActiveFontLayer()
{
	mScaledImage = nullptr;
	mOwnsImage = false;
}

ActiveFontLayer::ActiveFontLayer(const ActiveFontLayer& theActiveFontLayer) :
	mBaseFontLayer(theActiveFontLayer.mBaseFontLayer),
	mScaledImage(theActiveFontLayer.mScaledImage),
	mOwnsImage(theActiveFontLayer.mOwnsImage)
{
	if (mOwnsImage)
		mScaledImage = mBaseFontLayer->mFontData->mApp->CopyImage(mScaledImage);

	for (auto anItr = theActiveFontLayer.mScaledCharImageRects.begin(); anItr != theActiveFontLayer.mScaledCharImageRects.end(); anItr++)
	{
		mScaledCharImageRects.insert(CharRectMap::value_type(anItr->first, anItr->second));
	}
}

ActiveFontLayer::~ActiveFontLayer()
{
	if (mOwnsImage)
		delete mScaledImage;
}

ImageFont::ImageFont(SexyAppBase* theSexyApp, const std::string& theFontDescFileName)
{
	mScale = 1.0;
	mFontData = new FontData();
	mFontData->Ref();
	mFontData->Load(theSexyApp, theFontDescFileName);
	mPointSize = mFontData->mDefaultPointSize;
	GenerateActiveFontLayers();
	mActiveListValid = true;
	mForceScaledImagesWhite = false;
}

ImageFont::ImageFont(Image* theFontImage)
{
	mScale = 1.0;
	mFontData = new FontData();
	mFontData->Ref();
	mFontData->mInitialized = true;
	mPointSize = mFontData->mDefaultPointSize;
	mActiveListValid = false;
	mForceScaledImagesWhite = false;

	mFontData->mFontLayerList.push_back(FontLayer(mFontData));
	FontLayer* aFontLayer = &mFontData->mFontLayerList.back();

	// Weird stray .first
	(void)mFontData->mFontLayerMap.insert(FontLayerMap::value_type("", aFontLayer)).first;

	aFontLayer->mImage = (MemoryImage*)theFontImage;
	aFontLayer->mDefaultHeight = aFontLayer->mImage->GetHeight();
	aFontLayer->mAscent = aFontLayer->mImage->GetHeight();
}

ImageFont::ImageFont(const ImageFont& theImageFont) :
	_Font(theImageFont),
	mFontData(theImageFont.mFontData),
	mPointSize(theImageFont.mPointSize),
	mTagVector(theImageFont.mTagVector),
	mActiveListValid(theImageFont.mActiveListValid),
	mActiveLayerList(theImageFont.mActiveLayerList),
	mScale(theImageFont.mScale),
	mForceScaledImagesWhite(theImageFont.mForceScaledImagesWhite)
{
	mFontData->Ref();
}

ImageFont::ImageFont(Image* theFontImage, const std::string& theFontDescFileName)
{

	mScale = 1.0;
	mFontData = new FontData();
	mFontData->Ref();
	mFontData->LoadLegacy(theFontImage, theFontDescFileName);
	mPointSize = mFontData->mDefaultPointSize;
	GenerateActiveFontLayers();
	mActiveListValid = true;
}

ImageFont::~ImageFont()
{
	mFontData->DeRef();
}

void ImageFont::GenerateActiveFontLayers()
{
	if (!mFontData->mInitialized)
		return;

	mActiveLayerList.clear();

	uint32_t i;

	mAscent = 0;
	mAscentPadding = 0;
	mHeight = 0;
	mLineSpacingOffset = 0;

	FontLayerList::iterator anItr = mFontData->mFontLayerList.begin();

	bool firstLayer = true;

	while (anItr != mFontData->mFontLayerList.end())
	{
		FontLayer* aFontLayer = &*anItr;

		if ((mPointSize >= aFontLayer->mMinPointSize) &&
			((mPointSize <= aFontLayer->mMaxPointSize) || (aFontLayer->mMaxPointSize == -1)))
		{
			bool active = true;

			// Make sure all required tags are included
			for (i = 0; i < aFontLayer->mRequiredTags.size(); i++)
				if (std::find(mTagVector.begin(), mTagVector.end(), aFontLayer->mRequiredTags[i]) == mTagVector.end())
					active = false;

			// Make sure no excluded tags are included
			for (i = 0; i < mTagVector.size(); i++)
				if (std::find(aFontLayer->mExcludedTags.begin(), aFontLayer->mExcludedTags.end(),
					mTagVector[i]) != aFontLayer->mExcludedTags.end())
					active = false;

			if (active)
			{
				mActiveLayerList.push_back(ActiveFontLayer());

				ActiveFontLayer* anActiveFontLayer = &mActiveLayerList.back();

				anActiveFontLayer->mBaseFontLayer = aFontLayer;

				double aLayerPointSize = 1;
				double aPointSize = mScale;

				if ((mScale == 1.0) && ((aFontLayer->mPointSize == 0) || (mPointSize == aFontLayer->mPointSize)))
				{
					anActiveFontLayer->mScaledImage = aFontLayer->mImage;
					anActiveFontLayer->mOwnsImage = false;

					for (auto anItr = aFontLayer->mCharDataMap.begin(); anItr != aFontLayer->mCharDataMap.end(); anItr++)
					{
						anActiveFontLayer->mScaledCharImageRects.insert(CharRectMap::value_type(anItr->first, anItr->second.mImageRect));
					}
				}
				else
				{
					if (aFontLayer->mPointSize != 0)
					{
						aLayerPointSize = aFontLayer->mPointSize;
						aPointSize = mPointSize * mScale;
					}

					// Resize font elements
					MemoryImage* aMemoryImage = new MemoryImage(mFontData->mApp);

					int aCurX = 0;
					int aMaxHeight = 0;

					for (auto anItr = aFontLayer->mCharDataMap.begin(); anItr != aFontLayer->mCharDataMap.end(); anItr++)
					{
						Rect* anOrigRect = &anItr->second.mImageRect;

						Rect aScaledRect(aCurX, 0,
							(int)((anOrigRect->mWidth * aPointSize) / aLayerPointSize),
							(int)((anOrigRect->mHeight * aPointSize) / aLayerPointSize));

						anActiveFontLayer->mScaledCharImageRects[anItr->first] = aScaledRect;

						aMaxHeight = std::max(aMaxHeight, aScaledRect.mHeight);

						aCurX += aScaledRect.mWidth;
					}

					anActiveFontLayer->mScaledImage = aMemoryImage;
					anActiveFontLayer->mOwnsImage = true;

					// Create the image now
					aMemoryImage->Create(aCurX, aMaxHeight);

					Graphics g(aMemoryImage);

					for (auto anItr = aFontLayer->mCharDataMap.begin(); anItr != aFontLayer->mCharDataMap.end(); anItr++)
					{
						if ((Image*)aFontLayer->mImage != nullptr)
						{
							g.DrawImage(aFontLayer->mImage, anActiveFontLayer->mScaledCharImageRects[anItr->first], anItr->second.mImageRect);
						}
					}


					if (mForceScaledImagesWhite)
					{
						int aCount = aMemoryImage->mWidth * aMemoryImage->mHeight;
						uint32_t* aBits = aMemoryImage->GetBits();

						for (int i = 0; i < aCount; i++) {
							*(aBits) = *aBits | 0x00FFFFFF;
							aBits++;
						}
					}

					aMemoryImage->Palletize();
				}

				int aLayerAscent = (aFontLayer->mAscent * aPointSize) / aLayerPointSize;
				mAscent = std::max(mAscent, aLayerAscent);

				if (aFontLayer->mHeight != 0)
				{
					int aLayerHeight = (aFontLayer->mHeight * aPointSize) / aLayerPointSize;
					mHeight = std::max(mHeight, aLayerHeight);
				}
				else
				{
					int aLayerHeight = (aFontLayer->mDefaultHeight * aPointSize) / aLayerPointSize;
					mHeight = std::max(mHeight, aLayerHeight);
				}

				int anAscentPadding = (aFontLayer->mAscentPadding * aPointSize) / aLayerPointSize;
				if ((firstLayer) || (anAscentPadding < mAscentPadding))
					mAscentPadding = anAscentPadding;

				int aLineSpacingOffset = (aFontLayer->mLineSpacingOffset * aPointSize) / aLayerPointSize;
				if ((firstLayer) || (aLineSpacingOffset > mLineSpacingOffset))
					mLineSpacingOffset = aLineSpacingOffset;

				firstLayer = false;
			}
		}

		anItr++;
	}
}

int ImageFont::StringWidth(const std::string& theString)
{
	int aWidth = 0;
	char32_t aPrevChar = 0;
	size_t aOffset = 0;
	char32_t aChar = 0;
	while (UTF8DecodeNext(theString, aOffset, aChar))
	{
		aWidth += CharWidthKern(aChar, aPrevChar);
		aPrevChar = aChar;
	}

	return aWidth;
}

int ImageFont::CharWidthKern(char32_t theChar, char32_t thePrevChar)
{
	Prepare();

	int aMaxXPos = 0;
	double aPointSize = mPointSize * mScale;

	theChar = GetMappedChar(theChar);
	if (thePrevChar != 0)
		thePrevChar = GetMappedChar(thePrevChar);

	ActiveFontLayerList::iterator anItr = mActiveLayerList.begin();
	while (anItr != mActiveLayerList.end())
	{
		ActiveFontLayer* anActiveFontLayer = &*anItr;

		int aLayerXPos = 0;

		int aCharWidth;
		int aSpacing;

		int aLayerPointSize = anActiveFontLayer->mBaseFontLayer->mPointSize;

		if (aLayerPointSize == 0)
		{
			aCharWidth = anActiveFontLayer->mBaseFontLayer->GetCharData(theChar)/*mCharData[(uchar) theChar].*/->mWidth * mScale;

			if (thePrevChar != 0)
			{
				aSpacing = (anActiveFontLayer->mBaseFontLayer->mSpacing +
					anActiveFontLayer->mBaseFontLayer->GetCharData(thePrevChar)->mKerningOffsets[theChar]) * mScale;
			}
			else
				aSpacing = 0;
		}
		else
		{
			aCharWidth = (anActiveFontLayer->mBaseFontLayer->GetCharData(theChar)/*mCharData[(uchar) theChar].*/->mWidth * aPointSize / aLayerPointSize);

			if (thePrevChar != 0)
			{
				aSpacing = (anActiveFontLayer->mBaseFontLayer->mSpacing +
					anActiveFontLayer->mBaseFontLayer->GetCharData(thePrevChar)->mKerningOffsets[theChar]) * aPointSize / aLayerPointSize;
			}
			else
				aSpacing = 0;
		}

		aLayerXPos += aCharWidth + aSpacing;

		aMaxXPos = std::max(aMaxXPos, aLayerXPos);

		anItr++;
	}

	return aMaxXPos;
}

int ImageFont::CharWidth(char32_t theChar)
{
	return CharWidthKern(theChar, 0);
}

std::mutex gRenderCritSec;
static const int POOL_SIZE = 4096;
static RenderCommand gRenderCommandPool[POOL_SIZE];
static RenderCommand* gRenderTail[256];
static RenderCommand* gRenderHead[256];

void ImageFont::DrawStringEx(Graphics* g, int theX, int theY, const std::string& theString, const Color& theColor, RectList* theDrawnAreas, int* theWidth)
{
	std::scoped_lock anAutoCrit(gRenderCritSec);

	int aPoolIdx;

	for (aPoolIdx = 0; aPoolIdx < 256; aPoolIdx++)
	{
		gRenderHead[aPoolIdx] = nullptr;
		gRenderTail[aPoolIdx] = nullptr;
	}

	// int aXPos = theX; // unused

	if (theDrawnAreas != nullptr)
		theDrawnAreas->clear();

	if (!mFontData->mInitialized)
	{
		if (theWidth != nullptr)
			*theWidth = 0;
		return;
	}

	Prepare();

	bool colorizeImages = g->GetColorizeImages();
	g->SetColorizeImages(true);

	int aCurXPos = theX;
	int aCurPoolIdx = 0;

	size_t aDecodeOffset = 0;
	char32_t aCurRawChar = 0;
	char32_t aNextRawChar = 0;

	bool aHasCur = UTF8DecodeNext(theString, aDecodeOffset, aCurRawChar);

	while (aHasCur)
	{
		bool aHasNext = UTF8DecodeNext(theString, aDecodeOffset, aNextRawChar);

		char32_t aChar = GetMappedChar(aCurRawChar);

		char32_t aNextChar = aHasNext ? GetMappedChar(aNextRawChar) : 0;

		int aMaxXPos = aCurXPos;

		ActiveFontLayerList::iterator anItr = mActiveLayerList.begin();
		int aLayerOrderOffset = 0; // To ensure the order of font rendering
		while (anItr != mActiveLayerList.end())
		{
			ActiveFontLayer* anActiveFontLayer = &*anItr;

			int aLayerXPos = aCurXPos;

			int anImageX;
			int anImageY;
			int aCharWidth;
			int aSpacing;

			int aLayerPointSize = anActiveFontLayer->mBaseFontLayer->mPointSize;

			double aScale = mScale;
			if (aLayerPointSize != 0)
				aScale *= (float)mPointSize / (float)aLayerPointSize;

			if (aScale == 1.0)
			{
				anImageX = aLayerXPos + anActiveFontLayer->mBaseFontLayer->mOffset.mX + anActiveFontLayer->mBaseFontLayer->GetCharData(aChar)->mOffset.mX;
				anImageY = theY - (anActiveFontLayer->mBaseFontLayer->mAscent - anActiveFontLayer->mBaseFontLayer->mOffset.mY - anActiveFontLayer->mBaseFontLayer->GetCharData(aChar)->mOffset.mY);
				aCharWidth = anActiveFontLayer->mBaseFontLayer->GetCharData(aChar)->mWidth;

				if (aNextChar != 0)
				{
					aSpacing = anActiveFontLayer->mBaseFontLayer->mSpacing +
						anActiveFontLayer->mBaseFontLayer->GetCharData(aChar)->mKerningOffsets[aNextChar];
				}
				else
					aSpacing = 0;
			}
			else
			{
				anImageX = aLayerXPos + (int)((anActiveFontLayer->mBaseFontLayer->mOffset.mX + anActiveFontLayer->mBaseFontLayer->GetCharData(aChar)->mOffset.mX) * aScale);
				anImageY = theY - (int)((anActiveFontLayer->mBaseFontLayer->mAscent - anActiveFontLayer->mBaseFontLayer->mOffset.mY - anActiveFontLayer->mBaseFontLayer->GetCharData(aChar)->mOffset.mY) * aScale);
				aCharWidth = (anActiveFontLayer->mBaseFontLayer->GetCharData(aChar)->mWidth * aScale);

				if (aNextChar != 0)
				{
					aSpacing = (int)((anActiveFontLayer->mBaseFontLayer->mSpacing +
						anActiveFontLayer->mBaseFontLayer->GetCharData(aChar)->mKerningOffsets[aNextChar]) * aScale);
				}
				else
					aSpacing = 0;
			}

			Color aColor;
			aColor.mRed = std::min((theColor.mRed * anActiveFontLayer->mBaseFontLayer->mColorMult.mRed / 255) + anActiveFontLayer->mBaseFontLayer->mColorAdd.mRed, 255);
			aColor.mGreen = std::min((theColor.mGreen * anActiveFontLayer->mBaseFontLayer->mColorMult.mGreen / 255) + anActiveFontLayer->mBaseFontLayer->mColorAdd.mGreen, 255);
			aColor.mBlue = std::min((theColor.mBlue * anActiveFontLayer->mBaseFontLayer->mColorMult.mBlue / 255) + anActiveFontLayer->mBaseFontLayer->mColorAdd.mBlue, 255);
			aColor.mAlpha = std::min((theColor.mAlpha * anActiveFontLayer->mBaseFontLayer->mColorMult.mAlpha / 255) + anActiveFontLayer->mBaseFontLayer->mColorAdd.mAlpha, 255);

			int anOrder = aLayerOrderOffset + anActiveFontLayer->mBaseFontLayer->mBaseOrder + anActiveFontLayer->mBaseFontLayer->GetCharData(aChar)->mOrder;

			if (aCurPoolIdx >= POOL_SIZE)
				break;

			RenderCommand* aRenderCommand = &gRenderCommandPool[aCurPoolIdx++];

			aRenderCommand->mImage = anActiveFontLayer->mScaledImage;
			aRenderCommand->mColor = aColor;
			aRenderCommand->mDest[0] = anImageX;
			aRenderCommand->mDest[1] = anImageY;
			aRenderCommand->mSrc[0] = anActiveFontLayer->mScaledCharImageRects[aChar].mX;
			aRenderCommand->mSrc[1] = anActiveFontLayer->mScaledCharImageRects[aChar].mY;
			aRenderCommand->mSrc[2] = anActiveFontLayer->mScaledCharImageRects[aChar].mWidth;
			aRenderCommand->mSrc[3] = anActiveFontLayer->mScaledCharImageRects[aChar].mHeight;
			aRenderCommand->mMode = anActiveFontLayer->mBaseFontLayer->mDrawMode;
			aRenderCommand->mNext = nullptr;

			int anOrderIdx = std::clamp(anOrder + 128, 0, 255);

			if (gRenderTail[anOrderIdx] == nullptr)
			{
				gRenderTail[anOrderIdx] = aRenderCommand;
				gRenderHead[anOrderIdx] = aRenderCommand;
			}
			else
			{
				gRenderTail[anOrderIdx]->mNext = aRenderCommand;
				gRenderTail[anOrderIdx] = aRenderCommand;
			}

			//aRenderCommandMap.insert(RenderCommandMap::value_type(aPriority, aRenderCommand));

			if (theDrawnAreas != nullptr)
			{
				Rect aDestRect(anImageX, anImageY, anActiveFontLayer->mScaledCharImageRects[aChar].mWidth, anActiveFontLayer->mScaledCharImageRects[aChar].mHeight);

				theDrawnAreas->push_back(aDestRect);

			}

			aLayerXPos += aCharWidth + aSpacing;

			aMaxXPos = std::max(aMaxXPos, aLayerXPos);

			anItr++;
			aLayerOrderOffset++;
		}

		aCurXPos = aMaxXPos;

		aCurRawChar = aNextRawChar;
		aHasCur = aHasNext;
	}

	if (theWidth != nullptr)
		*theWidth = aCurXPos - theX;

	Color anOrigColor = g->GetColor();

	for (aPoolIdx = 0; aPoolIdx < 256; aPoolIdx++)
	{
		RenderCommand* aRenderCommand = gRenderHead[aPoolIdx];

		while (aRenderCommand != nullptr)
		{
			int anOldDrawMode = g->GetDrawMode();
			if (aRenderCommand->mMode != -1)
				g->SetDrawMode(aRenderCommand->mMode);
			g->SetColor(Color(aRenderCommand->mColor));
			if (aRenderCommand->mImage != nullptr)
				g->DrawImage(aRenderCommand->mImage, aRenderCommand->mDest[0], aRenderCommand->mDest[1], Rect(aRenderCommand->mSrc[0], aRenderCommand->mSrc[1], aRenderCommand->mSrc[2], aRenderCommand->mSrc[3]));
			g->SetDrawMode(anOldDrawMode);

			aRenderCommand = aRenderCommand->mNext;
		}
	}

	g->SetColor(anOrigColor);

	g->SetColorizeImages(colorizeImages);
}

void ImageFont::DrawString(Graphics* g, int theX, int theY, const std::string& theString, const Color& theColor, const Rect& theClipRect)
{
	(void)theClipRect;
	DrawStringEx(g, theX, theY, theString, theColor, nullptr, nullptr);
}

_Font* ImageFont::Duplicate()
{
	return new ImageFont(*this);
}

void ImageFont::SetPointSize(int thePointSize)
{
	mPointSize = thePointSize;
	mActiveListValid = false;
}

void ImageFont::SetScale(double theScale)
{
	mScale = theScale;
	mActiveListValid = false;
}

int	ImageFont::GetPointSize()
{
	return mPointSize;
}

int	ImageFont::GetDefaultPointSize()
{
	return mFontData->mDefaultPointSize;
}

bool ImageFont::AddTag(const std::string& theTagName)
{
	if (HasTag(theTagName))
		return false;

	std::string aTagName = StringToUpper(theTagName);
	mTagVector.push_back(aTagName);
	mActiveListValid = false;
	return true;
}

bool ImageFont::RemoveTag(const std::string& theTagName)
{
	std::string aTagName = StringToUpper(theTagName);

	std::vector<std::string>::iterator anItr = std::find(mTagVector.begin(), mTagVector.end(), aTagName);
	if (anItr == mTagVector.end())
		return false;

	mTagVector.erase(anItr);
	mActiveListValid = false;
	return true;
}

bool ImageFont::HasTag(const std::string& theTagName)
{
	std::vector<std::string>::iterator anItr = std::find(mTagVector.begin(), mTagVector.end(), theTagName);
	return anItr != mTagVector.end();
}

std::string ImageFont::GetDefine(const std::string& theName)
{
	DataElement* aDataElement = mFontData->Dereference(theName);

	if (aDataElement == nullptr)
		return "";

	return mFontData->DataElementToString(aDataElement);
}

void ImageFont::Prepare()
{
	if (!mActiveListValid)
	{
		GenerateActiveFontLayers();
		mActiveListValid = true;
	}
}

char32_t ImageFont::GetMappedChar(char32_t theChar)
{
	auto anItr = mFontData->mCharMap.find(theChar);
	if (anItr != mFontData->mCharMap.end())
	{
		return anItr->second;
	}
	return theChar;
}
