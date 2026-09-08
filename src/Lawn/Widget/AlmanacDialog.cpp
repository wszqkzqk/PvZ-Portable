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

#include <format>
#include "../Board.h"
#include "../Plant.h"
#include "../Zombie.h"
#include "GameButton.h"
#include "../SeedPacket.h"
#include "../../LawnApp.h"
#include "AlmanacDialog.h"
#include "../../Resources.h"
#include "../System/Music.h"
#include "../../GameConstants.h"
#include "../System/PlayerInfo.h"
#include "../System/PoolEffect.h"
#include "../System/ReanimationLawn.h"
#include "../../PvzpLib/PvzpStringFile.h"
#include "widget/WidgetManager.h"

bool gZombieDefeated[NUM_ZOMBIE_TYPES] = { false };

AlmanacDialog::AlmanacDialog(LawnApp* theApp) : LawnDialog(theApp, DIALOG_ALMANAC, true, theApp->GetString("ALMANAC_HEADER", "Almanac"), "", "", BUTTONS_NONE)
{
	mApp = (LawnApp*)gSexyAppBase;
	mOpenPage = ALMANAC_PAGE_INDEX;
	mSelectedSeed = SEED_PEASHOOTER;
	mSelectedZombie = ZOMBIE_NORMAL;
	mDrawStandardBack = false;
	mLoadedResourceNames.push_back("DelayLoad_Almanac");
	LawnDialog::Resize(0, 0, BOARD_WIDTH, BOARD_HEIGHT);

	for (std::string& resource : mLoadedResourceNames)
		PvzpLoadResources(resource.c_str());

	mCloseButton = std::make_unique<GameButton>(AlmanacDialog::ALMANAC_BUTTON_CLOSE);
	mCloseButton->SetLabel("[CLOSE_BUTTON]");
	mCloseButton->mButtonImage = Sexy::IMAGE_ALMANAC_CLOSEBUTTON;
	mCloseButton->mOverImage = Sexy::IMAGE_ALMANAC_CLOSEBUTTONHIGHLIGHT;
	mCloseButton->mDownImage = nullptr;
	mCloseButton->SetFont(Sexy::FONT_BRIANNETOD12);
	Color aColor = Color(42, 42, 90);
	mCloseButton->SetLabelColor(aColor);
	mCloseButton->SetLabelHiliteColor(aColor);
	mCloseButton->Resize(676, 567, 89, 26);
	mCloseButton->mParentWidget = this;
	mCloseButton->mTextOffsetX = -8;
	mCloseButton->mTextOffsetY = 1;

	mIndexButton = std::make_unique<GameButton>(AlmanacDialog::ALMANAC_BUTTON_INDEX);
	mIndexButton->SetLabel("[ALMANAC_INDEX]");
	mIndexButton->mButtonImage = Sexy::IMAGE_ALMANAC_INDEXBUTTON;
	mIndexButton->mOverImage = Sexy::IMAGE_ALMANAC_INDEXBUTTONHIGHLIGHT;
	mIndexButton->mDownImage = nullptr;
	mIndexButton->SetFont(Sexy::FONT_BRIANNETOD12);
	mIndexButton->SetLabelColor(aColor);
	mIndexButton->SetLabelHiliteColor(aColor);
	mIndexButton->Resize(32, 567, 164, 26);
	mIndexButton->mParentWidget = this;
	mIndexButton->mTextOffsetX = 8;
	mIndexButton->mTextOffsetY = 1;

	mPlantButton = std::make_unique<GameButton>(AlmanacDialog::ALMANAC_BUTTON_PLANT);
	mPlantButton->SetLabel("[VIEW_PLANTS]");
	mPlantButton->mButtonImage = Sexy::IMAGE_SEEDCHOOSER_BUTTON;
	mPlantButton->mOverImage = nullptr;
	mPlantButton->mDownImage = nullptr;
	mPlantButton->mDisabledImage = Sexy::IMAGE_SEEDCHOOSER_BUTTON_DISABLED;
	mPlantButton->mOverOverlayImage = Sexy::IMAGE_SEEDCHOOSER_BUTTON_GLOW;
	mPlantButton->SetFont(Sexy::FONT_DWARVENTODCRAFT18YELLOW);
	mPlantButton->SetLabelColor(Color::White);
	mPlantButton->SetLabelHiliteColor(Color::White);
	mPlantButton->Resize(130, 345, 156, 42);
	mPlantButton->mTextOffsetY = -1;
	mPlantButton->mParentWidget = this;

	mZombieButton = std::make_unique<GameButton>(AlmanacDialog::ALMANAC_BUTTON_ZOMBIE);
	mZombieButton->SetLabel("[VIEW_ZOMBIES]");
	mZombieButton->Resize(487, 345, 210, 48);
	mZombieButton->mDrawStoneButton = true;
	mZombieButton->mParentWidget = this;

	SetPage(ALMANAC_PAGE_INDEX);
	if (!mApp->mBoard || !mApp->mBoard->mPaused)
		mApp->mMusic->MakeSureMusicIsPlaying(MUSIC_TUNE_CHOOSE_YOUR_SEEDS);
}

AlmanacDialog::~AlmanacDialog()
{
	ClearPlantsAndZombies();
}

void AlmanacDialog::ClearPlantsAndZombies()
{
	if (mPlant)
	{
		mPlant->Die();
		mPlant.reset();
	}
	if (mZombie)
	{
		mZombie->DieNoLoot();
		mZombie.reset();
	}
	for (std::unique_ptr<Zombie>& aZombie : mZombiePerfTest)
	{
		if (aZombie)
		{
			aZombie->DieNoLoot();
			aZombie.reset();
		}
	}
}

void AlmanacDialog::RemovedFromManager(WidgetManager* theWidgetManager)
{
	LawnDialog::RemovedFromManager(theWidgetManager);
	ClearPlantsAndZombies();
}

void AlmanacDialog::SetupPlant()
{
	ClearPlantsAndZombies();

	float aPosX = ALMANAC_PLANT_POSITION_X;
	float aPosY = ALMANAC_PLANT_POSITION_Y;
	if (mSelectedSeed == SEED_TALLNUT)				aPosY += 18;
	else if (mSelectedSeed == SEED_COBCANNON)		aPosX -= 40;
	else if (mSelectedSeed == SEED_FLOWERPOT)		aPosY -= 20;
	else if (mSelectedSeed == SEED_INSTANT_COFFEE)	aPosY += 20;
	else if (mSelectedSeed == SEED_GRAVEBUSTER)		aPosY += 55;

	mPlant = std::make_unique<Plant>();
	mPlant->mBoard = nullptr;
	mPlant->mIsOnBoard = false;
	mPlant->PlantInitialize(0, 0, mSelectedSeed, SEED_NONE);
	mPlant->mX = aPosX;
	mPlant->mY = aPosY;
}

void AlmanacDialog::SetupZombie()
{
	ClearPlantsAndZombies();

	mZombie = std::make_unique<Zombie>();
	mZombie->mBoard = nullptr;
	mZombie->ZombieInitialize(0, mSelectedZombie, false, nullptr, Zombie::ZOMBIE_WAVE_UI);
	mZombie->mPosX = ALMANAC_ZOMBIE_POSITION_X;
	mZombie->mPosY = ALMANAC_ZOMBIE_POSITION_Y;
}

void AlmanacDialog::SetPage(AlmanacPage thePage)
{
	mOpenPage = thePage;
	ClearPlantsAndZombies();

	if (mOpenPage == AlmanacPage::ALMANAC_PAGE_INDEX)
	{
		mPlant = std::make_unique<Plant>();
		mPlant->mBoard = nullptr;
		mPlant->mIsOnBoard = false;
		mPlant->PlantInitialize(0, 0, SeedType::SEED_SUNFLOWER, SeedType::SEED_NONE);
		mPlant->mX = ALMANAC_INDEXPLANT_POSITION_X;
		mPlant->mY = ALMANAC_INDEXPLANT_POSITION_Y;

		mZombie = std::make_unique<Zombie>();
		mZombie->mBoard = nullptr;
		mZombie->ZombieInitialize(0, ZombieType::ZOMBIE_NORMAL, false, nullptr, Zombie::ZOMBIE_WAVE_UI);
		mZombie->mPosX = ALMANAC_INDEXZOMBIE_POSITION_X;
		mZombie->mPosY = ALMANAC_INDEXZOMBIE_POSITION_Y;

		mIndexButton->mBtnNoDraw = true;
		mPlantButton->mBtnNoDraw = false;
		mZombieButton->mBtnNoDraw = false;
	}
	else
	{
		if (mOpenPage == AlmanacPage::ALMANAC_PAGE_PLANTS)
			SetupPlant();
		else if (mOpenPage == AlmanacPage::ALMANAC_PAGE_ZOMBIES)
			SetupZombie();
		else return;

		mIndexButton->mBtnNoDraw = false;
		mPlantButton->mBtnNoDraw = true;
		mZombieButton->mBtnNoDraw = true;
	}
}

void AlmanacDialog::ShowPlant(SeedType theSeedType)
{
	mSelectedSeed = theSeedType;
	SetPage(ALMANAC_PAGE_PLANTS);
}

void AlmanacDialog::ShowZombie(ZombieType theZombieType)
{
	mSelectedZombie = theZombieType;
	SetPage(ALMANAC_PAGE_ZOMBIES);
}

void AlmanacDialog::Update()
{
	mCloseButton->Update();
	mIndexButton->Update();
	mPlantButton->Update();
	mZombieButton->Update();
	if (mPlant) mPlant->Update();
	if (mZombie) mZombie->Update();
	for (const std::unique_ptr<Zombie>& aZombie : mZombiePerfTest)
	{
		if (aZombie)
		{
			aZombie->Update();
		}
	}

	int aMouseX = mApp->mWidgetManager->mLastMouseX;
	int aMouseY = mApp->mWidgetManager->mLastMouseY;
	if (SeedHitTest(aMouseX, aMouseY) != SeedType::SEED_NONE || ZombieHitTest(aMouseX, aMouseY) != ZombieType::ZOMBIE_INVALID ||
		mCloseButton->IsMouseOver() || mIndexButton->IsMouseOver() || mPlantButton->IsMouseOver() || mZombieButton->IsMouseOver())
	{
		mApp->SetCursor(CURSOR_HAND);
	}
	else
	{
		mApp->SetCursor(CURSOR_POINTER);
	}

	mApp->mPoolEffect->PoolEffectUpdate();
	MarkDirty();
}

ZombieType AlmanacDialog::GetZombieType(int theIndex)
{
	return theIndex < NUM_ZOMBIE_TYPES ? (ZombieType)theIndex : ZOMBIE_INVALID;
}

void AlmanacDialog::DrawIndex(Graphics* g)
{
	g->DrawImage(Sexy::IMAGE_ALMANAC_INDEXBACK, 0, 0);
	PvzpDrawString(g, "[SUBURBAN_ALMANAC_INDEX]", BOARD_WIDTH / 2, 60, Sexy::FONT_HOUSEOFTERROR28, Color(220, 220, 220), DrawStringJustification::DS_ALIGN_CENTER);

	if (mPlant)
	{
		Graphics aPlantGraphics = Graphics(*g);
		mPlant->BeginDraw(&aPlantGraphics);
		mPlant->Draw(&aPlantGraphics);
	}
	if (mZombie)
	{
		Graphics aZombieGraphics = Graphics(*g);
		mZombie->BeginDraw(&aZombieGraphics);
		mZombie->Draw(&aZombieGraphics);
	}
}

void AlmanacDialog::DrawPlants(Graphics* g)
{
	g->DrawImage(Sexy::IMAGE_ALMANAC_PLANTBACK, 0, 0);
	PvzpDrawString(g, "[SUBURBAN_ALMANAC_PLANTS]", BOARD_WIDTH / 2, 48, Sexy::FONT_HOUSEOFTERROR20, Color(213, 159, 43), DrawStringJustification::DS_ALIGN_CENTER);

	SeedType aSeedMouseOn = SeedHitTest(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY);
	for (SeedType aSeedType = SeedType::SEED_PEASHOOTER; aSeedType < NUM_ALMANAC_SEEDS; aSeedType = (SeedType)(aSeedType + 1))
	{
		int aPosX, aPosY;
		GetSeedPosition(aSeedType, aPosX, aPosY);
		if (mApp->HasSeedType(aSeedType))
		{
			if (aSeedType == SeedType::SEED_IMITATER)
			{
				if (aSeedType == aSeedMouseOn)
					g->DrawImage(Sexy::IMAGE_ALMANAC_IMITATER, aPosX, aPosY);
				g->DrawImage(Sexy::IMAGE_ALMANAC_IMITATER, aPosX, aPosY);
			}
			else
			{
				DrawSeedPacket(g, aPosX, aPosY, aSeedType, SeedType::SEED_NONE, 0, 255, true, false);
				if (aSeedType == aSeedMouseOn)
					g->DrawImage(Sexy::IMAGE_SEEDPACKETFLASH, aPosX, aPosY);
			}
		}
	}

	if (mSelectedSeed == SeedType::SEED_LILYPAD || mSelectedSeed == SeedType::SEED_TANGLEKELP ||
		mSelectedSeed == SeedType::SEED_CATTAIL || mSelectedSeed == SeedType::SEED_SEASHROOM)
	{
		bool aNight = mSelectedSeed == SeedType::SEED_SEASHROOM;
		g->DrawImage(aNight ? Sexy::IMAGE_ALMANAC_GROUNDNIGHTPOOL : Sexy::IMAGE_ALMANAC_GROUNDPOOL, 521, 107);

		if (mApp->Is3DAccelerated())
		{
			g->SetClipRect(475, 0, 397, 500);
			g->mTransY -= 145;
			mApp->mPoolEffect->PoolEffectDraw(g, aNight);
			g->mTransY += 145;
			g->ClearClipRect();
		}
	}
	else
	{
		g->DrawImage(
			Plant::IsNocturnal(mSelectedSeed) || mSelectedSeed == SeedType::SEED_GRAVEBUSTER || mSelectedSeed == SeedType::SEED_PLANTERN ? Sexy::IMAGE_ALMANAC_GROUNDNIGHT :
			mSelectedSeed == SeedType::SEED_FLOWERPOT ? Sexy::IMAGE_ALMANAC_GROUNDROOF : Sexy::IMAGE_ALMANAC_GROUNDDAY,
			521, 107
		);
	}

	if (mPlant)
	{
		Graphics aPlantGraphics = Graphics(*g);
		mPlant->BeginDraw(&aPlantGraphics);
		mPlant->Draw(&aPlantGraphics);
	}

	g->DrawImage(Sexy::IMAGE_ALMANAC_PLANTCARD, 459, 86);
	const PlantDefinition& aPlantDef = GetPlantDefinition(mSelectedSeed);
	std::string aName = Plant::GetNameString(mSelectedSeed, SEED_NONE);
	std::string aDescriptionName = std::format("[{}_DESCRIPTION]", aPlantDef.mPlantName);
	PvzpDrawString(g, aName, 617, 288, Sexy::FONT_DWARVENTODCRAFT18YELLOW, Color::White, DS_ALIGN_CENTER);
	PvzpDrawStringWrapped(g, aDescriptionName, Rect(485, 309, 258, 230), Sexy::FONT_BRIANNETOD12, Color(40, 50, 90), DS_ALIGN_LEFT);

	if (mSelectedSeed != SeedType::SEED_IMITATER)
	{
		std::string aCostStr = PvzpReplaceString(std::format("{{KEYWORD}}{{COST}}:{{STAT}} {}", aPlantDef.mSeedCost), "{COST}", "[COST]");
		PvzpDrawStringWrapped(g, aCostStr, Rect(485, 520, 134, 50), Sexy::FONT_BRIANNETOD12, Color::White, DS_ALIGN_LEFT);

		std::string aRechargeStr = PvzpReplaceString(
			"{KEYWORD}{WAIT_TIME}: {STAT}{WAIT_TIME_LENGTH}",
			"{WAIT_TIME_LENGTH}",
			aPlantDef.mRefreshTime == 750 ? "[WAIT_TIME_SHORT]" : aPlantDef.mRefreshTime == 3000 ? "[WAIT_TIME_LONG]" : "[WAIT_TIME_VERY_LONG]"
		);
		aRechargeStr = PvzpReplaceString(aRechargeStr, "{WAIT_TIME}", "[WAIT_TIME]");
		PvzpDrawStringWrapped(g, aRechargeStr, Rect(600, 520, 139, 50), Sexy::FONT_BRIANNETOD12, Color(40, 50, 90), DS_ALIGN_RIGHT);
	}
}

void AlmanacDialog::DrawZombies(Graphics* g)
{
	g->DrawImage(Sexy::IMAGE_ALMANAC_ZOMBIEBACK, 0, 0);
	PvzpDrawString(g, "[SUBURBAN_ALMANAC_ZOMBIES]", BOARD_WIDTH / 2, 54, Sexy::FONT_DWARVENTODCRAFT24, Color(0, 196, 0), DS_ALIGN_CENTER);

	ZombieType aZombieMouseOn = ZombieHitTest(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY);
	for (int i = 0; i < NUM_ALMANAC_ZOMBIES; i++)
	{
		ZombieType aZombieType = GetZombieType(i);
		int aPosX, aPosY;
		GetZombiePosition(aZombieType, aPosX, aPosY);
		if (aZombieType != ZombieType::ZOMBIE_INVALID)
		{
			if (!ZombieIsShown(aZombieType))
				g->DrawImage(Sexy::IMAGE_ALMANAC_ZOMBIEBLANK, aPosX, aPosY);
			else
			{
				g->DrawImage(Sexy::IMAGE_ALMANAC_ZOMBIEWINDOW, aPosX, aPosY);
				if (aZombieType == aZombieMouseOn)
				{
					g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
					g->SetColor(Color(255, 255, 255, 48));
					g->SetColorizeImages(true);
					g->DrawImage(Sexy::IMAGE_ALMANAC_ZOMBIEWINDOW, aPosX, aPosY);
					g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
					g->SetColorizeImages(false);
				}

				ZombieType aZombieTypeToDraw = aZombieType;
				Graphics aZombieGraphics = Graphics(*g);
				aZombieGraphics.SetClipRect(aPosX + 2, aPosY + 2, 72, 72);
				aZombieGraphics.Translate(aPosX + 1, aPosY - 6);
				aZombieGraphics.mScaleX = 0.5f;
				aZombieGraphics.mScaleY = 0.5f;
				switch (aZombieType)
				{
				case ZombieType::ZOMBIE_POLEVAULTER:
					aZombieGraphics.TranslateF(2, -3);
					aZombieTypeToDraw = ZombieType::ZOMBIE_CACHED_POLEVAULTER_WITH_POLE;		break;
				case ZombieType::ZOMBIE_FLAG:			aZombieGraphics.TranslateF(2, 10);	break;
				case ZombieType::ZOMBIE_TRAFFIC_CONE:	aZombieGraphics.TranslateF(0, 12);	break;
				case ZombieType::ZOMBIE_PAIL:			aZombieGraphics.TranslateF(0, 9);		break;
				case ZombieType::ZOMBIE_FOOTBALL:		aZombieGraphics.TranslateF(-15, -1);	break;
				case ZombieType::ZOMBIE_ZAMBONI:		aZombieGraphics.TranslateF(0, 3);		break;
				case ZombieType::ZOMBIE_DOLPHIN_RIDER:	aZombieGraphics.TranslateF(-2, -10);	break;
				case ZombieType::ZOMBIE_POGO:			aZombieGraphics.TranslateF(0, -3);	break;
				case ZombieType::ZOMBIE_GARGANTUAR:		aZombieGraphics.TranslateF(15, 17);	break;
				case ZombieType::ZOMBIE_IMP:			aZombieGraphics.TranslateF(-8, -7);	break;
				case ZombieType::ZOMBIE_BUNGEE:			aZombieGraphics.TranslateF(-4, 3);	break;
				case ZombieType::ZOMBIE_DANCER:			aZombieGraphics.TranslateF(0, 15);	break;
				case ZombieType::ZOMBIE_BACKUP_DANCER:	aZombieGraphics.TranslateF(-4, 20);	break;
				case ZombieType::ZOMBIE_SNORKEL:		aZombieGraphics.TranslateF(-10, 0);	break;
				case ZombieType::ZOMBIE_YETI:			aZombieGraphics.TranslateF(0, 4);		break;
				case ZombieType::ZOMBIE_CATAPULT:		aZombieGraphics.TranslateF(-24, -1);	break;
				case ZombieType::ZOMBIE_BOBSLED:		aZombieGraphics.TranslateF(0, -8);	break;
				case ZombieType::ZOMBIE_LADDER:			aZombieGraphics.TranslateF(0, -3);	break;
				default: break;
				}
				if (ZombieHasSilhouette(aZombieType))
				{
					aZombieGraphics.SetColor(Color(0, 0, 0, 40));
					aZombieGraphics.SetColorizeImages(true);
				}
				mApp->mReanimatorCache->DrawCachedZombie(&aZombieGraphics, 0, 0, aZombieTypeToDraw);
				aZombieGraphics.SetColorizeImages(false);

				g->DrawImage(Sexy::IMAGE_ALMANAC_ZOMBIEWINDOW2, aPosX, aPosY);
				if (aZombieType == aZombieMouseOn)
				{
					g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
					g->SetColor(Color(255, 255, 255, 48));
					g->SetColorizeImages(true);
					g->DrawImage(Sexy::IMAGE_ALMANAC_ZOMBIEWINDOW2, aPosX, aPosY);
					g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
					g->SetColorizeImages(false);
				}
			}
		}
	}

	g->DrawImage(mZombie->mZombieType == ZombieType::ZOMBIE_ZAMBONI || mZombie->mZombieType == ZombieType::ZOMBIE_BOBSLED ?
		Sexy::IMAGE_ALMANAC_GROUNDICE : Sexy::IMAGE_ALMANAC_GROUNDDAY, 518, 110);
	if (mZombie && !ZombieHasSilhouette(mZombie->mZombieType))
	{
		Graphics aZombieGraphics = Graphics(*g);
		mZombie->BeginDraw(&aZombieGraphics);
		aZombieGraphics.SetClipRect(-42, -51, 197, 187);
		switch (mZombie->mZombieType)
		{
		case ZombieType::ZOMBIE_ZAMBONI:		aZombieGraphics.TranslateF(-30, 5);		break;
		case ZombieType::ZOMBIE_GARGANTUAR:		aZombieGraphics.TranslateF(0, 40);		break;
		case ZombieType::ZOMBIE_FOOTBALL:		aZombieGraphics.TranslateF(-10, 0);		break;
		case ZombieType::ZOMBIE_BALLOON:		aZombieGraphics.TranslateF(0, -20);		break;
		case ZombieType::ZOMBIE_BUNGEE:			aZombieGraphics.TranslateF(15, 0);		break;
		case ZombieType::ZOMBIE_CATAPULT:		aZombieGraphics.TranslateF(-10, 0);		break;
		case ZombieType::ZOMBIE_BOSS:			aZombieGraphics.TranslateF(-540, -175);	break;
		default: break;
		}
		if (mZombie->mZombieType != ZombieType::ZOMBIE_BUNGEE && mZombie->mZombieType != ZombieType::ZOMBIE_BOSS &&
			mZombie->mZombieType != ZombieType::ZOMBIE_ZAMBONI && mZombie->mZombieType != ZombieType::ZOMBIE_CATAPULT)
			mZombie->DrawShadow(&aZombieGraphics);
		mZombie->Draw(&aZombieGraphics);
	}
	g->DrawImage(Sexy::IMAGE_ALMANAC_ZOMBIECARD, 455, 78);

	const ZombieDefinition& aZombieDef = GetZombieDefinition(mSelectedZombie);
	std::string aName = ZombieHasSilhouette(mSelectedZombie) ? "???" : std::format("[{}]", aZombieDef.mZombieName);
	PvzpDrawString(g, aName, 613, 362, Sexy::FONT_DWARVENTODCRAFT18GREENINSET, Color(190, 255, 235, 255), DS_ALIGN_CENTER);

	std::string aDescription;
	DrawStringJustification aAlign;
	if (ZombieHasDescription(mSelectedZombie))
	{
		aDescription = PvzpStringTranslate(std::format("[{}_DESCRIPTION]", aZombieDef.mZombieName));
		aAlign = DS_ALIGN_LEFT;
	}
	else
	{
		aDescription = "[NOT_ENCOUNTERED_YET]";
		aAlign = DS_ALIGN_CENTER_VERTICAL_MIDDLE;
	}
	for (PvzpStringListFormat& aFormat : gLawnStringFormats)
	{
		if (TestBit(aFormat.mFormatFlags, PvzpStringFormatFlag::PVZP_FORMAT_HIDE_UNTIL_MAGNETSHROOM))
		{
			if (mApp->HasSeedType(SeedType::SEED_MAGNETSHROOM))
			{
				aFormat.mNewColor.mAlpha = 255;
				aFormat.mLineSpacingOffset = 0;
			}
			else
			{
				aFormat.mNewColor.mAlpha = 0;
				aFormat.mLineSpacingOffset = -17;
			}
		}
	}
	// todo: fix stuff that have another formatter after them, ex: "{KEYWORD}Weakness:{STAT} fume-shroom{METAL} and magnet-shroom{KEYWORD}" (magnet-shroom will show with the {KEYWORD} colors)
	PvzpDrawStringWrapped(g, aDescription, Rect(484, mSelectedZombie == ZombieType::ZOMBIE_ZAMBONI ? 372 : 377, 258, 170), Sexy::FONT_BRIANNETOD12, Color(40, 50, 90), aAlign);
}

void AlmanacDialog::Draw(Graphics* g)
{
	g->SetLinearBlend(true);
	switch (mOpenPage)
	{
	case AlmanacPage::ALMANAC_PAGE_INDEX:	DrawIndex(g);	break;
	case AlmanacPage::ALMANAC_PAGE_PLANTS:	DrawPlants(g);	break;
	case AlmanacPage::ALMANAC_PAGE_ZOMBIES:	DrawZombies(g);	break;
	}

	for (const std::unique_ptr<Zombie>& aZombie : mZombiePerfTest)
	{
		if (aZombie)
		{
			Graphics aTestGraphics = Graphics(*g);
			aZombie->Draw(&aTestGraphics);
		}
	}

	mCloseButton->Draw(g);
	mIndexButton->Draw(g);
	mPlantButton->Draw(g);
	mZombieButton->Draw(g);
}

void AlmanacDialog::GetSeedPosition(SeedType theSeedType, int& x, int& y)
{
	if (theSeedType == SeedType::SEED_IMITATER)
		x = 20, y = 23;
	else
	{
		x = theSeedType % 8 * 52 + 26;
		y = theSeedType / 8 * 78 + 92;
	}
}

SeedType AlmanacDialog::SeedHitTest(int x, int y)
{
	if (mMouseVisible && mOpenPage == AlmanacPage::ALMANAC_PAGE_PLANTS)
	{
		for (SeedType aSeedType = SeedType::SEED_PEASHOOTER; aSeedType < NUM_ALMANAC_SEEDS; aSeedType = (SeedType)(aSeedType + 1))
		{
			if (mApp->HasSeedType(aSeedType))
			{
				int aSeedX, aSeedY;
				GetSeedPosition(aSeedType, aSeedX, aSeedY);
				Rect aSeedRect = aSeedType == SeedType::SEED_IMITATER ? Rect(aSeedX, aSeedY, 34, 46) : Rect(aSeedX, aSeedY, SEED_PACKET_WIDTH, SEED_PACKET_HEIGHT);
				if (aSeedRect.Contains(x, y)) return aSeedType;
			}
		}
	}
	return SeedType::SEED_NONE;
}

bool AlmanacDialog::ZombieHasSilhouette(ZombieType theZombieType)
{
	// only the Yeti can be a silhouette, and not once it can spawn
	if (theZombieType != ZombieType::ZOMBIE_YETI || mApp->CanSpawnYetis())
		return false;

	// the silhouette shows once adventure is finished or the Yeti's debut level is passed
	return mApp->HasFinishedAdventure() || mApp->mPlayerInfo->GetLevel() > GetZombieDefinition(ZombieType::ZOMBIE_YETI).mStartingLevel;
}

bool AlmanacDialog::ZombieIsShown(ZombieType theZombieType)
{
	// trial mode only shows zombies up to the Snorkel Zombie
	if (mApp->IsTrialStageLocked() && theZombieType > ZombieType::ZOMBIE_SNORKEL)
		return false;

	// the Yeti is shown once it can spawn, or earlier as a silhouette
	if (theZombieType == ZombieType::ZOMBIE_YETI)
		return mApp->CanSpawnYetis() || ZombieHasSilhouette(ZombieType::ZOMBIE_YETI);

	// zombies encountered in adventure mode
	if (theZombieType <= ZombieType::ZOMBIE_BOSS)
	{
		// once adventure is finished, every zombie is shown
		if (mApp->HasFinishedAdventure())
			return true;

		int aLevel = mApp->mPlayerInfo->GetLevel();
		int aStart = GetZombieDefinition(theZombieType).mStartingLevel;
		// must have reached the zombie's debut level; spawn-only zombies also require passing it or defeating the zombie once
		return aStart <= aLevel && (aStart != aLevel || !Board::IsZombieTypeSpawnedOnly(theZombieType) || gZombieDefeated[theZombieType]);
	}

	return false;
}

bool AlmanacDialog::ZombieHasDescription(ZombieType theZombieType)
{
	int aLevel = mApp->mPlayerInfo->GetLevel();
	int aStart = GetZombieDefinition(theZombieType).mStartingLevel;

	if (theZombieType == ZombieType::ZOMBIE_YETI)
	{
		// no description until the Yeti can spawn
		if (!mApp->CanSpawnYetis())
			return false;
		// from the third playthrough on, the description always shows
		if (mApp->mPlayerInfo->mFinishedAdventure >= 2)
			return true;
	}
	// other zombies always show their description once adventure is finished
	else if (mApp->HasFinishedAdventure())
		return true;

	// otherwise require reaching the zombie's debut level, plus passing it or defeating the zombie
	return aStart <= aLevel && (aStart != aLevel || gZombieDefeated[theZombieType]);
}

void AlmanacDialog::GetZombiePosition(ZombieType theZombieType, int& x, int& y)
{
	if (theZombieType == ZombieType::ZOMBIE_BOSS)
		x = 192, y = 486;
	else
	{
		x = theZombieType % 5 * 85 + 22;
		y = theZombieType / 5 * 80 + 86;
	}
}

ZombieType AlmanacDialog::ZombieHitTest(int x, int y)
{
	if (mMouseVisible && mOpenPage == AlmanacPage::ALMANAC_PAGE_ZOMBIES)
	{
		for (int i = 0; i < NUM_ALMANAC_ZOMBIES; i++)
		{
			ZombieType aZombieType = GetZombieType(i);
			if (aZombieType != ZombieType::ZOMBIE_INVALID && ZombieIsShown(aZombieType))
			{
				int aZombieX, aZombieY;
				GetZombiePosition(aZombieType, aZombieX, aZombieY);
				if (Rect(aZombieX, aZombieY, 76, 76).Contains(x, y))
					return aZombieType;
			}
		}
	}
	return ZombieType::ZOMBIE_INVALID;
}

void AlmanacDialog::MouseUp([[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int theClickCount)
{
	if (mPlantButton->IsMouseOver())		SetPage(ALMANAC_PAGE_PLANTS);
	else if (mZombieButton->IsMouseOver())	SetPage(ALMANAC_PAGE_ZOMBIES);
	else if (mCloseButton->IsMouseOver())	mApp->KillAlmanacDialog();
	else if (mIndexButton->IsMouseOver())	SetPage(ALMANAC_PAGE_INDEX);
}

void AlmanacDialog::MouseDown(int x, int y, [[maybe_unused]] int theClickCount)
{
	if (mPlantButton->IsMouseOver() || mCloseButton->IsMouseOver() || mIndexButton->IsMouseOver())
		mApp->PlaySample(Sexy::SOUND_TAP);
	if (mZombieButton->IsMouseOver())
		mApp->PlaySample(Sexy::SOUND_GRAVEBUTTON);

	SeedType aSeedType = SeedHitTest(x, y);
	if (aSeedType != SeedType::SEED_NONE && aSeedType != mSelectedSeed)
	{
		mSelectedSeed = aSeedType;
		SetupPlant();
		mApp->PlaySample(Sexy::SOUND_TAP);
	}
	ZombieType aZombieType = ZombieHitTest(x, y);
	if (aZombieType != ZombieType::ZOMBIE_INVALID && aZombieType != mSelectedZombie)
	{
		mSelectedZombie = aZombieType;
		SetupZombie();
		mApp->PlaySample(Sexy::SOUND_TAP);
	}
}

void AlmanacDialog::KeyDown(KeyCode theKey)
{
	if (theKey == KeyCode::KEYCODE_ESCAPE)
	{
		if (mOpenPage == AlmanacPage::ALMANAC_PAGE_INDEX)
			mApp->KillAlmanacDialog();
		else
			SetPage(AlmanacPage::ALMANAC_PAGE_INDEX);
		return;
	}

	LawnDialog::KeyDown(theKey);
}

void AlmanacInitForPlayer()
{
	for (int i = 0; i < ZombieType::NUM_ZOMBIE_TYPES; i++)
		gZombieDefeated[i] = false;
}

void AlmanacPlayerDefeatedZombie(ZombieType theZombieType)
{
	gZombieDefeated[theZombieType] = true;
}
