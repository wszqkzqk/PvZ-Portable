/*
 * Portions of this file are based on the PopCap Games Framework
 * Copyright (C) 2005-2009 PopCap Games, Inc.
 * 
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
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

#ifndef __SEXYAPP_H__
#define __SEXYAPP_H__

#include "SexyAppBase.h"

namespace Sexy
{

class SexyApp : public SexyAppBase
{
public:	
	int						mBuildNum;
	std::string				mCommitDate;

	std::string				mUserName;	

public:
	void					UpdateFrames() override;

	void					WriteToRegistry() override;
	void					ReadFromRegistry() override;

public:
	SexyApp();
	~SexyApp() override;

	void					PreDisplayHook() override;
	void					InitPropertiesHook() override;
	void					Init() override;
	void					PreTerminate() override;

	void					HandleCmdLineParam(std::string_view theParamName, std::string_view theParamValue) override;
	std::string				GetGameSEHInfo() override;
};

extern SexyApp* gSexyApp;

};

#endif //__SEXYAPP_H__
