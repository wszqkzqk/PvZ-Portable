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

#ifndef __DATAARRAY_H__
#define __DATAARRAY_H__

#include <memory>
#include <new>
#include "TodDebug.h"
#include "TodCommon.h"

enum 
{
	DATA_ARRAY_INDEX_MASK = 65535,
	DATA_ARRAY_KEY_MASK = -65536,
	DATA_ARRAY_KEY_SHIFT = 16,
	DATA_ARRAY_MAX_SIZE = 65536,
	DATA_ARRAY_KEY_FIRST = 1
};

template <typename T> class DataArray
{
	struct DataArrayItem : T {};

	struct DataArrayStorage
	{
		std::unique_ptr<DataArrayItem[]>	mItems;
		std::unique_ptr<unsigned int[]>	mItemIds;

		DataArrayStorage(unsigned int theMaxSize) :
			mItems(std::make_unique<DataArrayItem[]>(theMaxSize)),
			mItemIds(std::make_unique<unsigned int[]>(theMaxSize))
		{
		}
	};

public:
	DataArrayStorage*		mStorage;
	unsigned int			mMaxUsedCount;
	unsigned int			mMaxSize;
	unsigned int			mFreeListHead;
	unsigned int			mSize;
	unsigned int			mNextKey;
	const char*				mName;

public:
	DataArray()
	{
		mStorage = nullptr;
		mMaxUsedCount = 0U;
		mMaxSize = 0U;
		mFreeListHead = 0U;
		mSize = 0U;
		mNextKey = 1U;
		mName = nullptr;
	}

	~DataArray()
	{
		DataArrayDispose();
	}

	void DataArrayInitialize(unsigned int theMaxSize, const char* theName)
	{
		TOD_ASSERT(mStorage == nullptr);
		mStorage = new DataArrayStorage(theMaxSize);
		mMaxSize = theMaxSize;
		mNextKey = 1001U;
		mName = theName;
	}

	void DataArrayDispose()
	{
		if (mStorage != nullptr)
		{
			DataArrayFreeAll();
			delete mStorage;
			mStorage = nullptr;
			mMaxUsedCount = 0U;
			mMaxSize = 0U;
			mFreeListHead = 0U;
			mSize = 0U;
			mName = nullptr;
		}
	}

	void DataArrayFree(T* theItem)
	{
		DataArrayItem* aItem = static_cast<DataArrayItem*>(theItem);
		unsigned int anIndex = static_cast<unsigned int>(aItem - mStorage->mItems.get());
		TOD_ASSERT(DataArrayGet(mStorage->mItemIds[anIndex]) == theItem, "Failed: DataArrayFree(0x%x) in %s", theItem, mName);
		DataArrayResetItemAt(anIndex);
		mStorage->mItemIds[anIndex] = mFreeListHead;
		mFreeListHead = anIndex;
		mSize--;
	}

	void DataArrayFreeAll()
	{
		T* aItem = nullptr;
		while (IterateNext(aItem))
			DataArrayFree(aItem);

		mFreeListHead = 0U;
		mMaxUsedCount = 0U;
	}

	inline unsigned int DataArrayGetID(T* theItem)
	{
		DataArrayItem* aItem = static_cast<DataArrayItem*>(theItem);
		unsigned int anIndex = static_cast<unsigned int>(aItem - mStorage->mItems.get());
		unsigned int anId = mStorage->mItemIds[anIndex];
		TOD_ASSERT(DataArrayGet(anId) == theItem, "Failed: DataArrayGetID(0x%x) for %s", theItem, mName);
		return anId;
	}

	bool IterateNext(T*& theItem)
	{
		unsigned int anIndex = 0U;
		if (theItem != nullptr)
		{
			DataArrayItem* aItem = static_cast<DataArrayItem*>(std::launder(theItem));
			anIndex = static_cast<unsigned int>(aItem - mStorage->mItems.get()) + 1U;
		}

		while (anIndex < mMaxUsedCount)
		{
			if (mStorage->mItemIds[anIndex] & DATA_ARRAY_KEY_MASK)
			{
				theItem = &mStorage->mItems[anIndex];
				return true;
			}
			anIndex++;
		}
		return false;
	}

	T* DataArrayAlloc()
	{
		TOD_ASSERT(mSize < mMaxSize, "Data array full: %s", mName);
		TOD_ASSERT(mFreeListHead <= mMaxUsedCount, "DataArrayAlloc error in %s", mName);
		unsigned int aNext = mMaxUsedCount;
		if (mFreeListHead == mMaxUsedCount)
			mFreeListHead = ++mMaxUsedCount;
		else
		{
			aNext = mFreeListHead;
			mFreeListHead = mStorage->mItemIds[mFreeListHead];
		}

		T& aNewItem = DataArrayResetItemAt(aNext);
		mStorage->mItemIds[aNext] = (mNextKey++ << DATA_ARRAY_KEY_SHIFT) | aNext;
		if (mNextKey == DATA_ARRAY_MAX_SIZE) mNextKey = 1;
		mSize++;

		return &aNewItem;
	}

	T* DataArrayTryToGet(unsigned int theId)
	{
		if (!theId || (theId & DATA_ARRAY_INDEX_MASK) >= mMaxSize)
			return nullptr;

		unsigned int anIndex = theId & DATA_ARRAY_INDEX_MASK;
		return (mStorage->mItemIds[anIndex] == theId) ? &mStorage->mItems[anIndex] : nullptr;
	}

	T* DataArrayGet(unsigned int theId)
	{
		TOD_ASSERT(DataArrayTryToGet(theId) != nullptr, "Failed: DataArrayGet(0x%x) for %s", theId, mName);
		return &mStorage->mItems[theId & DATA_ARRAY_INDEX_MASK];
	}

	T& DataArrayGetItemAt(unsigned int theIndex)
	{
		return mStorage->mItems[theIndex];
	}

	T& DataArrayResetItemAt(unsigned int theIndex)
	{
		DataArrayItem* aItem = &mStorage->mItems[theIndex];
		std::destroy_at(aItem);
		return *std::construct_at(aItem);
	}

	unsigned int& DataArrayGetIDAt(unsigned int theIndex)
	{
		return mStorage->mItemIds[theIndex];
	}
};

#endif
