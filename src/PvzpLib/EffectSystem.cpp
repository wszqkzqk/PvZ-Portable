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
#include "../GameConstants.h"
#include "graphics/GLImage.h"
#include "graphics/GLInterface.h"

EffectSystem* gEffectSystem = nullptr;

EffectSystem::~EffectSystem()
{
	EffectSystemDispose();
}

void EffectSystem::EffectSystemInitialize()
{
	PVZP_ASSERT(!gEffectSystem);
	PVZP_ASSERT(!mParticleHolder && !mTrailHolder && !mReanimationHolder && !mAttachmentHolder);

	gEffectSystem = this;
	mParticleHolder = std::make_unique<PvzpParticleHolder>();
	mTrailHolder = std::make_unique<TrailHolder>();
	mReanimationHolder = std::make_unique<ReanimationHolder>();
	mAttachmentHolder = std::make_unique<AttachmentHolder>();

	mParticleHolder->InitializeHolder();
	mTrailHolder->InitializeHolder();
	mReanimationHolder->InitializeHolder();
	mAttachmentHolder->InitializeHolder();
}

void EffectSystem::EffectSystemDispose()
{
	mParticleHolder.reset();
	mTrailHolder.reset();
	mReanimationHolder.reset();
	mAttachmentHolder.reset();

	gEffectSystem = nullptr;
}

void EffectSystem::EffectSystemFreeAll()
{
	mParticleHolder->mParticleSystems.DataArrayFreeAll();
	mParticleHolder->mEmitters.DataArrayFreeAll();
	mParticleHolder->mParticles.DataArrayFreeAll();
	mParticleHolder->mEmitterListNodeAllocator.FreeAll();
	mParticleHolder->mParticleListNodeAllocator.FreeAll();

	mTrailHolder->mTrails.DataArrayFreeAll();
	mReanimationHolder->mReanimations.DataArrayFreeAll();
	mAttachmentHolder->mAttachments.DataArrayFreeAll();
}

void EffectSystem::ProcessDeleteQueue()
{
	for (PvzpParticleSystem* aParticle : mParticleHolder->mParticleSystems)
		if (aParticle->mDead)
			mParticleHolder->mParticleSystems.DataArrayFree(aParticle);

	for (Trail* aTrail : mTrailHolder->mTrails)
		if (aTrail->mDead)
			mTrailHolder->mTrails.DataArrayFree(aTrail);

	for (Reanimation* aReanim : mReanimationHolder->mReanimations)
		if (aReanim->mDead)
			mReanimationHolder->mReanimations.DataArrayFree(aReanim);

	for (Attachment* aAttachment : mAttachmentHolder->mAttachments)
		if (aAttachment->mDead)
			mAttachmentHolder->mAttachments.DataArrayFree(aAttachment);
}

void EffectSystem::Update()
{
	for (PvzpParticleSystem* aParticle : mParticleHolder->mParticleSystems)
		if (!aParticle->mIsAttachment)
			aParticle->Update();

	for (Trail* aTrail : mTrailHolder->mTrails)
		if (!aTrail->mIsAttachment)
			aTrail->Update();

	for (Reanimation* aReanim : mReanimationHolder->mReanimations)
		if (!aReanim->mIsAttachment)
			aReanim->Update();
}

static TriVertex		gPvzpVertexReservoir[64];
static unsigned int		gPvzpVertexReservoirUsed = 0;

static int FixedFloor(int x)
{
	if (x > 0)
		return x & 0xFFFF0000;
	else
		return (x & 0xFFFF0000) - 0x10000;
}

static inline void Pvzp_lClip(TriVertex& dst, const TriVertex& on, const TriVertex& off, const float edge)
{
	float delta = (edge - off.x) / (on.x - off.x);
	dst.x = off.x + (on.x - off.x) * delta;
	dst.y = off.y + (on.y - off.y) * delta;
	dst.u = off.u + (on.u - off.u) * delta;
	dst.v = off.v + (on.v - off.v) * delta;
	dst.color =
		((int)((off.color >> 24 & 0xff) + ((on.color >> 24 & 0xff) - (off.color >> 24 & 0xff)) * delta) << 24) |
		((int)((off.color >> 16 & 0xff) + ((on.color >> 16 & 0xff) - (off.color >> 16 & 0xff)) * delta) << 16) |
		((int)((off.color >>  8 & 0xff) + ((on.color >>  8 & 0xff) - (off.color >>  8 & 0xff)) * delta) << 8) |
		((int)((off.color       & 0xff) + ((on.color       & 0xff) - (off.color       & 0xff)) * delta));
}
static inline void rClip(TriVertex& dst, const TriVertex& on, const TriVertex& off, const float edge)
{
	float delta = (edge - off.x) / (on.x - off.x);
	dst.x = off.x + (on.x - off.x) * delta;
	dst.y = off.y + (on.y - off.y) * delta;
	dst.u = off.u + (on.u - off.u) * delta;
	dst.v = off.v + (on.v - off.v) * delta;
	dst.color =
		((int)((off.color >> 24 & 0xff) + ((on.color >> 24 & 0xff) - (off.color >> 24 & 0xff)) * delta) << 24) |
		((int)((off.color >> 16 & 0xff) + ((on.color >> 16 & 0xff) - (off.color >> 16 & 0xff)) * delta) << 16) |
		((int)((off.color >>  8 & 0xff) + ((on.color >>  8 & 0xff) - (off.color >>  8 & 0xff)) * delta) << 8) |
		((int)((off.color       & 0xff) + ((on.color       & 0xff) - (off.color       & 0xff)) * delta));
}

static inline void Pvzp_tClip(TriVertex& dst, const TriVertex& on, const TriVertex& off, const float edge)
{
	float delta = (edge - off.y) / (on.y - off.y);
	dst.x = off.x + (on.x - off.x) * delta;
	dst.y = off.y + (on.y - off.y) * delta;
	dst.u = off.u + (on.u - off.u) * delta;
	dst.v = off.v + (on.v - off.v) * delta;
	dst.color =
		((int)((off.color >> 24 & 0xff) + ((on.color >> 24 & 0xff) - (off.color >> 24  & 0xff)) * delta) << 24) |
		((int)((off.color >> 16 & 0xff) + ((on.color >> 16 & 0xff) - (off.color >> 16  & 0xff)) * delta) << 16) |
		((int)((off.color >>  8 & 0xff) + ((on.color >>  8 & 0xff) - (off.color >>  8  & 0xff)) * delta) <<  8) |
		((int)((off.color       & 0xff) + ((on.color       & 0xff) - (off.color        & 0xff)) * delta));
}
static inline void Pvzp_bClip(TriVertex& dst, const TriVertex& on, const TriVertex& off, const float edge)
{
	float delta = (edge - off.y) / (on.y - off.y);
	dst.x = off.x + (on.x - off.x) * delta;
	dst.y = off.y + (on.y - off.y) * delta;
	dst.u = off.u + (on.u - off.u) * delta;
	dst.v = off.v + (on.v - off.v) * delta;
	dst.color =
		((int)((off.color >> 24 & 0xff) + ((on.color >> 24 & 0xff) - (off.color >> 24  & 0xff)) * delta) << 24) |
		((int)((off.color >> 16 & 0xff) + ((on.color >> 16 & 0xff) - (off.color >> 16  & 0xff)) * delta) << 16) |
		((int)((off.color >>  8 & 0xff) + ((on.color >>  8 & 0xff) - (off.color >>  8  & 0xff)) * delta) <<  8) |
		((int)((off.color       & 0xff) + ((on.color       & 0xff) - (off.color        & 0xff)) * delta));
}

static inline unsigned int Pvzp_leClip(TriVertex** src, TriVertex** dst, const float edge)
{
	TriVertex** _dst = dst;

	for (TriVertex** v = src; *v; ++v)
	{
		TriVertex* cur = *v;
		TriVertex* nex = *(v + 1) ? *(v + 1) : *src;

		switch ((cur->x < edge ? 1 : 0) | (nex->x < edge ? 2 : 0))
		{
		case 0:
			*dst = *v;
			++dst;
			break;
		case 1:
		{
			TriVertex& tmp = gPvzpVertexReservoir[gPvzpVertexReservoirUsed++];
			Pvzp_lClip(tmp, *nex, *cur, edge);
			*dst = &tmp;
			++dst;
			break;
		}
		case 2:
		{
			*dst = *v;
			++dst;
			TriVertex& tmp = gPvzpVertexReservoir[gPvzpVertexReservoirUsed++];
			Pvzp_lClip(tmp, *cur, *nex, edge);
			*dst = &tmp;
			++dst;
			break;
		}
		}
	}
	*dst = 0;
	return static_cast<int>(dst - _dst);
}

static inline unsigned int Pvzp_reClip(TriVertex** src, TriVertex** dst, const float edge)
{
	TriVertex** _dst = dst;

	for (TriVertex** v = src; *v; ++v)
	{
		TriVertex* cur = *v;
		TriVertex* nex = *(v + 1) ? *(v + 1) : *src;

		switch ((cur->x > edge ? 1 : 0) | (nex->x > edge ? 2 : 0))
		{
		case 0:
			*dst = *v;
			++dst;
			break;
		case 1:
		{
			TriVertex& tmp = gPvzpVertexReservoir[gPvzpVertexReservoirUsed++];
			rClip(tmp, *nex, *cur, edge);
			*dst = &tmp;
			++dst;
			break;
		}
		case 2:
		{
			*dst = *v;
			++dst;
			TriVertex& tmp = gPvzpVertexReservoir[gPvzpVertexReservoirUsed++];
			rClip(tmp, *cur, *nex, edge);
			*dst = &tmp;
			++dst;
			break;
		}
		}
	}
	*dst = 0;
	return static_cast<int>(dst - _dst);
}

static inline unsigned int Pvzp_teClip(TriVertex** src, TriVertex** dst, const float edge)
{
	TriVertex** _dst = dst;

	for (TriVertex** v = src; *v; ++v)
	{
		TriVertex* cur = *v;
		TriVertex* nex = *(v + 1) ? *(v + 1) : *src;

		switch ((cur->y < edge ? 1 : 0) | (nex->y < edge ? 2 : 0))
		{
		case 0:
			*dst = *v;
			++dst;
			break;
		case 1:
		{
			TriVertex& tmp = gPvzpVertexReservoir[gPvzpVertexReservoirUsed++];
			Pvzp_tClip(tmp, *nex, *cur, edge);
			*dst = &tmp;
			++dst;
			break;
		}
		case 2:
		{
			*dst = *v;
			++dst;
			TriVertex& tmp = gPvzpVertexReservoir[gPvzpVertexReservoirUsed++];
			Pvzp_tClip(tmp, *cur, *nex, edge);
			*dst = &tmp;
			++dst;
			break;
		}
		}
	}
	*dst = 0;
	return static_cast<int>(dst - _dst);
}

static inline unsigned int Pvzp_beClip(TriVertex** src, TriVertex** dst, const float edge)
{
	TriVertex** _dst = dst;

	for (TriVertex** v = src; *v; ++v)
	{
		TriVertex* cur = *v;
		TriVertex* nex = *(v + 1) ? *(v + 1) : *src;

		switch ((cur->y > edge ? 1 : 0) | (nex->y > edge ? 2 : 0))
		{
		case 0:
			*dst = *v;
			++dst;
			break;
		case 1:
		{
			TriVertex& tmp = gPvzpVertexReservoir[gPvzpVertexReservoirUsed++];
			Pvzp_bClip(tmp, *nex, *cur, edge);
			*dst = &tmp;
			++dst;
			break;
		}
		case 2:
		{
			*dst = *v;
			++dst;
			TriVertex& tmp = gPvzpVertexReservoir[gPvzpVertexReservoirUsed++];
			Pvzp_bClip(tmp, *cur, *nex, edge);
			*dst = &tmp;
			++dst;
			break;
		}
		}
	}
	*dst = 0;
	return static_cast<int>(dst - _dst);
}

static inline int Pvzp_clipShape(TriVertex** dst, TriVertex* src, const float left, const float right, const float top, const float bottom)
{
	gPvzpVertexReservoirUsed = 0;

	TriVertex* buf[64];
	TriVertex* ptr[4];
	ptr[0] = src;
	ptr[1] = src + 1;
	ptr[2] = src + 2;
	ptr[3] = 0;

	if (Pvzp_leClip(ptr, buf, left) < 3)
		return 0;
	if (Pvzp_reClip(buf, dst, right) < 3)
		return 0;
	if (Pvzp_teClip(dst, buf, top) < 3)
		return 0;
	return Pvzp_beClip(buf, dst, bottom);
}

bool gPvzpTriangleDrawAdditive = false;

#include "PvzpDrawTriangleInc.inc"

PvzpTriangleGroup::PvzpTriangleGroup()
{
	for (int i = 0; i < 256; i++)
		for (int j = 0; j < 3; j++)
			mVertArray[i][j].color = 0UL;

	mImage = nullptr;
	mTriangleCount = 0;
	mDrawMode = Graphics::DRAWMODE_NORMAL;
}

void PvzpTriangleGroup::DrawGroup(Graphics* g)
{
	if (mImage && mTriangleCount)
	{
		// without 3D acceleration, additive blending is messed up
		if (!gSexyAppBase->Is3DAccelerated() && mDrawMode == Graphics::DRAWMODE_ADDITIVE)
			gPvzpTriangleDrawAdditive = true;
		PvzpSandImageIfNeeded(mImage);

		if (GLImage::Check3D(g->mDestImage))
		{
			GLImage* anImage = (GLImage*)g->mDestImage;
			mImage->mDrawn = true;
			anImage->mGLInterface->DrawTrianglesTex(mVertArray, mTriangleCount, Color::White, mDrawMode, mImage, 0.0f, 0.0f, g->mLinearBlend);
		}
		else
		{
			g->mDestImage->BltTrianglesTex(mImage, mVertArray, mTriangleCount, Rect(0, 0, BOARD_WIDTH, BOARD_HEIGHT), Color::White, mDrawMode, 0.0f, 0.0f, g->mLinearBlend);
		}

		mTriangleCount = 0;
		gPvzpTriangleDrawAdditive = false;
	}
}

void PvzpTriangleGroup::AddTriangle(Graphics* g, Image* theImage, const SexyMatrix3& theMatrix, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect& theSrcRect)
{
	PVZP_ASSERT(theImage != nullptr);

	if (mTriangleCount > 0 && (mDrawMode != theDrawMode || mImage != theImage))
		DrawGroup(g);
	mImage = theImage;
	mDrawMode = theDrawMode;

	SexyVector2 p[4];
	float x = -theSrcRect.mWidth * 0.5f;
	float y = -theSrcRect.mHeight * 0.5f;
	p[0].x = x;
	p[0].y = y;
	p[1].x = x;
	p[1].y = y + theSrcRect.mHeight;
	p[2].x = x + theSrcRect.mWidth;
	p[2].y = y;
	p[3].x = x + theSrcRect.mWidth;
	p[3].y = y + theSrcRect.mHeight;
	SexyVector2 tp[4];
	for (int i = 0; i < 4; i++)
	{
		tp[i] = theMatrix * p[i];
		tp[i].x -= 0.5f;
		tp[i].y -= 0.5f;
	}

	PVZP_ASSERT(theSrcRect.mX >= 0 && theSrcRect.mWidth >= 0 && theSrcRect.mX + theSrcRect.mWidth <= mImage->mWidth);
	PVZP_ASSERT(theSrcRect.mY >= 0 && theSrcRect.mHeight >= 0 && theSrcRect.mY + theSrcRect.mHeight <= mImage->mHeight);

	float aOneOverWidth = 1.0f / mImage->mWidth;
	float aOneOverHeight = 1.0f / mImage->mHeight;
	float aTexX1 = aOneOverWidth * theSrcRect.mX;
	float aTexY1 = aOneOverHeight * theSrcRect.mY;
	float aTexX2 = aOneOverWidth * (theSrcRect.mX + theSrcRect.mWidth);
	float aTexY2 = aOneOverHeight * (theSrcRect.mY + theSrcRect.mHeight);
	unsigned int aTexColor = theColor.ToInt();

	bool aNoClipping = false;
	TriVertex aVertBuffer[2][3];
	TriVertex (*aTriRef)[3] = aVertBuffer;
	if (mTriangleCount + 2 <= MAX_TRIANGLES)
	{
		if ((
				theClipRect.mX == 0 && theClipRect.mY == 0 && theClipRect.mWidth == BOARD_WIDTH && theClipRect.mHeight == BOARD_HEIGHT && gSexyAppBase->Is3DAccelerated()
			) || (
				theClipRect.mX <= tp[0].x && theClipRect.mX + theClipRect.mWidth >= tp[0].x &&
				theClipRect.mX <= tp[1].x && theClipRect.mX + theClipRect.mWidth >= tp[1].x &&
				theClipRect.mX <= tp[2].x && theClipRect.mX + theClipRect.mWidth >= tp[2].x &&
				theClipRect.mX <= tp[3].x && theClipRect.mX + theClipRect.mWidth >= tp[3].x &&
				theClipRect.mY <= tp[0].y && theClipRect.mY + theClipRect.mHeight >= tp[0].y &&
				theClipRect.mY <= tp[1].y && theClipRect.mY + theClipRect.mHeight >= tp[1].y &&
				theClipRect.mY <= tp[2].y && theClipRect.mY + theClipRect.mHeight >= tp[2].y &&
				theClipRect.mY <= tp[3].y && theClipRect.mY + theClipRect.mHeight >= tp[3].y
			))
		{
			aNoClipping = true;
			aTriRef = &mVertArray[mTriangleCount];
			mTriangleCount += 2;
		}
	}

	aTriRef[0][0].x = tp[0].x;
	aTriRef[0][0].y = tp[0].y;
	aTriRef[0][0].u = aTexX1;
	aTriRef[0][0].v = aTexY1;
	aTriRef[0][0].color = aTexColor;
	aTriRef[0][1].x = tp[1].x;
	aTriRef[0][1].y = tp[1].y;
	aTriRef[0][1].u = aTexX1;
	aTriRef[0][1].v = aTexY2;
	aTriRef[0][1].color = aTexColor;
	aTriRef[0][2].x = tp[2].x;
	aTriRef[0][2].y = tp[2].y;
	aTriRef[0][2].u = aTexX2;
	aTriRef[0][2].v = aTexY1;
	aTriRef[0][2].color = aTexColor;
	aTriRef[1][0].x = tp[2].x;
	aTriRef[1][0].y = tp[2].y;
	aTriRef[1][0].u = aTexX2;
	aTriRef[1][0].v = aTexY1;
	aTriRef[1][0].color = aTexColor;
	aTriRef[1][1].x = tp[1].x;
	aTriRef[1][1].y = tp[1].y;
	aTriRef[1][1].u = aTexX1;
	aTriRef[1][1].v = aTexY2;
	aTriRef[1][1].color = aTexColor;
	aTriRef[1][2].x = tp[3].x;
	aTriRef[1][2].y = tp[3].y;
	aTriRef[1][2].u = aTexX2;
	aTriRef[1][2].v = aTexY2;
	aTriRef[1][2].color = aTexColor;

	if (aNoClipping)
	{
		if (mTriangleCount == MAX_TRIANGLES)
			DrawGroup(g);
	}
	else
	{
		TriVertex* clipped[64];
		float clipX0 = theClipRect.mX;
		float clipY0 = theClipRect.mY;
		float clipX1 = theClipRect.mX + theClipRect.mWidth;
		float clipY1 = theClipRect.mY + theClipRect.mHeight;

		for (int i = 0; i < 2; i++)
		{
			int vCount = Pvzp_clipShape(clipped, aTriRef[i], clipX0, clipX1, clipY0, clipY1);
			for (int j = 0; j < vCount - 2; j++)
			{
				if (mTriangleCount == MAX_TRIANGLES)
					DrawGroup(g);

				TriVertex* pVert = mVertArray[mTriangleCount];
				pVert[0].x = clipped[0]->x;
				pVert[0].y = clipped[0]->y;
				pVert[0].u = clipped[0]->u;
				pVert[0].v = clipped[0]->v;
				pVert[0].color = clipped[0]->color;
				pVert[1].x = clipped[j + 1]->x;
				pVert[1].y = clipped[j + 1]->y;
				pVert[1].u = clipped[j + 1]->u;
				pVert[1].v = clipped[j + 1]->v;
				pVert[1].color = clipped[j + 1]->color;
				pVert[2].x = clipped[j + 2]->x;
				pVert[2].y = clipped[j + 2]->y;
				pVert[2].u = clipped[j + 2]->u;
				pVert[2].v = clipped[j + 2]->v;
				pVert[2].color = clipped[j + 2]->color;
				if (++mTriangleCount == MAX_TRIANGLES)
					DrawGroup(g);
			}
		}
	}
}
