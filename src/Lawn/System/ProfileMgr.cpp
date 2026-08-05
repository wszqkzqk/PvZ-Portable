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

#include "DataSync.h"
#include "ProfileMgr.h"
#include "PlayerInfo.h"
#include "../../SexyAppFramework/SexyAppBase.h"

using namespace Sexy;
static int gProfileVersion = 14;

PlayerInfo* ProfileMgr::GetAnyProfile()
{
	if (mProfileMap.size() == 0)
		return nullptr;

	PlayerInfo* aPlayerInfo = &mProfileMap.begin()->second;
	aPlayerInfo->LoadDetails();
	aPlayerInfo->mUseSeq = mNextProfileUseSeq++;
	return aPlayerInfo;
}

void ProfileMgr::Clear()
{
	mProfileMap.clear();
	mNextProfileId = 1U;
	mNextProfileUseSeq = 1U;
}

void ProfileMgr::SyncState(DataSync& theSync)
{
	DataReader* aReader = theSync.GetReader();
	DataWriter* aWriter = theSync.GetWriter();

	int aVersion = gProfileVersion;
	theSync.SyncUInt32(aVersion);
	theSync.SetVersion(aVersion);
	if (aVersion == gProfileVersion)
	{
		if (aReader)
		{
			mProfileMap.clear();

			ulong aMaxProfileId = 0;
			ulong aMaxUseSeq = 0;
			for (int aProfileCount = aReader->ReadUInt16(); aProfileCount > 0; aProfileCount--)
			{
				PlayerInfo aProfile;
				aProfile.SyncSummary(theSync);

				if (aProfile.mId > aMaxProfileId)
					aMaxProfileId = aProfile.mId;
				if (aProfile.mUseSeq > aMaxUseSeq)
					aMaxUseSeq = aProfile.mUseSeq;

				mProfileMap[aProfile.mName] = aProfile;
			}

			mNextProfileId = aMaxProfileId + 1;
			mNextProfileUseSeq = aMaxUseSeq + 1;
		}
		else
		{
			aWriter->WriteUInt16((uint16_t)mProfileMap.size());

			for (auto anItr = mProfileMap.begin(); anItr != mProfileMap.end(); anItr++)
				anItr->second.SyncSummary(theSync);
		}
	}
}

void ProfileMgr::Load()
{
	Buffer aBuffer;
	std::string aFileName = GetAppDataPath("userdata/users.dat");

	try
	{
		if (gSexyAppBase->ReadBufferFromFile(aFileName, &aBuffer, false))
		{
			DataReader aReader;
			aReader.OpenMemory(aBuffer.GetDataPtr(), aBuffer.GetDataLen(), false);
			DataSync aSync(aReader);
			SyncState(aSync);
		}
	}
	catch (DataReaderException&)
	{
		Clear();
	}
}

void ProfileMgr::Save()
{
	DataWriter aWriter;
	aWriter.OpenMemory(0x20);
	DataSync aSync(aWriter);
	SyncState(aSync);

	MkDir(GetAppDataPath("userdata"));
	std::string aFileName = GetAppDataPath("userdata/users.dat");
	gSexyAppBase->WriteBytesToFile(aFileName, aWriter.GetDataPtr(), aWriter.GetDataLen());
}

void ProfileMgr::DeleteProfile(ProfileMap::iterator theProfile)
{
	theProfile->second.DeleteUserFiles();
	mProfileMap.erase(theProfile);
}

bool ProfileMgr::DeleteProfile(const std::string& theName)
{
	auto anItr = mProfileMap.find(theName);
	if (anItr == mProfileMap.end())
		return false;

	DeleteProfile(anItr);
	return true;
}

bool ProfileMgr::RenameProfile(const std::string& theOldName, const std::string& theNewName)
{
	auto anOldItr = mProfileMap.find(theOldName);
	if (anOldItr == mProfileMap.end())
		return false;
	else
	{
		// rename in place when the name only differs in case; otherwise move the profile to the new name
		if (strcasecmp(theOldName.c_str(), theNewName.c_str()) == 0)
			anOldItr->second.mName = theNewName;
		else
		{
			// insert the old profile under the new name
			auto aRet = mProfileMap.emplace(theNewName, anOldItr->second);  // auto aRet = mProfileMap.insert({theNewName, anOldItr->second});
			// emplace fails when the new name already exists
			if (!aRet.second)
				return false;
			else
			{
				mProfileMap.erase(anOldItr);
				aRet.first->second.mName = theNewName;
			}
		}
		return true;
	}
}

void ProfileMgr::DeleteOldestProfile()
{
	if (mProfileMap.size() == 0)
		return;

	// find the profile with the smallest mUseSeq
	auto anOldest = mProfileMap.begin();
	for (auto anItr = anOldest; anItr != mProfileMap.end(); anItr++)
		if (anItr->second.mUseSeq < anOldest->second.mUseSeq)
			anOldest = anItr;
	DeleteProfile(anOldest);
}

PlayerInfo* ProfileMgr::GetProfile(const std::string& theName)
{
	auto anItr = mProfileMap.find(theName);
	if (anItr != mProfileMap.end())
	{
		PlayerInfo* aProfile = &anItr->second;
		aProfile->LoadDetails();
		aProfile->mUseSeq = mNextProfileUseSeq++;
		return aProfile;
	}
	return nullptr;
}

PlayerInfo* ProfileMgr::AddProfile(const std::string& theName)
{
	auto aRet = mProfileMap.emplace(theName, PlayerInfo());
	if (aRet.second)
	{
		PlayerInfo* aProfile = &aRet.first->second;
		aProfile->mName = theName;
		aProfile->mId = mNextProfileId++;
		aProfile->mUseSeq = mNextProfileUseSeq++;
		DeleteOldProfiles();
		return aProfile;
	}
	return nullptr;
}
