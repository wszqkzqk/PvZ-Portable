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

#include <iterator>
#include <memory>
#include <new>
#include "PvzpDebug.h"
#include "PvzpCommon.h"

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
	// Value-initialization zeroes T's members only while this constructor stays implicit.
	struct DataArrayItem : T {};

	std::unique_ptr<DataArrayItem[]>	mItems;
	std::unique_ptr<unsigned int[]>		mItemIds;

public:
	unsigned int			mMaxUsedCount = 0U;
	unsigned int			mMaxSize = 0U;
	unsigned int			mFreeListHead = 0U;
	unsigned int			mSize = 0U;
	unsigned int			mNextKey = 1U;
	const char*				mName = nullptr;

public:
	DataArray() = default;

	~DataArray()
	{
		DataArrayDispose();
	}

	void DataArrayInitialize(unsigned int theMaxSize, const char* theName)
	{
		PVZP_ASSERT(mItems == nullptr);
		mItems = std::make_unique<DataArrayItem[]>(theMaxSize);
		mItemIds = std::make_unique<unsigned int[]>(theMaxSize);
		mMaxSize = theMaxSize;
		mNextKey = 1001U;
		mName = theName;
	}

	void DataArrayDispose()
	{
		DataArrayFreeAll();  // no-ops when uninitialized
		mItems.reset();
		mItemIds.reset();
		mMaxUsedCount = 0U;
		mMaxSize = 0U;
		mFreeListHead = 0U;
		mSize = 0U;
		mName = nullptr;
	}

	void DataArrayFree(T* theItem)
	{
		unsigned int anIndex = static_cast<unsigned int>(static_cast<DataArrayItem*>(theItem) - mItems.get());
		PVZP_ASSERT(DataArrayGet(mItemIds[anIndex]) == theItem, "Failed: DataArrayFree(%p) in %s", (void*)theItem, mName);
		DataArrayResetItemAt(anIndex);
		mItemIds[anIndex] = mFreeListHead;
		mFreeListHead = anIndex;
		mSize--;
	}

	void DataArrayFreeAll()
	{
		for (T* aItem : *this)
			DataArrayFree(aItem);

		mFreeListHead = 0U;
		mMaxUsedCount = 0U;
	}

	inline unsigned int DataArrayGetID(T* theItem)
	{
		unsigned int anIndex = static_cast<unsigned int>(static_cast<DataArrayItem*>(theItem) - mItems.get());
		unsigned int anId = mItemIds[anIndex];
		PVZP_ASSERT(DataArrayGet(anId) == theItem, "Failed: DataArrayGetID(%p) for %s", (void*)theItem, mName);
		return anId;
	}

	// Index-ascending, live-slots-only iteration; end re-checked against the live mMaxUsedCount.
	// The item address is resolved on dereference, so freeing it mid-loop is safe.
	class Iterator
	{
		const DataArray*	mArray;
		unsigned int		mIndex;

	public:
		Iterator(const DataArray* theArray, unsigned int theIndex) : mArray(theArray), mIndex(theIndex) {}

		T* operator*() const
		{
			return &mArray->mItems[mIndex];
		}

		Iterator& operator++()
		{
			do
			{
				mIndex++;
			} while (mIndex < mArray->mMaxUsedCount && !(mArray->mItemIds[mIndex] & DATA_ARRAY_KEY_MASK));
			return *this;
		}

		bool operator==(std::default_sentinel_t) const
		{
			return mIndex >= mArray->mMaxUsedCount;
		}
	};

	Iterator begin() const
	{
		Iterator anIterator(this, 0U);
		if (mMaxUsedCount != 0U && !(mItemIds[0] & DATA_ARRAY_KEY_MASK))
			++anIterator;
		return anIterator;
	}

	std::default_sentinel_t end() const
	{
		return std::default_sentinel;
	}

	T* DataArrayAlloc()
	{
		PVZP_ASSERT(mSize < mMaxSize, "Data array full: %s", mName);
		PVZP_ASSERT(mFreeListHead <= mMaxUsedCount, "DataArrayAlloc error in %s", mName);
		unsigned int aNext = mMaxUsedCount;
		if (mFreeListHead == mMaxUsedCount)
			mFreeListHead = ++mMaxUsedCount;
		else
		{
			aNext = mFreeListHead;
			mFreeListHead = mItemIds[mFreeListHead];
		}

		T& aNewItem = DataArrayResetItemAt(aNext);
		mItemIds[aNext] = (mNextKey++ << DATA_ARRAY_KEY_SHIFT) | aNext;
		if (mNextKey == DATA_ARRAY_MAX_SIZE) mNextKey = 1;
		mSize++;

		return &aNewItem;
	}

	T* DataArrayTryToGet(unsigned int theId)
	{
		if (!theId || (theId & DATA_ARRAY_INDEX_MASK) >= mMaxSize)
			return nullptr;

		unsigned int anIndex = theId & DATA_ARRAY_INDEX_MASK;
		return (mItemIds[anIndex] == theId) ? &mItems[anIndex] : nullptr;
	}

	T* DataArrayGet(unsigned int theId)
	{
		PVZP_ASSERT(DataArrayTryToGet(theId) != nullptr, "Failed: DataArrayGet(0x%x) for %s", theId, mName);
		return &mItems[theId & DATA_ARRAY_INDEX_MASK];
	}

	T& DataArrayGetItemAt(unsigned int theIndex)
	{
		return mItems[theIndex];
	}

	T& DataArrayResetItemAt(unsigned int theIndex)
	{
		DataArrayItem* aItem = &mItems[theIndex];
		std::destroy_at(aItem);
		return *std::construct_at(aItem);
	}

	unsigned int& DataArrayGetIDAt(unsigned int theIndex)
	{
		return mItemIds[theIndex];
	}
};

#endif
