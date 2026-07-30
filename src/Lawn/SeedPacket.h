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

#ifndef __SEEDPACKET_H__
#define __SEEDPACKET_H__

#include <cstdint>
#include "GameObject.h"
#include "../GameConstants.h"

class HitResult;
class SeedPacket : public GameObject
{
public:
    int32_t             mRefreshCounter;
    int32_t             mRefreshTime;
    int32_t             mIndex;
    int32_t             mOffsetX;
    SeedType            mPacketType;
    SeedType            mImitaterType;
    int32_t             mSlotMachineCountDown;
    SeedType            mSlotMachiningNextSeed;
    float               mSlotMachiningPosition;
    bool                mActive;
    bool                mRefreshing;
    int32_t             mTimesUsed;

public:
    SeedPacket();

    void                Update();
    void                Draw(Graphics* g);
    void                MouseDown(int x, int y, int theClickCount);
    bool                MouseHitTest(int theX, int theY, HitResult* theHitResult);
    void                Deactivate();
    void                Activate();
    /*inline*/ void     SetActivate(bool theActivate);
    void                PickNextSlotMachineSeed();
    void                WasPlanted();
    /*inline*/ void     SlotMachineStart();
    void                FlashIfReady();
    bool                CanPickUp();
    void                SetPacketType(SeedType theSeedType, SeedType theImitaterType = SeedType::SEED_NONE);
};

class SeedBank : public GameObject
{
public:
    int32_t			    mNumPackets;
    SeedPacket		    mSeedPackets[SEEDBANK_MAX];
    int32_t			    mCutSceneDarken;
    int32_t			    mConveyorBeltCounter;

public:
    SeedBank();

    void			    Draw(Graphics* g);
    bool			    MouseHitTest(int x, int y, HitResult* theHitResult);
    inline void		    Move(int x, int y) { mX = x; mY = y; }
    bool			    ContainsPoint(int theX, int theY);
    void			    AddSeed(SeedType theSeedType, bool thePlaceOnLeft = false);
    void			    RemoveSeed(int theIndex);
    int				    GetNumSeedsOnConveyorBelt();
    /*inline*/ int		CountOfTypeOnConveyorBelt(SeedType theSeedType);
    void			    UpdateConveyorBelt();
    void			    UpdateWidth();
    void			    RefreshAllPackets();
};

void				    SeedPacketDrawSeed(Graphics* g, float x, float y, SeedType theSeedType, SeedType theImitaterType, float theOffsetX, float theOffsetY, float theScale);
void				    DrawSeedPacket(Graphics* g, float x, float y, SeedType theSeedType, SeedType theImitaterType, float thePercentDark, int theGrayness, bool theDrawCost, bool theUseCurrentCost);


#endif
