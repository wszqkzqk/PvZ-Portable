/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef __ZOMBATARWIDGET_H__
#define __ZOMBATARWIDGET_H__

#include "widget/Widget.h"

class GameSelector;
class LawnApp;
class PlayerInfo;

using namespace Sexy;

enum ZombatarPage
{
    ZOMBATAR_PAGE_SKIN,
    ZOMBATAR_PAGE_HAIR,
    ZOMBATAR_PAGE_FACIAL_HAIR,
    ZOMBATAR_PAGE_TIDBITS,
    ZOMBATAR_PAGE_EYEWEAR,
    ZOMBATAR_PAGE_CLOTHES,
    ZOMBATAR_PAGE_ACCESSORY,
    ZOMBATAR_PAGE_HATS,
    ZOMBATAR_PAGE_BACKDROPS,
    NUM_ZOMBATAR_PAGES
};

enum ZombatarWidgetState
{
    ZOMBATAR_STATE_TOS,
    ZOMBATAR_STATE_LIST,
    ZOMBATAR_STATE_CREATE,
    ZOMBATAR_STATE_CONFIRM
};

class ZombatarWidget : public Widget
{
public:
    GameSelector*               mGameSelector;
    LawnApp*                    mApp;
    ZombatarWidgetState         mState;
    ZombatarPage                mPage;
    int                         mCurrentIndex;
    int                         mFacialHairPage;
    int                         mMouseX;
    int                         mMouseY;
    int                         mTosScroll;
    bool                        mTosAccepted;
    int                         mPart[NUM_ZOMBATAR_PAGES];
    int                         mColor[NUM_ZOMBATAR_PAGES];

public:
    ZombatarWidget(GameSelector* theGameSelector);
    virtual ~ZombatarWidget();

    void                        Open();
    void                        ResetDraft();
    void                        LoadCurrentToDraft();
    bool                        SaveDraft();
    void                        DeleteCurrent();
    bool                        CanSaveNewHead() const;
    int                         GetHeadCount() const;
    void                        ClampCurrentIndex();
    virtual void                Draw(Graphics* g);
    virtual void                MouseMove(int x, int y);
    virtual void                MouseDrag(int x, int y);
    virtual void                MouseWheel(int theDelta);
    virtual void                MouseDown(int x, int y, int theClickCount);
    virtual void                KeyDown(KeyCode theKey);

private:
    void                        DrawMain(Graphics* g);
    void                        DrawTOS(Graphics* g);
    void                        DrawList(Graphics* g);
    void                        DrawCreate(Graphics* g);
    void                        DrawAvatar(Graphics* g, int theX, int theY, const unsigned char* theRecord);
    void                        DrawDraftAvatar(Graphics* g, int theX, int theY);
    void                        DrawImageColorized(Graphics* g, Image* theImage, int theX, int theY, int theColorIndex);
    Rect                        GetCategoryRect(int theIndex) const;
    Rect                        GetItemRect(int theIndex) const;
    Rect                        GetColorRect(int theIndex) const;
    int                         GetItemCountForPage() const;
    bool                        PageAllowsNone() const;
    bool                        PageAllowsColors() const;
    Image*                      GetCategoryImage(ZombatarPage thePage, bool theSelected, bool theOver) const;
    Image*                      GetPartImage(ZombatarPage thePage, int theIndex) const;
    Image*                      GetPartMaskImage(ZombatarPage thePage, int theIndex) const;
    Image*                      GetBackgroundImage(int theIndex) const;
    bool                        ExportAvatarPNG(const unsigned char* theRecord, int theExportIndex);
    bool                        ExportAllAvatarPNGs();
    void                        EraseAvatarPNG(int theExportIndex);
    void                        DrawPartImage(Graphics* g, ZombatarPage thePage, int theIndex, int theX, int theY, int theColorIndex);
    void                        DrawButtonImage(Graphics* g, Image* theImage, Image* theHighlightImage, const Rect& theRect, bool theEnabled);
    void                        DecodeRecord(const unsigned char* theRecord, int* thePart, int* theColor) const;
    void                        EncodeRecord(unsigned char* theRecord) const;
    void                        BackToSelector();
    void                        ShowMaxHeadsMessage();
};

#endif
