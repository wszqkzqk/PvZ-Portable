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

#include "Board.h"
#include "Challenge.h"
#include "../LawnApp.h"
#include "../Resources.h"
#include "MessageWidget.h"
#include "graphics/Font.h"
#include "../PvzpLib/PvzpCommon.h"
#include "../PvzpLib/Reanimator.h"
#include "../PvzpLib/PvzpStringFile.h"
#include <algorithm>
#include <SDL.h>

MessageWidget::MessageWidget(LawnApp* theApp)
{
	mApp = theApp;
	mDuration = 0;
	mDisplayTime = 0;
	mLabel[0] = '\0';
	mMessageStyle = MessageStyle::MESSAGE_STYLE_OFF;
	mLabelNext[0] = '\0';
	mMessageStyleNext = MessageStyle::MESSAGE_STYLE_OFF;
	mSlideOffTime = 100;
	mReanimType = ReanimationType::REANIM_NONE;
	mTextReanimCount = 0;
	for (int i = 0; i < MAX_MESSAGE_LENGTH; i++)
	{
		mTextReanimID[i] = ReanimationID::REANIMATIONID_NULL;
		mTextReanimByteOffset[i] = 0;
	}
}

void MessageWidget::ClearReanim()
{
	for (int i = 0; i < MAX_MESSAGE_LENGTH; i++)
	{
		Reanimation* aReanim = mApp->ReanimationTryToGet(mTextReanimID[i]);
		if (aReanim)
		{
			aReanim->ReanimationDie();
			mTextReanimID[i] = ReanimationID::REANIMATIONID_NULL;
		}
	}
}

void MessageWidget::ClearLabel()
{
	if (mReanimType != ReanimationType::REANIM_NONE)
	{
		mDuration = std::min(mDuration, 100 + mSlideOffTime + 1);
	}
	else
	{
		mDuration = 0;
	}
}

// Enforce the label limits (buffer size, line count) in this single place;
// the rest of the class relies on them.
static void TruncateLabel(std::string& theLabel)
{
	size_t aBytePos = 0;
	int aLineCount = 1;
	while (aBytePos < theLabel.size())
	{
		size_t aNext = aBytePos;
		char32_t aChar;
		if (!UTF8DecodeNext(theLabel, aNext, aChar))
		{
			theLabel.resize(aBytePos); // drop everything from the invalid byte on
			return;
		}
		if (aNext > MAX_MESSAGE_LENGTH - 1 || (aChar == U'\n' && aLineCount == MAX_REANIM_LINES))
		{
			theLabel.resize(aBytePos);
			return;
		}
		if (aChar == U'\n')
			aLineCount++;
		aBytePos = aNext;
	}
}

void MessageWidget::SetLabel(std::string_view theNewLabel, MessageStyle theMessageStyle)
{
	std::string aLabel(PvzpStringTranslate(theNewLabel));
	TruncateLabel(aLabel);

	if (mReanimType != ReanimationType::REANIM_NONE && mDuration > 0)
	{
		mMessageStyleNext = theMessageStyle;
		SDL_strlcpy(mLabelNext, aLabel.c_str(), sizeof(mLabelNext));
		ClearLabel();
	}
	else
	{
		ClearReanim();
		SDL_strlcpy(mLabel, aLabel.c_str(), sizeof(mLabel));
		mMessageStyle = theMessageStyle;
		mReanimType = ReanimationType::REANIM_NONE;

		switch (theMessageStyle)
		{
		case MessageStyle::MESSAGE_STYLE_HINT_LONG:
		case MessageStyle::MESSAGE_STYLE_BIG_MIDDLE:
		case MessageStyle::MESSAGE_STYLE_ZEN_GARDEN_LONG:
		case MessageStyle::MESSAGE_STYLE_HINT_TALL_LONG:
			mDuration = 1500;
			break;

		case MessageStyle::MESSAGE_STYLE_HINT_TALL_UNLOCKMESSAGE:
			mDuration = 500;
			break;

		case MessageStyle::MESSAGE_STYLE_HINT_FAST:
		case MessageStyle::MESSAGE_STYLE_HINT_TALL_FAST:
		case MessageStyle::MESSAGE_STYLE_BIG_MIDDLE_FAST:
		case MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1:
		case MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL2:
		case MessageStyle::MESSAGE_STYLE_TUTORIAL_LATER:
			mDuration = 500;
			break;

		case MessageStyle::MESSAGE_STYLE_HINT_STAY:
		case MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY:
		case MessageStyle::MESSAGE_STYLE_TUTORIAL_LATER_STAY:
			mDuration = 10000;
			break;

		case MessageStyle::MESSAGE_STYLE_HOUSE_NAME:
			mDuration = 250;
			break;

		case MessageStyle::MESSAGE_STYLE_HUGE_WAVE:
			mDuration = 750;
			mReanimType = ReanimationType::REANIM_TEXT_FADE_ON;
			break;

		case MessageStyle::MESSAGE_STYLE_SLOT_MACHINE:
			mDuration = 750;
			break;

		case MessageStyle::MESSAGE_STYLE_ACHIEVEMENT:
			mDuration = 250;
			break;

		default:
			PVZP_ASSERT(false);
			break;
		}

		if (mReanimType != ReanimationType::REANIM_NONE)
		{
			LayoutReanimText();
		}
		mDisplayTime = mDuration;
	}
}

void MessageWidget::LayoutReanimText()
{
	float aMaxWidth = 0;
	int aCurLine = 0, aCurPos = 0;
	_Font* aFont = GetFont();
	int aLabelLen = strlen(mLabel);
	mSlideOffTime = aLabelLen + 100;

	float aLineWidth[MAX_REANIM_LINES];
	for (int aPos = 0; aPos <= aLabelLen; aPos++)
	{
		if (aPos == aLabelLen || mLabel[aPos] == '\n')
		{
			PVZP_ASSERT(aCurLine < MAX_REANIM_LINES);

			int aLen = aPos - aCurPos;
			int aOff = aCurPos;
			aCurPos = aPos + 1;
			std::string aLine(&mLabel[aOff], aLen);

			aLineWidth[aCurLine] = aFont->StringWidth(aLine);
			aMaxWidth = std::max(aMaxWidth, aLineWidth[aCurLine]);
			aCurLine++;
		}
	}

	aCurLine = 0;
	float aCurPosY = 0.0f;
	float aCurPosX = -aLineWidth[0] * 0.5f;
	// Iterate by code point so each reanimated glyph corresponds to one UTF-8 character.
	int aCharIdx = 0;
	size_t aBytePos = 0;
	while (aBytePos < (size_t)aLabelLen)
	{
		const size_t aCharStart = aBytePos;
		char32_t aChar = 0;
		if (!UTF8DecodeNext(mLabel, aBytePos, aChar))
			break;

		Reanimation* aReanimText = mApp->AddReanimation(aCurPosX, aCurPosY, 0, mReanimType);
		aReanimText->mIsAttachment = true;
		aReanimText->PlayReanim("anim_enter", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0.0f, 0.0f);
		mTextReanimID[aCharIdx] = mApp->ReanimationGetID(aReanimText);
		mTextReanimByteOffset[aCharIdx] = aCharStart;

		aCurPosX += aFont->CharWidth(aChar);
		if (aChar == U'\n')
		{
			aCurLine++;
			PVZP_ASSERT(aCurLine < MAX_REANIM_LINES);
			aCurPosX = -aLineWidth[aCurLine] * 0.5f;
			aCurPosY += aFont->GetLineSpacing();
		}
		aCharIdx++;
	}
	mTextReanimCount = aCharIdx;
}

void MessageWidget::Update()
{
	if (!mApp->mBoard || mApp->mBoard->mPaused)
		return;

	// count down the remaining time and switch to the next message
	if (mDuration < 10000 && mDuration > 0)
	{
		mDuration--;
		if (mDuration == 0)
		{
			mMessageStyle = MessageStyle::MESSAGE_STYLE_OFF;
			if (mMessageStyleNext != MessageStyle::MESSAGE_STYLE_OFF)
			{
				SetLabel(mLabelNext, mMessageStyleNext);
				mMessageStyleNext = MessageStyle::MESSAGE_STYLE_OFF;
			}
		}
	}

	// Iterate reanimated glyphs by code-point index.
	for (int aCharIdx = 0; aCharIdx < mTextReanimCount; aCharIdx++)
	{
		Reanimation* aTextReanim = mApp->ReanimationTryToGet(mTextReanimID[aCharIdx]);
		if (aTextReanim == nullptr)
		{
			break;
		}

		int aTextSpeed = mReanimType == ReanimationType::REANIM_TEXT_FADE_ON ? 100 : 1;
		if (mDuration > mSlideOffTime)
		{
			if (mReanimType == ReanimationType::REANIM_TEXT_FADE_ON)
			{
				aTextReanim->mAnimRate = 60.0f;
			}
			else
			{
				aTextReanim->mAnimRate = PvzpAnimateCurveFloat(0, 50, (mDisplayTime - mDuration) * aTextSpeed - aCharIdx, 0.0f, 40.0f, PvzpCurves::CURVE_LINEAR);
			}
		}
		else
		{
			if (mDuration == mSlideOffTime)
			{
				aTextReanim->PlayReanim("anim_leave", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 0.0f);
			}
			aTextReanim->mAnimRate = PvzpAnimateCurveFloat(0, 50, (mSlideOffTime - mDuration) * aTextSpeed - aCharIdx, 0.0f, 40.0f, PvzpCurves::CURVE_LINEAR);
		}

		aTextReanim->Update();
	}
}

void MessageWidget::DrawReanimatedText(Graphics* g, _Font* theFont, const Color& theColor, float thePosY)
{
	for (int aCharIdx = 0; aCharIdx < mTextReanimCount; aCharIdx++)
	{
		Reanimation* aTextReanim = mApp->ReanimationTryToGet(mTextReanimID[aCharIdx]);
		if (aTextReanim == nullptr)
		{
			break;
		}

		ReanimatorTransform aTransform;
		aTextReanim->GetCurrentTransform(2, &aTransform);

		int anAlpha = std::clamp(FloatRoundToInt(theColor.mAlpha * aTransform.mAlpha), 0, 255);
		if (anAlpha <= 0)
		{
			break;
		}
		Color aFinalColor(theColor);
		aFinalColor.mAlpha = anAlpha;

		aTransform.mTransX += aTextReanim->mOverlayMatrix.m02;
		aTransform.mTransY += aTextReanim->mOverlayMatrix.m12 + thePosY - BOARD_HEIGHT / 2;
		if (mReanimType == ReanimationType::REANIM_TEXT_FADE_ON && mDisplayTime - mDuration < mSlideOffTime)
		{
			float aStretch = 1.0f - aTextReanim->mAnimTime;
			aTransform.mTransX += aStretch * aTextReanim->mOverlayMatrix.m02;
		}

		SexyMatrix3 aMatrix;
		Reanimation::MatrixFromTransform(aTransform, aMatrix);
		const int aByteStart = mTextReanimByteOffset[aCharIdx];
		const int aByteEnd = (aCharIdx + 1 < mTextReanimCount) ? mTextReanimByteOffset[aCharIdx + 1] : strlen(mLabel);
		std::string aLetter(&mLabel[aByteStart], aByteEnd - aByteStart);
		PvzpDrawStringMatrix(g, theFont, aMatrix, aLetter, aFinalColor);
	}
}

_Font* MessageWidget::GetFont()
{
	switch (mMessageStyle)
	{
	case MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1:
	case MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY:
	case MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL2:
	case MessageStyle::MESSAGE_STYLE_TUTORIAL_LATER:
	case MessageStyle::MESSAGE_STYLE_TUTORIAL_LATER_STAY:
	case MessageStyle::MESSAGE_STYLE_HINT_LONG:
	case MessageStyle::MESSAGE_STYLE_HINT_FAST:
	case MessageStyle::MESSAGE_STYLE_HINT_STAY:
	case MessageStyle::MESSAGE_STYLE_HINT_TALL_FAST:
	case MessageStyle::MESSAGE_STYLE_HINT_TALL_UNLOCKMESSAGE:
	case MessageStyle::MESSAGE_STYLE_HINT_TALL_LONG:
	case MessageStyle::MESSAGE_STYLE_BIG_MIDDLE:
	case MessageStyle::MESSAGE_STYLE_BIG_MIDDLE_FAST:
	case MessageStyle::MESSAGE_STYLE_HOUSE_NAME:
	case MessageStyle::MESSAGE_STYLE_HUGE_WAVE:
	case MessageStyle::MESSAGE_STYLE_ZEN_GARDEN_LONG:
	case MessageStyle::MESSAGE_STYLE_ACHIEVEMENT:
		return Sexy::FONT_HOUSEOFTERROR28;

	case MessageStyle::MESSAGE_STYLE_SLOT_MACHINE:
		return Sexy::FONT_HOUSEOFTERROR16;
	case MessageStyle::MESSAGE_STYLE_OFF:
		break;
	}

	PVZP_ASSERT(false);
	unreachable();
}

void MessageWidget::Draw(Graphics* g)
{
	if (mDuration <= 0)
		return;

	_Font* aFont = GetFont();
	_Font* aOutlineFont = nullptr;
	int aPosX = BOARD_WIDTH / 2;
	int aPosY = 596;
	int aTextOffsetY = 0;
	int aRectHeight = 0;
	int aMinAlpha = 255;
	Color aColor(250, 250, 0, 255);
	Color aOutlineColor(0, 0, 0, 255);
	bool aFadeOut = false;
	if (aFont == Sexy::FONT_CONTINUUMBOLD14)
	{
		aOutlineFont = Sexy::FONT_CONTINUUMBOLD14OUTLINE;
	}

	switch (mMessageStyle)
	{
	case MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1:
	case MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY:
		aPosY = 400;
		aRectHeight = 110;
		aTextOffsetY = -4;
		aColor = Color(253, 245, 173);
		aMinAlpha = 192;
		break;

	case MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL2:
	case MessageStyle::MESSAGE_STYLE_TUTORIAL_LATER:
	case MessageStyle::MESSAGE_STYLE_TUTORIAL_LATER_STAY:
	case MessageStyle::MESSAGE_STYLE_HINT_TALL_FAST:
	case MessageStyle::MESSAGE_STYLE_HINT_TALL_UNLOCKMESSAGE:
	case MessageStyle::MESSAGE_STYLE_HINT_TALL_LONG:
	case MessageStyle::MESSAGE_STYLE_ACHIEVEMENT:
		aPosY = 476;
		aRectHeight = 100;
		aTextOffsetY = -4;
		aColor = Color(253, 245, 173);
		aMinAlpha = 192;
		break;

	case MessageStyle::MESSAGE_STYLE_HINT_LONG:
	case MessageStyle::MESSAGE_STYLE_HINT_FAST:
	case MessageStyle::MESSAGE_STYLE_HINT_STAY:
		aPosY = 527;
		aRectHeight = 55;
		aTextOffsetY = -4;
		aColor = Color(253, 245, 173);
		aMinAlpha = 192;
		break;

	case MessageStyle::MESSAGE_STYLE_BIG_MIDDLE:
	case MessageStyle::MESSAGE_STYLE_BIG_MIDDLE_FAST:
		aPosY = 300;
		aRectHeight = 110;
		aColor = Color(253, 245, 173);
		aMinAlpha = 192;
		break;

	case MessageStyle::MESSAGE_STYLE_HOUSE_NAME:
		aPosY = 550;
		aColor = Color(255, 255, 255, 255);
		aFadeOut = true;
		break;

	case MessageStyle::MESSAGE_STYLE_HUGE_WAVE:
		aPosY = 330;
		aColor = Color(255, 0, 0);
		break;

	case MessageStyle::MESSAGE_STYLE_SLOT_MACHINE:
		aPosY = 93;
		aPosX = 340;
		aMinAlpha = 64;
		break;

	case MessageStyle::MESSAGE_STYLE_ZEN_GARDEN_LONG:
		aPosY = 514;
		aRectHeight = 55;
		aTextOffsetY = -4;
		aColor = Color(253, 245, 173);
		aMinAlpha = 192;
		break;

	default:
		PVZP_ASSERT(false);
		break;
	}

	if (mReanimType != ReanimationType::REANIM_NONE)
	{
		if (aFont == Sexy::FONT_CONTINUUMBOLD14)
		{
			DrawReanimatedText(g, Sexy::FONT_CONTINUUMBOLD14OUTLINE, Color::Black, aPosY);
		}
		DrawReanimatedText(g, aFont, aColor, aPosY);
	}
	else
	{
		if (aMinAlpha != 255)
		{
			aColor.mAlpha = PvzpAnimateCurve(75, 0, mApp->mBoard->mMainCounter % 75, aMinAlpha, 255, PvzpCurves::CURVE_BOUNCE_SLOW_MIDDLE);
			aOutlineColor.mAlpha = aColor.mAlpha;
		}
		if (aFadeOut)
		{
			aColor.mAlpha = std::clamp(mDuration * 15, 0, 255);
			aOutlineColor.mAlpha = aColor.mAlpha;
		}

		if (aRectHeight > 0)
		{
			aOutlineColor = Color(0, 0, 0, 128);
			Rect aRect(0, aPosY, BOARD_WIDTH, aRectHeight);
			g->SetColor(aOutlineColor);
			g->FillRect(aRect);

			aRect.mY += aTextOffsetY;
			PvzpDrawStringWrapped(g, mLabel, aRect, aFont, aColor, DrawStringJustification::DS_ALIGN_CENTER_VERTICAL_MIDDLE);
		}
		else
		{
			Rect aRect(aPosX - mApp->mBoard->mX - BOARD_WIDTH / 2, aPosY - aFont->mAscent, BOARD_WIDTH, BOARD_HEIGHT);
			if (aOutlineFont)
			{
				PvzpDrawStringWrapped(g, mLabel, aRect, aOutlineFont, aOutlineColor, DrawStringJustification::DS_ALIGN_CENTER);
			}
			PvzpDrawStringWrapped(g, mLabel, aRect, aFont, aColor, DrawStringJustification::DS_ALIGN_CENTER);
		}

		if (mMessageStyle == MessageStyle::MESSAGE_STYLE_HOUSE_NAME)
		{
			std::string aSubStr;
			if (mApp->IsSurvivalMode() && mApp->mBoard->mChallenge->mSurvivalStage > 0)
			{
				int aFlags = mApp->mBoard->GetNumWavesPerSurvivalStage() * mApp->mBoard->mChallenge->mSurvivalStage / mApp->mBoard->GetNumWavesPerFlag();
				std::string aFlagStr = mApp->Pluralize(aFlags, "[ONE_FLAG]", "[COUNT_FLAGS]");
				aSubStr = PvzpReplaceString("[FLAGS_COMPLETED]", "{FLAGS}", aFlagStr);
			}

			if (aSubStr.size() > 0)
			{
				PvzpDrawString(
					g,
					aSubStr,
					BOARD_WIDTH / 2 - mApp->mBoard->mX,
					aPosY + 26,
					Sexy::FONT_HOUSEOFTERROR16,
					Color(224, 187, 62, aColor.mAlpha),
					DrawStringJustification::DS_ALIGN_CENTER
				);
			}
		}
	}
}
