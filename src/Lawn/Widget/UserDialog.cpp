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

#include "UserDialog.h"
#include "GameButton.h"
#include "../../LawnApp.h"
#include "../../Resources.h"
#include "../System/ProfileMgr.h"
#include "../System/PlayerInfo.h"
#include "../../PvzpLib/PvzpStringFile.h"
#include "widget/ListWidget.h"

static constexpr ListWidgetColorScheme gUserListWidgetColors{
	.mBkg = Color(23, 24, 35),
	.mOutline = Color(0, 0, 0),
	.mText = Color(235, 225, 180),
	.mHilite = Color(255, 255, 255),
	.mSelect = Color(20, 180, 15),
	.mSelectText = std::nullopt,
};

// these dialogs don't have localizations
UserDialog::UserDialog(LawnApp* theApp) : LawnDialog(theApp, Dialogs::DIALOG_USERDIALOG, true, theApp->GetString("WHO_ARE_YOU", "WHO ARE YOU?"), "", "", Dialog::BUTTONS_OK_CANCEL)
{
	mVerticalCenterText = false;
	mUserList = std::make_unique<ListWidget>(0, FONT_BRIANNETOD16, this);
	mUserList->SetColors(gUserListWidgetColors);
	mUserList->mDrawOutline = true;
	mUserList->mJustify = ListWidget::JUSTIFY_CENTER;
	mUserList->mItemHeight = 24;

	mRenameButton = MakeButton(UserDialog::UserDialog_RenameUser, this, mApp->GetString("RENAME_BUTTON", "Rename"));
	mDeleteButton = MakeButton(UserDialog::UserDialog_DeleteUser, this, mApp->GetString("DELETE_BUTTON", "Delete"));

	mNumUsers = 0;
	if (theApp->mPlayerInfo)
	{
		mUserList->SetSelect(mUserList->AddLine(theApp->mPlayerInfo->mName, false));
		mNumUsers++;
	}

	const ProfileMap& aMap = theApp->mProfileMgr->GetProfileMap();
	for (ProfileMap::const_iterator anItr = aMap.begin(); anItr != aMap.end(); anItr++)
	{
		if (theApp->mPlayerInfo && anItr->second.mName == theApp->mPlayerInfo->mName)
		{
			continue;
		}

		mUserList->AddLine(anItr->second.mName, false);
		mNumUsers++;
	}

	if (mNumUsers < 8)
	{
		mUserList->AddLine(mApp->GetString("CREATE_NEW_USER", "(Create a New User)"), false);
	}

	mTallBottom = true;
	CalcSize(210, 270);
}

UserDialog::~UserDialog() = default;

void UserDialog::Resize(int theX, int theY, int theWidth, int theHeight)
{
	LawnDialog::Resize(theX, theY, theWidth, theHeight);
	mUserList->Resize(GetLeft() + 30, GetTop() + 4, GetWidth() - 60, 200);
	mRenameButton->Layout(LayoutFlags::LAY_SameLeft | LayoutFlags::LAY_Above | LayoutFlags::LAY_SameHeight | LayoutFlags::LAY_SameWidth, mLawnYesButton.get(), 0, 0, 0, 0);
	mDeleteButton->Layout(LayoutFlags::LAY_SameLeft | LayoutFlags::LAY_Above | LayoutFlags::LAY_SameHeight | LayoutFlags::LAY_SameWidth, mLawnNoButton.get(), 0, 0, 0, 0);
}

int UserDialog::GetPreferredHeight(int theWidth)
{
	return LawnDialog::GetPreferredHeight(theWidth) + 190;
}

void UserDialog::AddedToManager(WidgetManager* theWidgetManager)
{
	LawnDialog::AddedToManager(theWidgetManager);
	AddWidget(mUserList.get());
	AddWidget(mDeleteButton.get());
	AddWidget(mRenameButton.get());
}

void UserDialog::RemovedFromManager(WidgetManager* theWidgetManager)
{
	LawnDialog::RemovedFromManager(theWidgetManager);
	RemoveWidget(mUserList.get());
	RemoveWidget(mDeleteButton.get());
	RemoveWidget(mRenameButton.get());
}

std::string UserDialog::GetSelName()
{
	if (mUserList->mSelectIdx < 0 || mUserList->mSelectIdx >= mNumUsers)
	{
		return "";
	}
	return mUserList->GetStringAt(mUserList->mSelectIdx);
}

void UserDialog::FinishDeleteUser()
{
	int aSelIdx = mUserList->mSelectIdx;
	mUserList->RemoveLine(mUserList->mSelectIdx);

	aSelIdx--;
	aSelIdx = std::max(aSelIdx, 0);
	if (mUserList->GetLineCount() > 0)
	{
		mUserList->SetSelect(aSelIdx);
	}

	mNumUsers--;
	if (mNumUsers == 7)
	{
		mUserList->AddLine(mApp->GetString("CREATE_NEW_USER", "(Create a New User)"), false);
	}
}

void UserDialog::FinishRenameUser(const std::string& theNewName)
{
	if (mUserList->mSelectIdx < mNumUsers)
	{
		mUserList->SetLine(mUserList->mSelectIdx, theNewName);
	}
}

void UserDialog::Draw(Graphics* g)
{
	LawnDialog::Draw(g);
}

void UserDialog::ListClicked([[maybe_unused]] int theId, int theIdx, int theClickCount)
{
	if (theIdx == mNumUsers)
	{
		mApp->DoCreateUserDialog();
	}
	else
	{
		mUserList->SetSelect(theIdx);
		if (theClickCount == 2)
		{
			mApp->FinishUserDialog(true);
		}
	}
}

void UserDialog::ButtonDepress(int theId)
{
	LawnDialog::ButtonDepress(theId);
	std::string aSelName = GetSelName();
	if (!aSelName.empty())
	{
		switch (theId)
		{
		case UserDialog::UserDialog_RenameUser:
			mApp->DoRenameUserDialog(aSelName);
			break;

		case UserDialog::UserDialog_DeleteUser:
			mApp->DoConfirmDeleteUserDialog(aSelName);
			break;
		}
	}
}

void UserDialog::EditWidgetText([[maybe_unused]] int theId, [[maybe_unused]] const std::string& theString)
{
	mApp->ButtonDepress(mId + 2000);
}

bool UserDialog::AllowChar([[maybe_unused]] int theId, char theChar)
{
	return isdigit(theChar);
}
