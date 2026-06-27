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

#ifndef __SEXYAPPBASE_H__
#define __SEXYAPPBASE_H__

#include "Common.h"
#include "misc/Rect.h"
#include "graphics/Color.h"
#include "widget/ButtonListener.h"
#include "widget/DialogListener.h"
#include "misc/Buffer.h"
#include <mutex>
#include <thread>
#include <set>
#include <string_view>
#include "graphics/SharedImage.h"
#include "misc/Ratio.h"
#include <atomic>

struct SDL_Cursor;

namespace ImageLib
{
	class Image;
};

namespace Sexy
{

class WidgetManager;
class GLInterface;
class Image;
class GLImage;
class Widget;
class SoundManager;
class MusicInterface;
class MemoryImage;
class HTTPTransfer;
class Dialog;

class ResourceManager;

class WidgetSafeDeleteInfo
{
public:
	int						mUpdateAppDepth;
	Widget*					mWidget;
};


typedef std::list<WidgetSafeDeleteInfo> WidgetSafeDeleteList;
typedef std::set<MemoryImage*> MemoryImageSet;
typedef std::map<int, Dialog*> DialogMap;
typedef std::list<Dialog*> DialogList;
//typedef std::list<MSG> WindowsMessageList;
//typedef std::basic_string<TCHAR> tstring; // string of TCHARs

typedef std::map<std::string, bool> StringBoolMap;
typedef std::map<std::string, int> StringIntMap;
typedef std::map<std::string, double> StringDoubleMap;
typedef std::map<std::string, std::vector<std::string>> StringStringVectorMap;

enum
{
	CURSOR_POINTER,
	CURSOR_HAND,
	CURSOR_DRAGGING,
	CURSOR_TEXT,
	CURSOR_CIRCLE_SLASH,
	CURSOR_SIZEALL,
	CURSOR_SIZENESW,
	CURSOR_SIZENS,
	CURSOR_SIZENWSE,
	CURSOR_SIZEWE,	
	CURSOR_WAIT,
	CURSOR_NONE,
	CURSOR_CUSTOM,
	NUM_CURSORS
};

enum
{
	DEMO_MOUSE_POSITION,	
	DEMO_ACTIVATE_APP,
	DEMO_SIZE,
	DEMO_KEY_DOWN,
	DEMO_KEY_UP,
	DEMO_KEY_CHAR,
	DEMO_CLOSE,
	DEMO_MOUSE_ENTER,
	DEMO_MOUSE_EXIT,
	DEMO_LOADING_COMPLETE,
	DEMO_REGISTRY_GETSUBKEYS,
	DEMO_REGISTRY_READ,
	DEMO_REGISTRY_WRITE,
	DEMO_REGISTRY_ERASE,	
	DEMO_FILE_EXISTS,
	DEMO_FILE_READ,
	DEMO_FILE_WRITE,
	DEMO_HTTP_RESULT,
	DEMO_SYNC,
	DEMO_ASSERT_STRING_EQUAL,
	DEMO_ASSERT_INT_EQUAL,
	DEMO_MOUSE_WHEEL,
	DEMO_HANDLE_COMPLETE,
	DEMO_VIDEO_DATA,
	DEMO_IDLE = 31
};

enum {
	FPS_ShowFPS,
	FPS_ShowCoords,
	Num_FPS_Types
};

enum
{
	UPDATESTATE_MESSAGES,
	UPDATESTATE_PROCESS_1,
	UPDATESTATE_PROCESS_2,
	UPDATESTATE_PROCESS_DONE
};

class SexyAppBase : public ButtonListener, public DialogListener
{
public:
	void*					mWindow;
	void*					mContext;
	void*					mSurface; // for EGL

	uint32_t				mRandSeed;
		
	std::string				mCompanyName;
	std::string				mFullCompanyName;
	std::string				mProdName;
	std::string				mTitle;
	std::string				mRegKey;
	std::string				mResourceDir;
	std::string				mCustomSaveDir;
	
	int						mRelaxUpdateBacklogCount; // app doesn't try to catch up for this many frames
	int						mPreferredX;
	int						mPreferredY;
	int						mWidth;
	int						mHeight;
	int						mFullscreenBits;
	double					mMusicVolume;
	double					mSfxVolume;
	double					mDemoMusicVolume;
	double					mDemoSfxVolume;
	bool					mNoSoundNeeded;
	bool					mWantFMod;
	bool					mCmdLineParsed;
	int						mArgc;
	char**					mArgv;
	bool					mSkipSignatureChecks;
	bool					mStandardWordWrap;
	bool					mbAllowExtendedChars;


	bool					mOnlyAllowOneCopyToRun;
	unsigned int			mNotifyGameMessage;
	std::mutex				mCritSect;	
	uchar					mAdd8BitMaxTable[512];
	WidgetManager*			mWidgetManager;
	DialogMap				mDialogMap;
	DialogList				mDialogList;
	std::thread::id			mPrimaryThreadId;
	bool					mSEHOccured;
	bool					mShutdown;
	bool					mExitToTop;
	bool					mIsWindowed;
	bool					mIsPhysWindowed;
	bool					mFullScreenWindow; // uses ChangeDisplaySettings to run fullscreen with mIsWindowed true
	bool					mForceFullscreen;
	bool					mForceWindowed;	
	bool					mInitialized;	
	bool					mProcessInTimer;
	uint32_t				mTimeLoaded;
	bool					mIsScreenSaver;
	bool					mAllowMonitorPowersave;
	bool					mNoDefer;	
	bool					mFullScreenPageFlip;	
	bool					mTabletPC;
	GLInterface*			mGLInterface;
	bool					mAlphaDisabled;
	MusicInterface*			mMusicInterface;	
	bool					mReadFromRegistry;
	std::string				mRegisterLink;
	std::string				mProductVersion;	
	Image*					mCursorImages[NUM_CURSORS];
	bool					mIsOpeningURL;
	bool					mShutdownOnURLOpen;
	std::string				mOpeningURL;
	uint32_t				mOpeningURLTime;
	uint32_t				mLastTimerTime;
	uint32_t				mLastBigDelayTime;	
	double					mUnmutedMusicVolume;
	double					mUnmutedSfxVolume;	
	int						mMuteCount;
	int						mAutoMuteCount;
	bool					mDemoMute;
	bool					mMuteOnLostFocus;
	MemoryImageSet			mMemoryImageSet;
	SharedImageMap			mSharedImageMap;
	std::atomic<bool>		mCleanupSharedImages;
	
	int						mNonDrawCount;
	int						mFrameTime;

	bool					mIsDrawing;
	bool					mLastDrawWasEmpty;
	bool					mHasPendingDraw;
	double					mPendingUpdatesAcc;
	double					mUpdateFTimeAcc;
	time_t					mLastTimeCheck;
	time_t					mLastTime;
	uint32_t				mLastUserInputTick;

	int						mSleepCount;
	int						mDrawCount;
	int						mUpdateCount;
	int						mUpdateAppState;
	int						mUpdateAppDepth;
	double					mUpdateMultiplier;		
	bool					mPaused;
	int						mFastForwardToUpdateNum;
	bool					mFastForwardToMarker;
	bool					mFastForwardStep;
	uint32_t				mLastDrawTick;
	uint32_t				mNextDrawTick;
	int						mStepMode;  // 0 = off, 1 = step, 2 = waiting for step

	int						mCursorNum;
	SDL_Cursor*				mSysCursors[NUM_CURSORS];
	SDL_Cursor*				mCustomCursor;
	Image*					mCustomCursorImage;
	int						mCustomCursorImageNum;
	SoundManager*			mSoundManager;
	WidgetSafeDeleteList	mSafeDeleteList;
	bool					mMouseIn;	
	bool					mRunning;
	bool					mActive;
	bool					mMinimized;
	bool					mPhysMinimized;
	bool					mIsDisabled;
	bool					mHasFocus;
	int						mDrawTime;
	uint32_t				mFPSStartTick;
	int						mFPSFlipCount;
	int						mFPSDirtyCount;
	int						mFPSTime;
	int						mFPSCount;
	bool					mShowFPS;
	int						mShowFPSMode;
	int						mScreenBltTime;
	bool					mAutoStartLoadingThread;
	bool					mLoadingThreadStarted;
	bool					mLoadingThreadCompleted;
	bool					mLoaded;
	bool					mYieldMainThread;
	bool					mLoadingFailed;
	bool					mCursorThreadRunning;
	bool					mSysCursor;	
	bool					mCustomCursorsEnabled;
	bool					mCustomCursorDirty;	
	bool					mLastShutdownWasGraceful;
	bool					mIsWideWindow;
	bool					mWriteToSexyCache;
	bool					mSexyCacheBuffers;

	int						mNumLoadingThreadTasks;
	int						mCompletedLoadingThreadTasks;

	// For recording/playback of program control
	bool					mRecordingDemoBuffer;
	bool					mPlayingDemoBuffer;
	bool					mManualShutdown;
	std::string				mDemoPrefix;
	std::string				mDemoFileName;
	Buffer					mDemoBuffer;
	int						mDemoLength;
	int						mLastDemoMouseX;
	int						mLastDemoMouseY;
	int						mLastDemoUpdateCnt;
	bool					mDemoNeedsCommand;
	bool					mDemoIsShortCmd;
	int						mDemoCmdNum;
	int						mDemoCmdOrder;
	int						mDemoCmdBitPos;
	bool					mDemoLoadingComplete;

	typedef std::pair<std::string, int> DemoMarker;
	typedef std::list<DemoMarker> DemoMarkerList;
	DemoMarkerList			mDemoMarkerList;

	bool					mDebugKeysEnabled;
	bool					mEnableMaximizeButton;
	bool					mCtrlDown;
	bool					mAltDown;
	bool					mAllowAltEnter;
	
	int						mSyncRefreshRate;
	bool					mVSyncUpdates;
	bool					mVSyncBroken;
	int						mVSyncBrokenCount;
	uint32_t				mVSyncBrokenTestStartTick;
	uint32_t				mVSyncBrokenTestUpdates;
	bool					mWaitForVSync;
	bool					mSoftVSyncWait;
	bool					mUserChanged3DSetting;
	bool					mAutoEnable3D;
	bool					mTest3D;
	uint32_t				mMinVidMemory3D;
	uint32_t				mRecommendedVidMemory3D;

	bool					mWidescreenAware;
	Rect					mScreenBounds;
	bool					mEnableWindowAspect;
	Ratio					mWindowAspect;

	std::map<std::string, std::string, std::less<>>	mStringProperties;
	StringBoolMap			mBoolProperties;
	StringIntMap			mIntProperties;
	StringDoubleMap			mDoubleProperties;
	StringStringVectorMap	mStringVectorProperties;
	ResourceManager*		mResourceManager;

protected:	
	void					RehupFocus();
	void					ClearKeysDown();
	bool					ProcessDeferredMessages(bool singleMessage);
	void					UpdateFTimeAcc();
	virtual bool			Process(bool allowSleep = true);		
	virtual void			UpdateFrames();
	virtual bool			DoUpdateFrames();
	virtual void			DoUpdateFramesF(float theFrac);
	virtual void			MakeWindow();
	virtual void			EnforceCursor();
	void					ResetCustomCursorCache();
	virtual void			ReInitImages();
	virtual void			DeleteNativeImageData();	
	virtual void			DeleteExtraImageData();
	
	// Loading thread methods	
	virtual void			LoadingThreadCompleted();
	static void				LoadingThreadProcStub(SexyAppBase *theArg);	

	// Cursor thread methods
	void					CursorThreadProc();
	static void				CursorThreadProcStub(void *theArg);
	void					StartCursorThread();
	
	void					WaitForLoadingThread();				
	void					ProcessSafeDeleteList();
#ifdef __EMSCRIPTEN__
	static void				EmscriptenMainLoopCallback();
#endif
	void					RestoreScreenResolution();
	void					DoExit(int theCode);

	void					ShowMemoryUsage();			

	// Registry helpers
	bool					RegistryRead(const std::string& theValueName, uint32_t* theType, uchar* theValue, uint32_t* theLength);
	bool					RegistryReadKey(const std::string& theValueName, uint32_t* theType, uchar* theValue, uint32_t* theLength);
	bool					RegistryWrite(const std::string& theValueName, uint32_t theType, const uchar* theValue, uint32_t theLength);

	// Demo recording helpers	
	void					ProcessDemo();

public:
	SexyAppBase();
	virtual ~SexyAppBase();

	// Common overrides:
	virtual MusicInterface*	CreateMusicInterface();
	virtual void			InitHook();
	virtual void			ShutdownHook();
	virtual void			PreTerminate();
	virtual void			LoadingThreadProc();
	virtual void			WriteToRegistry();
	virtual void			ReadFromRegistry();
	virtual Dialog*			NewDialog(int theDialogId, bool isModal, const std::string& theDialogHeader, const std::string& theDialogLines, const std::string& theDialogFooter, int theButtonMode);
	virtual void			PreDisplayHook();

	// Public methods
	virtual void			BeginPopup();
	virtual void			EndPopup();
	virtual int				MsgBox(const std::string &theText, const std::string &theTitle = "Message", int theFlags = 0);
	virtual void			Popup(const std::string& theString);
	virtual void			LogScreenSaverError(const std::string &theError);
	virtual void			SafeDeleteWidget(Widget* theWidget);	

	virtual void			URLOpenFailed(const std::string& theURL);
	virtual void			URLOpenSucceeded(const std::string& theURL);
	virtual bool			OpenURL(const std::string& theURL, bool shutdownOnOpen = false);	
	virtual std::string		GetProductVersion(const std::string& thePath);	

	virtual void			SEHOccured();
	virtual std::string		GetGameSEHInfo();
	virtual void			GetSEHWebParams(DefinesMap* theDefinesMap);
	virtual void			Shutdown();	

	virtual void			DoParseCmdLine();
	void					SetArgs(int argc, char** argv);
	virtual void			ParseCmdLine(const std::string& theCmdLine);
	virtual void			HandleCmdLineParam(const std::string& theParamName, const std::string& theParamValue);
	virtual void			HandleNotifyGameMessage(int theType); // for HWND_BROADCAST of mNotifyGameMessage (0-1000 are reserved for SexyAppBase for theType)
	virtual void			HandleGameAlreadyRunning(); 

	virtual void			Start();	
	virtual void			Init();
	virtual void			PreGLInterfaceInitHook();
	virtual void			PostGLInterfaceInitHook();
	virtual bool			ChangeDirHook(const char *theIntendedPath);
	virtual void			PlaySample(intptr_t theSoundNum);
	virtual void			PlaySample(intptr_t theSoundNum, int thePan);

	virtual double			GetMasterVolume();
	virtual double			GetMusicVolume();
	virtual double			GetSfxVolume();
	virtual bool			IsMuted();

	virtual void			SetMasterVolume(double theVolume);
	virtual void			SetMusicVolume(double theVolume);
	virtual void			SetSfxVolume(double theVolume);	
	virtual void			Mute(bool autoMute = false);
	virtual void			Unmute(bool autoMute = false);

	void					StartLoadingThread();
	virtual double			GetLoadingThreadProgress();	

	void					CopyToClipboard(const std::string& theString);
	std::string				GetClipboard();

	void					SetCursor(int theCursorNum);
	int						GetCursor();
	void					EnableCustomCursors(bool enabled);	
	virtual GLImage*		GetImage(const std::string& theFileName, bool commitBits = true);	
	virtual SharedImageRef	SetSharedImage(const std::string& theFileName, const std::string& theVariant, GLImage* theImage, bool* isNew);
	virtual SharedImageRef	GetSharedImage(const std::string& theFileName, const std::string& theVariant = "", bool* isNew = nullptr);

	void					CleanSharedImages();
	void					PrecacheAdditive(MemoryImage* theImage);
	void					PrecacheAlpha(MemoryImage* theImage);
	void					PrecacheNative(MemoryImage* theImage);
	void					SetCursorImage(int theCursorNum, Image* theImage);

	GLImage*				CreateCrossfadeImage(Image* theImage1, const Rect& theRect1, Image* theImage2, const Rect& theRect2, double theFadeFactor);
	void					ColorizeImage(Image* theImage, const Color& theColor);
	GLImage*				CreateColorizedImage(Image* theImage, const Color& theColor);
	GLImage*				CopyImage(Image* theImage, const Rect& theRect);
	GLImage*				CopyImage(Image* theImage);
	void					MirrorImage(Image* theImage);
	void					FlipImage(Image* theImage);
	void					RotateImageHue(Sexy::MemoryImage *theImage, int theDelta);
	uint32_t				HSLToRGB(int h, int s, int l);
	uint32_t				RGBToHSL(int r, int g, int b);
	void					HSLToRGB(const uint32_t* theSource, uint32_t* theDest, int theSize);
	void					RGBToHSL(const uint32_t* theSource, uint32_t* theDest, int theSize);

	void					AddMemoryImage(MemoryImage* theMemoryImage);
	void					RemoveMemoryImage(MemoryImage* theMemoryImage);
	void					Remove3DData(MemoryImage* theMemoryImage);
	virtual void			SwitchScreenMode();
	virtual void			SwitchScreenMode(bool wantWindowed);
	virtual void			SwitchScreenMode(bool wantWindowed, bool is3d, bool force = false);
	virtual void			SetAlphaDisabled(bool isDisabled);
	
	virtual Dialog*			DoDialog(int theDialogId, bool isModal, const std::string& theDialogHeader, const std::string& theDialogLines, const std::string& theDialogFooter, int theButtonMode);
	virtual Dialog*			GetDialog(int theDialogId);
	virtual void			AddDialog(int theDialogId, Dialog* theDialog);
	virtual void			AddDialog(Dialog* theDialog);
	virtual bool			KillDialog(int theDialogId, bool removeWidget, bool deleteWidget);
	virtual bool			KillDialog(int theDialogId);
	virtual bool			KillDialog(Dialog* theDialog);
	virtual int				GetDialogCount();
	virtual void			ModalOpen();
	virtual void			ModalClose();	
	void					DialogButtonPress(int theDialogId, int theButtonId) override;
	void					DialogButtonDepress(int theDialogId, int theButtonId) override;

	virtual void			GotFocus();
	virtual void			LostFocus();
	virtual bool			DebugKeyDown(int theKey);
//	virtual bool			DebugKeyDownAsync(int theKey, bool ctrlDown, bool altDown);
	virtual void			CloseRequestAsync();
	void					InitInput();
	bool					StartTextInput(std::string& theInput); // set theInput and return true if using soft keyboard capability and user pressed OK (e.g. Switch libnx swkbd)
	void					StopTextInput();
	bool					Is3DAccelerated();
	bool					Is3DAccelerationSupported();
	bool					Is3DAccelerationRecommended();
	void					DemoSyncRefreshRate();
	void					Set3DAcclerated(bool is3D, bool reinit = true);
	virtual void			Done3dTesting();
	virtual std::string		NotifyCrashHook(); // return file name that you want to upload
	
//	virtual bool			CheckSignature(const Buffer& theBuffer, const std::string& theFileName);
	virtual bool			DrawDirtyStuff();
	virtual void			Redraw(Rect* theClipRect);

	// Properties access methods
	bool					LoadProperties(const std::string& theFileName, bool required, bool checkSig);
	bool					LoadProperties();
	virtual void			InitPropertiesHook();

	// Resource access methods
	void					LoadResourceManifest();
	void					ShowResourceError(bool doExit = false);
	
	bool					GetBoolean(const std::string& theId);
	bool					GetBoolean(const std::string& theId, bool theDefault);	
	int						GetInteger(const std::string& theId);
	int						GetInteger(const std::string& theId, int theDefault);
	double					GetDouble(const std::string& theId);
	double					GetDouble(const std::string& theId, double theDefault);
	std::string				GetString(std::string_view theId);
	std::string				GetString(std::string_view theId, std::string_view theDefault);

	std::vector<std::string>	GetStringVector(const std::string& theId);

	void					SetBoolean(const std::string& theId, bool theValue);
	void					SetInteger(const std::string& theId, int theValue);
	void					SetDouble(const std::string& theId, double theValue);
	void					SetString(const std::string& theId, const std::string& theValue);
	
	// Demo access methods
	bool					PrepareDemoCommand(bool required);
	void					WriteDemoTimingBlock();
	void					WriteDemoBuffer();
	bool					ReadDemoBuffer(std::string &theError); //UNICODE
	void					DemoSyncBuffer(Buffer* theBuffer);
	void					DemoSyncString(std::string* theString);
	void					DemoSyncInt(int* theInt);
	void					DemoSyncBool(bool* theBool);
	void					DemoAssertStringEqual(const std::string& theString);
	void					DemoAssertIntEqual(int theInt);
	void					DemoAddMarker(const std::string& theString);

	

	// Registry access methods
	//bool					RegistryGetSubKeys(const std::string& theKeyName, std::vector<std::string>* theSubKeys);
	bool					RegistryReadString(const std::string& theValueName, std::string* theString);
	bool					RegistryReadInteger(const std::string& theValueName, int* theValue);
	bool					RegistryReadBoolean(const std::string& theValueName, bool* theValue);
	bool					RegistryReadData(const std::string& theValueName, uchar* theValue, uint32_t* theLength);
	bool					RegistryWriteString(const std::string& theValueName, const std::string& theString);
	bool					RegistryWriteInteger(const std::string& theValueName, int theValue);
	bool					RegistryWriteBoolean(const std::string& theValueName, bool theValue);
	bool					RegistryWriteData(const std::string& theValueName, const uchar* theValue, uint32_t theLength);	
	bool					RegistryEraseKey(const std::string& theKeyName);
	void					RegistryEraseValue(const std::string& theValueName);

	// File access methods
	bool					WriteBufferToFile(const std::string& theFileName, const Buffer* theBuffer);
	bool					ReadBufferFromFile(const std::string& theFileName, Buffer* theBuffer, bool dontWriteToDemo = false);//UNICODE
	bool					ReadUTF8StringFromFile(const std::string& theFileName, std::string* theString);
	bool					WriteBytesToFile(const std::string& theFileName, const void *theData, unsigned long theDataLen);
	bool					FileExists(const std::string& theFileName);
	bool					EraseFile(const std::string& theFileName);

	// Misc methods
	virtual void			DoMainLoop();
	virtual bool			UpdateAppStep(bool* updated);
	virtual bool			UpdateApp();
	int						InitGLInterface();
	void					ClearUpdateBacklog(bool relaxForASecond = false);
	bool					IsScreenSaver();
	virtual bool			AppCanRestore();
};

extern SexyAppBase* gSexyAppBase;

};

#endif //__SEXYAPPBASE_H__
