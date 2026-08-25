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

#ifndef __REANIMATLAS_H__
#define __REANIMATLAS_H__

#include <memory>
#include <vector>
#include "misc/Rect.h"
using namespace Sexy;

class ReanimatorDefinition;
namespace Sexy
{
	class Image;
	class MemoryImage;
};

class ReanimAtlasImage
{
public:
	int                             mX;
	int                             mY;
	int                             mWidth;
	int                             mHeight;
	Image*                          mOriginalImage;

public:
	ReanimAtlasImage() : mX(0), mY(0), mWidth(0), mHeight(0), mOriginalImage(nullptr){ }
};

bool                                sSortByNonIncreasingHeight(const ReanimAtlasImage& image1, const ReanimAtlasImage& image2);

class ReanimAtlas
{
public:
	std::vector<ReanimAtlasImage>   mImageArray;
	std::unique_ptr<MemoryImage>    mMemoryImage;

public:
	ReanimAtlas();
	~ReanimAtlas();

	void                            ReanimAtlasCreate(ReanimatorDefinition* theReanimDef);
	void                 AddImage(Image* theImage);
	int                  FindImage(Image* theImage);
	bool                            ImageFits(int theImageCount, const Rect& rectTest, int theMaxWidth);
	bool                            ImageFindPlaceOnSide(ReanimAtlasImage* theAtlasImageToPlace, int theImageCount, int theMaxWidth, bool theToRight);
	bool                 ImageFindPlace(ReanimAtlasImage* theAtlasImageToPlace, int theImageCount, int theMaxWidth);
	bool                 PlaceAtlasImage(ReanimAtlasImage* theAtlasImageToPlace, int theImageCount, int theMaxWidth);
	int                             PickAtlasWidth();
	void                            ArrangeImages(int& theAtlasWidth, int& theAtlasHeight);
	ReanimAtlasImage*               GetEncodedReanimAtlas(Image* theImage);
};

MemoryImage*                        ReanimAtlasMakeBlankMemoryImage(int theWidth, int theHeight);

#endif
