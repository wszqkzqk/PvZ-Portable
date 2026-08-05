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

#include "XMLParser.h"
#include "paklib/PakInterface.h"

using namespace Sexy;

static int EncodeUTF8(uint32_t code, char out[4])
{
	if (code <= 0x7F)
	{
		out[0] = static_cast<char>(code);
		return 1;
	}
	if (code <= 0x7FF)
	{
		out[0] = static_cast<char>(0xC0 | (code >> 6));
		out[1] = static_cast<char>(0x80 | (code & 0x3F));
		return 2;
	}
	if (code <= 0xFFFF)
	{
		out[0] = static_cast<char>(0xE0 | (code >> 12));
		out[1] = static_cast<char>(0x80 | ((code >> 6) & 0x3F));
		out[2] = static_cast<char>(0x80 | (code & 0x3F));
		return 3;
	}
	if (code <= 0x10FFFF)
	{
		out[0] = static_cast<char>(0xF0 | (code >> 18));
		out[1] = static_cast<char>(0x80 | ((code >> 12) & 0x3F));
		out[2] = static_cast<char>(0x80 | ((code >> 6) & 0x3F));
		out[3] = static_cast<char>(0x80 | (code & 0x3F));
		return 4;
	}
	out[0] = '?';
	return 1;
}

XMLParser::XMLParser()
{
	mFile = nullptr;
	mLineNum = 0;
	mAllowComments = false;
	mGetCharFunc = &XMLParser::GetUTF8Char;
	mForcedEncodingType = false;
}

XMLParser::~XMLParser()
{
	if (mFile != nullptr)
		p_fclose(mFile);
}

void XMLParser::SetEncodingType(XMLEncodingType theEncoding)
{
	switch (theEncoding)
	{
		case ASCII:		mGetCharFunc = &XMLParser::GetAsciiChar;	mForcedEncodingType = true; break;
		case UTF_8:		mGetCharFunc = &XMLParser::GetUTF8Char;		mForcedEncodingType = true; break;
		case UTF_16:	mGetCharFunc = &XMLParser::GetUTF16Char;	mForcedEncodingType = true; break;
		case UTF_16_LE:	mGetCharFunc = &XMLParser::GetUTF16LEChar;	mForcedEncodingType = true; break;
		case UTF_16_BE:	mGetCharFunc = &XMLParser::GetUTF16BEChar;	mForcedEncodingType = true; break;
	}
}

void XMLParser::Fail(const std::string& theErrorText)
{
	mHasFailed = true;
	mErrorText = theErrorText;
}

void XMLParser::Init()
{
	mSection = "";
	mLineNum = 1;
	mHasFailed = false;
	mErrorText = "";
	mFirstChar = true;
	mByteSwap = false;
}

bool XMLParser::AddAttribute(XMLElement* theElement, const std::string& theAttributeKey, const std::string& theAttributeValue)
{
	std::pair<XMLParamMap::iterator,bool> aRet;

	aRet = theElement->mAttributes.insert(XMLParamMap::value_type(theAttributeKey, theAttributeValue));
	if (!aRet.second)
		aRet.first->second = theAttributeValue;

	if (theAttributeKey != "/")
		theElement->mAttributeIteratorList.push_back(aRet.first);

	return aRet.second;
}

bool XMLParser::GetAsciiChar(char* theChar, bool* error)
{
	(void)error;
	char aChar = 0;
	if (p_fread(&aChar, 1, 1, mFile) != 1) return false;

	*theChar = aChar;
	return true;
}

bool XMLParser::GetUTF8Char(char* theChar, bool* error)
{
	(void)error; // EOF is not an encoding error
	unsigned char aChar = 0;
	if (p_fread(&aChar, 1, 1, mFile) != 1) return false;

	if (mFirstChar)
	{
		mFirstChar = false;
		if (aChar == 0xEF)
		{
			unsigned char b2 = 0;
			unsigned char b3 = 0;
			if (p_fread(&b2, 1, 1, mFile) == 1 && p_fread(&b3, 1, 1, mFile) == 1)
			{
				if (b2 == 0xBB && b3 == 0xBF)
					return GetUTF8Char(theChar, error);
				mBufferedText.push_back((char)b3);
				mBufferedText.push_back((char)b2);
			}
		}
	}

	*theChar = (char)aChar;
	return true;
}

bool XMLParser::GetUTF16Char(char* theChar, bool* error)
{
	*error = true;
	uint16_t aTempChar = 0;
	if (p_fread(&aTempChar, 2, 1, mFile) != 1) return false;

	if (mFirstChar)
	{
		mFirstChar = false;
		if (aTempChar == 0xFEFF)
		{
			mByteSwap = false;
			return GetUTF16Char(theChar, error);
		}
		else if (aTempChar == 0xFFFE)
		{
			mByteSwap = true;
			return GetUTF16Char(theChar, error);
		}
	}
	if (mByteSwap)
		aTempChar = (uint16_t)((aTempChar << 8) | (aTempChar >> 8));

	uint32_t codepoint = aTempChar;
	if ((aTempChar & 0xD800) == 0xD800)
	{
		uint16_t aNextChar = 0;
		if (p_fread(&aNextChar, 2, 1, mFile) != 1) return false;

		if (mByteSwap)
			aNextChar = (uint16_t)((aNextChar << 8) | (aNextChar >> 8));
		if ((aNextChar & 0xDC00) == 0xDC00)
			codepoint = (((aTempChar & ~0xD800) << 10) | (aNextChar & ~0xDC00)) + 0x10000;
		else
			return false;
	}

	char utf8[4];
	int len = EncodeUTF8(codepoint, utf8);
	for (int i = len - 1; i >= 1; --i)
		mBufferedText.push_back(utf8[i]);

	*theChar = utf8[0];
	*error = false;
	return true;
}

bool XMLParser::GetUTF16LEChar(char* theChar, bool* error)
{
	*error = true;
	uint16_t aTempChar = 0;
	if (p_fread(&aTempChar, 2, 1, mFile) != 1) return false;

	aTempChar = FromLE16(aTempChar);

	uint32_t codepoint = aTempChar;
	if ((aTempChar & 0xD800) == 0xD800)
	{
		uint16_t aNextChar = 0;
		if (p_fread(&aNextChar, 2, 1, mFile) != 1) return false;

		aNextChar = FromLE16(aNextChar);
		if ((aNextChar & 0xDC00) == 0xDC00)
			codepoint = (((aTempChar & ~0xD800) << 10) | (aNextChar & ~0xDC00)) + 0x10000;
		else
			return false;
	}

	char utf8[4];
	int len = EncodeUTF8(codepoint, utf8);
	for (int i = len - 1; i >= 1; --i)
		mBufferedText.push_back(utf8[i]);

	*theChar = utf8[0];
	*error = false;
	return true;
}

bool XMLParser::GetUTF16BEChar(char* theChar, bool* error)
{
	*error = true;
	uint16_t aTempChar = 0;
	if (p_fread(&aTempChar, 2, 1, mFile) != 1) return false;

	aTempChar = FromBE16(aTempChar);

	uint32_t codepoint = aTempChar;
	if ((aTempChar & 0xD800) == 0xD800)
	{
		uint16_t aNextChar = 0;
		if (p_fread(&aNextChar, 2, 1, mFile) != 1) return false;

		aNextChar = FromBE16(aNextChar);
		if ((aNextChar & 0xDC00) == 0xDC00)
			codepoint = (((aTempChar & ~0xD800) << 10) | (aNextChar & ~0xDC00)) + 0x10000;
		else
			return false;
	}

	char utf8[4];
	int len = EncodeUTF8(codepoint, utf8);
	for (int i = len - 1; i >= 1; --i)
		mBufferedText.push_back(utf8[i]);

	*theChar = utf8[0];
	*error = false;
	return true;
}

bool XMLParser::OpenFile(const std::string& theFileName)
{
	mFile = p_fopen(theFileName.c_str(), "r");

	if (mFile == nullptr)
	{
		mLineNum = 0;
		Fail("Unable to open file " + theFileName);
		return false;
	}
	else if (!mForcedEncodingType)
	{
		p_fseek(mFile, 0, SEEK_END);
		long aFileLen = p_ftell(mFile);
		p_fseek(mFile, 0, SEEK_SET);

		mGetCharFunc = &XMLParser::GetAsciiChar;
		if (aFileLen >= 2) // UTF-16?
		{
			int aChar1 = p_fgetc(mFile);
			int aChar2 = p_fgetc(mFile);

			if ( (aChar1 == 0xFF && aChar2 == 0xFE) || (aChar1 == 0xFE && aChar2 == 0xFF) )
				mGetCharFunc = &XMLParser::GetUTF16Char;

			p_ungetc(aChar2, mFile);
			p_ungetc(aChar1, mFile);
		}
		if (mGetCharFunc == &XMLParser::GetAsciiChar)
		{
			if (aFileLen >= 3) // UTF-8?
			{
				int aChar1 = p_fgetc(mFile);
				int aChar2 = p_fgetc(mFile);
				int aChar3 = p_fgetc(mFile);

				if (aChar1 == 0xEF && aChar2 == 0xBB && aChar3 == 0xBF)
					mGetCharFunc = &XMLParser::GetUTF8Char;

				p_ungetc(aChar3, mFile);
				p_ungetc(aChar2, mFile);
				p_ungetc(aChar1, mFile);
			}
		}
	}

	mFileName = theFileName.c_str();
	Init();
	return true;
}

void XMLParser::SetStringSource(std::string_view theString)
{
	Init();

	size_t offset = 0;
	if (theString.size() >= 3 &&
		(unsigned char)theString[0] == 0xEF &&
		(unsigned char)theString[1] == 0xBB &&
		(unsigned char)theString[2] == 0xBF)
	{
		offset = 3;
	}

	int aSize = (int)theString.size() - (int)offset;
	mBufferedText.resize(aSize);
	for (int i = 0; i < aSize; i++)
		mBufferedText[i] = theString[theString.size() - 1 - i];
}

bool XMLParser::NextElement(XMLElement* theElement)
{
	for (;;)
	{
		theElement->mType = XMLElement::TYPE_NONE;
		theElement->mSection = mSection;
		theElement->mValue = "";
		theElement->mAttributes.clear();
		theElement->mInstruction.erase();

		bool hasSpace = false;
		bool inQuote = false;
		bool gotEndQuote = false;

		bool doingAttribute = false;
		bool AttributeVal = false;
		std::string aAttributeKey;
		std::string aAttributeValue;

		std::string aLastAttributeKey;

		for (;;)
		{
			// Process character by character

			char c;
			int aVal;

			if (mBufferedText.size() > 0)
			{
				c = mBufferedText[mBufferedText.size()-1];
				mBufferedText.pop_back();

				aVal = 1;
			}
			else
			{
				if (mFile != nullptr)
				{
					bool error = false;
					if ((this->*mGetCharFunc)(&c, &error))
					{
						aVal = 1;
					}
					else
					{
						if (error) Fail("Illegal Character");
						aVal = 0;
					}
				}
				else
				{
					aVal = 0;
				}
			}

			if (aVal == 1)
			{
				bool processChar = false;

				if (c == '\n')
				{
					mLineNum++;
				}

				if (theElement->mType == XMLElement::TYPE_COMMENT)
				{
					// Just add text to theElement->mInstruction until we find -->

					std::string* aStrPtr = &theElement->mInstruction;

					*aStrPtr += c;

					int aLen = aStrPtr->length();

					if ((c == '>') && (aLen >= 3) && ((*aStrPtr)[aLen - 2] == '-') && ((*aStrPtr)[aLen - 3] == '-'))
					{
						*aStrPtr = aStrPtr->substr(0, aLen - 3);
						break;
					}
				}
				else if (theElement->mType == XMLElement::TYPE_INSTRUCTION)
				{
					// Just add text to theElement->mInstruction until we find ?>

					std::string* aStrPtr = &theElement->mValue;

					if ((theElement->mInstruction.length() != 0) || (::isspace((unsigned char)c)))
						aStrPtr = &theElement->mInstruction;

					*aStrPtr += c;

					int aLen = aStrPtr->length();

					if ((c == '>') && (aLen >= 2) && ((*aStrPtr)[aLen - 2] == '?'))
					{
						*aStrPtr = aStrPtr->substr(0, aLen - 2);
						break;
					}
				}
				else
				{
					if (c == '"')
					{
						inQuote = !inQuote;
						if (theElement->mType==XMLElement::TYPE_NONE || theElement->mType==XMLElement::TYPE_ELEMENT)
							processChar = true;

						if (!inQuote)
							gotEndQuote = true;
					}
					else if (!inQuote)
					{
						if (c == '<')
						{
							if (theElement->mType == XMLElement::TYPE_ELEMENT)
							{
								//TODO: Fix buffered text.  Not sure what I meant by that.

								//OLD: mBufferedText = c + mBufferedText;

								mBufferedText.push_back(c);
								break;
							}

							if (theElement->mType == XMLElement::TYPE_NONE)
							{
								theElement->mType = XMLElement::TYPE_START;
							}
							else
							{
								Fail("Unexpected '<'");
								return false;
							}
						}
						else if (c == '>')
						{
							if (theElement->mType == XMLElement::TYPE_START)
							{
								bool insertEnd = false;

								if (aAttributeKey == "/")
								{
									// We will get this if we have a space before the />, so we can ignore it
									//  and go about our business now
									insertEnd = true;
								}
								else
								{
									// Probably isn't committed yet
									if (aAttributeKey.length() > 0)
									{
//										theElement->mAttributes[aLastAttributeKey] = aAttributeValue;

										aAttributeKey = XMLDecodeString(aAttributeKey);
										aAttributeValue = XMLDecodeString(aAttributeValue);

										aLastAttributeKey = aAttributeKey;
										AddAttribute(theElement, aLastAttributeKey, aAttributeValue);

										aAttributeKey = "";
										aAttributeValue = "";
									}

									if (aLastAttributeKey.length() > 0)
									{
										std::string aVal = theElement->mAttributes[aLastAttributeKey];

										int aLen = aVal.length();

										if ((aLen > 0) && (aVal[aLen-1] == '/'))
										{
											// Its an empty element, fake start and end segments
//											theElement->mAttributes[aLastAttributeKey] = aVal.substr(0, aLen - 1);

											AddAttribute(theElement, aLastAttributeKey, XMLDecodeString(aVal.substr(0, aLen - 1)));

											insertEnd = true;
										}
									}
									else
									{
										int aLen = theElement->mValue.length();

										if ((aLen > 0) && (theElement->mValue[aLen-1] == '/'))
										{
											// Its an empty element, fake start and end segments
											theElement->mValue = theElement->mValue.substr(0, aLen - 1);
											insertEnd = true;
										}
									}
								}

								// Do we want to fake an ending section?
								if (insertEnd)
								{
									std::string anAddString = "</" + theElement->mValue + ">";

									int anOldSize = mBufferedText.size();
									int anAddLength = anAddString.length();

									mBufferedText.resize(anOldSize + anAddLength);

									for (int i = 0; i < anAddLength; i++)
										mBufferedText[anOldSize + i] = (char)(anAddString[anAddLength - i - 1]);

									// clear out aAttributeKey, since it contains "/" as its value and will insert
									// it into the element's attribute map.
									aAttributeKey = "";

									//OLD: mBufferedText = "</" + theElement->mValue + ">" + mBufferedText;
								}

								if (mSection.length() != 0)
									mSection += "/";

								mSection += theElement->mValue;

								break;
							}
							else if (theElement->mType == XMLElement::TYPE_END)
							{
								int aLastSlash = mSection.rfind('/');
								if ((aLastSlash == -1) && (mSection.length() == 0))
								{
									Fail("Unexpected End");
									return false;
								}

								std::string aLastSectionName = mSection.substr(aLastSlash + 1);

								if (aLastSectionName != theElement->mValue)
								{
									Fail("End '" + theElement->mValue + "' Doesn't Match Start '" + aLastSectionName + "'");
									return false;
								}

								if (aLastSlash == -1)
									mSection.erase(mSection.begin(), mSection.end());
								else
									mSection.erase(mSection.begin() + aLastSlash, mSection.end());

								break;
							}
							else
							{
								Fail("Unexpected '>'");
								return false;
							}
						}
						else if ((c == '/') && (theElement->mType == XMLElement::TYPE_START) && (theElement->mValue == ""))
						{
							theElement->mType = XMLElement::TYPE_END;
						}
						else if ((c == '?') && (theElement->mType == XMLElement::TYPE_START) && (theElement->mValue == ""))
						{
							theElement->mType = XMLElement::TYPE_INSTRUCTION;
						}
						else if (::isspace((uchar) c))
						{
							if (theElement->mValue != "")
								hasSpace = true;

							// It's a comment!
							if ((theElement->mType == XMLElement::TYPE_START) && (theElement->mValue == "!--"))
								theElement->mType = XMLElement::TYPE_COMMENT;
						}
						else if ((uchar) c > 32)
						{
							processChar = true;
						}
						else
						{
							Fail("Illegal Character");
							return false;
						}
					}
					else
					{
						processChar = true;
					}

					if (processChar)
					{
						if (theElement->mType == XMLElement::TYPE_NONE)
							theElement->mType = XMLElement::TYPE_ELEMENT;

						if (theElement->mType == XMLElement::TYPE_START)
						{
							if (hasSpace)
							{
								if ((!doingAttribute) || ((!AttributeVal) && (c != '=')) ||
									((AttributeVal) && ((aAttributeValue.length() > 0) || gotEndQuote)))
								{
									if (doingAttribute)
									{
										aAttributeKey = XMLDecodeString(aAttributeKey);
										aAttributeValue = XMLDecodeString(aAttributeValue);

//										theElement->mAttributes[aAttributeKey] = aAttributeValue;

										AddAttribute(theElement, aAttributeKey, aAttributeValue);

										aAttributeKey = "";
										aAttributeValue = "";

										aLastAttributeKey = aAttributeKey;
									}
									else
									{
										doingAttribute = true;
									}

									AttributeVal = false;
								}

								hasSpace = false;
							}

							std::string* aStrPtr = nullptr;

							if (!doingAttribute)
							{
								theElement->mValue += c;
							}
							else
							{
								if (c == '=')
								{
									AttributeVal = true;
									gotEndQuote = false;
								}
								else
								{
									if (!AttributeVal)
										aStrPtr = &aAttributeKey;
									else
										aStrPtr = &aAttributeValue;
								}
							}

							if (aStrPtr != nullptr)
							{
								*aStrPtr += c;
							}
						}
						else
						{
							if (hasSpace)
							{
								theElement->mValue += " ";
								hasSpace = false;
							}

							theElement->mValue += c;
						}
					}
				}
			}
			else
			{
				if (theElement->mType != XMLElement::TYPE_NONE)
					Fail("Unexpected End of File");

				return false;
			}
		}

		if (aAttributeKey.length() > 0)
		{
			aAttributeKey = XMLDecodeString(aAttributeKey);
			aAttributeValue = XMLDecodeString(aAttributeValue);
//			theElement->mAttributes[aAttributeKey] = aAttributeValue;

			AddAttribute(theElement, aAttributeKey, aAttributeValue);
		}

		theElement->mValue = XMLDecodeString(theElement->mValue);

		// Ignore comments
		if ((theElement->mType != XMLElement::TYPE_COMMENT) || mAllowComments)
			return true;
	}
}

bool XMLParser::HasFailed()
{
	return mHasFailed;
}

std::string XMLParser::GetErrorText()
{
	return mErrorText;
}

int XMLParser::GetCurrentLineNum()
{
	return mLineNum;
}

std::string XMLParser::GetFileName()
{
	return mFileName;
}
