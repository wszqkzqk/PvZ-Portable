/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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

#ifndef __PVZPDEFINITION_H__
#define __PVZPDEFINITION_H__

#include <string>
#include "PvzpList.h"
#include "Reanimator.h"
#include "PvzpParticle.h"

enum class DefFieldType : int
{
	DT_INVALID,
	DT_INT,
	DT_FLOAT,
	DT_STRING,
	DT_ENUM,
	DT_VECTOR2,
	DT_ARRAY,
	DT_TRACK_FLOAT,
	DT_FLAGS,
	DT_IMAGE,
	DT_FONT
};

// Terminology: a "definition class" (_DefClass) stores definition data for another class (_Class),
// e.g. ReanimatorDefinition for Reanimation, PvzpParticleDefinition for PvzpParticleSystem.

// A DefSymbol records the value of one flag bit (or one enum entry) of a flags/enum field.
class DefSymbol
{
public:
	int                 mSymbolValue;                   //+0x0: value of the flag bit or enum entry; -1 means no such entry
	const char*         mSymbolName;                    //+0x4: name of the flag bit or enum entry; nullptr marks the end of the list
};
//extern DefSymbol gParticleFlagSymbols[];
//extern DefSymbol gEmitterTypeSymbols[];
//extern DefSymbol gParticleTypeSymbols[];

// A DefField describes one member variable (_MemVar) of a class and its layout within the class.
class DefField
{
public:
	const char*         mFieldName;                     //+0x0: name of _MemVar; an empty string marks the end of the field list
	int                 mFieldOffset;                   //+0x4: offset of _MemVar within its class
	DefFieldType        mFieldType;                     //+0x8: storage type of _MemVar; each type is read differently
	const void*         mExtraData;                     //+0xC: extra data used to deep-copy pointers in _MemVar
	// For a pointer member, mExtraData points to the DefMap of the pointee's definition class;
	// for a flags/enum member, it points to a DefSymbol array; otherwise it is nullptr.
	// Nested DefMaps are read recursively until no expandable pointers remain (deep copy).
};

// A DefMap describes the storage format of a definition class (_DefClass) and how to read it.
class DefMap
{
public:
	const DefField*     mMapFields;                     //+0x0: array of DefField entries, one per member of _DefClass
	int                 mDefSize;                       //+0x4: size of a _DefClass instance, i.e. the initial read length; usually sizeof(_DefClass)
	void*               (*mConstructorFunc)(void*);     //+0x8: pointer to the _DefClass constructor
};

void*            PvzpParticleDefinitionConstructor(void* thePointer);
void*            PvzpEmitterDefinitionConstructor(void* thePointer);
void*            ParticleFieldConstructor(void* thePointer);
void*            TrailDefinitionConstructor(void* thePointer);
void*            ReanimatorTransformConstructor(void* thePointer);
void*            ReanimatorTrackConstructor(void* thePointer);
void*            ReanimatorDefinitionConstructor(void* thePointer);

//extern DefField gParticleFieldDefFields[];
extern const DefMap gParticleFieldDefMap;
//extern DefField gEmitterDefFields[];
extern const DefMap gEmitterDefMap;
//extern DefField gParticleDefFields[];
extern const DefMap gParticleDefMap;
extern const DefMap gTrailDefMap;
//extern DefField gReanimatorTransformDefFields[];
extern const DefMap gReanimatorTransformDefMap;
//extern DefField gReanimatorTrackDefFields[];
extern const DefMap gReanimatorTrackDefMap;
//extern DefField gReanimatorDefFields[];
extern const DefMap gReanimatorDefMap;

// A DefinitionArrayDef corresponds to the pointee type of a pointer member.
class DefinitionArrayDef
{
public:
	void*               mArrayData;                     //+0x0: array of instances of a definition type, e.g. the track defs of a reanimation definition
	int                 mArrayCount;                    //+0x4: number of array elements, e.g. the track count or emitter count
	// An "array pointer + count" pair in a definition class is read as one DefinitionArrayDef,
	// e.g. mEmitterDefs/mEmitterDefCount in PvzpParticleDefinition, mParticleFields/mParticleFieldCount in PvzpEmitterDefinition.
};

// Prepended to compressed data; used to verify data integrity when decompressing.
class CompressedDefinitionHeader
{
public:
	unsigned int        mCookie;                        //+0x0: cookie for compression validation
	unsigned int        mUncompressedSize;              //+0x4: length of the uncompressed data
};

// A DefLoadResPath maps an image prefix to the directory holding its images.
class DefLoadResPath
{
public:
	const char*         mPrefix;                        //+0x0: image prefix, e.g. "IMAGE_"
	const char*         mDirectory;                     //+0x4: directory for images with this prefix, e.g. "images\"
};

std::string             DefinitionGetCompiledFilePathFromXMLFilePath(const std::string& theXMLFilePath);
bool                    IsFileInPakFile(const std::string& theFilePath);
bool                    DefinitionIsCompiled(const std::string& theXMLFilePath);
bool                    DefinitionReadCompiledFile(const std::string& theCompiledFilePath, const DefMap* theDefMap, void* theDefinition);
void                    DefinitionFillWithDefaults(const DefMap* theDefMap, void* theDefinition);
void                    DefinitionXmlError(XMLParser* theXmlParser, char* theFormat, ...);
bool                    DefSymbolValueFromString(const DefSymbol* theSymbolMap, const char* theName, int* theResultValue);
bool                    DefinitionReadXMLString(XMLParser* theXmlParser, std::string& theValue);
bool                    DefinitionReadIntField(XMLParser* theXmlParser, int* theValue);
bool                    DefinitionReadFloatField(XMLParser* theXmlParser, float* theValue);
bool                    DefinitionReadStringField(XMLParser* theXmlParser, const char** theValue);
bool                    DefinitionReadEnumField(XMLParser* theXmlParser, int* theValue, const DefSymbol* theSymbolMap);
bool                    DefinitionReadVector2Field(XMLParser* theXmlParser, SexyVector2* theValue);
bool                    DefinitionReadArrayField(XMLParser* theXmlParser, DefinitionArrayDef* theArray, const DefField* theField);
bool                    DefinitionReadFloatTrackField(XMLParser* theXmlParser, FloatParameterTrack* theTrack);
bool                    DefinitionReadFlagField(XMLParser* theXmlParser, const std::string& theElementName, uint* theResultValue, const DefSymbol* theSymbolMap);
bool                    DefinitionReadImageField(XMLParser* theXmlParser, Image** theImage);
bool                    DefinitionReadFontField(XMLParser* theXmlParser, _Font** theFont);
bool                    DefinitionReadField(XMLParser* theXmlParser, const DefMap* theDefMap, void* theDefinition, bool* theDone);
bool                    DefinitionWriteCompiledFile(const std::string& theCompiledFilePath, const DefMap* theDefMap, void* theDefinition);
bool                    DefinitionCompileFile(const std::string& theXMLFilePath, const std::string& theCompiledFilePath, const DefMap* theDefMap, void* theDefinition);

void                    DefMapWriteToCache(void*& theWritePtr, const DefMap* theDefMap, void* theDefinition);
void                    DefWriteToCacheString(void*& theWritePtr, const char** theValue);
void                    DefWriteToCacheArray(void*& theWritePtr, DefinitionArrayDef* theValue, const DefMap* theDefMap);
void                    DefWriteToCacheFloatTrack(void*& theWritePtr, FloatParameterTrack* theValue);
void                    DefWriteToCacheImage(void*& theWritePtr, Image** theValue);
void                    DefWriteToCacheFont(void*& theWritePtr, _Font** theValue);

void*                   DefinitionCompressCompiledBuffer(void* theBuffer, unsigned int theBufferSize, unsigned int* theResultSize);

unsigned int DefGetSizeString(const char** theValue);
unsigned int DefinitionGetArraySize(DefinitionArrayDef* theValue, const DefMap* theDefMap);
unsigned int DefGetSizeFloatTrack(FloatParameterTrack* theValue);
unsigned int DefGetSizeImage(Image** theValue);
unsigned int DefGetSizeFont(_Font** theValue);

unsigned int DefinitionGetDeepSize(const DefMap* theDefMap, void* theDefinition);
unsigned int DefinitionGetSize(const DefMap* theDefMap, void* theDefinition);
void*        DefinitionAlloc(int theSize);
void*                   DefinitionUncompressCompiledBuffer(void* theCompressedBuffer, size_t theCompressedBufferSize, size_t& theUncompressedSize, const std::string& theCompiledFilePath);
uint                    DefinitionCalcHashSymbolMap(int aSchemaHash, const DefSymbol* theSymbolMap);
uint                    DefinitionCalcHashDefMap(int aSchemaHash, const DefMap* theDefMap, PvzpList<const DefMap*>& theProgressMaps);
uint                    DefinitionCalcHash(const DefMap* theDefMap);
inline bool             DefReadFromCacheString(void*& theReadPtr, const char** theString);
inline bool             DefReadFromCacheArray(void*& theReadPtr, DefinitionArrayDef* theArray, const DefMap* theDefMap);
inline bool             DefReadFromCacheImage(void*& theReadPtr, Image** theImage);
inline bool             DefReadFromCacheFont(void*& theReadPtr, _Font** theFont);
inline bool             DefReadFromCacheFloatTrack(void*& theReadPtr, FloatParameterTrack* theTrack);
bool                    DefMapReadFromCache(void*& theReadPtr, const DefMap* theDefMap, void* theDefinition);
bool                    DefinitionCompileAndLoad(const std::string& theXMLFilePath, const DefMap* theDefMap, void* theDefinition);
bool                    DefinitionLoadMap(XMLParser* theXmlParser, const DefMap* theDefMap, void* theDefinition);
bool                    DefinitionLoadImage(Image** theImage, const std::string& theName);
bool                    DefinitionLoadFont(_Font** theFont, const std::string& theName);
bool                    DefinitionLoadXML(const std::string& theFilename, const DefMap* theDefMap, void* theDefinition);
void                    DefinitionFreeArrayField(DefinitionArrayDef* theArray, const DefMap* theDefMap);
void                    DefinitionFreeMap(const DefMap* theDefMap, void* theDefinition);

bool         FloatTrackIsSet(const FloatParameterTrack& theTrack);
void         FloatTrackSetDefault(FloatParameterTrack& theTrack, float theValue);
float                   FloatTrackEvaluate(FloatParameterTrack& theTrack, float theTimeValue, float theInterp);
float                   FloatTrackEvaluateFromLastTime(FloatParameterTrack& theTrack, float theTimeValue, float theInterp);
bool         FloatTrackIsConstantZero(FloatParameterTrack& theTrack);

#endif
