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

#ifndef __SEXY_RESOURCEMANAGER_H__
#define __SEXY_RESOURCEMANAGER_H__

#include "Common.h"
#include "graphics/Image.h"
#include "SexyAppBase.h"
#include "XMLParser.h"
#include <memory>
#include <string>
#include <map>

namespace ImageLib
{
class Image;
};

namespace Sexy
{

class XMLParser;
class XMLElement;
class Image;
class SoundInstance;
class SexyAppBase;
class _Font;

class ResourceManager
{
public: // TODO: revert to protected
	enum ResType
	{
		ResType_Image,
		ResType_Sound,
		ResType_Font
	};


	struct BaseRes
	{
		ResType mType;
		std::string mId;
		std::string mResGroup;
		std::string mPath;
		XMLParamMap mXMLAttributes;
		bool mFromProgram;

		virtual ~BaseRes() = default;
		virtual void DeleteResource() { }
	};

	struct ImageRes : public BaseRes
	{
		SharedImageRef mImage;
		std::string mAlphaImage;
		std::string mAlphaGridImage;
		std::string mVariant;
		bool mAutoFindAlpha;
		bool mPalletize;
		bool mA4R4G4B4;
		bool mA8R8G8B8;
		bool mDDSurface;
		bool mPurgeBits;
		bool mMinimizeSubdivisions;
		int mRows;
		int mCols;
		uint32_t mAlphaColor;
		AnimInfo mAnimInfo;

		ImageRes() { mType = ResType_Image; }
		void         DeleteResource() override;
	};

	struct SoundRes : public BaseRes
	{
		intptr_t mSoundId;
		double mVolume;
		int mPanning;

		SoundRes() { mType = ResType_Sound; }
		void         DeleteResource() override;
	};

	struct FontRes : public BaseRes
	{
		std::unique_ptr<_Font> mFont;
		std::unique_ptr<Image> mImage;
		std::string mImagePath;
		std::string mTags;

		// For SysFonts
		bool mSysFont;
		bool mBold;
		bool mItalic;
		bool mUnderline;
		bool mShadow;
		int mSize;


		FontRes();
		void         DeleteResource() override;
	};

	typedef std::map<std::string,std::unique_ptr<BaseRes>> ResMap;
	typedef std::list<BaseRes*> ResList;
	typedef std::map<std::string,ResList,StringLessNoCase> ResGroupMap;

	std::set<std::string,StringLessNoCase> mLoadedGroups;

	ResMap					mImageMap;
	ResMap					mSoundMap;
	ResMap					mFontMap;

	std::unique_ptr<XMLParser>	mXMLParser;
	std::string				mError;
	bool					mHasFailed;
	SexyAppBase*			mApp;
	std::string				mCurResGroup;
	std::string				mDefaultPath;
	std::string				mDefaultIdPrefix;
	bool					mAllowMissingProgramResources;
	bool					mAllowAlreadyDefinedResources; // for reparsing file while running
	bool					mHadAlreadyDefinedError;

	ResGroupMap				mResGroupMap;
	ResList*				mCurResGroupList;
	ResList::iterator		mCurResGroupListItr;


	bool					Fail(const std::string& theErrorText);

	virtual bool			ParseCommonResource(XMLElement &theElement, std::unique_ptr<BaseRes> &theRes, ResMap &theMap);
	virtual bool			ParseSoundResource(XMLElement &theElement);
	virtual bool			ParseImageResource(XMLElement &theElement);
	virtual bool			ParseFontResource(XMLElement &theElement);
	virtual bool			ParseSetDefaults(XMLElement &theElement);
	virtual bool			ParseResources();

	bool					DoParseResources();
	void					DeleteMap(ResMap &theMap);
	virtual void			DeleteResources(ResMap &theMap, const std::string &theGroup);

	bool					LoadAlphaGridImage(ImageRes *theRes, GLImage *theImage);
	bool					LoadAlphaImage(ImageRes *theRes, GLImage *theImage);
	virtual bool			DoLoadImage(ImageRes *theRes);
	virtual bool			DoLoadFont(FontRes* theRes);
	virtual bool			DoLoadSound(SoundRes* theRes);

	int						GetNumResources(const std::string &theGroup, ResMap &theMap);

public:
	ResourceManager(SexyAppBase *theApp);
	virtual ~ResourceManager();

	bool					ParseResourcesFile(const std::string& theFilename);
	bool					ReparseResourcesFile(const std::string& theFilename);

	std::string				GetErrorText();
	bool					HadError();
	bool					IsGroupLoaded(const std::string &theGroup);

	int						GetNumImages(const std::string &theGroup);
	int						GetNumSounds(const std::string &theGroup);
	int						GetNumFonts(const std::string &theGroup);
	int						GetNumResources(const std::string &theGroup);

	virtual bool			LoadNextResource();
	virtual void			ResourceLoadedHook(BaseRes *theRes);

	virtual void			StartLoadResources(const std::string &theGroup);
	virtual bool			LoadResources(const std::string &theGroup);

	bool					ReplaceImage(const std::string &theId, Image *theImage);
	bool					ReplaceSound(const std::string &theId, intptr_t theSound);
	bool					ReplaceFont(const std::string &theId, _Font *theFont);

	void					DeleteImage(const std::string &theName);
	SharedImageRef			LoadImage(const std::string &theName);

	void					DeleteSound(const std::string &theName);

	void					DeleteFont(const std::string &theName);
	_Font*					LoadFont(const std::string &theName);

	SharedImageRef			GetImage(const std::string &theId);
	intptr_t					GetSound(const std::string &theId);
	_Font*					GetFont(const std::string &theId);

	// Returns all the XML attributes associated with the image
	const XMLParamMap&		GetImageAttributes(const std::string &theId);

	// These throw a ResourceManagerException if the resource is not found
	virtual SharedImageRef	GetImageThrow(const std::string &theId);
	virtual intptr_t			GetSoundThrow(const std::string &theId);
	virtual _Font*			GetFontThrow(const std::string &theId);

	void					SetAllowMissingProgramImages(bool allow);

	virtual void			DeleteResources(const std::string &theGroup);
	void					ReleaseTrackedResources(std::vector<std::string>& theNames);
	void					DeleteExtraImageBuffers(const std::string &theGroup);

	const ResList*			GetCurResGroupList()	{return mCurResGroupList;}
	std::string				GetCurResGroup()		{return mCurResGroup;}
	void					DumpCurResGroup(std::string& theDestStr);
};

struct ResourceManagerException : public std::exception
{
	std::string what;
	ResourceManagerException(const std::string &theWhat) : what(theWhat) { }
};

}

#endif //__PROPERTIESPARSER_H__
