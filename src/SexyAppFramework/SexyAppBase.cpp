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

//#define SEXY_TRACING_ENABLED
//#define SEXY_PERF_ENABLED
//#define SEXY_MEMTRACE

#include <string>
#include <fstream>
#include <time.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>

#include <SDL.h>

#ifdef __SWITCH__
#include <switch.h>
#include <locale>
#include <codecvt>
#elif defined(__3DS__)
#include <3ds.h>
#elif defined(__EMSCRIPTEN__)
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "SexyAppBase.h"
//#include "misc/SEHCatcher.h"
#include "widget/WidgetManager.h"
#include "widget/Widget.h"
#include "misc/Debug.h"
#include "misc/KeyCodes.h"
#include "graphics/GLInterface.h"
#include "graphics/GLImage.h"
#include "graphics/MemoryImage.h"
//#include "misc/HTTPTransfer.h"
#include "widget/Dialog.h"
#include "imagelib/ImageLib.h"
#include "sound/SDLSoundManager.h"
#include "sound/SDLSoundInstance.h"
#include "misc/Rect.h"
#include "misc/PropertiesParser.h"
#include "misc/PerfTimer.h"
#include "misc/MTRand.h"
#include "misc/ModVal.h"
//#include "graphics/SysFont.h"
#include "misc/ResourceManager.h"
#include "sound/SDLMusicInterface.h"
#include <mutex>
#include "misc/Debug.h"
#include "paklib/PakInterface.h"
#include "sound/DummyMusicInterface.h"
#include "misc/memmgr.h"
#include "misc/RegEmu.h"

using namespace Sexy;

const int DEMO_FILE_ID = 0x42BEEF78;
const int DEMO_VERSION = 2;

SexyAppBase* Sexy::gSexyAppBase = nullptr;

static bool gScreenSaverActive = false;

#ifndef SPI_GETSCREENSAVERRUNNING
#define SPI_GETSCREENSAVERRUNNING 114
#endif

static GLImage* gFPSImage = nullptr;

static SDL_SystemCursor CursorNumToSystemCursor(int theCursorNum)
{
	switch (theCursorNum)
	{
		case CURSOR_HAND:
			return SDL_SYSTEM_CURSOR_HAND;
		case CURSOR_TEXT:
			return SDL_SYSTEM_CURSOR_IBEAM;
		case CURSOR_CIRCLE_SLASH:
			return SDL_SYSTEM_CURSOR_NO;
		case CURSOR_SIZEALL:
			return SDL_SYSTEM_CURSOR_SIZEALL;
		case CURSOR_SIZENESW:
			return SDL_SYSTEM_CURSOR_SIZENESW;
		case CURSOR_SIZENS:
			return SDL_SYSTEM_CURSOR_SIZENS;
		case CURSOR_SIZENWSE:
			return SDL_SYSTEM_CURSOR_SIZENWSE;
		case CURSOR_SIZEWE:
			return SDL_SYSTEM_CURSOR_SIZEWE;
		case CURSOR_WAIT:
			return SDL_SYSTEM_CURSOR_WAIT;
		case CURSOR_DRAGGING:
		case CURSOR_POINTER:
		case CURSOR_NONE:
		case CURSOR_CUSTOM:
		default:
			return SDL_SYSTEM_CURSOR_ARROW;
	}
}

static SDL_Cursor* CreateCursorFromMemoryImage(MemoryImage* theImage)
{
	if (theImage == nullptr || theImage->mBits == nullptr)
		return nullptr;

	const int aWidth = theImage->GetWidth();
	const int aHeight = theImage->GetHeight();
	if (aWidth <= 0 || aHeight <= 0)
		return nullptr;

	SDL_Surface* aSurface = SDL_CreateRGBSurfaceWithFormatFrom(
		theImage->mBits,
		aWidth,
		aHeight,
		32,
		aWidth * static_cast<int>(sizeof(uint32_t)),
		SDL_PIXELFORMAT_BGRA32);
	if (aSurface == nullptr)
		return nullptr;

	SDL_Cursor* aCursor = SDL_CreateColorCursor(aSurface, 0, 0);
	SDL_FreeSurface(aSurface);
	return aCursor;
}

#ifdef __EMSCRIPTEN__
static void EmscriptenDeferredDoExit(void* arg)
{
	SexyAppBase* app = static_cast<SexyAppBase*>(arg);
	if (app != nullptr)
		app->Shutdown();
}
#endif

SexyAppBase::SexyAppBase()
{
	gSexyAppBase = this;

	SDL_Init(SDL_INIT_TIMER);

	mNotifyGameMessage = 0;

#ifdef PVZ_DEBUG
	mOnlyAllowOneCopyToRun = false;
#else
	mOnlyAllowOneCopyToRun = true;
#endif

#ifdef __SWITCH__
	mResourceDir = "sdmc:/switch/PvZPortable/";
#elif defined(__3DS__)
	mResourceDir = "sdmc:/3ds/PvZPortable/";
#elif defined(__ANDROID__) && !defined(__TERMUX__)
	const char* aExtPath = SDL_AndroidGetExternalStoragePath();
	if (aExtPath)
	{
		mResourceDir = std::string(aExtPath) + "/";
	}
	else
	{
		mResourceDir = "";
	}
#elif defined(__IPHONEOS__)
	// iOS: use Documents directory (visible in Files app via UIFileSharingEnabled)
	{
		const char* aHome = std::getenv("HOME");
		if (aHome != nullptr && aHome[0] != '\0')
		{
			mResourceDir = (std::filesystem::path(aHome) / "Documents").generic_string() + "/";
		}
		else
		{
			mResourceDir = "";
		}
	}
#elif defined(__EMSCRIPTEN__)
	mResourceDir = "/";
#else
	char* aBasePath = SDL_GetBasePath();
	if (aBasePath)
	{
		mResourceDir = aBasePath;
		SDL_free(aBasePath);
	}
	else
	{
		mResourceDir = "";
	}
#endif

	mNoDefer = false;	
	mFullScreenPageFlip = true; // should we page flip in fullscreen?
	mTimeLoaded = SDL_GetTicks();
	mSEHOccured = false;
	mProdName = "Product";
	mTitle = "SexyApp";
	mShutdown = false;
	mExitToTop = false;
	mWidth = 640;
	mHeight = 480;
	mFullscreenBits = 16;
#if defined(__IPHONEOS__) || (defined(__ANDROID__) && !defined(__TERMUX__)) || defined(__SWITCH__) || defined(__3DS__)
	mIsWindowed = false;
#else
	mIsWindowed = true;
#endif
	mIsPhysWindowed = true;
	mFullScreenWindow = false;
	mPreferredX = -1;
	mPreferredY = -1;
	mIsScreenSaver = false;
	mAllowMonitorPowersave = true;
	//mHWnd = nullptr;
	mGLInterface = nullptr;	
	mMusicInterface = nullptr;
	//mInvisHWnd = nullptr;
	mFrameTime = 10;
	mNonDrawCount = 0;
	mDrawCount = 0;
	mSleepCount = 0;
	mUpdateCount = 0;	
	mUpdateAppState = 0;
	mUpdateAppDepth = 0;
	mPendingUpdatesAcc = 0.0;
	mUpdateFTimeAcc = 0.0;
	mHasPendingDraw = true;
	mIsDrawing = false;
	mLastDrawWasEmpty = false;	
	mLastTimeCheck = 0;
	mUpdateMultiplier = 1;
	mPaused = false;
	mFastForwardToUpdateNum = 0;
	mFastForwardToMarker = false;
	mFastForwardStep = false;
	mCustomCursor = nullptr;
	mCustomCursorImage = nullptr;
	mCustomCursorImageNum = -1;
	mSoundManager = nullptr;
	mCursorNum = CURSOR_POINTER;		
	mMouseIn = false;
	mRunning = false;
	mActive = true;
	mProcessInTimer = false;
	mMinimized = false;	
	mPhysMinimized = false;
	mIsDisabled = false;
	mLoaded = false;	
	mYieldMainThread = false; 
	mLoadingFailed = false;	
	mLoadingThreadStarted = false;
	mAutoStartLoadingThread = true;
	mLoadingThreadCompleted = false;
	mCursorThreadRunning = false;
	mNumLoadingThreadTasks = 0;
	mCompletedLoadingThreadTasks = 0;	
	mLastDrawTick = SDL_GetTicks();
	mNextDrawTick = SDL_GetTicks();
	mSysCursor = true;	
#if defined(__IPHONEOS__) || (defined(__ANDROID__) && !defined(__TERMUX__)) || defined(__SWITCH__) || defined(__3DS__)
	mForceFullscreen = true;
#else
	mForceFullscreen = false;
#endif
	mForceWindowed = false;
	mHasFocus = true;			
	mCustomCursorsEnabled = false;	
	mCustomCursorDirty = false;
	//mOverrideCursor = nullptr;
	mIsOpeningURL = false;		
	mInitialized = false;	
	mLastShutdownWasGraceful = true;	
	mReadFromRegistry = false;	
	mCmdLineParsed = false;
	mSkipSignatureChecks = false;	
	mCtrlDown = false;
	mAltDown = false;
	mAllowAltEnter = true;
	mStepMode = 0;
	mCleanupSharedImages.store(false, std::memory_order_relaxed);
	mStandardWordWrap = true;
	mbAllowExtendedChars = true;
	mEnableMaximizeButton = false;
	mWriteToSexyCache = true;
	mSexyCacheBuffers = false;

	mMusicVolume = 0.85;
	mSfxVolume = 0.85;
	mDemoMusicVolume = mDemoSfxVolume = 0.0;
	mMuteCount = 0;	
	mAutoMuteCount = 0;
	mDemoMute = false;
	mMuteOnLostFocus = false;
	mFPSTime = 0;
	mFPSStartTick = SDL_GetTicks();
	mFPSFlipCount = 0;
	mFPSCount = 0;
	mFPSDirtyCount = 0;
	mShowFPS = false;
	mShowFPSMode = FPS_ShowFPS;
	mDrawTime = 0;
	mScreenBltTime = 0;
	mAlphaDisabled = false;	
	mDebugKeysEnabled = false;
	mNoSoundNeeded = false;
	mWantFMod = false;

	mSyncRefreshRate = 100;
	mVSyncUpdates = false;
	mVSyncBroken = false;
	mVSyncBrokenCount = 0;
	mVSyncBrokenTestStartTick = 0;
	mVSyncBrokenTestUpdates = 0;
	mWaitForVSync = false;
	mSoftVSyncWait = true;
	mUserChanged3DSetting = false;
	mAutoEnable3D = false;
	mArgc = 0;
	mArgv = nullptr;
	mTest3D = false;
	mMinVidMemory3D = 6;
	mRecommendedVidMemory3D = 14;
	mRelaxUpdateBacklogCount = 0;
	mWidescreenAware = false;
	mEnableWindowAspect = false;
	mWindowAspect.Set(4, 3);
	mIsWideWindow = false;

	mWindow = 0;
	mContext = 0;
	mSurface = 0;

	int i;

	for (i = 0; i < NUM_CURSORS; i++)
	{
		mCursorImages[i] = nullptr;
		mSysCursors[i] = nullptr;
	}

	for (i = 0; i < 256; i++)
		mAdd8BitMaxTable[i] = i;

	for (i = 256; i < 512; i++)
		mAdd8BitMaxTable[i] = 255;
	
	// Set default strings.  Init could read in overrides from partner.xml
	SetString("DIALOG_BUTTON_OK",		"OK");
	SetString("DIALOG_BUTTON_CANCEL","CANCEL");

	SetString("UPDATE_CHECK_TITLE",		"Update Check");
	SetString("UPDATE_CHECK_BODY",		"Checking if there are any updates available for this product ...");

	SetString("UP_TO_DATE_TITLE",		"Up to Date");
	SetString("UP_TO_DATE_BODY",		"There are no updates available for this product at this time.");
	SetString("NEW_VERSION_TITLE",		"New Version");
	SetString("NEW_VERSION_BODY",		"There is an update available for this product.  Would you like to visit the web site to download it?");

	mDemoPrefix = "sexyapp";
	mDemoFileName = mDemoPrefix + ".dmo";
	mPlayingDemoBuffer = false;
	mManualShutdown = false;
	mRecordingDemoBuffer = false;
	mLastDemoMouseX = 0;
	mLastDemoMouseY = 0;
	mLastDemoUpdateCnt = 0;
	mDemoNeedsCommand = true;
	mDemoLoadingComplete = false;
	mDemoLength = 0;
	mDemoCmdNum = 0;
	mDemoCmdOrder = -1; // Means we haven't processed any demo commands yet
	mDemoCmdBitPos = 0;

	mWidgetManager = new WidgetManager(this);
	mResourceManager = new ResourceManager(this);

	mPrimaryThreadId = std::this_thread::get_id();

	mTabletPC = false;
}

SexyAppBase::~SexyAppBase()
{
	Shutdown();

	DialogMap::iterator aDialogItr = mDialogMap.begin();
	while (aDialogItr != mDialogMap.end())
	{
		mWidgetManager->RemoveWidget(aDialogItr->second);
		delete aDialogItr->second;
		++aDialogItr;
	}
	mDialogMap.clear();
	mDialogList.clear();
	
	delete mWidgetManager;	
	delete mResourceManager;
	delete gFPSImage;
	gFPSImage = nullptr;
	
	SharedImageMap::iterator aSharedImageItr = mSharedImageMap.begin();
	while (aSharedImageItr != mSharedImageMap.end())
	{
		SharedImage* aSharedImage = &aSharedImageItr->second;
		DBG_ASSERTE(aSharedImage->mRefCount == 0);		
		delete aSharedImage->mImage;
		mSharedImageMap.erase(aSharedImageItr++);		
	}
	
	delete mGLInterface;
	delete mMusicInterface;
	delete mSoundManager;			

	ResetCustomCursorCache();

	for (int aCursorIdx = 0; aCursorIdx < NUM_CURSORS; aCursorIdx++)
	{
		if (mSysCursors[aCursorIdx] != nullptr)
		{
			SDL_FreeCursor(mSysCursors[aCursorIdx]);
			mSysCursors[aCursorIdx] = nullptr;
		}
	}

	WaitForLoadingThread();	

	gSexyAppBase = nullptr;

	WriteDemoBuffer();
}

void SexyAppBase::ClearUpdateBacklog(bool relaxForASecond)
{
	mLastTimeCheck = SDL_GetTicks();
	mUpdateFTimeAcc = 0.0;

	if (relaxForASecond)
		mRelaxUpdateBacklogCount = 1000;
}

bool SexyAppBase::IsScreenSaver()
{
	return mIsScreenSaver;
}

bool SexyAppBase::AppCanRestore()
{
	return !mIsDisabled;
}

bool SexyAppBase::ReadDemoBuffer(std::string &theError)
{
	std::ifstream aFile(PathFromU8(mDemoFileName), std::ios::in | std::ios::binary);
	if (!aFile)
	{
		theError = "Demo file not found: " + mDemoFileName;
		return false;
	}

	uint32_t aFileID;
	if (!aFile.read(reinterpret_cast<char*>(&aFileID), sizeof(aFileID))) return false;

	DBG_ASSERTE(aFileID == DEMO_FILE_ID);
	if (aFileID != DEMO_FILE_ID)
	{
		theError = "Invalid demo file.";
		return false;
	}


	uint32_t aVersion;
	if (!aFile.read(reinterpret_cast<char*>(&aVersion), sizeof(aVersion))) return false;
	
	if (!aFile.read(reinterpret_cast<char*>(&mRandSeed), sizeof(mRandSeed))) return false;
	SRand(mRandSeed);

	ushort aStrLen = 4;
	if (!aFile.read(reinterpret_cast<char*>(&aStrLen), sizeof(aStrLen))) return false;
	aStrLen = std::min<ushort>(aStrLen, 255);
	char aStr[256];
	if (!aFile.read(aStr, aStrLen)) return false;
	aStr[aStrLen] = '\0';

	DBG_ASSERTE(mProductVersion == aStr);
	if (mProductVersion != aStr)
	{
		theError = "This demo file appears to be for '" + std::string(aStr) + "'";
		return false;
	}

	std::streampos aFilePos = aFile.tellg();
	aFile.seekg(0, std::ios::end);
	std::streampos aFileEnd = aFile.tellg();
	aFile.seekg(aFilePos, std::ios::beg);
	int aBytesLeft = static_cast<int>(aFileEnd - aFilePos);

	uchar* aBuffer;
	// read marker list
	if (aVersion >= 2) 
	{
		int aSize;
		if (!aFile.read(reinterpret_cast<char*>(&aSize), sizeof(aSize))) return false;
		aBytesLeft -= 4;

		if (aSize >= aBytesLeft)
		{
			theError = "Invalid demo file.";
			return false;
		}

		Buffer aMarkerBuffer;

		aBuffer = new uchar[aSize];
		if (!aFile.read(reinterpret_cast<char*>(aBuffer), aSize)) { delete [] aBuffer; return false; }
		aMarkerBuffer.WriteBytes(aBuffer, aSize);
		aMarkerBuffer.SeekFront();

		int aNumItems = aMarkerBuffer.ReadLong();
		int i;
		for (i=0; i<aNumItems && !aMarkerBuffer.AtEnd(); i++)
		{
			mDemoMarkerList.push_back(DemoMarker());
			DemoMarker &aMarker = mDemoMarkerList.back();
			aMarker.first = aMarkerBuffer.ReadString();
			aMarker.second = aMarkerBuffer.ReadLong();
		}

		if (i!=aNumItems)
		{
			theError = "Invalid demo file.";
			return false;
		}

		aBytesLeft -= aSize;

		delete [] aBuffer;			
	}

	// Read demo commands
	if (!aFile.read(reinterpret_cast<char*>(&mDemoLength), sizeof(mDemoLength))) return false;
	aBytesLeft -= 4;
	
	if (aBytesLeft <= 0)
	{
		theError = "Invalid demo file.";
		return false;
	}


	aBuffer = new uchar[aBytesLeft];
	if (!aFile.read(reinterpret_cast<char*>(aBuffer), aBytesLeft)) { delete [] aBuffer; return false; }		

	mDemoBuffer.WriteBytes(aBuffer, aBytesLeft);
	mDemoBuffer.SeekFront();

	delete [] aBuffer;
	return true;
}

void SexyAppBase::WriteDemoBuffer()
{
	if (mRecordingDemoBuffer)
	{
		std::ofstream aFile(PathFromU8(mDemoFileName), std::ios::out | std::ios::binary | std::ios::trunc);
		if (aFile)
		{
			uint32_t aFileID = DEMO_FILE_ID;
			aFile.write(reinterpret_cast<const char*>(&aFileID), sizeof(aFileID));		

			uint32_t aVersion = DEMO_VERSION;
			aFile.write(reinterpret_cast<const char*>(&aVersion), sizeof(aVersion));
			
			aFile.write(reinterpret_cast<const char*>(&mRandSeed), sizeof(mRandSeed));

			ushort aStrLen = mProductVersion.length();
			aFile.write(reinterpret_cast<const char*>(&aStrLen), sizeof(aStrLen));		
			aFile.write(mProductVersion.c_str(), static_cast<std::streamsize>(mProductVersion.length()));

			Buffer aMarkerBuffer;
			aMarkerBuffer.WriteLong(mDemoMarkerList.size());
			for (DemoMarkerList::iterator aMarkerItr = mDemoMarkerList.begin(); aMarkerItr != mDemoMarkerList.end(); ++aMarkerItr)
			{
				aMarkerBuffer.WriteString(aMarkerItr->first);
				aMarkerBuffer.WriteLong(aMarkerItr->second);
			}
			int aMarkerBufferSize = aMarkerBuffer.GetDataLen();
			aFile.write(reinterpret_cast<const char*>(&aMarkerBufferSize), sizeof(aMarkerBufferSize));
			aFile.write(reinterpret_cast<const char*>(aMarkerBuffer.GetDataPtr()), aMarkerBufferSize);

			uint32_t aDemoLength = mUpdateCount;
			aFile.write(reinterpret_cast<const char*>(&aDemoLength), sizeof(aDemoLength));

			aFile.write(reinterpret_cast<const char*>(mDemoBuffer.GetDataPtr()), mDemoBuffer.GetDataLen());
		}		
	}
}

void SexyAppBase::DemoSyncBuffer(Buffer* theBuffer)
{
	if (mPlayingDemoBuffer)
	{
		if (mManualShutdown)
			return;

		PrepareDemoCommand(true);
		mDemoNeedsCommand = true;
		
		DBG_ASSERTE(!mDemoIsShortCmd);
		DBG_ASSERTE(mDemoCmdNum == DEMO_SYNC);

		uint32_t aLen = mDemoBuffer.ReadLong();
		
		theBuffer->Clear();
		for (int i = 0; i < static_cast<int>(aLen); i++)
			theBuffer->WriteByte(mDemoBuffer.ReadByte());		
	}
	else if (mRecordingDemoBuffer)
	{
		WriteDemoTimingBlock();
		mDemoBuffer.WriteNumBits(0, 1);
		mDemoBuffer.WriteNumBits(DEMO_SYNC, 5);		
		mDemoBuffer.WriteLong(theBuffer->GetDataLen());
		mDemoBuffer.WriteBytes(theBuffer->GetDataPtr(), theBuffer->GetDataLen());
	}
}

void SexyAppBase::DemoSyncString(std::string* theString)
{
	Buffer aBuffer;
	aBuffer.WriteString(*theString);
	DemoSyncBuffer(&aBuffer);
	*theString = aBuffer.ReadString();
}

void SexyAppBase::DemoSyncInt(int* theInt)
{
	Buffer aBuffer;
	aBuffer.WriteLong(*theInt);
	DemoSyncBuffer(&aBuffer);
	*theInt = aBuffer.ReadLong();
}

void SexyAppBase::DemoSyncBool(bool* theBool)
{
	Buffer aBuffer;
	aBuffer.WriteBoolean(*theBool);
	DemoSyncBuffer(&aBuffer);
	*theBool = aBuffer.ReadBoolean();
}

void SexyAppBase::DemoAssertStringEqual(const std::string& theString)
{
	if (mPlayingDemoBuffer)
	{
		if (mManualShutdown)
			return;

		PrepareDemoCommand(true);
		mDemoNeedsCommand = true;
		
		DBG_ASSERTE(!mDemoIsShortCmd);
		DBG_ASSERTE(mDemoCmdNum == DEMO_ASSERT_STRING_EQUAL);

		std::string aString = mDemoBuffer.ReadString();
		DBG_ASSERTE(aString == theString);
	}
	else if (mRecordingDemoBuffer)
	{
		WriteDemoTimingBlock();
		mDemoBuffer.WriteNumBits(0, 1);
		mDemoBuffer.WriteNumBits(DEMO_ASSERT_STRING_EQUAL, 5);				
		mDemoBuffer.WriteString(theString);
	}
}

void SexyAppBase::DemoAddMarker(const std::string& theString)
{
	if (mPlayingDemoBuffer)
	{
		mFastForwardToMarker = false;
	}
	else if (mRecordingDemoBuffer)
	{
		mDemoMarkerList.push_back(DemoMarker(theString,mUpdateCount));
	}
}

void SexyAppBase::DemoAssertIntEqual(int theInt)
{
	if (mPlayingDemoBuffer)
	{
		if (mManualShutdown)
			return;

		PrepareDemoCommand(true);
		mDemoNeedsCommand = true;
		
		DBG_ASSERTE(!mDemoIsShortCmd);
		DBG_ASSERTE(mDemoCmdNum == DEMO_ASSERT_INT_EQUAL);

		int anInt = mDemoBuffer.ReadLong();
		(void)anInt; // unused in Release mode
		DBG_ASSERTE(anInt == theInt);
	}
	else if (mRecordingDemoBuffer)
	{		
		WriteDemoTimingBlock();
		mDemoBuffer.WriteNumBits(0, 1);
		mDemoBuffer.WriteNumBits(DEMO_ASSERT_INT_EQUAL, 5);				
		mDemoBuffer.WriteLong(theInt);
	}
}

Dialog* SexyAppBase::NewDialog(int theDialogId, bool isModal, const std::string& theDialogHeader, const std::string& theDialogLines, const std::string& theDialogFooter, int theButtonMode)
{	
	Dialog* aDialog = new Dialog(nullptr, nullptr, theDialogId, isModal, theDialogHeader,	theDialogLines, theDialogFooter, theButtonMode);		
	return aDialog;
}

Dialog* SexyAppBase::DoDialog(int theDialogId, bool isModal, const std::string& theDialogHeader, const std::string& theDialogLines, const std::string& theDialogFooter, int theButtonMode)
{
	KillDialog(theDialogId);

	Dialog* aDialog = NewDialog(theDialogId, isModal, theDialogHeader, theDialogLines, theDialogFooter, theButtonMode);		

	AddDialog(theDialogId, aDialog);

	return aDialog;
}


Dialog*	SexyAppBase::GetDialog(int theDialogId)
{
	DialogMap::iterator anItr = mDialogMap.find(theDialogId);

	if (anItr != mDialogMap.end())	
		return anItr->second;

	return nullptr;
}

bool SexyAppBase::KillDialog(int theDialogId, bool removeWidget, bool deleteWidget)
{
	DialogMap::iterator anItr = mDialogMap.find(theDialogId);

	if (anItr != mDialogMap.end())
	{
		Dialog* aDialog = anItr->second;

		// set the result to something else so DoMainLoop knows that the dialog is gone 
		// in case nobody else sets mResult		
		if (aDialog->mResult == 0x7FFFFFFF)
			aDialog->mResult = 0;
		
		DialogList::iterator aListItr = std::find(mDialogList.begin(),mDialogList.end(),aDialog);
		if (aListItr != mDialogList.end())
			mDialogList.erase(aListItr);
		
		mDialogMap.erase(anItr);

		if (removeWidget || deleteWidget)
		{
			mWidgetManager->RemoveWidget(aDialog);
		}

		if (aDialog->IsModal())
		{			
			ModalClose();
			mWidgetManager->RemoveBaseModal(aDialog);
		}				

		if (deleteWidget)
		{
			SafeDeleteWidget(aDialog);
		}
		
		return true;
	}

	return false;
}

bool SexyAppBase::KillDialog(int theDialogId)
{
	return KillDialog(theDialogId,true,true);
}

bool SexyAppBase::KillDialog(Dialog* theDialog)
{
	return KillDialog(theDialog->mId);
}

int SexyAppBase::GetDialogCount()
{
	return mDialogMap.size();
}

void SexyAppBase::AddDialog(int theDialogId, Dialog* theDialog)
{
	KillDialog(theDialogId);

	if (theDialog->mWidth == 0)
	{
		// Set the dialog position ourselves
		int aWidth = mWidth/2;
		theDialog->Resize((mWidth - aWidth)/2, mHeight / 5, aWidth, theDialog->GetPreferredHeight(aWidth));
	}

	mDialogMap.insert(DialogMap::value_type(theDialogId, theDialog));
	mDialogList.push_back(theDialog);

	mWidgetManager->AddWidget(theDialog);
	if (theDialog->IsModal())
	{
		mWidgetManager->AddBaseModal(theDialog);
		ModalOpen();
	}
}

void SexyAppBase::AddDialog(Dialog* theDialog)
{
	AddDialog(theDialog->mId, theDialog);
}

void SexyAppBase::ModalOpen()
{
}

void SexyAppBase::ModalClose()
{
}

void SexyAppBase::DialogButtonPress(int theDialogId, int theButtonId)
{	
	if (theButtonId == Dialog::ID_YES)
		ButtonPress(2000 + theDialogId);
	else if (theButtonId == Dialog::ID_NO)
		ButtonPress(3000 + theDialogId);	
}

void SexyAppBase::DialogButtonDepress(int theDialogId, int theButtonId)
{
	if (theButtonId == Dialog::ID_YES)
		ButtonDepress(2000 + theDialogId);
	else if (theButtonId == Dialog::ID_NO)
		ButtonDepress(3000 + theDialogId);
}

void SexyAppBase::GotFocus()
{	
}

void SexyAppBase::LostFocus()
{	
}

void SexyAppBase::URLOpenFailed(const std::string& theURL)
{
	(void)theURL;
	mIsOpeningURL = false;
}

void SexyAppBase::URLOpenSucceeded(const std::string& theURL)
{	
	(void)theURL;
	mIsOpeningURL = false;

	if (mShutdownOnURLOpen)
		Shutdown();	
}

bool SexyAppBase::OpenURL(const std::string& theURL, bool shutdownOnOpen)
{
	if ((!mIsOpeningURL) || (theURL != mOpeningURL))
	{
		mShutdownOnURLOpen = shutdownOnOpen;
		mIsOpeningURL = true;
		mOpeningURL = theURL;
		mOpeningURLTime = SDL_GetTicks();
	}

	return true;
}

std::string SexyAppBase::GetProductVersion(const std::string& thePath)
{
	return "0";
}

void SexyAppBase::WaitForLoadingThread()
{
#ifdef __EMSCRIPTEN__
	return;
#endif
	int ms = 20;

	timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	while ((mLoadingThreadStarted) && (!mLoadingThreadCompleted))
		nanosleep(&ts, &ts);
}

void SexyAppBase::SetCursorImage(int theCursorNum, Image* theImage)
{
	if ((theCursorNum >= 0) && (theCursorNum < NUM_CURSORS))
	{
		if (mCustomCursorImageNum == theCursorNum && mCursorImages[theCursorNum] != theImage)
			ResetCustomCursorCache();

		mCursorImages[theCursorNum] = theImage;
		EnforceCursor();
	}
}


double SexyAppBase::GetLoadingThreadProgress()
{
	if (mLoaded)
		return 1.0;
	if (!mLoadingThreadStarted)
		return 0.0;
	if (mNumLoadingThreadTasks == 0)
		return 0.0;
	return std::min(mCompletedLoadingThreadTasks / static_cast<double>(mNumLoadingThreadTasks), 1.0);
}

bool SexyAppBase::RegistryWrite(const std::string& theValueName, uint32_t theType, const uchar* theValue, uint32_t theLength)
{
	if (mRegKey.length() == 0)
		return false;

	if (mPlayingDemoBuffer)
	{
		if (mManualShutdown)
			return true;

		PrepareDemoCommand(true);
		mDemoNeedsCommand = true;

		DBG_ASSERTE(!mDemoIsShortCmd);		
		DBG_ASSERTE(mDemoCmdNum == DEMO_REGISTRY_WRITE);

		return mDemoBuffer.ReadNumBits(1, false) != 0;		
	}

	//HKEY aGameKey;
	
	std::string aKeyName = RemoveTrailingSlash("SOFTWARE\\" + mRegKey);
	std::string aValueName;

	int aSlashPos = static_cast<int>(theValueName.rfind('\\'));
	if (aSlashPos != -1)
	{
		aKeyName += "\\" + theValueName.substr(0, aSlashPos);
		aValueName = theValueName.substr(aSlashPos + 1);
	}
	else
	{
		aValueName = theValueName;
	}

	if (!regemu::RegistryWrite(aKeyName, aValueName, theType, theValue, theLength))
	{
		if (mRecordingDemoBuffer)
		{
			WriteDemoTimingBlock();
			mDemoBuffer.WriteNumBits(0, 1);
			mDemoBuffer.WriteNumBits(DEMO_REGISTRY_WRITE, 5);
			mDemoBuffer.WriteNumBits(0, 1); // failure
		}

		return false;
	}

	if (mRecordingDemoBuffer)
	{
		WriteDemoTimingBlock();
		mDemoBuffer.WriteNumBits(0, 1);
		mDemoBuffer.WriteNumBits(DEMO_REGISTRY_WRITE, 5);
		mDemoBuffer.WriteNumBits(1, 1); // success
	}

	return true;
}

bool SexyAppBase::RegistryWriteString(const std::string& theValueName, const std::string& theString)
{
	return RegistryWrite(theValueName, regemu::REGEMU_SZ, reinterpret_cast<const uchar*>(theString.c_str()), theString.length());
}

bool SexyAppBase::RegistryWriteInteger(const std::string& theValueName, int theValue)
{
	return RegistryWrite(theValueName, regemu::REGEMU_DWORD, reinterpret_cast<const uchar*>(&theValue), sizeof(int));
}

bool SexyAppBase::RegistryWriteBoolean(const std::string& theValueName, bool theValue)
{
	int aValue = theValue ? 1 : 0;
	return RegistryWrite(theValueName, regemu::REGEMU_DWORD, reinterpret_cast<const uchar*>(&aValue), sizeof(int));
}

bool SexyAppBase::RegistryWriteData(const std::string& theValueName, const uchar* theValue, uint32_t theLength)
{
	return RegistryWrite(theValueName, regemu::REGEMU_BINARY, theValue, theLength);
}

void SexyAppBase::WriteToRegistry()
{	
	RegistryWriteInteger("MusicVolume", static_cast<int>(mMusicVolume * 100));
	RegistryWriteInteger("SfxVolume", static_cast<int>(mSfxVolume * 100));
	RegistryWriteInteger("Muted", (mMuteCount - mAutoMuteCount > 0) ? 1 : 0);
	RegistryWriteInteger("ScreenMode", mIsWindowed ? 0 : 1);
	RegistryWriteInteger("PreferredX", mPreferredX);
	RegistryWriteInteger("PreferredY", mPreferredY);
	RegistryWriteInteger("CustomCursors", mCustomCursorsEnabled ? 1 : 0);		
	RegistryWriteInteger("InProgress", 0);
	RegistryWriteBoolean("WaitForVSync", mWaitForVSync);	
}

bool SexyAppBase::RegistryEraseKey(const std::string& _theKeyName)
{
	std::string theKeyName = _theKeyName;
	if (mRegKey.length() == 0)
		return false;

	if (mPlayingDemoBuffer)
	{
		if (mManualShutdown)
			return true;

		PrepareDemoCommand(true);
		mDemoNeedsCommand = true;

		DBG_ASSERTE(!mDemoIsShortCmd);		
		DBG_ASSERTE(mDemoCmdNum == DEMO_REGISTRY_ERASE);

		return mDemoBuffer.ReadNumBits(1, false) != 0;		
	}	
	
	std::string aKeyName = RemoveTrailingSlash("SOFTWARE\\" + mRegKey) + "\\" + theKeyName;

	if (!regemu::RegistryEraseKey(aKeyName))
	{
		if (mRecordingDemoBuffer)
		{
			WriteDemoTimingBlock();
			mDemoBuffer.WriteNumBits(0, 1);
			mDemoBuffer.WriteNumBits(DEMO_REGISTRY_ERASE, 5);
			mDemoBuffer.WriteNumBits(0, 1); // failure
		}

		return false;
	}

	if (mRecordingDemoBuffer)
	{
		WriteDemoTimingBlock();
		mDemoBuffer.WriteNumBits(0, 1);
		mDemoBuffer.WriteNumBits(DEMO_REGISTRY_ERASE, 5);
		mDemoBuffer.WriteNumBits(1, 1); // success
	}

	return true;
}

void SexyAppBase::RegistryEraseValue(const std::string& _theValueName)
{
	std::string theValueName = _theValueName;
	if (mRegKey.length() == 0)
		return;

	//HKEY aGameKey;
	std::string aKeyName = RemoveTrailingSlash("SOFTWARE\\" + mRegKey);
	std::string aValueName;

	int aSlashPos = static_cast<int>(theValueName.rfind('\\'));
	if (aSlashPos != -1)
	{
		aKeyName += "\\" + theValueName.substr(0, aSlashPos);
		aValueName = theValueName.substr(aSlashPos + 1);
	}
	else
	{
		aValueName = theValueName;
	}

	regemu::RegistryEraseValue(aKeyName, aValueName);
}

bool SexyAppBase::RegistryRead(const std::string& theValueName, uint32_t* theType, uchar* theValue, uint32_t* theLength)
{
	return RegistryReadKey(theValueName, theType, theValue, theLength);
}

bool SexyAppBase::RegistryReadKey(const std::string& theValueName, uint32_t* theType, uchar* theValue, uint32_t* theLength)
{
	if (mRegKey.length() == 0)
		return false;

	if (mPlayingDemoBuffer)
	{
		if (mManualShutdown)
			return false;

		PrepareDemoCommand(true);
		mDemoNeedsCommand = true;
				
		DBG_ASSERTE(!mDemoIsShortCmd);		
		DBG_ASSERTE(mDemoCmdNum == DEMO_REGISTRY_READ);

		bool success = mDemoBuffer.ReadNumBits(1, false) != 0;
		if (!success)
			return false;

		*theType = mDemoBuffer.ReadLong();

		uint32_t aLen = mDemoBuffer.ReadLong();
		*theLength = aLen;
		
		if (*theLength >= aLen)
		{			
			mDemoBuffer.ReadBytes(theValue, aLen);
			return true;
		}
		else
		{
			for (int i = 0; i < static_cast<int>(aLen); i++)
				mDemoBuffer.ReadByte();
			return false;
		}		
	}
	else
	{		
		std::string aKeyName = RemoveTrailingSlash("SOFTWARE\\" + mRegKey);
		std::string aValueName;

		int aSlashPos = static_cast<int>(theValueName.rfind('\\'));
		if (aSlashPos != -1)
		{
			aKeyName += "\\" + theValueName.substr(0, aSlashPos);
			aValueName = theValueName.substr(aSlashPos + 1);
		}
		else
		{
			aValueName = theValueName;
		}		

		if (regemu::RegistryRead(aKeyName, aValueName, (uint32_t*)theType, theValue, (uint32_t*)theLength))
		{
			if (mRecordingDemoBuffer)
			{
				WriteDemoTimingBlock();
				mDemoBuffer.WriteNumBits(0, 1);
				mDemoBuffer.WriteNumBits(DEMO_REGISTRY_READ, 5);
				mDemoBuffer.WriteNumBits(1, 1); // success
				mDemoBuffer.WriteLong(*theType);
				mDemoBuffer.WriteLong(*theLength);
				mDemoBuffer.WriteBytes(theValue, *theLength);
			}

			return true;
		}

		if (mRecordingDemoBuffer)
		{
			WriteDemoTimingBlock();
			mDemoBuffer.WriteNumBits(0, 1);
			mDemoBuffer.WriteNumBits(DEMO_REGISTRY_READ, 5);
			mDemoBuffer.WriteNumBits(0, 1); // failure
		}
		
		return false;
	}
}

// aStr isn't initialised lmao
bool SexyAppBase::RegistryReadString(const std::string& theKey, std::string* theString)
{
	char aStr[1024];
	
	uint32_t aType;	
	uint32_t aLen = sizeof(aStr) - 1;
	if (!RegistryRead(theKey, &aType, (uchar*) aStr, &aLen))
		return false;

	if (aType != regemu::REGEMU_SZ)
		return false;

	aStr[aLen] = 0;

	*theString = aStr;
	return true;
}

bool SexyAppBase::RegistryReadInteger(const std::string& theKey, int* theValue)
{
	uint32_t aType;
	uint32_t aLong;
	uint32_t aLen = 4;
	if (!RegistryRead(theKey, &aType, (uchar*) &aLong, &aLen))
		return false;

	if (aType != regemu::REGEMU_DWORD)
		return false;

	*theValue = aLong;
	return true;
}

bool SexyAppBase::RegistryReadBoolean(const std::string& theKey, bool* theValue)
{
	int aValue;
	if (!RegistryReadInteger(theKey, &aValue))
		return false;
	
	*theValue = aValue != 0;
	return true;
}

bool SexyAppBase::RegistryReadData(const std::string& theKey, uchar* theValue, uint32_t* theLength)
{		
	uint32_t aType;
	if (!RegistryRead(theKey, &aType, (uchar*) theValue, theLength))
		return false;

	if (aType != regemu::REGEMU_BINARY)
		return false;
	
	return true;
}

void SexyAppBase::ReadFromRegistry()
{
	mReadFromRegistry = true;
	mRegKey = GetString("RegistryKey", mRegKey);

	if (mRegKey.length() == 0)
		return;

	regemu::SetRegFile(GetAppDataPath("registry.regemu"));

	int anInt;
	if (RegistryReadInteger("MusicVolume", &anInt))
		mMusicVolume = anInt / 100.0;
	
	if (RegistryReadInteger("SfxVolume", &anInt))
		mSfxVolume = anInt / 100.0;

	if (RegistryReadInteger("Muted", &anInt))
		mMuteCount = anInt;

#if !defined(__IPHONEOS__) && (!defined(__ANDROID__) || defined(__TERMUX__)) && !defined(__SWITCH__) && !defined(__3DS__) && !defined(__EMSCRIPTEN__)
	if (RegistryReadInteger("ScreenMode", &anInt))
		mIsWindowed = anInt == 0;
#endif

	RegistryReadInteger("PreferredX", &mPreferredX);
	RegistryReadInteger("PreferredY", &mPreferredY);	

	if (RegistryReadInteger("CustomCursors", &anInt))
		EnableCustomCursors(anInt != 0);	
			
	RegistryReadBoolean("WaitForVSync", &mWaitForVSync);	

	if (RegistryReadInteger("InProgress", &anInt))
		mLastShutdownWasGraceful = anInt == 0;
		
	if (!IsScreenSaver())
		RegistryWriteInteger("InProgress", 1);	
}

bool SexyAppBase::WriteBytesToFile(const std::string& theFileName, const void *theData, unsigned long theDataLen)
{
	if (mPlayingDemoBuffer)
	{
		if (mManualShutdown)
			return true;

		PrepareDemoCommand(true);
		mDemoNeedsCommand = true;

		DBG_ASSERTE(!mDemoIsShortCmd);		
		DBG_ASSERTE(mDemoCmdNum == DEMO_FILE_WRITE);		

		bool success = mDemoBuffer.ReadNumBits(1, false) != 0;
		if (!success)
			return false;

		return true;
	}	

	MkDir(GetFileDir(theFileName));
	std::ofstream aFile(PathFromU8(theFileName), std::ios::out | std::ios::binary | std::ios::trunc);
	if (!aFile)
	{
		if (mRecordingDemoBuffer)
		{
			WriteDemoTimingBlock();
			mDemoBuffer.WriteNumBits(0, 1);
			mDemoBuffer.WriteNumBits(DEMO_FILE_WRITE, 5);
			mDemoBuffer.WriteNumBits(0, 1); // failure				
		}

		return false;
	}

	aFile.write(reinterpret_cast<const char*>(theData), static_cast<std::streamsize>(theDataLen));
	if (!aFile)
		return false;

	if (mRecordingDemoBuffer)
	{
		WriteDemoTimingBlock();
		mDemoBuffer.WriteNumBits(0, 1);
		mDemoBuffer.WriteNumBits(DEMO_FILE_WRITE, 5);
		mDemoBuffer.WriteNumBits(1, 1); // success
	}

	return true;
}

bool SexyAppBase::WriteBufferToFile(const std::string& theFileName, const Buffer* theBuffer)
{
	return WriteBytesToFile(theFileName,theBuffer->GetDataPtr(),theBuffer->GetDataLen());
}


bool SexyAppBase::ReadBufferFromFile(const std::string& theFileName, Buffer* theBuffer, bool dontWriteToDemo)
{
	if ((mPlayingDemoBuffer) && (!dontWriteToDemo))
	{
		if (mManualShutdown)
			return false;

		PrepareDemoCommand(true);
		mDemoNeedsCommand = true;
		
		DBG_ASSERTE(!mDemoIsShortCmd);		
		DBG_ASSERTE(mDemoCmdNum == DEMO_FILE_READ);

		bool success = mDemoBuffer.ReadNumBits(1, false) != 0;
		if (!success)
			return false;

		uint32_t aLen = mDemoBuffer.ReadLong();		
				
		theBuffer->Clear();
		for (int i = 0; i < static_cast<int>(aLen); i++)
			theBuffer->WriteByte(mDemoBuffer.ReadByte());

		return true;		
	}
	else
	{
		PFILE* aFP = p_fopen(theFileName.c_str(), "rb");

		if (aFP == nullptr)
		{
			if ((mRecordingDemoBuffer) && (!dontWriteToDemo))
			{
				WriteDemoTimingBlock();
				mDemoBuffer.WriteNumBits(0, 1);
				mDemoBuffer.WriteNumBits(DEMO_FILE_READ, 5);
				mDemoBuffer.WriteNumBits(0, 1); // failure				
			}

			return false;
		}
		
		p_fseek(aFP, 0, SEEK_END);
		int aFileSize = p_ftell(aFP);
		p_fseek(aFP, 0, SEEK_SET);
		
		uchar* aData = new uchar[aFileSize];

		p_fread(aData, 1, aFileSize, aFP);
		p_fclose(aFP);

		theBuffer->Clear();
		theBuffer->SetData(aData, aFileSize);

		if ((mRecordingDemoBuffer) && (!dontWriteToDemo))
		{
			WriteDemoTimingBlock();
			mDemoBuffer.WriteNumBits(0, 1);
			mDemoBuffer.WriteNumBits(DEMO_FILE_READ, 5);
			mDemoBuffer.WriteNumBits(1, 1); // success			
			mDemoBuffer.WriteLong(aFileSize);
			mDemoBuffer.WriteBytes(aData, aFileSize);
		}

		delete [] aData;

		return true;
	}
}

bool SexyAppBase::ReadUTF8StringFromFile(const std::string& theFileName, std::string* theString)
{
	Buffer aBuffer;
	if (!ReadBufferFromFile(theFileName, &aBuffer))
		return false;

	return aBuffer.ToUTF8String(theString);
}

bool SexyAppBase::FileExists(const std::string& theFileName)
{
	if (mPlayingDemoBuffer)
	{
		if (mManualShutdown)
			return true;

		PrepareDemoCommand(true);
		mDemoNeedsCommand = true;
		
		DBG_ASSERTE(!mDemoIsShortCmd);		
		DBG_ASSERTE(mDemoCmdNum == DEMO_FILE_EXISTS);		

		bool success = mDemoBuffer.ReadNumBits(1, false) != 0;		
		return success;
	}
	else
	{
		PFILE* aFP = p_fopen(theFileName.c_str(), "rb");

		if (mRecordingDemoBuffer)
		{
			WriteDemoTimingBlock();
			mDemoBuffer.WriteNumBits(0, 1);
			mDemoBuffer.WriteNumBits(DEMO_FILE_EXISTS, 5);
			mDemoBuffer.WriteNumBits((aFP != nullptr) ? 1 : 0, 1); 
		}

		if (aFP == nullptr)		
			return false;		
		
		p_fclose(aFP);
		return true;
	}
}

bool SexyAppBase::EraseFile(const std::string& theFileName)
{
	if (mPlayingDemoBuffer)
		return true;

	return remove(theFileName.c_str()) == 0;
}

void SexyAppBase::SEHOccured()
{
	SetMusicVolume(0);
	//::ShowWindow(mHWnd, SW_HIDE);
	mSEHOccured = true;
	EnforceCursor();
}

std::string SexyAppBase::GetGameSEHInfo()
{
	int aSecLoaded = (SDL_GetTicks() - mTimeLoaded) / 1000;

	char aTimeStr[16];
	snprintf(aTimeStr, sizeof(aTimeStr), "%02d:%02d:%02d", (aSecLoaded/60/60), (aSecLoaded/60)%60, aSecLoaded%60);

	std::string anInfoString = 
		"Product: " + mProdName + "\r\n" +		
		"Version: " + mProductVersion + "\r\n";			

	anInfoString +=
		"Time Loaded: " + std::string(aTimeStr) + "\r\n"
		"Fullscreen: " + (mIsWindowed ? std::string("No") : std::string("Yes")) + "\r\n";	

	return anInfoString;						
}

void SexyAppBase::GetSEHWebParams(DefinesMap*){}

void SexyAppBase::ShutdownHook()
{
}

void SexyAppBase::Shutdown()
{
	if (std::this_thread::get_id() != mPrimaryThreadId)
	{
		mLoadingFailed = true;
	}
	else if (!mShutdown)
	{
		mExitToTop = true;
		mShutdown = true;
		ShutdownHook();

		if (mPlayingDemoBuffer)
		{
			//if the music/sfx volume is 0, then it means that in playback
			//someone pressed the "S" key to mute sounds (or that the 
			//sound volume was set to 0 in the first place). Out of politeness,
			//return the system sound volume to what it last was in the game.
			SetMusicVolume(mDemoMusicVolume);
			SetSfxVolume(mDemoSfxVolume);
		}

		// Blah
		
		if (mMusicInterface != nullptr)
			mMusicInterface->StopAllMusic();		
		
		RestoreScreenResolution();

		if (mReadFromRegistry)
			WriteToRegistry();

#ifdef __EMSCRIPTEN__
		// Exit browser fullscreen so the shell page returns to normal layout.
		emscripten_exit_fullscreen();
		EM_ASM(
			if (typeof FS !== 'undefined' && FS.syncfs) {
				FS.syncfs(false, function(err) {
					if (err) console.warn('IDBFS sync error on shutdown:', err);
				});
			}
		);
#endif
	}
}

void SexyAppBase::RestoreScreenResolution()
{
	// Screen resolution restoration not needed
}
	
void SexyAppBase::DoExit(int theCode)
{
	RestoreScreenResolution();

#if defined(__EMSCRIPTEN__)
	if (mRunning)
	{
		emscripten_async_call(EmscriptenDeferredDoExit, this, 0);
	}
	else
	{
		Shutdown();
		EM_ASM(
			if (typeof window.onGameExit === 'function') window.onGameExit();
		);
	}
	(void)theCode;
#elif (defined(__ANDROID__) && !defined(__TERMUX__)) || defined(__IPHONEOS__)
	Shutdown();
#else
	exit(theCode);
#endif
}

void SexyAppBase::UpdateFrames()
{
	mUpdateCount++;	

	if (!mMinimized)
	{		
		if (mWidgetManager->UpdateFrame())
			++mFPSDirtyCount;
	}

	mMusicInterface->Update();	
	CleanSharedImages();
}

void SexyAppBase::DoUpdateFramesF(float theFrac)
{
	if ((mVSyncUpdates) && (!mMinimized))
		mWidgetManager->UpdateFrameF(theFrac);	
}

bool SexyAppBase::DoUpdateFrames()
{
	SEXY_AUTO_PERF("SexyAppBase::DoUpdateFrames");

	if (gScreenSaverActive)
		return false;

	if (mPlayingDemoBuffer)
	{
		if ((mLoadingThreadCompleted) && (!mLoaded) && (mDemoLoadingComplete))
		{			
			mLoaded = true;
			//::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_NORMAL);
			mYieldMainThread = false;
			LoadingThreadCompleted();
		}

		// Hrrm not sure why we check (mUpdateCount != mLastDemoUpdateCnt) here
		if ((mLoaded == mDemoLoadingComplete) && (mUpdateCount != mLastDemoUpdateCnt))		
		{
			UpdateFrames();		
			return true;
		}

		return false;
	}
	else
	{
		if ((mLoadingThreadCompleted) && (!mLoaded))
		{
			//::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_NORMAL);
			mLoaded = true;
			mYieldMainThread = false;
			LoadingThreadCompleted();

			if (mRecordingDemoBuffer)
			{
				WriteDemoTimingBlock();
				mDemoBuffer.WriteNumBits(0, 1);
				mDemoBuffer.WriteNumBits(DEMO_LOADING_COMPLETE, 5);	
			}
		}
				
		UpdateFrames();		
		return true;
	}
}

bool gIsFailing = false;

void SexyAppBase::Redraw(Rect* theClipRect)
{
	SEXY_AUTO_PERF("SexyAppBase::Redraw");

	// Do mIsDrawing check because we could enter here at a bad time if any windows messages
	//  are processed during WidgetManager->Draw
	if ((mIsDrawing) || (mShutdown))
		return;

	if (gScreenSaverActive)
		return;

	mGLInterface->Redraw(theClipRect);

	mFPSFlipCount++;
}

///////////////////////////// FPS Stuff
static PerfTimer gFPSTimer;
static int gFrameCount;
static int gFPSDisplay;
static bool gForceDisplay = false;
static void CalculateFPS()
{
	gFrameCount++;

	if (gFPSTimer.GetDuration() >= 1000 || gForceDisplay)
	{
		gFPSTimer.Stop();
		if (!gForceDisplay)
			gFPSDisplay = static_cast<int>(gFrameCount * 1000 / gFPSTimer.GetDuration() + 0.5f);
		else
		{
			gForceDisplay = false;
			gFPSDisplay = 0;
		}

		gFPSTimer.Start();
		gFrameCount = 0;
	}
}

///////////////////////////// FPS Stuff to draw mouse coords
static void FPSDrawCoords(int theX, int theY)
{
	(void)theX;
	(void)theY;
	// FPS coordinate drawing not implemented
}

///////////////////////////// Demo TimeLeft Stuff
static GLImage* gDemoTimeLeftImage = nullptr;
static void CalculateDemoTimeLeft()
{
	// Demo time left calculation not implemented
}

static void UpdateScreenSaverInfo(uint32_t theTick)
{
	(void)theTick;
	// Screen saver info not needed
}

bool SexyAppBase::DrawDirtyStuff()
{
	SEXY_AUTO_PERF("SexyAppBase::DrawDirtyStuff");
	MTAutoDisallowRand aDisallowRand;

	if (gIsFailing) // just try to reinit
	{
		Redraw(nullptr);
		mHasPendingDraw = false;
		mLastDrawWasEmpty = true;		
		return false;
	}	

	if (mShowFPS)
	{
		switch(mShowFPSMode)
		{
			case FPS_ShowFPS: CalculateFPS(); break;
			case FPS_ShowCoords:
				if (mWidgetManager!=nullptr)
					FPSDrawCoords(mWidgetManager->mLastMouseX, mWidgetManager->mLastMouseY); 
				break;
		}

		if (mPlayingDemoBuffer)
			CalculateDemoTimeLeft();
	}

	uint32_t aStartTime = SDL_GetTicks();

	// Update user input and screen saver info
	static uint32_t aPeriodicTick = 0;
	if (aStartTime-aPeriodicTick > 1000)
	{
		aPeriodicTick = aStartTime;
		UpdateScreenSaverInfo(aStartTime);
	}

	if (gScreenSaverActive)
	{
		mHasPendingDraw = false;
		mLastDrawWasEmpty = true;		
		return false;
	}

	mIsDrawing = true;
	bool drewScreen = mWidgetManager->DrawScreen();
	mIsDrawing = false;

	if ((drewScreen || (aStartTime - mLastDrawTick >= 1000) || (mCustomCursorDirty)) &&
		(static_cast<int>(aStartTime - mNextDrawTick) >= 0))
	{
		mLastDrawWasEmpty = false;

		mDrawCount++;

		uint32_t aMidTime = SDL_GetTicks();

		mFPSCount++;
		mFPSTime += aMidTime - aStartTime;

		mDrawTime += aMidTime - aStartTime;

		if (mShowFPS)
		{
			Graphics g(mGLInterface->GetScreenImage());
			g.DrawImage(gFPSImage,mWidth-gFPSImage->GetWidth()-10,mHeight-gFPSImage->GetHeight()-10);
		
			if (mPlayingDemoBuffer)
				g.DrawImage(gDemoTimeLeftImage,mWidth-gDemoTimeLeftImage->GetWidth()-10,mHeight-gFPSImage->GetHeight()-gDemoTimeLeftImage->GetHeight()-15);
		}

		uint32_t aPreScreenBltTime = SDL_GetTicks();
		mLastDrawTick = aPreScreenBltTime;

		if (drewScreen)
			Redraw(nullptr);

		// This is our one UpdateFTimeAcc if we are vsynched
		UpdateFTimeAcc(); 

		uint32_t aEndTime = SDL_GetTicks();

		mScreenBltTime = aEndTime - aPreScreenBltTime;

		if ((mLoadingThreadStarted) && (!mLoadingThreadCompleted))
		{
			int aTotalTime = aEndTime - aStartTime;

			mNextDrawTick += 35 + std::max(aTotalTime, 15);

			if (static_cast<int>(aEndTime - mNextDrawTick) >= 0)			
				mNextDrawTick = aEndTime;			
		}
		else
			mNextDrawTick = aEndTime;

		mHasPendingDraw = false;		
		mCustomCursorDirty = false;

		return true;
	}
	else
	{		
		mHasPendingDraw = false;
		mLastDrawWasEmpty = true;		
		return false;
	}
}

void SexyAppBase::LogScreenSaverError(const std::string &theError)
{
	(void)theError;
	// Screen saver error logging not implemented
}

void SexyAppBase::BeginPopup()
{
	// Begin popup not needed
}

void SexyAppBase::EndPopup()
{
	if (!mIsPhysWindowed)
		mNoDefer = false;

	ClearUpdateBacklog();
	ClearKeysDown();

	if (mWidgetManager->mDownButtons)
	{
		mWidgetManager->DoMouseUps();
		//ReleaseCapture();
	}
}

int SexyAppBase::MsgBox(const std::string& theText, const std::string& theTitle, int theFlags)
{
	(void)theFlags;

	BeginPopup();
	Sexy::PrintF("%s\n===\n%s\n", theTitle.c_str(), theText.c_str());

#ifdef __SWITCH__
	ErrorApplicationConfig c;
	errorApplicationCreate(&c, theTitle.c_str(), theText.c_str());
	errorApplicationShow(&c);
#endif

	EndPopup();

	return 0;
}

void SexyAppBase::Popup(const std::string& theString)
{
	if (IsScreenSaver())
	{
		LogScreenSaverError(theString);
		return;
	}

	BeginPopup();
	if (!mShutdown)
		Sexy::PrintF("FATAL ERROR\n===\n%s\n", theString.c_str());

#ifdef __SWITCH__
	ErrorApplicationConfig c;
	errorApplicationCreate(&c, "Fatal error", theString.c_str());
	errorApplicationShow(&c);
#endif

	EndPopup();
}


void SexyAppBase::SafeDeleteWidget(Widget* theWidget)
{
	WidgetSafeDeleteInfo aWidgetSafeDeleteInfo;
	aWidgetSafeDeleteInfo.mUpdateAppDepth = mUpdateAppDepth;
	aWidgetSafeDeleteInfo.mWidget = theWidget;
	mSafeDeleteList.push_back(aWidgetSafeDeleteInfo);
}

static int ListDemoMarkers()
{
	gSexyAppBase->mLastTime = SDL_GetTicks();

    return 0; 
}

static int DemoJumpToTime()
{
	gSexyAppBase->mLastTime = SDL_GetTicks();

    return 0; 
}

static void ToggleDemoSoundVolume()
{
	if (gSexyAppBase->GetMusicVolume() == 0.0)
		gSexyAppBase->SetMusicVolume(gSexyAppBase->mDemoMusicVolume);
	else
	{
		gSexyAppBase->mDemoMusicVolume = gSexyAppBase->mMusicVolume;
		gSexyAppBase->SetMusicVolume(0.0);
	}

	if (gSexyAppBase->GetSfxVolume() == 0.0)
		gSexyAppBase->SetSfxVolume(gSexyAppBase->mDemoSfxVolume);
	else
	{
		gSexyAppBase->mDemoSfxVolume = gSexyAppBase->mSfxVolume;
		gSexyAppBase->SetSfxVolume(0.0);
	}
}

static uint32_t gPowerSaveTick = 0;

void SexyAppBase::HandleNotifyGameMessage(int theType)
{
	(void)theType;
	// Notify game message handling not implemented
}

void SexyAppBase::RehupFocus()
{
	bool wantHasFocus = mActive && !mMinimized;

	if (wantHasFocus != mHasFocus)
	{					
		mHasFocus = wantHasFocus;

		if (mHasFocus)
		{
			if (mMuteOnLostFocus)
				Unmute(true);
	
			mWidgetManager->GotFocus();
			GotFocus();
		}
		else
		{
			if (mMuteOnLostFocus)
				Mute(true);
	
			mWidgetManager->LostFocus();
			LostFocus();

			//ReleaseCapture();
			mWidgetManager->DoMouseUps();
		}
	}
}

void SexyAppBase::ClearKeysDown()
{
	if (mWidgetManager != nullptr) // fix stuck alt-key problem
	{
		for (int aKeyNum = 0; aKeyNum < 0xFF; aKeyNum++)
			mWidgetManager->mKeyDown[aKeyNum] = false;
	}
	mCtrlDown = false;
	mAltDown = false;
}

void SexyAppBase::WriteDemoTimingBlock()
{
	// Demo writing functions can only be called from the main thread and after SexyAppBase::Init
	DBG_ASSERTE(std::this_thread::get_id() == mPrimaryThreadId);

	while (mUpdateCount - mLastDemoUpdateCnt > 15)
	{
		mDemoBuffer.WriteNumBits(15, 4);
		mLastDemoUpdateCnt += 15;

		mDemoBuffer.WriteNumBits(0, 1);
		mDemoBuffer.WriteNumBits(DEMO_IDLE, 5);
		mDemoCmdOrder++;
	}
	
	mDemoBuffer.WriteNumBits(mUpdateCount - mLastDemoUpdateCnt, 4);
	mLastDemoUpdateCnt = mUpdateCount;
	mDemoCmdOrder++;
}

int aNumBigMoveMessages = 0;
int aNumSmallMoveMessages = 0;
int aNumTimerMessages = 0;

// Required unused in release mode
bool SexyAppBase::PrepareDemoCommand([[maybe_unused]] bool required)
{
	if (mDemoNeedsCommand)
	{
		mDemoCmdBitPos = mDemoBuffer.mReadBitPos;

		mLastDemoUpdateCnt += mDemoBuffer.ReadNumBits(4, false);

		mDemoIsShortCmd = mDemoBuffer.ReadNumBits(1, false) == 1;

		if (mDemoIsShortCmd)		
			mDemoCmdNum = mDemoBuffer.ReadNumBits(1, false);
		else
			mDemoCmdNum = mDemoBuffer.ReadNumBits(5, false);

		mDemoNeedsCommand = false;

		mDemoCmdOrder++;
	}

	DBG_ASSERTE((mUpdateCount == mLastDemoUpdateCnt) || (!required));

	return mUpdateCount == mLastDemoUpdateCnt;
}

void SexyAppBase::ProcessDemo()
{
	if (mPlayingDemoBuffer)
	{
		// At end of demo buffer?  How dare you!
		DBG_ASSERTE(!mDemoBuffer.AtEnd());

		while ((!mShutdown) && (mUpdateCount == mLastDemoUpdateCnt) && (!mDemoBuffer.AtEnd()))
		{
			if (PrepareDemoCommand(false))
			{
				mDemoNeedsCommand = true;
				
				if (mDemoIsShortCmd)
				{
					switch (mDemoCmdNum)
					{
					case 0:
						{
							int aDeltaX = mDemoBuffer.ReadNumBits(6, true);
							int aDeltaY = mDemoBuffer.ReadNumBits(6, true);
							mLastDemoMouseX += aDeltaX;
							mLastDemoMouseY += aDeltaY;

							mWidgetManager->MouseMove(mLastDemoMouseX, mLastDemoMouseY);
						}
						break;
					case 1:
						{
							bool down = mDemoBuffer.ReadNumBits(1, false) != 0;
							int aBtnCount = mDemoBuffer.ReadNumBits(3, true);							
		
							if (down)
								mWidgetManager->MouseDown(mLastDemoMouseX, mLastDemoMouseY, aBtnCount);
							else
								mWidgetManager->MouseUp(mLastDemoMouseX, mLastDemoMouseY, aBtnCount);
						}
						break;
					}
				}
				else
				{				
					switch (mDemoCmdNum)
					{
					case DEMO_MOUSE_POSITION:
						{
							mLastDemoMouseX = mDemoBuffer.ReadNumBits(12, false);
							mLastDemoMouseY = mDemoBuffer.ReadNumBits(12, false);

							mWidgetManager->MouseMove(mLastDemoMouseX, mLastDemoMouseY);						
						}
						break;
					case DEMO_ACTIVATE_APP:
						{
							mActive = mDemoBuffer.ReadNumBits(1, false) != 0;

							RehupFocus();
							
							if ((mActive) && (!mIsWindowed))
								mWidgetManager->MarkAllDirty();

							if ((mIsOpeningURL) && (!mActive))
								URLOpenSucceeded(mOpeningURL);
						}
						break;
					case DEMO_SIZE:
						{
							bool isMinimized = mDemoBuffer.ReadBoolean();

							if ((!mShutdown) && (isMinimized != mMinimized))
							{
								mMinimized = isMinimized;

								// We don't want any sounds (or music) playing while its minimized
								if (mMinimized)
									Mute(true);
								else
								{
									Unmute(true);
									mWidgetManager->MarkAllDirty();
								}
							}

							RehupFocus();
						}
						break;
					case DEMO_MOUSE_WHEEL:
						{
							int aScroll = mDemoBuffer.ReadNumBits(8, true);
							mWidgetManager->MouseWheel(aScroll);
						}
						break;
					case DEMO_KEY_DOWN:
						{
							KeyCode aKeyCode = static_cast<KeyCode>(mDemoBuffer.ReadNumBits(8, false));
							mWidgetManager->KeyDown(aKeyCode);
						}
						break;
					case DEMO_KEY_UP:
						{
							KeyCode aKeyCode = static_cast<KeyCode>(mDemoBuffer.ReadNumBits(8, false));
							mWidgetManager->KeyUp(aKeyCode);
						}
						break;
					case DEMO_KEY_CHAR:
						{
							int sizeMult = static_cast<int>(mDemoBuffer.ReadNumBits(1, false)) + 1; // will be 1 for single, 2 for double
							char aChar = static_cast<char>(mDemoBuffer.ReadNumBits(8 * sizeMult, false));
							mWidgetManager->KeyChar(aChar);
						}
						break;
					case DEMO_CLOSE:
						Shutdown();
						break;
					case DEMO_MOUSE_ENTER:
						mMouseIn = true;
						EnforceCursor();
						break;
					case DEMO_MOUSE_EXIT:
						mWidgetManager->MouseExit(mLastDemoMouseX, mLastDemoMouseY);
						mMouseIn = false;
						EnforceCursor();
						break;
					case DEMO_LOADING_COMPLETE:
						mDemoLoadingComplete = true;
						break;
					case DEMO_VIDEO_DATA:
						mIsWindowed = mDemoBuffer.ReadBoolean();
						mSyncRefreshRate = mDemoBuffer.ReadByte();
						break;
					case DEMO_IDLE:
						break;
					default:
						DBG_ASSERTE("Invalid Demo Command" == 0);
						break;
					}
				}
			}
		}
	}
}

void SexyAppBase::ShowMemoryUsage()
{
	// Memory usage display not implemented
}

bool SexyAppBase::DebugKeyDown(int theKey)
{
	return false;
}

void SexyAppBase::CloseRequestAsync()
{
}

void SexyAppBase::Done3dTesting()
{
}

// return file name that you want to upload
std::string	SexyAppBase::NotifyCrashHook()
{
	return "";
}

void SexyAppBase::DeleteNativeImageData()
{
	std::vector<MemoryImage*> imagesToProcess;
	{
		std::scoped_lock anAutoCrit(mGLInterface->mCritSect);
		MemoryImageSet::iterator anItr = mMemoryImageSet.begin();
		while (anItr != mMemoryImageSet.end())
		{
			imagesToProcess.push_back(*anItr);
			++anItr;
		}
	}

	for (MemoryImage* img : imagesToProcess)
		img->DeleteNativeData();
}

void SexyAppBase::DeleteExtraImageData()
{
	std::vector<MemoryImage*> imagesToProcess;
	{
		std::scoped_lock anAutoCrit(mGLInterface->mCritSect);
		MemoryImageSet::iterator anItr = mMemoryImageSet.begin();
		while (anItr != mMemoryImageSet.end())
		{
			imagesToProcess.push_back(*anItr);
			++anItr;
		}
	}

	for (MemoryImage* img : imagesToProcess)
		img->DeleteExtraBuffers();
}

void SexyAppBase::ReInitImages()
{
	std::vector<MemoryImage*> imagesToProcess;
	{
		std::scoped_lock anAutoCrit(mGLInterface->mCritSect);
		MemoryImageSet::iterator anItr = mMemoryImageSet.begin();
		while (anItr != mMemoryImageSet.end())
		{
			imagesToProcess.push_back(*anItr);
			++anItr;
		}
	}

	for (MemoryImage* img : imagesToProcess)
		img->ReInit();
}


void SexyAppBase::LoadingThreadProc()
{
}

void SexyAppBase::LoadingThreadCompleted()
{
}

void SexyAppBase::LoadingThreadProcStub(SexyAppBase *theArg)
{
	SexyAppBase* aSexyApp = theArg;
	
	aSexyApp->LoadingThreadProc();		

	Sexy::PrintF("Resource Loading Time: %d\r\n", (SDL_GetTicks() - aSexyApp->mTimeLoaded));

	aSexyApp->mLoadingThreadCompleted = true;
}

void SexyAppBase::StartLoadingThread()
{	
	if (!mLoadingThreadStarted)
	{
		mYieldMainThread = true; 
		//::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);		
		mLoadingThreadStarted = true;
#ifdef __EMSCRIPTEN__
		LoadingThreadProcStub(this);
#else
		//_beginthread(LoadingThreadProcStub, 0, this);
		std::thread(LoadingThreadProcStub, this).detach();
#endif
	}
}
void SexyAppBase::CursorThreadProc()
{
	// Cursor thread not implemented
}

void SexyAppBase::CursorThreadProcStub(void *theArg)
{
	(void)theArg;
}

void SexyAppBase::StartCursorThread()
{
	// Cursor thread not implemented
}

void SexyAppBase::SwitchScreenMode(bool wantWindowed, bool is3d, bool force)
{
#if defined(__IPHONEOS__) || (defined(__ANDROID__) && !defined(__TERMUX__)) || defined(__SWITCH__) || defined(__3DS__)
	// Mobile/console platforms are always fullscreen; skip mode switching entirely.
	Set3DAcclerated(is3d);
	return;
#endif

	if (mForceFullscreen)
		wantWindowed = false;

	if (mIsWindowed == wantWindowed && !force)
	{
		Set3DAcclerated(is3d);
		return;
	}

	// Set 3d acceleration preference
	Set3DAcclerated(is3d,false);

	// Always make the app windowed when playing demos, in order to
	//  make it easier to track down bugs.  We place this after the
	//  sanity check just so things get re-initialized and stuff
	//if (mPlayingDemoBuffer)
	//	wantWindowed = true;

	mIsWindowed = wantWindowed;	

	MakeWindow();
	
	// We need to do this check to allow IE to get focus instead of
	//  stealing it away for ourselves
	if (!mIsOpeningURL)
	{
		//::ShowWindow(mHWnd, SW_NORMAL);
		//::SetForegroundWindow(mHWnd);
	}
	else
	{
		// Show it but don't activate it
		//::ShowWindow(mHWnd, SW_SHOWNOACTIVATE);
	}

	if (mSoundManager!=nullptr)
	{
		//mSoundManager->SetCooperativeWindow(mHWnd);
	}	

	mLastTime = SDL_GetTicks();
}

void SexyAppBase::SwitchScreenMode(bool wantWindowed)
{
	SwitchScreenMode(wantWindowed, Is3DAccelerated());
}

void SexyAppBase::SwitchScreenMode()
{
	SwitchScreenMode(mIsWindowed, Is3DAccelerated(), true);
}

void SexyAppBase::SetAlphaDisabled(bool isDisabled)
{
	if (mAlphaDisabled != isDisabled)
	{
		mAlphaDisabled = isDisabled;
		mGLInterface->SetVideoOnlyDraw(mAlphaDisabled);
		mWidgetManager->mImage = mGLInterface->GetScreenImage();
		mWidgetManager->MarkAllDirty();
	}
}

void SexyAppBase::ResetCustomCursorCache()
{
	if (mCustomCursor != nullptr)
	{
		SDL_FreeCursor(mCustomCursor);
		mCustomCursor = nullptr;
	}

	mCustomCursorImage = nullptr;
	mCustomCursorImageNum = -1;
}

void SexyAppBase::EnforceCursor()
{
	int aCursorNum = mSEHOccured ? CURSOR_POINTER : mCursorNum;
	if (aCursorNum < 0 || aCursorNum >= NUM_CURSORS)
		aCursorNum = CURSOR_POINTER;

	if (aCursorNum == CURSOR_NONE)
	{
		SDL_ShowCursor(SDL_DISABLE);
		return;
	}

	SDL_Cursor* aCursor = nullptr;

	if (mCustomCursorsEnabled && mCursorImages[aCursorNum] != nullptr)
	{
		Image* aCursorImage = mCursorImages[aCursorNum];
		MemoryImage* aMemoryImage = dynamic_cast<MemoryImage*>(aCursorImage);
		if (aMemoryImage != nullptr)
		{
			if (mCustomCursor != nullptr && mCustomCursorImage == aCursorImage && mCustomCursorImageNum == aCursorNum)
			{
				aCursor = mCustomCursor;
			}
			else
			{
				SDL_Cursor* aNewCursor = CreateCursorFromMemoryImage(aMemoryImage);
				if (aNewCursor != nullptr)
				{
					ResetCustomCursorCache();

					mCustomCursor = aNewCursor;
					mCustomCursorImage = aCursorImage;
					mCustomCursorImageNum = aCursorNum;
					aCursor = mCustomCursor;
				}
			}
		}
	}

	if (aCursor == nullptr)
	{
		SDL_Cursor*& aCachedCursor = mSysCursors[aCursorNum];
		if (aCachedCursor == nullptr)
			aCachedCursor = SDL_CreateSystemCursor(CursorNumToSystemCursor(aCursorNum));

		aCursor = aCachedCursor;
		if (aCursor == nullptr)
			aCursor = SDL_GetDefaultCursor();
	}

	if (aCursor != nullptr)
		SDL_SetCursor(aCursor);

	SDL_ShowCursor(SDL_ENABLE);
}

void SexyAppBase::ProcessSafeDeleteList()
{
	MTAutoDisallowRand aDisallowRand;

	std::erase_if(mSafeDeleteList, [&](const WidgetSafeDeleteInfo& theInfo) {
		if (mUpdateAppDepth <= theInfo.mUpdateAppDepth)
		{
			delete theInfo.mWidget;
			return true;
		}
		return false;
	});
}

void SexyAppBase::UpdateFTimeAcc()
{
	uint32_t aCurTime = SDL_GetTicks();

	if (mLastTimeCheck != 0)
	{				
		int aDeltaTime = aCurTime - mLastTimeCheck;		

		mUpdateFTimeAcc = std::min(mUpdateFTimeAcc + aDeltaTime, 200.0);

		if (mRelaxUpdateBacklogCount > 0)				
			mRelaxUpdateBacklogCount = std::max(mRelaxUpdateBacklogCount - aDeltaTime, 0);				
	}

	mLastTimeCheck = aCurTime;
}

//int aNumCalls = 0;
//uint32_t aLastCheck = 0;

bool SexyAppBase::Process(bool allowSleep)
{
	if (mLoadingFailed)
		Shutdown();
	
	bool isVSynched = (!mPlayingDemoBuffer) && (mVSyncUpdates) && (!mLastDrawWasEmpty) && (!mVSyncBroken) &&
		((!mIsPhysWindowed) || (mIsPhysWindowed && mWaitForVSync && !mSoftVSyncWait));
	double aFrameFTime;
	double anUpdatesPerUpdateF;

	if (mVSyncUpdates)
	{
		aFrameFTime = (1000.0 / mSyncRefreshRate) / mUpdateMultiplier;
		anUpdatesPerUpdateF = static_cast<float>(1000.0 / (mFrameTime * mSyncRefreshRate));
	}
	else
	{
		aFrameFTime = mFrameTime / mUpdateMultiplier;
		anUpdatesPerUpdateF = 1.0;
	}

	// Do we need to fast forward?
	if (mPlayingDemoBuffer)
	{
		if (mUpdateCount < mFastForwardToUpdateNum || mFastForwardToMarker)
		{
			if (!mDemoMute && !mFastForwardStep)
			{
				mDemoMute = true;
				Mute(true);
			}

			static uint32_t aTick = SDL_GetTicks();
			while (mUpdateCount < mFastForwardToUpdateNum || mFastForwardToMarker)
			{
				ClearUpdateBacklog();
				int aLastUpdateCount = mUpdateCount;
								
				// Actual updating code below
				//////////////////////////////////////////////////////////////////////////
				
				bool hadRealUpdate = DoUpdateFrames();

				if (hadRealUpdate)
				{					
					mPendingUpdatesAcc += anUpdatesPerUpdateF;
					mPendingUpdatesAcc -= 1.0;
					ProcessSafeDeleteList();

					// Process any extra updates
					while (mPendingUpdatesAcc >= 1.0)
					{						
						// These should just be IDLE commands we have to clear out
						ProcessDemo();												

						bool hasRealUpdate = DoUpdateFrames();
						DBG_ASSERTE(hasRealUpdate);

						if (!hasRealUpdate)
							break;
												
						ProcessSafeDeleteList();
						mPendingUpdatesAcc -= 1.0;
					}
										
					DoUpdateFramesF(static_cast<float>(anUpdatesPerUpdateF));					
					ProcessSafeDeleteList();
				}

				//////////////////////////////////////////////////////////////////////////				

				// If the update count doesn't change, its because we are
				//  playing back a demo and need to read more
				if (aLastUpdateCount == mUpdateCount)
					return true;

				uint32_t aNewTick = SDL_GetTicks();
				if (aNewTick - aTick >= 1000 || mFastForwardStep) // let the app draw some
				{
					mFastForwardStep = false;
					aTick = SDL_GetTicks();
					DrawDirtyStuff();			
					return true;
				}
			}
		}

		if (mDemoMute)
		{
			mDemoMute = false;
			mSoundManager->StopAllSounds();
			Unmute(true);
		}
	}

	// Make sure we're not paused
	if ((!mPaused) && (mUpdateMultiplier > 0))
	{
		uint32_t aStartTime = SDL_GetTicks();
		
		// uint32_t aCurTime = aStartTime; // Unused
		int aCumSleepTime = 0;
		
		// When we are VSynching, only calculate this FTimeAcc right after drawing
		
		if (!isVSynched)		
			UpdateFTimeAcc();					

		// mNonDrawCount is used to make sure we draw the screen at least
		// 10 times per second, even if it means we have to slow down
		// the updates to make it draw 10 times per second in "game time"
		
		bool didUpdate = false;
		
		if (mUpdateAppState == UPDATESTATE_PROCESS_1)
		{
			if ((++mNonDrawCount < static_cast<int>(ceil(10 * mUpdateMultiplier))) || (!mLoaded))
			{
				bool doUpdate = false;
				
				if (isVSynched)
				{
					// Synch'ed to vertical refresh, so update as soon as possible after draw
					doUpdate = (!mHasPendingDraw) || (mUpdateFTimeAcc >= static_cast<int>(aFrameFTime * 0.75));
				}
				else if (mUpdateFTimeAcc >= aFrameFTime)
				{
					doUpdate = true;
				}

				if (doUpdate)
				{
					// Do VSyncBroken test.  This test fails if we're in fullscreen and
					// "don't vsync" has been forced in Advanced settings up Display Properties
					if ((!mPlayingDemoBuffer) && (mUpdateMultiplier == 1.0))
					{
						mVSyncBrokenTestUpdates++;
						if (mVSyncBrokenTestUpdates >= static_cast<uint32_t>((1000 + mFrameTime - 1) / mFrameTime))
						{
							// It has to be running 33% fast to be "broken" (25% = 1/0.800)
							if (aStartTime - mVSyncBrokenTestStartTick <= 800)
							{
								// The test has to fail 3 times in a row before we decide that
								//  vsync is broken overall
								mVSyncBrokenCount++;
								if (mVSyncBrokenCount >= 3)
									mVSyncBroken = true;
							}
							else
								mVSyncBrokenCount = 0;

							mVSyncBrokenTestStartTick = aStartTime;
							mVSyncBrokenTestUpdates = 0;
						}
					}
					
					bool hadRealUpdate = DoUpdateFrames();
					if (hadRealUpdate)
						mUpdateAppState = UPDATESTATE_PROCESS_2;					

					mHasPendingDraw = true;
					didUpdate = true;
				}
			}
		}
		else if (mUpdateAppState == UPDATESTATE_PROCESS_2)
		{
			mUpdateAppState = UPDATESTATE_PROCESS_DONE;
			
			mPendingUpdatesAcc += anUpdatesPerUpdateF;
			mPendingUpdatesAcc -= 1.0;
			ProcessSafeDeleteList();

			// Process any extra updates
			while (mPendingUpdatesAcc >= 1.0)
			{	
				// These should just be IDLE commands we have to clear out
				ProcessDemo();												

				++mNonDrawCount;
				bool hasRealUpdate = DoUpdateFrames();
				DBG_ASSERTE(hasRealUpdate);

				if (!hasRealUpdate)
					break;
										
				ProcessSafeDeleteList();
				mPendingUpdatesAcc -= 1.0;
			}					

			//aNumCalls++;
			DoUpdateFramesF(static_cast<float>(anUpdatesPerUpdateF));
			ProcessSafeDeleteList();		
		
			// Don't let mUpdateFTimeAcc dip below 0
			//  Subtract an extra 0.2ms, because sometimes refresh rates have some
			//  fractional component that gets truncated, and it's better to take off
			//  too much to keep our timing tending toward occuring right after 
			//  redraws
			if (isVSynched)
				mUpdateFTimeAcc = std::max(mUpdateFTimeAcc - aFrameFTime - 0.2f, 0.0);
			else
				mUpdateFTimeAcc -= aFrameFTime;

			if (mRelaxUpdateBacklogCount > 0)
				mUpdateFTimeAcc = 0;

			didUpdate = true;
		}
		
		if (!didUpdate)
		{			
			mUpdateAppState = UPDATESTATE_PROCESS_DONE;

			mNonDrawCount = 0;
			
			if (mHasPendingDraw)
			{
				DrawDirtyStuff();
			}
			else
			{
				// Let us take into account the time it took to draw dirty stuff			
				int aTimeToNextFrame = static_cast<int>(aFrameFTime - mUpdateFTimeAcc);
				if (aTimeToNextFrame > 0)
				{
					if (!allowSleep)
						return false;

					// Wait till next processing cycle
					++mSleepCount;

#ifndef __EMSCRIPTEN__
					timespec ts;
					ts.tv_sec = aTimeToNextFrame / 1000;
					ts.tv_nsec = (aTimeToNextFrame % 1000) * 1000000;
					nanosleep(&ts, &ts);
#endif

					aCumSleepTime += aTimeToNextFrame;					
				}
			}
		}

		if (mYieldMainThread)
		{
			// This is to make sure that the title screen doesn't take up any more than 
			// 1/3 of the processor time

			uint32_t anEndTime = SDL_GetTicks();
			int anElapsedTime = (anEndTime - aStartTime) - aCumSleepTime;
			int aLoadingYieldSleepTime = std::min(250, (anElapsedTime * 2) - aCumSleepTime);

			if (aLoadingYieldSleepTime >= 0)
			{
				if (!allowSleep)
					return false;

#ifndef __EMSCRIPTEN__
				timespec ts;
				ts.tv_sec = aLoadingYieldSleepTime / 1000;
				ts.tv_nsec = (aLoadingYieldSleepTime % 1000) * 1000000;
				nanosleep(&ts, &ts);
#endif
			}
		}
	}

	ProcessSafeDeleteList();	
	return true;
}

#ifdef __EMSCRIPTEN__
void SexyAppBase::EmscriptenMainLoopCallback()
{
	SexyAppBase* app = Sexy::gSexyAppBase;
	if (app->mWindow != nullptr && app->mGLInterface != nullptr)
	{
		app->mGLInterface->UpdateViewport();
		app->mWidgetManager->Resize(app->mScreenBounds, app->mGLInterface->mPresentationRect);
	}
	if (app->mShutdown)
	{
		emscripten_cancel_main_loop();
		app->ProcessSafeDeleteList();
		app->mRunning = false;
		EM_ASM(
			if (typeof FS !== 'undefined' && FS.syncfs) {
				FS.syncfs(false, function(err) {
					if (err) console.warn('IDBFS sync error:', err);
					if (typeof window.onGameExit === 'function') window.onGameExit();
				});
			} else {
				if (typeof window.onGameExit === 'function') window.onGameExit();
			}
		);
		return;
	}
	if (app->mExitToTop)
		app->mExitToTop = false;

	bool updated = false;
	if (!app->UpdateAppStep(&updated))
		return;

	// Prevent web FPS drop and input lag: complete all pending stages and process all events in one rAF.
	while (app->mUpdateAppState != UPDATESTATE_PROCESS_DONE || app->mHasPendingDraw)
	{
		if (!app->UpdateAppStep(&updated))
			break;
	}
}
#endif

void SexyAppBase::DoMainLoop()
{
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(SexyAppBase::EmscriptenMainLoopCallback, 0, 1);
#else
	while (!mShutdown)
	{
		if (mExitToTop)
			mExitToTop = false;
		UpdateApp();
	}
#endif
}

bool SexyAppBase::UpdateAppStep(bool* updated)
{
	if (updated != nullptr)
		*updated = false;

	if (mExitToTop)
		return false;

	if (mUpdateAppState == UPDATESTATE_PROCESS_DONE)
		mUpdateAppState = UPDATESTATE_MESSAGES;

	mUpdateAppDepth++;

	// We update in two stages to avoid doing a Process if our loop termination
	//  condition has already been met by processing windows messages		
	if (mUpdateAppState == UPDATESTATE_MESSAGES)
	{
		ProcessDemo();
		if (!ProcessDeferredMessages(true))
		{			
			mUpdateAppState = UPDATESTATE_PROCESS_1;
		}
	}
	else
	{
		// Process changes state by itself
		if (mStepMode)
		{
			if (mStepMode==2)
			{
#ifndef __EMSCRIPTEN__
				timespec ts;
				ts.tv_sec = mFrameTime / 1000;
				ts.tv_nsec = (mFrameTime % 1000) * 1000000;
				nanosleep(&ts, &ts);
#endif

				mUpdateAppState = UPDATESTATE_PROCESS_DONE; // skip actual update until next step
			}
			else
			{
				mStepMode = 2;
				DoUpdateFrames();
				DoUpdateFramesF(1.0f);
				DrawDirtyStuff();
			}
		}
		else
		{
			int anOldUpdateCnt = mUpdateCount;
			Process();		
			if (updated != nullptr)
				*updated = mUpdateCount != anOldUpdateCnt;			
		}
	}

	mUpdateAppDepth--;

	return true;
}

bool SexyAppBase::UpdateApp()
{
	bool updated;
	for (;;)
	{
		if (!UpdateAppStep(&updated))
			return false;
		if (updated)
			return true;
	}
}

int SexyAppBase::InitGLInterface()
{
	PreGLInterfaceInitHook();
	DeleteNativeImageData();
	int aResult = mGLInterface->Init(mIsPhysWindowed);
	DemoSyncRefreshRate();
	if (aResult)
	{
		mScreenBounds.mX = ( mWidth - mGLInterface->mWidth ) / 2;
		mScreenBounds.mY = ( mHeight - mGLInterface->mHeight ) / 2;
		mScreenBounds.mWidth = mGLInterface->mWidth;
		mScreenBounds.mHeight = mGLInterface->mHeight;
		mWidgetManager->Resize(mScreenBounds, mGLInterface->mPresentationRect);
		PostGLInterfaceInitHook();
	}
	return aResult;
}

void SexyAppBase::PreTerminate()
{
}

void SexyAppBase::Start()
{
	if (mShutdown)
		return;

	StartCursorThread();

	if (mAutoStartLoadingThread)
		StartLoadingThread();

	//::ShowWindow(mHWnd, SW_SHOW);
	//::SetFocus(mHWnd);

	//timeBeginPeriod(1);

	//int aCount = 0; // unused
	//int aSleepCount = 0; // unused

	uint32_t aStartTime = SDL_GetTicks();

	mRunning = true;
	mLastTime = aStartTime;
	mLastUserInputTick = aStartTime;
	mLastTimerTime = aStartTime;

	DoMainLoop();
#ifndef __EMSCRIPTEN__
	ProcessSafeDeleteList();

	mRunning = false;

	WaitForLoadingThread();

	Sexy::PrintF("Seconds       = %g\r\n", (SDL_GetTicks() - aStartTime) / 1000.0);
	//Sexy::PrintF("Count         = %d\r\n", aCount);
	Sexy::PrintF("Sleep Count   = %d\r\n", mSleepCount);
	Sexy::PrintF("Update Count  = %d\r\n", mUpdateCount);
	Sexy::PrintF("Draw Count    = %d\r\n", mDrawCount);
	Sexy::PrintF("Draw Time     = %d\r\n", mDrawTime);
	Sexy::PrintF("Screen Blt    = %d\r\n", mScreenBltTime);
	if (mDrawTime+mScreenBltTime > 0)
	{
		Sexy::PrintF("Avg FPS       = %d\r\n", (mDrawCount*1000)/(mDrawTime+mScreenBltTime));
	}

	//timeEndPeriod(1);	

	PreTerminate();

	WriteToRegistry();
#endif
}

bool SexyAppBase::LoadProperties(const std::string& theFileName, bool required, bool checkSig)
{
	Buffer aBuffer;
	if (!ReadBufferFromFile(theFileName, &aBuffer))
	{
		if (!required)
			return true;
		else
		{
			Popup(GetString("UNABLE_OPEN_PROPERTIES", "Unable to open properties file ") + theFileName);
			return false;
		}
	}
	if (checkSig)
	{
		//if (!CheckSignature(aBuffer, theFileName))
		//{
			//Popup(GetString("PROPERTIES_SIG_FAILED", "Signature check failed on ") + (theFileName + "'"));
			//return false;
		//}
	}

	PropertiesParser aPropertiesParser(this);

	// Load required language-file properties
	if (!aPropertiesParser.ParsePropertiesBuffer(aBuffer))
	{
		Popup(aPropertiesParser.GetErrorText());
		return false;
	}
	else
		return true;
}

bool SexyAppBase::LoadProperties()
{
	// Load required language-file properties
	return LoadProperties("properties/default.xml", true, false);
}

void SexyAppBase::LoadResourceManifest()
{
	if (!mResourceManager->ParseResourcesFile("properties/resources.xml"))
		ShowResourceError(true);
}

void SexyAppBase::ShowResourceError(bool doExit)
{
	Popup(mResourceManager->GetErrorText());	
	if (doExit)
		DoExit(0);
}

bool SexyAppBase::GetBoolean(const std::string& theId)
{
	StringBoolMap::iterator anItr = mBoolProperties.find(theId);
	DBG_ASSERTE(anItr != mBoolProperties.end());
	
	if (anItr != mBoolProperties.end())	
		return anItr->second;
	else
		return false;
}

bool SexyAppBase::GetBoolean(const std::string& theId, bool theDefault)
{
	StringBoolMap::iterator anItr = mBoolProperties.find(theId);	
	
	if (anItr != mBoolProperties.end())	
		return anItr->second;
	else
		return theDefault;	
}

int SexyAppBase::GetInteger(const std::string& theId)
{
	StringIntMap::iterator anItr = mIntProperties.find(theId);
	DBG_ASSERTE(anItr != mIntProperties.end());
	
	if (anItr != mIntProperties.end())	
		return anItr->second;
	else
		return false;
}

int SexyAppBase::GetInteger(const std::string& theId, int theDefault)
{
	StringIntMap::iterator anItr = mIntProperties.find(theId);	
	
	if (anItr != mIntProperties.end())	
		return anItr->second;
	else
		return theDefault;	
}

double SexyAppBase::GetDouble(const std::string& theId)
{
	StringDoubleMap::iterator anItr = mDoubleProperties.find(theId);
	DBG_ASSERTE(anItr != mDoubleProperties.end());
	
	if (anItr != mDoubleProperties.end())
		return anItr->second;
	else
		return false;
}

double SexyAppBase::GetDouble(const std::string& theId, double theDefault)
{
	StringDoubleMap::iterator anItr = mDoubleProperties.find(theId);	
	
	if (anItr != mDoubleProperties.end())	
		return anItr->second;
	else
		return theDefault;
}

std::string SexyAppBase::GetString(const std::string& theId)
{
	std::map<std::string, std::string>::iterator anItr = mStringProperties.find(theId);
	DBG_ASSERTE(anItr != mStringProperties.end());
	
	if (anItr != mStringProperties.end())	
		return anItr->second;
	else
		return "";
}

std::string SexyAppBase::GetString(const std::string& theId, const std::string& theDefault)
{
	std::map<std::string, std::string>::iterator anItr = mStringProperties.find(theId);
	
	if (anItr != mStringProperties.end())	
		return anItr->second;
	else
		return theDefault;	
}

std::vector<std::string> SexyAppBase::GetStringVector(const std::string& theId)
{
	StringStringVectorMap::iterator anItr = mStringVectorProperties.find(theId);
	DBG_ASSERTE(anItr != mStringVectorProperties.end());
	
	if (anItr != mStringVectorProperties.end())	
		return anItr->second;
	else
		return std::vector<std::string>();
}

void SexyAppBase::SetString(const std::string& theId, const std::string& theValue)
{
	std::pair<std::map<std::string, std::string>::iterator, bool> aPair = mStringProperties.insert(std::map<std::string, std::string>::value_type(theId, theValue));
	if (!aPair.second) // Found it, change value
		aPair.first->second = theValue;
}


void SexyAppBase::SetBoolean(const std::string& theId, bool theValue)
{
	std::pair<StringBoolMap::iterator, bool> aPair = mBoolProperties.insert(StringBoolMap::value_type(theId, theValue));
	if (!aPair.second) // Found it, change value
		aPair.first->second = theValue;
}

void SexyAppBase::SetInteger(const std::string& theId, int theValue)
{
	std::pair<StringIntMap::iterator, bool> aPair = mIntProperties.insert(StringIntMap::value_type(theId, theValue));
	if (!aPair.second) // Found it, change value
		aPair.first->second = theValue;
}

void SexyAppBase::SetDouble(const std::string& theId, double theValue)
{
	std::pair<StringDoubleMap::iterator, bool> aPair = mDoubleProperties.insert(StringDoubleMap::value_type(theId, theValue));
	if (!aPair.second) // Found it, change value
		aPair.first->second = theValue;
}

void SexyAppBase::DoParseCmdLine()
{
	if (mArgv != nullptr)
	{
		for (int i = 1; i < mArgc; i++)
		{
			std::string aCurParamName = mArgv[i];
			std::string aCurParamValue;

			size_t anEqualsPos = aCurParamName.find('=');
			if (anEqualsPos != std::string::npos)
			{
				aCurParamValue = aCurParamName.substr(anEqualsPos + 1);
				aCurParamName = aCurParamName.substr(0, anEqualsPos);
			}

			HandleCmdLineParam(aCurParamName, aCurParamValue);
		}
	}

	mCmdLineParsed = true;
}

void SexyAppBase::SetArgs(int argc, char** argv)
{
	mArgc = argc;
	mArgv = argv;
}

void SexyAppBase::ParseCmdLine(const std::string& theCmdLine)
{
	// Command line example:  -play -demofile="game demo.dmo"
	// Results in HandleCmdLineParam("-play", ""); HandleCmdLineParam("-demofile", "game demo.dmo");
	std::string aCurParamName;
	std::string aCurParamValue;

	//int aSpacePos = 0; // Unused
	bool inQuote = false;
	bool onValue = false;

	for (size_t i = 0; i < theCmdLine.length(); i++)
	{
		char c = theCmdLine[i];
		bool atEnd = false;
		
		if (c == '"')
		{
			inQuote = !inQuote;

			if (!inQuote)
				atEnd = true;
		}
		else if ((c == ' ') && (!inQuote))
			atEnd = true;
		else if (c == '=')
			onValue = true;
		else if (onValue)
			aCurParamValue += c;
		else
			aCurParamName += c;
		
		if (i == theCmdLine.length() - 1)
			atEnd = true;
		
		if (atEnd && !aCurParamName.empty())
		{
			HandleCmdLineParam(aCurParamName, aCurParamValue);
			aCurParamName = "";
			aCurParamValue = "";
			onValue = false;
		}	
	}
}

static int GetMaxDemoFileNum(const std::string& theDemoPrefix, int theMaxToKeep, bool doErase)
{
	(void)theDemoPrefix;
	(void)theMaxToKeep;
	(void)doErase;
	// Demo file enumeration not implemented
	return 0;
}

void SexyAppBase::HandleCmdLineParam(const std::string& theParamName, const std::string& theParamValue)
{
	if (theParamName == "-play")
	{
		mPlayingDemoBuffer = true;
		mRecordingDemoBuffer = false;
	}
	else if (theParamName == "-recnum")
	{
		int aNum = atoi(theParamValue.c_str());
		if (aNum<=0)
			aNum=5;

		int aDemoFileNum = GetMaxDemoFileNum(mDemoPrefix, aNum, true) + 1;
		mDemoFileName = StrFormat((mDemoPrefix + "%d.dmo").c_str(), aDemoFileNum);
		if (mDemoFileName.length() < 2)
		{
			mDemoFileName = GetAppDataPath(mDemoFileName);
		}
		mRecordingDemoBuffer = true;
		mPlayingDemoBuffer = false;
	}
	else if (theParamName == "-playnum")
	{
		int aNum = atoi(theParamValue.c_str())-1;
		aNum = std::max(aNum, 0);

		int aDemoFileNum = GetMaxDemoFileNum(mDemoPrefix, aNum, false)-aNum;
		mDemoFileName = StrFormat((mDemoPrefix + "%d.dmo").c_str(), aDemoFileNum);
		mRecordingDemoBuffer = false;
		mPlayingDemoBuffer = true;
	}
	else if (theParamName == "-record")
	{
		mRecordingDemoBuffer = true;
		mPlayingDemoBuffer = false;
	}
	else if (theParamName == "-demofile")
	{
		mDemoFileName = theParamValue;
		if (mDemoFileName.length() < 2)
		{
			mDemoFileName = GetAppDataPath(mDemoFileName);
		}
	}	
	else if (theParamName == "-crash")
	{
		// Try to access nullptr
		char* a = 0;
		*a = '!';		
	}
	else if (theParamName == "-screensaver")
	{
		mIsScreenSaver = true;
	}
	else if (theParamName == "-resdir")
	{
		mResourceDir = theParamValue;
	}
	else if (theParamName == "-savedir")
	{
		mCustomSaveDir = theParamValue;
	}
	else
	{
		Popup(GetString("INVALID_COMMANDLINE_PARAM", "Invalid command line parameter: ") + theParamName);
		DoExit(0);
	}
}

void SexyAppBase::PreDisplayHook()
{
}

void SexyAppBase::PreGLInterfaceInitHook()
{
}

void SexyAppBase::PostGLInterfaceInitHook()
{
}

bool SexyAppBase::ChangeDirHook(const char *theIntendedPath)
{
	(void)theIntendedPath;
	return false;
}

MusicInterface* SexyAppBase::CreateMusicInterface()
{
	if (mNoSoundNeeded)
		return new DummyMusicInterface();
	else
		return new SDLMusicInterface();
}

void SexyAppBase::InitPropertiesHook()
{
}

void SexyAppBase::InitHook()
{
}

void SexyAppBase::Init()
{
	mPrimaryThreadId = std::this_thread::get_id();	
	
	if (mShutdown)
		return;

	// Set resource directory (for main.pak, properties/, etc.)
	if (!ChangeDirHook(mResourceDir.c_str()))
	{
		SetResourceFolder(mResourceDir);
	}

	gPakInterface->AddPakFile(GetResourcePath("main.pak"));

	InitPropertiesHook();

#if defined(__ANDROID__) && !defined(__TERMUX__)
	{
		const char* aExtPath = SDL_AndroidGetExternalStoragePath();
		if (aExtPath)
		{
			SetAppDataFolder(std::string(aExtPath) + "/");
		}
	}
#elif defined(__IPHONEOS__)
	{
		const char* aHome = std::getenv("HOME");
		if (aHome != nullptr && aHome[0] != '\0')
		{
			SetAppDataFolder((std::filesystem::path(aHome) / "Documents").generic_string() + "/");
		}
	}
#elif defined(__EMSCRIPTEN__)
	{
		SetAppDataFolder("/saves/");
	}
#elif !defined(__SWITCH__) && !defined(__3DS__)
	{
		char* aPrefPath = SDL_GetPrefPath("io.github.wszqkzqk", "PvZPortable"); // Avoid conflict with official Plants vs. Zombies
		if (aPrefPath)
		{
			SetAppDataFolder(aPrefPath);
			SDL_free(aPrefPath);
		}
	}
#endif
	
	if (!mCmdLineParsed)
		mCmdLineParsed = true;

	if (IsScreenSaver())	
		mOnlyAllowOneCopyToRun = false;	

	// Handle custom save directory from -savedir parameter
	if (!mCustomSaveDir.empty())
	{
		SetAppDataFolder(mCustomSaveDir);
	}
	else if (GetAppDataFolder().empty())
	{
		SetAppDataFolder(GetResourcePath("savedata"));
	}

	ReadFromRegistry();	

	mRandSeed = SDL_GetTicks();
	SRand(mRandSeed);

	// Set up demo recording stuff
	if (mPlayingDemoBuffer)
	{
		std::string anError;
		if (!ReadDemoBuffer(anError))
		{
			mPlayingDemoBuffer = false;
			Popup(anError);
			DoExit(0);
		}
	}

	srand(SDL_GetTicks());

	mIsWideWindow = sizeof(char) > 1;
		
	// Let app do something before showing window, or switching to fullscreen mode
	// NOTE: Moved call to PreDisplayHook above mIsWindowed and GetSystemsMetrics
	// checks because the checks below use values that could change in PreDisplayHook.
	// PreDisplayHook must call mWidgetManager->Resize if it changes mWidth or mHeight.
	PreDisplayHook();

	mWidgetManager->Resize(Rect(0, 0, mWidth, mHeight), Rect(0, 0, mWidth, mHeight));

	MakeWindow();

	if (mGLInterface == nullptr)
	{
		fprintf(stderr, "FATAL: Failed to create OpenGL interface.\n");
		mShutdown = true;
		return;
	}

	if (mPlayingDemoBuffer)
	{
		// Get video data
		
		PrepareDemoCommand(true);
		mDemoNeedsCommand = true;
		
		DBG_ASSERTE(!mDemoIsShortCmd);
		DBG_ASSERTE(mDemoCmdNum == DEMO_VIDEO_DATA);

		mIsWindowed = mDemoBuffer.ReadBoolean();
		mSyncRefreshRate = mDemoBuffer.ReadByte();
	}

	if (mSoundManager == nullptr)		
		mSoundManager = new SDLSoundManager();

	SetSfxVolume(mSfxVolume);
	
	mMusicInterface = CreateMusicInterface();	

	SetMusicVolume(mMusicVolume);	

	if (IsScreenSaver())
	{
		SetCursor(CURSOR_NONE);
	}

	InitHook();

	InitInput();

	mInitialized = true;
}

void SexyAppBase::HandleGameAlreadyRunning()
{
	if(mOnlyAllowOneCopyToRun)
	{
		DoExit(0);
	}
}

void SexyAppBase::CopyToClipboard(const std::string& theString)
{
	if (mPlayingDemoBuffer)
		return;

	SDL_SetClipboardText(theString.c_str());
}

std::string	SexyAppBase::GetClipboard()
{
	if (mPlayingDemoBuffer)
		return "";

	char* text = SDL_GetClipboardText();
	if (text == nullptr)
		return "";

	std::string result(text);
	SDL_free(text);
	return result;
}

void SexyAppBase::SetCursor(int theCursorNum)
{
	mCursorNum = theCursorNum;
	EnforceCursor();
}

int SexyAppBase::GetCursor()
{
	return mCursorNum;
}

void SexyAppBase::EnableCustomCursors(bool enabled)
{
	if (mCustomCursorsEnabled == enabled)
	{
		EnforceCursor();
		return;
	}

	mCustomCursorsEnabled = enabled;
	if (!enabled)
		ResetCustomCursorCache();

	EnforceCursor();
}

Sexy::GLImage* SexyAppBase::GetImage(const std::string& theFileName, bool commitBits)
{	
	ImageLib::Image* aLoadedImage = ImageLib::GetImage(theFileName, true);
	
	if (aLoadedImage == nullptr)
		return nullptr;	
	
	GLImage* anImage = new GLImage(mGLInterface);
	anImage->mFilePath = theFileName;
	anImage->SetBits(aLoadedImage->GetBits(), aLoadedImage->GetWidth(), aLoadedImage->GetHeight(), commitBits);	
	anImage->mFilePath = theFileName;
	delete aLoadedImage;
	
	return anImage;
}

Sexy::GLImage* SexyAppBase::CreateCrossfadeImage(Sexy::Image* theImage1, const Rect& theRect1, Sexy::Image* theImage2, const Rect& theRect2, double theFadeFactor)
{
	MemoryImage* aMemoryImage1 = dynamic_cast<MemoryImage*>(theImage1);
	MemoryImage* aMemoryImage2 = dynamic_cast<MemoryImage*>(theImage2);

	if ((aMemoryImage1 == nullptr) || (aMemoryImage2 == nullptr))
		return nullptr;

	if ((theRect1.mX < 0) || (theRect1.mY < 0) || 
		(theRect1.mX + theRect1.mWidth > theImage1->GetWidth()) ||
		(theRect1.mY + theRect1.mHeight > theImage1->GetHeight()))
	{
		DBG_ASSERTE("Crossfade Rect1 out of bounds");
		return nullptr;
	}

	if ((theRect2.mX < 0) || (theRect2.mY < 0) || 
		(theRect2.mX + theRect2.mWidth > theImage2->GetWidth()) ||
		(theRect2.mY + theRect2.mHeight > theImage2->GetHeight()))
	{
		DBG_ASSERTE("Crossfade Rect2 out of bounds");
		return nullptr;
	}

	int aWidth = theRect1.mWidth;
	int aHeight = theRect1.mHeight;

	GLImage* anImage = new GLImage(mGLInterface);
	anImage->Create(aWidth, aHeight);

	uint32_t* aDestBits = anImage->GetBits();
	uint32_t* aSrcBits1 = aMemoryImage1->GetBits();
	uint32_t* aSrcBits2 = aMemoryImage2->GetBits();

	int aSrc1Width = aMemoryImage1->GetWidth();
	int aSrc2Width = aMemoryImage2->GetWidth();
	uint32_t aMult = static_cast<uint32_t>(theFadeFactor * 256);
	uint32_t aOMM = (256 - aMult);

	for (int y = 0; y < aHeight; y++)
	{
		uint32_t* s1 = &aSrcBits1[(y+theRect1.mY)*aSrc1Width+theRect1.mX];
		uint32_t* s2 = &aSrcBits2[(y+theRect2.mY)*aSrc2Width+theRect2.mX];
		uint32_t* d = &aDestBits[y*aWidth];

		for (int x = 0; x < aWidth; x++)
		{
			uint32_t p1 = *s1++;
			uint32_t p2 = *s2++;

			//p1 = 0;
			//p2 = 0xFFFFFFFF;

			*d++ = 
				((((p1 & 0x000000FF)*aOMM + (p2 & 0x000000FF)*aMult)>>8) & 0x000000FF) |
				((((p1 & 0x0000FF00)*aOMM + (p2 & 0x0000FF00)*aMult)>>8) & 0x0000FF00) |
				((((p1 & 0x00FF0000)*aOMM + (p2 & 0x00FF0000)*aMult)>>8) & 0x00FF0000) |
				((((p1 >> 24)*aOMM + (p2 >> 24)*aMult)<<16) & 0xFF000000);
		}
	}

	anImage->BitsChanged();
	
	return anImage;
}

void SexyAppBase::ColorizeImage(Image* theImage, const Color& theColor)
{
	MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);

	if (aSrcMemoryImage == nullptr)
		return;

	uint32_t* aBits;	
	int aNumColors;

	if (aSrcMemoryImage->mColorTable == nullptr)
	{
		aBits = aSrcMemoryImage->GetBits();		
		aNumColors = theImage->GetWidth()*theImage->GetHeight();				
	}
	else
	{
		aBits = aSrcMemoryImage->mColorTable;		
		aNumColors = 256;				
	}
						
	if ((theColor.mAlpha <= 255) && (theColor.mRed <= 255) && 
		(theColor.mGreen <= 255) && (theColor.mBlue <= 255))
	{
		for (int i = 0; i < aNumColors; i++)
		{
			uint32_t aColor = aBits[i];

			aBits[i] = 
				((((aColor & 0xFF000000) >> 8) * theColor.mAlpha) & 0xFF000000) |
				((((aColor & 0x00FF0000) * theColor.mRed) >> 8) & 0x00FF0000) |
				((((aColor & 0x0000FF00) * theColor.mGreen) >> 8) & 0x0000FF00)|
				((((aColor & 0x000000FF) * theColor.mBlue) >> 8) & 0x000000FF);
		}
	}
	else
	{
		for (int i = 0; i < aNumColors; i++)
		{
			uint32_t aColor = aBits[i];

			int aAlpha = ((aColor >> 24) * theColor.mAlpha) / 255;
			int aRed = (((aColor >> 16) & 0xFF) * theColor.mRed) / 255;
			int aGreen = (((aColor >> 8) & 0xFF) * theColor.mGreen) / 255;
			int aBlue = ((aColor & 0xFF) * theColor.mBlue) / 255;

			aAlpha = std::min(aAlpha, 255);
			aRed = std::min(aRed, 255);
			aGreen = std::min(aGreen, 255);
			aBlue = std::min(aBlue, 255);

			aBits[i] = (aAlpha << 24) | (aRed << 16) | (aGreen << 8) | (aBlue);
		}
	}	

	aSrcMemoryImage->BitsChanged();
}

GLImage* SexyAppBase::CreateColorizedImage(Image* theImage, const Color& theColor)
{
	MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);

	if (aSrcMemoryImage == nullptr)
		return nullptr;

	GLImage* anImage = new GLImage(mGLInterface);
	
	anImage->Create(theImage->GetWidth(), theImage->GetHeight());
	
	uint32_t* aSrcBits;
	uint32_t* aDestBits;
	int aNumColors;

	if (aSrcMemoryImage->mColorTable == nullptr)
	{
		aSrcBits = aSrcMemoryImage->GetBits();
		aDestBits = anImage->GetBits();
		aNumColors = theImage->GetWidth()*theImage->GetHeight();				
	}
	else
	{
		aSrcBits = aSrcMemoryImage->mColorTable;
		aDestBits = anImage->mColorTable = new uint32_t[256];
		aNumColors = 256;
		
		anImage->mColorIndices = new uchar[anImage->mWidth*theImage->mHeight];
		memcpy(anImage->mColorIndices, aSrcMemoryImage->mColorIndices, anImage->mWidth*theImage->mHeight);
	}
						
	if ((theColor.mAlpha <= 255) && (theColor.mRed <= 255) && 
		(theColor.mGreen <= 255) && (theColor.mBlue <= 255))
	{
		for (int i = 0; i < aNumColors; i++)
		{
			uint32_t aColor = aSrcBits[i];

			aDestBits[i] = 
				((((aColor & 0xFF000000) >> 8) * theColor.mAlpha) & 0xFF000000) |
				((((aColor & 0x00FF0000) * theColor.mRed) >> 8) & 0x00FF0000) |
				((((aColor & 0x0000FF00) * theColor.mGreen) >> 8) & 0x0000FF00)|
				((((aColor & 0x000000FF) * theColor.mBlue) >> 8) & 0x000000FF);
		}
	}
	else
	{
		for (int i = 0; i < aNumColors; i++)
		{
			uint32_t aColor = aSrcBits[i];

			int aAlpha = ((aColor >> 24) * theColor.mAlpha) / 255;
			int aRed = (((aColor >> 16) & 0xFF) * theColor.mRed) / 255;
			int aGreen = (((aColor >> 8) & 0xFF) * theColor.mGreen) / 255;
			int aBlue = ((aColor & 0xFF) * theColor.mBlue) / 255;

			aAlpha = std::min(aAlpha, 255);
			aRed = std::min(aRed, 255);
			aGreen = std::min(aGreen, 255);
			aBlue = std::min(aBlue, 255);

			aDestBits[i] = (aAlpha << 24) | (aRed << 16) | (aGreen << 8) | (aBlue);
		}
	}	

	anImage->BitsChanged();

	return anImage;
}

GLImage* SexyAppBase::CopyImage(Image* theImage, const Rect& theRect)
{
	GLImage* anImage = new GLImage(mGLInterface);

	anImage->Create(theRect.mWidth, theRect.mHeight);
	
	Graphics g(anImage);
	g.DrawImage(theImage, -theRect.mX, -theRect.mY);

	anImage->CopyAttributes(theImage);

	return anImage;
}

GLImage* SexyAppBase::CopyImage(Image* theImage)
{
	return CopyImage(theImage, Rect(0, 0, theImage->GetWidth(), theImage->GetHeight()));
}

void SexyAppBase::MirrorImage(Image* theImage)
{
	MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);	

	uint32_t* aSrcBits = aSrcMemoryImage->GetBits();

	int aPhysSrcWidth = aSrcMemoryImage->mWidth;
	for (int y = 0; y < aSrcMemoryImage->mHeight; y++)
	{
		uint32_t* aLeftBits = aSrcBits + (y * aPhysSrcWidth);		
		uint32_t* aRightBits = aLeftBits + (aPhysSrcWidth - 1);

		for (int x = 0; x < (aPhysSrcWidth >> 1); x++)
		{
			uint32_t aSwap = *aLeftBits;

			*(aLeftBits++) = *aRightBits;
			*(aRightBits--) = aSwap;
		}
	}

	aSrcMemoryImage->BitsChanged();	
}

void SexyAppBase::FlipImage(Image* theImage)
{
	MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);

	uint32_t* aSrcBits = aSrcMemoryImage->GetBits();

	int aPhysSrcHeight = aSrcMemoryImage->mHeight;
	int aPhysSrcWidth = aSrcMemoryImage->mWidth;
	for (int x = 0; x < aPhysSrcWidth; x++)
	{
		uint32_t* aTopBits    = aSrcBits + x;
		uint32_t* aBottomBits = aTopBits + (aPhysSrcWidth * (aPhysSrcHeight - 1));

		for (int y = 0; y < (aPhysSrcHeight >> 1); y++)
		{
			uint32_t aSwap = *aTopBits;

			*aTopBits = *aBottomBits;
			aTopBits += aPhysSrcWidth;
			*aBottomBits = aSwap;
			aBottomBits -= aPhysSrcWidth;
		}
	}

	aSrcMemoryImage->BitsChanged();	
}

void SexyAppBase::RotateImageHue(Sexy::MemoryImage *theImage, int theDelta)
{
	while (theDelta < 0)
		theDelta += 256;

	int aSize = theImage->mWidth * theImage->mHeight;
	uint32_t *aPtr = theImage->GetBits();
	for (int i=0; i<aSize; i++)
	{
		uint32_t aPixel = *aPtr;
		int alpha = aPixel&0xff000000;
		int r = (aPixel>>16)&0xff;
		int g = (aPixel>>8) &0xff;
		int b = aPixel&0xff;

		int maxval = std::max(r, std::max(g, b));
		int minval = std::min(r, std::min(g, b));
		int h = 0;
		int s = 0;
		int l = (minval+maxval)/2;
		int delta = maxval - minval;

		if (delta != 0)
		{			
			s = (delta * 256) / ((l <= 128) ? (minval + maxval) : (512 - maxval - minval));
			
			if (r == maxval)
				h = (g == minval ? 1280 + (((maxval-b) * 256) / delta) :  256 - (((maxval - g) * 256) / delta));
			else if (g == maxval)
				h = (b == minval ?  256 + (((maxval-r) * 256) / delta) :  768 - (((maxval - b) * 256) / delta));
			else
				h = (r == minval ?  768 + (((maxval-g) * 256) / delta) : 1280 - (((maxval - r) * 256) / delta));
			
			h /= 6;
		}

		h += theDelta;
		if (h >= 256)
			h -= 256;

		double v= (l < 128) ? (l * (255+s))/255 :
				(l+s-l*s/255);
		
		int y = static_cast<int>(2 * l - v);

		int aColorDiv = (6 * h) / 256;
		int x = static_cast<int>(y + (v - y) * ((h - (aColorDiv * 256 / 6)) * 6) / 255);
		x = std::min(x, 255);

		int z = static_cast<int>(v - (v - y) * ((h - (aColorDiv * 256 / 6)) * 6) / 255);
		z = std::max(z, 0);
		
		switch (aColorDiv)
		{
			case 0: r = static_cast<int>(v); g = x; b = y; break;
			case 1: r = z; g = static_cast<int>(v); b = y; break;
			case 2: r = y; g = static_cast<int>(v); b = x; break;
			case 3: r = y; g = z; b = static_cast<int>(v); break;
			case 4: r = x; g = y; b = static_cast<int>(v); break;
			case 5: r = static_cast<int>(v); g = y; b = z; break;
			default: r = static_cast<int>(v); g = x; b = y; break;
		}

		*aPtr++ = alpha | (r<<16) | (g << 8) | (b);	 

	}

	theImage->BitsChanged();
}

uint32_t SexyAppBase::HSLToRGB(int h, int s, int l)
{
	int r;
	int g;
	int b;

	double v= (l < 128) ? (l * (255+s))/255 :
			(l+s-l*s/255);
	
	int y = static_cast<int>(2 * l - v);

	int aColorDiv = (6 * h) / 256;
	int x = static_cast<int>(y + (v - y) * ((h - (aColorDiv * 256 / 6)) * 6) / 255);
	x = std::min(x, 255);

	int z = static_cast<int>(v - (v - y) * ((h - (aColorDiv * 256 / 6)) * 6) / 255);
	z = std::max(z, 0);
	
	switch (aColorDiv)
	{
		case 0: r = static_cast<int>(v); g = x; b = y; break;
		case 1: r = z; g = static_cast<int>(v); b = y; break;
		case 2: r = y; g = static_cast<int>(v); b = x; break;
		case 3: r = y; g = z; b = static_cast<int>(v); break;
		case 4: r = x; g = y; b = static_cast<int>(v); break;
		case 5: r = static_cast<int>(v); g = y; b = z; break;
		default: r = static_cast<int>(v); g = x; b = y; break;
	}

	return 0xFF000000 | (r << 16) | (g << 8) | (b);
}

uint32_t SexyAppBase::RGBToHSL(int r, int g, int b)
{					
	int maxval = std::max(r, std::max(g, b));
	int minval = std::min(r, std::min(g, b));
	int hue = 0;
	int saturation = 0;
	int luminosity = (minval+maxval)/2;
	int delta = maxval - minval;

	if (delta != 0)
	{			
		saturation = (delta * 256) / ((luminosity <= 128) ? (minval + maxval) : (512 - maxval - minval));
		
		if (r == maxval)
			hue = (g == minval ? 1280 + (((maxval-b) * 256) / delta) :  256 - (((maxval - g) * 256) / delta));
		else if (g == maxval)
			hue = (b == minval ?  256 + (((maxval-r) * 256) / delta) :  768 - (((maxval - b) * 256) / delta));
		else
			hue = (r == minval ?  768 + (((maxval-g) * 256) / delta) : 1280 - (((maxval - r) * 256) / delta));
		
		hue /= 6;
	}

	return 0xFF000000 | (hue) | (saturation << 8) | (luminosity << 16);	 
}

void SexyAppBase::HSLToRGB(const uint32_t* theSource, uint32_t* theDest, int theSize)
{
	for (int i = 0; i < theSize; i++)
	{
		uint32_t src = theSource[i];
		theDest[i] = (src & 0xFF000000) | (HSLToRGB((src & 0xFF), (src >> 8) & 0xFF, (src >> 16) & 0xFF) & 0x00FFFFFF);
	}
}

void SexyAppBase::RGBToHSL(const uint32_t* theSource, uint32_t* theDest, int theSize)
{
	for (int i = 0; i < theSize; i++)
	{
		uint32_t src = theSource[i];
		theDest[i] = (src & 0xFF000000) | (RGBToHSL(((src >> 16) & 0xFF), (src >> 8) & 0xFF, (src & 0xFF)) & 0x00FFFFFF);
	}
}

void SexyAppBase::PrecacheAdditive(MemoryImage* theImage)
{
	theImage->GetRLAdditiveData(mGLInterface);
}

void SexyAppBase::PrecacheAlpha(MemoryImage* theImage)
{
	theImage->GetRLAlphaData();
}

void SexyAppBase::PrecacheNative(MemoryImage* theImage)
{
	theImage->GetNativeAlphaData(mGLInterface);
}


void SexyAppBase::PlaySample(intptr_t theSoundNum)
{
	if (!mSoundManager)
		return;

	SoundInstance* aSoundInstance = mSoundManager->GetSoundInstance(theSoundNum);
	if (aSoundInstance != nullptr)
	{
		aSoundInstance->Play(false, true);
	}
}


void SexyAppBase::PlaySample(intptr_t theSoundNum, int thePan)
{
	if (!mSoundManager)
		return;

	SoundInstance* aSoundInstance = mSoundManager->GetSoundInstance(theSoundNum);
	if (aSoundInstance != nullptr)
	{
		aSoundInstance->SetPan(thePan);
		aSoundInstance->Play(false, true);
	}
}

bool SexyAppBase::IsMuted()
{
	return mMuteCount > 0;
}

void SexyAppBase::Mute(bool autoMute)
{	
	mMuteCount++;
	if (autoMute)
		mAutoMuteCount++;

	SetMusicVolume(mMusicVolume);
	SetSfxVolume(mSfxVolume);
}

void SexyAppBase::Unmute(bool autoMute)
{	
	if (mMuteCount > 0)
	{
		mMuteCount--;
		if (autoMute)
			mAutoMuteCount--;
	}

	SetMusicVolume(mMusicVolume);
	SetSfxVolume(mSfxVolume);
}


double SexyAppBase::GetMusicVolume()
{
	return mMusicVolume;
}

void SexyAppBase::SetMusicVolume(double theVolume)
{
	mMusicVolume = theVolume;

	if (mMusicInterface != nullptr)
		mMusicInterface->SetVolume((mMuteCount > 0) ? 0.0 : mMusicVolume);
}

double SexyAppBase::GetSfxVolume()
{
	return mSfxVolume;
}

void SexyAppBase::SetSfxVolume(double theVolume)
{
	mSfxVolume = theVolume;

	if (mSoundManager != nullptr)
		mSoundManager->SetVolume((mMuteCount > 0) ? 0.0 : mSfxVolume);
}

double SexyAppBase::GetMasterVolume()
{
	return mSoundManager->GetMasterVolume();
}

void SexyAppBase::SetMasterVolume(double theMasterVolume)
{
	mSfxVolume = theMasterVolume;
	mSoundManager->SetMasterVolume(mSfxVolume);
}

void SexyAppBase::AddMemoryImage(MemoryImage* theMemoryImage)
{
	std::scoped_lock anAutoCrit(mGLInterface->mCritSect);
	mMemoryImageSet.insert(theMemoryImage);
}

void SexyAppBase::RemoveMemoryImage(MemoryImage* theMemoryImage)
{
	{
		std::scoped_lock anAutoCrit(mGLInterface->mCritSect);
		MemoryImageSet::iterator anItr = mMemoryImageSet.find(theMemoryImage);
		if (anItr != mMemoryImageSet.end())
			mMemoryImageSet.erase(anItr);
	}

	Remove3DData(theMemoryImage);
}

void SexyAppBase::Remove3DData(MemoryImage* theMemoryImage)
{
	if (mGLInterface)
		mGLInterface->Remove3DData(theMemoryImage);
}


bool SexyAppBase::Is3DAccelerated()
{
	//return mDDInterface->mIs3D;
	return true;
}

bool SexyAppBase::Is3DAccelerationSupported()
{
	return true;
}

bool SexyAppBase::Is3DAccelerationRecommended()
{
	return true;
}

void SexyAppBase::DemoSyncRefreshRate()
{
	mSyncRefreshRate = mGLInterface->mRefreshRate;

	if (mRecordingDemoBuffer)
	{
		WriteDemoTimingBlock();
		mDemoBuffer.WriteNumBits(0, 1);
		mDemoBuffer.WriteNumBits(DEMO_VIDEO_DATA, 5);
		mDemoBuffer.WriteBoolean(mIsWindowed);
		uchar aByte = static_cast<uchar>(mSyncRefreshRate);
		mDemoBuffer.WriteByte(aByte);		
	}
}

void SexyAppBase::Set3DAcclerated(bool is3D, bool reinit)
{
	(void)is3D;
	(void)reinit;
	// 3D acceleration toggle not implemented
}

SharedImageRef SexyAppBase::SetSharedImage(const std::string& theFileName, const std::string& theVariant, GLImage* theImage, bool* isNew)
{
	std::string anUpperFileName = StringToUpper(theFileName);
	std::string anUpperVariant = StringToUpper(theVariant);

	std::pair<SharedImageMap::iterator, bool> aResultPair;
	SharedImageRef aSharedImageRef;
	
	if (mGLInterface != nullptr) {
		std::scoped_lock anAutoCrit(mGLInterface->mCritSect);
		aResultPair = mSharedImageMap.try_emplace(SharedImageMap::key_type(anUpperFileName, anUpperVariant));
		aSharedImageRef = &aResultPair.first->second;
	} else {
		aResultPair = mSharedImageMap.try_emplace(SharedImageMap::key_type(anUpperFileName, anUpperVariant));
		aSharedImageRef = &aResultPair.first->second;
	}

	if (isNew != nullptr)
		*isNew = aResultPair.second;

	if (aResultPair.second)
	{
		aSharedImageRef.mSharedImage->mImage = theImage;
	}

	return aSharedImageRef;
}

SharedImageRef SexyAppBase::GetSharedImage(const std::string& theFileName, const std::string& theVariant, bool* isNew)
{	
	std::string anUpperFileName = StringToUpper(theFileName);
	std::string anUpperVariant = StringToUpper(theVariant);

	std::pair<SharedImageMap::iterator, bool> aResultPair;
	SharedImageRef aSharedImageRef;

	if (mGLInterface != nullptr) {
		std::scoped_lock anAutoCrit(mGLInterface->mCritSect);
		aResultPair = mSharedImageMap.try_emplace(SharedImageMap::key_type(anUpperFileName, anUpperVariant));
		aSharedImageRef = &aResultPair.first->second;
	} else {
		aResultPair = mSharedImageMap.try_emplace(SharedImageMap::key_type(anUpperFileName, anUpperVariant));
		aSharedImageRef = &aResultPair.first->second;
	}

	if (isNew != nullptr)
		*isNew = aResultPair.second;

	if (aResultPair.second)
	{
		// Leading '!' means create a blank image rather than loading from file
		if ((theFileName.length() > 0) && (theFileName[0] == '!'))
			aSharedImageRef.mSharedImage->mImage = new GLImage(mGLInterface);
		else
			aSharedImageRef.mSharedImage->mImage = GetImage(theFileName,false);
	}

	return aSharedImageRef;
}

void SexyAppBase::CleanSharedImages()
{
	if (mGLInterface == nullptr) return;

	std::vector<GLImage*> imagesToDelete;
	{
		std::scoped_lock anAutoCrit(mGLInterface->mCritSect);

		if (mCleanupSharedImages.load(std::memory_order_relaxed))
		{
			// Delete shared images with reference counts of 0
			// This doesn't occur in ~SharedImageRef because sometimes we can not only access the image
			//  through the SharedImageRef returned by GetSharedImage, but also by calling GetSharedImage
			//  again with the same params -- so we can have instances where we do the 'final' deref on
			//  an image but immediately re-request it via GetSharedImage
			SharedImageMap::iterator aSharedImageItr = mSharedImageMap.begin();
			while (aSharedImageItr != mSharedImageMap.end())
			{
				SharedImage* aSharedImage = &aSharedImageItr->second;
				if (aSharedImage->mRefCount == 0)
				{
					imagesToDelete.push_back(aSharedImage->mImage);
					mSharedImageMap.erase(aSharedImageItr++);
				}
				else
					++aSharedImageItr;
			}

			mCleanupSharedImages.store(false, std::memory_order_relaxed);
		}
	}

	for (GLImage* img : imagesToDelete)
		delete img;
}
