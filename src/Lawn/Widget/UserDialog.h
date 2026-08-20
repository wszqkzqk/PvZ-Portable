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

#ifndef __USERDIALOG_H__
#define __USERDIALOG_H__

#include "LawnDialog.h"
#include "widget/ListListener.h"
#include "widget/EditListener.h"
#include <memory>

namespace Sexy
{
	class ListWidget;
};

class UserDialog : public LawnDialog, public ListListener, public EditListener
{
protected:
	enum
	{
		UserDialog_RenameUser,
		UserDialog_DeleteUser
	};

public:
	std::unique_ptr<ListWidget>	mUserList;
	std::unique_ptr<DialogButton>	mRenameButton;
	std::unique_ptr<DialogButton>	mDeleteButton;
	int					mNumUsers;

public:
	UserDialog(LawnApp* theApp);
	~UserDialog() override;

	void				Resize(int theX, int theY, int theWidth, int theHeight) override;
	int					GetPreferredHeight(int theWidth) override;
	void				AddedToManager(WidgetManager* theWidgetManager) override;
	void				RemovedFromManager(WidgetManager* theWidgetManager) override;
	void 				ListClicked(int theId, int theIdx, int theClickCount) override;
	void 				ListClosed(int) override{}
	void 				ListHiliteChanged(int, int, int) override{}
	void				ButtonDepress(int theId) override;
	void				EditWidgetText(int theId, const std::string& theString) override;
	virtual bool		AllowChar(int theId, char theChar);
	void				Draw(Graphics* g) override;
	void				FinishDeleteUser();
	void				FinishRenameUser(const std::string& theNewName);
	std::string			GetSelName();
};

#endif
