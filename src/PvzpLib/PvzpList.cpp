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

#include "PvzpList.h"
#include "PvzpDebug.h"
#include "PvzpCommon.h"
#include "misc/Debug.h"

void PvzpAllocator::Initialize(int theGrowCount, int theItemSize)
{
	PVZP_ASSERT(static_cast<size_t>(theItemSize) >= sizeof(void*));

	mFreeList = nullptr;
	mBlockList = nullptr;
	mGrowCount = theGrowCount;
	mTotalItems = 0;
	mItemSize = theItemSize;
}

void PvzpAllocator::Dispose()
{
	FreeAll();
}

void PvzpAllocator::Grow()
{
	PVZP_ASSERT(mGrowCount > 0);
	PVZP_ASSERT(static_cast<size_t>(mItemSize) >= sizeof(void*));

	void* aBlock = PvzpMalloc(mGrowCount * mItemSize + sizeof(void*));
	*(void**)aBlock = mBlockList;
	mBlockList = aBlock;

	void* aFreeList = mFreeList;
	void* aItem = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(aBlock) + sizeof(void*));
	for (int i = 0; i < mGrowCount; i++)
	{
		*(void**)aItem = aFreeList;
		aFreeList = aItem;
		aItem = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(aItem) + mItemSize);
	}
	mFreeList = aFreeList;
}

bool PvzpAllocator::IsPointerFromAllocator(void* theItem)
{
	size_t aBlockSize = mGrowCount * mItemSize;
	for (void* aPtr = mBlockList; aPtr != nullptr; aPtr = *(void**)aPtr)
	{
		uintptr_t aItemPtr = (uintptr_t)theItem;
		// the first bytes of each block hold the pointer to the next block
		uintptr_t aBlockPtr = (uintptr_t)aPtr + sizeof(void*);
		// theItem must point to the start of an item within this block
		if (aItemPtr >= aBlockPtr && aItemPtr < aBlockPtr + aBlockSize && (aItemPtr - aBlockPtr) % mItemSize == 0)
			return true;
	}
	return false;
}

bool PvzpAllocator::IsPointerOnFreeList(void* theItem)
{
	for (void* aPtr = mFreeList; aPtr != nullptr; aPtr = *(void**)aPtr)
		if (theItem == aPtr)
			return true;
	return false;
}

void* PvzpAllocator::Alloc([[maybe_unused]] int theItemSize)
{
	mTotalItems++;
	if (mFreeList == nullptr)
		Grow();

	void* anItem = (void*)mFreeList;
	mFreeList = *(void**)anItem;
	return anItem;
}

void* PvzpAllocator::Calloc(int theItemSize)
{
	void* anItem = Alloc(theItemSize);
	memset(anItem, 0, theItemSize);
	return anItem;
}

void PvzpAllocator::Free(void* theItem, [[maybe_unused]] int theItemSize)
{
	mTotalItems--;
	PVZP_ASSERT(IsPointerFromAllocator(theItem));
	PVZP_ASSERT(!IsPointerOnFreeList(theItem));
	*(void**)theItem = mFreeList;
	mFreeList = theItem;
}

void PvzpAllocator::FreeAll()
{
	for (void* aBlock = mBlockList; aBlock != nullptr; )
	{
		void* aNext = *(void**)aBlock;
		PvzpFree(aBlock);
		aBlock = aNext;
	}

	mBlockList = nullptr;
	mFreeList = nullptr;
	mTotalItems = 0;
}
