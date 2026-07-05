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

#ifndef __LAWNAPP_H__
#define __LAWNAPP_H__

#include "ConstEnums.h"
#include "SexyAppFramework/SexyApp.h"
#include "Sexy.TodLib/TodFoley.h"

class Board;
class GameSelector;
class ChallengeDefinition;
class SeedChooserScreen;
class AwardScreen;
class CreditScreen;
class TodFoley;
class PoolEffect;
class ZenGarden;
class PottedPlant;
class EffectSystem;
class TodParticleSystem;
class Reanimation;
class ReanimatorCache;
class ProfileMgr;
class PlayerInfo;
class Music;
class TitleScreen;
class ChallengeScreen;
class StoreScreen;
class AlmanacDialog;
class TypingCheck;

namespace Sexy
{
	class Dialog;
	class Graphics;
	class ButtonWidget;
};

using namespace Sexy;

typedef std::list<ButtonWidget*> ButtonList;
typedef std::list<Image*> ImageList;

class LevelStats
{
public:
	int								mUnusedLawnMowers;

public:
	LevelStats() { Reset(); }
	inline void						Reset() { mUnusedLawnMowers = 0; }
};

class LawnApp : public SexyApp
{
public:
	Board*							mBoard;
	TitleScreen*					mTitleScreen;
	GameSelector*					mGameSelector;
	SeedChooserScreen*				mSeedChooserScreen;
	AwardScreen*					mAwardScreen;
	CreditScreen*					mCreditScreen;
	ChallengeScreen*				mChallengeScreen;
	TodFoley*						mSoundSystem;
	ButtonList						mControlButtonList;
	ImageList						mCreatedImageList;
	std::string						mReferId;
	std::string						mRegisterLink;
	std::string						mMod;
	bool							mRegisterResourcesLoaded;
	bool							mTodCheatKeys;
	GameMode						mGameMode;
	GameScenes						mGameScene;
	bool							mLoadingZombiesThreadCompleted;
	bool							mFirstTimeGameSelector;
	int								mGamesPlayed;
	int								mMaxExecutions;
	int								mMaxPlays;
	int								mMaxTime;
	bool							mEasyPlantingCheat;
	PoolEffect*						mPoolEffect;
	ZenGarden*						mZenGarden;
	EffectSystem*					mEffectSystem;
	ReanimatorCache*				mReanimatorCache;
	ProfileMgr*						mProfileMgr;
	PlayerInfo*						mPlayerInfo;
	LevelStats*						mLastLevelStats;
	bool							mCloseRequest;
	uint32_t						mAppCounter;
	Music*							mMusic;
	ReanimationID					mCrazyDaveReanimID;
	CrazyDaveState					mCrazyDaveState;
	int								mCrazyDaveBlinkCounter;
	ReanimationID					mCrazyDaveBlinkReanimID;
	int								mCrazyDaveMessageIndex;
	std::string						mCrazyDaveMessageText;
	int								mAppRandSeed;
	//HICON							mBigArrowCursor;
	intptr_t						mSessionID;
	int								mPlayTimeActiveSession;
	int								mPlayTimeInactiveSession;
	BoardResult						mBoardResult;
	bool							mSawYeti;
	TypingCheck*					mKonamiCheck;
	TypingCheck*					mMustacheCheck;
	TypingCheck*					mMoustacheCheck;
	TypingCheck*					mSuperMowerCheck;
	TypingCheck*					mSuperMowerCheck2;
	TypingCheck*					mFutureCheck;
	TypingCheck*					mPinataCheck;
	TypingCheck*					mDanceCheck;
	TypingCheck*					mDaisyCheck;
	TypingCheck*					mSukhbirCheck;
	bool							mMustacheMode;
	bool							mSuperMowerMode;
	bool							mFutureMode;
	bool							mPinataMode;
	bool							mDanceMode;
	bool							mDaisyMode;
	bool							mSukhbirMode;
	TrialType						mTrialType;
	bool							mDebugTrialLocked;
	bool							mMuteSoundsForCutscene;

public:
	LawnApp();
	~LawnApp() override;

	bool							KillNewOptionsDialog();
	void							GotFocus() override;
	void							LostFocus() override;
	void							InitHook() override;
	void							WriteToRegistry() override;
	void							ReadFromRegistry() override;
	void							LoadingThreadProc() override;
	virtual void					LoadingCompleted();
	void							LoadingThreadCompleted() override;
	void							URLOpenFailed(const std::string& theURL) override;
	void							URLOpenSucceeded(const std::string& theURL) override;
	bool							OpenURL(const std::string& theURL, bool shutdownOnOpen) override;
	bool							DebugKeyDown(int theKey) override;
	void							HandleCmdLineParam(const std::string& theParamName, const std::string& theParamValue) override;
	void							ConfirmQuit();
	void							ConfirmCheckForUpdates() { ; }
	void							CheckForUpdates() { ; }
	void							DoUserDialog();
	void							FinishUserDialog(bool isYes);
	void							DoCreateUserDialog();
	void							DoCheatDialog();
	void							FinishCheatDialog(bool isYes);
	void							FinishCreateUserDialog(bool isYes);
	void							DoConfirmDeleteUserDialog(const std::string& theName);
	void							FinishConfirmDeleteUserDialog(bool isYes);
	void							DoRenameUserDialog(const std::string& theName);
	void							FinishRenameUserDialog(bool isYes);
	void							FinishNameError(int theId);
	void							FinishRestartConfirmDialog();
	void							DoConfirmSellDialog(const std::string& theMessage);
	void							FinishTimesUpDialog();
	void							KillBoard();
	void							MakeNewBoard();
	void							StartPlaying();
	bool							TryLoadGame();
	void							NewGame();
	void							PreNewGame(GameMode theGameMode, bool theLookForSavedGame);
	void							ShowGameSelector();
	void							KillGameSelector();
	void							ShowAwardScreen(AwardType theAwardType, bool theShowAchievements); // @Patoke: add argument
	void							KillAwardScreen();
	void							ShowSeedChooserScreen();
	void							KillSeedChooserScreen();
	void							DoHighScoreDialog();
	void							DoBackToMain();
	void							DoConfirmBackToMain();
	void							DoNewOptions(bool theFromGameSelector);
	void							DoRegister();
	void							DoRegisterError();
	bool							CanDoRegisterDialog();
	/*inline*/ bool					WriteCurrentUserConfig();
	void							DoNeedRegisterDialog();
	void							DoContinueDialog();
	void							DoPauseDialog();
	void							FinishModelessDialogs();
	Dialog*							DoDialog(int theDialogId, bool isModal, const std::string& theDialogHeader, const std::string& theDialogLines, const std::string& theDialogFooter, int theButtonMode) override;
	virtual Dialog*					DoDialogDelay(int theDialogId, bool isModal, const std::string& theDialogHeader, const std::string& theDialogLines, const std::string& theDialogFooter, int theButtonMode);
	void							Shutdown() override;
	void							ShutdownHook() override;
	void							Init() override;
	void							Start() override;
	Dialog*							NewDialog(int theDialogId, bool isModal, const std::string& theDialogHeader, const std::string& theDialogLines, const std::string& theDialogFooter, int theButtonMode) override;
	bool							KillDialog(int theDialogId) override;
	void							ModalOpen() override;
	void							ModalClose() override;
	void							PreDisplayHook() override;
	bool							ChangeDirHook(const char* theIntendedPath) override;
	virtual bool					NeedRegister();
	virtual void					UpdateRegisterInfo();
	void							ButtonPress(int theId) override;
	void							ButtonDepress(int theId) override;
	void							ButtonDownTick(int theId) override;
	void							ButtonMouseEnter(int theId) override;
	void							ButtonMouseLeave(int theId) override;
	void							ButtonMouseMove(int theId, int theX, int theY) override;
	void							UpdateFrames() override;
	bool							UpdateAppStep(bool* updated) override;
	bool							UpdateApp() override;
	/*inline*/ bool					IsAdventureMode();
	/*inline*/ bool					IsSurvivalMode();
	bool							IsContinuousChallenge();
	/*inline*/ bool					IsArtChallenge();
	bool							NeedPauseGame();
	virtual void					ShowResourceError(bool doExit = false);
	void							ToggleSlowMo();
	void							ToggleFastMo();
	void							PlayFoley(FoleyType theFoleyType);
	void							PlayFoleyPitch(FoleyType theFoleyType, float thePitch);
	void							PlaySample(intptr_t theSoundNum) override;
	void							FastLoad(GameMode theGameMode);
	static std::string				GetStageString(int theLevel);
	/*inline*/ void					KillChallengeScreen();
	void							ShowChallengeScreen(ChallengePage thePage);
	const ChallengeDefinition&			GetCurrentChallengeDef();
	void							CheckForGameEnd();
	void							CloseRequestAsync() override;
	/*inline*/ bool					IsChallengeWithoutSeedBank();
	AlmanacDialog*					DoAlmanacDialog(SeedType theSeedType = SeedType::SEED_NONE, ZombieType theZombieType = ZombieType::ZOMBIE_INVALID);
	bool							KillAlmanacDialog();
	int								GetSeedsAvailable();
	Reanimation*					AddReanimation(float theX, float theY, int theRenderOrder, ReanimationType theReanimationType);
	TodParticleSystem*				AddTodParticle(float theX, float theY, int theRenderOrder, ParticleEffect theEffect);
	/*inline*/ ParticleSystemID		ParticleGetID(TodParticleSystem* theParticle);
	/*inline*/ TodParticleSystem*	ParticleGet(ParticleSystemID theParticleID);
	/*inline*/ TodParticleSystem*	ParticleTryToGet(ParticleSystemID theParticleID);
	/*inline*/ ReanimationID		ReanimationGetID(Reanimation* theReanimation);
	/*inline*/ Reanimation*			ReanimationGet(ReanimationID theReanimationID);
	/*inline*/ Reanimation*			ReanimationTryToGet(ReanimationID theReanimationID);
	void							RemoveReanimation(ReanimationID theReanimationID);
	void							RemoveParticle(ParticleSystemID theParticleID);
	StoreScreen*					ShowStoreScreen();
	void							KillStoreScreen();
	bool							HasSeedType(SeedType theSeedType);
	/*inline*/ void					EndLevel();
	inline bool						IsIceDemo() { return false; }
	/*inline*/ bool					IsShovelLevel();
	/*inline*/ bool					IsWallnutBowlingLevel();
	/*inline*/ bool					IsMiniBossLevel();
	/*inline*/ bool					IsSlotMachineLevel();
	/*inline*/ bool					IsLittleTroubleLevel();
	/*inline*/ bool					IsStormyNightLevel();
	/*inline*/ bool					IsFinalBossLevel();
	/*inline*/ bool					IsBungeeBlitzLevel();
	static /*inline*/ SeedType		GetAwardSeedForLevel(int theLevel);
	std::string						GetCrazyDaveText(int theMessageIndex);
	/*inline*/ bool					CanShowAlmanac();
	/*inline*/ bool					IsNight();
	/*inline*/ bool					CanShowStore();
	/*inline*/ bool					HasBeatenChallenge(GameMode theGameMode);
	PottedPlant*					GetPottedPlantByIndex(int thePottedPlantIndex);
	static /*inline*/ bool			IsSurvivalNormal(GameMode theGameMode);
	static /*inline*/ bool			IsSurvivalHard(GameMode theGameMode);
	static /*inline*/ bool			IsSurvivalEndless(GameMode theGameMode);
	/*inline*/ bool					HasFinishedAdventure();
	/*inline*/ bool					IsFirstTimeAdventureMode();
	/*inline*/ bool					CanSpawnYetis();
	void							CrazyDaveEnter();
	void							UpdateCrazyDave();
	void							CrazyDaveTalkIndex(int theMessageIndex);
	void							CrazyDaveTalkMessage(const std::string& theMessage);
	void							CrazyDaveLeave();
	void							DrawCrazyDave(Graphics* g);
	void							CrazyDaveDie();
	void							CrazyDaveStopTalking();
	void							PreloadForUser();
	int								GetNumPreloadingTasks();
	int								LawnMessageBox(int theDialogId, const char* theHeaderName, const char* theLinesName, const char* theButton1Name, const char* theButton2Name, int theButtonMode);
	void							ShowCreditScreen();
	void							KillCreditScreen();
	static std::string				Pluralize(int theCount, const char* theSingular, const char* thePlural);
	int								GetNumTrophies(ChallengePage thePage);
	/*inline*/ bool					EarnedGoldTrophy();
	inline bool						IsRegistered() { return false; }
	inline bool						IsExpired() { return false; }
	inline bool						IsDRMConnected() { return false; }
	/*inline*/ bool					IsScaryPotterLevel();
	static /*inline*/ bool			IsEndlessScaryPotter(GameMode theGameMode);
	/*inline*/ bool					IsSquirrelLevel();
	/*inline*/ bool					IsIZombieLevel();
	/*inline*/ bool					CanShowZenGarden();
	static std::string				GetMoneyString(int theAmount);
	bool							AdvanceCrazyDaveText();
	/*inline*/ bool					IsWhackAZombieLevel();
	void							UpdatePlayTimeStats();
	void							BetaAddFile(std::list<std::string>& theUploadFileList, std::string theFileName, std::string theShortName);
	bool							CanPauseNow();
	/*inline*/ bool					IsPuzzleMode();
	/*inline*/ bool					IsChallengeMode();
	static /*inline*/ bool			IsEndlessIZombie(GameMode theGameMode);
	void							CrazyDaveDoneHanding();
	inline std::string				GetCurrentLevelName() { return "Unknown"; }
	/*inline*/ int					TrophiesNeedForGoldSunflower();
	/*inline*/ int					GetCurrentChallengeIndex();
	void							LoadGroup(const char* theGroupName, int theGroupAveMsToLoad);
//	void							TraceLoadGroup(const char* theGroupName, int theGroupTime, int theTotalGroupWeigth, int theTaskWeight);
	void							CrazyDaveStopSound();
	/*inline*/ bool					IsTrialStageLocked();
	/*inline*/ void					FinishZenGardenToturial();
	bool							UpdatePlayerProfileForFinishingLevel();
	bool							SaveFileExists();
	/*inline*/ bool					CanDoPinataMode();
	/*inline*/ bool					CanDoDanceMode();
	/*inline*/ bool					CanDoDaisyMode();
	void							SwitchScreenMode(bool wantWindowed, bool is3d, bool force = false) override;
	static /*inline*/ void			CenterDialog(Dialog* theDialog, int theWidth, int theHeight);
};

std::string							LawnGetCurrentLevelName();
bool								LawnGetCloseRequest();
bool								LawnHasUsedCheatKeys();
void								BetaSubmitFunc();

extern bool (*gAppCloseRequest)();
extern bool (*gAppHasUsedCheatKeys)();
extern std::string (*gGetCurrentLevelName)();

extern bool gIsPartnerBuild;
extern bool gFastMo;
extern bool gSlowMo;
extern LawnApp* gLawnApp;
extern int gSlowMoCounter;


#endif	// __LAWNAPP_H__
