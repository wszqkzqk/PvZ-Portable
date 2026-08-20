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

#ifndef __NEWOPTIONSDIALOG_H__
#define __NEWOPTIONSDIALOG_H__

#include "widget/Dialog.h"
#include "widget/SliderListener.h"
#include "widget/CheckboxListener.h"
#include <memory>

class LawnApp;
class LawnStoneButton;
class NewLawnButton;
namespace Sexy
{
	class Slider;
	class Checkbox;
};

class NewOptionsDialog : public Sexy::Dialog, public Sexy::SliderListener, public Sexy::CheckboxListener
{
protected:
	enum
	{
		NewOptionsDialog_Almanac,
		NewOptionsDialog_MainMenu,
		NewOptionsDialog_Restart,
		NewOptionsDialog_Update,
		NewOptionsDialog_MusicVolume,
		NewOptionsDialog_SoundVolume,
		NewOptionsDialog_Fullscreen,
		NewOptionsDialog_HardwareAcceleration,
	};

public:
	LawnApp*							mApp;
	std::unique_ptr<Sexy::Slider>		mMusicVolumeSlider;
	std::unique_ptr<Sexy::Slider>		mSfxVolumeSlider;
	std::unique_ptr<Sexy::Checkbox>		mFullscreenCheckbox;
	std::unique_ptr<Sexy::Checkbox>		mHardwareAccelerationCheckbox;
	std::unique_ptr<LawnStoneButton>	mAlmanacButton;
	std::unique_ptr<LawnStoneButton>	mBackToMainButton;
	std::unique_ptr<LawnStoneButton>	mRestartButton;
	std::unique_ptr<NewLawnButton>		mBackToGameButton;
	bool								mFromGameSelector;

public:
	NewOptionsDialog(LawnApp* theApp, bool theFromGameSelector);
	~NewOptionsDialog() override;

	int						GetPreferredHeight(int theWidth) override;
	void					AddedToManager(Sexy::WidgetManager* theWidgetManager) override;
	void					RemovedFromManager(Sexy::WidgetManager* theWidgetManager) override;
	void					Resize(int theX, int theY, int theWidth, int theHeight) override;
	void					Draw(Sexy::Graphics* g) override;
	void					SliderVal(int theId, double theVal) override;
	void					CheckboxChecked(int theId, bool checked) override;
	void					ButtonPress(int theId) override;
	void					ButtonDepress(int theId) override;
	void					KeyDown(Sexy::KeyCode theKey) override;
};

#endif
