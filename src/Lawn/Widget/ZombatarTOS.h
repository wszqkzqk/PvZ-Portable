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

#ifndef __ZOMBATARTOS_H__
#define __ZOMBATARTOS_H__

#include "LawnDialog.h"
#include "widget/SliderListener.h"
#include "widget/CheckboxListener.h"

class LawnApp;
class NewLawnButton;
namespace Sexy
{
	class Slider;
	class Checkbox;
}

// Modal Terms-of-Service dialog shown the first time a profile enters Zombatar.
class ZombatarTOS : public LawnDialog, public Sexy::SliderListener, public Sexy::CheckboxListener
{
protected:
	enum
	{
		ZombatarTOS_Checkbox = 500,
		ZombatarTOS_Accept = 501,
		ZombatarTOS_Back = 502,
		ZombatarTOS_Slider = 503
	};

public:
	Sexy::Slider*				mTOSSlider;
	NewLawnButton*				mBackButton;
	NewLawnButton*				mAcceptButton;
	Sexy::Checkbox*				mTOSCheckbox;
	int							mTextHeight;
	bool						mFlashArrow;
	int							mArrowAlpha;
	int							mArrowFadeDir;
	std::string					mBodyText;

public:
	ZombatarTOS(LawnApp* theApp);
	~ZombatarTOS() override;

	void						Draw(Graphics* g) override;
	void						Update() override;
	void						AddedToManager(WidgetManager* theWidgetManager) override;
	void						RemovedFromManager(WidgetManager* theWidgetManager) override;
	void						Resize(int theX, int theY, int theWidth, int theHeight) override;
	void						ButtonPress(int theId) override;
	void						ButtonDepress(int theId) override;
	void						KeyDown(KeyCode theKey) override;
	void						MouseWheel(int theDelta) override;
	void						CheckboxChecked(int theId, bool checked) override;
	void						SliderVal(int theId, double theVal) override;
};

#endif
