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

#include "Trail.h"
#include "PvzpDebug.h"
#include "Attachment.h"
#include "Reanimator.h"
#include "PvzpParticle.h"
#include "EffectSystem.h"
#include "graphics/Graphics.h"

static void PruneDeadEffects(Attachment* theAttachment)
{
	PVZP_ASSERT(gEffectSystem);
	PVZP_ASSERT(theAttachment);
	DataArray<PvzpParticleSystem>& aParticleSystems = gEffectSystem->mParticleHolder->mParticleSystems;
	DataArray<Trail>& aTrails = gEffectSystem->mTrailHolder->mTrails;
	DataArray<Reanimation>& aReanimations = gEffectSystem->mReanimationHolder->mReanimations;
	DataArray<Attachment>& aAttachments = gEffectSystem->mAttachmentHolder->mAttachments;

	for (int i = 0; i < theAttachment->mNumEffects;)
	{
		AttachEffect* aAttachEffect = &theAttachment->mEffectArray[i];
		bool aStillAlive = false;
		switch (aAttachEffect->mEffectType)
		{
		case EffectType::EFFECT_PARTICLE:
		{
			PvzpParticleSystem* aParticleSystem = aParticleSystems.DataArrayTryToGet(aAttachEffect->mEffectID);
			aStillAlive = (aParticleSystem != nullptr && !aParticleSystem->mDead);
			break;
		}
		case EffectType::EFFECT_TRAIL:
		{
			Trail* aTrail = aTrails.DataArrayTryToGet(aAttachEffect->mEffectID);
			aStillAlive = (aTrail != nullptr && !aTrail->mDead);
			break;
		}
		case EffectType::EFFECT_REANIM:
		{
			Reanimation* aReanimation = aReanimations.DataArrayTryToGet(aAttachEffect->mEffectID);
			aStillAlive = (aReanimation != nullptr && !aReanimation->mDead);
			break;
		}
		case EffectType::EFFECT_ATTACHMENT:
		{
			Attachment* aAttachment = aAttachments.DataArrayTryToGet(aAttachEffect->mEffectID);
			aStillAlive = (aAttachment != nullptr && !aAttachment->mDead);
			break;
		}
		case EffectType::EFFECT_OTHER:
			aStillAlive = true;
			break;
		default:
			PVZP_ASSERT(false);
			break;
		}

		if (!aStillAlive)
		{
			int aNumEffectsRemaining = theAttachment->mNumEffects - i - 1;
			if (aNumEffectsRemaining > 0)
				memmove(aAttachEffect, aAttachEffect + 1, aNumEffectsRemaining * sizeof(AttachEffect));
			theAttachment->mNumEffects--;
			continue;
		}

		i++;
	}

	if (theAttachment->mNumEffects == 0)
		theAttachment->mDead = true;
}

Attachment::Attachment()
{
	mNumEffects = 0;
	mDead = false;
}

Attachment::~Attachment()
{
	AttachmentDie();
}

void Attachment::Update()
{
	PVZP_ASSERT(gEffectSystem);

	for (int i = 0; i < mNumEffects; i++)
	{
		AttachEffect* aAttachEffect = &mEffectArray[i];
		bool isEmpty = true;
		switch (aAttachEffect->mEffectType)
		{
		case EffectType::EFFECT_PARTICLE:
		{
			PvzpParticleSystem* aParticleSystem = gEffectSystem->mParticleHolder->mParticleSystems.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aParticleSystem && !aParticleSystem->mDead)
			{
				aParticleSystem->Update();
				isEmpty = false;
			}
			break;
		}

		case EffectType::EFFECT_TRAIL:
		{
			Trail* aTrail = gEffectSystem->mTrailHolder->mTrails.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aTrail && !aTrail->mDead)
			{
				aTrail->Update();
				isEmpty = false;
			}
			break;
		}

		case EffectType::EFFECT_REANIM:
		{
			Reanimation* aReanimation = gEffectSystem->mReanimationHolder->mReanimations.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aReanimation && !aReanimation->mDead)
			{
				aReanimation->Update();
				isEmpty = false;
			}
			break;
		}

		case EffectType::EFFECT_ATTACHMENT:
		{
			Attachment* aAttachment = gEffectSystem->mAttachmentHolder->mAttachments.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aAttachment)
			{
				aAttachment->Update();
				isEmpty = false;
			}
			break;
		}

		default:
			PVZP_ASSERT(false);
			break;
		}

		if (isEmpty)
		{
			int aNumEffectsRemaining = mNumEffects - i - 1;
			if (aNumEffectsRemaining > 0)
			{
				memmove(aAttachEffect, aAttachEffect + 1, aNumEffectsRemaining * sizeof(AttachEffect));
				--i;
			}
			mNumEffects--;
		}
	}

	if (mNumEffects == 0)
	{
		mDead = true;
	}
}

void Attachment::SetPosition(const SexyVector2& thePosition)
{
	PVZP_ASSERT(gEffectSystem);

	for (int i = 0; i < mNumEffects; i++)
	{
		AttachEffect* aAttachEffect = &mEffectArray[i];
		SexyVector2 aNewPos = aAttachEffect->mOffset * thePosition;

		switch (aAttachEffect->mEffectType)
		{
		case EffectType::EFFECT_PARTICLE:
		{
			PvzpParticleSystem* aParticleSystem = gEffectSystem->mParticleHolder->mParticleSystems.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aParticleSystem)
			{
				aParticleSystem->SystemMove(aNewPos.x, aNewPos.y);
			}
			break;
		}

		case EffectType::EFFECT_TRAIL:
		{
			Trail* aTrail = gEffectSystem->mTrailHolder->mTrails.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aTrail)
			{
				aTrail->AddPoint(aNewPos.x, aNewPos.y);
			}
			break;
		}

		case EffectType::EFFECT_REANIM:
		{
			Reanimation* aReanimation = gEffectSystem->mReanimationHolder->mReanimations.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aReanimation)
			{
				aReanimation->SetPosition(aNewPos.x, aNewPos.y);
			}
			break;
		}

		case EffectType::EFFECT_ATTACHMENT:
		{
			Attachment* aAttachment = gEffectSystem->mAttachmentHolder->mAttachments.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aAttachment)
			{
				aAttachment->SetPosition(aNewPos);
			}
			break;
		}

		case EffectType::EFFECT_OTHER:
			break;
		}
	}
}

void Attachment::OverrideColor(const Color& theColor)
{
	PVZP_ASSERT(gEffectSystem);

	for (int i = 0; i < mNumEffects; i++)
	{
		AttachEffect* aAttachEffect = &mEffectArray[i];
		switch (aAttachEffect->mEffectType)
		{
		case EffectType::EFFECT_PARTICLE:
		{
			PvzpParticleSystem* aParticleSystem = gEffectSystem->mParticleHolder->mParticleSystems.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aParticleSystem)
			{
				aParticleSystem->OverrideColor("", theColor);
			}
			break;
		}

		case EffectType::EFFECT_REANIM:
		{
			Reanimation* aReanimation = gEffectSystem->mReanimationHolder->mReanimations.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aReanimation)
			{
				aReanimation->mColorOverride = theColor;
			}
			break;
		}

		case EffectType::EFFECT_ATTACHMENT:
		{
			Attachment* aAttachment = gEffectSystem->mAttachmentHolder->mAttachments.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aAttachment)
			{
				aAttachment->OverrideColor(theColor);
			}
			break;
		}
		case EffectType::EFFECT_TRAIL:
		case EffectType::EFFECT_OTHER:
			break;
		}
	}
}

void Attachment::PropogateColor(const Color& theColor, bool theEnableAdditiveColor, const Color& theAdditiveColor, bool theEnableOverlayColor, const Color& theOverlayColor)
{
	PVZP_ASSERT(gEffectSystem);

	for (int i = 0; i < mNumEffects; i++)
	{
		AttachEffect* aAttachEffect = &mEffectArray[i];
		if (aAttachEffect->mDontPropogateColor)
		{
			continue;
		}

		switch (aAttachEffect->mEffectType)
		{
		case EffectType::EFFECT_PARTICLE:
		{
			PvzpParticleSystem* aParticleSystem = gEffectSystem->mParticleHolder->mParticleSystems.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aParticleSystem)
			{
				aParticleSystem->OverrideColor(nullptr, theColor);
				aParticleSystem->OverrideExtraAdditiveDraw(nullptr, theEnableAdditiveColor);
			}
			break;
		}

		case EffectType::EFFECT_REANIM:
		{
			Reanimation* aReanimation = gEffectSystem->mReanimationHolder->mReanimations.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aReanimation)
			{
				aReanimation->mColorOverride = theColor;
				aReanimation->mExtraAdditiveColor = theAdditiveColor;
				aReanimation->mEnableExtraAdditiveDraw = theEnableAdditiveColor;
				aReanimation->mExtraOverlayColor = theOverlayColor;
				aReanimation->mEnableExtraOverlayDraw = theEnableOverlayColor;
				aReanimation->PropogateColorToAttachments();
			}
			break;
		}

		case EffectType::EFFECT_ATTACHMENT:
		{
			Attachment* aAttachment = gEffectSystem->mAttachmentHolder->mAttachments.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aAttachment)
			{
				aAttachment->PropogateColor(theColor, theEnableAdditiveColor, theAdditiveColor, theEnableOverlayColor, theOverlayColor);
			}
			break;
		}
		case EffectType::EFFECT_TRAIL:
		case EffectType::EFFECT_OTHER:
			break;
		}
	}
}

void Attachment::OverrideScale(float theScale)
{
	PVZP_ASSERT(gEffectSystem);

	for (int i = 0; i < mNumEffects; i++)
	{
		AttachEffect* aAttachEffect = &mEffectArray[i];
		switch (aAttachEffect->mEffectType)
		{
		case EffectType::EFFECT_PARTICLE:
		{
			PvzpParticleSystem* aParticleSystem = gEffectSystem->mParticleHolder->mParticleSystems.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aParticleSystem)
			{
				aParticleSystem->OverrideScale(nullptr, theScale);
			}
			break;
		}

		case EffectType::EFFECT_REANIM:
		{
			Reanimation* aReanimation = gEffectSystem->mReanimationHolder->mReanimations.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aReanimation)
			{
				aReanimation->OverrideScale(theScale, theScale);
			}
			break;
		}

		case EffectType::EFFECT_ATTACHMENT:
		{
			Attachment* aAttachment = gEffectSystem->mAttachmentHolder->mAttachments.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aAttachment)
			{
				aAttachment->OverrideScale(theScale);
			}
			break;
		}
		case EffectType::EFFECT_TRAIL:
		case EffectType::EFFECT_OTHER:
			break;
		}
	}
}

void Attachment::CrossFade(const char* theCrossFadeName)
{
	PVZP_ASSERT(gEffectSystem);

	for (int i = 0; i < mNumEffects; i++)
	{
		AttachEffect* aAttachEffect = &mEffectArray[i];
		if (aAttachEffect->mEffectType == EffectType::EFFECT_PARTICLE)
		{
			PvzpParticleSystem* aParticleSystem = gEffectSystem->mParticleHolder->mParticleSystems.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aParticleSystem)
			{
				aParticleSystem->CrossFade(theCrossFadeName);
			}
		}
	}
}

void Attachment::SetMatrix(const SexyTransform2D& theMatrix)
{
	PVZP_ASSERT(gEffectSystem);

	for (int i = 0; i < mNumEffects; i++)
	{
		AttachEffect* aAttachEffect = &mEffectArray[i];
		SexyTransform2D aPosition = theMatrix * aAttachEffect->mOffset;

		switch (aAttachEffect->mEffectType)
		{
		case EffectType::EFFECT_PARTICLE:
		{
			PvzpParticleSystem* aParticleSystem = gEffectSystem->mParticleHolder->mParticleSystems.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aParticleSystem)
			{
				aParticleSystem->SystemMove(aPosition.m02, aPosition.m12);
			}
			break;
		}

		case EffectType::EFFECT_TRAIL:
		{
			Trail* aTrail = gEffectSystem->mTrailHolder->mTrails.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aTrail)
			{
				aTrail->mTrailCenter = SexyVector2(aPosition.m02, aPosition.m12);
			}
			break;
		}

		case EffectType::EFFECT_REANIM:
		{
			Reanimation* aReanimation = gEffectSystem->mReanimationHolder->mReanimations.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aReanimation)
			{
				aReanimation->mOverlayMatrix = aPosition;
			}
			break;
		}

		case EffectType::EFFECT_ATTACHMENT:
		{
			Attachment* aAttachment = gEffectSystem->mAttachmentHolder->mAttachments.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aAttachment)
			{
				aAttachment->SetMatrix(aPosition);
			}
			break;
		}
		case EffectType::EFFECT_OTHER:
			break;
		}
	}
}

void Attachment::Draw(Graphics* g, bool theParentHidden)
{
	PVZP_ASSERT(gEffectSystem);

	DataArray<PvzpParticleSystem>& aParticleSystems = gEffectSystem->mParticleHolder->mParticleSystems;
	DataArray<Trail>& aTrails = gEffectSystem->mTrailHolder->mTrails;
	DataArray<Reanimation>& aReanimations = gEffectSystem->mReanimationHolder->mReanimations;
	DataArray<Attachment>& aAttachments = gEffectSystem->mAttachmentHolder->mAttachments;

	for (int i = 0; i < mNumEffects; i++)
	{
		AttachEffect* aAttachEffect = &mEffectArray[i];
		if (theParentHidden && aAttachEffect->mDontDrawIfParentHidden)
		{
			continue;
		}

		switch (aAttachEffect->mEffectType)
		{
		case EffectType::EFFECT_PARTICLE:
		{
			PvzpParticleSystem* aParticleSystem = aParticleSystems.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aParticleSystem)
			{
				aParticleSystem->Draw(g);
			}
			break;
		}

		case EffectType::EFFECT_TRAIL:
		{
			Trail* aTrail = aTrails.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aTrail)
			{
				aTrail->Draw(g);
			}
			break;
		}

		case EffectType::EFFECT_REANIM:
		{
			Reanimation* aReanimation = aReanimations.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aReanimation)
			{
				aReanimation->DrawAllRenderGroups(g);
			}
			break;
		}

		case EffectType::EFFECT_ATTACHMENT:
		{
			Attachment* aAttachment = aAttachments.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aAttachment)
			{
				aAttachment->Draw(g, theParentHidden);
			}
			break;
		}

		case EffectType::EFFECT_OTHER:
			break;
		}
	}
}

void Attachment::Detach()
{
	PVZP_ASSERT(gEffectSystem);

	DataArray<PvzpParticleSystem>& aParticleSystems = gEffectSystem->mParticleHolder->mParticleSystems;
	DataArray<Trail>& aTrails = gEffectSystem->mTrailHolder->mTrails;
	DataArray<Reanimation>& aReanimations = gEffectSystem->mReanimationHolder->mReanimations;
	DataArray<Attachment>& aAttachments = gEffectSystem->mAttachmentHolder->mAttachments;

	for (int i = 0; i < mNumEffects; i++)
	{
		AttachEffect* aAttachEffect = &mEffectArray[i];
		switch (aAttachEffect->mEffectType)
		{
		case EffectType::EFFECT_PARTICLE:
		{
			PvzpParticleSystem* aParticleSystem = aParticleSystems.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aParticleSystem)
			{
				PVZP_ASSERT(aParticleSystem->mIsAttachment);
				aParticleSystem->mIsAttachment = false;
			}
			break;
		}

		case EffectType::EFFECT_TRAIL:
		{
			Trail* aTrail = aTrails.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aTrail)
			{
				PVZP_ASSERT(aTrail->mIsAttachment);
				aTrail->mIsAttachment = false;
			}
			break;
		}

		case EffectType::EFFECT_REANIM:
		{
			Reanimation* aReanimation = aReanimations.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aReanimation)
			{
				PVZP_ASSERT(aReanimation->mIsAttachment);
				aReanimation->mIsAttachment = false;
			}
			break;
		}

		case EffectType::EFFECT_ATTACHMENT:
		{
			Attachment* aAttachment = aAttachments.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aAttachment)
			{
				aAttachment->Detach();
			}
			break;
		}

		case EffectType::EFFECT_OTHER:
			break;
		}

		aAttachEffect->mEffectID = 0U;
	}

	mNumEffects = 0;
	mDead = true;
}

void Attachment::AttachmentDie()
{
	if (mNumEffects == 0)
	{
		mDead = true;
		return;
	}

	if (!gEffectSystem)
	{
		mNumEffects = 0;
		mDead = true;
		return;
	}

	DataArray<PvzpParticleSystem> *aParticleSystems = nullptr;
	DataArray<Trail> *aTrails = nullptr;
	DataArray<Reanimation> *aReanimations = nullptr;
	DataArray<Attachment> *aAttachments = nullptr;
	if (gEffectSystem->mParticleHolder) aParticleSystems = &gEffectSystem->mParticleHolder->mParticleSystems;
	if (gEffectSystem->mTrailHolder) aTrails = &gEffectSystem->mTrailHolder->mTrails;
	if (gEffectSystem->mReanimationHolder) aReanimations = &gEffectSystem->mReanimationHolder->mReanimations;
	if (gEffectSystem->mAttachmentHolder) aAttachments = &gEffectSystem->mAttachmentHolder->mAttachments;

	for (int i = 0; i < mNumEffects; i++)
	{
		AttachEffect* aAttachEffect = &mEffectArray[i];
		switch (aAttachEffect->mEffectType)
		{
		case EffectType::EFFECT_PARTICLE:
		{
			PvzpParticleSystem* aParticleSystem = aParticleSystems?aParticleSystems->DataArrayTryToGet(aAttachEffect->mEffectID):nullptr;
			if (aParticleSystem)
			{
				aParticleSystem->ParticleSystemDie();
			}
			break;
		}

		case EffectType::EFFECT_TRAIL:
		{
			Trail* aTrail = aTrails?aTrails->DataArrayTryToGet(aAttachEffect->mEffectID):nullptr;
			if (aTrail)
			{
				aTrail->mDead = true;
			}
			break;
		}

		case EffectType::EFFECT_REANIM:
		{
			Reanimation* aReanimation = aReanimations?aReanimations->DataArrayTryToGet(aAttachEffect->mEffectID):nullptr;
			if (aReanimation)
			{
				aReanimation->ReanimationDie();
			}
			break;
		}

		case EffectType::EFFECT_ATTACHMENT:
		{
			Attachment* aAttachment = aAttachments?aAttachments->DataArrayTryToGet(aAttachEffect->mEffectID):nullptr;
			if (aAttachment)
			{
				aAttachment->AttachmentDie();
			}
			break;
		}

		case EffectType::EFFECT_OTHER:
			break;
		}

		aAttachEffect->mEffectID = 0U;
	}

	mNumEffects = 0;
	mDead = true;
}

AttachmentHolder::AttachmentHolder()
{
	//InitializeHolder();
}

AttachmentHolder::~AttachmentHolder()
{
	DisposeHolder();
}

void AttachmentHolder::InitializeHolder()
{
	mAttachments.DataArrayInitialize(1024U, "attachments");
}

void AttachmentHolder::DisposeHolder()
{
	mAttachments.DataArrayDispose();
}

Attachment* AttachmentHolder::AllocAttachment()
{
	if (mAttachments.mSize + 1 >= mAttachments.mMaxSize)
	{
		for (Attachment* aAttachment : mAttachments)
		{
			PruneDeadEffects(aAttachment);
			if (aAttachment->mDead)
			{
				mAttachments.DataArrayFree(aAttachment);
			}
		}
	}

	return mAttachments.DataArrayAlloc();
}

void AttachmentUpdateAndSetMatrix(AttachmentID& theAttachmentID, SexyTransform2D& theMatrix)
{
	if (theAttachmentID == AttachmentID::ATTACHMENTID_NULL)
		return;

	PVZP_ASSERT(gEffectSystem);
	Attachment* aAttachment = gEffectSystem->mAttachmentHolder->mAttachments.DataArrayTryToGet(static_cast<unsigned int>(theAttachmentID));
	if (aAttachment)
	{
		aAttachment->Update();
		aAttachment->SetMatrix(theMatrix);
	}
	else
	{
		theAttachmentID = AttachmentID::ATTACHMENTID_NULL;
	}
}

void AttachmentUpdateAndMove(AttachmentID& theAttachmentID, float theX, float theY)
{
	if (theAttachmentID == AttachmentID::ATTACHMENTID_NULL)
		return;

	PVZP_ASSERT(gEffectSystem);
	Attachment* aAttachment = gEffectSystem->mAttachmentHolder->mAttachments.DataArrayTryToGet(static_cast<unsigned int>(theAttachmentID));
	if (aAttachment)
	{
		aAttachment->Update();
		aAttachment->SetPosition(SexyVector2(theX, theY));
	}
	else
	{
		theAttachmentID = AttachmentID::ATTACHMENTID_NULL;
	}
}

void AttachmentOverrideColor(AttachmentID& theAttachmentID, const Color& theColor)
{
	if (theAttachmentID == AttachmentID::ATTACHMENTID_NULL)
		return;

	PVZP_ASSERT(gEffectSystem);
	Attachment* aAttachment = gEffectSystem->mAttachmentHolder->mAttachments.DataArrayTryToGet(static_cast<unsigned int>(theAttachmentID));
	if (aAttachment)
	{
		aAttachment->OverrideColor(theColor);
	}
}

void AttachmentOverrideScale(AttachmentID& theAttachmentID, float theScale)
{
	if (theAttachmentID == AttachmentID::ATTACHMENTID_NULL)
		return;

	PVZP_ASSERT(gEffectSystem);
	Attachment* aAttachment = gEffectSystem->mAttachmentHolder->mAttachments.DataArrayTryToGet((unsigned int)theAttachmentID);
	if (aAttachment)
	{
		aAttachment->OverrideScale(theScale);
	}
}

void AttachmentReanimTypeDie(AttachmentID& theAttachmentID, ReanimationType theReanimType)
{
	Attachment* aAttachment = gEffectSystem->mAttachmentHolder->mAttachments.DataArrayTryToGet((unsigned int)theAttachmentID);
	if (aAttachment == nullptr)
	{
		return;
	}

	for (int i = 0; i < aAttachment->mNumEffects; i++)
	{
		AttachEffect* aAttachEffect = &aAttachment->mEffectArray[i];
		if (aAttachEffect->mEffectType == EffectType::EFFECT_REANIM)
		{
			Reanimation* aReanimation = gEffectSystem->mReanimationHolder->mReanimations.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aReanimation && aReanimation->mReanimationType == theReanimType)
			{
				aReanimation->ReanimationDie();
			}
		}
	}
}

void AttachmentDetachCrossFadeParticleType(AttachmentID& theAttachmentID, ParticleEffect theParticleEffect, const char* theCrossFadeName)
{
	Attachment* aAttachment = gEffectSystem->mAttachmentHolder->mAttachments.DataArrayTryToGet((unsigned int)theAttachmentID);
	if (aAttachment == nullptr)
	{
		return;
	}

	PVZP_ASSERT(theParticleEffect >= 0 && theParticleEffect < gParticleDefCount);
	PvzpParticleDefinition* aDefinition = &gParticleDefArray[(int)theParticleEffect];

	for (int i = 0; i < aAttachment->mNumEffects;)
	{
		AttachEffect* aAttachEffect = &aAttachment->mEffectArray[i];
		if (aAttachEffect->mEffectType != EffectType::EFFECT_PARTICLE)
		{
			i++;
			continue;
		}

		PvzpParticleSystem* aParticleSystem = gEffectSystem->mParticleHolder->mParticleSystems.DataArrayTryToGet(aAttachEffect->mEffectID);
		if (aParticleSystem && aParticleSystem->mParticleDef == aDefinition)
		{
			if (theCrossFadeName)
			{
				aParticleSystem->mIsAttachment = false;
				aParticleSystem->CrossFade(theCrossFadeName);
			}
			else
			{
				aParticleSystem->ParticleSystemDie();
			}

			// Remove the effect entry immediately so the parent attachment ownership remains consistent.
			int aNumEffectsRemaining = aAttachment->mNumEffects - i - 1;
			if (aNumEffectsRemaining > 0)
			{
				memmove(aAttachEffect, aAttachEffect + 1, aNumEffectsRemaining * sizeof(AttachEffect));
			}
			aAttachment->mNumEffects--;
			continue;
		}

		i++;
	}

	if (aAttachment->mNumEffects == 0)
	{
		aAttachment->mDead = true;
		theAttachmentID = AttachmentID::ATTACHMENTID_NULL;
	}
}

void AttachmentPropogateColor(AttachmentID& theAttachmentID, const Color& theColor, bool theEnableAdditiveColor, const Color& theAdditiveColor, bool theEnableOverlayColor, const Color& theOverlayColor)
{
	if (theAttachmentID == AttachmentID::ATTACHMENTID_NULL)
		return;

	PVZP_ASSERT(gEffectSystem);
	Attachment* aAttachment = gEffectSystem->mAttachmentHolder->mAttachments.DataArrayTryToGet((unsigned int)theAttachmentID);
	if (aAttachment)
	{
		aAttachment->PropogateColor(theColor, theEnableAdditiveColor, theAdditiveColor, theEnableOverlayColor, theOverlayColor);
	}
}

void AttachmentCrossFade(AttachmentID& theAttachmentID, const char* theCrossFadeName)
{
	if (theAttachmentID == AttachmentID::ATTACHMENTID_NULL)
		return;

	PVZP_ASSERT(gEffectSystem);
	Attachment* aAttachment = gEffectSystem->mAttachmentHolder->mAttachments.DataArrayTryToGet((unsigned int)theAttachmentID);
	if (aAttachment)
	{
		aAttachment->CrossFade(theCrossFadeName);
	}
}

void AttachmentDraw(AttachmentID& theAttachmentID, Graphics* g, bool theParentHidden)
{
	if (theAttachmentID == AttachmentID::ATTACHMENTID_NULL)
		return;

	PVZP_ASSERT(gEffectSystem);
	Attachment* aAttachment = gEffectSystem->mAttachmentHolder->mAttachments.DataArrayTryToGet((unsigned int)theAttachmentID);
	if (aAttachment)
	{
		aAttachment->Draw(g, theParentHidden);
	}
}

void AttachmentDie(AttachmentID& theAttachmentID)
{
	if (theAttachmentID == AttachmentID::ATTACHMENTID_NULL)
		return;

	PVZP_ASSERT(gEffectSystem);
	Attachment* aAttachment = gEffectSystem->mAttachmentHolder->mAttachments.DataArrayTryToGet((unsigned int)theAttachmentID);
	theAttachmentID = AttachmentID::ATTACHMENTID_NULL;
	if (aAttachment)
	{
		aAttachment->AttachmentDie();
	}
}

void AttachmentDetach(AttachmentID& theAttachmentID)
{
	if (theAttachmentID == AttachmentID::ATTACHMENTID_NULL)
		return;

	PVZP_ASSERT(gEffectSystem);
	Attachment* aAttachment = gEffectSystem->mAttachmentHolder->mAttachments.DataArrayTryToGet((unsigned int)theAttachmentID);
	theAttachmentID = AttachmentID::ATTACHMENTID_NULL;
	if (aAttachment)
	{
		aAttachment->Detach();
	}
}

Reanimation* FindReanimAttachment(AttachmentID& theAttachmentID)
{
	PVZP_ASSERT(gEffectSystem);
	Attachment* aAttachment = gEffectSystem->mAttachmentHolder->mAttachments.DataArrayTryToGet((unsigned int)theAttachmentID);
	if (aAttachment == nullptr)
	{
		return nullptr;
	}

	for (int i = 0; i < aAttachment->mNumEffects; i++)
	{
		AttachEffect* aAttachEffect = &aAttachment->mEffectArray[i];
		if (aAttachEffect->mEffectType == EffectType::EFFECT_REANIM)
		{
			Reanimation* aReanimation = gEffectSystem->mReanimationHolder->mReanimations.DataArrayTryToGet(aAttachEffect->mEffectID);
			if (aReanimation)
			{
				return aReanimation;
			}
		}
	}

	return nullptr;
}

AttachEffect* FindFirstAttachment(AttachmentID& theAttachmentID)
{
	PVZP_ASSERT(gEffectSystem);
	Attachment* aAttachment = gEffectSystem->mAttachmentHolder->mAttachments.DataArrayTryToGet((unsigned int)theAttachmentID);
	if (aAttachment == nullptr)
	{
		return nullptr;
	}

	return (aAttachment->mNumEffects == 0) ? nullptr : &aAttachment->mEffectArray[0];
}

AttachEffect* CreateEffectAttachment(AttachmentID& theAttachmentID, EffectType theEffectType, unsigned int theDataID, float theOffsetX, float theOffsetY)
{
	PVZP_ASSERT(gEffectSystem);
	Attachment* aAttachment = gEffectSystem->mAttachmentHolder->mAttachments.DataArrayTryToGet((unsigned int)theAttachmentID);
	if (aAttachment == nullptr || aAttachment->mDead)
	{
		aAttachment = gEffectSystem->mAttachmentHolder->AllocAttachment();
		theAttachmentID = (AttachmentID)gEffectSystem->mAttachmentHolder->mAttachments.DataArrayGetID(aAttachment);
	}

	PVZP_ASSERT(aAttachment->mNumEffects < MAX_EFFECTS_PER_ATTACHMENT);
	PVZP_ASSERT(!aAttachment->mDead);

	AttachEffect* aAttachEffect = &aAttachment->mEffectArray[aAttachment->mNumEffects];
	aAttachEffect->mEffectType = theEffectType;
	aAttachEffect->mEffectID = theDataID;
	aAttachEffect->mDontDrawIfParentHidden = false;
	aAttachEffect->mOffset.LoadIdentity();
	aAttachEffect->mOffset.m02 = theOffsetX;
	aAttachEffect->mOffset.m12 = theOffsetY;
	aAttachment->mNumEffects++;
	return aAttachEffect;
}

AttachEffect* AttachReanim(AttachmentID& theAttachmentID, Reanimation* theReanimation, float theOffsetX, float theOffsetY)
{
	unsigned int aReanimId = gEffectSystem->mReanimationHolder->mReanimations.DataArrayGetID(theReanimation);
	AttachEffect* aAttachEffect = CreateEffectAttachment(theAttachmentID, EffectType::EFFECT_REANIM, aReanimId, theOffsetX, theOffsetY);

	PVZP_ASSERT(!theReanimation->mIsAttachment);
	theReanimation->mIsAttachment = true;

	return aAttachEffect;
}

AttachEffect* AttachParticle(AttachmentID& theAttachmentID, PvzpParticleSystem* theParticleSystem, float theOffsetX, float theOffsetY)
{
	if (theParticleSystem == nullptr)
		return nullptr;

	unsigned int aParticleId = gEffectSystem->mParticleHolder->mParticleSystems.DataArrayGetID(theParticleSystem);
	AttachEffect* aAttachEffect = CreateEffectAttachment(theAttachmentID, EffectType::EFFECT_PARTICLE, aParticleId, theOffsetX, theOffsetY);

	PVZP_ASSERT(!theParticleSystem->mIsAttachment);
	theParticleSystem->mIsAttachment = true;

	return aAttachEffect;
}

AttachEffect* AttachTrail(AttachmentID& theAttachmentID, Trail* theTrail, float theOffsetX, float theOffsetY)
{
	unsigned int aTrailId = gEffectSystem->mTrailHolder->mTrails.DataArrayGetID(theTrail);
	AttachEffect* aAttachEffect = CreateEffectAttachment(theAttachmentID, EffectType::EFFECT_TRAIL, aTrailId, theOffsetX, theOffsetY);

	PVZP_ASSERT(!theTrail->mIsAttachment);
	theTrail->mIsAttachment = true;

	return aAttachEffect;
}

bool IsFullOfAttachments(AttachmentID& theAttachmentID)
{
	PVZP_ASSERT(gEffectSystem);
	Attachment* aAttachment = gEffectSystem->mAttachmentHolder->mAttachments.DataArrayTryToGet((unsigned int)theAttachmentID);
	return aAttachment && aAttachment->mNumEffects >= MAX_EFFECTS_PER_ATTACHMENT;
}
