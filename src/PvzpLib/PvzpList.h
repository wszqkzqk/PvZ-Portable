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

#ifndef __PVZPLIST_H__
#define __PVZPLIST_H__

#define MAX_GLOBAL_ALLOCATORS 128

#include "PvzpDebug.h"
#include "PvzpCommon.h"

struct PvzpAllocator
{
	void*				mFreeList;
	void*				mBlockList;
	int					mGrowCount;
	int					mTotalItems;
	int					mItemSize;

	void                Initialize(int theGrowCount, int theItemSize);
	void                Dispose();
	void                FreeAll();
	void*               Alloc(int theItemSize);
	void*               Calloc(int theItemSize);
	void                Free(void* theItem, int theItemSize);
	void                Grow();
	bool                IsPointerFromAllocator(void* theItem);
	bool                IsPointerOnFreeList(void* theItem);
};
extern int gNumGlobalAllocators;
extern PvzpAllocator gGlobalAllocators[MAX_GLOBAL_ALLOCATORS];

template <typename T> class PvzpListNode
{
public:
	T					mValue;
	PvzpListNode<T>*		mNext;
	PvzpListNode<T>*		mPrev;
};

template <typename T> class PvzpList
{
public:
	PvzpListNode<T>*		mHead;
	PvzpListNode<T>*		mTail;
	int					mSize;
	PvzpAllocator*		mpAllocator;

public:
	PvzpList()
	{
		mHead = nullptr;
		mTail = nullptr;
		mSize = 0;
		mpAllocator = nullptr;
	}

	~PvzpList()
	{
		RemoveAll();
	}

	PvzpListNode<T>* GetHead()
	{
		PVZP_ASSERT(mHead != nullptr);
		return mHead;
	}

	PvzpListNode<T>* GetTail()
	{
		PVZP_ASSERT(mTail != nullptr);
		return mTail;
	}

	void AddHead(const T& theHead)
	{
		if (mpAllocator == nullptr)
			mpAllocator = FindGlobalAllocator(sizeof(PvzpListNode<T>));

		PvzpListNode<T>* aNode = (PvzpListNode<T>*)mpAllocator->Calloc(sizeof(PvzpListNode<T>));
		if (aNode)
			aNode->mValue = theHead;
		aNode->mNext = mHead;
		aNode->mPrev = nullptr;
		if (mHead)
			mHead->mPrev = aNode;
		else
			mTail = aNode;
		mSize++;
		mHead = aNode;
	}

	void AddTail(const T& theTail)
	{
		if (mpAllocator == nullptr)
			mpAllocator = FindGlobalAllocator(sizeof(PvzpListNode<T>));

		PvzpListNode<T>* aNode = (PvzpListNode<T>*)mpAllocator->Calloc(sizeof(PvzpListNode<T>));
		if (aNode)
			aNode->mValue = theTail;
		aNode->mNext = nullptr;
		aNode->mPrev = mTail;
		if (mTail)
			mTail->mNext = aNode;
		else
			mHead = aNode;
		mSize++;
		mTail = aNode;
	}

	inline T RemoveHead()
	{
		PvzpListNode<T>* aHead = mHead;
		PvzpListNode<T>* aSecNode = aHead->mNext;
		mHead = aSecNode;
		if (aSecNode)
			aSecNode->mPrev = nullptr;
		else
			mTail = nullptr;

		T aVal = aHead->mValue;
		mSize--;
		mpAllocator->Free(aHead, sizeof(PvzpListNode<T>));
		return aVal;
	}

	inline PvzpListNode<T>* RemoveAt(PvzpListNode<T>* theNode)
	{
		PvzpListNode<T>* aNext = theNode->mNext;
		if (theNode->mPrev != nullptr)
			theNode->mPrev->mNext = aNext;
		else
			mHead = aNext;

		if (aNext != nullptr)
			aNext->mPrev = theNode->mPrev;
		else
			mTail = theNode->mPrev;

		mSize--;
		mpAllocator->Free(theNode, sizeof(PvzpListNode<T>));
		return aNext;
	}

	inline PvzpListNode<T>* Find(const T& theItem) const
	{
		for (PvzpListNode<T>* aNode = mHead; aNode != nullptr; aNode = aNode->mNext)
			if (aNode->mValue == theItem)
				return aNode;
		return nullptr;
	}

	inline void RemoveAll()
	{
		PvzpListNode<T>* aNode = mHead;
		while (aNode)
		{
			PvzpListNode<T>* temp = aNode;
			aNode = aNode->mNext;
			mpAllocator->Free(temp, sizeof(PvzpListNode<T>));
		}

		mSize = 0;
		mHead = nullptr;
		mTail = nullptr;
	}

	inline void SetAllocator(PvzpAllocator* theAllocator)
	{
		PVZP_ASSERT(mSize == 0);
		mpAllocator = theAllocator;
	}
};

#endif
