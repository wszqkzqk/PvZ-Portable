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

#include "TodCommon.h"
#include "TodParticle.h"
#include "Trail.h"
#include <assert.h>
#include <cstring>
#include <stddef.h>
#include <sys/stat.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include "TodDebug.h"
#include "Definition.h"
#include "zlib.h"
#include "paklib/PakInterface.h"
#include "misc/PerfTimer.h"
#include "misc/XMLParser.h"
#include "../Resources.h"
#include "Common.h"

constinit const DefSymbol gTrailFlagDefSymbols[] = {
    { .mSymbolValue = 0, .mSymbolName = "Loops" },                 { .mSymbolValue = -1, .mSymbolName = nullptr }
};
constinit const DefField gTrailDefFields[] = {
    { .mFieldName = "Image", .mFieldOffset = offsetof(TrailDefinition, mImage), .mFieldType = DefFieldType::DT_IMAGE, .mExtraData = nullptr },
    { .mFieldName = "MaxPoints", .mFieldOffset = offsetof(TrailDefinition, mMaxPoints), .mFieldType = DefFieldType::DT_INT, .mExtraData = nullptr },
    { .mFieldName = "MinPointDistance", .mFieldOffset = offsetof(TrailDefinition, mMinPointDistance), .mFieldType = DefFieldType::DT_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "TrailFlags", .mFieldOffset = offsetof(TrailDefinition, mTrailFlags), .mFieldType = DefFieldType::DT_FLAGS, .mExtraData = gTrailFlagDefSymbols },
    { .mFieldName = "WidthOverLength", .mFieldOffset = offsetof(TrailDefinition, mWidthOverLength), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "WidthOverTime", .mFieldOffset = offsetof(TrailDefinition, mWidthOverTime), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "AlphaOverLength", .mFieldOffset = offsetof(TrailDefinition, mAlphaOverLength), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "AlphaOverTime", .mFieldOffset = offsetof(TrailDefinition, mAlphaOverTime), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "TrailDuration", .mFieldOffset = offsetof(TrailDefinition, mTrailDuration), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "", .mFieldOffset = 0x0, .mFieldType = DefFieldType::DT_INVALID, .mExtraData = nullptr }
};
constinit const DefMap gTrailDefMap = { .mMapFields = gTrailDefFields, .mDefSize = sizeof(TrailDefinition), .mConstructorFunc = TrailDefinitionConstructor };

constinit const DefSymbol gParticleFlagSymbols[] = {
    { .mSymbolValue = 0, .mSymbolName = "RandomLaunchSpin" },     { .mSymbolValue = 1, .mSymbolName = "AlignLaunchSpin" },  { .mSymbolValue = 2, .mSymbolName = "AlignToPixel" },     { .mSymbolValue = 4, .mSymbolName = "ParticleLoops" },    { .mSymbolValue = 3, .mSymbolName = "SystemLoops" },
    { .mSymbolValue = 5, .mSymbolName = "ParticlesDontFollow" },  { .mSymbolValue = 6, .mSymbolName = "RandomStartTime" },  { .mSymbolValue = 7, .mSymbolName = "DieIfOverloaded" },  { .mSymbolValue = 8, .mSymbolName = "Additive" },         { .mSymbolValue = 9, .mSymbolName = "FullScreen" },
    { .mSymbolValue = 10, .mSymbolName = "SoftwareOnly" },         { .mSymbolValue = 11, .mSymbolName = "HardwareOnly" },     { .mSymbolValue = -1, .mSymbolName = nullptr }
};
constinit const DefSymbol gEmitterTypeSymbols[] = {
    { .mSymbolValue = 0, .mSymbolName = "Circle" },               { .mSymbolValue = 1, .mSymbolName = "Box" },              { .mSymbolValue = 2, .mSymbolName = "BoxPath" },          { .mSymbolValue = 3, .mSymbolName = "CirclePath" },       { .mSymbolValue = 4, .mSymbolName = "CircleEvenSpacing" },
    { .mSymbolValue = -1, .mSymbolName = nullptr }
};
constinit const DefSymbol gParticleTypeSymbols[] = {
    { .mSymbolValue = 1, .mSymbolName = "Friction" },             { .mSymbolValue = 2, .mSymbolName = "Acceleration" },     { .mSymbolValue = 3, .mSymbolName = "Attractor" },        { .mSymbolValue = 4, .mSymbolName = "MaxVelocity" },      { .mSymbolValue = 5, .mSymbolName = "Velocity" },
    { .mSymbolValue = 6, .mSymbolName = "Position" },             { .mSymbolValue = 7, .mSymbolName = "SystemPosition" },   { .mSymbolValue = 8, .mSymbolName = "GroundConstraint" }, { .mSymbolValue = 9, .mSymbolName = "Shake" },            { .mSymbolValue = 10, .mSymbolName = "Circle" },
    { .mSymbolValue = 11, .mSymbolName = "Away" },                 { .mSymbolValue = -1, .mSymbolName = nullptr }
};

constinit const DefField gParticleFieldDefFields[] = {
    { .mFieldName = "FieldType", .mFieldOffset = offsetof(ParticleField, mFieldType), .mFieldType = DefFieldType::DT_ENUM, .mExtraData = gParticleTypeSymbols },
    { .mFieldName = "x", .mFieldOffset = offsetof(ParticleField, mX), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "y", .mFieldOffset = offsetof(ParticleField, mY), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "", .mFieldOffset = 0x0, .mFieldType = DefFieldType::DT_INVALID, .mExtraData = nullptr },
};
constinit const DefMap gParticleFieldDefMap = { .mMapFields = gParticleFieldDefFields, .mDefSize = sizeof(ParticleField), .mConstructorFunc = ParticleFieldConstructor };

constinit const DefField gEmitterDefFields[] = {
    { .mFieldName = "Image", .mFieldOffset = offsetof(TodEmitterDefinition,mImage), .mFieldType = DefFieldType::DT_IMAGE, .mExtraData = nullptr },
    { .mFieldName = "ImageRow", .mFieldOffset = offsetof(TodEmitterDefinition,mImageRow), .mFieldType = DefFieldType::DT_INT, .mExtraData = nullptr },
    { .mFieldName = "ImageCol", .mFieldOffset = offsetof(TodEmitterDefinition,mImageCol), .mFieldType = DefFieldType::DT_INT, .mExtraData = nullptr },
    { .mFieldName = "ImageFrames", .mFieldOffset = offsetof(TodEmitterDefinition,mImageFrames), .mFieldType = DefFieldType::DT_INT, .mExtraData = nullptr },
    { .mFieldName = "Animated", .mFieldOffset = offsetof(TodEmitterDefinition,mAnimated), .mFieldType = DefFieldType::DT_INT, .mExtraData = nullptr },
    { .mFieldName = "ParticleFlags", .mFieldOffset = offsetof(TodEmitterDefinition,mParticleFlags), .mFieldType = DefFieldType::DT_FLAGS, .mExtraData = gParticleFlagSymbols },
    { .mFieldName = "EmitterType", .mFieldOffset = offsetof(TodEmitterDefinition,mEmitterType), .mFieldType = DefFieldType::DT_ENUM, .mExtraData = gEmitterTypeSymbols },
    { .mFieldName = "Name", .mFieldOffset = offsetof(TodEmitterDefinition,mName), .mFieldType = DefFieldType::DT_STRING, .mExtraData = nullptr },
    { .mFieldName = "SystemDuration", .mFieldOffset = offsetof(TodEmitterDefinition,mSystemDuration), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "OnDuration", .mFieldOffset = offsetof(TodEmitterDefinition,mOnDuration), .mFieldType = DefFieldType::DT_STRING, .mExtraData = nullptr },
    { .mFieldName = "CrossFadeDuration", .mFieldOffset = offsetof(TodEmitterDefinition,mCrossFadeDuration), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "SpawnRate", .mFieldOffset = offsetof(TodEmitterDefinition,mSpawnRate), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "SpawnMinActive", .mFieldOffset = offsetof(TodEmitterDefinition,mSpawnMinActive), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "SpawnMaxActive", .mFieldOffset = offsetof(TodEmitterDefinition,mSpawnMaxActive), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "SpawnMaxLaunched", .mFieldOffset = offsetof(TodEmitterDefinition,mSpawnMaxLaunched), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "EmitterRadius", .mFieldOffset = offsetof(TodEmitterDefinition,mEmitterRadius), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "EmitterOffsetX", .mFieldOffset = offsetof(TodEmitterDefinition,mEmitterOffsetX), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "EmitterOffsetY", .mFieldOffset = offsetof(TodEmitterDefinition,mEmitterOffsetY), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "EmitterBoxX", .mFieldOffset = offsetof(TodEmitterDefinition,mEmitterBoxX), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "EmitterBoxY", .mFieldOffset = offsetof(TodEmitterDefinition,mEmitterBoxY), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "EmitterPath", .mFieldOffset = offsetof(TodEmitterDefinition,mEmitterPath), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "EmitterSkewX", .mFieldOffset = offsetof(TodEmitterDefinition,mEmitterSkewX), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "EmitterSkewY", .mFieldOffset = offsetof(TodEmitterDefinition,mEmitterSkewY), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "ParticleDuration", .mFieldOffset = offsetof(TodEmitterDefinition,mParticleDuration), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "SystemRed", .mFieldOffset = offsetof(TodEmitterDefinition,mSystemRed), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "SystemGreen", .mFieldOffset = offsetof(TodEmitterDefinition,mSystemGreen), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "SystemBlue", .mFieldOffset = offsetof(TodEmitterDefinition,mSystemBlue), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "SystemAlpha", .mFieldOffset = offsetof(TodEmitterDefinition,mSystemAlpha), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "SystemBrightness", .mFieldOffset = offsetof(TodEmitterDefinition,mSystemBrightness), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "LaunchSpeed", .mFieldOffset = offsetof(TodEmitterDefinition,mLaunchSpeed), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "LaunchAngle", .mFieldOffset = offsetof(TodEmitterDefinition,mLaunchAngle), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "Field", .mFieldOffset = offsetof(TodEmitterDefinition,mParticleFields), .mFieldType = DefFieldType::DT_ARRAY, .mExtraData = &gParticleFieldDefMap },
    { .mFieldName = "SystemField", .mFieldOffset = offsetof(TodEmitterDefinition,mSystemFields), .mFieldType = DefFieldType::DT_ARRAY, .mExtraData = &gParticleFieldDefMap },
    { .mFieldName = "ParticleRed", .mFieldOffset = offsetof(TodEmitterDefinition,mParticleRed), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "ParticleGreen", .mFieldOffset = offsetof(TodEmitterDefinition,mParticleGreen), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "ParticleBlue", .mFieldOffset = offsetof(TodEmitterDefinition,mParticleBlue), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "ParticleAlpha", .mFieldOffset = offsetof(TodEmitterDefinition,mParticleAlpha), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "ParticleBrightness", .mFieldOffset = offsetof(TodEmitterDefinition,mParticleBrightness), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "ParticleSpinAngle", .mFieldOffset = offsetof(TodEmitterDefinition,mParticleSpinAngle), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "ParticleSpinSpeed", .mFieldOffset = offsetof(TodEmitterDefinition,mParticleSpinSpeed), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "ParticleScale", .mFieldOffset = offsetof(TodEmitterDefinition,mParticleScale), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "ParticleStretch", .mFieldOffset = offsetof(TodEmitterDefinition,mParticleStretch), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "CollisionReflect", .mFieldOffset = offsetof(TodEmitterDefinition,mCollisionReflect), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "CollisionSpin", .mFieldOffset = offsetof(TodEmitterDefinition,mCollisionSpin), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "ClipTop", .mFieldOffset = offsetof(TodEmitterDefinition,mClipTop), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "ClipBottom", .mFieldOffset = offsetof(TodEmitterDefinition,mClipBottom), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "ClipLeft", .mFieldOffset = offsetof(TodEmitterDefinition,mClipLeft), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "ClipRight", .mFieldOffset = offsetof(TodEmitterDefinition,mClipRight), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "AnimationRate", .mFieldOffset = offsetof(TodEmitterDefinition,mAnimationRate), .mFieldType = DefFieldType::DT_TRACK_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "", .mFieldOffset = 0x0, .mFieldType = DefFieldType::DT_INVALID, .mExtraData = nullptr },
};
constinit const DefMap gEmitterDefMap = { .mMapFields = gEmitterDefFields, .mDefSize = sizeof(TodEmitterDefinition), .mConstructorFunc = TodEmitterDefinitionConstructor };

constinit const DefField gParticleDefFields[] = {
    { .mFieldName = "Emitter", .mFieldOffset = offsetof(TodParticleDefinition,mEmitterDefs), .mFieldType = DefFieldType::DT_ARRAY, .mExtraData = &gEmitterDefMap },
    { .mFieldName = "", .mFieldOffset = 0x0, .mFieldType = DefFieldType::DT_INVALID, .mExtraData = nullptr }
};
constinit const DefMap gParticleDefMap = { .mMapFields = gParticleDefFields, .mDefSize = sizeof(TodParticleDefinition), .mConstructorFunc = TodParticleDefinitionConstructor };

constinit const DefField gReanimatorTransformDefFields[] = {
    { .mFieldName = "x", .mFieldOffset = offsetof(ReanimatorTransform,mTransX), .mFieldType = DefFieldType::DT_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "y", .mFieldOffset = offsetof(ReanimatorTransform,mTransY), .mFieldType = DefFieldType::DT_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "kx", .mFieldOffset = offsetof(ReanimatorTransform,mSkewX), .mFieldType = DefFieldType::DT_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "ky", .mFieldOffset = offsetof(ReanimatorTransform,mSkewY), .mFieldType = DefFieldType::DT_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "sx", .mFieldOffset = offsetof(ReanimatorTransform,mScaleX), .mFieldType = DefFieldType::DT_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "sy", .mFieldOffset = offsetof(ReanimatorTransform,mScaleY), .mFieldType = DefFieldType::DT_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "f", .mFieldOffset = offsetof(ReanimatorTransform,mFrame), .mFieldType = DefFieldType::DT_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "a", .mFieldOffset = offsetof(ReanimatorTransform,mAlpha), .mFieldType = DefFieldType::DT_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "i", .mFieldOffset = offsetof(ReanimatorTransform,mImage), .mFieldType = DefFieldType::DT_IMAGE, .mExtraData = nullptr },
    { .mFieldName = "font", .mFieldOffset = offsetof(ReanimatorTransform,mFont), .mFieldType = DefFieldType::DT_FONT, .mExtraData = nullptr },
    { .mFieldName = "text", .mFieldOffset = offsetof(ReanimatorTransform,mText), .mFieldType = DefFieldType::DT_STRING, .mExtraData = nullptr },
    { .mFieldName = "", .mFieldOffset = 0, .mFieldType = DefFieldType::DT_INVALID, .mExtraData = nullptr }
};
constinit const DefMap gReanimatorTransformDefMap = { .mMapFields = gReanimatorTransformDefFields, .mDefSize = sizeof(ReanimatorTransform), .mConstructorFunc = ReanimatorTransformConstructor };

constinit const DefField gReanimatorTrackDefFields[] = {
    { .mFieldName = "name", .mFieldOffset = offsetof(ReanimatorTrack,mName), .mFieldType = DefFieldType::DT_STRING, .mExtraData = nullptr },
    { .mFieldName = "t", .mFieldOffset = offsetof(ReanimatorTrack,mTransforms), .mFieldType = DefFieldType::DT_ARRAY, .mExtraData = &gReanimatorTransformDefMap },
    { .mFieldName = "", .mFieldOffset = 0x0, .mFieldType = DefFieldType::DT_INVALID, .mExtraData = nullptr }
};
constinit const DefMap gReanimatorTrackDefMap = { .mMapFields = gReanimatorTrackDefFields, .mDefSize = sizeof(ReanimatorTrack), .mConstructorFunc = ReanimatorTrackConstructor };

constinit const DefField gReanimatorDefFields[] = {
    { .mFieldName = "track", .mFieldOffset = offsetof(ReanimatorDefinition,mTracks), .mFieldType = DefFieldType::DT_ARRAY, .mExtraData = &gReanimatorTrackDefMap },
    { .mFieldName = "fps", .mFieldOffset = offsetof(ReanimatorDefinition,mFPS), .mFieldType = DefFieldType::DT_FLOAT, .mExtraData = nullptr },
    { .mFieldName = "", .mFieldOffset = 0x0, .mFieldType = DefFieldType::DT_INVALID, .mExtraData = nullptr }
};
constinit const DefMap gReanimatorDefMap = { .mMapFields = gReanimatorDefFields, .mDefSize = sizeof(ReanimatorDefinition), .mConstructorFunc = ReanimatorDefinitionConstructor };

constinit const static DefLoadResPath gDefLoadResPaths[4] = { { .mPrefix = "IMAGE_", .mDirectory = "" }, { .mPrefix = "IMAGE_", .mDirectory = "particles/" }, { .mPrefix = "IMAGE_REANIM_", .mDirectory = "reanim/" }, { .mPrefix = "IMAGE_REANIM_", .mDirectory = "images/" } };

void* ParticleFieldConstructor(void* thePointer)
{
    if (thePointer)
    {
        ((ParticleField*)thePointer)->mX.mNodes = nullptr;
        ((ParticleField*)thePointer)->mX.mCountNodes = 0;
        ((ParticleField*)thePointer)->mY.mNodes = nullptr;
        ((ParticleField*)thePointer)->mY.mCountNodes = 0;
        ((ParticleField*)thePointer)->mFieldType = ParticleFieldType::FIELD_INVALID;
    }
    return thePointer;
}

void* TodEmitterDefinitionConstructor(void* thePointer)
{
    if (thePointer)
    {
        memset(thePointer, 0, sizeof(TodEmitterDefinition));
        ((TodEmitterDefinition*)thePointer)->mImageFrames = 1;
        ((TodEmitterDefinition*)thePointer)->mEmitterType = EmitterType::EMITTER_BOX;
        ((TodEmitterDefinition*)thePointer)->mName = "";
        ((TodEmitterDefinition*)thePointer)->mOnDuration = "";
        ((TodEmitterDefinition*)thePointer)->mImageRow = 0;
        ((TodEmitterDefinition*)thePointer)->mImageCol = 0;
        ((TodEmitterDefinition*)thePointer)->mAnimated = 0;
        ((TodEmitterDefinition*)thePointer)->mImage = nullptr;
        ((TodEmitterDefinition*)thePointer)->mParticleFields.count = 0;
    }
    return thePointer;
}

void* TodParticleDefinitionConstructor(void* thePointer)
{
    if (thePointer)
    {
        ((TodParticleDefinition*)thePointer)->mEmitterDefs = nullptr;
        ((TodParticleDefinition*)thePointer)->mEmitterDefCount = 0;
    }
    return thePointer;
}

void* TrailDefinitionConstructor(void* thePointer)
{
    if (thePointer)
    {
        memset(thePointer, 0, sizeof(TrailDefinition));
        ((TrailDefinition*)thePointer)->mMinPointDistance = 1.0f;
        ((TrailDefinition*)thePointer)->mMaxPoints = 2;
        ((TrailDefinition*)thePointer)->mTrailFlags = 0U;
        ((TrailDefinition*)thePointer)->mImage = nullptr;
    }
    return thePointer;
}

void* ReanimatorTransformConstructor(void* thePointer)
{
    if (thePointer)
    {
        ((ReanimatorTransform*)thePointer)->mTransX = DEFAULT_FIELD_PLACEHOLDER;
        ((ReanimatorTransform*)thePointer)->mTransY = DEFAULT_FIELD_PLACEHOLDER;
        ((ReanimatorTransform*)thePointer)->mSkewX = DEFAULT_FIELD_PLACEHOLDER;
        ((ReanimatorTransform*)thePointer)->mSkewY = DEFAULT_FIELD_PLACEHOLDER;
        ((ReanimatorTransform*)thePointer)->mScaleX = DEFAULT_FIELD_PLACEHOLDER;
        ((ReanimatorTransform*)thePointer)->mScaleY = DEFAULT_FIELD_PLACEHOLDER;
        ((ReanimatorTransform*)thePointer)->mFrame = DEFAULT_FIELD_PLACEHOLDER;
        ((ReanimatorTransform*)thePointer)->mAlpha = DEFAULT_FIELD_PLACEHOLDER;
        ((ReanimatorTransform*)thePointer)->mImage = nullptr;
        ((ReanimatorTransform*)thePointer)->mFont = nullptr;
        ((ReanimatorTransform*)thePointer)->mText = "";
    }
    return thePointer;
}

void* ReanimatorTrackConstructor(void* thePointer)
{
    if (thePointer)
    {
        ((ReanimatorTrack*)thePointer)->mName = "";
        ((ReanimatorTrack*)thePointer)->mTransforms = {nullptr, 0};
    }
    return thePointer;
}

void* ReanimatorDefinitionConstructor(void* thePointer)
{
    if (thePointer)
    {
        ((ReanimatorDefinition*)thePointer)->mTracks = {nullptr, 0};
        ((ReanimatorDefinition*)thePointer)->mFPS = 12.0f;
        ((ReanimatorDefinition*)thePointer)->mReanimAtlas = nullptr;
    }
    return thePointer;
}

// @Patoke implement
unsigned int DefGetSizeString(const char** theValue) {
    return strlen(*theValue) + sizeof(unsigned int);
}

unsigned int DefinitionGetArraySize(DefinitionArrayDef* theValue, const DefMap* theDefMap) {
    unsigned int aResult = theValue->mArrayCount * theDefMap->mDefSize + sizeof(unsigned int);
    for (int i = 0; theValue->mArrayCount > i; ++i) {
        aResult += DefinitionGetDeepSize(theDefMap, (void*)((intptr_t)theValue->mArrayData + i * theDefMap->mDefSize));
    }
    return aResult;
}

unsigned int DefGetSizeFloatTrack(FloatParameterTrack* theValue) {
    return sizeof(FloatParameterTrackNode) * theValue->mCountNodes + sizeof(unsigned int);
}

unsigned int DefGetSizeImage(Image** theValue) {
    std::string aImagePath{};
    if (*theValue)
        TodFindImagePath(*theValue, &aImagePath);
    return aImagePath.length() + sizeof(unsigned int);
}

unsigned int DefGetSizeFont(_Font** theValue) {
    std::string aFontPath{};
    if (*theValue)
        TodFindFontPath(*theValue, &aFontPath);
    return aFontPath.length() + sizeof(unsigned int);
}

unsigned int DefinitionGetDeepSize(const DefMap* theDefMap, void* theDefinition) {
    unsigned int aResult = 0;
    for (const DefField* aField = theDefMap->mMapFields; *aField->mFieldName != '\0'; aField++) {
        void* aDest = (void*)((intptr_t)theDefinition + aField->mFieldOffset);
        switch (aField->mFieldType) {
        case DefFieldType::DT_STRING:
            aResult += DefGetSizeString((const char**)aDest);
            break;
        case DefFieldType::DT_ARRAY:
            aResult += DefinitionGetArraySize((DefinitionArrayDef*)aDest, (const DefMap*)aField->mExtraData);
            break;
        case DefFieldType::DT_TRACK_FLOAT:
            aResult += DefGetSizeFloatTrack((FloatParameterTrack*)aDest);
            break;
        case DefFieldType::DT_IMAGE:
            aResult += DefGetSizeImage((Image**)aDest);
            break;
        case DefFieldType::DT_FONT:
            aResult += DefGetSizeFont((_Font**)aDest);
            break;
        default:
            continue;
        }
    }

    return aResult;
}

unsigned int DefinitionGetSize(const DefMap* theDefMap, void* theDefinition) {
    return theDefMap->mDefSize + DefinitionGetDeepSize(theDefMap, theDefinition);
}

void* DefinitionAlloc(int theSize)
{
    void* aPtr = operator new[](theSize);
    TOD_ASSERT(aPtr);
    memset(aPtr, 0, theSize);
    return aPtr;
}

bool DefinitionLoadImage(Image** theImage, const std::string& theName)
{
    // 当贴图文件路径不存在时，无须获取贴图
    if (theName.empty())
    {
        *theImage = nullptr;
        return true;
    }

    // 尝试借助资源管理器，从 XML 中加载贴图
    Image* anImage = (Image*)gSexyAppBase->mResourceManager->LoadImage(theName);
    if (anImage)
    {
        *theImage = anImage;
        return true;
    }

    for (const DefLoadResPath& aLoadResPath : gDefLoadResPaths)
    {
        size_t aPrefixLen = strlen(aLoadResPath.mPrefix);
        if (aPrefixLen < theName.size())
        {
            std::string aPathToTry = aLoadResPath.mDirectory + theName.substr(aPrefixLen);
            SharedImageRef aImageRef = gSexyAppBase->GetSharedImage(aPathToTry);
            if ((Image*)aImageRef != nullptr)
            {
                TodHesitationTrace("Load Image '%s'", theName.c_str());
                TodAddImageToMap(&aImageRef, theName);
                TodMarkImageForSanding((Image*)aImageRef);
                *theImage = (Image*)aImageRef;
                return true;
            }
        }
    }
    return false;
}

bool DefinitionLoadFont(_Font** theFont, const std::string& theName)
{
    _Font* aFont = gSexyAppBase->mResourceManager->LoadFont(theName);
    *theFont = aFont;
    return aFont != nullptr;
}

bool DefinitionLoadXML(const std::string& theFileName, const DefMap* theDefMap, void* theDefinition)
{
    return DefinitionCompileAndLoad(theFileName, theDefMap, theDefinition);
}

inline bool DefReadFromCacheArray(void*& theReadPtr, DefinitionArrayDef* theArray, const DefMap* theDefMap)
{
    int aDefSize;
    SMemR(theReadPtr, &aDefSize, sizeof(int));  // 先读取一个整数表示 theDefMap 描述的定义结构类的大小
    if (aDefSize != theDefMap->mDefSize)  // 比较其与当前给出的定义结构图声明的大小是否一致
    {
        TodTrace("cache has old def: array size");
        return false;
    }
    if (theArray->mArrayCount == 0)  // 如果类中没有实例，则无需读取
        return true;

    int aArraySize = aDefSize * theArray->mArrayCount;
    theArray->mArrayData = DefinitionAlloc(aArraySize);  // 申请内存并初始化填充为 0
    SMemR(theReadPtr, theArray->mArrayData, aArraySize);  // 仍然是粗略读取全部数据，然后再根据 theDefMap 的结构字段数组修复指针
    for (int i = 0; i < theArray->mArrayCount; i++)
        if (!DefMapReadFromCache(theReadPtr, theDefMap, (void*)((intptr_t)theArray->mArrayData + theDefMap->mDefSize * i)))  // 最后一个参数表示 pData[i]
            return false;
    return true;
}

inline bool DefReadFromCacheFloatTrack(void*& theReadPtr, FloatParameterTrack* theTrack)
{
    int& aCountNodes = theTrack->mCountNodes;
    SMemR(theReadPtr, &aCountNodes, sizeof(int));
    if (aCountNodes > 0)
    {
        int aSize = aCountNodes * sizeof(FloatParameterTrackNode);
        FloatParameterTrackNode* aPtr = (FloatParameterTrackNode*)DefinitionAlloc(aSize);
        theTrack->mNodes = aPtr;
        SMemR(theReadPtr, aPtr, aSize);
    }
    return true;
}

inline bool DefReadFromCacheString(void*& theReadPtr, const char** theString)
{
    int aLen;
    SMemR(theReadPtr, &aLen, sizeof(int));
    if (aLen == 0)
        *theString = "";
    else
    {
        auto aPtr = static_cast<char*>(DefinitionAlloc(aLen + 1));
        *theString = aPtr;
        SMemR(theReadPtr, aPtr, aLen);
        aPtr[aLen] = '\0';
    }
    return true;
}

inline bool DefReadFromCacheImage(void*& theReadPtr, Image** theImage)
{
    int aLen;
    SMemR(theReadPtr, &aLen, sizeof(int));  // 读取贴图标签字符数组的长度
    std::string aImageName(aLen, '\0');
    SMemR(theReadPtr, aImageName.data(), aLen);  // 读取贴图标签字符数组

    *theImage = nullptr;
    return aImageName[0] == '\0' || DefinitionLoadImage(theImage, aImageName);
}

inline bool DefReadFromCacheFont(void*& theReadPtr, _Font** theFont)
{
    int aLen;
    SMemR(theReadPtr, &aLen, sizeof(int));  // 读取字体标签字符数组的长度
    std::string aFontName(aLen, '\0');
    SMemR(theReadPtr, aFontName.data(), aLen);  // 读取字体标签字符数组

    *theFont = nullptr;
    return aFontName[0] == '\0' || DefinitionLoadFont(theFont, aFontName);
}

bool DefMapReadFromCache(void*& theReadPtr, const DefMap* theDefMap, void* theDefinition)
{
    // 分别确认每一个成员变量，并修复其中的指针类型和标志类型的变量
    for (const DefField* aField = theDefMap->mMapFields; *aField->mFieldName != '\0'; aField++)
    {
        bool aSucceed = true;
        void* aDest = (void*)((intptr_t)theDefinition + aField->mFieldOffset);  // 指向该成员变量的指针
        switch (aField->mFieldType)
        {
        case DefFieldType::DT_STRING:
            aSucceed = DefReadFromCacheString(theReadPtr, (const char**)aDest);
            break;
        case DefFieldType::DT_ARRAY:
            aSucceed = DefReadFromCacheArray(theReadPtr, (DefinitionArrayDef*)aDest, (const DefMap*)aField->mExtraData);
            break;
        case DefFieldType::DT_IMAGE:
            aSucceed = DefReadFromCacheImage(theReadPtr, (Image**)aDest);
            break;
        case DefFieldType::DT_FONT:
            aSucceed = DefReadFromCacheFont(theReadPtr, (_Font**)aDest);
            break;
        case DefFieldType::DT_TRACK_FLOAT:
            aSucceed = DefReadFromCacheFloatTrack(theReadPtr, (FloatParameterTrack*)aDest);
            break;
        default:
            break;
        }

        if (!aSucceed)
            return false;
    }
    return true;
}

uint DefinitionCalcHashSymbolMap(int aSchemaHash, const DefSymbol* theSymbolMap)
{
    while (theSymbolMap->mSymbolName != nullptr)
    {
        aSchemaHash = crc32(aSchemaHash, (const Bytef*)theSymbolMap->mSymbolName, strlen(theSymbolMap->mSymbolName));
        aSchemaHash = crc32(aSchemaHash, (const Bytef*)&theSymbolMap->mSymbolValue, sizeof(int));
        theSymbolMap++;
    }
    return aSchemaHash;
}

uint DefinitionCalcHashDefMap(int aSchemaHash, const DefMap* theDefMap, TodList<const DefMap*>& theProgressMaps)
{
    for (TodListNode<const DefMap*>* aNode = theProgressMaps.mHead; aNode != nullptr; aNode = aNode->mNext)
        if (aNode->mValue == theDefMap)
            return aSchemaHash;
    theProgressMaps.AddTail(theDefMap);

    aSchemaHash = crc32(aSchemaHash, (Bytef*)&theDefMap->mDefSize, sizeof(int));
    for (const DefField* aField = theDefMap->mMapFields; *aField->mFieldName != '\0'; aField++)
    {
        aSchemaHash = crc32(aSchemaHash, (Bytef*)&aField->mFieldType, sizeof(DefFieldType));
        aSchemaHash = crc32(aSchemaHash, (Bytef*)&aField->mFieldOffset, sizeof(int));
        switch (aField->mFieldType)
        {
        case DefFieldType::DT_ENUM:
        case DefFieldType::DT_FLAGS:
            aSchemaHash = DefinitionCalcHashSymbolMap(aSchemaHash, (const DefSymbol*)aField->mExtraData);
            break;
        case DefFieldType::DT_ARRAY:
            aSchemaHash = DefinitionCalcHashDefMap(aSchemaHash, (const DefMap*)aField->mExtraData, theProgressMaps);
            break;
        default:
            break;
        }
    }
    return aSchemaHash;
}

uint DefinitionCalcHash(const DefMap* theDefMap)
{
    // Uninitialised!!
    TodList<const DefMap*> aProgressMaps = TodList<const DefMap*>();
    uint aResult = DefinitionCalcHashDefMap(crc32(0L, (Bytef*)Z_NULL, 0) + 1, theDefMap, aProgressMaps);

    // TodList destructor is called upon it going out of scope.
    return aResult;
}

// UnCompress(&theUncompressedSize, theCompressedBufferSize, esi = *theCompressedBuffer)
void* DefinitionUncompressCompiledBuffer(void* theCompressedBuffer, size_t theCompressedBufferSize, size_t& theUncompressedSize, const std::string& theCompiledFilePath)
{
    // theCompressedBuffer 的前两个四字节存有特殊数据，此处检测其长度是否足够 8 字节（即 2 个四字节）
    if (theCompressedBufferSize < 8)
    {
        TodTrace("Compile def too small: %s", theCompiledFilePath.c_str());
        return nullptr;
    }
    // 将 theCompressedBuffer 的前两个四字节视为一个 CompressedDefinitionHeader
    CompressedDefinitionHeader* aHeader = (CompressedDefinitionHeader*)theCompressedBuffer;
    if (aHeader->mCookie != 0xDEADFED4L)
    {
        TodTrace("Compiled fire cookie wrong: %s\n", theCompiledFilePath.c_str());
        return nullptr;
    }
    
    Bytef* aUncompressedBuffer = (Bytef*)DefinitionAlloc(aHeader->mUncompressedSize);
    Bytef* aSrc = (Bytef*)((intptr_t)theCompressedBuffer + sizeof(CompressedDefinitionHeader));  // 实际解压数据从第 3 个四字节开始
    // BuGFIXX!!
    ulong aUncompressedSizeResult = aHeader->mUncompressedSize;  // 用作出参的未压缩数据实际长度
    int aResult = uncompress(aUncompressedBuffer, &aUncompressedSizeResult, aSrc, theCompressedBufferSize - sizeof(CompressedDefinitionHeader));
    (void)aResult; // Compiler can't work out that this is used in the Debug build
    TOD_ASSERT(aResult == Z_OK);
    TOD_ASSERT(aUncompressedSizeResult == aHeader->mUncompressedSize);
    theUncompressedSize = aHeader->mUncompressedSize;
    return aUncompressedBuffer;
}

std::string DefinitionGetCompiledFilePathFromXMLFilePath(const std::string& theXMLFilePath)
{
    return "compiled/" + theXMLFilePath + ".compiled";
}

static std::string DefinitionGetCompiledCacheFullPath(const std::string& theCompiledFilePath)
{
    std::string aCacheRoot = (sizeof(void*) == 8) ? "cache64/" : "cache32/";
    return GetAppDataPath(aCacheRoot + theCompiledFilePath);
}

static bool DefinitionGetFileModTime(const std::string& theFilePath, std::filesystem::file_time_type& theTime)
{
    std::error_code ec;
    theTime = std::filesystem::last_write_time(Sexy::PathFromU8(theFilePath), ec);
    return !ec;
}

// (void* def, *defMap, eax = string& compiledFilePath)  //esp -= 8
bool DefinitionReadCompiledFile(const std::string& theCompiledFilePath, const DefMap* theDefMap, void* theDefinition)
{
    PerfTimer aTimer;
    aTimer.Start();

    std::string aFullCompiledPath = DefinitionGetCompiledCacheFullPath(theCompiledFilePath);
    std::ifstream aFileStream(Sexy::PathFromU8(aFullCompiledPath), std::ios::binary);
    if (!aFileStream)
    {
        aFileStream.open(Sexy::PathFromU8(theCompiledFilePath), std::ios::binary);
    }

    if (!aFileStream) return false;

    aFileStream.seekg(0, std::ios::end);
    size_t aCompressedSize = (size_t)aFileStream.tellg();
    aFileStream.seekg(0, std::ios::beg);
    std::vector<char> aCompressedBuffer(aCompressedSize);
    aFileStream.read(aCompressedBuffer.data(), (std::streamsize)aCompressedSize);
    bool aReadCompressedFailed = !aFileStream || (size_t)aFileStream.gcount() != aCompressedSize;
    if (aReadCompressedFailed) { // 判断是否读取成功
        TodTrace("Failed to read compiled file: %s\n", theCompiledFilePath.c_str());
        return false;
    }

    size_t aUncompressedSize;
    std::unique_ptr<char[]> aUncompressedBuffer(
        static_cast<char*>(DefinitionUncompressCompiledBuffer(aCompressedBuffer.data(), aCompressedSize, aUncompressedSize, theCompiledFilePath)));
    if (!aUncompressedBuffer) return false;

    uint aDefHash = DefinitionCalcHash(theDefMap);  // 计算 CRC 校验值，后将用于检测数据的完整性
    if (aUncompressedSize < theDefMap->mDefSize + sizeof(uint)) {
        TodTrace("Compiled file size too small: %s\n", theCompiledFilePath.c_str());
        return false;
    } // 检测解压数据的长度是否足够“定义数据 + 一个校验值记录数据”的长度


    // 复制一份解压数据的指针用于读取时移动，原指针后续要用于计算读取区域大小
    void* aBufferPtr = aUncompressedBuffer.get();
    uint aCashHash;
    SMemR(aBufferPtr, &aCashHash, sizeof(uint));  // 读取记录的 CRC 校验值
    if (aCashHash != aDefHash) {
        TodTrace("Compiled file schema wrong: %s\n", theCompiledFilePath.c_str());
        return false;
    } // 判断校验值是否一致，若不一致则说明数据发生错误

    // ☆ 正式开始读取定义数据 ☆
    // 初次粗略读取 theDefinition 原类型的定义数据，囫囵吞枣地将所有记录的数据全部读入到 theDefinition 中
    // 此时 theDefinition 原本的非指针类型的数据将全部被正确读取，但其指针类型的变量会被读取并赋值为野指针
    // 这些野指针的问题后续将会在 DefMapReadFromCache() 中借助相应 DefField 的 mExtraData 进行修复
    SMemR(aBufferPtr, theDefinition, theDefMap->mDefSize);
    // 修复野指针及标志型数据，并保存是否成功的结果，后续作为返回值
    bool aResult = DefMapReadFromCache(aBufferPtr, theDefMap, theDefinition);
    size_t aReadMemSize = (uintptr_t)aBufferPtr - (uintptr_t)aUncompressedBuffer.get();
    if (aResult && aReadMemSize != aUncompressedSize) {
        TodTrace("Compiled file wrong size: %s\n", theCompiledFilePath.c_str());
        return false;
    }
    return aResult;
}

bool IsFileInPakFile(const std::string& theFilePath)
{
    PFILE* pFile = p_fopen(theFilePath.c_str(), "rb");
    bool aIsInPak = pFile && !pFile->mFP;  // 通过 mPakRecordMap.find 找到并打开的文件，其 mFP 为空指针（因为不是从实际文件中打开的）
    if (pFile)
    {
        p_fclose(pFile);
    }
    return aIsInPak;
}

bool DefinitionIsCompiled(const std::string& theXMLFilePath)
{
    std::string aCompiledFilePath = DefinitionGetCompiledFilePathFromXMLFilePath(theXMLFilePath);
    if (IsFileInPakFile(aCompiledFilePath))
        return true;

    std::string aFullCompiledPath = DefinitionGetCompiledCacheFullPath(aCompiledFilePath);
    std::filesystem::file_time_type aCompiledFileTime{};
    if (!DefinitionGetFileModTime(aFullCompiledPath, aCompiledFileTime))
    {
        if (!DefinitionGetFileModTime(aCompiledFilePath, aCompiledFileTime))
            return false;
    }

    std::filesystem::file_time_type aXMLFileTime{};
    if (!DefinitionGetFileModTime(theXMLFilePath, aXMLFileTime))
    {
        TodTrace("Can't find source file to compile '%s'", theXMLFilePath.c_str());
        return false;
    }

    return aXMLFileTime <= aCompiledFileTime;
}

void DefinitionFillWithDefaults(const DefMap* theDefMap, void* theDefinition)
{
    memset(theDefinition, 0, theDefMap->mDefSize);  // 将 theDefinition 初始化填充为 0
    for (const DefField* aField = theDefMap->mMapFields; *aField->mFieldName != '\0'; aField++)  // 遍历 theDefinition 的每一个成员变量
        if (aField->mFieldType == DefFieldType::DT_STRING)
            *(const char**)((uintptr_t)theDefinition + aField->mFieldOffset) = "";  // 将所有 const char* 类型的成员变量赋值为空字符串的指针
}

void DefinitionXmlError(XMLParser* theXmlParser, const char* theFormat, ...)
{
    va_list argList;
    va_start(argList, theFormat);
    std::string aFormattedMessage = Sexy::VFormat(theFormat, argList);
    va_end(argList);

    int aLine = theXmlParser->GetCurrentLineNum();
    std::string aFileName = theXmlParser->GetFileName();
    TodTraceAndLogLn("%s(%d): XML Definition Error: %s", aFileName.c_str(), aLine, aFormattedMessage.c_str());
}

bool DefinitionReadXMLString(XMLParser* theXmlParser, std::string& theValue)
{
    XMLElement aXMLElement;
    if (!theXmlParser->NextElement(&aXMLElement))  // 读取下一个 XML 元素
    {
        DefinitionXmlError(theXmlParser, "Missing element value");
        return false;
    }
    if (aXMLElement.mType == XMLElement::TYPE_END)  // 读取到结束标签则结束处理
        return true;
    else if (aXMLElement.mType != XMLElement::TYPE_ELEMENT)  // 除结束标签外，正常情况下，此处读取到的应是定义的正片内容
    {
        DefinitionXmlError(theXmlParser, "unknown element type");
        return false;
    }

    theValue = aXMLElement.mValue;  // ☆ 赋值出参

    if (!theXmlParser->NextElement(&aXMLElement))  // 继续读取下一个 XML 元素
    {
        DefinitionXmlError(theXmlParser, "Can't read element end");
        return false;
    }
    if (aXMLElement.mType != XMLElement::TYPE_END)  // 正常情况下，此处读取到的应是结束标签
    {
        DefinitionXmlError(theXmlParser, "Missing element end");
        return false;
    }
    return true;
}

bool DefSymbolValueFromString(const DefSymbol* theSymbolMap, const char* theName, int* theResultValue)
{
    while (theSymbolMap->mSymbolName != nullptr)
    {
        if (strcasecmp(theName, theSymbolMap->mSymbolName) == 0)
        {
            *theResultValue = theSymbolMap->mSymbolValue;
            return true;
        }
        theSymbolMap++;
    }
    return false;
}

bool DefinitionReadIntField(XMLParser* theXmlParser, int* theValue)
{
    std::string aStringValue;
    if (!DefinitionReadXMLString(theXmlParser, aStringValue))
        return false;

    if (sscanf(aStringValue.c_str(), "%d", theValue) == 1)
        return true;

    DefinitionXmlError(theXmlParser, "Can't parse int value '%s'", aStringValue.c_str());
    return false;
}

bool DefinitionReadFloatField(XMLParser* theXmlParser, float* theValue)
{
    std::string aStringValue;
    if (!DefinitionReadXMLString(theXmlParser, aStringValue))
        return false;

    if (sscanf(aStringValue.c_str(), "%f", theValue) == 1)
        return true;

    DefinitionXmlError(theXmlParser, "Can't parse float value '%s'", aStringValue.c_str());
    return false;
}

bool DefinitionReadStringField(XMLParser* theXmlParser, const char** theValue)
{
    std::string aStringValue;
    if (!DefinitionReadXMLString(theXmlParser, aStringValue))
        return false;

    if (aStringValue.empty())
    {
        *theValue = "";
    }
    else
    {
        char* aPtr = static_cast<char*>(DefinitionAlloc(aStringValue.size() + 1));
        memcpy(aPtr, aStringValue.c_str(), aStringValue.size() + 1);
        *theValue = aPtr;
    }
    return true;
}

bool DefinitionReadEnumField(XMLParser* theXmlParser, int* theValue, const DefSymbol* theSymbolMap)
{
    std::string aStringValue;
    if (!DefinitionReadXMLString(theXmlParser, aStringValue))
        return false;

    if (DefSymbolValueFromString(theSymbolMap, aStringValue.c_str(), theValue))
        return true;

    DefinitionXmlError(theXmlParser, "Can't parse enum value '%s'", aStringValue.c_str());
    return false;
}

bool DefinitionReadVector2Field(XMLParser* theXmlParser, SexyVector2* theValue)
{
    std::string aStringValue;
    if (!DefinitionReadXMLString(theXmlParser, aStringValue))
        return false;

    if (sscanf(aStringValue.c_str(), "%f %f", &theValue->x, &theValue->y) == 2)
        return true;

    DefinitionXmlError(theXmlParser, "Can't parse vector2 value '%s'", aStringValue.c_str());
    return false;
}

bool DefinitionReadArrayField(XMLParser* theXmlParser, DefinitionArrayDef* theArray, const DefField* theField)
{
    const DefMap* aDefMap = (const DefMap*)theField->mExtraData;

    if (theArray->mArrayCount == 0)
    {
        theArray->mArrayCount = 1;
        theArray->mArrayData = DefinitionAlloc(aDefMap->mDefSize);
    }
    else
    {
        // 当 theArray 中已存在元素，且元素的个数为 2 的整数次幂时
        // TODO Potential error with the bracketing for the &
        if (theArray->mArrayCount >= 1 && (theArray->mArrayCount == 1 || ((theArray->mArrayCount & (theArray->mArrayCount - 1)) == 0)))
        {
            void* anOldData = theArray->mArrayData;
            theArray->mArrayData = DefinitionAlloc(2 * theArray->mArrayCount * aDefMap->mDefSize);
            memcpy(theArray->mArrayData, anOldData, theArray->mArrayCount * aDefMap->mDefSize);
            delete[] (char *)anOldData;
        }
        theArray->mArrayCount++;
    }

    if (DefinitionLoadMap(theXmlParser, aDefMap, (unsigned char*)theArray->mArrayData + aDefMap->mDefSize * (theArray->mArrayCount - 1)))
        return true;

    DefinitionXmlError(theXmlParser, "failed to read sub def");
    return false;
}

/*
Float Track Field EBNF:
    <test>     ::= <trackdef> ("\n"+ <trackdef>?)*
    <trackdef> ::= <node> (" " <node>)*
    <node>     ::= <norange> | <range>
    <norange>  ::= <float> ("," <float>)? (" " <curve>)?
    <range>    ::= "[" (<float> | <float> (" " <curve>)? " " <float>) "]" ("," <float>)?
    <curve>    ::= "Bounce" | "EaseIn" | "EaseInOutWeak" | "EaseOut" | "FastInOutWeak"
    <float>    ::= "-"? (<natural> ("." <natural>?)? | ("." <natural>))
    <natural>  ::= <digit>+
    <digit>    ::= [0-9]
*/

/*
struct TodCurveStringMap {
    char *mString;
    TodCurves mCurveType;
};

const TodCurveStringMap TodCurveStrings[] = {
    {(char *)"Bounce",        TodCurves::CURVE_BOUNCE},
    {(char *)"FastInOutWeak", TodCurves::CURVE_FAST_IN_OUT_WEAK},
    {(char *)"EaseInOutWeak", TodCurves::CURVE_EASE_IN_OUT_WEAK},
    {(char *)"EaseOut",       TodCurves::CURVE_EASE_OUT},
    {(char *)"EaseIn",        TodCurves::CURVE_EASE_IN},
};
*/

constinit const DefSymbol gDefTrackEaseSymbols[] = {
    { .mSymbolValue = TodCurves::CURVE_EASE_IN_OUT_WEAK,   .mSymbolName = "EaseInOutWeak" },
    { .mSymbolValue = TodCurves::CURVE_FAST_IN_OUT_WEAK,   .mSymbolName = "FastInOutWeak" },
    { .mSymbolValue = TodCurves::CURVE_EASE_IN_OUT,        .mSymbolName = "EaseInOut" },
    { .mSymbolValue = TodCurves::CURVE_FAST_IN_OUT,        .mSymbolName = "FastInOut" },
    { .mSymbolValue = TodCurves::CURVE_EASE_IN,            .mSymbolName = "EaseIn" },
    { .mSymbolValue = TodCurves::CURVE_EASE_OUT,           .mSymbolName = "EaseOut" },
    { .mSymbolValue = TodCurves::CURVE_EASE_SIN_WAVE,      .mSymbolName = "EaseSinWave" },
    { .mSymbolValue = TodCurves::CURVE_BOUNCE_FAST_MIDDLE, .mSymbolName = "BounceFastMiddle" },
    { .mSymbolValue = TodCurves::CURVE_BOUNCE_SLOW_MIDDLE, .mSymbolName = "BounceSlowMiddle" },
    { .mSymbolValue = TodCurves::CURVE_BOUNCE,             .mSymbolName = "Bounce" },
    { .mSymbolValue = TodCurves::CURVE_SIN_WAVE,           .mSymbolName = "SinWave" },
    { .mSymbolValue = TodCurves::CURVE_LINEAR,             .mSymbolName = "Linear" },
};

bool DefinitionReadFloatTrackField(XMLParser* theXmlParser, FloatParameterTrack* theTrack)
{
    std::string aStringValue;

    if (!DefinitionReadXMLString(theXmlParser, aStringValue)) return false;
    
    float aValue = 0;
    int aLen = 0;

    const char *aStringChars = aStringValue.c_str();
    size_t anIdx = 0;

    theTrack->mCountNodes = 0;

    std::vector<FloatParameterTrackNode> aFloatTrackVec = std::vector<FloatParameterTrackNode>();
    FloatParameterTrackNode aTrackNode = FloatParameterTrackNode();
    while(true) {
        if (anIdx >= aStringValue.length()) {
            return false;
        }
        if (aStringChars[anIdx] == '\0') goto _m_break; // No empty strings allowed

        aTrackNode.mTime = -1;
        aTrackNode.mCurveType = TodCurves::CURVE_LINEAR;
        aTrackNode.mDistribution = TodCurves::CURVE_LINEAR;

        if (aStringChars[anIdx] == '[') {
            // <range>
            anIdx++;
            if (sscanf(aStringChars + anIdx, "%f%n", &aValue, &aLen) != 1) return false; // mLowValue
            anIdx += aLen;
            aTrackNode.mLowValue = aValue;
            aTrackNode.mHighValue = aValue;
            if (aStringChars[anIdx] != ']') {
                anIdx++; // space (' ')
                // <curve>
                for (size_t i = 0; i < sizeof(gDefTrackEaseSymbols)/sizeof(gDefTrackEaseSymbols[0]); ++i) {
                    size_t aStrLen = strlen(gDefTrackEaseSymbols[i].mSymbolName);
                    if (strncmp(gDefTrackEaseSymbols[i].mSymbolName, aStringChars + anIdx, aStrLen) == 0) // could be the distribution?
                    {
                        aTrackNode.mDistribution = (TodCurves)gDefTrackEaseSymbols[i].mSymbolValue;
                        anIdx += aStrLen + 1; // Accounts for space (' '), expressions never end with a curve
                        break;
                    }
                }
                switch(sscanf(aStringChars + anIdx, "%f%n", &aValue, &aLen)) // mHighValue
                { 
                case 1: // Float read successfully
                    anIdx += aLen; 
                    aTrackNode.mHighValue = aValue;
                    break;
                case 0: // No float to read just continue
                    break;
                default: // Something bad happened, panic!
                    return false;
                }
            }
            if (aStringChars[anIdx] != ']') return false; // Invalid format
            anIdx++;
            if (aStringChars[anIdx] == '\0') goto _m_break; // Done!

            if (aStringChars[anIdx] == ',') {
                anIdx++;
                if (sscanf(aStringChars + anIdx, "%f%n", &aValue, &aLen) != 1) return false; // mTime
                anIdx += aLen;
                aTrackNode.mTime = aValue * 0.01;
            }
            if (aStringChars[anIdx] == '\0') goto _m_break; // Done!
            anIdx++;
        } else {
            // <norange>
            if (sscanf(aStringChars + anIdx, "%f%n", &aValue, &aLen) != 1) return false; // mLow/HighValue
            anIdx += aLen;
            aTrackNode.mLowValue = aValue;
            aTrackNode.mHighValue = aValue;

            if (aStringChars[anIdx] == '\0') goto _m_break; // Done!
    
            if (aStringChars[anIdx] == ',') {
                anIdx++;
                if (sscanf(aStringChars + anIdx, "%f%n", &aValue, &aLen) != 1) return false; // mTime
                anIdx += aLen;
                aTrackNode.mTime = aValue * 0.01;
            }
            if (aStringChars[anIdx] == '\0') goto _m_break; // Done!
            anIdx++;
            // <curve>
            for (size_t i = 0; i < sizeof(gDefTrackEaseSymbols)/sizeof(gDefTrackEaseSymbols[0]); ++i) {
                size_t aStrLen = strlen(gDefTrackEaseSymbols[i].mSymbolName);
                if (strncmp(gDefTrackEaseSymbols[i].mSymbolName, aStringChars + anIdx, aStrLen) == 0) // mCurveType
                {
                    aTrackNode.mCurveType = (TodCurves)gDefTrackEaseSymbols[i].mSymbolValue;
                    anIdx += aStrLen;
                    if (aStringChars[anIdx] == '\0') goto _m_break; // Done!
                    anIdx++;
                    break;
                }
            }
        }
        
        aFloatTrackVec.push_back(aTrackNode);
    }
    _m_break:
    aFloatTrackVec.push_back(aTrackNode);
    
    // Search forward for a timestamp:
    size_t aBaseIdx = 0;
    anIdx = 0;
    float high = 0.0;
    float low = 0.0;
    do {
        for(anIdx = aBaseIdx; anIdx < aFloatTrackVec.size(); ++anIdx) {
            if (aFloatTrackVec[anIdx].mTime >= 0.0){
                // Found a timestamp!
                high = aFloatTrackVec[anIdx].mTime;
                goto _m_found; // Since we break out anIdx isn't incremented.
            }
        }
        // Didn't find another value, we're finished.
        // Since we did break, anIdx == aFloatTrackVec.size(), which means final value is set
        high = 1.0;
    _m_found:
        // Going backwards set previous timestamps
        for(size_t i = aBaseIdx; i < anIdx; ++i) { // Iterate up to anIdx - 1
            float interp;
            if (((anIdx - 1) - aBaseIdx) != 0) interp = ((float)(i - aBaseIdx))/((float)((anIdx - 1) - aBaseIdx));
            else if (aBaseIdx == 0) interp = 0.0;
            else interp = 1.0;
            aFloatTrackVec[i].mTime = high*interp + low*(1 - interp);
        }
        // Start again
        aBaseIdx = anIdx + 1;
        low = high;
    } while (aBaseIdx < aFloatTrackVec.size());

    
    /*
    TodTraceAndLogLn("%s | %d", aStringChars, aFloatTrackVec.size());
    for (auto &i : aFloatTrackVec) {
        TodTraceAndLogLn("%f", i.mTime);
    }
    */

    size_t alloc_size = aFloatTrackVec.size() * sizeof(FloatParameterTrackNode);
    theTrack->mNodes = (FloatParameterTrackNode*)DefinitionAlloc(alloc_size);
    if (!theTrack->mNodes) return false;

    ::memcpy(theTrack->mNodes, aFloatTrackVec.data(), alloc_size);
    theTrack->mCountNodes = aFloatTrackVec.size();

    return true;
}

bool DefinitionReadFlagField(XMLParser* theXmlParser, const std::string& theElementName, uint* theResultValue, const DefSymbol* theSymbolMap)
{
    int aValue;
    if (!DefSymbolValueFromString(theSymbolMap, theElementName.c_str(), &aValue))
        return false;

    std::string aStringValue;
    if (!DefinitionReadXMLString(theXmlParser, aStringValue))
        return false;

    float aFlag; // This was obviously a bug, the casting is wrong, although amusingly it just woks since it's just a bit
    if (sscanf(aStringValue.c_str(), "%f", &aFlag) != 1)
    {
        DefinitionXmlError(theXmlParser, "Can't parse int value '%s'", aStringValue.c_str());
        return false;
    }

    // Still I'll let the compiler work out the optimisation
    int aFlag_int = aFlag;

    if (theResultValue) {
        if (aFlag_int)
        {
            *theResultValue |= 1 << aValue;
        }
        else
        {
            *theResultValue &= ~(1 << aValue);
        }
    }
    
    return true;
}

bool DefinitionReadImageField(XMLParser* theXmlParser, Image** theImage)
{
    std::string aStringValue;
    if (!DefinitionReadXMLString(theXmlParser, aStringValue))
        return false;

    if (DefinitionLoadImage(theImage, aStringValue))
        return true;

    std::string aMessgae = StrFormat("Failed to find image '%s' in %s", aStringValue.c_str(), theXmlParser->GetFileName().c_str());
    TodErrorMessageBox(aMessgae.c_str(), "Missing image");

    return false;
}

bool DefinitionReadFontField(XMLParser* theXmlParser, _Font** theFont)
{
    std::string aStringValue;
    if (!DefinitionReadXMLString(theXmlParser, aStringValue))
        return false;

    if (DefinitionLoadFont(theFont, aStringValue))
        return true;

    std::string aMessgae = StrFormat("Failed to find font '%s' in %s", aStringValue.c_str(), theXmlParser->GetFileName().c_str());
    TodErrorMessageBox(aMessgae.c_str(), "Missing font");

    return false;
}

bool DefinitionReadField(XMLParser* theXmlParser, const DefMap* theDefMap, void* theDefinition, bool* theDone)
{
    if (theXmlParser->HasFailed())
        return false;

    XMLElement aXMLElement = XMLElement();
    if (!theXmlParser->NextElement(&aXMLElement) || aXMLElement.mType == XMLElement::TYPE_END)  // 读取下一个 XML 元素
    {
        *theDone = true;  // 没有下一个元素则表示读取完成
        return true;
    }
    if (aXMLElement.mType != XMLElement::TYPE_START)  // 正常情况下，此处读取到的应是开始标签，而其他内容在后续的相应函数中读取
    {
        DefinitionXmlError(theXmlParser, "Missing element start");
        return false;
    }

    for (const DefField* aField = theDefMap->mMapFields; *aField->mFieldName != '\0'; aField++)
    {
        void* pVar = (void*)((uintptr_t)theDefinition + aField->mFieldOffset);
        // Missing pvar field for some reason!
        if (aField->mFieldType == DefFieldType::DT_FLAGS && DefinitionReadFlagField(theXmlParser, aXMLElement.mValue, (uint*)pVar, (const DefSymbol*)aField->mExtraData))
            return true;
        
        if (strcasecmp(aXMLElement.mValue.c_str(), aField->mFieldName) == 0)  // 判断 aXMLElement 定义的是否为该成员变量
        {
            bool aSuccess;
            switch (aField->mFieldType)
            {
            case DefFieldType::DT_INT:
                aSuccess = DefinitionReadIntField(theXmlParser, (int*)pVar);
                break;
            case DefFieldType::DT_FLOAT:
                aSuccess = DefinitionReadFloatField(theXmlParser, (float*)pVar);
                break;
            case DefFieldType::DT_STRING:
                aSuccess = DefinitionReadStringField(theXmlParser, (const char**)pVar);
                break;
            case DefFieldType::DT_ENUM:
                aSuccess = DefinitionReadEnumField(theXmlParser, (int*)pVar, (const DefSymbol*)aField->mExtraData);
                break;
            case DefFieldType::DT_VECTOR2:
                aSuccess = DefinitionReadVector2Field(theXmlParser, (SexyVector2*)pVar);
                break;
            case DefFieldType::DT_ARRAY:
                aSuccess = DefinitionReadArrayField(theXmlParser, (DefinitionArrayDef*)pVar, aField);
                break;
            case DefFieldType::DT_TRACK_FLOAT:
                aSuccess = DefinitionReadFloatTrackField(theXmlParser, (FloatParameterTrack*)pVar);
                break;
            case DefFieldType::DT_IMAGE:
                aSuccess = DefinitionReadImageField(theXmlParser, (Image**)pVar);
                break;
            case DefFieldType::DT_FONT:
                aSuccess = DefinitionReadFontField(theXmlParser, (_Font**)pVar);
                break;
            default:
                aSuccess = false;
                TOD_ASSERT(false);
                break;
            }
            if (aSuccess)
                return true;

            DefinitionXmlError(theXmlParser, "Failed to read '%s' field", aXMLElement.mValue.c_str());
            return false;
        }
    }
    DefinitionXmlError(theXmlParser, "Ignoring unknown element '%s'", aXMLElement.mValue.c_str());  // aXMLElement 未定义任何成员变量时
    return false;
}

bool DefinitionLoadMap(XMLParser* theXmlParser, const DefMap* theDefMap, void* theDefinition)
{
    if (theDefMap->mConstructorFunc)
        theDefMap->mConstructorFunc(theDefinition);  // 利用构造函数构造 theDefinition
    else
        DefinitionFillWithDefaults(theDefMap, theDefinition);  // 以默认值填充 theDefinition

    bool aDone = false;
    while (!aDone)
        if (!DefinitionReadField(theXmlParser, theDefMap, theDefinition, &aDone))
            return false;
    return true;
}

// @Patoke implemented
void DefWriteToCacheString(void*& theWritePtr, const char** theValue) {
    unsigned int aStringSize = strlen(*theValue);
    SMemW(theWritePtr, &aStringSize, sizeof(unsigned int));
    if (aStringSize > 0)
        SMemW(theWritePtr, *theValue, aStringSize);
}

void DefWriteToCacheArray(void*& theWritePtr, DefinitionArrayDef* theValue, const DefMap* theDefMap) {
	SMemW(theWritePtr, &theDefMap->mDefSize, sizeof(unsigned int));
	SMemW(theWritePtr, theValue->mArrayData, theDefMap->mDefSize * theValue->mArrayCount);
	for (int i = 0; i < theValue->mArrayCount; ++i)
		DefMapWriteToCache(theWritePtr, theDefMap, (void*)((intptr_t)theValue->mArrayData + i * theDefMap->mDefSize));
}

void DefWriteToCacheFloatTrack(void*& theWritePtr, FloatParameterTrack* theValue) {
    SMemW(theWritePtr, &theValue->mCountNodes, sizeof(unsigned int));
    if (theValue->mCountNodes > 0)
        SMemW(theWritePtr, theValue->mNodes, theValue->mCountNodes * sizeof(FloatParameterTrackNode));
}

void DefWriteToCacheImage(void*& theWritePtr, Image** theValue) {
    std::string aImageName{};
    if (*theValue)
        TodFindImagePath(*theValue, &aImageName);

    unsigned int aImageSize = aImageName.length();
    SMemW(theWritePtr, &aImageSize, sizeof(unsigned int));
    if (aImageSize > 0)
        SMemW(theWritePtr, aImageName.c_str(), aImageSize);
}

void DefWriteToCacheFont(void*& theWritePtr, _Font** theValue) {
    std::string aFontName{};
    if (*theValue) {
        TodFindFontPath(*theValue, &aFontName);
    }

    unsigned int aFontSize = aFontName.length();
    SMemW(theWritePtr, &aFontSize, sizeof(unsigned int));
    if (aFontSize > 0)
        SMemW(theWritePtr, aFontName.c_str(), aFontSize);
}

void DefMapWriteToCache(void*& theWritePtr, const DefMap* theDefMap, void* theDefinition) {
	for (const DefField* aField = theDefMap->mMapFields; *aField->mFieldName != '\0'; aField++) {
		void* aDest = (void*)((intptr_t)theDefinition + aField->mFieldOffset);
		switch (aField->mFieldType) {
		case DefFieldType::DT_STRING:
			DefWriteToCacheString(theWritePtr, (const char**)aDest);
			break;
		case DefFieldType::DT_ARRAY:
			DefWriteToCacheArray(theWritePtr, (DefinitionArrayDef*)aDest, (const DefMap*)aField->mExtraData);
			break;
		case DefFieldType::DT_TRACK_FLOAT:
			DefWriteToCacheFloatTrack(theWritePtr, (FloatParameterTrack*)aDest);
			break;
		case DefFieldType::DT_IMAGE:
			DefWriteToCacheImage(theWritePtr, (Image**)aDest);
			break;
		case DefFieldType::DT_FONT:
			DefWriteToCacheFont(theWritePtr, (_Font**)aDest);
			break;
		default:
			break;
		}
	}
}

void* DefinitionCompressCompiledBuffer(void* theBuffer, unsigned int theBufferSize, unsigned int* theResultSize) {
    uLongf aCompressedSize = compressBound(theBufferSize);
    auto aCompressedBuffer = (CompressedDefinitionHeader*)DefinitionAlloc(aCompressedSize + sizeof(CompressedDefinitionHeader));
    compress((Bytef*)((uintptr_t)aCompressedBuffer + sizeof(CompressedDefinitionHeader)), &aCompressedSize, (Bytef*)theBuffer, theBufferSize);
    aCompressedBuffer->mCookie = 0xDEADFED4;
    aCompressedBuffer->mUncompressedSize = theBufferSize;
    *theResultSize = aCompressedSize + sizeof(CompressedDefinitionHeader);
    return aCompressedBuffer;
}

bool DefinitionWriteCompiledFile(const std::string& theCompiledFilePath, const DefMap* theDefMap, void* theDefinition) {
    unsigned int aCompressedSize = 0;
    unsigned int aDefSize = DefinitionGetSize(theDefMap, theDefinition) + sizeof(unsigned int);
    std::vector<char> aDefBasePtr(aDefSize);
    void* aDef = aDefBasePtr.data();
    uint aDefHash = DefinitionCalcHash(theDefMap);

    SMemW(aDef, &aDefHash, sizeof(uint));
    SMemW(aDef, theDefinition, theDefMap->mDefSize);
    DefMapWriteToCache(aDef, theDefMap, theDefinition);
    std::unique_ptr<char[]> aCompressedDef(
        static_cast<char*>(DefinitionCompressCompiledBuffer(aDefBasePtr.data(), aDefSize, &aCompressedSize)));

    std::string aFullCompiledPath = DefinitionGetCompiledCacheFullPath(theCompiledFilePath);
    std::string aFilePath = GetFileDir(aFullCompiledPath);
    MkDir(aFilePath);

    std::ofstream aFileStream(Sexy::PathFromU8(aFullCompiledPath), std::ios::binary);
    if (aFileStream) {
        aFileStream.write(aCompressedDef.get(), (std::streamsize)aCompressedSize);
        return aFileStream.good();
    }

    return false;
}

bool DefinitionCompileFile(const std::string& theXMLFilePath, const std::string& theCompiledFilePath, const DefMap* theDefMap, void* theDefinition)
{
    XMLParser aXMLParser = XMLParser();
    if (!aXMLParser.OpenFile(theXMLFilePath))
    {
        TodTrace("XML file not found: %s\n", theXMLFilePath.c_str());
        return false;
    }
    else if (!DefinitionLoadMap(&aXMLParser, theDefMap, theDefinition))
        return false;
    
    return DefinitionWriteCompiledFile(theCompiledFilePath, theDefMap, theDefinition);
}

// (void* def, *defMap, string& xmlFilePath)  //esp -= 0xC
bool DefinitionCompileAndLoad(const std::string& theXMLFilePath, const DefMap* theDefMap, void* theDefinition)
{
#ifdef PVZ_DEBUG
    const bool aRequireCompiledUpToDate = true;
#else
    const bool aRequireCompiledUpToDate = false;
#endif

    TodHesitationTrace("predef");
    std::string aCompiledFilePath = DefinitionGetCompiledFilePathFromXMLFilePath(theXMLFilePath);

    const bool aShouldTryCompiled = !aRequireCompiledUpToDate || DefinitionIsCompiled(theXMLFilePath);
    if (aShouldTryCompiled && DefinitionReadCompiledFile(aCompiledFilePath, theDefMap, theDefinition))
    {
        TodHesitationTrace("loaded %s", aCompiledFilePath.c_str());
        return true;
    }

    PerfTimer aTimer;
    aTimer.Start();
    bool aResult = DefinitionCompileFile(theXMLFilePath, aCompiledFilePath, theDefMap, theDefinition);
    TodTrace("compile %d ms:'%s'", (int)aTimer.GetDuration(), aCompiledFilePath.c_str());
    TodHesitationTrace("compiled %s", aCompiledFilePath.c_str());
    if (aResult)
        return true;

#ifndef PVZ_DEBUG
    TodErrorMessageBox(StrFormat("missing resource %s", aCompiledFilePath.c_str()).c_str(), "Error");
    exit(0);
#endif
    return false;
}

float FloatTrackEvaluate(FloatParameterTrack& theTrack, float theTimeValue, float theInterp)
{
    if (theTrack.mCountNodes == 0)
        return 0.0f;

    if (theTimeValue < theTrack.mNodes[0].mTime)  // 如果当前时间小于第一个节点的开始时间
        return TodCurveEvaluate(theInterp, theTrack.mNodes[0].mLowValue, theTrack.mNodes[0].mHighValue, theTrack.mNodes[0].mDistribution);

    for (int i = 1; i < theTrack.mCountNodes; i++)
    {
        FloatParameterTrackNode* aNodeNxt = &theTrack.mNodes[i];
        if (theTimeValue <= aNodeNxt->mTime)  // 寻找首个开始时间大于当前时间的节点
        {
            FloatParameterTrackNode* aNodeCur = &theTrack.mNodes[i - 1];
            // 计算当前时间在〔当前节点至下一节点〕的过程中的进度
            float aTimeFraction = (theTimeValue - aNodeCur->mTime) / (aNodeNxt->mTime - aNodeCur->mTime);
            float aLeftValue = TodCurveEvaluate(theInterp, aNodeCur->mLowValue, aNodeCur->mHighValue, aNodeCur->mDistribution);
            float aRightValue = TodCurveEvaluate(theInterp, aNodeNxt->mLowValue, aNodeNxt->mHighValue, aNodeNxt->mDistribution);
            return TodCurveEvaluate(aTimeFraction, aLeftValue, aRightValue, aNodeCur->mCurveType);
        }
    }

    FloatParameterTrackNode* aLastNode = &theTrack.mNodes[theTrack.mCountNodes - 1];  // 如果当前时间大于最后一个节点的开始时间
    return TodCurveEvaluate(theInterp, aLastNode->mLowValue, aLastNode->mHighValue, aLastNode->mDistribution);
}

void FloatTrackSetDefault(FloatParameterTrack& theTrack, float theValue)
{
    if (theTrack.mNodes == nullptr && theValue != 0.0f)  // 确保该参数轨道无节点（未被赋值过）且给定的默认值不为 0
    {
        theTrack.mCountNodes = 1;  // 默认参数轨道有且仅有 1 个节点
        FloatParameterTrackNode* aNode = (FloatParameterTrackNode*)DefinitionAlloc(sizeof(FloatParameterTrackNode));
        theTrack.mNodes = aNode;
        aNode->mTime = 0.0f;
        aNode->mLowValue = theValue;
        aNode->mHighValue = theValue;
        aNode->mCurveType = TodCurves::CURVE_CONSTANT;
        aNode->mDistribution = TodCurves::CURVE_LINEAR;
    } else if (theTrack.mNodes == nullptr) {
        theTrack.mCountNodes = 0;
    }
}

bool FloatTrackIsSet(const FloatParameterTrack& theTrack)
{
    return theTrack.mCountNodes != 0 && theTrack.mNodes[0].mCurveType != TodCurves::CURVE_CONSTANT;
}

bool FloatTrackIsConstantZero(FloatParameterTrack& theTrack)
{
    // 当轨道无节点，或仅存在一个节点且该节点的最大、最小值均为 0 时，认为该轨道上的值恒为零
    return theTrack.mCountNodes == 0 || (theTrack.mCountNodes == 1 && theTrack.mNodes[0].mLowValue == 0.0f && theTrack.mNodes[0].mHighValue == 0.0f);
}

float FloatTrackEvaluateFromLastTime(FloatParameterTrack& theTrack, float theTimeValue, float theInterp)
{
    return theTimeValue < 0.0f ? 0.0f : FloatTrackEvaluate(theTrack, theTimeValue, theInterp);
}

void DefinitionFreeArrayField(DefinitionArrayDef* theArray, const DefMap* theDefMap)
{
    for (int i = 0; i < theArray->mArrayCount; i++)
        DefinitionFreeMap(theDefMap, (void*)((intptr_t)theArray->mArrayData + theDefMap->mDefSize * i));  // 最后一个参数表示 pData[i]
    delete[] (char *)theArray->mArrayData;
    theArray->mArrayData = nullptr;
}

void DefinitionFreeMap(const DefMap* theDefMap, void* theDefinition)
{
    // 根据 theDefMap 遍历 theDefinition 的每个成员变量
    for (const DefField* aField = theDefMap->mMapFields; *aField->mFieldName != '\0'; aField++)
    {
        void* aVar = (void*)((intptr_t)theDefinition + aField->mFieldOffset);  // 指向该成员变量的指针
        switch (aField->mFieldType)
        {
        case DefFieldType::DT_STRING:
            // @Patoke todo: removed this, caused a heap problem when closing the game, add back properly (causes memory leak)
            //if (**(const char**)aVar != '\0')
            //    delete[] *(const char**)aVar;  // 释放字符数组
            *(const char**)aVar = nullptr;
            break;
        case DefFieldType::DT_ARRAY:
            DefinitionFreeArrayField((DefinitionArrayDef*)aVar, (const DefMap*)aField->mExtraData);
            break;
        case DefFieldType::DT_TRACK_FLOAT:
            if (((FloatParameterTrack*)aVar)->mCountNodes != 0)
                delete[]((FloatParameterTrack*)aVar)->mNodes;  // 释放浮点参数轨道的节点
            ((FloatParameterTrack*)aVar)->mNodes = nullptr;
            break;
        default:
            break;
        }
    }
}
