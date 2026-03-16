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

#include <SDL.h>

#include "SexyAppBase.h"
#include "graphics/GLInterface.h"
#include "graphics/GLImage.h"
#include "widget/WidgetManager.h"
#include "misc/KeyCodes.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EM_JS(void, WasmStartSoftKeyboard, (), {
	var input = document.getElementById('pvz-soft-keyboard');
	if (!input) return;

	if (!Module.wasmSoftKeyboardState) {
		var state = {
			active: false,
			pendingChars: [],
			pendingKeys: [],
			lastValue: ""
		};

		function syncInputValue() {
			if (!state.active) return;

			var nextValue = input.value || "";
			var prefixLen = 0;
			while (prefixLen < state.lastValue.length && prefixLen < nextValue.length
				&& state.lastValue.charCodeAt(prefixLen) === nextValue.charCodeAt(prefixLen)) {
				prefixLen++;
			}

			for (var i = state.lastValue.length; i > prefixLen; --i) {
				state.pendingKeys.push(8);
			}

			for (var j = prefixLen; j < nextValue.length; ++j) {
				var charCode = nextValue.charCodeAt(j);
				if (charCode > 0 && charCode <= 0x7f) {
					state.pendingChars.push(charCode);
				}
			}

			state.lastValue = nextValue;
		}

		input.addEventListener('input', syncInputValue);
		input.addEventListener('keydown', function(event) {
			if (!state.active) return;

			switch (event.key) {
				case 'Enter':
					state.pendingKeys.push(13);
					event.preventDefault();
					break;
				case 'Escape':
					state.pendingKeys.push(27);
					event.preventDefault();
					break;
				case 'Tab':
					state.pendingKeys.push(9);
					event.preventDefault();
					break;
				case 'Delete':
					state.pendingKeys.push(46);
					event.preventDefault();
					break;
				case 'ArrowLeft':
					state.pendingKeys.push(37);
					event.preventDefault();
					break;
				case 'ArrowRight':
					state.pendingKeys.push(39);
					event.preventDefault();
					break;
				case 'Home':
					state.pendingKeys.push(36);
					event.preventDefault();
					break;
				case 'End':
					state.pendingKeys.push(35);
					event.preventDefault();
					break;
			}
		});

		Module.wasmSoftKeyboardState = state;
	}

	var state = Module.wasmSoftKeyboardState;
	state.active = true;
	state.pendingChars.length = 0;
	state.pendingKeys.length = 0;
	state.lastValue = "";
	input.value = "";
	if (typeof input.focus === 'function') {
		input.focus();
	}
	if (typeof input.setSelectionRange === 'function') {
		input.setSelectionRange(0, 0);
	}
});

EM_JS(void, WasmStopSoftKeyboard, (), {
	var input = document.getElementById('pvz-soft-keyboard');
	var state = Module.wasmSoftKeyboardState;
	if (state) {
		state.active = false;
		state.pendingChars.length = 0;
		state.pendingKeys.length = 0;
		state.lastValue = "";
	}
	if (input) {
		input.value = "";
		if (typeof input.blur === 'function') {
			input.blur();
		}
	}
	if (Module.canvas && typeof Module.canvas.focus === 'function') {
		Module.canvas.focus();
	}
});

EM_JS(int, WasmPopSoftKeyboardChar, (), {
	var state = Module.wasmSoftKeyboardState;
	if (!state || state.pendingChars.length === 0) return 0;
	return state.pendingChars.shift();
});

EM_JS(int, WasmPopSoftKeyboardKey, (), {
	var state = Module.wasmSoftKeyboardState;
	if (!state || state.pendingKeys.length === 0) return 0;
	return state.pendingKeys.shift();
});

EM_JS(int, WasmHasSoftKeyboardEvents, (), {
	var state = Module.wasmSoftKeyboardState;
	if (!state) return 0;
	return (state.pendingChars.length + state.pendingKeys.length) > 0 ? 1 : 0;
});
#endif

using namespace Sexy;

// Map SDL_Keycode to internal KeyCode (Windows VK-compatible).
static KeyCode SDLKeyToKeyCode(SDL_Keycode theSDLKey)
{
	if (theSDLKey >= SDLK_a && theSDLKey <= SDLK_z)
		return static_cast<KeyCode>(theSDLKey - SDLK_a + 'A');

	if (theSDLKey >= SDLK_0 && theSDLKey <= SDLK_9)
		return static_cast<KeyCode>(theSDLKey);

	switch (theSDLKey)
	{
		case SDLK_BACKSPACE:    return KEYCODE_BACK;
		case SDLK_TAB:          return KEYCODE_TAB;
		case SDLK_CLEAR:        return KEYCODE_CLEAR;
		case SDLK_RETURN:       return KEYCODE_RETURN;
		case SDLK_AC_BACK:
		case SDLK_ESCAPE:       return KEYCODE_ESCAPE;
		case SDLK_SPACE:        return KEYCODE_SPACE;
		case SDLK_DELETE:       return KEYCODE_DELETE;

		case SDLK_LEFT:         return KEYCODE_LEFT;
		case SDLK_UP:           return KEYCODE_UP;
		case SDLK_RIGHT:        return KEYCODE_RIGHT;
		case SDLK_DOWN:         return KEYCODE_DOWN;

		case SDLK_INSERT:       return KEYCODE_INSERT;
		case SDLK_HOME:         return KEYCODE_HOME;
		case SDLK_END:          return KEYCODE_END;
		case SDLK_PAGEUP:       return KEYCODE_PRIOR;
		case SDLK_PAGEDOWN:     return KEYCODE_NEXT;

		case SDLK_LSHIFT:
		case SDLK_RSHIFT:       return KEYCODE_SHIFT;
		case SDLK_LCTRL:
		case SDLK_RCTRL:        return KEYCODE_CONTROL;
		case SDLK_LALT:
		case SDLK_RALT:         return KEYCODE_MENU;
		case SDLK_PAUSE:        return KEYCODE_PAUSE;
		case SDLK_CAPSLOCK:     return KEYCODE_CAPITAL;
		case SDLK_NUMLOCKCLEAR: return KEYCODE_NUMLOCK;
		case SDLK_SCROLLLOCK:   return KEYCODE_SCROLL;

		case SDLK_KP_0:         return KEYCODE_NUMPAD0;
		case SDLK_KP_1:         return KEYCODE_NUMPAD1;
		case SDLK_KP_2:         return KEYCODE_NUMPAD2;
		case SDLK_KP_3:         return KEYCODE_NUMPAD3;
		case SDLK_KP_4:         return KEYCODE_NUMPAD4;
		case SDLK_KP_5:         return KEYCODE_NUMPAD5;
		case SDLK_KP_6:         return KEYCODE_NUMPAD6;
		case SDLK_KP_7:         return KEYCODE_NUMPAD7;
		case SDLK_KP_8:         return KEYCODE_NUMPAD8;
		case SDLK_KP_9:         return KEYCODE_NUMPAD9;
		case SDLK_KP_MULTIPLY:  return KEYCODE_MULTIPLY;
		case SDLK_KP_PLUS:      return KEYCODE_ADD;
		case SDLK_KP_MINUS:     return KEYCODE_SUBTRACT;
		case SDLK_KP_PERIOD:    return KEYCODE_DECIMAL;
		case SDLK_KP_DIVIDE:    return KEYCODE_DIVIDE;
		case SDLK_KP_ENTER:     return KEYCODE_RETURN;

		case SDLK_F1:           return KEYCODE_F1;
		case SDLK_F2:           return KEYCODE_F2;
		case SDLK_F3:           return KEYCODE_F3;
		case SDLK_F4:           return KEYCODE_F4;
		case SDLK_F5:           return KEYCODE_F5;
		case SDLK_F6:           return KEYCODE_F6;
		case SDLK_F7:           return KEYCODE_F7;
		case SDLK_F8:           return KEYCODE_F8;
		case SDLK_F9:           return KEYCODE_F9;
		case SDLK_F10:          return KEYCODE_F10;
		case SDLK_F11:          return KEYCODE_F11;
		case SDLK_F12:          return KEYCODE_F12;

		default:                return KEYCODE_UNKNOWN;
	}
}

// Synthesize a minimal ASCII char stream from keydown so legacy KeyChar hotkeys still work.
static bool SDLSynthesizeAsciiCharFromKeyDown(const SDL_KeyboardEvent& theEvent, char& theChar)
{
	theChar = 0;

	if (SDL_IsTextInputActive())
		return false;

	SDL_Keycode aSym = theEvent.keysym.sym;
	SDL_Keymod aMods = static_cast<SDL_Keymod>(theEvent.keysym.mod);
	const bool aHasCtrl = (aMods & KMOD_CTRL) != 0;
	const bool aHasAlt = (aMods & KMOD_ALT) != 0;
	const bool aHasGui = (aMods & KMOD_GUI) != 0;
	const bool aHasShift = (aMods & KMOD_SHIFT) != 0;

	if (aHasAlt || aHasGui)
		return false;

	if (aSym >= SDLK_a && aSym <= SDLK_z)
	{
		theChar = aHasCtrl
			? static_cast<char>(aSym - SDLK_a + 1)
			: static_cast<char>(aHasShift ? aSym - SDLK_a + 'A' : aSym);
		return true;
	}

	if (aHasCtrl)
		return false;

	switch (aSym)
	{
		case SDLK_KP_1: theChar = '1'; return true;
		case SDLK_KP_2: theChar = '2'; return true;
		case SDLK_KP_3: theChar = '3'; return true;
		case SDLK_KP_4: theChar = '4'; return true;
		case SDLK_KP_5: theChar = '5'; return true;
		case SDLK_KP_6: theChar = '6'; return true;
		case SDLK_KP_7: theChar = '7'; return true;
		case SDLK_KP_8: theChar = '8'; return true;
		case SDLK_KP_9: theChar = '9'; return true;
		case SDLK_KP_0: theChar = '0'; return true;
		case SDLK_KP_PLUS: theChar = '+'; return true;
		case SDLK_KP_MINUS: theChar = '-'; return true;
		case SDLK_KP_MULTIPLY: theChar = '*'; return true;
		case SDLK_KP_DIVIDE: theChar = '/'; return true;
		case SDLK_KP_PERIOD: theChar = '.'; return true;
		case SDLK_KP_EQUALS: theChar = '='; return true;
		case SDLK_1: theChar = aHasShift ? '!' : '1'; return true;
		case SDLK_2: theChar = aHasShift ? '@' : '2'; return true;
		case SDLK_3: theChar = aHasShift ? '#' : '3'; return true;
		case SDLK_4: theChar = aHasShift ? '$' : '4'; return true;
		case SDLK_5: theChar = aHasShift ? '%' : '5'; return true;
		case SDLK_6: theChar = aHasShift ? '^' : '6'; return true;
		case SDLK_7: theChar = aHasShift ? '&' : '7'; return true;
		case SDLK_8: theChar = aHasShift ? '*' : '8'; return true;
		case SDLK_9: theChar = aHasShift ? '(' : '9'; return true;
		case SDLK_0: theChar = aHasShift ? ')' : '0'; return true;
		case SDLK_MINUS: theChar = aHasShift ? '_' : '-'; return true;
		case SDLK_EQUALS: theChar = aHasShift ? '+' : '='; return true;
		case SDLK_LEFTBRACKET: theChar = aHasShift ? '{' : '['; return true;
		case SDLK_RIGHTBRACKET: theChar = aHasShift ? '}' : ']'; return true;
		case SDLK_BACKSLASH: theChar = aHasShift ? '|' : '\\'; return true;
		case SDLK_SEMICOLON: theChar = aHasShift ? ':' : ';'; return true;
		case SDLK_QUOTE: theChar = aHasShift ? '"' : '\''; return true;
		case SDLK_COMMA: theChar = aHasShift ? '<' : ','; return true;
		case SDLK_PERIOD: theChar = aHasShift ? '>' : '.'; return true;
		case SDLK_SLASH: theChar = aHasShift ? '?' : '/'; return true;
		case SDLK_BACKQUOTE: theChar = aHasShift ? '~' : '`'; return true;
		case SDLK_SPACE: theChar = ' '; return true;
		default: return false;
	}
}

void SexyAppBase::InitInput()
{
	SDL_Init(SDL_INIT_EVENTS);
}

bool SexyAppBase::StartTextInput(std::string& theInput)
{
	(void)theInput;
	SDL_StartTextInput();

#ifdef __EMSCRIPTEN__
	WasmStartSoftKeyboard();
#endif

	return false;
}

void SexyAppBase::StopTextInput()
{
	SDL_StopTextInput();

#ifdef __EMSCRIPTEN__
	WasmStopSoftKeyboard();
#endif
}

bool SexyAppBase::ProcessDeferredMessages(bool singleMessage)
{
#ifdef __EMSCRIPTEN__
	int aPendingKey = WasmPopSoftKeyboardKey();
	if (aPendingKey != 0)
	{
		mLastUserInputTick = mLastTimerTime;
		mWidgetManager->KeyDown(static_cast<KeyCode>(aPendingKey));
		return WasmHasSoftKeyboardEvents() || SDL_HasEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
	}

	int aPendingChar = WasmPopSoftKeyboardChar();
	if (aPendingChar != 0)
	{
		mLastUserInputTick = mLastTimerTime;
		mWidgetManager->KeyChar(static_cast<char>(aPendingChar));
		return WasmHasSoftKeyboardEvents() || SDL_HasEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
	}
#endif

	SDL_Event event;
	if (SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			case SDL_QUIT:
				CloseRequestAsync();
				break;

			case SDL_APP_WILLENTERBACKGROUND:
				mMinimized = true;
				RehupFocus();
				break;

			case SDL_APP_DIDENTERFOREGROUND:
				mMinimized = false;
				RehupFocus();
				mWidgetManager->MarkAllDirty();
				break;

			case SDL_WINDOWEVENT:
				switch(event.window.event)
				{
					case SDL_WINDOWEVENT_CLOSE:
						CloseRequestAsync();
						break;

					case SDL_WINDOWEVENT_RESIZED:
						mGLInterface->UpdateViewport();
						mWidgetManager->Resize(mScreenBounds, mGLInterface->mPresentationRect);
						mWidgetManager->MarkAllDirty();
						break;

					case SDL_WINDOWEVENT_MINIMIZED:
						mMinimized = true;
						RehupFocus();
						break;

					case SDL_WINDOWEVENT_RESTORED:
						mMinimized = false;
						RehupFocus();
						mWidgetManager->MarkAllDirty();
						break;

					case SDL_WINDOWEVENT_FOCUS_GAINED:
					case SDL_WINDOWEVENT_FOCUS_LOST:
						mActive = event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED;
						RehupFocus();
						break;
				}
				break;

			case SDL_MOUSEWHEEL:
			{
				mLastUserInputTick = mLastTimerTime;
				mWidgetManager->MouseWheel(event.wheel.y);
				break;
			}

			case SDL_MOUSEMOTION:
			{
				if (!mMouseIn)
					mMouseIn = true;

				int x = event.motion.x;
				int y = event.motion.y;
				mWidgetManager->RemapMouse(x, y);

				mLastUserInputTick = mLastTimerTime;
				
				mWidgetManager->MouseMove(x, y);
				break;
			}

			case SDL_MOUSEBUTTONDOWN:
			{
				if (!mMouseIn)
					mMouseIn = true;

				int x = event.button.x;
				int y = event.button.y;
				mWidgetManager->RemapMouse(x, y);

				mLastUserInputTick = mLastTimerTime;
				
				mWidgetManager->MouseMove(x, y);
				int btn =
					(event.button.button == SDL_BUTTON_LEFT) ? 1 :
					(event.button.button == SDL_BUTTON_RIGHT) ? -1 :
					3;
				if (event.button.clicks == 2)
					btn = (event.button.button == SDL_BUTTON_LEFT) ? 2 : -2;

				mWidgetManager->MouseDown(x, y, btn);
				break;
			}

			case SDL_MOUSEBUTTONUP:
			{
				if (!mMouseIn)
					mMouseIn = true;

				int x = event.button.x;
				int y = event.button.y;
				mWidgetManager->RemapMouse(x, y);

				mLastUserInputTick = mLastTimerTime;
				
				mWidgetManager->MouseMove(x, y);
				int btn =
					(event.button.button == SDL_BUTTON_LEFT) ? 1 :
					(event.button.button == SDL_BUTTON_RIGHT) ? -1 :
					3;

				mWidgetManager->MouseUp(x, y, btn);
				break;
			}

			case SDL_KEYDOWN:
			{
				mLastUserInputTick = mLastTimerTime;

				if (mAllowAltEnter &&
					event.key.repeat == 0 &&
					(event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) &&
					(event.key.keysym.mod & KMOD_ALT))
				{
					SwitchScreenMode(!mIsWindowed);
					break;
				}

				mWidgetManager->KeyDown(SDLKeyToKeyCode(event.key.keysym.sym));

				char aSynthesizedChar = 0;
				if (SDLSynthesizeAsciiCharFromKeyDown(event.key, aSynthesizedChar))
					mWidgetManager->KeyChar(aSynthesizedChar);

				break;
			}

			case SDL_KEYUP:
				mLastUserInputTick = mLastTimerTime;
				mWidgetManager->KeyUp(SDLKeyToKeyCode(event.key.keysym.sym));
				break;

			case SDL_TEXTINPUT:
				mLastUserInputTick = mLastTimerTime;
				mWidgetManager->KeyChar((char)event.text.text[0]);
				break;
		}
	}

	return SDL_HasEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
}
