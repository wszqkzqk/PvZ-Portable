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

#include "PvzpDebug.h"
#include "PvzpCommon.h"
#include "Definition.h"
#include "Reanimator.h"
#include "Attachment.h"
#include "SexyAppBase.h"
#include "../LawnApp.h"
#include "ReanimAtlas.h"
#include "EffectSystem.h"
#include "../GameConstants.h"
#include "graphics/Font.h"
#include "misc/PerfTimer.h"
#include "graphics/MemoryImage.h"
#include <algorithm>
#include <format>

constexpr const int NO_BASE_POSE = -2;

unsigned int gReanimatorDefCount;
std::unique_ptr<ReanimatorDefinition[]> gReanimatorDefArray;
unsigned int gReanimationParamArraySize;
const ReanimationParams* gReanimationParamArray;

constinit const ReanimationParams gLawnReanimationArray[ReanimationType::NUM_REANIMS] = {
	{ .mReanimationType = ReanimationType::REANIM_LOADBAR_SPROUT, .mReanimFileName = "reanim/LoadBar_sprout.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_LOADBAR_ZOMBIEHEAD, .mReanimFileName = "reanim/LoadBar_Zombiehead.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_SODROLL, .mReanimFileName = "reanim/SodRoll.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_FINAL_WAVE, .mReanimFileName = "reanim/FinalWave.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_PEASHOOTER, .mReanimFileName = "reanim/PeaShooterSingle.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_WALLNUT, .mReanimFileName = "reanim/Wallnut.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_LILYPAD, .mReanimFileName = "reanim/Lilypad.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_SUNFLOWER, .mReanimFileName = "reanim/SunFlower.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_LAWNMOWER, .mReanimFileName = "reanim/LawnMower.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_READYSETPLANT, .mReanimFileName = "reanim/StartReadySetPlant.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_CHERRYBOMB, .mReanimFileName = "reanim/CherryBomb.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_SQUASH, .mReanimFileName = "reanim/Squash.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_DOOMSHROOM, .mReanimFileName = "reanim/DoomShroom.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_SNOWPEA, .mReanimFileName = "reanim/SnowPea.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_REPEATER, .mReanimFileName = "reanim/PeaShooter.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_SUNSHROOM, .mReanimFileName = "reanim/SunShroom.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_TALLNUT, .mReanimFileName = "reanim/Tallnut.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_FUMESHROOM, .mReanimFileName = "reanim/Fumeshroom.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_PUFFSHROOM, .mReanimFileName = "reanim/Puffshroom.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_HYPNOSHROOM, .mReanimFileName = "reanim/Hypnoshroom.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_CHOMPER, .mReanimFileName = "reanim/Chomper.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_ZOMBIE, .mReanimFileName = "reanim/Zombie.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_SUN, .mReanimFileName = "reanim/Sun.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_POTATOMINE, .mReanimFileName = "reanim/PotatoMine.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_SPIKEWEED, .mReanimFileName = "reanim/Caltrop.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_SPIKEROCK, .mReanimFileName = "reanim/SpikeRock.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_THREEPEATER, .mReanimFileName = "reanim/ThreePeater.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_MARIGOLD, .mReanimFileName = "reanim/Marigold.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_ICESHROOM, .mReanimFileName = "reanim/IceShroom.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_ZOMBIE_FOOTBALL, .mReanimFileName = "reanim/Zombie_football.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_ZOMBIE_NEWSPAPER, .mReanimFileName = "reanim/Zombie_paper.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_ZOMBIE_ZAMBONI, .mReanimFileName = "reanim/Zombie_zamboni.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_SPLASH, .mReanimFileName = "reanim/splash.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_JALAPENO, .mReanimFileName = "reanim/Jalapeno.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_JALAPENO_FIRE, .mReanimFileName = "reanim/fire.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_COIN_SILVER, .mReanimFileName = "reanim/Coin_silver.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_ZOMBIE_CHARRED, .mReanimFileName = "reanim/Zombie_charred.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_ZOMBIE_CHARRED_IMP, .mReanimFileName = "reanim/Zombie_charred_imp.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_ZOMBIE_CHARRED_DIGGER, .mReanimFileName = "reanim/Zombie_charred_digger.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_ZOMBIE_CHARRED_ZAMBONI, .mReanimFileName = "reanim/Zombie_charred_zamboni.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_ZOMBIE_CHARRED_CATAPULT, .mReanimFileName = "reanim/Zombie_charred_catapult.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_ZOMBIE_CHARRED_GARGANTUAR, .mReanimFileName = "reanim/Zombie_charred_gargantuar.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_SCRAREYSHROOM, .mReanimFileName = "reanim/ScaredyShroom.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_PUMPKIN, .mReanimFileName = "reanim/Pumpkin.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_PLANTERN, .mReanimFileName = "reanim/Plantern.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_TORCHWOOD, .mReanimFileName = "reanim/Torchwood.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_SPLITPEA, .mReanimFileName = "reanim/SplitPea.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_SEASHROOM, .mReanimFileName = "reanim/SeaShroom.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_BLOVER, .mReanimFileName = "reanim/Blover.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_FLOWER_POT, .mReanimFileName = "reanim/Pot.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_CACTUS, .mReanimFileName = "reanim/Cactus.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_DANCER, .mReanimFileName = "reanim/Zombie_disco.reanim", .mReanimParamFlags = 0 }, // GOTY uses a different reanim file name
	{ .mReanimationType = ReanimationType::REANIM_TANGLEKELP, .mReanimFileName = "reanim/Tanglekelp.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_STARFRUIT, .mReanimFileName = "reanim/Starfruit.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_POLEVAULTER, .mReanimFileName = "reanim/Zombie_polevaulter.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_BALLOON, .mReanimFileName = "reanim/Zombie_balloon.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_GARGANTUAR, .mReanimFileName = "reanim/Zombie_gargantuar.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_IMP, .mReanimFileName = "reanim/Zombie_imp.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_DIGGER, .mReanimFileName = "reanim/Zombie_digger.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_DIGGER_DIRT, .mReanimFileName = "reanim/Digger_rising_dirt.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_ZOMBIE_DOLPHINRIDER, .mReanimFileName = "reanim/Zombie_dolphinrider.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_POGO, .mReanimFileName = "reanim/Zombie_pogo.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_BACKUP_DANCER, .mReanimFileName = "reanim/Zombie_backup.reanim", .mReanimParamFlags = 0 }, // GOTY uses a different reanim file name
	{ .mReanimationType = ReanimationType::REANIM_BOBSLED, .mReanimFileName = "reanim/Zombie_bobsled.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_JACKINTHEBOX, .mReanimFileName = "reanim/Zombie_jackbox.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_SNORKEL, .mReanimFileName = "reanim/Zombie_snorkle.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_BUNGEE, .mReanimFileName = "reanim/Zombie_bungi.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_CATAPULT, .mReanimFileName = "reanim/Zombie_catapult.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_LADDER, .mReanimFileName = "reanim/Zombie_ladder.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_PUFF, .mReanimFileName = "reanim/Puff.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_SLEEPING, .mReanimFileName = "reanim/Z.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_GRAVE_BUSTER, .mReanimFileName = "reanim/Gravebuster.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_ZOMBIES_WON, .mReanimFileName = "reanim/ZombiesWon.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_MAGNETSHROOM, .mReanimFileName = "reanim/Magnetshroom.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_BOSS, .mReanimFileName = "reanim/Zombie_boss.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_CABBAGEPULT, .mReanimFileName = "reanim/Cabbagepult.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_KERNELPULT, .mReanimFileName = "reanim/Cornpult.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_MELONPULT, .mReanimFileName = "reanim/Melonpult.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_COFFEEBEAN, .mReanimFileName = "reanim/Coffeebean.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_UMBRELLALEAF, .mReanimFileName = "reanim/Umbrellaleaf.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_GATLINGPEA, .mReanimFileName = "reanim/GatlingPea.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_CATTAIL, .mReanimFileName = "reanim/Cattail.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_GLOOMSHROOM, .mReanimFileName = "reanim/GloomShroom.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_BOSS_ICEBALL, .mReanimFileName = "reanim/Zombie_boss_iceball.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_BOSS_FIREBALL, .mReanimFileName = "reanim/Zombie_boss_fireball.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_COBCANNON, .mReanimFileName = "reanim/CobCannon.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_GARLIC, .mReanimFileName = "reanim/Garlic.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_GOLD_MAGNET, .mReanimFileName = "reanim/GoldMagnet.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_WINTER_MELON, .mReanimFileName = "reanim/WinterMelon.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_TWIN_SUNFLOWER, .mReanimFileName = "reanim/TwinSunflower.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_POOL_CLEANER, .mReanimFileName = "reanim/PoolCleaner.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_ROOF_CLEANER, .mReanimFileName = "reanim/RoofCleaner.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_FIRE_PEA, .mReanimFileName = "reanim/FirePea.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_IMITATER, .mReanimFileName = "reanim/Imitater.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_YETI, .mReanimFileName = "reanim/Zombie_yeti.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_BOSS_DRIVER, .mReanimFileName = "reanim/Zombie_Boss_driver.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_LAWN_MOWERED_ZOMBIE, .mReanimFileName = "reanim/LawnMoweredZombie.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_CRAZY_DAVE, .mReanimFileName = "reanim/CrazyDave.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_TEXT_FADE_ON, .mReanimFileName = "reanim/TextFadeOn.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_HAMMER, .mReanimFileName = "reanim/Hammer.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_SLOT_MACHINE_HANDLE, .mReanimFileName = "reanim/SlotMachine.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_CREDITS_FOOTBALL, .mReanimFileName = "reanim/Credits_Football.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_CREDITS_JACKBOX, .mReanimFileName = "reanim/Credits_Jackbox.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_SELECTOR_SCREEN, .mReanimFileName = "reanim/SelectorScreen.reanim", .mReanimParamFlags = 3 },
	{ .mReanimationType = ReanimationType::REANIM_PORTAL_CIRCLE, .mReanimFileName = "reanim/Portal_Circle.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_PORTAL_SQUARE, .mReanimFileName = "reanim/Portal_Square.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_ZENGARDEN_SPROUT, .mReanimFileName = "reanim/ZenGarden_sprout.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_ZENGARDEN_WATERINGCAN, .mReanimFileName = "reanim/ZenGarden_wateringcan.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_ZENGARDEN_FERTILIZER, .mReanimFileName = "reanim/ZenGarden_fertilizer.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_ZENGARDEN_BUGSPRAY, .mReanimFileName = "reanim/ZenGarden_bugspray.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_ZENGARDEN_PHONOGRAPH, .mReanimFileName = "reanim/ZenGarden_phonograph.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_DIAMOND, .mReanimFileName = "reanim/Diamond.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_ZOMBIE_HAND, .mReanimFileName = "reanim/Zombie_hand.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_STINKY, .mReanimFileName = "reanim/Stinky.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_RAKE, .mReanimFileName = "reanim/Rake.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_RAIN_CIRCLE, .mReanimFileName = "reanim/Rain_circle.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_RAIN_SPLASH, .mReanimFileName = "reanim/Rain_splash.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_ZOMBIE_SURPRISE, .mReanimFileName = "reanim/Zombie_surprise.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_COIN_GOLD, .mReanimFileName = "reanim/Coin_gold.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_TREEOFWISDOM, .mReanimFileName = "reanim/TreeOfWisdom.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_TREEOFWISDOM_CLOUDS, .mReanimFileName = "reanim/TreeOfWisdomClouds.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_TREEOFWISDOM_TREEFOOD, .mReanimFileName = "reanim/TreeFood.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_CREDITS_MAIN, .mReanimFileName = "reanim/Credits_Main.reanim", .mReanimParamFlags = 3 },
	{ .mReanimationType = ReanimationType::REANIM_CREDITS_MAIN2, .mReanimFileName = "reanim/Credits_Main2.reanim", .mReanimParamFlags = 3 },
	{ .mReanimationType = ReanimationType::REANIM_CREDITS_MAIN3, .mReanimFileName = "reanim/Credits_Main3.reanim", .mReanimParamFlags = 3 },
	{ .mReanimationType = ReanimationType::REANIM_ZOMBIE_CREDITS_DANCE, .mReanimFileName = "reanim/Zombie_credits_dance.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_CREDITS_STAGE, .mReanimFileName = "reanim/Credits_stage.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_CREDITS_BIGBRAIN, .mReanimFileName = "reanim/Credits_BigBrain.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_CREDITS_FLOWER_PETALS, .mReanimFileName = "reanim/Credits_Flower_petals.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_CREDITS_INFANTRY, .mReanimFileName = "reanim/Credits_Infantry.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_CREDITS_THROAT, .mReanimFileName = "reanim/Credits_Throat.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_CREDITS_CRAZYDAVE, .mReanimFileName = "reanim/Credits_CrazyDave.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_CREDITS_BOSSDANCE, .mReanimFileName = "reanim/Credits_Bossdance.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_ZOMBIE_CREDITS_SCREEN_DOOR, .mReanimFileName = "reanim/Zombie_Credits_Screendoor.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_ZOMBIE_CREDITS_CONEHEAD, .mReanimFileName = "reanim/Zombie_Credits_Conehead.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_CREDITS_ZOMBIEARMY1, .mReanimFileName = "reanim/Credits_ZombieArmy1.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_CREDITS_ZOMBIEARMY2, .mReanimFileName = "reanim/Credits_ZombieArmy2.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_CREDITS_TOMBSTONES, .mReanimFileName = "reanim/Credits_Tombstones.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_CREDITS_SOLARPOWER, .mReanimFileName = "reanim/Credits_SolarPower.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_CREDITS_ANYHOUR, .mReanimFileName = "reanim/Credits_Anyhour.reanim", .mReanimParamFlags = 3 },
	{ .mReanimationType = ReanimationType::REANIM_CREDITS_WEARETHEUNDEAD, .mReanimFileName = "reanim/Credits_WeAreTheUndead.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_CREDITS_DISCOLIGHTS, .mReanimFileName = "reanim/Credits_DiscoLights.reanim", .mReanimParamFlags = 1 },
	{ .mReanimationType = ReanimationType::REANIM_FLAG, .mReanimFileName = "reanim/Zombie_FlagPole.reanim", .mReanimParamFlags = 0 },
	{ .mReanimationType = ReanimationType::REANIM_ZOMBATAR_HEAD, .mReanimFileName = "reanim/zombatar_zombie_head.reanim", .mReanimParamFlags = 0 },
};

ReanimatorTransform::ReanimatorTransform() :
	mTransX(DEFAULT_FIELD_PLACEHOLDER),
	mTransY(DEFAULT_FIELD_PLACEHOLDER),
	mSkewX(DEFAULT_FIELD_PLACEHOLDER),
	mSkewY(DEFAULT_FIELD_PLACEHOLDER),
	mScaleX(DEFAULT_FIELD_PLACEHOLDER),
	mScaleY(DEFAULT_FIELD_PLACEHOLDER),
	mFrame(DEFAULT_FIELD_PLACEHOLDER),
	mAlpha(DEFAULT_FIELD_PLACEHOLDER),
	mImage(nullptr),
	mFont(nullptr),
	mText("") { }

inline void ReanimationFillInMissingData(float& thePrev, float& theValue)
{
	if (theValue == DEFAULT_FIELD_PLACEHOLDER)
		theValue = thePrev;  // unset: inherit the previous frame's value
	else
		thePrev = theValue;  // otherwise record the current value as the previous one
}

inline void ReanimationFillInMissingData(void*& thePrev, void*& theValue)
{
	if (theValue == nullptr)
		theValue = thePrev;
	else
		thePrev = theValue;
}

bool ReanimationLoadDefinition(const std::string& theFileName, ReanimatorDefinition* theDefinition)
{
	if (!DefinitionLoadXML(theFileName, &gReanimatorDefMap, theDefinition))
		return false;

	for (int aTrackIndex = 0; aTrackIndex < theDefinition->mTracks.count; aTrackIndex++)
	{
		ReanimatorTrack* aTrack = &theDefinition->mTracks.tracks[aTrackIndex];
		float aPrevTransX = 0.0f;
		float aPrevTransY = 0.0f;
		float aPrevSkewX = 0.0f;
		float aPrevSkewY = 0.0f;
		float aPrevScaleX = 1.0f;
		float aPrevScaleY = 1.0f;
		float aPrevFrame = 0.0f;
		float aPrevAlpha = 1.0f;
		Image* aPrevImage = nullptr;
		_Font* aPrevFont = nullptr;
		const char* aPrevText = "";

		// fill each frame's unset fields from the previous frame
		for (int i = 0; i < aTrack->mTransforms.count; i++)
		{
			ReanimatorTransform& aTransform = aTrack->mTransforms.mTransforms[i];
			ReanimationFillInMissingData(aPrevTransX, aTransform.mTransX);
			ReanimationFillInMissingData(aPrevTransY, aTransform.mTransY);
			ReanimationFillInMissingData(aPrevSkewX, aTransform.mSkewX);
			ReanimationFillInMissingData(aPrevSkewY, aTransform.mSkewY);
			ReanimationFillInMissingData(aPrevScaleX, aTransform.mScaleX);
			ReanimationFillInMissingData(aPrevScaleY, aTransform.mScaleY);
			ReanimationFillInMissingData(aPrevFrame, aTransform.mFrame);
			ReanimationFillInMissingData(aPrevAlpha, aTransform.mAlpha);
			ReanimationFillInMissingData((void*&)aPrevImage, (void*&)aTransform.mImage);
			ReanimationFillInMissingData((void*&)aPrevFont, (void*&)aTransform.mFont);
			if (*aTransform.mText == '\0')
				aTransform.mText = aPrevText;
			else
				aPrevText = aTransform.mText;
		}
	}
	return true;
}

void ReanimationFreeDefinition(ReanimatorDefinition* theDefinition)
{
	// free the atlas
	if (theDefinition->mReanimAtlas != nullptr)
	{
		delete theDefinition->mReanimAtlas;
		theDefinition->mReanimAtlas = nullptr;
	}

	// restore the definition data
	for (int aTrackIndex = 0; aTrackIndex < theDefinition->mTracks.count; aTrackIndex++)
	{
		ReanimatorTrack* aTrack = &theDefinition->mTracks.tracks[aTrackIndex];
		const char* aPrevText = nullptr;
		for (int i = 0; i < aTrack->mTransforms.count; i++)
		{
			ReanimatorTransform& aTransform = aTrack->mTransforms.mTransforms[i];
			if (*aTransform.mText != '\0' && aTransform.mText == aPrevText)
				aTransform.mText = "";
			else
				aPrevText = aTransform.mText;
		}
	}

	// free the definition
	DefinitionFreeMap(&gReanimatorDefMap, theDefinition);
}

ReanimatorTrackInstance::ReanimatorTrackInstance()
{
	mBlendCounter = 0;
	mBlendTime = 0;
	mShakeOverride = 0.0f;
	mShakeX = 0.0f;
	mShakeY = 0.0f;
	mAttachmentID = AttachmentID::ATTACHMENTID_NULL;
	mRenderGroup = RENDER_GROUP_NORMAL;
	mIgnoreClipRect = false;
	mImageOverride = nullptr;
	mTruncateDisappearingFrames = true;
	mTrackColor = Color::White;
	mIgnoreColorOverride = false;
	mIgnoreExtraAdditiveColor = false;
}

Reanimation::Reanimation()
{
	mAnimTime = 0;
	mAnimRate = 12.0f;
	mDefinition = nullptr;
	mLoopType = ReanimLoopType::REANIM_PLAY_ONCE;
	mLastFrameTime = -1.0f;
	mDead = false;
	mFrameStart = 0;
	mFrameCount = 0;
	mFrameBasePose = -1;
	mOverlayMatrix.LoadIdentity();
	mColorOverride = Color::White;
	mExtraAdditiveColor = Color::White;
	mEnableExtraAdditiveDraw = false;
	mExtraOverlayColor = Color::White;
	mEnableExtraOverlayDraw = false;
	mLoopCount = 0;
	mIsAttachment = false;
	mRenderOrder = 0;
	mReanimationHolder = nullptr;
	mTrackInstances = nullptr;
	mFilterEffect = FilterEffect::FILTER_EFFECT_NONE;
	mReanimationType = ReanimationType::REANIM_NONE;
}

Reanimation::~Reanimation()
{
	ReanimationDie();
	ReanimationDelete();
}

void Reanimation::ReanimationDelete()
{
	PVZP_ASSERT(mDead);
	if (mTrackInstances != nullptr)
	{
		int aItemSize = mDefinition->mTracks.count * sizeof(ReanimatorTrackInstance);
		FindGlobalAllocator(aItemSize)->Free(mTrackInstances, aItemSize);
		mTrackInstances = nullptr;
	}
}

void Reanimation::ReanimationInitializeType(float theX, float theY, ReanimationType theReanimType)
{
	PVZP_ASSERT(theReanimType >= 0 && theReanimType < gReanimatorDefCount);
	ReanimatorEnsureDefinitionLoaded(theReanimType, false);
	mReanimationType = theReanimType;
	ReanimationInitialize(theX, theY, &gReanimatorDefArray[theReanimType]);
}

void ReanimationCreateAtlas(ReanimatorDefinition* theDefinition, ReanimationType theReanimationType)
{
	const ReanimationParams& aParam = gReanimationParamArray[theReanimationType];
	if (theDefinition->mReanimAtlas != nullptr || TestBit(aParam.mReanimParamFlags, ReanimFlags::REANIM_NO_ATLAS))
		return;

	PerfTimer aTimer;
	aTimer.Start();
	PvzpHesitationTrace("preatlas");
	ReanimAtlas* aAtlas = new ReanimAtlas();
	theDefinition->mReanimAtlas = aAtlas;
	aAtlas->ReanimAtlasCreate(theDefinition);

	PvzpHesitationTrace("atlas '{}'", aParam.mReanimFileName);
	int aDuration = std::max(aTimer.GetDuration(), 0.0);
	if (aDuration > 20 && theReanimationType != ReanimationType::REANIM_NONE)  // report slow atlas creation
		PvzpLogLn("LOADING:Long atlas '{}' {} ms on {}", aParam.mReanimFileName, aDuration, LawnGetCurrentLevelName());
}

void ReanimationPreload(ReanimationType theReanimationType)
{
	PVZP_ASSERT(theReanimationType >= 0 && theReanimationType < gReanimatorDefCount);

	ReanimatorDefinition* aReanimDef = &gReanimatorDefArray[theReanimationType];
	ReanimationCreateAtlas(aReanimDef, theReanimationType);
	if (aReanimDef->mReanimAtlas)
	{
		PvzpSandImageIfNeeded(aReanimDef->mReanimAtlas->mMemoryImage.get());
	}
}

void Reanimation::ReanimationInitialize(float theX, float theY, ReanimatorDefinition* theDefinition)
{
	PVZP_ASSERT(mTrackInstances == nullptr);
	ReanimationCreateAtlas(theDefinition, mReanimationType);
	mDead = false;
	SetPosition(theX, theY);
	mDefinition = theDefinition;
	mAnimRate = theDefinition->mFPS;
	mLastFrameTime = -1.0f;

	if (theDefinition->mTracks.count != 0)
	{
		mFrameCount = mDefinition->mTracks.tracks[0].mTransforms.count;
		int aItemSize = theDefinition->mTracks.count * sizeof(ReanimatorTrackInstance);
		mTrackInstances = (ReanimatorTrackInstance*)FindGlobalAllocator(aItemSize)->Calloc(aItemSize);
		for (int aTrackIndex = 0; aTrackIndex < mDefinition->mTracks.count; aTrackIndex++)
		{
			ReanimatorTrackInstance* aTrack = &mTrackInstances[aTrackIndex];
			if (aTrack != nullptr)
				new (aTrack)ReanimatorTrackInstance();
		}
	}
	else
		mFrameCount = 0;
}

void Reanimation::Update()
{
	if (mFrameCount == 0 || mDead)
		return;

	PVZP_ASSERT(std::isfinite(mAnimRate));
	mLastFrameTime = mAnimTime;  // save the previous loop position
	mAnimTime += SECONDS_PER_UPDATE * mAnimRate / mFrameCount;  // advance the loop position

	if (mAnimRate > 0)
	{
		switch (mLoopType)
		{
		case ReanimLoopType::REANIM_LOOP:
		case ReanimLoopType::REANIM_LOOP_FULL_LAST_FRAME:
			while (mAnimTime >= 1.0f)
			{
				mLoopCount++;
				mAnimTime -= 1.0f;
			}
			break;
		case ReanimLoopType::REANIM_PLAY_ONCE:
		case ReanimLoopType::REANIM_PLAY_ONCE_FULL_LAST_FRAME:
			if (mAnimTime >= 1.0f)
			{
				mLoopCount = 1;
				mAnimTime = 1.0f;
				mDead = true;
			}
			break;
		case ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD:
		case ReanimLoopType::REANIM_PLAY_ONCE_FULL_LAST_FRAME_AND_HOLD:
			if (mAnimTime >= 1.0f)
			{
				mLoopCount = 1;
				mAnimTime = 1.0f;
			}
			break;
		default:
			PVZP_ASSERT(false);
			break;
		}
	}
	else
	{
		switch (mLoopType)
		{
		case ReanimLoopType::REANIM_LOOP:
		case ReanimLoopType::REANIM_LOOP_FULL_LAST_FRAME:
			while (mAnimTime < 0.0f)
			{
				mLoopCount++;
				mAnimTime += 1.0f;
			}
			break;
		case ReanimLoopType::REANIM_PLAY_ONCE:
		case ReanimLoopType::REANIM_PLAY_ONCE_FULL_LAST_FRAME:
			if (mAnimTime < 0.0f)
			{
				mLoopCount = 1;
				mAnimTime = 0.0f;
				mDead = true;
			}
			break;
		case ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD:
		case ReanimLoopType::REANIM_PLAY_ONCE_FULL_LAST_FRAME_AND_HOLD:
			if (mAnimTime < 0.0f)
			{
				mLoopCount = 1;
				mAnimTime = 0.0f;
			}
			break;
		default:
			PVZP_ASSERT(false);
			break;
		}
	}

	for (int aTrackIndex = 0; aTrackIndex < mDefinition->mTracks.count; aTrackIndex++)
	{
		ReanimatorTrackInstance* aTrack = &mTrackInstances[aTrackIndex];
		if (aTrack->mBlendCounter > 0)
			aTrack->mBlendCounter--;

		if (aTrack->mShakeOverride != 0.0f)
		{
			aTrack->mShakeX = RandRangeFloat(-aTrack->mShakeOverride, aTrack->mShakeOverride);
			aTrack->mShakeY = RandRangeFloat(-aTrack->mShakeOverride, aTrack->mShakeOverride);
		}

		if (strncasecmp(mDefinition->mTracks.tracks[aTrackIndex].mName, "attacher__", 10) == 0)  // IsAttacher
			UpdateAttacherTrack(aTrackIndex);

		if (aTrack->mAttachmentID != AttachmentID::ATTACHMENTID_NULL)
		{
			SexyTransform2D aOverlayMatrix;
			GetAttachmentOverlayMatrix(aTrackIndex, aOverlayMatrix);
			AttachmentUpdateAndSetMatrix(aTrack->mAttachmentID, aOverlayMatrix);
		}
	}
}

void BlendTransform(ReanimatorTransform* theResult, const ReanimatorTransform& theTransform1, const ReanimatorTransform& theTransform2, float theBlendFactor)
{
	theResult->mTransX = FloatLerp(theTransform1.mTransX, theTransform2.mTransX, theBlendFactor);
	theResult->mTransY = FloatLerp(theTransform1.mTransY, theTransform2.mTransY, theBlendFactor);
	theResult->mScaleX = FloatLerp(theTransform1.mScaleX, theTransform2.mScaleX, theBlendFactor);
	theResult->mScaleY = FloatLerp(theTransform1.mScaleY, theTransform2.mScaleY, theBlendFactor);
	theResult->mAlpha = FloatLerp(theTransform1.mAlpha, theTransform2.mAlpha, theBlendFactor);

	float aSkewX2 = theTransform2.mSkewX;
	float aSkewY2 = theTransform2.mSkewY;
	// when the skew differs by more than 180 degrees, theTransform2's skew is ignored
	while (aSkewX2 > theTransform1.mSkewX + 180.0f)
		aSkewX2 = theTransform1.mSkewX;  // (aSkewX2 -= 360.0f)
	while (aSkewX2 < theTransform1.mSkewX - 180.0f)
		aSkewX2 = theTransform1.mSkewX;  // (aSkewX2 += 360.0f)
	while (aSkewY2 > theTransform1.mSkewY + 180.0f)
		aSkewY2 = theTransform1.mSkewY;  // (aSkewY2 -= 360.0f)
	while (aSkewY2 < theTransform1.mSkewY - 180.0f)
		aSkewY2 = theTransform1.mSkewY;  // (aSkewY2 += 360.0f)

	theResult->mSkewX = FloatLerp(theTransform1.mSkewX, aSkewX2, theBlendFactor);
	theResult->mSkewY = FloatLerp(theTransform1.mSkewY, aSkewY2, theBlendFactor);
	theResult->mFrame = theTransform1.mFrame;
	theResult->mFont = theTransform1.mFont;
	theResult->mText = theTransform1.mText;
	theResult->mImage = theTransform1.mImage;
}

void Reanimation::GetCurrentTransform(int theTrackIndex, ReanimatorTransform* theTransformCurrent)
{
	ReanimatorFrameTime aFrameTime;
	GetFrameTime(&aFrameTime);
	GetTransformAtTime(theTrackIndex, theTransformCurrent, &aFrameTime);  // base transform interpolated between the two frames

	ReanimatorTrackInstance* aTrack = &mTrackInstances[theTrackIndex];
	if (FloatRoundToInt(theTransformCurrent->mFrame) >= 0 && aTrack->mBlendCounter > 0)  // not a blank frame and a blend is in progress
	{
		float aBlendFactor = aTrack->mBlendCounter / static_cast<float>(aTrack->mBlendTime);
		BlendTransform(theTransformCurrent, *theTransformCurrent, aTrack->mBlendTransform, aBlendFactor);  // blend with the recorded source transform
	}
}

void Reanimation::GetTransformAtTime(int theTrackIndex, ReanimatorTransform* theTransform, ReanimatorFrameTime* theFrameTime)
{
	PVZP_ASSERT(theTrackIndex >= 0 && theTrackIndex < mDefinition->mTracks.count);
	ReanimatorTrack* aTrack = &mDefinition->mTracks.tracks[theTrackIndex];
	PVZP_ASSERT(aTrack->mTransforms.count == mDefinition->mTracks.tracks[0].mTransforms.count);
	ReanimatorTransform& aTransBefore = aTrack->mTransforms.mTransforms[theFrameTime->mAnimFrameBeforeInt];  // previous frame's transform definition
	ReanimatorTransform& aTransAfter = aTrack->mTransforms.mTransforms[theFrameTime->mAnimFrameAfterInt];  // next frame's transform definition

	theTransform->mTransX = FloatLerp(aTransBefore.mTransX, aTransAfter.mTransX, theFrameTime->mFraction);
	theTransform->mTransY = FloatLerp(aTransBefore.mTransY, aTransAfter.mTransY, theFrameTime->mFraction);
	theTransform->mSkewX = FloatLerp(aTransBefore.mSkewX, aTransAfter.mSkewX, theFrameTime->mFraction);
	theTransform->mSkewY = FloatLerp(aTransBefore.mSkewY, aTransAfter.mSkewY, theFrameTime->mFraction);
	theTransform->mScaleX = FloatLerp(aTransBefore.mScaleX, aTransAfter.mScaleX, theFrameTime->mFraction);
	theTransform->mScaleY = FloatLerp(aTransBefore.mScaleY, aTransAfter.mScaleY, theFrameTime->mFraction);
	theTransform->mAlpha = FloatLerp(aTransBefore.mAlpha, aTransAfter.mAlpha, theFrameTime->mFraction);
	theTransform->mImage = aTransBefore.mImage;
	theTransform->mFont = aTransBefore.mFont;
	theTransform->mText = aTransBefore.mText;

	if (aTransBefore.mFrame != -1.0f && aTransAfter.mFrame == -1.0f && theFrameTime->mFraction > 0.0f && mTrackInstances[theTrackIndex].mTruncateDisappearingFrames)
		theTransform->mFrame = -1.0f;  // cut the transition to a blank frame when the track truncates disappearing frames
	else
		theTransform->mFrame = aTransBefore.mFrame;
}

void Reanimation::MatrixFromTransform(const ReanimatorTransform& theTransform, SexyMatrix3& theMatrix)
{
	float aSkewX = -DEG_TO_RAD(theTransform.mSkewX);
	float aSkewY = -DEG_TO_RAD(theTransform.mSkewY);

	theMatrix.m00 = cos(aSkewX) * theTransform.mScaleX;
	theMatrix.m10 = -sin(aSkewX) * theTransform.mScaleX;
	theMatrix.m20 = 0.0f;
	theMatrix.m01 = sin(aSkewY) * theTransform.mScaleY;
	theMatrix.m11 = cos(aSkewY) * theTransform.mScaleY;
	theMatrix.m21 = 0.0f;
	theMatrix.m02 = theTransform.mTransX;
	theMatrix.m12 = theTransform.mTransY;
	theMatrix.m22 = 1.0f;
}

void Reanimation::ReanimBltMatrix(Graphics* g, Image* theImage, SexyMatrix3& theTransform, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect& theSrcRect)
{
	if (!gSexyAppBase->Is3DAccelerated() &&
		TestBit(gReanimationParamArray[mReanimationType].mReanimParamFlags, static_cast<int>(ReanimFlags::REANIM_FAST_DRAW_IN_SW_MODE)) &&
		FloatApproxEqual(theTransform.m01, 0.0f) && FloatApproxEqual(theTransform.m10, 0.0f) &&  // no horizontal or vertical skew
		theTransform.m00 > 0.0f && theTransform.m11 > 0.0f &&  // positive horizontal and vertical scale
		theColor == Color::White)
	{
		float aScaleX = theTransform.m00;
		float aScaleY = theTransform.m11;
		int aPosX = FloatRoundToInt(theTransform.m02 - aScaleX * theSrcRect.mWidth * 0.5f);
		int aPosY = FloatRoundToInt(theTransform.m12 - aScaleY * theSrcRect.mHeight * 0.5f);
		int aOldMode = g->GetDrawMode();
		g->SetDrawMode(theDrawMode);
		Rect aOldClipRect = g->mClipRect;
		g->SetClipRect(theClipRect);

		if (FloatApproxEqual(aScaleX, 1.0f) && FloatApproxEqual(aScaleY, 1.0f))  // no scaling
			g->DrawImage(theImage, aPosX, aPosY, theSrcRect);
		else
		{
			int aWidth = FloatRoundToInt(aScaleX * theSrcRect.mWidth);
			int aHeight = FloatRoundToInt(aScaleY * theSrcRect.mHeight);
			Rect aDestRect(aPosX, aPosY, aWidth, aHeight);
			g->DrawImage(theImage, aDestRect, theSrcRect);
		}

		g->SetDrawMode(aOldMode);
		g->SetClipRect(aOldClipRect);
	}
	else
		PvzpBltMatrix(g, theImage, theTransform, theClipRect, theColor, theDrawMode, theSrcRect);
}

bool Reanimation::DrawTrack(Graphics* g, int theTrackIndex, [[maybe_unused]] int theRenderGroup, PvzpTriangleGroup* theTriangleGroup)
{
	ReanimatorTransform aTransform;
	ReanimatorTrackInstance* aTrackInstance = &mTrackInstances[theTrackIndex];
	GetCurrentTransform(theTrackIndex, &aTransform);
	int aImageFrame = FloatRoundToInt(aTransform.mFrame);  // cel index within the image
	if (aImageFrame < 0)  // no image to draw
		return false;

	Color aColor = aTrackInstance->mTrackColor;
	if (!aTrackInstance->mIgnoreColorOverride)
	{
		aColor = ColorsMultiply(aColor, mColorOverride);
	}
	if (g->GetColorizeImages())
	{
		aColor = ColorsMultiply(aColor, g->GetColor());
	}
	int aImageAlpha = std::clamp(FloatRoundToInt(aTransform.mAlpha * aColor.mAlpha), 0, 255);
	if (aImageAlpha <= 0)
	{
		return false;
	}
	aColor.mAlpha = aImageAlpha;

	Color aExtraAdditiveColor;
	if (mEnableExtraAdditiveDraw)
	{
		aExtraAdditiveColor = mExtraAdditiveColor;
		aExtraAdditiveColor.mAlpha = ColorComponentMultiply(mExtraAdditiveColor.mAlpha, aImageAlpha);
	}
	Color aExtraOverlayColor;
	if (mEnableExtraOverlayDraw)
	{
		aExtraOverlayColor = mExtraOverlayColor;
		aExtraOverlayColor.mAlpha = ColorComponentMultiply(mExtraOverlayColor.mAlpha, aImageAlpha);
	}

	Rect aClipRect = g->mClipRect;
	if (aTrackInstance->mIgnoreClipRect)
	{
		aClipRect = Rect(0, 0, BOARD_WIDTH, BOARD_HEIGHT);
	}

	Image* aImage = aTransform.mImage;
	ReanimAtlasImage* aAtlasImage = nullptr;
	// Extract pivot info from the original track image before any override / cleanup logic.
	Image* aPivotImage = aImage;
	ReanimAtlasImage* aPivotAtlasImage = nullptr;
	if (mDefinition->mReanimAtlas != nullptr && aPivotImage != nullptr)
	{
		aPivotAtlasImage = mDefinition->mReanimAtlas->GetEncodedReanimAtlas(aPivotImage);
		if (aPivotAtlasImage == nullptr && reinterpret_cast<uintptr_t>(aPivotImage) <= 1000)
			aPivotImage = nullptr;  // Invalid encoded handle.
	}

	if (mDefinition->mReanimAtlas != nullptr && aImage != nullptr)
	{
		aAtlasImage = mDefinition->mReanimAtlas->GetEncodedReanimAtlas(aImage);  // Decode atlas handle from transform image.
		if (aTrackInstance->mImageOverride != nullptr)
		{
			aImage = aTrackInstance->mImageOverride;
			aAtlasImage = nullptr;
		}
		else if (aAtlasImage == nullptr && reinterpret_cast<uintptr_t>(aImage) <= 1000)
		{
			aImage = nullptr;  // Invalid encoded handle; never treat it as a raw Image*.
		}
	}
	else if (aTrackInstance->mImageOverride != nullptr)
	{
		aImage = aTrackInstance->mImageOverride;
	}

	SexyMatrix3 aMatrix;
	bool aFullScreen = false;
	if (aPivotAtlasImage != nullptr)
	{
		aMatrix.LoadIdentity();
		SexyMatrix3Translation(aMatrix, aPivotAtlasImage->mWidth * 0.5f, aPivotAtlasImage->mHeight * 0.5f);  // Use original atlas metadata for pivot sizing.
	}
	else if (aPivotImage != nullptr)
	{
		int aCelWidth = aPivotImage->GetCelWidth();
		int aCelHeight = aPivotImage->GetCelHeight();
		aMatrix.LoadIdentity();
		SexyMatrix3Translation(aMatrix, aCelWidth * 0.5f, aCelHeight * 0.5f);
	}
	else if (aImage != nullptr)
	{
		int aCelWidth = aImage->GetCelWidth();
		int aCelHeight = aImage->GetCelHeight();
		aMatrix.LoadIdentity();
		SexyMatrix3Translation(aMatrix, aCelWidth * 0.5f, aCelHeight * 0.5f);
	}
	else if (aTransform.mFont != nullptr && *aTransform.mText != '\0')
	{
		aMatrix.LoadIdentity();
		int aWidth = aTransform.mFont->StringWidth(aTransform.mText);
		SexyMatrix3Translation(aMatrix, -aWidth * 0.5f, aTransform.mFont->mAscent);
	}
	else
	{
		if (strcasecmp(mDefinition->mTracks.tracks[theTrackIndex].mName, "fullscreen"))  // no image and no text, and not the fullscreen track
			return false;  // nothing to draw
		aFullScreen = true;  // a screen-sized rect is filled later
	}

	if (mDefinition->mReanimAtlas != nullptr && aAtlasImage == nullptr)  // atlas exists but this track does not use it
		theTriangleGroup->DrawGroup(g);  // flush the accumulated triangles first

	SexyMatrix3 aTransformMatrix;
	MatrixFromTransform(aTransform, aTransformMatrix);
	SexyMatrix3Multiply(aMatrix, aTransformMatrix, aMatrix);  // apply the track transform
	SexyMatrix3Multiply(aMatrix, mOverlayMatrix, aMatrix);  // apply the overlay matrix
	SexyMatrix3Translation(aMatrix, aTrackInstance->mShakeX + g->mTransX, aTrackInstance->mShakeY + g->mTransY);  // apply track shake and g's translation

	if (aAtlasImage != nullptr)  // atlas exists, the frame has an image, and no override is set
	{
		Rect aSrcRect(aAtlasImage->mX, aAtlasImage->mY, aAtlasImage->mWidth, aAtlasImage->mHeight);
		aImage = mDefinition->mReanimAtlas->mMemoryImage.get();
		if (mFilterEffect != FilterEffect::FILTER_EFFECT_NONE)
		{
			aImage = FilterEffectGetImage(aImage, mFilterEffect);
		}
		theTriangleGroup->AddTriangle(g, aImage, aMatrix, aClipRect, aColor, g->mDrawMode, aSrcRect);
		if (mEnableExtraAdditiveDraw && !aTrackInstance->mIgnoreExtraAdditiveColor)
		{
			theTriangleGroup->AddTriangle(g, aImage, aMatrix, aClipRect, aExtraAdditiveColor, Graphics::DRAWMODE_ADDITIVE, aSrcRect);
		}
		if (mEnableExtraOverlayDraw)
		{
			theTriangleGroup->AddTriangle(
				g, FilterEffectGetImage(aImage, FilterEffect::FILTER_EFFECT_WHITE), aMatrix, aClipRect, aExtraOverlayColor, Graphics::DRAWMODE_NORMAL, aSrcRect);
		}
	}
	else if (aImage != nullptr)
	{
		if (mFilterEffect != FilterEffect::FILTER_EFFECT_NONE)
		{
			aImage = FilterEffectGetImage(aImage, mFilterEffect);
		}

		while (aImageFrame >= aImage->mNumCols)
		{
			aImageFrame -= aImage->mNumCols;
		}

		int aCelWidth = aImage->GetCelWidth();
		Rect aSrcRect(aImageFrame * aCelWidth, 0, aCelWidth, aImage->GetCelHeight());
		ReanimBltMatrix(g, aImage, aMatrix, aClipRect, aColor, g->mDrawMode, aSrcRect);
		if (mEnableExtraAdditiveDraw)
		{
			ReanimBltMatrix(g, aImage, aMatrix, aClipRect, aExtraAdditiveColor, Graphics::DRAWMODE_ADDITIVE, aSrcRect);
		}
		if (mEnableExtraOverlayDraw)
		{
			Image* aOverlayImage = FilterEffectGetImage(aImage, FilterEffect::FILTER_EFFECT_WHITE);
			ReanimBltMatrix(g, aOverlayImage, aMatrix, aClipRect, aExtraOverlayColor, Graphics::DRAWMODE_NORMAL, aSrcRect);
		}
	}
	else if (aTransform.mFont != nullptr && *aTransform.mText != '\0')
	{
		PvzpDrawStringMatrix(g, aTransform.mFont, aMatrix, aTransform.mText, aColor);
		if (mEnableExtraAdditiveDraw)
		{
			int aOldMode = g->GetDrawMode();
			g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
			PvzpDrawStringMatrix(g, aTransform.mFont, aMatrix, aTransform.mText, aExtraAdditiveColor);
			g->SetDrawMode(aOldMode);
		}
	}
	else if (aFullScreen)
	{
		Color aOldColor = g->GetColor();
		g->SetColor(aColor);
		g->FillRect(-g->mTransX, -g->mTransY, BOARD_WIDTH, BOARD_HEIGHT);
		g->SetColor(aOldColor);
	}
	return true;
}

Image* Reanimation::GetCurrentTrackImage(const char* theTrackName)
{
	int aTrackIndex = FindTrackIndex(theTrackName);
	ReanimatorTrackInstance* aTrackInstance = &mTrackInstances[aTrackIndex];
	if (aTrackInstance->mImageOverride != nullptr)
		return aTrackInstance->mImageOverride;

	ReanimatorTransform aTransform;
	GetCurrentTransform(aTrackIndex, &aTransform);

	Image* aImage = aTransform.mImage;
	if (mDefinition->mReanimAtlas != nullptr && aImage != nullptr && mDefinition->mReanimAtlas->GetEncodedReanimAtlas(aImage) != nullptr)
	{
		// Encoded atlas handles do not map to stable source-image pointers at runtime.
		aImage = nullptr;
	}
	return aImage;
}

void Reanimation::GetTrackMatrix(int theTrackIndex, SexyTransform2D& theMatrix)
{
	ReanimatorTrackInstance* aTrackInstance = &mTrackInstances[theTrackIndex];
	ReanimatorTransform aTransform;
	GetCurrentTransform(theTrackIndex, &aTransform);
	int aImageFrame = FloatRoundToInt(aTransform.mFrame);
	Image* aImage = aTransform.mImage;
	ReanimAtlasImage* aAtlasImage = nullptr;
	// Extract pivot info from the original track image before any override / cleanup logic.
	Image* aPivotImage = aImage;
	ReanimAtlasImage* aPivotAtlasImage = nullptr;
	if (mDefinition->mReanimAtlas != nullptr && aPivotImage != nullptr)
	{
		aPivotAtlasImage = mDefinition->mReanimAtlas->GetEncodedReanimAtlas(aPivotImage);
		if (aPivotAtlasImage == nullptr && reinterpret_cast<uintptr_t>(aPivotImage) <= 1000)
			aPivotImage = nullptr;  // Invalid encoded handle.
	}

	if (mDefinition->mReanimAtlas != nullptr && aImage != nullptr)
	{
		aAtlasImage = mDefinition->mReanimAtlas->GetEncodedReanimAtlas(aImage);  // Decode atlas handle from transform image.
		if (aAtlasImage == nullptr && reinterpret_cast<uintptr_t>(aImage) <= 1000)
			aImage = nullptr;  // Invalid encoded handle; keep non-atlas path safe.
	}
	if (aTrackInstance->mImageOverride != nullptr)
	{
		aImage = aTrackInstance->mImageOverride;
		aAtlasImage = nullptr;
	}

	theMatrix.LoadIdentity();
	if (aPivotAtlasImage != nullptr && aImageFrame >= 0)
	{
		SexyMatrix3Translation(theMatrix, aPivotAtlasImage->mWidth * 0.5f, aPivotAtlasImage->mHeight * 0.5f);
	}
	else if (aPivotImage != nullptr && aImageFrame >= 0)
	{
		int aCelWidth = aPivotImage->GetCelWidth();
		int aCelHeight = aPivotImage->GetCelHeight();
		SexyMatrix3Translation(theMatrix, aCelWidth * 0.5f, aCelHeight * 0.5f);
	}
	else if (aImage != nullptr && aImageFrame >= 0)
	{
		int aCelWidth = aImage->GetCelWidth();
		int aCelHeight = aImage->GetCelHeight();
		SexyMatrix3Translation(theMatrix, aCelWidth * 0.5f, aCelHeight * 0.5f);
	}
	else if (aTransform.mFont != nullptr && *aTransform.mText != '\0')
		SexyMatrix3Translation(theMatrix, 0.0f, aTransform.mFont->mAscent);

	SexyTransform2D aTransformMatrix;
	MatrixFromTransform(aTransform, aTransformMatrix);
	SexyMatrix3Multiply(theMatrix, aTransformMatrix, theMatrix);  // apply the track transform
	SexyMatrix3Multiply(theMatrix, mOverlayMatrix, theMatrix);  // apply the overlay matrix
	SexyMatrix3Translation(theMatrix, aTrackInstance->mShakeX - 0.5f, aTrackInstance->mShakeY - 0.5f);  // apply track shake
}

void Reanimation::GetFrameTime(ReanimatorFrameTime* theFrameTime)
{
	PVZP_ASSERT(mFrameStart + mFrameCount <= mDefinition->mTracks.tracks[0].mTransforms.count);
	int aFrameCount;
	if (mLoopType == ReanimLoopType::REANIM_PLAY_ONCE_FULL_LAST_FRAME || mLoopType == ReanimLoopType::REANIM_LOOP_FULL_LAST_FRAME ||
		mLoopType == ReanimLoopType::REANIM_PLAY_ONCE_FULL_LAST_FRAME_AND_HOLD)
		aFrameCount = mFrameCount;
	else
		aFrameCount = mFrameCount - 1;
	float aAnimPosition = mFrameStart + mAnimTime * aFrameCount;
	float aAnimFrameBefore = floor(aAnimPosition);
	theFrameTime->mFraction = aAnimPosition - aAnimFrameBefore;
	theFrameTime->mAnimFrameBeforeInt = FloatRoundToInt(aAnimFrameBefore);
	if (theFrameTime->mAnimFrameBeforeInt >= mFrameStart + mFrameCount - 1)  // on the last frame
	{
		theFrameTime->mAnimFrameBeforeInt = mFrameStart + mFrameCount - 1;
		theFrameTime->mAnimFrameAfterInt = theFrameTime->mAnimFrameBeforeInt;
	}
	else
		theFrameTime->mAnimFrameAfterInt = theFrameTime->mAnimFrameBeforeInt + 1;
	PVZP_ASSERT(theFrameTime->mAnimFrameBeforeInt >= 0 && theFrameTime->mAnimFrameAfterInt < mDefinition->mTracks.tracks[0].mTransforms.count);
}

void Reanimation::DrawRenderGroup(Graphics* g, int theRenderGroup)
{
	if (mDead)
		return;

	PvzpTriangleGroup aTriangleGroup;
	for (int aTrackIndex = 0; aTrackIndex < mDefinition->mTracks.count; aTrackIndex++)
	{
		ReanimatorTrackInstance* aTrackInstance = &mTrackInstances[aTrackIndex];
		if (aTrackInstance->mRenderGroup == theRenderGroup)
		{
			bool aTrackDrawn = DrawTrack(g, aTrackIndex, theRenderGroup, &aTriangleGroup);
			if (aTrackInstance->mAttachmentID != AttachmentID::ATTACHMENTID_NULL)
			{
				aTriangleGroup.DrawGroup(g);
				AttachmentDraw(aTrackInstance->mAttachmentID, g, !aTrackDrawn);
			}
		}
	}
	aTriangleGroup.DrawGroup(g);
}

void Reanimation::Draw(Graphics* g)
{
	DrawRenderGroup(g, RENDER_GROUP_NORMAL);
}

void Reanimation::DrawAllRenderGroups(Graphics* g)
{
	for (int aRenderGroup = RENDER_GROUP_NORMAL; aRenderGroup <= RENDER_GROUP_MAX; aRenderGroup++)
		DrawRenderGroup(g, aRenderGroup);
}

int Reanimation::FindTrackIndex(const char* theTrackName)
{
	for (int aTrackIndex = 0; aTrackIndex < mDefinition->mTracks.count; aTrackIndex++)
		if (strcasecmp(mDefinition->mTracks.tracks[aTrackIndex].mName, theTrackName) == 0)
			return aTrackIndex;

	PvzpLogLn("Can't find track '{}'", theTrackName);
	return 0;
}

ReanimatorTrackInstance* Reanimation::GetTrackInstanceByName(const char* theTrackName)
{
	return &mTrackInstances[FindTrackIndex(theTrackName)];
}

void Reanimation::AttachToAnotherReanimation(Reanimation* theAttachReanim, const char* theTrackName)
{
	if (theAttachReanim->mDefinition->mTracks.count <= 0)
		return;

	if (theAttachReanim->mFrameBasePose == -1)
		theAttachReanim->mFrameBasePose = theAttachReanim->mFrameStart;  // use the current animation's start frame as the base pose
	AttachReanim(theAttachReanim->GetTrackInstanceByName(theTrackName)->mAttachmentID, this, 0.0f, 0.0f);
}

void Reanimation::SetBasePoseFromAnim(const char* theTrackName)
{
	int aFrameStart, aFrameCount;
	GetFramesForLayer(theTrackName, aFrameStart, aFrameCount);
	mFrameBasePose = aFrameStart;  // use the track animation's start frame as the base pose
}

void Reanimation::GetTrackBasePoseMatrix(int theTrackIndex, SexyTransform2D& theBasePosMatrix)
{
	if (mFrameBasePose == NO_BASE_POSE)
	{
		theBasePosMatrix.LoadIdentity();
		return;
	}

	int aBasePos = mFrameBasePose == -1 ? mFrameStart : mFrameBasePose;
	ReanimatorFrameTime aStartTime = { 0.0f, aBasePos, aBasePos + 1 };
	ReanimatorTransform aTransformStart;
	GetTransformAtTime(theTrackIndex, &aTransformStart, &aStartTime);
	MatrixFromTransform(aTransformStart, theBasePosMatrix);
}

AttachEffect* Reanimation::AttachParticleToTrack(const char* theTrackName, PvzpParticleSystem* theParticleSystem, float thePosX, float thePosY)
{
	int aTrackIndex = FindTrackIndex(theTrackName);
	ReanimatorTrackInstance* aTrackInstance = &mTrackInstances[aTrackIndex];
	SexyTransform2D aBasePoseMatrix;
	GetTrackBasePoseMatrix(aTrackIndex, aBasePoseMatrix);
	SexyVector2 aPosition = aBasePoseMatrix * SexyVector2(thePosX, thePosY);
	return AttachParticle(aTrackInstance->mAttachmentID, theParticleSystem, aPosition.x, aPosition.y);
}

void Reanimation::GetAttachmentOverlayMatrix(int theTrackIndex, SexyTransform2D& theOverlayMatrix)
{
	ReanimatorTransform aTransform;
	GetCurrentTransform(theTrackIndex, &aTransform);  // blended transform without overrides
	SexyTransform2D aTransformMatrix;
	MatrixFromTransform(aTransform, aTransformMatrix);
	SexyMatrix3Multiply(aTransformMatrix, mOverlayMatrix, aTransformMatrix);  // apply the overlay matrix

	SexyTransform2D aBasePoseMatrix;
	GetTrackBasePoseMatrix(theTrackIndex, aBasePoseMatrix);
	SexyTransform2D aBasePoseMatrixInv;
	SexyMatrix3Inverse(aBasePoseMatrix, aBasePoseMatrixInv);
	theOverlayMatrix = aTransformMatrix * aBasePoseMatrixInv;
}

void Reanimation::GetFramesForLayer(const char* theTrackName, int& theFrameStart, int& theFrameCount)
{
	if (mDefinition->mTracks.count == 0)
	{
		theFrameStart = 0;
		theFrameCount = 0;
		return;
	}

	int aTrackIndex = FindTrackIndex(theTrackName);
	PVZP_ASSERT(aTrackIndex >= 0 && aTrackIndex < mDefinition->mTracks.count);
	ReanimatorTrack* aTrack = &mDefinition->mTracks.tracks[aTrackIndex];
	theFrameStart = 0;
	theFrameCount = 1;
	for (int i = 0; i < aTrack->mTransforms.count; i++)
		if (aTrack->mTransforms.mTransforms[i].mFrame >= 0.0f)
		{
			theFrameStart = i;  // first non-blank frame
			break;
		}
	for (int j = theFrameStart; j < aTrack->mTransforms.count; j++)
		if (aTrack->mTransforms.mTransforms[j].mFrame >= 0.0f)
			theFrameCount = j - theFrameStart + 1;  // span from start to the last non-blank frame
}

void Reanimation::SetFramesForLayer(const char* theTrackName)
{
	if (mAnimRate >= 0)
		mAnimTime = 0.0f;
	else
		mAnimTime = 0.9999999f;
	mLastFrameTime = -1.0f;
	GetFramesForLayer(theTrackName, mFrameStart, mFrameCount);
}

bool Reanimation::TrackExists(const char* theTrackName)
{
	for (int aTrackIndex = 0; aTrackIndex < mDefinition->mTracks.count; aTrackIndex++)
		if (strcasecmp(mDefinition->mTracks.tracks[aTrackIndex].mName, theTrackName) == 0)
			return true;
	return false;
}

void Reanimation::StartBlend(int theBlendTime)
{
	for (int aTrackIndex = 0; aTrackIndex < mDefinition->mTracks.count; aTrackIndex++)
	{
		ReanimatorTransform aTransform;
		GetCurrentTransform(aTrackIndex, &aTransform);
		if (FloatRoundToInt(aTransform.mFrame) >= 0)  // not on a blank frame
		{
			ReanimatorTrackInstance* aTrackInstance = &mTrackInstances[aTrackIndex];
			aTrackInstance->mBlendTransform = aTransform;  // record the current transform as the blend source
			aTrackInstance->mBlendTime = theBlendTime;
			aTrackInstance->mBlendCounter = theBlendTime;
			aTrackInstance->mBlendTransform.mFont = nullptr;
			aTrackInstance->mBlendTransform.mText = "";
			aTrackInstance->mBlendTransform.mImage = nullptr;
		}
	}
}

void Reanimation::ReanimationDie()
{
	if (!mDead)
	{
		mDead = true;
		if (mDefinition == nullptr)
			return;
		for (int aTrackIndex = 0; aTrackIndex < mDefinition->mTracks.count; aTrackIndex++)
		{
			PVZP_ASSERT(mTrackInstances);
			AttachmentDie(mTrackInstances[aTrackIndex].mAttachmentID);
		}
	}
}

void Reanimation::SetShakeOverride(const char* theTrackName, float theShakeAmount)
{
	GetTrackInstanceByName(theTrackName)->mShakeOverride = theShakeAmount;
}

void Reanimation::SetPosition(float theX, float theY)
{
	mOverlayMatrix.m02 = theX;
	mOverlayMatrix.m12 = theY;
}

void Reanimation::OverrideScale(float theScaleX, float theScaleY)
{
	mOverlayMatrix.m00 = theScaleX;
	mOverlayMatrix.m11 = theScaleY;
}

Image* Reanimation::GetImageOverride(const char* theTrackName)
{
	return GetTrackInstanceByName(theTrackName)->mImageOverride;
}

void Reanimation::SetImageOverride(const char* theTrackName, Image* theImage)
{
	GetTrackInstanceByName(theTrackName)->mImageOverride = theImage;
}

void Reanimation::SetTruncateDisappearingFrames(const char* theTrackName, bool theTruncateDisappearingFrames)
{
	if (theTrackName == nullptr)
	{
		for (int aTrackIndex = 0; aTrackIndex < mDefinition->mTracks.count; aTrackIndex++)
			mTrackInstances[aTrackIndex].mTruncateDisappearingFrames = theTruncateDisappearingFrames;
	}
	else
		GetTrackInstanceByName(theTrackName)->mTruncateDisappearingFrames = theTruncateDisappearingFrames;
}

void ReanimationHolder::DisposeHolder()
{
	mReanimations.DataArrayDispose();
}

ReanimationHolder::~ReanimationHolder()
{
	DisposeHolder();
}

void ReanimationHolder::InitializeHolder()
{
	mReanimations.DataArrayInitialize(1024U, "reanims");
}

Reanimation* ReanimationHolder::AllocReanimation(float theX, float theY, int theRenderOrder, ReanimationType theReanimationType)
{
	PVZP_ASSERT(mReanimations.mSize != mReanimations.mMaxSize);
	Reanimation* aReanim = mReanimations.DataArrayAlloc();
	aReanim->mRenderOrder = theRenderOrder;
	aReanim->mReanimationHolder = this;
	aReanim->ReanimationInitializeType(theX, theY, theReanimationType);
	return aReanim;
}

void ReanimatorEnsureDefinitionLoaded(ReanimationType theReanimType, bool theIsPreloading)
{
	PVZP_ASSERT(theReanimType >= 0 && theReanimType < gReanimatorDefCount);
	ReanimatorDefinition* aReanimDef = &gReanimatorDefArray[theReanimType];
	if (aReanimDef->mTracks.tracks != nullptr)  // non-null tracks means the definition is already loaded
		return;
	const ReanimationParams* aReanimParams = &gReanimationParamArray[theReanimType];
	PvzpLogLn("'{}'", aReanimParams->mReanimFileName);
	if (theIsPreloading)
	{
		if (gSexyAppBase->mShutdown || LawnGetCloseRequest())  // abort preloading when the app is shutting down
			return;
	}
	else
	{
		if (LawnHasUsedCheatKeys())
			PvzpLogLn("Cheater failed to preload '{}' on {}", aReanimParams->mReanimFileName, LawnGetCurrentLevelName());
		else
			PvzpLogLn("Non-cheater failed to preload '{}' on {}", aReanimParams->mReanimFileName, LawnGetCurrentLevelName());
	}

	PerfTimer aTimer;
	aTimer.Start();
	PvzpHesitationBracket aHesitation("Load Reanim '{}'", aReanimParams->mReanimFileName);
	if (!ReanimationLoadDefinition(aReanimParams->mReanimFileName, aReanimDef))
	{
		PvzpErrorMessageBox(std::format("Failed to load reanim '{}'", aReanimParams->mReanimFileName), "Error");
	}
	int aDuration = aTimer.GetDuration();
	if (aDuration > 100)  // report slow reanim loading
		PvzpLogLn("LOADING:Long reanim '{}' {} ms on {}", aReanimParams->mReanimFileName, aDuration, LawnGetCurrentLevelName());
}

void ReanimatorLoadDefinitions(const ReanimationParams* theReanimationParamArray, int theReanimationParamArraySize)
{
	PvzpHesitationBracket aHesitation("ReanimatorLoadDefinitions");
	PVZP_ASSERT(!gReanimationParamArray && !gReanimatorDefArray);
	gReanimationParamArraySize = theReanimationParamArraySize;
	gReanimationParamArray = theReanimationParamArray;
	gReanimatorDefCount = theReanimationParamArraySize;
	gReanimatorDefArray = std::make_unique<ReanimatorDefinition[]>(theReanimationParamArraySize);

#ifndef LOW_MEMORY
	for (unsigned int i = 0; i < gReanimationParamArraySize; i++)
	{
		const ReanimationParams* aReanimationParams = &theReanimationParamArray[i];
		PVZP_ASSERT(aReanimationParams->mReanimationType == i);
		if (DefinitionIsCompiled(aReanimationParams->mReanimFileName))
			ReanimatorEnsureDefinitionLoaded(aReanimationParams->mReanimationType, true);
	}
#endif
}

void ReanimatorFreeDefinitions()
{
	for (unsigned int i = 0; i < gReanimatorDefCount; i++)
		ReanimationFreeDefinition(&gReanimatorDefArray[i]);

	gReanimatorDefArray.reset();
	gReanimatorDefCount = 0;
	gReanimationParamArray = nullptr;
	gReanimationParamArraySize = 0;
}

float Reanimation::GetTrackVelocity(const char* theTrackName)
{
	ReanimatorFrameTime aFrameTime;
	GetFrameTime(&aFrameTime);
	int aTrackIndex = FindTrackIndex(theTrackName);
	PVZP_ASSERT(aTrackIndex >= 0 && aTrackIndex < mDefinition->mTracks.count);

	ReanimatorTrack* aTrack = &mDefinition->mTracks.tracks[aTrackIndex];
	float aDis = aTrack->mTransforms.mTransforms[aFrameTime.mAnimFrameAfterInt].mTransX - aTrack->mTransforms.mTransforms[aFrameTime.mAnimFrameBeforeInt].mTransX;
	return aDis * SECONDS_PER_UPDATE * mAnimRate;  // velocity = x displacement between frames * frame duration * anim rate
}

bool Reanimation::IsTrackShowing(const char* theTrackName)
{
	ReanimatorFrameTime aFrameTime;
	GetFrameTime(&aFrameTime);
	int aTrackIndex = FindTrackIndex(theTrackName);
	PVZP_ASSERT(aTrackIndex >= 0 && aTrackIndex < mDefinition->mTracks.count);

	return mDefinition->mTracks.tracks[aTrackIndex].mTransforms.mTransforms[aFrameTime.mAnimFrameAfterInt].mFrame >= 0.0f;  // whether the next integer frame has an image
}

void Reanimation::ShowOnlyTrack(const char* theTrackName)
{
	for (int i = 0; i < mDefinition->mTracks.count; i++)
	{
		mTrackInstances[i].mRenderGroup = strcasecmp(mDefinition->mTracks.tracks[i].mName, theTrackName) == 0 ? RENDER_GROUP_NORMAL : RENDER_GROUP_HIDDEN;
	}
}

void Reanimation::AssignRenderGroupToTrack(const char* theTrackName, int theRenderGroup)
{
	for (int i = 0; i < mDefinition->mTracks.count; i++)
		if (strcasecmp(mDefinition->mTracks.tracks[i].mName, theTrackName) == 0)
		{
			mTrackInstances[i].mRenderGroup = theRenderGroup;  // only the first exact match
			return;
		}
}

void Reanimation::AssignRenderGroupToPrefix(const char* theTrackName, int theRenderGroup)
{
	size_t aPrifixLength = strlen(theTrackName);
	for (int i = 0; i < mDefinition->mTracks.count; i++)
	{
		const char* const aTrackName = mDefinition->mTracks.tracks[i].mName;
		if (strlen(aTrackName) >= aPrifixLength && !strncasecmp(aTrackName, theTrackName, aPrifixLength))  // the name must be at least as long as the prefix
			mTrackInstances[i].mRenderGroup = theRenderGroup;
	}
}

void Reanimation::PropogateColorToAttachments()
{
	for (int i = 0; i < mDefinition->mTracks.count; i++)
		AttachmentPropogateColor(
			mTrackInstances[i].mAttachmentID, mColorOverride, mEnableExtraAdditiveDraw, mExtraAdditiveColor, mEnableExtraOverlayDraw, mExtraOverlayColor
		);
}

bool Reanimation::ShouldTriggerTimedEvent(float theEventTime)
{
	PVZP_ASSERT(theEventTime >= 0.0f && theEventTime <= 1.0f);
	if (mFrameCount == 0 || mLastFrameTime <= 0.0f || mAnimRate <= 0.0f)  // no animation, reverse playback, or not playing
		return false;

	if (mAnimTime >= mLastFrameTime)  // normal case: trigger range is [mLastFrameTime, mAnimTime]
		return theEventTime >= mLastFrameTime && theEventTime < mAnimTime;
	else  // wrapped into the next loop: trigger range is [0, mAnimTime] ∪ [mLastFrameTime, 1]
		return theEventTime >= mLastFrameTime || theEventTime < mAnimTime;
}
void Reanimation::PlayReanim(const char* theTrackName, ReanimLoopType theLoopType, int theBlendTime, float theAnimRate)
{
	if (theBlendTime > 0)
		StartBlend(theBlendTime);
	if (theAnimRate != 0.0f)  // a rate of 0 keeps the current anim rate
		mAnimRate = theAnimRate;

	mLoopType = theLoopType;
	mLoopCount = 0;
	SetFramesForLayer(theTrackName);
}

void Reanimation::ParseAttacherTrack(const ReanimatorTransform& theTransform, AttacherInfo& theAttacherInfo)
{
	theAttacherInfo.mReanimName = "";
	theAttacherInfo.mTrackName = "";
	theAttacherInfo.mAnimRate = 12.0f;
	theAttacherInfo.mLoopType = ReanimLoopType::REANIM_LOOP;
	if (theTransform.mFrame == -1.0f)  // blank frame
		return;

	// attacher track name format: attacher__REANIMNAME__TRACKNAME[TAG1][TAG2]...

	const char* aReanimName = strstr(theTransform.mText, "__");  // the "__" before the reanim name
	if (aReanimName == nullptr)
		return;
	const char* aTags = strstr(aReanimName + 2, "[");  // the "[" starting the tags, after the reanim name
	const char* aTrackName = strstr(aReanimName + 2, "__");  // the "__" before the track name, after the reanim name
	if (aTags && aTrackName && ((uintptr_t)aTags < (uintptr_t)aTrackName))  // a "__" after "[" makes the string invalid
		return;

	if (aTrackName)  // track name defined
	{
		theAttacherInfo.mReanimName.assign(aReanimName + 2, aTrackName - aReanimName - 2);  // between the two "__" (REANIMNAME)
		if (aTags)  // tags defined
			theAttacherInfo.mTrackName.assign(aTrackName + 2, aTags - aTrackName - 2);  // up to the "["
		else
			theAttacherInfo.mTrackName.assign(aTrackName + 2);  // to the end of the string
	}
	else if (aTags)  // no track name, but tags defined
		theAttacherInfo.mReanimName.assign(aReanimName + 2, aTags - aReanimName - 2);  // between "__" and "["
	else  // only the reanim name is defined
		theAttacherInfo.mReanimName.assign(aReanimName + 2);  // from after "__" to the end of the string

	while (aTags)  // read each tag
	{
		const char* aTagEnds = strstr(aTags + 1, "]");
		if (aTagEnds == nullptr)  // no closing "]"
			break;

		std::string aCode(aTags + 1, aTagEnds - aTags - 1);  // text inside the brackets
		if (sscanf(aCode.c_str(), "%f", &theAttacherInfo.mAnimRate) != 1)  // if the text parses as a float, it is the anim rate
		{
			if (aCode.compare("hold") == 0)
				theAttacherInfo.mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
			else if (aCode.compare("once") == 0)
				theAttacherInfo.mLoopType = ReanimLoopType::REANIM_PLAY_ONCE;
		}

		aTags = strstr(aTagEnds + 1, "[");  // find the next tag's "["
	}
}

void Reanimation::AttacherSynchWalkSpeed(int theTrackIndex, Reanimation* theAttachReanim, [[maybe_unused]] AttacherInfo& theAttacherInfo)
{
	ReanimatorTrack* aTrack = &mDefinition->mTracks.tracks[theTrackIndex];
	ReanimatorFrameTime aFrameTime;
	GetFrameTime(&aFrameTime);

	int aPlaceHolderFrameStart = aFrameTime.mAnimFrameBeforeInt;
	while (aPlaceHolderFrameStart > mFrameStart && aTrack->mTransforms.mTransforms[aPlaceHolderFrameStart - 1].mText == aTrack->mTransforms.mTransforms[aPlaceHolderFrameStart].mText)
		aPlaceHolderFrameStart--;  // first frame of the current segment
	int aPlaceHolderFrameEnd = aFrameTime.mAnimFrameBeforeInt;
	while (aPlaceHolderFrameEnd < mFrameStart + mFrameCount - 1 && aTrack->mTransforms.mTransforms[aPlaceHolderFrameEnd + 1].mText == aTrack->mTransforms.mTransforms[aPlaceHolderFrameEnd].mText)
		aPlaceHolderFrameEnd++;  // last frame of the current segment
	int aPlaceHolderFrameCount = aPlaceHolderFrameEnd - aPlaceHolderFrameStart;
	ReanimatorTransform& aPlaceHolderStartTrans = aTrack->mTransforms.mTransforms[aPlaceHolderFrameStart];
	ReanimatorTransform& aPlaceHolderEndTrans = aTrack->mTransforms.mTransforms[aPlaceHolderFrameEnd];
	if (FloatApproxEqual(mAnimRate, 0.0f))
	{
		theAttachReanim->mAnimRate = 0.0f;
		return;
	}
	float aPlaceHolderDistance = -(aPlaceHolderEndTrans.mTransX - aPlaceHolderStartTrans.mTransX);  // placeholder track's displacement in this segment
	float aPlaceHolderSeconds = aPlaceHolderFrameCount / mAnimRate;  // placeholder track's duration in this segment
	if (FloatApproxEqual(aPlaceHolderSeconds, 0.0f))  // the segment has no frames
	{
		theAttachReanim->mAnimRate = 0.0f;
		return;
	}

	int aGroundTrackIndex = theAttachReanim->FindTrackIndex("_ground");
	ReanimatorTrack* aGroundTrack = &theAttachReanim->mDefinition->mTracks.tracks[aGroundTrackIndex];
	ReanimatorTransform& aTransformGuyStart = aGroundTrack->mTransforms.mTransforms[theAttachReanim->mFrameStart];
	ReanimatorTransform& aTransformGuyEnd = aGroundTrack->mTransforms.mTransforms[theAttachReanim->mFrameStart + theAttachReanim->mFrameCount - 1];
	float aGuyDistance = aTransformGuyEnd.mTransX - aTransformGuyStart.mTransX;  // attached anim's displacement over one full cycle
	if (aGuyDistance < FLT_EPSILON || aPlaceHolderDistance < FLT_EPSILON)  // zero displacement: the attached anim cannot move
	{
		theAttachReanim->mAnimRate = 0.0f;
		return;
	}

	float aLoops = aPlaceHolderDistance / aGuyDistance;  // cycles needed = placeholder displacement / cycle displacement
	ReanimatorTransform aTransformGuyCurrent;
	theAttachReanim->GetCurrentTransform(aGroundTrackIndex, &aTransformGuyCurrent);
	AttachEffect* aAttachEffect = FindFirstAttachment(mTrackInstances[theTrackIndex].mAttachmentID);
	if (aAttachEffect != nullptr)
	{
		float aGuyCurrentDistance = aTransformGuyCurrent.mTransX - aTransformGuyStart.mTransX;  // attached anim's current displacement within its cycle
		float aGuyExpectedDistance = aGuyDistance * theAttachReanim->mAnimTime;  // expected displacement from the uniformly moving placeholder
		aAttachEffect->mOffset.m02 = aGuyExpectedDistance - aGuyCurrentDistance;  // correct the x offset to keep the attached anim in sync with the placeholder
	}
	theAttachReanim->mAnimRate = aLoops * theAttachReanim->mFrameCount / aPlaceHolderSeconds;  // rate = frames to play / available time
}

void Reanimation::UpdateAttacherTrack(int theTrackIndex)
{
	ReanimatorTrackInstance* aTrackInstance = &mTrackInstances[theTrackIndex];
	ReanimatorTransform aTransform;
	GetCurrentTransform(theTrackIndex, &aTransform);
	AttacherInfo aAttacherInfo;
	ParseAttacherTrack(aTransform, aAttacherInfo);

	ReanimationType aReanimationType = ReanimationType::REANIM_NONE;
	if (aAttacherInfo.mReanimName.size() != 0)
	{
		std::string aReanimFileName = std::format("reanim/{}.reanim", aAttacherInfo.mReanimName);
		for (unsigned int i = 0; i < gReanimationParamArraySize; i++)  // find the reanim type for this file name
		{
			const ReanimationParams* aParams = &gReanimationParamArray[i];
			if (strcasecmp(aReanimFileName.c_str(), aParams->mReanimFileName) == 0)
			{
				aReanimationType = aParams->mReanimationType;
				break;
			}
		}
	}
	if (aReanimationType == ReanimationType::REANIM_NONE)  // no name set or no matching reanim found
	{
		AttachmentDie(aTrackInstance->mAttachmentID);
		return;
	}

	Reanimation* aAttachReanim = FindReanimAttachment(aTrackInstance->mAttachmentID);
	if (aAttachReanim == nullptr || aAttachReanim->mReanimationType != aReanimationType)
	{
		AttachmentDie(aTrackInstance->mAttachmentID);
		aAttachReanim = gEffectSystem->mReanimationHolder->AllocReanimation(0.0f, 0.0f, 0, aReanimationType);
		aAttachReanim->mLoopType = aAttacherInfo.mLoopType;
		aAttachReanim->mAnimRate = aAttacherInfo.mAnimRate;
		AttachReanim(aTrackInstance->mAttachmentID, aAttachReanim, 0.0f, 0.0f);
		mFrameBasePose = NO_BASE_POSE;  // with an attachment set, this reanim has no base pose frame
	}

	if (aAttacherInfo.mTrackName.size() != 0)
	{
		int aAnimFrameStart, aAnimFrameCount;
		aAttachReanim->GetFramesForLayer(aAttacherInfo.mTrackName.c_str(), aAnimFrameStart, aAnimFrameCount);
		if (aAttachReanim->mFrameStart != aAnimFrameStart || aAttachReanim->mFrameCount != aAnimFrameCount)  // if (!aAttachReanim->IsAnimPlaying(……))
		{
			aAttachReanim->StartBlend(20);
			aAttachReanim->SetFramesForLayer(aAttacherInfo.mTrackName.c_str());
		}

		if (aAttachReanim->mAnimRate == 12.0f && aAttacherInfo.mTrackName.compare("anim_walk") == 0 && aAttachReanim->TrackExists("_ground"))
			AttacherSynchWalkSpeed(theTrackIndex, aAttachReanim, aAttacherInfo);
		else
			aAttachReanim->mAnimRate = aAttacherInfo.mAnimRate;
		aAttachReanim->mLoopType = aAttacherInfo.mLoopType;
	}

	Color aColor = ColorsMultiply(mColorOverride, aTrackInstance->mTrackColor);
	aColor.mAlpha = std::clamp(FloatRoundToInt(aTransform.mAlpha * aColor.mAlpha), 0, 255);
	AttachmentPropogateColor(aTrackInstance->mAttachmentID, aColor, mEnableExtraAdditiveDraw, mExtraAdditiveColor, mEnableExtraOverlayDraw, mExtraOverlayColor);
}

bool Reanimation::IsAnimPlaying(const char* theTrackName)
{
	int aFrameStart, aFrameCount;
	GetFramesForLayer(theTrackName, aFrameStart, aFrameCount);
	return mFrameStart == aFrameStart && mFrameCount == aFrameCount;
}

Reanimation* Reanimation::FindSubReanim(ReanimationType theReanimType)
{
	if (mReanimationType == theReanimType)
		return this;

	for (int i = 0; i < mDefinition->mTracks.count; i++)
	{
		Reanimation* aReanimation = FindReanimAttachment(mTrackInstances[i].mAttachmentID);
		if (aReanimation != nullptr)
		{
			Reanimation* aSubReanim = aReanimation->FindSubReanim(theReanimType);
			if (aSubReanim != nullptr)
				return aSubReanim;
		}
	}

	return nullptr;
}
