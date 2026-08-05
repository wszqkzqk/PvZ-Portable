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

#ifndef __ATTACHMENT_H__
#define __ATTACHMENT_H__

#include <cstdint>
#include "../ConstEnums.h"
#include "DataArray.h"
#include "misc/SexyMatrix.h"

namespace Sexy
{
	class Graphics;
}
using namespace Sexy;

#define MAX_EFFECTS_PER_ATTACHMENT 16

class Trail;
class Reanimation;
class PvzpParticleSystem;

class AttachEffect
{
public:
	unsigned int            mEffectID;
	EffectType              mEffectType;
	SexyTransform2D         mOffset;
	bool                    mDontDrawIfParentHidden;
	bool                    mDontPropogateColor;
};

class AttacherInfo
{
public:
	std::string             mReanimName;
	std::string             mTrackName;
	float                   mAnimRate;
	ReanimLoopType          mLoopType;

public:
	~AttacherInfo() { ; }
};

class Attachment
{
public:
	AttachEffect            mEffectArray[MAX_EFFECTS_PER_ATTACHMENT];
	int32_t                 mNumEffects;
	bool                    mDead;

public:
	Attachment();
	~Attachment();

	void                    Update();
	void                    SetPosition(const SexyVector2& thePosition);
	void                    SetMatrix(const SexyTransform2D& theMatrix);
	void                    OverrideColor(const Color& theColor);
	void                    OverrideScale(float theScale);
	void                    Draw(Graphics* g, bool theParentHidden);
	void                    AttachmentDie();
	void                    Detach();
	void                    CrossFade(const char* theCrossFadeName);
	void                    PropogateColor(const Color& theColor, bool theEnableAdditiveColor, const Color& theAdditiveColor, bool theEnableOverlayColor, const Color& theOverlayColor);
};

AttachEffect*               AttachReanim(AttachmentID& theAttachmentID, Reanimation* theReanimation, float theOffsetX, float theOffsetY);
AttachEffect*               AttachParticle(AttachmentID& theAttachmentID, PvzpParticleSystem* theParticleSystem, float theOffsetX, float theOffsetY);
AttachEffect*               AttachTrail(AttachmentID& theAttachmentID, Trail* theTrail, float theOffsetX, float theOffsetY);
void             AttachmentPropogateColor(AttachmentID& theAttachmentID, const Color& theColor, bool theEnableAdditiveColor, const Color& theAdditiveColor, bool theEnableOverlayColor, const Color& theOverlayColor);
void             AttachmentOverrideColor(AttachmentID& theAttachmentID, const Color& theColor);
void             AttachmentOverrideScale(AttachmentID& theAttachmentID, float theScale);
void             AttachmentUpdateAndMove(AttachmentID& theAttachmentID, float theX, float theY);
void             AttachmentUpdateAndSetMatrix(AttachmentID& theAttachmentID, SexyTransform2D& theMatrix);
void             AttachmentDraw(AttachmentID& theAttachmentID, Graphics* g, bool theParentHidden);
void             AttachmentDetach(AttachmentID& theAttachmentID);
void             AttachmentDetachCrossFadeParticleType(AttachmentID& theAttachmentID, ParticleEffect theParticleEffect, const char* theCrossFadeName);
void             AttachmentReanimTypeDie(AttachmentID& theAttachmentID, ReanimationType theReanimType);
void             AttachmentDie(AttachmentID& theAttachmentID);
void             AttachmentCrossFade(AttachmentID& theAttachmentID, const char* theCrossFadeName);
AttachEffect*               FindFirstAttachment(AttachmentID& theAttachmentID);
Reanimation*                FindReanimAttachment(AttachmentID& theAttachmentID);
AttachEffect*               CreateEffectAttachment(AttachmentID& theAttachmentID, EffectType theEffectType, unsigned int theDataID, float theOffsetX, float theOffsetY);
bool             IsFullOfAttachments(AttachmentID& theAttachmentID);

class AttachmentHolder
{
public:
	DataArray<Attachment>   mAttachments;

public:
	AttachmentHolder();
	~AttachmentHolder();

	void                    InitializeHolder();
	void                    DisposeHolder();
	Attachment*             AllocAttachment();
};

#endif
