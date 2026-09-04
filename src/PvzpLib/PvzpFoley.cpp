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

#include "PvzpFoley.h"
#include "PvzpDebug.h"
#include "PvzpCommon.h"
#include "sound/SoundManager.h"

int gFoleyParamArraySize;
const FoleyParams* gFoleyParamArray;

constinit const FoleyParams gLawnFoleyParamArray[FoleyType::NUM_FOLEY] = {
	{ .mFoleyType = FoleyType::FOLEY_SUN, .mPitchRange = 10.0f, .mSfxID = { &Sexy::SOUND_POINTS}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_SPLAT, .mPitchRange = 10.0f, .mSfxID = { &Sexy::SOUND_SPLAT, &Sexy::SOUND_SPLAT2, &Sexy::SOUND_SPLAT3}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_LAWNMOWER, .mPitchRange = 10.0f, .mSfxID = { &Sexy::SOUND_LAWNMOWER}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_THROW, .mPitchRange = 10.0f, .mSfxID = { &Sexy::SOUND_THROW, &Sexy::SOUND_THROW, &Sexy::SOUND_THROW, &Sexy::SOUND_THROW2 }, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_SPAWN_SUN, .mPitchRange = 10.0f, .mSfxID = { &Sexy::SOUND_THROW}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_CHOMP, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_CHOMP, &Sexy::SOUND_CHOMP2}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_CHOMP_SOFT, .mPitchRange = 4.0f, .mSfxID = { &Sexy::SOUND_CHOMPSOFT}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_PLANT, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_PLANT, &Sexy::SOUND_PLANT2}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_USE_SHOVEL, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_PLANT2}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_DROP, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_TAP2}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_BLEEP, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_BLEEP}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_GROAN, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_GROAN, &Sexy::SOUND_GROAN2, &Sexy::SOUND_GROAN3, &Sexy::SOUND_GROAN4, &Sexy::SOUND_GROAN5, &Sexy::SOUND_GROAN6 }, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_BRAINS, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_GROAN, &Sexy::SOUND_GROAN2, &Sexy::SOUND_GROAN3, &Sexy::SOUND_GROAN4, &Sexy::SOUND_GROAN5, &Sexy::SOUND_GROAN6, &Sexy::SOUND_SUKHBIR4, &Sexy::SOUND_SUKHBIR5, &Sexy::SOUND_SUKHBIR6 }, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_SUKHBIR, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_GROAN, &Sexy::SOUND_GROAN2, &Sexy::SOUND_GROAN3, &Sexy::SOUND_GROAN4, &Sexy::SOUND_GROAN5, &Sexy::SOUND_GROAN6, &Sexy::SOUND_SUKHBIR, &Sexy::SOUND_SUKHBIR2, &Sexy::SOUND_SUKHBIR3 }, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_JACKINTHEBOX, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_JACKINTHEBOX}, .mFoleyFlags = 7U },
	{ .mFoleyType = FoleyType::FOLEY_ART_CHALLENGE, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_DIAMOND}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_ZAMBONI, .mPitchRange = 5.0f, .mSfxID = { &Sexy::SOUND_ZAMBONI}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_THUNDER, .mPitchRange = 10.0f, .mSfxID = { &Sexy::SOUND_THUNDER}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_FROZEN, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_FROZEN}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_ZOMBIESPLASH, .mPitchRange = 10.0f, .mSfxID = { &Sexy::SOUND_PLANT_WATER, &Sexy::SOUND_ZOMBIE_ENTERING_WATER}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_BOWLINGIMPACT, .mPitchRange = -3.0f, .mSfxID = { &Sexy::SOUND_BOWLINGIMPACT}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_SQUISH, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_CHOMP, &Sexy::SOUND_CHOMP2}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_TIRE_POP, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_BALLOON_POP}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_EXPLOSION, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_EXPLOSION}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_SLURP, .mPitchRange = 2.0f, .mSfxID = { &Sexy::SOUND_SLURP}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_LIMBS_POP, .mPitchRange = 10.0f, .mSfxID = { &Sexy::SOUND_LIMBS_POP}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_POGO_ZOMBIE, .mPitchRange = 4.0f, .mSfxID = { &Sexy::SOUND_POGO_ZOMBIE}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_SNOW_PEA_SPARKLES, .mPitchRange = 10.0f, .mSfxID = { &Sexy::SOUND_SNOW_PEA_SPARKLES}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_ZOMBIE_FALLING, .mPitchRange = 10.0f, .mSfxID = { &Sexy::SOUND_ZOMBIE_FALLING_1, &Sexy::SOUND_ZOMBIE_FALLING_2}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_PUFF, .mPitchRange = 10.0f, .mSfxID = { &Sexy::SOUND_PUFF}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_FUME, .mPitchRange = 10.0f, .mSfxID = { &Sexy::SOUND_FUME}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_COIN, .mPitchRange = 10.0f, .mSfxID = { &Sexy::SOUND_COIN}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_KERNEL_SPLAT, .mPitchRange = 10.0f, .mSfxID = { &Sexy::SOUND_KERNELPULT, &Sexy::SOUND_KERNELPULT2}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_DIGGER, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_DIGGER_ZOMBIE}, .mFoleyFlags = 7U },
	{ .mFoleyType = FoleyType::FOLEY_JACK_SURPRISE, .mPitchRange = 1.0f, .mSfxID = { &Sexy::SOUND_JACK_SURPRISE, &Sexy::SOUND_JACK_SURPRISE, &Sexy::SOUND_JACK_SURPRISE2}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_VASE_BREAKING, .mPitchRange = -5.0f, .mSfxID = { &Sexy::SOUND_VASE_BREAKING}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_POOL_CLEANER, .mPitchRange = 2.0f, .mSfxID = { &Sexy::SOUND_POOL_CLEANER}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_BASKETBALL, .mPitchRange = 10.0f, .mSfxID = { &Sexy::SOUND_BASKETBALL}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_IGNITE, .mPitchRange = 5.0f, .mSfxID = { &Sexy::SOUND_IGNITE, &Sexy::SOUND_IGNITE, &Sexy::SOUND_IGNITE, &Sexy::SOUND_IGNITE2}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_FIREPEA, .mPitchRange = 10.0f, .mSfxID = { &Sexy::SOUND_FIREPEA}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_THUMP, .mPitchRange = 2.0f, .mSfxID = { &Sexy::SOUND_GARGANTUAR_THUMP}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_SQUASH_HMM, .mPitchRange = 2.0f, .mSfxID = { &Sexy::SOUND_SQUASH_HMM, &Sexy::SOUND_SQUASH_HMM, &Sexy::SOUND_SQUASH_HMM2}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_MAGNETSHROOM, .mPitchRange = 2.0f, .mSfxID = { &Sexy::SOUND_MAGNETSHROOM}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_BUTTER, .mPitchRange = 2.0f, .mSfxID = { &Sexy::SOUND_BUTTER}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_BUNGEE_SCREAM, .mPitchRange = 2.0f, .mSfxID = { &Sexy::SOUND_BUNGEE_SCREAM, &Sexy::SOUND_BUNGEE_SCREAM2, &Sexy::SOUND_BUNGEE_SCREAM3}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_BOSS_EXPLOSION_SMALL, .mPitchRange = 2.0f, .mSfxID = { &Sexy::SOUND_EXPLOSION}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_SHIELD_HIT, .mPitchRange = 10.0f, .mSfxID = { &Sexy::SOUND_SHIELDHIT, &Sexy::SOUND_SHIELDHIT2}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_SWING, .mPitchRange = 2.0f, .mSfxID = { &Sexy::SOUND_SWING}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_BONK, .mPitchRange = 2.0f, .mSfxID = { &Sexy::SOUND_BONK}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_RAIN, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_RAIN}, .mFoleyFlags = 5U },
	{ .mFoleyType = FoleyType::FOLEY_DOLPHIN_BEFORE_JUMPING, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_DOLPHIN_BEFORE_JUMPING}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_DOLPHIN_APPEARS, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_DOLPHIN_APPEARS}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_PLANT_WATER, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_PLANT_WATER}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_ZOMBIE_ENTERING_WATER, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_ZOMBIE_ENTERING_WATER}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_GRAVEBUSTERCHOMP, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_GRAVEBUSTERCHOMP}, .mFoleyFlags = 4U },
	{ .mFoleyType = FoleyType::FOLEY_CHERRYBOMB, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_CHERRYBOMB}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_JALAPENO_IGNITE, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_JALAPENO}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_REVERSE_EXPLOSION, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_REVERSE_EXPLOSION}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_PLASTIC_HIT, .mPitchRange = 5.0f, .mSfxID = { &Sexy::SOUND_PLASTICHIT, &Sexy::SOUND_PLASTICHIT2}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_WINMUSIC, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_WINMUSIC}, .mFoleyFlags = 8U },
	{ .mFoleyType = FoleyType::FOLEY_BALLOONINFLATE, .mPitchRange = 10.0f, .mSfxID = { &Sexy::SOUND_BALLOONINFLATE}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_BIGCHOMP, .mPitchRange = -2.0f, .mSfxID = { &Sexy::SOUND_BIGCHOMP}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_MELONIMPACT, .mPitchRange = -5.0f, .mSfxID = { &Sexy::SOUND_MELONIMPACT, &Sexy::SOUND_MELONIMPACT2}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_PLANTGROW, .mPitchRange = -2.0f, .mSfxID = { &Sexy::SOUND_PLANTGROW}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_SHOOP, .mPitchRange = -5.0f, .mSfxID = { &Sexy::SOUND_SHOOP}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_JUICY, .mPitchRange = 2.0f, .mSfxID = { &Sexy::SOUND_JUICY}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_NEWSPAPER_RARRGH, .mPitchRange = -2.0f, .mSfxID = { &Sexy::SOUND_NEWSPAPER_RARRGH, &Sexy::SOUND_NEWSPAPER_RARRGH2, &Sexy::SOUND_NEWSPAPER_RARRGH2}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_NEWSPAPER_RIP, .mPitchRange = -2.0f, .mSfxID = { &Sexy::SOUND_NEWSPAPER_RIP}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_FLOOP, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_FLOOP}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_COFFEE, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_COFFEE}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_LOW_GROAN, .mPitchRange = 2.0f, .mSfxID = { &Sexy::SOUND_LOWGROAN, &Sexy::SOUND_LOWGROAN2}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_PRIZE, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_PRIZE}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_YUCK, .mPitchRange = 1.0f, .mSfxID = { &Sexy::SOUND_YUCK, &Sexy::SOUND_YUCK, &Sexy::SOUND_YUCK2}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_UMBRELLA, .mPitchRange = 2.0f, .mSfxID = { &Sexy::SOUND_THROW2}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_GRASSSTEP, .mPitchRange = 2.0f, .mSfxID = { &Sexy::SOUND_GRASSSTEP}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_SHOVEL, .mPitchRange = 5.0f, .mSfxID = { &Sexy::SOUND_SHOVEL}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_COB_LAUNCH, .mPitchRange = 10.0f, .mSfxID = { &Sexy::SOUND_COBLAUNCH}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_WATERING, .mPitchRange = 10.0f, .mSfxID = { &Sexy::SOUND_WATERING}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_POLEVAULT, .mPitchRange = 5.0f, .mSfxID = { &Sexy::SOUND_POLEVAULT}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_GRAVESTONE_RUMBLE, .mPitchRange = 10.0f, .mSfxID = { &Sexy::SOUND_GRAVESTONE_RUMBLE}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_DIRT_RISE, .mPitchRange = 5.0f, .mSfxID = { &Sexy::SOUND_DIRT_RISE}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_FERTILIZER, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_FERTILIZER}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_PORTAL, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_PORTAL}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_WAKEUP, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_WAKEUP}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_BUGSPRAY, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_BUGSPRAY}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_SCREAM, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_SCREAM}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_PAPER, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_PAPER}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_MONEYFALLS, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_MONEYFALLS}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_IMP, .mPitchRange = 5.0f, .mSfxID = { &Sexy::SOUND_IMP, &Sexy::SOUND_IMP2}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_HYDRAULIC_SHORT, .mPitchRange = 3.0f, .mSfxID = { &Sexy::SOUND_HYDRAULIC_SHORT}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_HYDRAULIC, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_HYDRAULIC}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_GARGANTUDEATH, .mPitchRange = 3.0f, .mSfxID = { &Sexy::SOUND_GARGANTUDEATH}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_CERAMIC, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_CERAMIC}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_BOSS_BOULDER_ATTACK, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_BOSSBOULDERATTACK}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_CHIME, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_CHIME}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_CRAZY_DAVE_SHORT, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_CRAZYDAVESHORT1, &Sexy::SOUND_CRAZYDAVESHORT2, &Sexy::SOUND_CRAZYDAVESHORT3}, .mFoleyFlags = 16U },
	{ .mFoleyType = FoleyType::FOLEY_CRAZY_DAVE_LONG, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_CRAZYDAVELONG1, &Sexy::SOUND_CRAZYDAVELONG2, &Sexy::SOUND_CRAZYDAVELONG3}, .mFoleyFlags = 16U },
	{ .mFoleyType = FoleyType::FOLEY_CRAZY_DAVE_EXTRA_LONG, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_CRAZYDAVEEXTRALONG1, &Sexy::SOUND_CRAZYDAVEEXTRALONG2, &Sexy::SOUND_CRAZYDAVEEXTRALONG3}, .mFoleyFlags = 16U },
	{ .mFoleyType = FoleyType::FOLEY_CRAZY_DAVE_CRAZY, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_CRAZYDAVECRAZY}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_PHONOGRAPH, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_PHONOGRAPH}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_DANCER, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_DANCER}, .mFoleyFlags = 6U },
	{ .mFoleyType = FoleyType::FOLEY_FINAL_FANFARE, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_FINALFANFARE}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_CRAZY_DAVE_SCREAM, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_CRAZYDAVESCREAM}, .mFoleyFlags = 0U },
	{ .mFoleyType = FoleyType::FOLEY_CRAZY_DAVE_SCREAM_2, .mPitchRange = 0.0f, .mSfxID = { &Sexy::SOUND_CRAZYDAVESCREAM2}, .mFoleyFlags = 0U }
};

FoleyInstance::FoleyInstance()
{
	mInstance = nullptr;
	mRefCount = 0;
	mPaused = false;
	mStartTime = 0;
	mPauseOffset = 0;
}

FoleyTypeData::FoleyTypeData()
{
	mLastVariationPlayed = -1;
}

int PvzpDSoundInstance::GetSoundPosition()
{
	return 0;
}

void PvzpDSoundInstance::SetSoundPosition(int thePosition)
{
}

void PvzpFoleyInitialize(const FoleyParams* theFoleyParamArray, int theFoleyParamArraySize)
{
	PVZP_ASSERT(gFoleyParamArray == nullptr && gFoleyParamArraySize == 0);
	gFoleyParamArray = theFoleyParamArray;
	gFoleyParamArraySize = theFoleyParamArraySize;
}

void PvzpFoleyDispose()
{
	gFoleyParamArray = nullptr;
	gFoleyParamArraySize = 0;
}

void SoundSystemReleaseFinishedInstances(PvzpFoley* theSoundSystem)
{
	for (int aFoleyType = 0; aFoleyType < gFoleyParamArraySize; aFoleyType++)
		for (int i = 0; i < MAX_FOLEY_INSTANCES; i++)
		{
			FoleyInstance* aFoleyInstance = &theSoundSystem->mFoleyTypeData[aFoleyType].mFoleyInstances[i];
			if (aFoleyInstance->mRefCount == 0)
			{
				PVZP_ASSERT(aFoleyInstance->mInstance == nullptr);
			}
			else if (!aFoleyInstance->mPaused)
			{
				PVZP_ASSERT(aFoleyInstance->mInstance);
				if (!aFoleyInstance->mInstance->IsPlaying())
				{
					aFoleyInstance->mInstance->Release();
					aFoleyInstance->mInstance = nullptr;
					aFoleyInstance->mRefCount = 0;
				}
			}
		}
}

bool SoundSystemHasFoleyPlayedTooRecently(PvzpFoley* theSoundSystem, FoleyType theFoleyType)
{
	FoleyTypeData* aFoleyData = &theSoundSystem->mFoleyTypeData[theFoleyType];
	for (int i = 0; i < MAX_FOLEY_INSTANCES; i++)
	{
		FoleyInstance* aFoleyInstance = &aFoleyData->mFoleyInstances[i];
		if (aFoleyInstance->mRefCount != 0 && gSexyAppBase->mUpdateCount - aFoleyInstance->mStartTime < 10U)  // an instance of this sound was played within the last 10 cs
			return true;
	}
	return false;
}

const FoleyParams* LookupFoley(FoleyType theFoleyType)
{
	PVZP_ASSERT(theFoleyType >= 0 && theFoleyType < gFoleyParamArraySize);
	PVZP_ASSERT(gFoleyParamArraySize < MAX_FOLEY_TYPES);
	const FoleyParams* aFoleyParams = &gFoleyParamArray[theFoleyType];
	PVZP_ASSERT(aFoleyParams->mFoleyType == theFoleyType);
	return aFoleyParams;
}

FoleyInstance* SoundSystemFindInstance(PvzpFoley* theSoundSystem, FoleyType theFoleyType)
{
	FoleyTypeData* aFoleyData = &theSoundSystem->mFoleyTypeData[theFoleyType];
	for (int i = 0; i < MAX_FOLEY_INSTANCES; i++)
	{
		FoleyInstance* aFoleyInstance = &aFoleyData->mFoleyInstances[i];
		if (aFoleyInstance->mRefCount > 0)
		{
			PVZP_ASSERT(aFoleyInstance->mInstance);
			return aFoleyInstance;
		}
	}
	return nullptr;
}

FoleyInstance* SoundSystemGetFreeInstanceIndex(PvzpFoley* theSoundSystem, FoleyType theFoleyType)
{
	FoleyTypeData* aFoleyData = &theSoundSystem->mFoleyTypeData[theFoleyType];
	for (int i = 0; i < MAX_FOLEY_INSTANCES; i++)
	{
		FoleyInstance* aFoleyInstance = &aFoleyData->mFoleyInstances[i];
		if (aFoleyInstance->mRefCount == 0)
		{
			PVZP_ASSERT(aFoleyInstance->mInstance == nullptr);
			return aFoleyInstance;
		}
	}
	return nullptr;
}

void PvzpFoley::PlayFoleyPitch(FoleyType theFoleyType, float thePitch)
{
	const FoleyParams* aFoleyParams = LookupFoley(theFoleyType);
	SoundSystemReleaseFinishedInstances(this);
	if (SoundSystemHasFoleyPlayedTooRecently(this, theFoleyType) && !TestBit(aFoleyParams->mFoleyFlags, FoleyFlags::FOLEYFLAGS_LOOP))
		return;  // non-looping sounds may not overlap

	if (TestBit(aFoleyParams->mFoleyFlags, FoleyFlags::FOLEYFLAGS_ONE_AT_A_TIME))
	{
		FoleyInstance* aFoleyInstance = SoundSystemFindInstance(this, theFoleyType);
		if (aFoleyInstance != nullptr)
		{
			aFoleyInstance->mRefCount++;
			aFoleyInstance->mStartTime = gSexyAppBase->mUpdateCount;
			return;
		}
	}
	FoleyInstance* aFoleyInstance = SoundSystemGetFreeInstanceIndex(this, theFoleyType);
	if (aFoleyInstance == nullptr)  // all instances are in use
		return;

	int aVariations = 0;
	int aVariationsArray[10];
	FoleyTypeData* aFoleyData = &mFoleyTypeData[theFoleyType];
	for (int i = 0; i < 10; i++)
	{
		if (!TestBit(aFoleyParams->mFoleyFlags, FoleyFlags::FOLEYFLAGS_DONT_REPEAT) || aFoleyData->mLastVariationPlayed != i)  // exclude the last-played variation when repeats are forbidden
		{
			if (aFoleyParams->mSfxID[i] == nullptr)
				break;
			aVariationsArray[aVariations] = i;
			aVariations++;
		}
	}
	PVZP_ASSERT(aVariations > 0);
	int aVariation = PvzpPickFromArray(aVariationsArray, aVariations);
	aFoleyData->mLastVariationPlayed = aVariation;
	SoundInstance* aSoundInstance = gSexyAppBase->mSoundManager->GetSoundInstance(*aFoleyParams->mSfxID[aVariation]);
	if (aSoundInstance == nullptr)
		return;

	aFoleyInstance->mInstance = aSoundInstance;
	aFoleyInstance->mRefCount = 1;
	aFoleyInstance->mStartTime = gSexyAppBase->mUpdateCount;
	aFoleyInstance->mPaused = false;
	aFoleyData->mLastVariationPlayed = aVariation;
	if (thePitch != 0.0f)
		aSoundInstance->AdjustPitch(thePitch);
	if (TestBit(aFoleyParams->mFoleyFlags, FoleyFlags::FOLEYFLAGS_USES_MUSIC_VOLUME))
		ApplyMusicVolume(aFoleyInstance);
	bool aIsLooping = TestBit(aFoleyParams->mFoleyFlags, FoleyFlags::FOLEYFLAGS_LOOP);
	aSoundInstance->Play(aIsLooping, false);
}

void PvzpFoley::PlayFoley(FoleyType theFoleyType)
{
	const FoleyParams* aFoleyParams = LookupFoley(theFoleyType);
	float aPitch = 0.0f;
	if (aFoleyParams->mPitchRange != 0.0f)
		aPitch = Sexy::Rand(aFoleyParams->mPitchRange);
	PlayFoleyPitch(theFoleyType, aPitch);
}

void PvzpFoley::StopFoley(FoleyType theFoleyType)
{
	SoundSystemReleaseFinishedInstances(this);
	FoleyInstance* aFoleyInstance = SoundSystemFindInstance(this, theFoleyType);
	if (aFoleyInstance == nullptr)
		return;

	PVZP_ASSERT(aFoleyInstance->mRefCount > 0);
	PVZP_ASSERT(aFoleyInstance->mInstance);
	aFoleyInstance->mRefCount--;
	if (aFoleyInstance->mRefCount == 0)
	{
		aFoleyInstance->mInstance->Release();
		aFoleyInstance->mInstance = nullptr;
	}
}

void PvzpFoley::GamePause(bool theEnteringPause)
{
	SoundSystemReleaseFinishedInstances(this);
	for (int aFoleyType = 0; aFoleyType < gFoleyParamArraySize; aFoleyType++)
	{
		const FoleyParams* aFoleyParams = LookupFoley((FoleyType)aFoleyType);
		if (TestBit(aFoleyParams->mFoleyFlags, FoleyFlags::FOLEYFLAGS_MUTE_ON_PAUSE))
		{
			FoleyTypeData* aFoleyData = &mFoleyTypeData[aFoleyType];
			for (int i = 0; i < MAX_FOLEY_INSTANCES; i++)
			{
				FoleyInstance* aFoleyInstance = &aFoleyData->mFoleyInstances[i];
				if (aFoleyInstance->mRefCount != 0)
				{
					PvzpDSoundInstance* aSoundInstance = (PvzpDSoundInstance*)aFoleyInstance->mInstance;
					if (theEnteringPause)
					{
						aFoleyInstance->mPaused = true;
						{
							aFoleyInstance->mPauseOffset = 0;
							aSoundInstance->Stop();
						}
					}
					else if (aFoleyInstance->mPaused)
					{
						aFoleyInstance->mPaused = false;
						bool aIsLooping = TestBit(aFoleyParams->mFoleyFlags, FoleyFlags::FOLEYFLAGS_LOOP);
						aSoundInstance->Play(aIsLooping, false);
					}
				}
			}
		}
	}
}

void PvzpFoley::CancelPausedFoley()
{
	SoundSystemReleaseFinishedInstances(this);
	for (int aFoleyType = 0; aFoleyType < gFoleyParamArraySize; aFoleyType++)
	{
		FoleyTypeData* aFoleyData = &mFoleyTypeData[aFoleyType];
		for (int i = 0; i < MAX_FOLEY_INSTANCES; i++)
		{
			FoleyInstance* aFoleyInstance = &aFoleyData->mFoleyInstances[i];
			if (aFoleyInstance->mRefCount != 0 && aFoleyInstance->mPaused)
			{
				aFoleyInstance->mRefCount = 0;
				aFoleyInstance->mInstance->Release();
				aFoleyInstance->mInstance = nullptr;
			}
		}
	}
}

void PvzpFoley::ApplyMusicVolume(FoleyInstance* theFoleyInstance)
{
	if (gSexyAppBase->mSfxVolume < 1e-6)
		theFoleyInstance->mInstance->SetVolume(0.0);
	else
		theFoleyInstance->mInstance->SetVolume(gSexyAppBase->mMusicVolume / gSexyAppBase->mSfxVolume);  // so the volume equals the music volume once the sfx volume is applied
}

void PvzpFoley::RehookupSoundWithMusicVolume()
{
	SoundSystemReleaseFinishedInstances(this);
	for (int aFoleyType = 0; aFoleyType < gFoleyParamArraySize; aFoleyType++)
	{
		const FoleyParams* aFoleyParams = LookupFoley((FoleyType)aFoleyType);
		if (TestBit(aFoleyParams->mFoleyFlags, FoleyFlags::FOLEYFLAGS_USES_MUSIC_VOLUME))
		{
			FoleyTypeData* aFoleyData = &mFoleyTypeData[aFoleyType];
			for (int i = 0; i < MAX_FOLEY_INSTANCES; i++)
			{
				FoleyInstance* aFoleyInstance = &aFoleyData->mFoleyInstances[i];
				if (aFoleyInstance->mRefCount != 0)
					ApplyMusicVolume(aFoleyInstance);
			}
		}
	}
}

bool PvzpFoley::IsFoleyPlaying(FoleyType theFoleyType)
{
	SoundSystemReleaseFinishedInstances(this);
	return SoundSystemFindInstance(this, theFoleyType) != nullptr;
}
