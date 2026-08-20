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

#ifndef __STORESCREEN_H__
#define __STORESCREEN_H__

#include "../../ConstEnums.h"
#include "../System/PlayerInfo.h"
#include "../../PvzpLib/DataArray.h"
#include "widget/Dialog.h"
#include <memory>
//using namespace std;
using namespace Sexy;

#define MAX_PAGE_SPOTS 8
#define MAX_PURCHASES 80

class Coin;
class LawnApp;
class NewLawnButton;

class StoreScreen : public Dialog
{
private:
	enum
	{
		StoreScreen_Back = 100,
		StoreScreen_Prev = 101,
		StoreScreen_Next = 102
	};

public:
	LawnApp*                    mApp;
	std::unique_ptr<NewLawnButton>  mBackButton;
	std::unique_ptr<NewLawnButton>  mPrevButton;
	std::unique_ptr<NewLawnButton>  mNextButton;
	std::unique_ptr<Widget>         mOverlayWidget;
	int                         mStoreTime;
	std::string                 mBubbleText;
	int                         mBubbleCountDown;
	bool                        mBubbleClickToContinue;
	int                         mAmbientSpeechCountDown;
	int                         mPreviousAmbientSpeechIndex;
	StorePages                  mPage;
	StoreItem                   mMouseOverItem;
	int                         mHatchTimer;
	bool                        mHatchOpen;
	int                         mShakeX;
	int                         mShakeY;
	int                         mStartDialog;
	bool                        mEasyBuyingCheat;
	bool                        mWaitForDialog;
	PottedPlant                 mPottedPlantSpecs;
	DataArray<Coin>             mCoins;
	bool                        mDrawnOnce;
	uint                        mAddedAtUpdateCount; // tick snapshot for demo-mode preload timing
	bool                        mGoToTreeNow;
	bool                        mPurchasedFullVersion;
	bool                        mTrialLockedWhenStoreOpened;

public:
	StoreScreen(LawnApp* theApp);
	~StoreScreen() override;

	StoreItem        GetStoreItemType(int theSpotIndex);
	bool                        IsFullVersionOnly(StoreItem theStoreItem);
	static bool      IsPottedPlant(StoreItem theStoreItem);
	bool                        IsComingSoon(StoreItem theStoreItem);
	bool                        IsItemSoldOut(StoreItem theStoreItem);
	bool                        IsItemUnavailable(StoreItem theStoreItem);
	static void      GetStorePosition(int theSpotIndex, int& thePosX, int& thePosY);
	void                        DrawItemIcon(Graphics* g, int theItemPosition, StoreItem theItemType, bool theIsForHighlight);
	void                        DrawItem(Graphics* g, int theItemPosition, StoreItem theItemType);
	void                        Draw(Graphics* g) override;
	void                        DrawOverlay(Graphics* g) override;
	void             SetBubbleText(int theCrazyDaveMessage, int theTime, bool theClickToContinue);
	void                        UpdateMouse();
	void                        StorePreload();
	bool             CanInteractWithButtons();
	void                        Update() override;
	void                        AddedToManager(WidgetManager* theWidgetManager) override;
	void                        RemovedFromManager(WidgetManager* theWidgetManager) override;
	void                        ButtonPress(int theId) override;
	bool             IsPageShown(StorePages thePage);
	void                        ButtonDepress(int theId) override;
	void                        KeyDown(KeyCode theKey) override;
	static int		GetItemCost(StoreItem theStoreItem);
	bool             CanAffordItem(StoreItem theStoreItem);
	void                        PurchaseItem(StoreItem theStoreItem);
	void                        AdvanceCrazyDaveDialog();
	void                        MouseDown(int x, int y, int theClickCount) override;
	void             EnableButtons(bool theEnable);
	void                        SetupForIntro(int theDialogIndex);
};

class StoreScreenOverlay : public Widget
{
public:
	StoreScreen*                mParent;

public:
	StoreScreenOverlay(StoreScreen* theParent);
	void                        Draw(Graphics* g) override;
};


#endif
