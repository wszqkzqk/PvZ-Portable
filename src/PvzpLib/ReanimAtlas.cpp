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

#include <algorithm>
#include "PvzpDebug.h"
#include "PvzpCommon.h"
#include "Reanimator.h"
#include "ReanimAtlas.h"
#include "misc/PerfTimer.h"
#include "graphics/Graphics.h"
#include "graphics/MemoryImage.h"

ReanimAtlas::ReanimAtlas() = default;

ReanimAtlas::~ReanimAtlas() = default;

ReanimAtlasImage* ReanimAtlas::GetEncodedReanimAtlas(Image* theImage)
{
	if (theImage == nullptr || reinterpret_cast<uintptr_t>(theImage) > 1000)
		return nullptr;

	intptr_t aAtlasIndex = reinterpret_cast<intptr_t>(theImage) - 1;
	if (aAtlasIndex < 0 || aAtlasIndex >= static_cast<intptr_t>(mImageArray.size()))
		return nullptr;
	return &mImageArray[aAtlasIndex];
}

MemoryImage* ReanimAtlasMakeBlankMemoryImage(int theWidth, int theHeight)
{
	MemoryImage* aImage = new MemoryImage();

	int aBitsCount = theWidth * theHeight;
	aImage->mBits = std::make_unique<uint32_t[]>(aBitsCount + 1);
	aImage->mWidth = theWidth;
	aImage->mHeight = theHeight;
	aImage->mHasTrans = true;
	aImage->mHasAlpha = true;
	memset(aImage->mBits.get(), 0, aBitsCount * 4);
	aImage->mBits[aBitsCount] = Sexy::MEMORYCHECK_ID;
	return aImage;
}

bool sSortByNonIncreasingHeight(const ReanimAtlasImage& image1, const ReanimAtlasImage& image2)
{
	if (image1.mHeight != image2.mHeight)
		return image1.mHeight > image2.mHeight;
	else if (image1.mWidth != image2.mWidth)
		return image1.mWidth > image2.mWidth;
	else  // Stable tiebreaker using original image pointer (invariant during sort)
		return reinterpret_cast<uintptr_t>(image1.mOriginalImage) > reinterpret_cast<uintptr_t>(image2.mOriginalImage);
}

static int GetClosestPowerOf2Above(int theNum)
{
	int aPower2 = 1;
	while (aPower2 < theNum)
		aPower2 <<= 1;

	return aPower2;
}

int ReanimAtlas::PickAtlasWidth()
{
	int totalArea = 0;
	int aMaxWidth = 0;
	for (size_t i = 0; i < mImageArray.size(); i++)
	{
		ReanimAtlasImage* aImage = &mImageArray[i];
		totalArea += aImage->mWidth * aImage->mHeight;
		aMaxWidth = std::max(aMaxWidth, aImage->mWidth + 2);
	}

	int aWidth = FloatRoundToInt(sqrt(totalArea));  // side length assuming a square region
	return GetClosestPowerOf2Above(std::min(std::max(aWidth, aMaxWidth), 2048));  // max of side length and widest image, capped at 2048, rounded up to a power of 2
}

bool ReanimAtlas::ImageFits(int theImageCount, const Rect& rectTest, int theMaxWidth)
{
	if (rectTest.mX + rectTest.mWidth > theMaxWidth)
		return false;

	for (int i = 0; i < theImageCount; i++)  // check the rect against the first theImageCount placed images
	{
		ReanimAtlasImage* aImage = &mImageArray[i];
		if (Rect(aImage->mX, aImage->mY, aImage->mWidth, aImage->mHeight).Inflate(1, 1).Intersects(rectTest))  // an image occupies its rect inflated by 1 pixel
			return false;
	}
	return true;
}

bool ReanimAtlas::ImageFindPlaceOnSide(ReanimAtlasImage* theAtlasImageToPlace, int theImageCount, int theMaxWidth, bool theToRight)
{
	Rect rectTest;
	rectTest.mWidth = theAtlasImageToPlace->mWidth + 2;
	rectTest.mHeight = theAtlasImageToPlace->mHeight + 2;

	for (int i = 0; i < theImageCount; i++)
	{
		ReanimAtlasImage* aImage = &mImageArray[i];
		if (theToRight)  // to the right of the placed image
		{
			rectTest.mX = aImage->mX + aImage->mWidth + 1;
			rectTest.mY = aImage->mY;
		}
		else  // below the placed image
		{
			rectTest.mX = aImage->mX;
			rectTest.mY = aImage->mY + aImage->mHeight + 1;
		}

		if (ImageFits(theImageCount, rectTest, theMaxWidth))
		{
			theAtlasImageToPlace->mX = rectTest.mX;
			theAtlasImageToPlace->mY = rectTest.mY;
			if (theToRight)
				theAtlasImageToPlace->mX += 1;
			else
				theAtlasImageToPlace->mY += 1;

			return true;
		}
	}

	return false;
}

bool ReanimAtlas::ImageFindPlace(ReanimAtlasImage* theAtlasImageToPlace, int theImageCount, int theMaxWidth)
{
	return
		ImageFindPlaceOnSide(theAtlasImageToPlace, theImageCount, theMaxWidth, true) ||
		ImageFindPlaceOnSide(theAtlasImageToPlace, theImageCount, theMaxWidth, false);  // try placing to the right, then below
}

bool ReanimAtlas::PlaceAtlasImage(ReanimAtlasImage* theAtlasImageToPlace, int theImageCount, int theMaxWidth)
{
	if (theImageCount == 0)
	{
		theAtlasImageToPlace->mX = 1;
		theAtlasImageToPlace->mY = 1;
		return true;
	}

	if (ImageFindPlace(theAtlasImageToPlace, theImageCount, theMaxWidth))
		return true;

	PVZP_ASSERT(false);
	return false;
}

void ReanimAtlas::ArrangeImages(int& theAtlasWidth, int& theAtlasHeight)
{
	std::sort(mImageArray.begin(), mImageArray.end(), sSortByNonIncreasingHeight);
	theAtlasWidth = PickAtlasWidth();
	theAtlasHeight = 0;

	for (int i = 0; i < static_cast<int>(mImageArray.size()); i++)
	{
		ReanimAtlasImage* aImage = &mImageArray[i];
		PlaceAtlasImage(aImage, i, theAtlasWidth);

		// computed once here so the max expression does not evaluate it twice
		int aImageHeight = GetClosestPowerOf2Above(aImage->mY + aImage->mHeight);
		theAtlasHeight = std::max(aImageHeight, theAtlasHeight);
	}
}

void ReanimAtlas::AddImage(Image* theImage)
{
	if (theImage->mNumCols == 1 && theImage->mNumRows == 1)
	{
		auto& aImage = mImageArray.emplace_back();
		aImage.mHeight = theImage->mHeight;
		aImage.mWidth = theImage->mWidth;
		aImage.mOriginalImage = theImage;
	}
}

int ReanimAtlas::FindImage(Image* theImage)
{
	for (int i = 0; i < static_cast<int>(mImageArray.size()); i++)
		if (mImageArray[i].mOriginalImage == theImage)
			return i;

	return -1;
}

void ReanimAtlas::ReanimAtlasCreate(ReanimatorDefinition* theReanimDef)
{
	PerfTimer aTimer;
	aTimer.Start();

	for (int aTrackIndex = 0; aTrackIndex < theReanimDef->mTracks.count; aTrackIndex++)
	{
		ReanimatorTrack* aTrack = &theReanimDef->mTracks.tracks[aTrackIndex];
		for (int aKeyIndex = 0; aKeyIndex < aTrack->mTransforms.count; aKeyIndex++)
		{
			Image* aImage = aTrack->mTransforms.mTransforms[aKeyIndex].mImage;
			if (aImage != nullptr && aImage->mWidth <= 254 && aImage->mHeight <= 254 && FindImage(aImage) < 0)
				AddImage(aImage);  // add it now; its atlas position is decided later
		}
	}

	int aAtlasWidth, aAtlasHeight;
	ArrangeImages(aAtlasWidth, aAtlasHeight);

	for (int aTrackIndex = 0; aTrackIndex < theReanimDef->mTracks.count; aTrackIndex++)
	{
		ReanimatorTrack* aTrack = &theReanimDef->mTracks.tracks[aTrackIndex];
		for (int aKeyIndex = 0; aKeyIndex < aTrack->mTransforms.count; aKeyIndex++)
		{
			Image*& aImage = aTrack->mTransforms.mTransforms[aKeyIndex].mImage;
			if (aImage != nullptr && aImage->mWidth <= 254 && aImage->mHeight <= 254)
			{
				intptr_t aImageIndex = FindImage(aImage);
				if (aImageIndex < 0)  // Image not in atlas (e.g. mNumCols>1)
					continue;
				aImage = (Image*)(aImageIndex + 1);  // Encode atlas index as Image*
			}
		}
	}

	mMemoryImage.reset(ReanimAtlasMakeBlankMemoryImage(aAtlasWidth, aAtlasHeight));
	Graphics aMemoryGraphis(mMemoryImage.get());
	for (int aImageIndex = 0; aImageIndex < static_cast<int>(mImageArray.size()); aImageIndex++)
	{
		ReanimAtlasImage* aImage = &mImageArray[aImageIndex];
		aMemoryGraphis.DrawImage(aImage->mOriginalImage, aImage->mX, aImage->mY);
	}
	FixPixelsOnAlphaEdgeForBlending(mMemoryImage.get());  // set transparent pixels to the average color of their neighbors
}
