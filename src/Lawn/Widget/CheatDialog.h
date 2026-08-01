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

#ifndef __CHEATDIALOG_H__
#define __CHEATDIALOG_H__

#include "LawnDialog.h"
#include "widget/EditListener.h"

namespace Sexy
{
	class EditWidget;
}

class CheatDialog : public LawnDialog, public EditListener
{
public:
	LawnApp*			mApp;
	EditWidget*			mLevelEditWidget;

public:
	CheatDialog(LawnApp* theApp);
	~CheatDialog() override;

	int					GetPreferredHeight(int theWidth) override;
	void				Resize(int theX, int theY, int theWidth, int theHeight) override;
	void				AddedToManager(WidgetManager* theWidgetManager) override;
	void				RemovedFromManager(WidgetManager* theWidgetManager) override;
	void				Draw(Graphics* g) override;
	void				EditWidgetText(int theId, const std::string& theString) override;
	virtual bool		AllowChar(int theId, char theChar);
	bool				ApplyCheat();
};

#endif
