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

#ifndef __PVZPPARTICLE_H__
#define __PVZPPARTICLE_H__

#include <cstdint>
#include <memory>
#include "PvzpList.h"
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

// Particle system definitions

enum ParticleFlags : int32_t
{
	PARTICLE_RANDOM_LAUNCH_SPIN,        // random initial spin in [0, 2π] at launch
	PARTICLE_ALIGN_LAUNCH_SPIN,         // initial spin aligns with the launch direction (lower priority than random launch spin)
	PARTICLE_ALIGN_TO_PIXELS,           // render position rounded to integer pixels
	PARTICLE_SYSTEM_LOOPS,              // emitter restarts its cycle when its lifetime ends
	PARTICLE_PARTICLE_LOOPS,            // particle restarts its cycle when its lifetime ends
	PARTICLE_PARTICLES_DONT_FOLLOW,     // launched particles don't move with the emitter
	PARTICLE_RANDOM_START_TIME,         // particle starts at a random age within its duration
	PARTICLE_DIE_IF_OVERLOADED,         // the particle system fails to be created when overloaded
	PARTICLE_ADDITIVE,                  // always rendered in additive mode
	PARTICLE_FULLSCREEN,                // rendered as a screen-filling rect
	PARTICLE_SOFTWARE_ONLY,             // rendered only without 3D acceleration
	PARTICLE_HARDWARE_ONLY              // rendered only with 3D acceleration
};

enum ParticleFieldType : int32_t
{
	FIELD_INVALID,
	FIELD_FRICTION,                     // friction: velocity decays proportionally
	FIELD_ACCELERATION,                 // acceleration: velocity increases by a fixed acceleration
	FIELD_ATTRACTOR,                    // attractor: acceleration depends on the distance to the emitter
	FIELD_MAX_VELOCITY,                 // velocity magnitude clamped to a limit
	FIELD_VELOCITY,                     // constant drift velocity
	FIELD_POSITION,                     // position pinned to a given value
	FIELD_SYSTEM_POSITION,              // emitter only: emitter position pinned to a given value
	FIELD_GROUND_CONSTRAINT,            // vertical position can't go below the ground; bounces on contact
	FIELD_SHAKE,                        // shake: random -1.0..+1.0 position offset each frame
	FIELD_CIRCLE,                       // circle: orbits the emitter center (a spiral in practice due to error)
	FIELD_AWAY,                         // away: moves radially away from the emitter center
	PARTICLE_FIELD_COUNT
};  // see PvzpParticleEmitter::UpdateParticleField() (UpdateSystemField() for FIELD_SYSTEM_POSITION)

// Correspondence between the definition types and the XML format:
// <Emitter>                                                         —
//                       ↓←Node→↓                                ↑
//     <SpawnRate>[.5 2] [2.5 4.5],40 [5 10]</SpawnRate>    PvzpEmitterDefinition
//     ↑←      FloatParameterTrack::mNodes        →↑             ↓
// </Emitter>                                                        —
// Definition items omitted in the XML get their default values after loading

// Each node describes one stage of a value's change over time
class FloatParameterTrackNode
{
public:
	float                       mTime;                          // start time of the stage
	float                       mLowValue;                      // minimum value allowed in the stage
	float                       mHighValue;                     // maximum value allowed in the stage
	PvzpCurves                   mCurveType;                     // easing curve of the transition to the next stage
	PvzpCurves                   mDistribution;                  // probability distribution between min and max in the stage
};

// Each track describes how one emitter attribute varies over time and its allowed range
class FloatParameterTrack
{
public:
	FloatParameterTrackNode*    mNodes;
	int32_t                     mCountNodes;
};

// Physical environment for particle motion; up to 4 fields can be stacked
class ParticleField
{
public:
	ParticleFieldType           mFieldType;                     // field type, determines how the field affects particle motion
	FloatParameterTrack         mX;                             // field effect along the X axis
	FloatParameterTrack         mY;                             // field effect along the Y axis
};

struct EmitterFieldArray
{
public:
	ParticleField* Fields;
	int32_t count;
};

// Describes the variation and range of a particle emitter's behavior parameters
class PvzpEmitterDefinition
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

// The set of emitter definitions that make up a particle system
class PvzpParticleDefinition
{
public:
	PvzpEmitterDefinition*       mEmitterDefs;
	int32_t                     mEmitterDefCount;
};

extern int gParticleDefCount;
extern std::unique_ptr<PvzpParticleDefinition[]> gParticleDefArray;    // loaded and assigned in LawnApp::LoadingThreadProc()

// Maps a particle system type to the file name of its data file
class ParticleParams
{
public:
	ParticleEffect              mParticleEffect;
	const char*                 mParticleFileName;
};
extern int gParticleParamArraySize;
extern const ParticleParams* gParticleParamArray;

bool                            PvzpParticleLoadADef(PvzpParticleDefinition* theParticleDef, const char* theParticleFileName);
void                            PvzpParticleLoadDefinitions(const ParticleParams* theParticleParamArray, int theParticleParamArraySize);
void                            PvzpParticleFreeDefinitions();

extern const ParticleParams gLawnParticleArray[static_cast<int>(ParticleEffect::NUM_PARTICLES)];  // 0x6A0FF0

// Particle system declarations

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

class PvzpParticleSystem;
class PvzpParticleEmitter;
class PvzpParticle;
class PvzpParticleHolder
{
public:
	DataArray<PvzpParticleSystem>	mParticleSystems;
	DataArray<PvzpParticleEmitter>	mEmitters;
	DataArray<PvzpParticle>			mParticles;
	PvzpAllocator					mParticleListNodeAllocator;
	PvzpAllocator					mEmitterListNodeAllocator;

public:
	~PvzpParticleHolder();

	void							InitializeHolder();
	void							DisposeHolder();
	PvzpParticleSystem*				AllocParticleSystemFromDef(float theX, float theY, int theRenderOrder, PvzpParticleDefinition* theDefinition, ParticleEffect theParticleEffect);
	PvzpParticleSystem*				AllocParticleSystem(float theX, float theY, int theRenderOrder, ParticleEffect theParticleEffect);
	bool					IsOverLoaded();
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

class PvzpParticle
{
public:
	PvzpParticleEmitter*				mParticleEmitter;
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

class PvzpTriangleGroup;
class PvzpParticleEmitter
{
public:
	PvzpEmitterDefinition*			mEmitterDef;
	PvzpParticleSystem*				mParticleSystem;
	PvzpList<ParticleID>				mParticleList;
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
	void							PvzpEmitterInitialize(float theX, float theY, PvzpParticleSystem* theSystem, PvzpEmitterDefinition* theEmitterDef);
	void							Update();
	void							Draw(Graphics* g);
	void							SystemMove(float theX, float theY);
	static bool						GetRenderParams(PvzpParticle* theParticle, ParticleRenderParams* theParams);
	void							DrawParticle(Graphics* g, PvzpParticle* theParticle, PvzpTriangleGroup* theTriangleGroup);
	void							UpdateSpawning();
	bool							UpdateParticle(PvzpParticle* theParticle);
	PvzpParticle*					SpawnParticle(int theIndex, int theSpawnCount);
	bool							CrossFadeParticle(PvzpParticle* theParticle, PvzpParticleEmitter* theToEmitter);
	void							CrossFadeEmitter(PvzpParticleEmitter* theToEmitter);
	bool							CrossFadeParticleToName(PvzpParticle* theParticle, const char* theEmitterName);
	void							DeleteAll();
	void							UpdateParticleField(PvzpParticle* theParticle, ParticleField* theParticleField, float theParticleTimeValue, int theFieldIndex);
	void							UpdateSystemField(ParticleField* theParticleField, float theParticleTimeValue, int theFieldIndex);
	float				SystemTrackEvaluate(FloatParameterTrack& theTrack, ParticleSystemTracks theSystemTrack);
	static float			ParticleTrackEvaluate(FloatParameterTrack& theTrack, PvzpParticle* theParticle, ParticleTracks theParticleTrack);
	void							DeleteParticle(PvzpParticle* theParticle);
	void							DeleteNonCrossFading();
};
float                    CrossFadeLerp(float theFrom, float theTo, bool theFromIsSet, bool theToIsSet, float theFraction);
void								RenderParticle(Graphics* g, PvzpParticle* theParticle, const Color& theColor, ParticleRenderParams* theParams, PvzpTriangleGroup* theTriangleGroup);

class PvzpParticleSystem
{
public:
	ParticleEffect					mEffectType;
	PvzpParticleDefinition*			mParticleDef;
	PvzpParticleHolder*				mParticleHolder;
	PvzpList<ParticleEmitterID>		mEmitterList;
	bool							mDead;
	bool							mIsAttachment;
	int32_t							mRenderOrder;
	bool							mDontUpdate;

public:
	PvzpParticleSystem();
	~PvzpParticleSystem();

	void							PvzpParticleInitializeFromDef(float theX, float theY, int theRenderOrder, PvzpParticleDefinition* theDefinition, ParticleEffect theEffectType);
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
	PvzpParticleEmitter*				FindEmitterByName(const char* theEmitterName);
	PvzpEmitterDefinition*			FindEmitterDefByName(const char* theEmitterName);
};

#endif
