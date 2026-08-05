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

#ifndef __PROFILEMGR_H__
#define __PROFILEMGR_H__

#include <map>
#include <string>
#include "../../SexyAppFramework/Common.h"

class DataSync;
class PlayerInfo;
typedef std::pair<std::string, PlayerInfo> ProfilePair;
typedef std::map<std::string, PlayerInfo, Sexy::StringLessNoCase> ProfileMap;

class ProfileMgr
{
protected:
	ProfileMap			mProfileMap;
	unsigned long		mNextProfileId;
	unsigned long		mNextProfileUseSeq;

protected:
	void				SyncState(DataSync& theSync);
	void				DeleteOldestProfile();
	inline void			DeleteOldProfiles() { while(mProfileMap.size() > 200) DeleteOldestProfile(); }

public:
	bool				DeleteProfile(const std::string& theName);

protected:
	void		DeleteProfile(ProfileMap::iterator theProfile);

public:
	ProfileMgr() { Clear(); }
	virtual ~ProfileMgr() { ; }

	void		Clear();
	void				Load();
	void				Save();
	inline int			GetNumProfiles() const { return mProfileMap.size(); }
	PlayerInfo*			GetProfile(const std::string& theName);
	PlayerInfo*			AddProfile(const std::string& theName);
	PlayerInfo*			GetAnyProfile();
	bool				RenameProfile(const std::string& theOldName, const std::string& theNewName);
	inline ProfileMap&	GetProfileMap() { return mProfileMap; }
};

#endif
