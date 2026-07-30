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

#ifndef __ZENGARDEN_H__
#define __ZENGARDEN_H__

#include "../ConstEnums.h"
#include <vector>
#include <string>
#include <stdint.h>
#include <time.h>

#define ZEN_MAX_GRIDSIZE_X 8
#define ZEN_MAX_GRIDSIZE_Y 4

class LawnApp;
class Board;
class Plant;
class GridItem;
class HitResult;
class PottedPlant;
namespace Sexy
{
    class Graphics;
}
using namespace Sexy;

class SpecialGridPlacement
{
public:
    int                     mPixelX;
    int                     mPixelY;
    int                     mGridX;
    int                     mGridY;
};

class ZenGarden
{
public:
    LawnApp*                mApp;
    Board*                  mBoard;
    GardenType              mGardenType;
	std::vector<std::string> mLoadedResourceNames;
    time_t                  mNowTime;       // cached per-frame
    tm                      mNowTM;         // cached per-frame

public:
    ZenGarden();
    ~ZenGarden();

    void                    ZenGardenInitLevel();
    /*inline*/ void         DrawPottedPlantIcon(Graphics* g, float x, float y, PottedPlant* thePottedPlant);
    void                    DrawPottedPlant(Graphics* g, float x, float y, PottedPlant* thePottedPlant, float theScale, bool theDrawPot);
    bool                    IsZenGardenFull(bool theIncludeDroppedPresents);
    void                    FindOpenZenGardenSpot(int& theSpotX, int& theSpotY);
    void                    AddPottedPlant(PottedPlant* thePottedPlant);
    void                    MouseDownWithTool(int x, int y, CursorType theCursorType);
    void                    MovePlant(Plant* thePlant, int theGridX, int theGridY);
    void                    MouseDownWithMoneySign(Plant* thePlant);
    Plant*                  PlacePottedPlant(intptr_t thePottedPlantIndex);
    float                   PlantPottedDrawHeightOffset(SeedType theSeedType, float theScale);
    static float            ZenPlantOffsetX(PottedPlant* thePottedPlant);
    int                     GetPlantSellPrice(Plant* thePlant);
    void                    ZenGardenUpdate();
    void                    MouseDownWithFullWheelBarrow(int x, int y);
    void                    MouseDownWithEmptyWheelBarrow(Plant* thePlant);
    void                    GotoNextGarden();
    /*inline*/ PottedPlant* GetPottedPlantInWheelbarrow();
    void                    RemovePottedPlant(Plant* thePlant);
    const SpecialGridPlacement*   GetSpecialGridPlacements(int& theCount);
    int                     PixelToGridX(int theX, int theY);
    int                     PixelToGridY(int theX, int theY);
    int                     GridToPixelX(int theGridX, int theGridY);
    int                     GridToPixelY(int theGridX, int theGridY);
    void                    DrawBackdrop(Graphics* g);
    bool                    MouseDownZenGarden(int x, int y, int theClickCount, HitResult* theHitResult);
    void                    PlantFulfillNeed(Plant* thePlant);
    void                    PlantWatered(Plant* thePlant);
    PottedPlantNeed         GetPlantsNeed(PottedPlant* thePottedPlant);
    void                    MouseDownWithFeedingTool(int x, int y, CursorType theCursorType);
    void                    DrawPlantOverlay(Graphics* g, Plant* thePlant);
    PottedPlant*            PottedPlantFromIndex(intptr_t thePottedPlantIndex);
    bool                    WasPlantNeedFulfilledToday(PottedPlant* thePottedPlant);
    void                    PottedPlantUpdate(Plant* thePlant);
    void                    AddHappyEffect(Plant* thePlant);
    void                    RemoveHappyEffect(Plant* thePlant);
    void                    PlantUpdateProduction(Plant* thePlant);
    bool                    CanDropPottedPlantLoot();
    void                    ShowTutorialArrowOnWateringCan();
    bool                    PlantsNeedWater();
    void                    ZenGardenStart();
    void                    UpdatePlantEffectState(Plant* thePlant);
    bool                    CanUseGameObject(GameObjectType);
    void                    ZenToolUpdate(GridItem* theZenTool);
    void                    DoFeedingTool(int x, int y, GridItemState theToolType);
    void                    AddStinky();
    void                    StinkyUpdate(GridItem* theStinky);
    void                    OpenStore();
    GridItem*               GetStinky();
    void                    StinkyPickGoal(GridItem* theStinky);
    bool                    PlantShouldRefreshNeed(PottedPlant* thePottedPlant);
    void                    PlantFertilized(Plant* thePlant);
    bool                    WasPlantFertilizedInLastHour(PottedPlant* thePottedPlant);
    void                    SetupForZenTutorial();
    bool                    HasPurchasedStinky();
    int                     CountPlantsNeedingFertilizer();
    bool                    AllPlantsHaveBeenFertilized();
    void                    WakeStinky();
    bool                    ShouldStinkyBeAwake();
    bool                    IsStinkySleeping();
    static SeedType         PickRandomSeedType();
    void                    StinkyWakeUp(GridItem* theStinky);
    void                    StinkyStartFallingAsleep(GridItem* theStinky);
    void                    StinkyFinishFallingAsleep(GridItem* theStinky, int theBlendTime);
    void                    AdvanceCrazyDaveDialog();
    void                    LeaveGarden();
    bool                    CanDropChocolate();
    void                    FeedChocolateToPlant(Plant* thePlant);
    bool                    PlantHighOnChocolate(PottedPlant* thePottedPlant);
    bool                    PlantCanHaveChocolate(Plant* thePlant);
    void                    SetPlantAnimSpeed(Plant* thePlant);
    void                    UpdateStinkyMotionTrail(GridItem* theStinky, bool theStinkyHighOnChocolate);
    void                    ResetPlantTimers(PottedPlant* thePottedPlant);
    void                    ResetStinkyTimers();
    void                    UpdatePlantNeeds();
    void                    RefreshPlantNeeds(PottedPlant* thePottedPlant);
    void                    PlantSetLaunchCounter(Plant* thePlant);
    int                     PlantGetMinutesSinceHappy(Plant* thePlant);
    /*inline*/ bool         IsStinkyHighOnChocolate();
    void                    StinkyAnimRateUpdate(GridItem* theStinky);
    /*inline*/ bool         PlantCanBeWatered(Plant* thePlant);
};

#endif
