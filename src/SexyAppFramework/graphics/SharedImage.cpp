/*
 * Portions of this file are based on the PopCap Games Framework
 * Copyright (C) 2005-2009 PopCap Games, Inc.
 *
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
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

#include "SharedImage.h"
#include "graphics/GLImage.h"
#include "SexyAppBase.h"

using namespace Sexy;

SharedImage::SharedImage()
{
	mImage = nullptr;
	mRefCount = 0;
}

SharedImageRef::SharedImageRef(const SharedImageRef& theSharedImageRef)
{
	mSharedImage = theSharedImageRef.mSharedImage;
	if (mSharedImage != nullptr)
		mSharedImage->mRefCount++;
	mUnsharedImage = theSharedImageRef.mUnsharedImage;
	mOwnsUnshared = false;
}

SharedImageRef::SharedImageRef()
{
	mSharedImage = nullptr;
	mUnsharedImage = nullptr;
	mOwnsUnshared = false;
}

SharedImageRef::SharedImageRef(SharedImage* theSharedImage)
{
	mSharedImage = theSharedImage;
	if (theSharedImage != nullptr)
		mSharedImage->mRefCount++;

	mUnsharedImage = nullptr;
	mOwnsUnshared = false;
}

SharedImageRef::~SharedImageRef()
{
	Release();
}

void SharedImageRef::Release()
{
	if (mOwnsUnshared)
		delete mUnsharedImage;
	mUnsharedImage = nullptr;
	if (mSharedImage != nullptr)
	{
		if (--mSharedImage->mRefCount == 0)
			gSexyAppBase->mCleanupSharedImages.store(true, std::memory_order_relaxed);
	}
	mSharedImage = nullptr;
}

SharedImageRef& SharedImageRef::operator=(const SharedImageRef& theSharedImageRef)
{
	Release();
	mSharedImage = theSharedImageRef.mSharedImage;
	if (mSharedImage != nullptr)
		mSharedImage->mRefCount++;
	return *this;
}

SharedImageRef&	SharedImageRef::operator=(SharedImage* theSharedImage)
{
	Release();
	mSharedImage = theSharedImage;
	mSharedImage->mRefCount++;
	return *this;
}

SharedImageRef& SharedImageRef::operator=(MemoryImage* theUnsharedImage)
{
	Release();
	mUnsharedImage = theUnsharedImage;
	return *this;
}

MemoryImage* SharedImageRef::operator->()
{
	return (MemoryImage*) *this;
}


SharedImageRef::operator Image*()
{
	return (MemoryImage*) *this;
}

SharedImageRef::operator MemoryImage*()
{
	if (mUnsharedImage != nullptr)
		return mUnsharedImage;
	else
		return (GLImage*) *this;
}

SharedImageRef::operator GLImage*()
{
	if (mSharedImage != nullptr)
		return mSharedImage->mImage;
	else
		return nullptr;
}
