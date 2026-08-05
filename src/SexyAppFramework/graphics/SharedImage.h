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

#ifndef __SHARED_IMAGE_H__
#define __SHARED_IMAGE_H__

#include "Common.h"
#include <atomic>

namespace Sexy
{

class Image;
class GLImage;
class MemoryImage;

class SharedImage
{
public:
	GLImage*				mImage;
	std::atomic<int>		mRefCount;

	SharedImage();
};

typedef std::map<std::pair<std::string, std::string>, SharedImage> SharedImageMap;

class SharedImageRef
{
public:
	SharedImage*			mSharedImage;
	MemoryImage*			mUnsharedImage;
	bool					mOwnsUnshared;

public:
	SharedImageRef();
	SharedImageRef(const SharedImageRef& theSharedImageRef);
	SharedImageRef(SharedImage* theSharedImage);
	~SharedImageRef();

	void					Release();

	SharedImageRef&			operator=(const SharedImageRef& theSharedImageRef);
	SharedImageRef&			operator=(SharedImage* theSharedImage);
	SharedImageRef&			operator=(MemoryImage* theUnsharedImage);
	MemoryImage*			operator->();
	operator Image*();
	operator MemoryImage*();
	operator GLImage*();
};

}

#endif //__SHARED_IMAGE_H__
