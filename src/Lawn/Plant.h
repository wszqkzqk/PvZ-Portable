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

#pragma once

#include <cstdint>
#include <string>
#include "GameObject.h"

#define MAX_MAGNET_ITEMS 5

enum PlantSubClass : int32_t
{
	SUBCLASS_NORMAL = 0,
	SUBCLASS_SHOOTER = 1
};

enum PlantWeapon : int32_t
{
	WEAPON_PRIMARY,
	WEAPON_SECONDARY
};

enum PlantOnBungeeState : int32_t
{
	NOT_ON_BUNGEE,
	GETTING_GRABBED_BY_BUNGEE,
	RISING_WITH_BUNGEE
};

enum PlantState : int32_t
{
	STATE_NOTREADY,
	STATE_READY,
	STATE_DOINGSPECIAL,
	STATE_SQUASH_LOOK,
	STATE_SQUASH_PRE_LAUNCH,
	STATE_SQUASH_RISING,
	STATE_SQUASH_FALLING,
	STATE_SQUASH_DONE_FALLING,
	STATE_GRAVEBUSTER_LANDING,
	STATE_GRAVEBUSTER_EATING,
	STATE_CHOMPER_BITING,
	STATE_CHOMPER_BITING_GOT_ONE,
	STATE_CHOMPER_BITING_MISSED,
	STATE_CHOMPER_DIGESTING,
	STATE_CHOMPER_SWALLOWING,
	STATE_POTATO_RISING,
	STATE_POTATO_ARMED,
	STATE_POTATO_MASHED,
	STATE_SPIKEWEED_ATTACKING,
	STATE_SPIKEWEED_ATTACKING_2,
	STATE_SCAREDYSHROOM_LOWERING,
	STATE_SCAREDYSHROOM_SCARED,
	STATE_SCAREDYSHROOM_RAISING,
	STATE_SUNSHROOM_SMALL,
	STATE_SUNSHROOM_GROWING,
	STATE_SUNSHROOM_BIG,
	STATE_MAGNETSHROOM_SUCKING,
	STATE_MAGNETSHROOM_CHARGING,
	STATE_BOWLING_UP,
	STATE_BOWLING_DOWN,
	STATE_CACTUS_LOW,
	STATE_CACTUS_RISING,
	STATE_CACTUS_HIGH,
	STATE_CACTUS_LOWERING,
	STATE_TANGLEKELP_GRABBING,
	STATE_COBCANNON_ARMING,
	STATE_COBCANNON_LOADING,
	STATE_COBCANNON_READY,
	STATE_COBCANNON_FIRING,
	STATE_KERNELPULT_BUTTER,
	STATE_UMBRELLA_TRIGGERED,
	STATE_UMBRELLA_REFLECTING,
	STATE_IMITATER_MORPHING,
	STATE_ZEN_GARDEN_WATERED,
	STATE_ZEN_GARDEN_NEEDY,
	STATE_ZEN_GARDEN_HAPPY,
	STATE_MARIGOLD_ENDING,
	STATE_FLOWERPOT_INVULNERABLE,
	STATE_LILYPAD_INVULNERABLE
};

enum PLANT_LAYER : int32_t
{
	PLANT_LAYER_BELOW = -1,
	PLANT_LAYER_MAIN,
	PLANT_LAYER_REANIM,
	PLANT_LAYER_REANIM_HEAD,
	PLANT_LAYER_REANIM_BLINK,
	PLANT_LAYER_ON_TOP,
	NUM_PLANT_LAYERS
};

enum PLANT_ORDER : int32_t
{
	PLANT_ORDER_LILYPAD,
	PLANT_ORDER_NORMAL,
	PLANT_ORDER_PUMPKIN,
	PLANT_ORDER_FLYER,
	PLANT_ORDER_CHERRYBOMB
};

enum MagnetItemType : int32_t
{
	MAGNET_ITEM_NONE,
	MAGNET_ITEM_PAIL_1,
	MAGNET_ITEM_PAIL_2,
	MAGNET_ITEM_PAIL_3,
	MAGNET_ITEM_FOOTBALL_HELMET_1,
	MAGNET_ITEM_FOOTBALL_HELMET_2,
	MAGNET_ITEM_FOOTBALL_HELMET_3,
	MAGNET_ITEM_DOOR_1,
	MAGNET_ITEM_DOOR_2,
	MAGNET_ITEM_DOOR_3,
	//MAGNET_ITEM_PROPELLER,
	MAGNET_ITEM_POGO_1,
	MAGNET_ITEM_POGO_2,
	MAGNET_ITEM_POGO_3,
	MAGNET_ITEM_JACK_IN_THE_BOX,
	MAGNET_ITEM_LADDER_1,
	MAGNET_ITEM_LADDER_2,
	MAGNET_ITEM_LADDER_3,
	MAGNET_ITEM_LADDER_PLACED,
	MAGNET_ITEM_SILVER_COIN,
	MAGNET_ITEM_GOLD_COIN,
	MAGNET_ITEM_DIAMOND,
	MAGNET_ITEM_PICK_AXE
};

class MagnetItem
{
public:
	float                   mPosX;
	float                   mPosY;
	float                   mDestOffsetX;
	float                   mDestOffsetY;
	MagnetItemType          mItemType;
};

class Coin;
class Zombie;
class Reanimation;
class PvzpParticleSystem;

class Plant : public GameObject
{
public:
	SeedType                mSeedType;
	int32_t                 mPlantCol;
	int32_t                 mAnimCounter;
	int32_t                 mFrame;
	int32_t                 mFrameLength;
	int32_t                 mNumFrames;
	PlantState              mState;
	int32_t                 mPlantHealth;
	int32_t                 mPlantMaxHealth;
	int32_t                 mSubclass;
	int32_t                 mDisappearCountdown;
	int32_t                 mDoSpecialCountdown;
	int32_t                 mStateCountdown;
	int32_t                 mLaunchCounter;
	int32_t                 mLaunchRate;
	Rect                    mPlantRect;
	Rect                    mPlantAttackRect;
	int32_t                 mTargetX;
	int32_t                 mTargetY;
	int32_t                 mStartRow;
	ParticleSystemID        mParticleID;
	int32_t                 mShootingCounter;
	ReanimationID           mBodyReanimID;
	ReanimationID           mHeadReanimID;
	ReanimationID           mHeadReanimID2;
	ReanimationID           mHeadReanimID3;
	ReanimationID           mBlinkReanimID;
	ReanimationID           mLightReanimID;
	ReanimationID           mSleepingReanimID;
	int32_t                 mBlinkCountdown;
	int32_t                 mRecentlyEatenCountdown;
	int32_t                 mEatenFlashCountdown;
	int32_t                 mBeghouledFlashCountdown;
	float                   mShakeOffsetX;
	float                   mShakeOffsetY;
	MagnetItem              mMagnetItems[MAX_MAGNET_ITEMS];
	ZombieID                mTargetZombieID;
	int32_t                 mWakeUpCounter;
	PlantOnBungeeState      mOnBungeeState;
	SeedType                mImitaterType;
	int32_t                 mPottedPlantIndex;
	bool                    mAnimPing;
	bool                    mDead;
	bool                    mSquished;
	bool                    mIsAsleep;
	bool                    mIsOnBoard;
	bool                    mHighlighted;

public:
	Plant();

	void                    PlantInitialize(int theGridX, int theGridY, SeedType theSeedType, SeedType theImitaterType);
	void                    Update();
	void                    Animate();
	void                    Draw(Graphics* g);
	void                    MouseDown(int x, int y, int theClickCount);
	void                    DoSpecial();
	void                    Fire(Zombie* theTargetZombie, int theRow, PlantWeapon thePlantWeapon = PlantWeapon::WEAPON_PRIMARY);
	Zombie*                 FindTargetZombie(int theRow, PlantWeapon thePlantWeapon = PlantWeapon::WEAPON_PRIMARY);
	void                    Die();
	void                    UpdateProductionPlant();
	void                    UpdateShooter();
	bool                    FindTargetAndFire(int theRow, PlantWeapon thePlantWeapon = PlantWeapon::WEAPON_PRIMARY);
	void                    LaunchThreepeater();
	static Image*           GetImage(SeedType theSeedType);
	static int              GetCost(SeedType theSeedType, SeedType theImitaterType = SeedType::SEED_NONE);
	static std::string       GetNameString(SeedType theSeedType, SeedType theImitaterType = SeedType::SEED_NONE);
	static std::string       GetToolTip(SeedType theSeedType);
	static int              GetRefreshTime(SeedType theSeedType, SeedType theImitaterType = SeedType::SEED_NONE);
	static bool  IsNocturnal(SeedType theSeedtype);
	static bool  IsFungus(SeedType theSeedType);
	static bool  IsAquatic(SeedType theSeedType);
	static bool  IsFlying(SeedType theSeedtype);
	static bool  IsUpgrade(SeedType theSeedtype);
	void                    UpdateAbilities();
	void                    Squish();
	void                    DoRowAreaDamage(int theDamage, unsigned int theDamageFlags);
	int                     GetDamageRangeFlags(PlantWeapon thePlantWeapon = PlantWeapon::WEAPON_PRIMARY);
	Rect                    GetPlantRect();
	Rect                    GetPlantAttackRect(PlantWeapon thePlantWeapon = PlantWeapon::WEAPON_PRIMARY);
	Zombie*                 FindSquashTarget();
	void                    UpdateSquash();
	bool         NotOnGround();
	void                    DoSquashDamage();
	void                    BurnRow(int theRow);
	void                    IceZombies();
	void                    BlowAwayFliers();
	void                    UpdateGraveBuster();
	PvzpParticleSystem*      AddAttachedParticle(int thePosX, int thePosY, int theRenderPosition, ParticleEffect theEffect);
	void                    GetPeaHeadOffset(int& theOffsetX, int& theOffsetY);
	bool         MakesSun();
	static void             DrawSeedType(Graphics* g, SeedType theSeedType, SeedType theImitaterType, DrawVariation theDrawVariation, float thePosX, float thePosY);
	void                    KillAllPlantsNearDoom();
	bool                    IsOnHighGround();
	void                    UpdateTorchwood();
	void                    LaunchStarFruit();
	bool                    FindStarFruitTarget();
	void                    UpdateChomper();
	void                    DoBlink();
	void                    UpdateBlink();
	void                    PlayBodyReanim(const char* theTrackName, ReanimLoopType theLoopType, int theBlendTime, float theAnimRate);
	void                    UpdateMagnetShroom();
	MagnetItem*             GetFreeMagnetItem();
	void                    DrawMagnetItems(Graphics* g);
	void                    UpdateDoomShroom();
	void                    UpdateIceShroom();
	void                    UpdatePotato();
	int                     CalcRenderOrder();
	void                    AnimateNuts();
	void                    SetSleeping(bool theIsAsleep);
	void                    UpdateShooting();
	void                    DrawShadow(Graphics* g, float theOffsetX, float theOffsetY);
	void                    UpdateScaredyShroom();
	int                     DistanceToClosestZombie();
	void                    UpdateSpikeweed();
	void                    MagnetShroomAttactItem(Zombie* theZombie);
	void                    UpdateSunShroom();
	void                    UpdateBowling();
	void                    AnimatePumpkin();
	void                    UpdateBlover();
	void                    UpdateCactus();
	void                    StarFruitFire();
	void                    UpdateTanglekelp();
	Reanimation*            AttachBlinkAnim(Reanimation* theReanimBody);
	void                    UpdateReanimColor();
	bool                    IsUpgradableTo(SeedType theUpgradedType);
	bool                    IsPartOfUpgradableTo(SeedType theUpgradedType);
	void                    UpdateCobCannon();
	void                    CobCannonFire(int theTargetX, int theTargetY);
	void                    UpdateGoldMagnetShroom();
	bool         IsOnBoard();
	void                    RemoveEffects();
	void                    UpdateCoffeeBean();
	void                    UpdateUmbrella();
	void                    EndBlink();
	void                    AnimateGarlic();
	Coin*                   FindGoldMagnetTarget();
	void                    SpikeweedAttack();
	void                    ImitaterMorph();
	void                    UpdateImitater();
	void                    UpdateReanim();
	void                    SpikeRockTakeDamage();
	bool                    IsSpiky();
	static void  PreloadPlantResources(SeedType theSeedType);
	bool         IsInPlay();
	void                    UpdateNeedsFood() { ; }
	void                    PlayIdleAnim(float theRate);
	void                    UpdateFlowerPot();
	void                    UpdateLilypad();
	void                    GoldMagnetFindTargets();
	bool                    IsAGoldMagnetAboutToSuck();
	bool                    DrawMagnetItemsOnTop();
};

float                       PlantDrawHeightOffset(Board* theBoard, Plant* thePlant, SeedType theSeedType, int theCol, int theRow);
float                       PlantFlowerPotHeightOffset(SeedType theSeedType, float theFlowerPotScale);

class PlantDefinition
{
public:
	SeedType                mSeedType;
	Image**                 mPlantImage;
	ReanimationType         mReanimationType;
	int                     mPacketIndex;
	int                     mSeedCost;
	int                     mRefreshTime;
	PlantSubClass           mSubClass;
	int                     mLaunchRate;
	const char*         mPlantName;
};
extern const PlantDefinition gPlantDefs[SeedType::NUM_SEED_TYPES];

const PlantDefinition& GetPlantDefinition(SeedType theSeedType);
