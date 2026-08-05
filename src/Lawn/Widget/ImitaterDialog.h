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

#ifndef __IMITATERDIALOG_H__
#define __IMITATERDIALOG_H__

#include "LawnDialog.h"

class ToolTipWidget;
class ImitaterDialog : public LawnDialog
{
public:
	ToolTipWidget*          mToolTip;
	SeedType                mToolTipSeed;

public:
	ImitaterDialog();
	~ImitaterDialog() override;

	SeedType                SeedHitTest(int x, int y);
	void                    UpdateCursor();
	void                    Update() override;
	void         GetSeedPosition(int theIndex, int& x, int& y);
	void                    Draw(Graphics* g) override;
	void                    ShowToolTip();
	void         RemoveToolTip();
	void                    MouseDown(int x, int y, int theClickCount) override;
	void                    MouseUp(int, int, int) override{}
};

#endif
