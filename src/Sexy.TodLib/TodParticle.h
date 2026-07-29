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

#ifndef __TODPARTICLE_H__
#define __TODPARTICLE_H__

#include <cstdint>
#include "TodList.h"
#include "DataArray.h"
#include "misc/SexyVector.h"
namespace Sexy
{
	class Image;
	class Graphics;
};
//using namespace std;
using namespace Sexy;

#define MAX_PARTICLES_SIZE 900
#define MAX_PARTICLE_FIELDS 4

// ######################################################################################################################################################
// ############################################################# 以下为粒子系统定义相关内容 #############################################################
// ######################################################################################################################################################

enum ParticleFlags : int32_t
{
    PARTICLE_RANDOM_LAUNCH_SPIN,        // 随机发射旋转，指定粒子在发射时使用 [0, 2π] 内随机的初始旋转角度
    PARTICLE_ALIGN_LAUNCH_SPIN,         // 对齐发射角度，指定粒子在发射时的初始旋转角度与发射方向一致（优先级低于随机发射旋转）
    PARTICLE_ALIGN_TO_PIXELS,           // 对齐至像素，指定粒子渲染时的坐标四舍五入地对齐至整数像素值
    PARTICLE_SYSTEM_LOOPS,              // 系统循环，指定发射器在生命周期结束后立即回到周期起始的时刻
    PARTICLE_PARTICLE_LOOPS,            // 粒子循环，指定粒子在生命周期结束后立即回到周期起始的时刻
    PARTICLE_PARTICLES_DONT_FOLLOW,     // 粒子不跟随，指定当粒子发射器的位置移动时，已发射的粒子不随之移动
    PARTICLE_RANDOM_START_TIME,         // 随机初始时刻，指定粒子初始时的存在时间为 0 至最大持续时间之间的随机值
    PARTICLE_DIE_IF_OVERLOADED,         // 过载限制，指定在粒子数量过多时，该发射器所属的粒子系统无法被创建
    PARTICLE_ADDITIVE,                  // 叠加模式，指定粒子渲染时固定使用叠加（Additive）模式
    PARTICLE_FULLSCREEN,                // 全屏模式，指定粒子在渲染时改为填充一个屏幕大小的矩形
    PARTICLE_SOFTWARE_ONLY,             // 仅软件渲染，指定粒子仅在未开启 3D 加速时可被渲染
    PARTICLE_HARDWARE_ONLY              // 仅硬件渲染，指定粒子仅在已开启 3D 加速时可被渲染
};

enum ParticleFieldType : int32_t
{
    FIELD_INVALID,
    FIELD_FRICTION,                     // 摩擦力场：该场内粒子的速度按一定比例不断衰减
    FIELD_ACCELERATION,                 // 加速度场：该场内粒子以一定加速度做加速运动
    FIELD_ATTRACTOR,                    // 弹性力场：该场内粒子加速度的大小和方向均受到粒子与发射器之间距离大小的影响
    FIELD_MAX_VELOCITY,                 // 限速场：该场内粒子速度的大小不能超过一定的上限值
    FIELD_VELOCITY,                     // 匀速场：该场内粒子速度的大小总是为一给定值
    FIELD_POSITION,                     // 定位场：该场内粒子的位置总是为一给定值
    FIELD_SYSTEM_POSITION,              // 系统定位场：仅发射器可用，该场内发射器的位置总是为一给定值
    FIELD_GROUND_CONSTRAINT,            // 地面限制：粒子的纵向位置不能低于地面，且接触地面时会触发弹起效果
    FIELD_SHAKE,                        // 震动：粒子的坐标会有随机 -1.0 到 +1.0 的偏移，每帧独立计算
    FIELD_CIRCLE,                       // 引力场：该场内粒子围绕发射器中心做圆周运动（考虑误差，实为螺线运动）
    FIELD_AWAY,                         // 斥力场：该场内粒子沿径向不断远离发射器中心
    PARTICLE_FIELD_COUNT
};  // 粒子场相关内容详见 TodParticleEmitter::UpdateParticleField() 函数（系统定位场为 TodParticleEmitter::UpdateSystemField() 函数）

// ################################################################################
// ▲ 以下粒子系统定义相关的各类型的实例与 XML 中的格式的对应关系示例简图
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// <Emitter>                                                         —
//                       ↓←Node→↓                                ↑
//     <SpawnRate>[.5 2] [2.5 4.5],40 [5 10]</SpawnRate>    TodEmitterDefinition
//     ↑←      FloatParameterTrack::mNodes        →↑             ↓
// </Emitter>                                                        —
// ################################################################################
// 【!】XML 中没有给出的定义项，将会在读取结束后被赋予该项的默认值

// ====================================================================================================
// ★ 【浮点参数轨道节点】
// ----------------------------------------------------------------------------------------------------
// 每个节点描述属性数值随时间变化的一个阶段。
// ====================================================================================================
class FloatParameterTrackNode
{
public:
    float                       mTime;                          // 阶段的起始时间
    float                       mLowValue;                      // 阶段内数据允许的最小值
    float                       mHighValue;                     // 阶段内数据允许的最大值
    TodCurves                   mCurveType;                     // 从当前阶段过渡至下一阶段的缓动效果曲线
    TodCurves                   mDistribution;                  // 阶段内数据在最小值和最大值之间的概率分布曲线
};

// ====================================================================================================
// ★ 【浮点参数轨道】
// ----------------------------------------------------------------------------------------------------
// 每条轨道描述发射器的一种属性的数值随时间的变化规律和取值范围。
// ====================================================================================================
class FloatParameterTrack
{
public:
    FloatParameterTrackNode*    mNodes;
    int32_t                     mCountNodes;
};

// ====================================================================================================
// ★ 【粒子场】
// ----------------------------------------------------------------------------------------------------
// 发射器中粒子运动的物理环境。多个粒子场可以叠加存在，最多叠加 4 个。
// ====================================================================================================
class ParticleField
{
public:
    ParticleFieldType           mFieldType;                     // 场的类型，决定了场对粒子运动的影响方式
    FloatParameterTrack         mX;                             // 场在水平方向（横向）上对粒子的影响
    FloatParameterTrack         mY;                             // 场在竖直方向（纵向）上对粒子的影响
};

struct EmitterFieldArray
{
public:
	ParticleField* Fields;
	int32_t count;
};

// ====================================================================================================
// ★ 【发射器定义】
// ----------------------------------------------------------------------------------------------------
// 粒子发射器的定义数据描述了其各种行为的参数的变化规律和范围。
// ====================================================================================================
class TodEmitterDefinition
{
public:
    Image*                      mImage;
    int32_t                     mImageCol;
    int32_t                     mImageRow;
    int32_t                     mImageFrames;
    int32_t                     mAnimated;
    int32_t                     mParticleFlags;
    EmitterType                 mEmitterType;
    const char*                 mName;
    const char*                 mOnDuration;
    FloatParameterTrack         mSystemDuration;
    FloatParameterTrack         mCrossFadeDuration;
    FloatParameterTrack         mSpawnRate;
    FloatParameterTrack         mSpawnMinActive;
    FloatParameterTrack         mSpawnMaxActive;
    FloatParameterTrack         mSpawnMaxLaunched;
    FloatParameterTrack         mEmitterRadius;
    FloatParameterTrack         mEmitterOffsetX;
    FloatParameterTrack         mEmitterOffsetY;
    FloatParameterTrack         mEmitterBoxX;
    FloatParameterTrack         mEmitterBoxY;
    FloatParameterTrack         mEmitterSkewX;
    FloatParameterTrack         mEmitterSkewY;
    FloatParameterTrack         mEmitterPath;
    FloatParameterTrack         mParticleDuration;
    FloatParameterTrack         mLaunchSpeed;
    FloatParameterTrack         mLaunchAngle;
    FloatParameterTrack         mSystemRed;
    FloatParameterTrack         mSystemGreen;
    FloatParameterTrack         mSystemBlue;
    FloatParameterTrack         mSystemAlpha;
    FloatParameterTrack         mSystemBrightness;
    EmitterFieldArray           mParticleFields;
    EmitterFieldArray           mSystemFields;
    FloatParameterTrack         mParticleRed;
    FloatParameterTrack         mParticleGreen;
    FloatParameterTrack         mParticleBlue;
    FloatParameterTrack         mParticleAlpha;
    FloatParameterTrack         mParticleBrightness;
    FloatParameterTrack         mParticleSpinAngle;
    FloatParameterTrack         mParticleSpinSpeed;
    FloatParameterTrack         mParticleScale;
    FloatParameterTrack         mParticleStretch;
    FloatParameterTrack         mCollisionReflect;
    FloatParameterTrack         mCollisionSpin;
    FloatParameterTrack         mClipTop;
    FloatParameterTrack         mClipBottom;
    FloatParameterTrack         mClipLeft;
    FloatParameterTrack         mClipRight;
    FloatParameterTrack         mAnimationRate;
};

// ====================================================================================================
// ★ 【粒子系统定义】
// ----------------------------------------------------------------------------------------------------
// 粒子系统的定义数据，是粒子系统中各个粒子发射器的定义数据的集合。
// ====================================================================================================
class TodParticleDefinition
{
public:
    TodEmitterDefinition*       mEmitterDefs;
    int32_t                     mEmitterDefCount;
};

extern int gParticleDefCount;
extern TodParticleDefinition* gParticleDefArray;    // 由 LawnApp::LoadingThreadProc() 函数中读取并赋值

// ====================================================================================================
// ★ 【粒子参数】
// ----------------------------------------------------------------------------------------------------
// 用于描述一种粒子系统类型与该粒子系统的数据文件的文件名之间的对应关系。
// ====================================================================================================
class ParticleParams
{
public:
    ParticleEffect              mParticleEffect;
    const char*                 mParticleFileName;
};
extern int gParticleParamArraySize;
extern const ParticleParams* gParticleParamArray;

bool                            TodParticleLoadADef(TodParticleDefinition* theParticleDef, const char* theParticleFileName);
void                            TodParticleLoadDefinitions(const ParticleParams* theParticleParamArray, int theParticleParamArraySize);
void                            TodParticleFreeDefinitions();

extern const ParticleParams gLawnParticleArray[static_cast<int>(ParticleEffect::NUM_PARTICLES)];

// ######################################################################################################################################################
// ############################################################ 以下正式开始粒子系统相关声明 ############################################################
// ######################################################################################################################################################

enum ParticleSystemTracks : int32_t
{
	TRACK_SPAWN_RATE,
	TRACK_SPAWN_MIN_ACTIVE,
	TRACK_SPAWN_MAX_ACTIVE,
	TRACK_SPAWN_MAX_LAUNCHED,
	TRACK_EMITTER_PATH,
	TRACK_SYSTEM_RED,
	TRACK_SYSTEM_GREEN,
	TRACK_SYSTEM_BLUE,
	TRACK_SYSTEM_ALPHA,
	TRACK_SYSTEM_BRIGHTNESS,
	NUM_SYSTEM_TRACKS
};

enum ParticleTracks : int32_t
{
	TRACK_PARTICLE_RED,
	TRACK_PARTICLE_GREEN,
	TRACK_PARTICLE_BLUE,
	TRACK_PARTICLE_ALPHA,
	TRACK_PARTICLE_BRIGHTNESS,
	TRACK_PARTICLE_SPIN_SPEED,
	TRACK_PARTICLE_SPIN_ANGLE,
	TRACK_PARTICLE_SCALE,
	TRACK_PARTICLE_STRETCH,
	TRACK_PARTICLE_COLLISION_REFLECT,
	TRACK_PARTICLE_COLLISION_SPIN,
	TRACK_PARTICLE_CLIP_TOP,
	TRACK_PARTICLE_CLIP_BOTTOM,
	TRACK_PARTICLE_CLIP_LEFT,
	TRACK_PARTICLE_CLIP_RIGHT,
	TRACK_PARTICLE_ANIMATION_RATE,
	NUM_PARTICLE_TRACKS
};

class TodParticleSystem;
class TodParticleEmitter;
class TodParticle;
class TodParticleHolder
{
public:
	DataArray<TodParticleSystem>	mParticleSystems;
	DataArray<TodParticleEmitter>	mEmitters;
	DataArray<TodParticle>			mParticles;
	TodAllocator					mParticleListNodeAllocator;
	TodAllocator					mEmitterListNodeAllocator;

public:
    ~TodParticleHolder();

    void							InitializeHolder();
    void							DisposeHolder();
    TodParticleSystem*				AllocParticleSystemFromDef(float theX, float theY, int theRenderOrder, TodParticleDefinition* theDefinition, ParticleEffect theParticleEffect);
    TodParticleSystem*				AllocParticleSystem(float theX, float theY, int theRenderOrder, ParticleEffect theParticleEffect);
    bool							IsOverLoaded();
};

class ParticleRenderParams
{
public:
	bool							mRedIsSet;
	bool							mGreenIsSet;
	bool							mBlueIsSet;
	bool							mAlphaIsSet;
	bool							mParticleScaleIsSet;
	bool							mParticleStretchIsSet;
	bool							mSpinPositionIsSet;
	bool							mPositionIsSet;
	float							mRed;
	float							mGreen;
	float							mBlue;
	float							mAlpha;
	float							mParticleScale;
	float							mParticleStretch;
	float							mSpinPosition;
	float							mPosX;
	float							mPosY;
};

class TodParticle
{
public:
	TodParticleEmitter*				mParticleEmitter;
	int32_t							mParticleDuration;
	int32_t							mParticleAge;
	float							mParticleTimeValue;
	float							mParticleLastTimeValue;
	float							mAnimationTimeValue;
	SexyVector2						mVelocity;
	SexyVector2						mPosition;
	int32_t							mImageFrame;
	float							mSpinPosition;
	float							mSpinVelocity;
	ParticleID						mCrossFadeParticleID;
	int32_t							mCrossFadeDuration;
	float							mParticleInterp[ParticleTracks::NUM_PARTICLE_TRACKS];
	float							mParticleFieldInterp[MAX_PARTICLE_FIELDS][2];
};

class TodTriangleGroup;
class TodParticleEmitter
{
public:
	TodEmitterDefinition*			mEmitterDef;
	TodParticleSystem*				mParticleSystem;
	TodList<ParticleID>				mParticleList;
	float							mSpawnAccum;
	Sexy::SexyVector2				mSystemCenter;
	int32_t							mParticlesSpawned;
	int32_t							mSystemAge;
	int32_t							mSystemDuration;
	float							mSystemTimeValue;
	float							mSystemLastTimeValue;
	bool							mDead;
	Sexy::Color						mColorOverride;
	bool							mExtraAdditiveDrawOverride;
	float							mScaleOverride;
	Sexy::Image*					mImageOverride;
	ParticleEmitterID				mCrossFadeEmitterID;
	int32_t							mEmitterCrossFadeCountDown;
	int32_t							mFrameOverride;
	float							mTrackInterp[ParticleSystemTracks::NUM_SYSTEM_TRACKS];
	float							mSystemFieldInterp[MAX_PARTICLE_FIELDS][2];

public:
	void							TodEmitterInitialize(float theX, float theY, TodParticleSystem* theSystem, TodEmitterDefinition* theEmitterDef);
	void							Update();
	void							Draw(Graphics* g);
	void							SystemMove(float theX, float theY);
	static bool						GetRenderParams(TodParticle* theParticle, ParticleRenderParams* theParams);
	void							DrawParticle(Graphics* g, TodParticle* theParticle, TodTriangleGroup* theTriangleGroup);
	void							UpdateSpawning();
	bool							UpdateParticle(TodParticle* theParticle);
	TodParticle*					SpawnParticle(int theIndex, int theSpawnCount);
	bool							CrossFadeParticle(TodParticle* theParticle, TodParticleEmitter* theToEmitter);
	void							CrossFadeEmitter(TodParticleEmitter* theToEmitter);
	bool							CrossFadeParticleToName(TodParticle* theParticle, const char* theEmitterName);
	void							DeleteAll();
	void							UpdateParticleField(TodParticle* theParticle, ParticleField* theParticleField, float theParticleTimeValue, int theFieldIndex);
	void							UpdateSystemField(ParticleField* theParticleField, float theParticleTimeValue, int theFieldIndex);
    float							SystemTrackEvaluate(FloatParameterTrack& theTrack, ParticleSystemTracks theSystemTrack);
	static float					ParticleTrackEvaluate(FloatParameterTrack& theTrack, TodParticle* theParticle, ParticleTracks theParticleTrack);
	void							DeleteParticle(TodParticle* theParticle);
	void							DeleteNonCrossFading();
};
float                               CrossFadeLerp(float theFrom, float theTo, bool theFromIsSet, bool theToIsSet, float theFraction);
void								RenderParticle(Graphics* g, TodParticle* theParticle, const Color& theColor, ParticleRenderParams* theParams, TodTriangleGroup* theTriangleGroup);

class TodParticleSystem
{
public:
	ParticleEffect					mEffectType;
    TodParticleDefinition*			mParticleDef;
    TodParticleHolder*				mParticleHolder;
    TodList<ParticleEmitterID>		mEmitterList;
    bool							mDead;
    bool							mIsAttachment;
    int32_t							mRenderOrder;
    bool							mDontUpdate;

public:
    TodParticleSystem();
    ~TodParticleSystem();

    void							TodParticleInitializeFromDef(float theX, float theY, int theRenderOrder, TodParticleDefinition* theDefinition, ParticleEffect theEffectType);
    void							ParticleSystemDie();
    void							Update();
    void							Draw(Graphics* g);
    void							SystemMove(float theX, float theY);
    void							OverrideColor(const char* theEmitterName, const Color& theColor);
    void							OverrideExtraAdditiveDraw(const char* theEmitterName, bool theEnableExtraAdditiveDraw);
    void							OverrideImage(const char* theEmitterName, Image* theImage);
    void							OverrideFrame(const char* theEmitterName, int theFrame);
    void							OverrideScale(const char* theEmitterName, float theScale);
    void							CrossFade(const char* theEmitterName);
    TodParticleEmitter*				FindEmitterByName(const char* theEmitterName);
    TodEmitterDefinition*			FindEmitterDefByName(const char* theEmitterName);
};

#endif
