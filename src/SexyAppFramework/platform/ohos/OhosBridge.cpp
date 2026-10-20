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

#include "OhosBridge.h"

#include <napi/native_api.h>
#include <SDL.h>
#include <mutex>

static std::mutex gTerminateMutex;
static napi_threadsafe_function gTerminateFunc = nullptr;

static void ReleaseTerminateHandler(napi_threadsafe_function theFunc, napi_threadsafe_function_release_mode theMode)
{
	if (theFunc != nullptr)
		napi_release_threadsafe_function(theFunc, theMode);
}

static void CleanupTerminateHandler([[maybe_unused]] void* theData)
{
	napi_threadsafe_function anOld;
	{
		std::lock_guard aLock(gTerminateMutex);
		anOld = gTerminateFunc;
		gTerminateFunc = nullptr;
	}
	ReleaseTerminateHandler(anOld, napi_tsfn_abort);
}

static void CallTerminateHandler(napi_env theEnv, napi_value theCallback, [[maybe_unused]] void* theContext, [[maybe_unused]] void* theData)
{
	if (theEnv == nullptr || theCallback == nullptr)
		return;

	napi_value aUndefined;
	if (napi_get_undefined(theEnv, &aUndefined) != napi_ok)
		return;
	napi_call_function(theEnv, aUndefined, theCallback, 0, nullptr, nullptr);
}

static napi_value SetTerminateHandler(napi_env theEnv, napi_callback_info theInfo)
{
	size_t anArgc = 1;
	napi_value anArgv[1] = {nullptr};
	if (napi_get_cb_info(theEnv, theInfo, &anArgc, anArgv, nullptr, nullptr) != napi_ok)
	{
		napi_throw_error(theEnv, nullptr, "Failed to read setTerminateHandler arguments");
		return nullptr;
	}

	napi_valuetype aType = napi_undefined;
	if (anArgc < 1 || napi_typeof(theEnv, anArgv[0], &aType) != napi_ok || aType != napi_function)
	{
		napi_throw_type_error(theEnv, nullptr, "setTerminateHandler expects a function");
		return nullptr;
	}

	napi_value aName;
	if (napi_create_string_utf8(theEnv, "PvzpTerminate", NAPI_AUTO_LENGTH, &aName) != napi_ok)
	{
		napi_throw_error(theEnv, nullptr, "Failed to name the terminate handler");
		return nullptr;
	}

	napi_threadsafe_function aFunc = nullptr;
	if (napi_create_threadsafe_function(theEnv, anArgv[0], nullptr, aName, 0, 1, nullptr, nullptr, nullptr, CallTerminateHandler, &aFunc) != napi_ok)
	{
		napi_throw_error(theEnv, nullptr, "Failed to create the terminate handler");
		return nullptr;
	}

	// Don't let the pending handler keep the ArkTS event loop alive on its own.
	if (napi_unref_threadsafe_function(theEnv, aFunc) != napi_ok)
	{
		ReleaseTerminateHandler(aFunc, napi_tsfn_release);
		napi_throw_error(theEnv, nullptr, "Failed to unref the terminate handler");
		return nullptr;
	}

	napi_threadsafe_function anOld;
	{
		std::lock_guard aLock(gTerminateMutex);
		anOld = gTerminateFunc;
		gTerminateFunc = aFunc;
	}
	ReleaseTerminateHandler(anOld, napi_tsfn_release);
	return nullptr;
}

static napi_value SendBackKey([[maybe_unused]] napi_env theEnv, [[maybe_unused]] napi_callback_info theInfo)
{
	if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
		return nullptr;

	SDL_Event anEvent{};
	anEvent.type = SDL_KEYDOWN;
	anEvent.key.state = SDL_PRESSED;
	anEvent.key.keysym.scancode = SDL_SCANCODE_AC_BACK;
	anEvent.key.keysym.sym = SDLK_AC_BACK;
	SDL_PushEvent(&anEvent);

	anEvent.type = SDL_KEYUP;
	anEvent.key.state = SDL_RELEASED;
	SDL_PushEvent(&anEvent);
	return nullptr;
}

void PvzpOhosTerminateApp()
{
	napi_threadsafe_function aFunc;
	{
		std::lock_guard aLock(gTerminateMutex);
		aFunc = gTerminateFunc;
		if (aFunc != nullptr && napi_acquire_threadsafe_function(aFunc) != napi_ok)
			aFunc = nullptr;
	}
	if (aFunc == nullptr)
		return;

	const napi_status aStatus = napi_call_threadsafe_function(aFunc, nullptr, napi_tsfn_nonblocking);
	ReleaseTerminateHandler(aFunc, napi_tsfn_release);
	if (aStatus != napi_ok)
		return;

	// terminateSelf() tears the process down; fall back to returning if it never happens.
	SDL_Delay(5000);
}

static napi_value InitModule(napi_env theEnv, napi_value theExports)
{
	if (napi_add_env_cleanup_hook(theEnv, CleanupTerminateHandler, nullptr) != napi_ok)
	{
		napi_throw_error(theEnv, nullptr, "Failed to register the terminate handler cleanup");
		return nullptr;
	}

	napi_property_descriptor aDescs[] = {
		{"setTerminateHandler", nullptr, SetTerminateHandler, nullptr, nullptr, nullptr, napi_default, nullptr},
		{"sendBackKey", nullptr, SendBackKey, nullptr, nullptr, nullptr, napi_default, nullptr},
	};
	if (napi_define_properties(theEnv, theExports, sizeof(aDescs) / sizeof(aDescs[0]), aDescs) != napi_ok)
	{
		napi_remove_env_cleanup_hook(theEnv, CleanupTerminateHandler, nullptr);
		napi_throw_error(theEnv, nullptr, "Failed to export the HarmonyOS bridge");
		return nullptr;
	}
	return theExports;
}

static napi_module gMainModule = {
	.nm_version = 1,
	.nm_flags = 0,
	.nm_filename = nullptr,
	.nm_register_func = InitModule,
	.nm_modname = "main",
	.nm_priv = nullptr,
	.reserved = {nullptr},
};

extern "C" __attribute__((constructor)) void PvzpRegisterOhosModule()
{
	napi_module_register(&gMainModule);
}
