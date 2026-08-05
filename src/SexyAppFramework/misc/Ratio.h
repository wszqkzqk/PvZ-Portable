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

#ifndef __RATIO_H__
#define __RATIO_H__

namespace Sexy
{
	struct Ratio
	{
		Ratio();
		Ratio(int theNumerator, int theDenominator);
		void Set(int theNumerator, int theDenominator);
		bool operator==(const Ratio& theRatio) const;
		bool operator!=(const Ratio& theRatio) const;
		bool operator<(const Ratio& theRatio) const;
		int operator*(int theInt) const;
		int operator/(int theInt) const;
		int mNumerator;
		int mDenominator;
	};

	inline bool Ratio::operator==(const Ratio& theRatio) const
	{
		return mNumerator == theRatio.mNumerator && mDenominator == theRatio.mDenominator;
	}

	inline bool Ratio::operator!=(const Ratio& theRatio) const
	{
		return ! (*this == theRatio);
	}

	inline bool Ratio::operator<(const Ratio& theRatio) const
	{
		return (mNumerator*theRatio.mDenominator/mDenominator < theRatio.mNumerator)
			|| (mNumerator < theRatio.mNumerator*mDenominator/theRatio.mDenominator);
	}

	inline int Ratio::operator*(int theInt) const
	{
		return theInt * mNumerator / mDenominator;
	}

	inline int Ratio::operator/(int theInt) const
	{
		return theInt * mDenominator / mNumerator;
	}

	inline int operator*(int theInt, const Ratio& theRatio)
	{
		return theInt * theRatio.mNumerator / theRatio.mDenominator;
	}

	inline int operator/(int theInt, const Ratio& theRatio)
	{
		return theInt * theRatio.mDenominator / theRatio.mNumerator;
	}

}

#endif
