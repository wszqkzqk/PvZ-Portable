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

#include <SDL3/SDL.h>
#include <emscripten.h>
#include <emscripten/html5.h>

#include <memory>
#include <utility>

#include "SexyAppBase.h"
#include "graphics/GLInterface.h"
#include "graphics/GLImage.h"
#include "widget/WidgetManager.h"

using namespace Sexy;

void SexyAppBase::MakeWindow()
{
	if (mWindow)
	{
		// Use the browser Fullscreen API; SDL_SetWindowFullscreen has no effect on Emscripten.
		if (!mIsWindowed)
			emscripten_request_fullscreen("#canvas-container", EM_FALSE);
		else
			emscripten_exit_fullscreen();
	}
	else
	{
		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			Sexy::LogErrorLn("Failed to initialize SDL video: {}", SDL_GetError());
			return;
		}

		SDL_WindowFlags winFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE;

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);

		SDLWindowPtr window(SDL_CreateWindow(mTitle.c_str(), mWidth, mHeight, winFlags), &SDL_DestroyWindow);
		SDLGLContextPtr context(nullptr, &SDL_GL_DestroyContext);

		if (window)
			context.reset(SDL_GL_CreateContext(window.get()));

		if (!context)
		{
			Sexy::LogErrorLn("Failed to create WebGL context: {}", SDL_GetError());
			return;
		}

		SDL_GL_SetSwapInterval(0);

		mWindow = std::move(window);
		mContext = std::move(context);
	}

	if (mGLInterface == nullptr)
	{
		mGLInterface = std::make_unique<GLInterface>(this);
		if (!InitGLInterface())
		{
			mGLInterface = nullptr;
			return;
		}
	}

	bool isActive = mActive;
	mActive = !!(SDL_GetWindowFlags(mWindow.get()) & SDL_WINDOW_INPUT_FOCUS);

	mPhysMinimized = false;
	if (mMinimized)
	{
		if (mMuteOnLostFocus)
			Unmute(true);

		mMinimized = false;
		isActive = mActive;
		RehupFocus();
	}

	if (isActive != mActive)
		RehupFocus();

	ReInitImages();

	mWidgetManager->mImage = mGLInterface->GetScreenImage();
	mWidgetManager->MarkAllDirty();

	mGLInterface->UpdateViewport();
	mWidgetManager->Resize(mScreenBounds, mGLInterface->mPresentationRect);
}
