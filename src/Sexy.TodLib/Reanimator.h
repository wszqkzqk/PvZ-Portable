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

#ifndef __REANIMATION_H__
#define __REANIMATION_H__

#include <cstdint>
#include "DataArray.h"
#include "FilterEffect.h"
#include "misc/SexyMatrix.h"
//using namespace std;
using namespace Sexy;

class Reanimation;
class ReanimAtlas;
class AttacherInfo;
class AttachEffect;
class TodTriangleGroup;
class TodParticleSystem;
class ReanimatorTransform;
class ReanimatorDefinition;
namespace Sexy
{
    class _Font;
    class Image;
    class Graphics;
    class MemoryImage;
};

// ######################################################################################################################################################
// ############################################################### 以下为动画定义相关内容 ###############################################################
// ######################################################################################################################################################

constexpr const float DEFAULT_FIELD_PLACEHOLDER = -10000.0f;
constexpr const double SECONDS_PER_UPDATE = 0.01f;

enum ReanimFlags : int32_t
{
    REANIM_NO_ATLAS,
    REANIM_FAST_DRAW_IN_SW_MODE
};

struct ReanimatorTransformArray {
    ReanimatorTransform* mTransforms;
    int32_t count;
};

class ReanimatorTrack
{
public:
    const char*                     mName;                          //+0x0：轨道名称
    ReanimatorTransformArray        mTransforms;                    //+0x4：每一帧的动画变换的数组
    
public:
    ReanimatorTrack() : mName(""), mTransforms({nullptr,0}) { }
};

struct ReanimatorTrackArray {
    ReanimatorTrack* tracks;
    int32_t count;
};

// ====================================================================================================
// ★ 【动画器定义】
// ----------------------------------------------------------------------------------------------------
// 用于描述一种动画类型与该动画的数据文件的文件名及标志之间的对应关系。
// ====================================================================================================
class ReanimatorDefinition
{
public:
    ReanimatorTrackArray            mTracks;
    float                           mFPS;
    ReanimAtlas*                    mReanimAtlas;

public:
    ReanimatorDefinition() : mTracks({nullptr, 0}), mFPS(12.0f), mReanimAtlas(nullptr) { }
};
extern unsigned int gReanimatorDefCount;
extern ReanimatorDefinition* gReanimatorDefArray;

// ====================================================================================================
// ★ 【动画参数】
// ----------------------------------------------------------------------------------------------------
// 用于描述一种动画类型与该动画的数据文件的文件名及标志之间的对应关系。
// ====================================================================================================
class ReanimationParams
{
public:
    ReanimationType                 mReanimationType;
    const char*                     mReanimFileName;
    int32_t                         mReanimParamFlags;
};
extern unsigned int gReanimationParamArraySize;
extern const ReanimationParams* gReanimationParamArray;

inline void                         ReanimationFillInMissingData(float& thePrev, float& theValue);
inline void                         ReanimationFillInMissingData(void*& thePrev, void*& theValue);
bool                                ReanimationLoadDefinition(const std::string& theFileName, ReanimatorDefinition* theDefinition);
void                                ReanimationFreeDefinition(ReanimatorDefinition* theDefinition);
void                                ReanimatorEnsureDefinitionLoaded(ReanimationType theReanimType, bool theIsPreloading);
void                                ReanimatorLoadDefinitions(const ReanimationParams* theReanimationParamArray, int theReanimationParamArraySize);
void                                ReanimatorFreeDefinitions();

extern const ReanimationParams gLawnReanimationArray[static_cast<int>(ReanimationType::NUM_REANIMS)];

// ######################################################################################################################################################
// ############################################################## 以下正式开始动画相关声明 ##############################################################
// ######################################################################################################################################################

enum
{
    RENDER_GROUP_HIDDEN = -1,
    RENDER_GROUP_NORMAL = 0
};

class ReanimationHolder
{
public:
    DataArray<Reanimation>          mReanimations;

public:
    ReanimationHolder() { ; }
    ~ReanimationHolder();

    void                            InitializeHolder();
    void                            DisposeHolder();
    Reanimation*                    AllocReanimation(float theX, float theY, int theRenderOrder, ReanimationType theReanimationType);
};

// ====================================================================================================
// ★ 【动画器时间】
// ----------------------------------------------------------------------------------------------------
// 用于描述动画当前正在播放的时间位置。
// ====================================================================================================
class ReanimatorFrameTime
{
public:
    float                           mFraction;                      //+0x0：两帧之间已经过的比例
    int32_t                         mAnimFrameBeforeInt;            //+0x4：前一个整数帧
    int32_t                         mAnimFrameAfterInt;             //+0x8：后一个整数帧
};

class ReanimatorTransform
{
public:
    float                           mTransX;
    float                           mTransY;
    float                           mSkewX;
    float                           mSkewY;
    float                           mScaleX;
    float                           mScaleY;
    float                           mFrame;
    float                           mAlpha;
    Image*                          mImage;
    _Font*                           mFont;
    const char*                     mText;

public:
    ReanimatorTransform();
};

class ReanimatorTrackInstance
{
public:
    int32_t                         mBlendCounter;
    int32_t                         mBlendTime;
    ReanimatorTransform             mBlendTransform;
    float                           mShakeOverride;
    float                           mShakeX;
    float                           mShakeY;
    AttachmentID                    mAttachmentID;
    Image*                          mImageOverride;
    int32_t                         mRenderGroup;
    Color                           mTrackColor;
    bool                            mIgnoreClipRect;
    bool                            mTruncateDisappearingFrames;
    bool                            mIgnoreColorOverride;
    bool                            mIgnoreExtraAdditiveColor;

public:
    ReanimatorTrackInstance();
};

class Reanimation
{
public:
    ReanimationType                 mReanimationType;
    float                           mAnimTime;
    float                           mAnimRate;
    ReanimatorDefinition*           mDefinition;
    ReanimLoopType                  mLoopType;
    bool                            mDead;
    int32_t                         mFrameStart;
    int32_t                         mFrameCount;
    int32_t                         mFrameBasePose;
    SexyTransform2D                 mOverlayMatrix;
    Color                           mColorOverride;
    ReanimatorTrackInstance*        mTrackInstances;
    int32_t                         mLoopCount;
    ReanimationHolder*              mReanimationHolder;
    bool                            mIsAttachment;
    int32_t                         mRenderOrder;
    Color                           mExtraAdditiveColor;
    bool                            mEnableExtraAdditiveDraw;
    Color                           mExtraOverlayColor;
    bool                            mEnableExtraOverlayDraw;
    float                           mLastFrameTime;
    FilterEffect                    mFilterEffect;

public:
    Reanimation();
    ~Reanimation();

    void                            ReanimationInitialize(float theX, float theY, ReanimatorDefinition* theDefinition);
    /*inline*/ void                 ReanimationInitializeType(float theX, float theY, ReanimationType theReanimType);
    void                            ReanimationDie();
    void                            Update();
    /*inline*/ void                 Draw(Graphics* g);
    void                            DrawRenderGroup(Graphics* g, int theRenderGroup);
    bool                            DrawTrack(Graphics* g, int theTrackIndex, int theRenderGroup, TodTriangleGroup* theTriangleGroup);
    void                            GetCurrentTransform(int theTrackIndex, ReanimatorTransform* theTransformCurrent);
    void                            GetTransformAtTime(int theTrackIndex, ReanimatorTransform* theTransform, ReanimatorFrameTime* theFrameTime);
    void                            GetFrameTime(ReanimatorFrameTime* theFrameTime);
    int                             FindTrackIndex(const char* theTrackName);
    void                            AttachToAnotherReanimation(Reanimation* theAttachReanim, const char* theTrackName);
    void                            GetAttachmentOverlayMatrix(int theTrackIndex, SexyTransform2D& theOverlayMatrix);
    /*inline*/ void                 SetFramesForLayer(const char* theTrackName);
    static void                     MatrixFromTransform(const ReanimatorTransform& theTransform, SexyMatrix3& theMatrix);
    bool                            TrackExists(const char* theTrackName);
    void                            StartBlend(int theBlendTime);
    /*inline*/ void                 SetShakeOverride(const char* theTrackName, float theShakeAmount);
    /*inline*/ void                 SetPosition(float theX, float theY);
    /*inline*/ void                 OverrideScale(float theScaleX, float theScaleY);
    float                           GetTrackVelocity(const char* theTrackName);
    /*inline*/ void                 SetImageOverride(const char* theTrackName, Image* theImage);
    /*inline*/ Image*               GetImageOverride(const char* theTrackName);
    void                            ShowOnlyTrack(const char* theTrackName);
    void                            GetTrackMatrix(int theTrackIndex, SexyTransform2D& theMatrix);
    void                            AssignRenderGroupToTrack(const char* theTrackName, int theRenderGroup);
    void                            AssignRenderGroupToPrefix(const char* theTrackName, int theRenderGroup);
    void                            PropogateColorToAttachments();
    bool                            ShouldTriggerTimedEvent(float theEventTime);
//  void                            TodTriangleGroupDraw(Graphics* g, TodTriangleGroup* theTriangleGroup) { ; }
    Image*                          GetCurrentTrackImage(const char* theTrackName);
    AttachEffect*                   AttachParticleToTrack(const char* theTrackName, TodParticleSystem* theParticleSystem, float thePosX, float thePosY);
    void                            GetTrackBasePoseMatrix(int theTrackIndex, SexyTransform2D& theBasePosMatrix);
    bool                            IsTrackShowing(const char* theTrackName);
    /*inline*/ void                 SetTruncateDisappearingFrames(const char* theTrackName = nullptr, bool theTruncateDisappearingFrames = false);
    /*inline*/ void                 PlayReanim(const char* theTrackName, ReanimLoopType theLoopType, int theBlendTime, float theAnimRate);
    void                            ReanimationDelete();
    ReanimatorTrackInstance*        GetTrackInstanceByName(const char* theTrackName);
    void                            GetFramesForLayer(const char* theTrackName, int& theFrameStart, int& theFrameCount);
    void                            UpdateAttacherTrack(int theTrackIndex);
    static void                     ParseAttacherTrack(const ReanimatorTransform& theTransform, AttacherInfo& theAttacherInfo);
    void                            AttacherSynchWalkSpeed(int theTrackIndex, Reanimation* theAttachReanim, AttacherInfo& theAttacherInfo);
    /*inline*/ bool                 IsAnimPlaying(const char* theTrackName);
    void                            SetBasePoseFromAnim(const char* theTrackName);
    void                            ReanimBltMatrix(Graphics* g, Image* theImage, SexyMatrix3& theTransform, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect& theSrcRect);
    Reanimation*                    FindSubReanim(ReanimationType theReanimType);
};

void                                ReanimationCreateAtlas(ReanimatorDefinition* theDefinition, ReanimationType theReanimationType);
void                                ReanimationPreload(ReanimationType theReanimationType);
void                                BlendTransform(ReanimatorTransform* theResult, const ReanimatorTransform& theTransform1, const ReanimatorTransform& theTransform2, float theBlendFactor);

#endif
