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

#ifndef __CURSOROBJECT_H__
#define __CURSOROBJECT_H__

#include <cstdint>
#include "GameObject.h"

class CursorObject : public GameObject
{
public:
	int32_t					mSeedBankIndex;
	SeedType				mType;
	SeedType				mImitaterType;
	CursorType				mCursorType;
	CoinID					mCoinID;
	PlantID					mGlovePlantID;
	PlantID					mDuplicatorPlantID;
	PlantID					mCobCannonPlantID;
	int32_t					mHammerDownCounter;
	ReanimationID			mReanimCursorID;

public:
	CursorObject(Board* theBoard);

	void					Update();
	void					Draw(Graphics* g);
	void					Die();
};

class CursorPreview : public GameObject
{
public:
	int32_t					mGridX;
	int32_t					mGridY;

public:
	CursorPreview(Board* theBoard);

	void					Update();
	void					Draw(Graphics* g);
};

#endif
