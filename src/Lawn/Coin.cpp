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

#include "Coin.h"
#include "Board.h"
#include <algorithm>
#include "Cutscene.h"
#include "ZenGarden.h"
#include "Challenge.h"
#include "../LawnApp.h"
#include "SeedPacket.h"
#include "CursorObject.h"
#include "../Resources.h"
#include "MessageWidget.h"
#include "../GameConstants.h"
#include "System/PlayerInfo.h"
#include "../Sexy.TodLib/TodFoley.h"
#include "../Sexy.TodLib/TodDebug.h"
#include "../Sexy.TodLib/Reanimator.h"
#include "../Sexy.TodLib/Attachment.h"
#include "Widget/AchievementsScreen.h"

Coin::Coin()
{
}

Coin::~Coin()
{
	AttachmentDie(mAttachmentID);
}

void Coin::CoinInitialize(int theX, int theY, CoinType theCoinType, CoinMotion theCoinMotion)
{
	mPosX = theX;
	mPosY = theY;
	mType = theCoinType;
	mCollectionDistance = 0.0f;
	mDead = false;
	mWidth = 60;
	mHeight = 60;
	mDisappearCounter = 0;
	mIsBeingCollected = false;
	mFadeCount = 0;
	mCoinMotion = theCoinMotion;
	mCoinAge = 0;
	mAttachmentID = AttachmentID::ATTACHMENTID_NULL;
	mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_COIN_BANK, 0, 1);
    mScale = 1.0f;
	mUsableSeedType = SeedType::SEED_NONE;
	mNeedsBouncyArrow = false;
	mHasBouncyArrow = false;
	mHitGround = false;
	mTimesDropped = 0;
	mPottedPlantSpec.InitializePottedPlant(SeedType::SEED_NONE);

    if (IsSun())
    {
        float aPosX = mWidth * 0.5f;
        float aPosY = mHeight * 0.5f;
        Reanimation* aSunReanim = mApp->AddReanimation(0.0f, 0.0f, 0, ReanimationType::REANIM_SUN);
        aSunReanim->SetPosition(mPosX + aPosX, mPosY + aPosY);
        aSunReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
        aSunReanim->mAnimRate = 6.0f;
        AttachReanim(mAttachmentID, aSunReanim, aPosX, aPosY);
    }
    else if (mType == CoinType::COIN_SILVER)
    {
        mPosX -= 10.0f;
        mPosY -= 8.0f;

        float aPosX = 9.0f;
        float aPosY = 9.0f;
        Reanimation* aCoinReanim = mApp->AddReanimation(0.0f, 0.0f, 0, ReanimationType::REANIM_COIN_SILVER);
        aCoinReanim->SetPosition(mPosX + aPosX, mPosY + aPosY);
        aCoinReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
        aCoinReanim->mAnimTime = RandRangeFloat(0.0f, 0.99f);
        aCoinReanim->mAnimRate *= RandRangeFloat(0.6f, 1.0f);
        AttachReanim(mAttachmentID, aCoinReanim, aPosX, aPosY);
    }
    else if (mType == CoinType::COIN_GOLD)
    {
        mPosX -= 10.0f;
        mPosY -= 8.0f;

        float aPosX = 9.0f;
        float aPosY = 9.0f;
        Reanimation* aCoinReanim = mApp->AddReanimation(0.0f, 0.0f, 0, ReanimationType::REANIM_COIN_GOLD);
        aCoinReanim->SetPosition(mPosX + aPosX, mPosY + aPosY);
        aCoinReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
        aCoinReanim->mAnimTime = RandRangeFloat(0.0f, 0.99f);
        aCoinReanim->mAnimRate *= RandRangeFloat(0.6f, 1.0f);
        AttachReanim(mAttachmentID, aCoinReanim, aPosX, aPosY);
    }
    else if (mType == CoinType::COIN_DIAMOND)
    {
        mPosX -= 15.0f;
        mPosY -= 15.0f;

        float aPosX = -3.0f;
        float aPosY = 4.0f;
        Reanimation* aCoinReanim = mApp->AddReanimation(0.0f, 0.0f, 0, ReanimationType::REANIM_DIAMOND);
        aCoinReanim->SetPosition(mPosX + aPosX, mPosY + aPosY);
        aCoinReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
        aCoinReanim->mAnimTime = RandRangeFloat(0.0f, 0.99f);
        aCoinReanim->mAnimRate = RandRangeFloat(50.0f, 80.0f);
        AttachReanim(mAttachmentID, aCoinReanim, aPosX, aPosY);
    }

    if (mApp->IsStormyNightLevel())
    {
        mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 0);
    }

    if (mType == CoinType::COIN_FINAL_SEED_PACKET)
    {
        mWidth = IMAGE_SEEDS->GetCelWidth();
        mHeight = IMAGE_SEEDS->GetCelHeight();
        mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 0);
    }
    else if (mType == CoinType::COIN_TROPHY)
    {
        mWidth = IMAGE_TROPHY->GetCelWidth();
        mHeight = IMAGE_TROPHY->GetCelHeight();
        mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 0);
    }
    else if (mType == CoinType::COIN_AWARD_SILVER_SUNFLOWER || mType == CoinType::COIN_AWARD_GOLD_SUNFLOWER)
    {
        mWidth = IMAGE_SUNFLOWER_TROPHY->GetCelWidth();
        mHeight = IMAGE_SUNFLOWER_TROPHY->GetCelHeight();
        mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 0);
    }
    else if (mType == CoinType::COIN_SHOVEL)
    {
        mWidth = IMAGE_SHOVEL->GetCelWidth();
        mHeight = IMAGE_SHOVEL->GetCelHeight();
        mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 0);
    }
    else if (mType == CoinType::COIN_CARKEYS)
    {
        mWidth = IMAGE_CARKEYS->GetCelWidth();
        mHeight = IMAGE_CARKEYS->GetCelHeight();
        mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 0);
    }
    else if (mType == CoinType::COIN_ALMANAC)
    {
        mWidth = IMAGE_ALMANAC->GetCelWidth();
        mHeight = IMAGE_ALMANAC->GetCelHeight();
        mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 0);
    }
    else if (mType == CoinType::COIN_VASE)
    {
        mWidth = IMAGE_SCARY_POT->GetCelWidth();
        mHeight = IMAGE_SCARY_POT->GetCelHeight();
        mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 0);
    }
    else if (mType == CoinType::COIN_WATERING_CAN)
    {
        mWidth = IMAGE_WATERINGCAN->GetCelWidth();
        mHeight = IMAGE_WATERINGCAN->GetCelHeight();
        mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 0);
    }
    else if (mType == CoinType::COIN_TACO)
    {
        mWidth = IMAGE_TACO->GetCelWidth();
        mHeight = IMAGE_TACO->GetCelHeight();
        mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 0);
    }
    else if (mType == CoinType::COIN_NOTE)
    {
        mWidth = IMAGE_ZOMBIE_NOTE_SMALL->GetCelWidth();
        mHeight = IMAGE_ZOMBIE_NOTE_SMALL->GetCelHeight();
        mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 0);
    }
    else if (mType == CoinType::COIN_USABLE_SEED_PACKET)
    {
        mWidth = IMAGE_SEEDS->GetCelWidth();
        mHeight = IMAGE_SEEDS->GetCelHeight();
        mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_FOG, 0, 2);
    }
    else if (mType == CoinType::COIN_PRESENT_PLANT || mType == CoinType::COIN_AWARD_PRESENT)
    {
        TOD_ASSERT(mBoard);

        mWidth = IMAGE_PRESENT->GetCelWidth();
        mHeight = IMAGE_PRESENT->GetCelHeight();
        if (mApp->IsSurvivalEndless(mApp->mGameMode) || mApp->IsEndlessIZombie(mApp->mGameMode) || mApp->IsEndlessScaryPotter(mApp->mGameMode))
        {
            SeedType aSeedType = mApp->mZenGarden->PickRandomSeedType();
            mPottedPlantSpec.InitializePottedPlant(aSeedType);
        }
        else if (mBoard->mBackground == BackgroundType::BACKGROUND_1_DAY)
        {
            SeedType aSeedList[] = {
                SeedType::SEED_PEASHOOTER,
                SeedType::SEED_SUNFLOWER,
                SeedType::SEED_CHERRYBOMB,
                SeedType::SEED_WALLNUT,
                SeedType::SEED_REPEATER,
                SeedType::SEED_POTATOMINE,
                SeedType::SEED_SNOWPEA,
                SeedType::SEED_CHOMPER
            };
            
            SeedType aSeedType = TodPickFromArray(aSeedList, LENGTH(aSeedList));
            mPottedPlantSpec.InitializePottedPlant(aSeedType);
        }
        else if (mBoard->mBackground == BackgroundType::BACKGROUND_2_NIGHT)
        {
            SeedType aSeedList[] = {
                SeedType::SEED_PUFFSHROOM,
                SeedType::SEED_SUNSHROOM,
                SeedType::SEED_FUMESHROOM,
                SeedType::SEED_GRAVEBUSTER,
                SeedType::SEED_HYPNOSHROOM,
                SeedType::SEED_SCAREDYSHROOM,
                SeedType::SEED_ICESHROOM,
                SeedType::SEED_DOOMSHROOM
            };

            SeedType aSeedType = TodPickFromArray(aSeedList, LENGTH(aSeedList));
            mPottedPlantSpec.InitializePottedPlant(aSeedType);
        }
        else if (mBoard->mBackground == BackgroundType::BACKGROUND_3_POOL)
        {
            SeedType aSeedList[] = {
                SeedType::SEED_LILYPAD,
                SeedType::SEED_SQUASH,
                SeedType::SEED_THREEPEATER,
                SeedType::SEED_TANGLEKELP,
                SeedType::SEED_JALAPENO,
                SeedType::SEED_SPIKEWEED,
                SeedType::SEED_TORCHWOOD,
                SeedType::SEED_TALLNUT
            };

            SeedType aSeedType = TodPickFromArray(aSeedList, LENGTH(aSeedList));
            mPottedPlantSpec.InitializePottedPlant(aSeedType);
        }
        else if (mBoard->mBackground == BackgroundType::BACKGROUND_4_FOG)
        {
            SeedType aSeedList[] = {
                SeedType::SEED_SEASHROOM,
                SeedType::SEED_PLANTERN,
                SeedType::SEED_CACTUS,
                SeedType::SEED_BLOVER,
                SeedType::SEED_SPLITPEA,
                SeedType::SEED_STARFRUIT,
                SeedType::SEED_PUMPKINSHELL,
                SeedType::SEED_MAGNETSHROOM
            };

            SeedType aSeedType = TodPickFromArray(aSeedList, LENGTH(aSeedList));
            mPottedPlantSpec.InitializePottedPlant(aSeedType);
        }
        else if (mBoard->mBackground == BackgroundType::BACKGROUND_5_ROOF)
        {
            SeedType aSeedList[] = {
                SeedType::SEED_CABBAGEPULT,
                SeedType::SEED_KERNELPULT,
                SeedType::SEED_INSTANT_COFFEE,
                SeedType::SEED_GARLIC,
                SeedType::SEED_UMBRELLA,
                SeedType::SEED_MELONPULT
            };

            SeedType aSeedType = TodPickFromArray(aSeedList, LENGTH(aSeedList));
            mPottedPlantSpec.InitializePottedPlant(aSeedType);
        }
        else
        {
            SeedType aSeedType = mApp->mZenGarden->PickRandomSeedType();
            mPottedPlantSpec.InitializePottedPlant(aSeedType);
        }
        
    }
    else if (mType == CoinType::COIN_AWARD_MONEY_BAG || mType == CoinType::COIN_AWARD_BAG_DIAMOND)
    {
        mWidth = IMAGE_MONEYBAG->GetCelWidth();
        mHeight = IMAGE_MONEYBAG->GetCelHeight();
        mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 0);
    }
    else if (mType == CoinType::COIN_CHOCOLATE || mType == CoinType::COIN_AWARD_CHOCOLATE)
    {
        mWidth = IMAGE_CHOCOLATE->GetCelWidth();
        mHeight = IMAGE_CHOCOLATE->GetCelHeight();
        mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 0);
    }
    else if (IsPresentWithAdvice())
    {
        mWidth = IMAGE_PRESENT->GetCelWidth();
        mHeight = IMAGE_PRESENT->GetCelHeight();
        mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 0);
    }

    switch (mCoinMotion)
    {
    case CoinMotion::COIN_MOTION_FROM_SKY:
        mVelY = 0.67f;
        mVelX = 0.0f;
        mGroundY = Rand(250) + 300;
        break;

    case CoinMotion::COIN_MOTION_FROM_SKY_SLOW:
        mVelY = 0.33f;
        mVelX = 0.0f;
        mGroundY = Rand(250) + 300;
        break;

    case CoinMotion::COIN_MOTION_FROM_PLANT:
        mVelY = -1.7f - RandRangeFloat(0.0f, 1.7f);
        mVelX = -0.4f + RandRangeFloat(0.0f, 0.8f);
        mGroundY = mPosY + 15 + Rand(20);
        mScale = 0.4f;
        break;

    case CoinMotion::COIN_MOTION_COIN:
        mVelY = -3.0f - RandRangeFloat(0.0f, 2.0f);
        mVelX = -0.5f + RandRangeFloat(0.0f, 1.0f);
        mGroundY = mPosY + 45 + Rand(20);
        mGroundY = std::clamp(mGroundY, 80, 521);
        if (mType == CoinType::COIN_AWARD_SILVER_SUNFLOWER || mType == CoinType::COIN_AWARD_GOLD_SUNFLOWER)
        {
            mPosY -= 100.0f;
            mGroundY = mPosY + 45.0f;
            mGroundY = std::min(mGroundY, 400);
        }
        if (mType == CoinType::COIN_FINAL_SEED_PACKET || 
            mType == CoinType::COIN_USABLE_SEED_PACKET || 
            mType == CoinType::COIN_TROPHY || 
            mType == CoinType::COIN_SHOVEL || 
            mType == CoinType::COIN_CARKEYS || 
            mType == CoinType::COIN_ALMANAC || 
            mType == CoinType::COIN_VASE || 
            mType == CoinType::COIN_WATERING_CAN || 
            mType == CoinType::COIN_TACO || 
            mType == CoinType::COIN_NOTE)
        {
            mGroundY -= 30;
        }
        break;

    case CoinMotion::COIN_MOTION_LAWNMOWER_COIN:
        mVelY = 0.0f;
        mVelX = 0.0f;
        mGroundY = 600;
        Collect();
        break;

    case CoinMotion::COIN_MOTION_FROM_PRESENT:
        mVelY = 0.0f;
        mVelX = 0.0f;
        mGroundY = 600;
        mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 1);
        break;

    case CoinMotion::COIN_MOTION_FROM_BOSS:
        mVelY = -5.0f;
        mVelX = -3.0f;
        mPosX = 750.0f;
        mPosY = mType == CoinType::COIN_AWARD_SILVER_SUNFLOWER ? 130.0f : 245.0f;
        mGroundY = mPosY + 40;
        break;

    default:
        TOD_ASSERT(false);
        break;
    }

    float aScale;
    if (mType == CoinType::COIN_SMALLSUN)
    {
        aScale = 0.5f;
    }
    else if (mType == CoinType::COIN_LARGESUN)
    {
        aScale = 2.0f;
    }
    else
    {
        aScale = 1.0f;
    }
    mScale *= aScale;

    if (CoinGetsBouncyArrow())
    {
        mNeedsBouncyArrow = true;
    }

    if (mCoinMotion != CoinMotion::COIN_MOTION_FROM_PRESENT)
    {
        PlayLaunchSound();
    }
}

bool Coin::IsMoney(CoinType theType)
{
    // 这个函数原本是完全没有的，但是由于在 Board 中涉及到，并且感觉能在需要添加新钱币种类的时候提供方便，所以就留下来了
    return theType == CoinType::COIN_SILVER || theType == CoinType::COIN_GOLD || theType == CoinType::COIN_DIAMOND;
}

bool Coin::IsMoney()
{
    return IsMoney(mType);
}

bool Coin::IsSun()
{
    return mType == CoinType::COIN_SUN || mType == CoinType::COIN_SMALLSUN || mType == CoinType::COIN_LARGESUN;
}

bool Coin::IsPresentWithAdvice()
{
    return mType == CoinType::COIN_PRESENT_MINIGAMES || mType == CoinType::COIN_PRESENT_PUZZLE_MODE || mType == CoinType::COIN_PRESENT_SURVIVAL_MODE;
}

// GOTY @Patoke: 0x4336C0
void Coin::ScoreCoin()
{
    Die();

    if (IsSun())
    {
        int aSunValue = GetSunValue();
        mBoard->AddSunMoney(aSunValue);
    }
    else if (IsMoney())
    {
        int aCoinValue = Coin::GetCoinValue(mType);
        mApp->mPlayerInfo->AddCoins(aCoinValue);
        if (mBoard)
        {
            mBoard->mCoinsCollected += aCoinValue;

            // @Patoke: implemented
            if (mType == CoinType::COIN_SILVER || mType == CoinType::COIN_GOLD) {
                mBoard->mLevelCoinsCollected++;
                if (mBoard->mLevelCoinsCollected == 30 && mApp->mPlayerInfo->mCoins != 0)
                    ReportAchievement::GiveAchievement(mApp, PennyPincher, true);
            }
        }
    }

    if (mType == CoinType::COIN_DIAMOND && mBoard)
    {
        mBoard->mDiamondsCollected++;
    }
}

void Coin::StartFade()
{
    mFadeCount = 15;
}

void Coin::UpdateFade()
{
    if (mApp->IsEndlessIZombie(mApp->mGameMode) || mApp->IsEndlessScaryPotter(mApp->mGameMode) || mType == CoinType::COIN_NOTE || !IsLevelAward())
    {
        mFadeCount--;
        if (mFadeCount == 0)
        {
            Die();
        }
    }
}

void Coin::UpdateFall()
{
    if (mCoinMotion == CoinMotion::COIN_MOTION_FROM_PRESENT)
    {
        mPosX += mVelX;
        mPosY += mVelY;
        mVelX *= 0.95f;
        mVelY *= 0.95f;
        if (mCoinAge >= 80)
        {
            Collect();
        }
    }
    else if (mPosY + mVelY < mGroundY)
    {
        mPosY += mVelY;
        if (mCoinMotion == CoinMotion::COIN_MOTION_FROM_PLANT)
        {
            mVelY += 0.09f;
        }
        else if (mCoinMotion == CoinMotion::COIN_MOTION_COIN || mCoinMotion == CoinMotion::COIN_MOTION_FROM_BOSS)
        {
            mVelY += 0.15f;
        }

        mPosX += mVelX;
        if (mPosX > BOARD_WIDTH - mWidth && mCoinMotion != CoinMotion::COIN_MOTION_FROM_BOSS)
        {
            mPosX = BOARD_WIDTH - mWidth;
            mVelX = -0.4f - RandRangeFloat(0.0f, 0.4f);
        }
        else if (mPosX < 0.0f)
        {
            mPosX = 0.0f;
            mVelX = 0.4f + RandRangeFloat(0.0f, 0.4f);
        }
    }
    else
    {
        if (mNeedsBouncyArrow && !mHasBouncyArrow)
        {
            float aParticleOffsetX = mWidth / 2;
            float aParticleOffsetY = mHeight / 2 - 60;
            if (mType == CoinType::COIN_TROPHY)
            {
                aParticleOffsetX += 2.0f;
            }
            else if (mType == CoinType::COIN_AWARD_MONEY_BAG || mType == CoinType::COIN_AWARD_BAG_DIAMOND)
            {
                aParticleOffsetX += 2.0f;
                aParticleOffsetY -= 2.0f;
            }
            else if (mType == CoinType::COIN_AWARD_PRESENT || IsPresentWithAdvice())
            {
                aParticleOffsetY -= 20.0f;
            }
            else if (mType == CoinType::COIN_AWARD_SILVER_SUNFLOWER || mType == CoinType::COIN_AWARD_GOLD_SUNFLOWER)
            {
                aParticleOffsetX -= 6.0f;
                aParticleOffsetY -= 40.0f;
            }
            else if (IsMoney())
            {
                aParticleOffsetX += 12.0f;
                aParticleOffsetY += 21.0f;
            }

            ParticleEffect aEffect;
            if (mType == CoinType::COIN_FINAL_SEED_PACKET)
            {
                aEffect = ParticleEffect::PARTICLE_SEED_PACKET;
            }
            else if (IsMoney())
            {
                aEffect = ParticleEffect::PARTICLE_COIN_PICKUP_ARROW;
            }
            else
            {
                aEffect = ParticleEffect::PARTICLE_AWARD_PICKUP_ARROW;
            }

            TodParticleSystem* aParticle = mApp->AddTodParticle(mPosX + aParticleOffsetX, mPosY + aParticleOffsetY, 0, aEffect);
            AttachParticle(mAttachmentID, aParticle, aParticleOffsetX, aParticleOffsetY);
            mHasBouncyArrow = true;
        }

        if (!mHitGround)
        {
            mHitGround = true;
            PlayGroundSound();
        }

        mPosY = mGroundY;
        mPosX = FloatRoundToInt(mPosX);


        if (mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_LAST_STAND || mBoard == nullptr || 
            mBoard->mChallenge->mChallengeState == ChallengeState::STATECHALLENGE_LAST_STAND_ONSLAUGHT)
        {
            if (!IsLevelAward() && !IsPresentWithAdvice())
            {
                mDisappearCounter++;
                if (mDisappearCounter >= GetDisappearTime())
                {
                    StartFade();
                }
            }
        }
    }

    if (mCoinMotion == CoinMotion::COIN_MOTION_FROM_PLANT)
    {
        float aFinalScale = GetSunScale();
        if (mScale < aFinalScale)
        {
            mScale += 0.02f;
        }
        else
        {
            mScale = aFinalScale;
        }
    }
}

// GOTY @Patoke: 0x433BD0
void Coin::UpdateCollected()
{
    int aDestX, aDestY;
    if (IsSun())
    {
        aDestX = 15;
        aDestY = 0;
    }
    else if (IsMoney())
    {
        aDestX = 39;
        aDestY = 558;

        if (mApp->GetDialog(static_cast<int>(Dialogs::DIALOG_STORE)))
        {
            aDestX = 662;
            aDestY = 546;
        }
        else if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN || mApp->mCrazyDaveState != CrazyDaveState::CRAZY_DAVE_OFF)
        {
            aDestX = 442;
        }
    }
    else if (IsPresentWithAdvice())
    {
        aDestX = 35;
        aDestY = 487;
    }
    else if(mType == CoinType::COIN_AWARD_PRESENT || mType == CoinType::COIN_PRESENT_PLANT)
    {
        mDisappearCounter++;
        if (mDisappearCounter >= 200)
        {
            StartFade();
        }

        return;
    }
    else if (!IsLevelAward())
    {
        if (mType == CoinType::COIN_USABLE_SEED_PACKET)
        {
            mDisappearCounter++;
        }

        return;
    }
    else
    {
        aDestX = 400 - mWidth / 2;
        aDestY = 200 - mHeight / 2;
        mDisappearCounter++;
    }

    if (IsLevelAward())
    {
        mScale = TodAnimateCurveFloat(0, 400, mDisappearCounter, 1.01f, 2.0f, TodCurves::CURVE_EASE_IN_OUT);
        mPosX = TodAnimateCurveFloat(0, 350, mDisappearCounter, mCollectX, aDestX, TodCurves::CURVE_EASE_OUT);
        mPosY = TodAnimateCurveFloat(0, 350, mDisappearCounter, mCollectY, aDestY, TodCurves::CURVE_EASE_OUT);
        return;
    }

    float aDeltaX = abs(mPosX - aDestX);
    float aDeltaY = abs(mPosY - aDestY);
    if (mPosX > aDestX)
    {
        mPosX -= aDeltaX / 21.0f;
    }
    else if (mPosX < aDestX)
    {
        mPosX += aDeltaX / 21.0f;
    }
    if (mPosY > aDestY)
    {
        mPosY -= aDeltaY / 21.0f;
    }
    else if (mPosY < aDestY)
    {
        mPosY += aDeltaY / 21.0f;
    }

    mCollectionDistance = sqrt(aDeltaY * aDeltaY + aDeltaX * aDeltaX);

    if (IsPresentWithAdvice())
    {
        if (mCollectionDistance < 15.0f)
        {
            if (!mBoard->mHelpDisplayed[AdviceType::ADVICE_UNLOCKED_MODE])
            {
                if (mType == CoinType::COIN_PRESENT_MINIGAMES)
                {
                    // 注：此处的 theMessageStyle 参数，原版中为 MESSAGE_STYLE_HINT_TALL_UNLOCKMESSAGE，内测版中为 MESSAGE_STYLE_HINT_TALL_8SECONDS
                    mBoard->DisplayAdvice("[UNLOCKED_MINIGAMES]", MessageStyle::MESSAGE_STYLE_HINT_TALL_UNLOCKMESSAGE, AdviceType::ADVICE_UNLOCKED_MODE);
                }
                else if (mType == CoinType::COIN_PRESENT_PUZZLE_MODE)
                {
                    mBoard->DisplayAdvice("[UNLOCKED_PUZZLE_MODE]", MessageStyle::MESSAGE_STYLE_HINT_TALL_UNLOCKMESSAGE, AdviceType::ADVICE_UNLOCKED_MODE);
                }
                else { // @Patoke: add case
                    mBoard->DisplayAdvice("[UNLOCKED_SURVIVAL_MODE]", MessageStyle::MESSAGE_STYLE_HINT_TALL_UNLOCKMESSAGE, AdviceType::ADVICE_UNLOCKED_MODE);
                }
            }
            else if (mBoard->mHelpIndex != AdviceType::ADVICE_UNLOCKED_MODE || !mBoard->mAdvice->IsBeingDisplayed())
            {
                Die();
            }
        }
    }
    else
    {
        float aScoringDistance = 8.0f;
        if (IsMoney())
        {
            aScoringDistance = 12.0f;
        }

        if (mCollectionDistance < aScoringDistance)
        {
            ScoreCoin();
        }

        mScale = ClampFloat(mCollectionDistance * 0.05f, 0.5f, 1.0f);
        mScale *= GetSunScale();
    }
}

void Coin::Update()
{
    mCoinAge++;
    if (mApp->mGameScene != GameScenes::SCENE_PLAYING && mApp->mGameScene != GameScenes::SCENE_AWARD && mBoard && !mBoard->mCutScene->ShouldRunUpsellBoard())
    {
        return;
    }

    if (mFadeCount != 0)
    {
        UpdateFade();
    }
    else if (!mIsBeingCollected)
    {
        UpdateFall();
    }
    else
    {
        UpdateCollected();
    }

    if (mAttachmentID != AttachmentID::ATTACHMENTID_NULL)
    {
        float aOffsetX = 0.0f;
        float aOffsetY = 0.0f;
        if (mType == CoinType::COIN_DIAMOND)
        {
            aOffsetX = 18.0f - 18.0f * mScale;
            aOffsetY = 13.0f - 13.0f * mScale;
        }

        AttachmentUpdateAndMove(mAttachmentID, mPosX + aOffsetX, mPosY + aOffsetY);
        AttachmentOverrideColor(mAttachmentID, GetColor());
        AttachmentOverrideScale(mAttachmentID, mScale);

        if ((!mHitGround || mIsBeingCollected) && (mType == CoinType::COIN_SILVER || mType == CoinType::COIN_GOLD))
        {
            AttachmentOverrideColor(mAttachmentID, Color(0, 0, 0, 0));  // 运动中的金币和银币使用贴图，故以此法隐藏附件的动画
        }
    }
}

Color Coin::GetColor()
{
    if ((IsSun() || IsMoney()) && mIsBeingCollected)
    {
        float aAlpha = ClampFloat(mCollectionDistance * 0.035f, 0.35f, 1.0f) * 255.0f;
        return Color(255, 255, 255, aAlpha);
    }

    if (mFadeCount > 0)
    {
        int aAlpha = TodAnimateCurve(15, 0, mFadeCount, 255, 0, TodCurves::CURVE_LINEAR);
        return Color(255, 255, 255, aAlpha);
    }

    return Color::White;
}

SeedType Coin::GetFinalSeedPacketType()
{
    if (mApp->IsFirstTimeAdventureMode() && mBoard && mBoard->mLevel <= 50)
    {
        return mApp->GetAwardSeedForLevel(mBoard->mLevel);
    }

    return SeedType::SEED_NONE;
}

void Coin::Draw(Graphics* g)
{
    g->SetColor(GetColor());
    if (mType == CoinType::COIN_DIAMOND)
    {
        g->SetColorizeImages(true);
        g->DrawImage(IMAGE_AWARDPICKUPGLOW, mPosX - 56.0f, mPosY - 66.0f);
        g->SetColorizeImages(false);
    }
    if (mType == CoinType::COIN_PRESENT_PLANT)
    {
        g->SetColorizeImages(true);
        g->DrawImage(IMAGE_AWARDPICKUPGLOW, mPosX - 50.0f, mPosY - 64.0f);
        g->SetColorizeImages(false);
    }
    if (mType == CoinType::COIN_AWARD_PRESENT && mIsBeingCollected)
    {
        g->SetColorizeImages(true);
        g->DrawImage(IMAGE_AWARDPICKUPGLOW, mPosX - 50.0f, mPosY - 64.0f);
        g->SetColorizeImages(false);
    }
    if (mType == CoinType::COIN_CHOCOLATE || mType == CoinType::COIN_AWARD_CHOCOLATE)
    {
        g->SetColorizeImages(true);
        g->DrawImage(IMAGE_AWARDPICKUPGLOW, mPosX - 56.0f, mPosY - 50.0f);
        g->SetColorizeImages(false);
    }

    if (mAttachmentID != AttachmentID::ATTACHMENTID_NULL)
    {
        Graphics theAttachmentGraphics(*g);
        MakeParentGraphicsFrame(&theAttachmentGraphics);
        AttachmentDraw(mAttachmentID, &theAttachmentGraphics, false);
    }

    if ((mType == CoinType::COIN_SILVER || mType == CoinType::COIN_GOLD) && mHitGround && !mIsBeingCollected)
    {
        return;
    }

    if (mType == CoinType::COIN_DIAMOND)
    {
        return;
    }

    if (IsLevelAward() && !mIsBeingCollected)
    {
        Color aFlashingColor = GetFlashingColor(mCoinAge, 75);
        g->SetColor(aFlashingColor);
    }

    if (mType == CoinType::COIN_SILVER || mType == CoinType::COIN_GOLD)
    {
        g->SetColorizeImages(true);
        TodDrawImageCenterScaledF(g, IMAGE_REANIM_COINGLOW, mPosX - 14.0f, mPosY - 12.0f, mScale, mScale);
        g->SetColorizeImages(false);
    }

    Image* aImage = nullptr;
    int aImageCelCol = 0;
    float aDrawScale = mScale;
    float aOffsetX = 0.0f;
    float aOffsetY = 0.0f;
    if (mType == CoinType::COIN_SILVER)
    {
        aImage = IMAGE_REANIM_COIN_SILVER_DOLLAR;
        aOffsetX = 8.0f;
        aOffsetY = 10.0f;
    }
    else if (mType == CoinType::COIN_GOLD)
    {
        aImage = IMAGE_REANIM_COIN_GOLD_DOLLAR;
        aOffsetX = 8.0f;
        aOffsetY = 10.0f;
    }
    else if (IsSun())
    {
        return;
    }
    else if (mType == CoinType::COIN_FINAL_SEED_PACKET)
    {
        SeedType aSeedType = GetFinalSeedPacketType();
        g->SetScale(mScale, mScale, 0.0f, 0.0f);
        DrawSeedPacket(g, 0.5f * (mWidth - mScale * mWidth) + mPosX, 0.5f * (mHeight - mScale * mHeight) + mPosY, aSeedType, SeedType::SEED_NONE, 0.0f, 255, true, false);
        g->SetScale(1.0f, 1.0f, 0.0f, 0.0f);
        return;
    }
    else if (mType == CoinType::COIN_PRESENT_PLANT || mType == CoinType::COIN_AWARD_PRESENT)
    {
        if (mIsBeingCollected)
        {
            mApp->mZenGarden->DrawPottedPlantIcon(g, mPosX + 10.0f, mPosY - 20.0f, &mPottedPlantSpec);
            return;
        }

        aImage = IMAGE_PRESENT;
        aOffsetY = -20.0f;
    }
    else if (IsPresentWithAdvice())
    {
        aOffsetY = -20.0f;

        if (mIsBeingCollected)
        {
            aOffsetX = -10.0f;
            aOffsetY -= -10.0f;
            aImage = IMAGE_PRESENTOPEN;
        }
        else
        {
            aImage = IMAGE_PRESENT;
        }
    }
    else if (mType == CoinType::COIN_AWARD_MONEY_BAG || mType == CoinType::COIN_AWARD_BAG_DIAMOND)
    {
        aImage = IMAGE_MONEYBAG_HI_RES;
        aOffsetX -= mWidth / 2;
        aOffsetY -= mHeight / 2;
        aDrawScale *= 0.5f;
    }
    else if (mType == CoinType::COIN_CHOCOLATE || mType == CoinType::COIN_AWARD_CHOCOLATE)
    {
        aImage = IMAGE_CHOCOLATE;
    }
    else if (mType == CoinType::COIN_TROPHY)
    {
        aImage = IMAGE_TROPHY_HI_RES;
        aOffsetX -= mWidth / 2;
        aOffsetY -= mHeight / 2;
        aDrawScale *= 0.5f;
    }
    else if (mType == CoinType::COIN_AWARD_SILVER_SUNFLOWER)
    {
        aImage = IMAGE_SUNFLOWER_TROPHY;
        aOffsetX -= 5.0f;
        aDrawScale *= 0.6f;
    }
    else if (mType == CoinType::COIN_AWARD_GOLD_SUNFLOWER)
    {
        aImage = IMAGE_SUNFLOWER_TROPHY;
        aImageCelCol = 1;
        aOffsetX -= 5.0f;
        aDrawScale *= 0.6f;
    }
    else if (mType == CoinType::COIN_SHOVEL)
    {
        aImage = IMAGE_SHOVEL_HI_RES;
        aOffsetX -= 20.0f;
        aOffsetY -= 20.0f;
        aDrawScale *= 0.5f;
    }
    else if (mType == CoinType::COIN_CARKEYS)
    {
        aImage = IMAGE_CARKEYS;
    }
    else if (mType == CoinType::COIN_ALMANAC)
    {
        aImage = IMAGE_ALMANAC;
    }
    else if (mType == CoinType::COIN_TACO)
    {
        aImage = IMAGE_TACO;
    }
    else if (mType == CoinType::COIN_VASE)
    {
        aImage = IMAGE_SCARY_POT;
    }
    else if (mType == CoinType::COIN_WATERING_CAN)
    {
        aImage = IMAGE_WATERINGCAN;
    }
    else if (mType == CoinType::COIN_NOTE)
    {
        aImage = IMAGE_ZOMBIE_NOTE_SMALL;
    }
    else if (mType == CoinType::COIN_USABLE_SEED_PACKET)
    {
        int aGrayness = 255;
        if (mIsBeingCollected)
        {
            aGrayness = 128;
        }
        else
        {
            int aDisappearTime = GetDisappearTime();
            if (mDisappearCounter > aDisappearTime - 300 && mDisappearCounter % 60 < 30)
            {
                aGrayness = 192;
            }
        }

        g->SetColorizeImages(true);
        DrawSeedPacket(g, static_cast<int>(mPosX), static_cast<int>(mPosY), mUsableSeedType, SeedType::SEED_NONE, 0.0f, aGrayness, false, false);
        g->SetColorizeImages(false);

        return;
    }
    else
    {
        TOD_ASSERT(false);
    }

    g->SetColorizeImages(true);
    TodDrawImageCelCenterScaledF(g, aImage, mPosX + aOffsetX, mPosY + aOffsetY, aImageCelCol, aDrawScale, aDrawScale);
    g->SetColorizeImages(false);
}

void Coin::FanOutCoins(CoinType theCoinType, int theNumCoins)
{
    TOD_ASSERT(mBoard);

    for (int i = 0; i < theNumCoins; i++)
    {
        float aAngle = PI / 2 + PI * (i + 1) / (theNumCoins + 1);
        float aPosX = mPosX + 20.0f;
        float aPosY = mPosY;
        Coin* aCoin = mBoard->AddCoin(static_cast<int>(aPosX), static_cast<int>(aPosY), theCoinType, CoinMotion::COIN_MOTION_FROM_PRESENT);
        aCoin->mVelX = 5.0f * sin(aAngle);
        aCoin->mVelY = 5.0f * cos(aAngle);
    }
}

void Coin::TryAutoCollectAfterLevelAward()
{
    bool aCanBeAutoCollected = false;
    if (IsMoney() && mCoinMotion != CoinMotion::COIN_MOTION_FROM_PRESENT)
    {
        aCanBeAutoCollected = true;
    }
    if (IsSun())
    {
        aCanBeAutoCollected = true;
    }
    if (mType == CoinType::COIN_PRESENT_PLANT || mType == CoinType::COIN_CHOCOLATE || IsPresentWithAdvice())
    {
        aCanBeAutoCollected = true;
    }

    if (aCanBeAutoCollected)
    {
        PlayCollectSound();
        Collect();
    }
}

// GOTY @Patoke: 0x434DC0
void Coin::Collect()
{
    if (mDead)
        return;

    mCollectX = mPosX;
    mCollectY = mPosY;
    mIsBeingCollected = true;

    bool aIsEndlessAward = false;
    if ((mApp->IsEndlessIZombie(mApp->mGameMode) || mApp->IsEndlessScaryPotter(mApp->mGameMode)) && IsLevelAward())
    {
        aIsEndlessAward = true;
    }

    if (mType == CoinType::COIN_PRESENT_PLANT || mType == CoinType::COIN_AWARD_PRESENT)
    {
        TOD_ASSERT(mBoard);

        if (mApp->mZenGarden->IsZenGardenFull(false))
        {
            mBoard->DisplayAdvice("[DIALOG_ZEN_GARDEN_FULL]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_NONE);
        }
        else
        {
            mBoard->mPottedPlantsCollected++;
            mBoard->DisplayAdvice("[ADVICE_FOUND_PLANT]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_NONE);
            mApp->AddTodParticle(mPosX + 30.0f, mPosY + 30.0f, mRenderOrder + 1, ParticleEffect::PARTICLE_PRESENT_PICKUP);
            mApp->mZenGarden->AddPottedPlant(&mPottedPlantSpec);
        }

        mDisappearCounter = 0;
        mFadeCount = 0;
        if (aIsEndlessAward)
        {
            AttachmentDetachCrossFadeParticleType(mAttachmentID, ParticleEffect::PARTICLE_AWARD_PICKUP_ARROW, nullptr);
            mBoard->FadeOutLevel();
        }

        return;
    }

    if (mType == CoinType::COIN_PRESENT_MINIGAMES)
    {
        TOD_ASSERT(mBoard);

        mApp->AddTodParticle(mPosX + 30.0f, mPosY + 30.0f, mRenderOrder + 1, ParticleEffect::PARTICLE_PRESENT_PICKUP);

        mDisappearCounter = 0;
        mFadeCount = 0;

        AttachmentDetachCrossFadeParticleType(mAttachmentID, ParticleEffect::PARTICLE_AWARD_PICKUP_ARROW, nullptr);
        mApp->mPlayerInfo->mHasUnlockedMinigames = 1;

        return;
    }
    if (mType == CoinType::COIN_PRESENT_PUZZLE_MODE)
    {
        TOD_ASSERT(mBoard);

        mApp->AddTodParticle(mPosX + 30.0f, mPosY + 30.0f, mRenderOrder + 1, ParticleEffect::PARTICLE_PRESENT_PICKUP);

        mDisappearCounter = 0;
        mFadeCount = 0;

        AttachmentDetachCrossFadeParticleType(mAttachmentID, ParticleEffect::PARTICLE_AWARD_PICKUP_ARROW, nullptr);
        mApp->mPlayerInfo->mHasUnlockedPuzzleMode = 1;

        return;
    }
    if (mType == CoinType::COIN_PRESENT_SURVIVAL_MODE)
    {
        TOD_ASSERT(mBoard);

        mApp->AddTodParticle(mPosX + 30.0f, mPosY + 30.0f, mRenderOrder + 1, ParticleEffect::PARTICLE_PRESENT_PICKUP);

        mDisappearCounter = 0;
        mFadeCount = 0;

        AttachmentDetachCrossFadeParticleType(mAttachmentID, ParticleEffect::PARTICLE_AWARD_PICKUP_ARROW, nullptr);
        mApp->mPlayerInfo->mHasUnlockedSurvivalMode = 1;

        return;
    }

    if (mType == CoinType::COIN_CHOCOLATE || mType == CoinType::COIN_AWARD_CHOCOLATE)
    {
        TOD_ASSERT(mBoard);

        mBoard->mChocolateCollected++;
        mApp->AddTodParticle(mPosX + 30.0f, mPosY + 30.0f, mRenderOrder + 1, ParticleEffect::PARTICLE_PRESENT_PICKUP);

        if (mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_CHOCOLATE] < PURCHASE_COUNT_OFFSET)
        {
            mBoard->DisplayAdvice("[ADVICE_FOUND_CHOCOLATE]", MessageStyle::MESSAGE_STYLE_HINT_TALL_FAST, AdviceType::ADVICE_NONE);
            mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_CHOCOLATE] = PURCHASE_COUNT_OFFSET + 1;
        }
        else
        {
            mApp->mPlayerInfo->mPurchases[StoreItem::STORE_ITEM_CHOCOLATE]++;
        }

        mDisappearCounter = 0;
        StartFade();

        if (aIsEndlessAward)
        {
            AttachmentDetachCrossFadeParticleType(mAttachmentID, ParticleEffect::PARTICLE_AWARD_PICKUP_ARROW, nullptr);
            mBoard->FadeOutLevel();
        }

        return;
    }

    if (IsLevelAward())
    {
        if (aIsEndlessAward)
        {
            if (mType == CoinType::COIN_AWARD_BAG_DIAMOND)
            {
                mApp->PlaySample(SOUND_DIAMOND);
                FanOutCoins(CoinType::COIN_DIAMOND, 1);
                StartFade();
            }
            else if (mType == CoinType::COIN_AWARD_MONEY_BAG)
            {
                mApp->PlayFoley(FoleyType::FOLEY_COIN);
                FanOutCoins(CoinType::COIN_GOLD, 5);
                StartFade();
            }
        }
        else if (mApp->IsScaryPotterLevel())
        {
            if (mType == CoinType::COIN_TROPHY)
            {
                mApp->PlayFoley(FoleyType::FOLEY_COIN);
                FanOutCoins(CoinType::COIN_GOLD, 5);
            }
            else if (mType == CoinType::COIN_AWARD_MONEY_BAG)
            {
                mApp->PlayFoley(FoleyType::FOLEY_COIN);
                FanOutCoins(CoinType::COIN_GOLD, 2);
            }
        }
        else if (mApp->IsAdventureMode() && mBoard->mLevel == 50)
        {
            FanOutCoins(CoinType::COIN_DIAMOND, 3);
        }
        else if (mType == CoinType::COIN_AWARD_GOLD_SUNFLOWER)
        {
            FanOutCoins(CoinType::COIN_DIAMOND, 5);
        }
        else if (mApp->IsFirstTimeAdventureMode() && mBoard->mLevel == 4)
        {
            mApp->PlaySample(SOUND_SHOVEL);
        }
        else if (mApp->IsFirstTimeAdventureMode() && (mBoard->mLevel == 24 || mBoard->mLevel == 34 || mBoard->mLevel == 44))
        {
            mApp->PlaySample(SOUND_TAP2);
        }
        else if (mType == CoinType::COIN_TROPHY)
        {
            mApp->PlaySample(SOUND_DIAMOND);
            FanOutCoins(CoinType::COIN_DIAMOND, 1);
        }
        else if (mType == CoinType::COIN_AWARD_MONEY_BAG)
        {
            mApp->PlayFoley(FoleyType::FOLEY_COIN);
            FanOutCoins(CoinType::COIN_GOLD, 5);
        }
        else
        {
            mApp->PlaySample(SOUND_SEEDLIFT);
            mApp->PlaySample(SOUND_TAP2);
        }

        mApp->AddTodParticle(mPosX + 30.0f, mPosY + 30.0f, mRenderOrder + 1, ParticleEffect::PARTICLE_STARBURST);

        TOD_ASSERT(mBoard);

        mBoard->FadeOutLevel();
        AttachmentDetachCrossFadeParticleType(mAttachmentID, ParticleEffect::PARTICLE_SEED_PACKET, nullptr);
        AttachmentDetachCrossFadeParticleType(mAttachmentID, ParticleEffect::PARTICLE_AWARD_PICKUP_ARROW, nullptr);
        AttachmentDetachCrossFadeParticleType(mAttachmentID, ParticleEffect::PARTICLE_COIN_PICKUP_ARROW, nullptr);

        if (mType == CoinType::COIN_NOTE)
        {
            mApp->AddTodParticle(mPosX + 30.0f, mPosY + 30.0f, mRenderOrder + 1, ParticleEffect::PARTICLE_PRESENT_PICKUP);
            StartFade();
        }
        else if (!aIsEndlessAward && mApp->Is3DAccelerated())
        {
            float aParticleOffsetX = mWidth / 2;
            float aParticleOffsetY = mHeight / 2;
            TodParticleSystem* aParticle = mApp->AddTodParticle(mPosX + aParticleOffsetX, mPosY + aParticleOffsetY, mRenderOrder - 1, PARTICLE_SEED_PACKET_PICKUP);
            AttachParticle(mAttachmentID, aParticle, aParticleOffsetX, aParticleOffsetY);
        }

        mDisappearCounter = 0;
        return;
    }

    if (mType == CoinType::COIN_USABLE_SEED_PACKET)
    {
        TOD_ASSERT(mBoard);

        mBoard->mCursorObject->mType = mUsableSeedType;
        mBoard->mCursorObject->mCursorType = CursorType::CURSOR_TYPE_PLANT_FROM_USABLE_COIN;
        mBoard->mCursorObject->mCoinID = static_cast<CoinID>(mBoard->mCoins.DataArrayGetID(this));

        mGroundY = static_cast<int>(mPosY);
        mFadeCount = 0;
        return;
    }

    if (IsMoney() && mBoard)
    {
        mBoard->ShowCoinBank();
    }

    mFadeCount = 0;

    if (IsSun() && mBoard && !mBoard->HasConveyorBeltSeedBank())
    {
        for (int i = 0; i < mBoard->mSeedBank->mNumPackets; i++)
        {
            SeedPacket* aSeedPacket = &mBoard->mSeedBank->mSeedPackets[i];
            int aCost = mBoard->GetCurrentPlantCost(aSeedPacket->mPacketType, aSeedPacket->mImitaterType);
            int aSunProfit = mBoard->mSunMoney + mBoard->CountSunBeingCollected() - aCost;
            if (aSunProfit >= 0 && aSunProfit < GetSunValue())
            {
                aSeedPacket->FlashIfReady();
            }
        }

        if (mBoard->StageHasFog())
        {
            mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_ABOVE_UI, 0, 2);
        }
    }

    AttachmentDetachCrossFadeParticleType(mAttachmentID, ParticleEffect::PARTICLE_COIN_PICKUP_ARROW, nullptr);
    if (mApp->IsFirstTimeAdventureMode() && mBoard && mBoard->mLevel == 11 && (mType == CoinType::COIN_GOLD || mType == CoinType::COIN_SILVER))
    {
        mBoard->DisplayAdvice("[ADVICE_CLICKED_ON_COIN]", MessageStyle::MESSAGE_STYLE_HINT_FAST, AdviceType::ADVICE_CLICKED_ON_COIN);
    }
}

float Coin::GetSunScale()
{
    return mType == CoinType::COIN_SMALLSUN ? 0.5f : mType == CoinType::COIN_LARGESUN ? 2.0f : 1.0f;
}

int Coin::GetSunValue()
{
    return mType == CoinType::COIN_SUN ? 25 : mType == CoinType::COIN_SMALLSUN ? 15 : mType == CoinType::COIN_LARGESUN ? 50 : 0;
}

int Coin::GetCoinValue(CoinType theCoinType)
{
    return theCoinType == CoinType::COIN_SILVER ? 1 : theCoinType == CoinType::COIN_GOLD ? 5 : theCoinType == CoinType::COIN_DIAMOND ? 100 : 0;
}

void Coin::PlayLaunchSound()
{
    if (mType == CoinType::COIN_DIAMOND || 
        mType == CoinType::COIN_CHOCOLATE || 
        mType == CoinType::COIN_AWARD_CHOCOLATE || 
        mType == CoinType::COIN_PRESENT_PLANT ||
        mType == CoinType::COIN_AWARD_PRESENT || 
        IsPresentWithAdvice())
    {
        mApp->PlayFoley(FoleyType::FOLEY_CHIME);
    }
}

void Coin::PlayGroundSound()
{
    if (mType == CoinType::COIN_GOLD)
    {
        mApp->PlayFoley(FoleyType::FOLEY_MONEYFALLS);
        return;
    }
}

// GOTY @Patoke: 0x435850
void Coin::PlayCollectSound()
{
    if (mType == CoinType::COIN_USABLE_SEED_PACKET)
    {
        mApp->PlaySample(SOUND_SEEDLIFT);
        return;
    }

    if (mType == CoinType::COIN_SILVER || mType == CoinType::COIN_GOLD)
    {
        mApp->PlayFoley(FoleyType::FOLEY_COIN);
        return;
    }

    if (mType == CoinType::COIN_DIAMOND)
    {
        mApp->PlaySample(SOUND_DIAMOND);
        return;
    }

    if (IsSun())
    {
        mApp->PlayFoley(FoleyType::FOLEY_SUN);
        return;
    }

    if (mType == CoinType::COIN_CHOCOLATE || 
        mType == CoinType::COIN_PRESENT_PLANT || 
        IsPresentWithAdvice() || 
        mType == CoinType::COIN_AWARD_PRESENT || 
        mType == CoinType::COIN_AWARD_CHOCOLATE)
    {
        mApp->PlayFoley(FoleyType::FOLEY_PRIZE);
        return;
    }

    if (IsSun())
    {
        mApp->PlayFoley(FoleyType::FOLEY_SUN);
    }
}

void Coin::DroppedUsableSeed()
{
    mIsBeingCollected = false;

    if (mTimesDropped == 0)
    {
        mDisappearCounter = std::min(mDisappearCounter, 1200);
    }

    mTimesDropped++;
}

void Coin::MouseDown(int x, int y, int theClickCount)
{
    (void)x;(void)y;
    if (mBoard == nullptr || mBoard->mPaused || mApp->mGameScene != GameScenes::SCENE_PLAYING || mDead)
    {
        return;
    }

    if (theClickCount >= 0 && !mIsBeingCollected)
    {
        PlayCollectSound();
        Collect();

        if (mApp->IsFirstTimeAdventureMode() && mBoard->mLevel == 1)
        {
            mBoard->DisplayAdvice("[ADVICE_CLICKED_ON_SUN]", MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY, AdviceType::ADVICE_CLICKED_ON_SUN);
        }
    }
}

// GOTY @Patoke: 0x435B20
void Coin::Die()
{
    TOD_ASSERT(!mBoard || mBoard->mCursorObject->mCoinID != static_cast<CoinID>(mBoard->mCoins.DataArrayGetID(this)));

    mDead = true;
    AttachmentDie(mAttachmentID);
}

bool Coin::MouseHitTest(int theX, int theY, HitResult* theHitResult)
{
    int aOffsetY = 0;
    if (mType == CoinType::COIN_PRESENT_PLANT || mType == CoinType::COIN_AWARD_PRESENT || IsPresentWithAdvice())
    {
        aOffsetY = -20;
    }

    int aExtraClickSize = 0;
    int aExtraClickHeight = 0;
    if (mApp->IsWhackAZombieLevel())
    {
        aExtraClickHeight = 30;
        aExtraClickSize = 15;
    }
    if (mType == CoinType::COIN_SUN)
    {
        aExtraClickSize = 15;
    }

    bool aCanHitCoin = true;
    if (mDead || mIsBeingCollected)
    {
        aCanHitCoin = false;
    }
    if (mType == CoinType::COIN_USABLE_SEED_PACKET && mBoard)
    {
        if (mBoard->mCursorObject->mCursorType != CursorType::CURSOR_TYPE_NORMAL && !mApp->IsWhackAZombieLevel())
        {
            aCanHitCoin = false;
        }
    }

    if (aCanHitCoin &&
        theX >= mPosX - aExtraClickSize &&
        theX < mPosX + mWidth + aExtraClickSize &&
        theY >= mPosY + aOffsetY - aExtraClickSize &&
        theY < mPosY + mHeight + aOffsetY + aExtraClickSize + aExtraClickHeight)
    {
        theHitResult->mObject = this;
        theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_COIN;
        return true;
    }

    theHitResult->mObject = nullptr;
    theHitResult->mObjectType = GameObjectType::OBJECT_TYPE_NONE;
    return false;
}

bool Coin::IsLevelAward()
{
    return 
        mType == CoinType::COIN_FINAL_SEED_PACKET || 
        mType == CoinType::COIN_TROPHY || 
        mType == CoinType::COIN_AWARD_SILVER_SUNFLOWER || 
        mType == CoinType::COIN_AWARD_GOLD_SUNFLOWER || 
        mType == CoinType::COIN_SHOVEL || 
        mType == CoinType::COIN_CARKEYS || 
        mType == CoinType::COIN_ALMANAC || 
        mType == CoinType::COIN_VASE || 
        mType == CoinType::COIN_WATERING_CAN || 
        mType == CoinType::COIN_TACO || 
        mType == CoinType::COIN_NOTE || 
        mType == CoinType::COIN_AWARD_MONEY_BAG || 
        mType == CoinType::COIN_AWARD_BAG_DIAMOND || 
        mType == CoinType::COIN_AWARD_PRESENT || 
        mType == CoinType::COIN_AWARD_CHOCOLATE;
}

bool Coin::CoinGetsBouncyArrow()
{
    if (IsLevelAward())
        return true;

    if (mType == CoinType::COIN_SILVER || mType == CoinType::COIN_GOLD)
    {
        if (mApp->IsFirstTimeAdventureMode() && mBoard && mBoard->mLevel == 11 && !mBoard->mDroppedFirstCoin)
        {
            return true;
        }
    }

    return IsPresentWithAdvice();
}

int Coin::GetDisappearTime()
{
    int aTime = 750;

    if (mType == CoinType::COIN_DIAMOND || mType == CoinType::COIN_PRESENT_PLANT || mType == CoinType::COIN_CHOCOLATE || mHasBouncyArrow)
    {
        aTime = 1500;
    }

    if ((mApp->IsScaryPotterLevel() || mApp->IsSlotMachineLevel()) && mType == CoinType::COIN_USABLE_SEED_PACKET)
    {
        aTime = 1500;
    }

    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
    {
        aTime = 6000;
    }

    return aTime;
}
