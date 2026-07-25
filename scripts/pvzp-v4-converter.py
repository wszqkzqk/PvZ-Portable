#!/usr/bin/env python3
# Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
#
# SPDX-License-Identifier: LGPL-3.0-or-later
#
# This file is part of PvZ-Portable.
#
# PvZ-Portable is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# PvZ-Portable is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with PvZ-Portable. If not, see <https://www.gnu.org/licenses/>.

"""PvZ-Portable mid-level save editor: lossless .v4 <-> YAML conversion."""

import argparse
import base64
import math
import struct
import sys
import zlib

try:
    import yaml
except ModuleNotFoundError:
    sys.exit("Error: PyYAML is required. Install it with: python3 -m pip install pyyaml")

# ============================================================================
# Constants
# ============================================================================

SAVE_MAGIC = b"PVZP_SAVE4\x00\x00"  # 12 bytes, NUL-padded
SAVE_VERSION = 1
HEADER_SIZE = 24
YAML_LAYOUT = 2

KEY_MASK = 0xFFFF0000
INDEX_MASK = 0xFFFF
KEY_FIRST = 1001  # DataArrayInitialize: mNextKey starts at 1001

SEEDBANK_MAX = 10
MAX_WAVES = 100
MAX_ZOMBIES_IN_WAVE = 50
NUM_ADVICE_TYPES = 66
MAX_ZOMBIE_FOLLOWERS = 4
MAX_MAGNET_ITEMS = 5
NUM_MOTION_TRAIL_FRAMES = 12

CHUNK_TYPES = {
    1: "BOARD_BASE", 2: "ZOMBIES", 3: "PLANTS", 4: "PROJECTILES", 5: "COINS",
    6: "MOWERS", 7: "GRIDITEMS", 8: "PARTICLE_EMITTERS", 9: "PARTICLE_PARTICLES",
    10: "PARTICLE_SYSTEMS", 11: "REANIMATIONS", 12: "TRAILS", 13: "ATTACHMENTS",
    14: "CURSOR", 15: "CURSOR_PREVIEW", 16: "ADVICE", 17: "SEEDBANK",
    18: "SEEDPACKETS", 19: "CHALLENGE", 20: "MUSIC",
}
CHUNK_IDS = {name: cid for cid, name in CHUNK_TYPES.items()}


class ConvError(Exception):
    """A user-facing conversion error (bad save file or bad YAML input)."""


# ============================================================================
# Enum tables (values verified against src/ConstEnums.h and game headers)
# ============================================================================

SEED_TYPE = {
    -1: "SEED_NONE", 0: "SEED_PEASHOOTER", 1: "SEED_SUNFLOWER", 2: "SEED_CHERRYBOMB",
    3: "SEED_WALLNUT", 4: "SEED_POTATOMINE", 5: "SEED_SNOWPEA", 6: "SEED_CHOMPER",
    7: "SEED_REPEATER", 8: "SEED_PUFFSHROOM", 9: "SEED_SUNSHROOM", 10: "SEED_FUMESHROOM",
    11: "SEED_GRAVEBUSTER", 12: "SEED_HYPNOSHROOM", 13: "SEED_SCAREDYSHROOM", 14: "SEED_ICESHROOM",
    15: "SEED_DOOMSHROOM", 16: "SEED_LILYPAD", 17: "SEED_SQUASH", 18: "SEED_THREEPEATER",
    19: "SEED_TANGLEKELP", 20: "SEED_JALAPENO", 21: "SEED_SPIKEWEED", 22: "SEED_TORCHWOOD",
    23: "SEED_TALLNUT", 24: "SEED_SEASHROOM", 25: "SEED_PLANTERN", 26: "SEED_CACTUS",
    27: "SEED_BLOVER", 28: "SEED_SPLITPEA", 29: "SEED_STARFRUIT", 30: "SEED_PUMPKINSHELL",
    31: "SEED_MAGNETSHROOM", 32: "SEED_CABBAGEPULT", 33: "SEED_FLOWERPOT", 34: "SEED_KERNELPULT",
    35: "SEED_INSTANT_COFFEE", 36: "SEED_GARLIC", 37: "SEED_UMBRELLA", 38: "SEED_MARIGOLD",
    39: "SEED_MELONPULT", 40: "SEED_GATLINGPEA", 41: "SEED_TWINSUNFLOWER", 42: "SEED_GLOOMSHROOM",
    43: "SEED_CATTAIL", 44: "SEED_WINTERMELON", 45: "SEED_GOLD_MAGNET", 46: "SEED_SPIKEROCK",
    47: "SEED_COBCANNON", 48: "SEED_IMITATER", 49: "SEED_EXPLODE_O_NUT", 50: "SEED_GIANT_WALLNUT",
    51: "SEED_SPROUT", 52: "SEED_LEFTPEATER",
    53: "SEED_BEGHOULED_BUTTON_SHUFFLE", 54: "SEED_BEGHOULED_BUTTON_CRATER",
    55: "SEED_SLOT_MACHINE_SUN", 56: "SEED_SLOT_MACHINE_DIAMOND",
    57: "SEED_ZOMBIQUARIUM_SNORKLE", 58: "SEED_ZOMBIQUARIUM_TROPHY",
    59: "SEED_ZOMBIE_NORMAL", 60: "SEED_ZOMBIE_TRAFFIC_CONE", 61: "SEED_ZOMBIE_POLEVAULTER",
    62: "SEED_ZOMBIE_PAIL", 63: "SEED_ZOMBIE_LADDER", 64: "SEED_ZOMBIE_DIGGER",
    65: "SEED_ZOMBIE_BUNGEE", 66: "SEED_ZOMBIE_FOOTBALL", 67: "SEED_ZOMBIE_BALLOON",
    68: "SEED_ZOMBIE_SCREEN_DOOR", 69: "SEED_ZOMBONI", 70: "SEED_ZOMBIE_POGO",
    71: "SEED_ZOMBIE_DANCER", 72: "SEED_ZOMBIE_GARGANTUAR", 73: "SEED_ZOMBIE_IMP",
}

ZOMBIE_TYPE = {
    -1: "ZOMBIE_INVALID", 0: "ZOMBIE_NORMAL", 1: "ZOMBIE_FLAG", 2: "ZOMBIE_TRAFFIC_CONE",
    3: "ZOMBIE_POLEVAULTER", 4: "ZOMBIE_PAIL", 5: "ZOMBIE_NEWSPAPER", 6: "ZOMBIE_DOOR",
    7: "ZOMBIE_FOOTBALL", 8: "ZOMBIE_DANCER", 9: "ZOMBIE_BACKUP_DANCER", 10: "ZOMBIE_DUCKY_TUBE",
    11: "ZOMBIE_SNORKEL", 12: "ZOMBIE_ZAMBONI", 13: "ZOMBIE_BOBSLED", 14: "ZOMBIE_DOLPHIN_RIDER",
    15: "ZOMBIE_JACK_IN_THE_BOX", 16: "ZOMBIE_BALLOON", 17: "ZOMBIE_DIGGER", 18: "ZOMBIE_POGO",
    19: "ZOMBIE_YETI", 20: "ZOMBIE_BUNGEE", 21: "ZOMBIE_LADDER", 22: "ZOMBIE_CATAPULT",
    23: "ZOMBIE_GARGANTUAR", 24: "ZOMBIE_IMP", 25: "ZOMBIE_BOSS", 26: "ZOMBIE_PEA_HEAD",
    27: "ZOMBIE_WALLNUT_HEAD", 28: "ZOMBIE_JALAPENO_HEAD", 29: "ZOMBIE_GATLING_HEAD",
    30: "ZOMBIE_SQUASH_HEAD", 31: "ZOMBIE_TALLNUT_HEAD", 32: "ZOMBIE_REDEYE_GARGANTUAR",
}

BACKGROUND_TYPE = {
    0: "BACKGROUND_1_DAY", 1: "BACKGROUND_2_NIGHT", 2: "BACKGROUND_3_POOL",
    3: "BACKGROUND_4_FOG", 4: "BACKGROUND_5_ROOF", 5: "BACKGROUND_6_BOSS",
    6: "BACKGROUND_MUSHROOM_GARDEN", 7: "BACKGROUND_GREENHOUSE",
    8: "BACKGROUND_ZOMBIQUARIUM", 9: "BACKGROUND_TREEOFWISDOM",
}

GRID_SQUARE_TYPE = {0: "GRIDSQUARE_NONE", 1: "GRIDSQUARE_GRASS", 2: "GRIDSQUARE_DIRT",
                    3: "GRIDSQUARE_POOL", 4: "GRIDSQUARE_HIGH_GROUND"}

PLANT_ROW_TYPE = {0: "PLANTROW_DIRT", 1: "PLANTROW_NORMAL", 2: "PLANTROW_POOL",
                  3: "PLANTROW_HIGH_GROUND"}

TUTORIAL_STATE = {
    0: "TUTORIAL_OFF", 1: "TUTORIAL_LEVEL_1_PICK_UP_PEASHOOTER", 2: "TUTORIAL_LEVEL_1_PLANT_PEASHOOTER",
    3: "TUTORIAL_LEVEL_1_REFRESH_PEASHOOTER", 4: "TUTORIAL_LEVEL_1_COMPLETED",
    5: "TUTORIAL_LEVEL_2_PICK_UP_SUNFLOWER", 6: "TUTORIAL_LEVEL_2_PLANT_SUNFLOWER",
    7: "TUTORIAL_LEVEL_2_REFRESH_SUNFLOWER", 8: "TUTORIAL_LEVEL_2_COMPLETED",
    9: "TUTORIAL_MORESUN_PICK_UP_SUNFLOWER", 10: "TUTORIAL_MORESUN_PLANT_SUNFLOWER",
    11: "TUTORIAL_MORESUN_REFRESH_SUNFLOWER", 12: "TUTORIAL_MORESUN_COMPLETED",
    13: "TUTORIAL_SLOT_MACHINE_PULL", 14: "TUTORIAL_SLOT_MACHINE_COMPLETED",
    15: "TUTORIAL_SHOVEL_PICKUP", 16: "TUTORIAL_SHOVEL_DIG", 17: "TUTORIAL_SHOVEL_KEEP_DIGGING",
    18: "TUTORIAL_SHOVEL_COMPLETED", 19: "TUTORIAL_ZOMBIQUARIUM_BUY_SNORKEL",
    20: "TUTORIAL_ZOMBIQUARIUM_BOUGHT_SNORKEL", 21: "TUTORIAL_ZOMBIQUARIUM_CLICK_TROPHY",
    22: "TUTORIAL_ZEN_GARDEN_PICKUP_WATER", 23: "TUTORIAL_ZEN_GARDEN_WATER_PLANT",
    24: "TUTORIAL_ZEN_GARDEN_KEEP_WATERING", 25: "TUTORIAL_ZEN_GARDEN_VISIT_STORE",
    26: "TUTORIAL_ZEN_GARDEN_FERTILIZE_PLANTS", 27: "TUTORIAL_ZEN_GARDEN_COMPLETED",
    28: "TUTORIAL_WHACK_A_ZOMBIE_BEFORE_PICK_SEED", 29: "TUTORIAL_WHACK_A_ZOMBIE_PICK_SEED",
    30: "TUTORIAL_WHACK_A_ZOMBIE_COMPLETED",
}

DEBUG_TEXT_MODE = {0: "DEBUG_TEXT_NONE", 1: "DEBUG_TEXT_ZOMBIE_SPAWN", 2: "DEBUG_TEXT_MUSIC",
                   3: "DEBUG_TEXT_MEMORY", 4: "DEBUG_TEXT_COLLISION"}

BOARD_RESULT = {0: "BOARDRESULT_NONE", 1: "BOARDRESULT_WON", 2: "BOARDRESULT_LOST",
                3: "BOARDRESULT_RESTART", 4: "BOARDRESULT_QUIT", 5: "BOARDRESULT_QUIT_APP",
                6: "BOARDRESULT_CHEAT"}

HELM_TYPE = {
    0: "HELMTYPE_NONE", 1: "HELMTYPE_TRAFFIC_CONE", 2: "HELMTYPE_PAIL", 3: "HELMTYPE_FOOTBALL",
    4: "HELMTYPE_DIGGER", 5: "HELMTYPE_REDEYES", 6: "HELMTYPE_HEADBAND", 7: "HELMTYPE_BOBSLED",
    8: "HELMTYPE_WALLNUT", 9: "HELMTYPE_TALLNUT",
}

SHIELD_TYPE = {0: "SHIELDTYPE_NONE", 1: "SHIELDTYPE_DOOR", 2: "SHIELDTYPE_NEWSPAPER",
               3: "SHIELDTYPE_LADDER"}

ZOMBIE_PHASE = {
    0: "PHASE_ZOMBIE_NORMAL", 1: "PHASE_ZOMBIE_DYING", 2: "PHASE_ZOMBIE_BURNED",
    3: "PHASE_ZOMBIE_MOWERED", 4: "PHASE_BUNGEE_DIVING", 5: "PHASE_BUNGEE_DIVING_SCREAMING",
    6: "PHASE_BUNGEE_AT_BOTTOM", 7: "PHASE_BUNGEE_GRABBING", 8: "PHASE_BUNGEE_RISING",
    9: "PHASE_BUNGEE_HIT_OUCHY", 10: "PHASE_BUNGEE_CUTSCENE", 11: "PHASE_POLEVAULTER_PRE_VAULT",
    12: "PHASE_POLEVAULTER_IN_VAULT", 13: "PHASE_POLEVAULTER_POST_VAULT", 14: "PHASE_RISING_FROM_GRAVE",
    15: "PHASE_JACK_IN_THE_BOX_RUNNING", 16: "PHASE_JACK_IN_THE_BOX_POPPING",
    17: "PHASE_BOBSLED_SLIDING", 18: "PHASE_BOBSLED_BOARDING", 19: "PHASE_BOBSLED_CRASHING",
    20: "PHASE_POGO_BOUNCING", 21: "PHASE_POGO_HIGH_BOUNCE_1", 22: "PHASE_POGO_HIGH_BOUNCE_2",
    23: "PHASE_POGO_HIGH_BOUNCE_3", 24: "PHASE_POGO_HIGH_BOUNCE_4", 25: "PHASE_POGO_HIGH_BOUNCE_5",
    26: "PHASE_POGO_HIGH_BOUNCE_6", 27: "PHASE_POGO_FORWARD_BOUNCE_2", 28: "PHASE_POGO_FORWARD_BOUNCE_7",
    29: "PHASE_NEWSPAPER_READING", 30: "PHASE_NEWSPAPER_MADDENING", 31: "PHASE_NEWSPAPER_MAD",
    32: "PHASE_DIGGER_TUNNELING", 33: "PHASE_DIGGER_RISING",
    34: "PHASE_DIGGER_TUNNELING_PAUSE_WITHOUT_AXE", 35: "PHASE_DIGGER_RISE_WITHOUT_AXE",
    36: "PHASE_DIGGER_STUNNED", 37: "PHASE_DIGGER_WALKING", 38: "PHASE_DIGGER_WALKING_WITHOUT_AXE",
    39: "PHASE_DIGGER_CUTSCENE", 40: "PHASE_DANCER_DANCING_IN", 41: "PHASE_DANCER_SNAPPING_FINGERS",
    42: "PHASE_DANCER_SNAPPING_FINGERS_WITH_LIGHT", 43: "PHASE_DANCER_SNAPPING_FINGERS_HOLD",
    44: "PHASE_DANCER_DANCING_LEFT", 45: "PHASE_DANCER_WALK_TO_RAISE", 46: "PHASE_DANCER_RAISE_LEFT_1",
    47: "PHASE_DANCER_RAISE_RIGHT_1", 48: "PHASE_DANCER_RAISE_LEFT_2", 49: "PHASE_DANCER_RAISE_RIGHT_2",
    50: "PHASE_DANCER_RISING", 51: "PHASE_DOLPHIN_WALKING", 52: "PHASE_DOLPHIN_INTO_POOL",
    53: "PHASE_DOLPHIN_RIDING", 54: "PHASE_DOLPHIN_IN_JUMP", 55: "PHASE_DOLPHIN_WALKING_IN_POOL",
    56: "PHASE_DOLPHIN_WALKING_WITHOUT_DOLPHIN", 57: "PHASE_SNORKEL_WALKING",
    58: "PHASE_SNORKEL_INTO_POOL", 59: "PHASE_SNORKEL_WALKING_IN_POOL", 60: "PHASE_SNORKEL_UP_TO_EAT",
    61: "PHASE_SNORKEL_EATING_IN_POOL", 62: "PHASE_SNORKEL_DOWN_FROM_EAT",
    63: "PHASE_ZOMBIQUARIUM_ACCEL", 64: "PHASE_ZOMBIQUARIUM_DRIFT",
    65: "PHASE_ZOMBIQUARIUM_BACK_AND_FORTH", 66: "PHASE_ZOMBIQUARIUM_BITE",
    67: "PHASE_CATAPULT_LAUNCHING", 68: "PHASE_CATAPULT_RELOADING", 69: "PHASE_GARGANTUAR_THROWING",
    70: "PHASE_GARGANTUAR_SMASHING", 71: "PHASE_IMP_GETTING_THROWN", 72: "PHASE_IMP_LANDING",
    73: "PHASE_BALLOON_FLYING", 74: "PHASE_BALLOON_POPPING", 75: "PHASE_BALLOON_WALKING",
    76: "PHASE_LADDER_CARRYING", 77: "PHASE_LADDER_PLACING", 78: "PHASE_BOSS_ENTER",
    79: "PHASE_BOSS_IDLE", 80: "PHASE_BOSS_SPAWNING", 81: "PHASE_BOSS_STOMPING",
    82: "PHASE_BOSS_BUNGEES_ENTER", 83: "PHASE_BOSS_BUNGEES_DROP", 84: "PHASE_BOSS_BUNGEES_LEAVE",
    85: "PHASE_BOSS_DROP_RV", 86: "PHASE_BOSS_HEAD_ENTER", 87: "PHASE_BOSS_HEAD_IDLE_BEFORE_SPIT",
    88: "PHASE_BOSS_HEAD_IDLE_AFTER_SPIT", 89: "PHASE_BOSS_HEAD_SPIT", 90: "PHASE_BOSS_HEAD_LEAVE",
    91: "PHASE_YETI_RUNNING", 92: "PHASE_SQUASH_PRE_LAUNCH", 93: "PHASE_SQUASH_RISING",
    94: "PHASE_SQUASH_FALLING", 95: "PHASE_SQUASH_DONE_FALLING",
}

ZOMBIE_HEIGHT = {
    0: "HEIGHT_ZOMBIE_NORMAL", 1: "HEIGHT_IN_TO_POOL", 2: "HEIGHT_OUT_OF_POOL",
    3: "HEIGHT_DRAGGED_UNDER", 4: "HEIGHT_UP_TO_HIGH_GROUND", 5: "HEIGHT_DOWN_OFF_HIGH_GROUND",
    6: "HEIGHT_UP_LADDER", 7: "HEIGHT_FALLING", 8: "HEIGHT_IN_TO_CHIMNEY",
    9: "HEIGHT_GETTING_BUNGEE_DROPPED", 10: "HEIGHT_ZOMBIQUARIUM",
}

PLANT_STATE = {
    0: "STATE_NOTREADY", 1: "STATE_READY", 2: "STATE_DOINGSPECIAL", 3: "STATE_SQUASH_LOOK",
    4: "STATE_SQUASH_PRE_LAUNCH", 5: "STATE_SQUASH_RISING", 6: "STATE_SQUASH_FALLING",
    7: "STATE_SQUASH_DONE_FALLING", 8: "STATE_GRAVEBUSTER_LANDING", 9: "STATE_GRAVEBUSTER_EATING",
    10: "STATE_CHOMPER_BITING", 11: "STATE_CHOMPER_BITING_GOT_ONE", 12: "STATE_CHOMPER_BITING_MISSED",
    13: "STATE_CHOMPER_DIGESTING", 14: "STATE_CHOMPER_SWALLOWING", 15: "STATE_POTATO_RISING",
    16: "STATE_POTATO_ARMED", 17: "STATE_POTATO_MASHED", 18: "STATE_SPIKEWEED_ATTACKING",
    19: "STATE_SPIKEWEED_ATTACKING_2", 20: "STATE_SCAREDYSHROOM_LOWERING",
    21: "STATE_SCAREDYSHROOM_SCARED", 22: "STATE_SCAREDYSHROOM_RAISING", 23: "STATE_SUNSHROOM_SMALL",
    24: "STATE_SUNSHROOM_GROWING", 25: "STATE_SUNSHROOM_BIG", 26: "STATE_MAGNETSHROOM_SUCKING",
    27: "STATE_MAGNETSHROOM_CHARGING", 28: "STATE_BOWLING_UP", 29: "STATE_BOWLING_DOWN",
    30: "STATE_CACTUS_LOW", 31: "STATE_CACTUS_RISING", 32: "STATE_CACTUS_HIGH",
    33: "STATE_CACTUS_LOWERING", 34: "STATE_TANGLEKELP_GRABBING", 35: "STATE_COBCANNON_ARMING",
    36: "STATE_COBCANNON_LOADING", 37: "STATE_COBCANNON_READY", 38: "STATE_COBCANNON_FIRING",
    39: "STATE_KERNELPULT_BUTTER", 40: "STATE_UMBRELLA_TRIGGERED", 41: "STATE_UMBRELLA_REFLECTING",
    42: "STATE_IMITATER_MORPHING", 43: "STATE_ZEN_GARDEN_WATERED", 44: "STATE_ZEN_GARDEN_NEEDY",
    45: "STATE_ZEN_GARDEN_HAPPY", 46: "STATE_MARIGOLD_ENDING", 47: "STATE_FLOWERPOT_INVULNERABLE",
    48: "STATE_LILYPAD_INVULNERABLE",
}

PLANT_SUBCLASS = {0: "SUBCLASS_NORMAL", 1: "SUBCLASS_SHOOTER"}

PLANT_ON_BUNGEE_STATE = {0: "NOT_ON_BUNGEE", 1: "GETTING_GRABBED_BY_BUNGEE",
                         2: "RISING_WITH_BUNGEE"}

MAGNET_ITEM_TYPE = {
    0: "MAGNET_ITEM_NONE", 1: "MAGNET_ITEM_PAIL_1", 2: "MAGNET_ITEM_PAIL_2", 3: "MAGNET_ITEM_PAIL_3",
    4: "MAGNET_ITEM_FOOTBALL_HELMET_1", 5: "MAGNET_ITEM_FOOTBALL_HELMET_2",
    6: "MAGNET_ITEM_FOOTBALL_HELMET_3", 7: "MAGNET_ITEM_DOOR_1", 8: "MAGNET_ITEM_DOOR_2",
    9: "MAGNET_ITEM_DOOR_3", 10: "MAGNET_ITEM_POGO_1", 11: "MAGNET_ITEM_POGO_2",
    12: "MAGNET_ITEM_POGO_3", 13: "MAGNET_ITEM_JACK_IN_THE_BOX", 14: "MAGNET_ITEM_LADDER_1",
    15: "MAGNET_ITEM_LADDER_2", 16: "MAGNET_ITEM_LADDER_3", 17: "MAGNET_ITEM_LADDER_PLACED",
    18: "MAGNET_ITEM_SILVER_COIN", 19: "MAGNET_ITEM_GOLD_COIN", 20: "MAGNET_ITEM_DIAMOND",
    21: "MAGNET_ITEM_PICK_AXE",
}

COIN_TYPE = {
    0: "COIN_NONE", 1: "COIN_SILVER", 2: "COIN_GOLD", 3: "COIN_DIAMOND", 4: "COIN_SUN",
    5: "COIN_SMALLSUN", 6: "COIN_LARGESUN", 7: "COIN_FINAL_SEED_PACKET", 8: "COIN_TROPHY",
    9: "COIN_SHOVEL", 10: "COIN_ALMANAC", 11: "COIN_CARKEYS", 12: "COIN_VASE",
    13: "COIN_WATERING_CAN", 14: "COIN_TACO", 15: "COIN_NOTE", 16: "COIN_USABLE_SEED_PACKET",
    17: "COIN_PRESENT_PLANT", 18: "COIN_AWARD_MONEY_BAG", 19: "COIN_AWARD_PRESENT",
    20: "COIN_AWARD_BAG_DIAMOND", 21: "COIN_AWARD_SILVER_SUNFLOWER", 22: "COIN_AWARD_GOLD_SUNFLOWER",
    23: "COIN_CHOCOLATE", 24: "COIN_AWARD_CHOCOLATE", 25: "COIN_PRESENT_MINIGAMES",
    26: "COIN_PRESENT_PUZZLE_MODE", 27: "COIN_PRESENT_SURVIVAL_MODE",
}

COIN_MOTION = {
    0: "COIN_MOTION_FROM_SKY", 1: "COIN_MOTION_FROM_SKY_SLOW", 2: "COIN_MOTION_FROM_PLANT",
    3: "COIN_MOTION_COIN", 4: "COIN_MOTION_LAWNMOWER_COIN", 5: "COIN_MOTION_FROM_PRESENT",
    6: "COIN_MOTION_FROM_BOSS",
}

GRID_ITEM_TYPE = {
    0: "GRIDITEM_NONE", 1: "GRIDITEM_GRAVESTONE", 2: "GRIDITEM_CRATER", 3: "GRIDITEM_LADDER",
    4: "GRIDITEM_PORTAL_CIRCLE", 5: "GRIDITEM_PORTAL_SQUARE", 6: "GRIDITEM_BRAIN",
    7: "GRIDITEM_SCARY_POT", 9: "GRIDITEM_ZEN_TOOL", 10: "GRIDITEM_STINKY", 11: "GRIDITEM_RAKE",
    12: "GRIDITEM_IZOMBIE_BRAIN",
}

GRID_ITEM_STATE = {
    0: "GRIDITEM_STATE_NORMAL", 1: "GRIDITEM_STATE_GRAVESTONE_SPECIAL",
    2: "GRIDITEM_STATE_PORTAL_CLOSED", 3: "GRIDITEM_STATE_SCARY_POT_QUESTION",
    4: "GRIDITEM_STATE_SCARY_POT_LEAF", 5: "GRIDITEM_STATE_SCARY_POT_ZOMBIE",
    6: "GRIDITEM_STATE_SQUIRREL_WAITING", 7: "GRIDITEM_STATE_SQUIRREL_PEEKING",
    8: "GRIDITEM_STATE_SQUIRREL_RUNNING_UP", 9: "GRIDITEM_STATE_SQUIRREL_RUNNING_DOWN",
    10: "GRIDITEM_STATE_SQUIRREL_RUNNING_LEFT", 11: "GRIDITEM_STATE_SQUIRREL_RUNNING_RIGHT",
    12: "GRIDITEM_STATE_SQUIRREL_CAUGHT", 13: "GRIDITEM_STATE_SQUIRREL_ZOMBIE",
    14: "GRIDITEM_STATE_ZEN_TOOL_WATERING_CAN", 15: "GRIDITEM_STATE_ZEN_TOOL_FERTILIZER",
    16: "GRIDITEM_STATE_ZEN_TOOL_BUG_SPRAY", 17: "GRIDITEM_STATE_ZEN_TOOL_PHONOGRAPH",
    18: "GRIDITEM_STATE_ZEN_TOOL_GOLD_WATERING_CAN", 19: "GRIDITEM_STINKY_WALKING_LEFT",
    20: "GRIDITEM_STINKY_TURNING_LEFT", 21: "GRIDITEM_STINKY_WALKING_RIGHT",
    22: "GRIDITEM_STINKY_TURNING_RIGHT", 23: "GRIDITEM_STINKY_SLEEPING",
    24: "GRIDITEM_STINKY_FALLING_ASLEEP", 25: "GRIDITEM_STINKY_WAKING_UP",
    26: "GRIDITEM_STATE_RAKE_ATTRACTING", 27: "GRIDITEM_STATE_RAKE_WAITING",
    28: "GRIDITEM_STATE_RAKE_TRIGGERED", 29: "GRIDITEM_STATE_BRAIN_SQUISHED",
}

SCARY_POT_TYPE = {0: "SCARYPOT_NONE", 1: "SCARYPOT_SEED", 2: "SCARYPOT_ZOMBIE",
                  3: "SCARYPOT_SUN"}

LAWN_MOWER_STATE = {0: "MOWER_ROLLING_IN", 1: "MOWER_READY", 2: "MOWER_TRIGGERED",
                    3: "MOWER_SQUISHED"}
LAWN_MOWER_TYPE = {0: "LAWNMOWER_LAWN", 1: "LAWNMOWER_POOL", 2: "LAWNMOWER_ROOF",
                   3: "LAWNMOWER_SUPER_MOWER"}
MOWER_HEIGHT = {0: "MOWER_HEIGHT_LAND", 1: "MOWER_HEIGHT_DOWN_TO_POOL",
                2: "MOWER_HEIGHT_IN_POOL", 3: "MOWER_HEIGHT_UP_TO_LAND"}

CURSOR_TYPE = {
    0: "CURSOR_TYPE_NORMAL", 1: "CURSOR_TYPE_PLANT_FROM_BANK",
    2: "CURSOR_TYPE_PLANT_FROM_USABLE_COIN", 3: "CURSOR_TYPE_PLANT_FROM_GLOVE",
    4: "CURSOR_TYPE_PLANT_FROM_DUPLICATOR", 5: "CURSOR_TYPE_PLANT_FROM_WHEEL_BARROW",
    6: "CURSOR_TYPE_SHOVEL", 7: "CURSOR_TYPE_HAMMER", 8: "CURSOR_TYPE_COBCANNON_TARGET",
    9: "CURSOR_TYPE_WATERING_CAN", 10: "CURSOR_TYPE_FERTILIZER", 11: "CURSOR_TYPE_BUG_SPRAY",
    12: "CURSOR_TYPE_PHONOGRAPH", 13: "CURSOR_TYPE_CHOCOLATE", 14: "CURSOR_TYPE_GLOVE",
    15: "CURSOR_TYPE_MONEY_SIGN", 16: "CURSOR_TYPE_WHEEELBARROW", 17: "CURSOR_TYPE_TREE_FOOD",
}

CHALLENGE_STATE = {
    0: "STATECHALLENGE_NORMAL", 1: "STATECHALLENGE_BEGHOULED_MOVING",
    2: "STATECHALLENGE_BEGHOULED_FALLING", 3: "STATECHALLENGE_BEGHOULED_NO_MATCHES",
    4: "STATECHALLENGE_SLOT_MACHINE_ROLLING", 5: "STATECHALLENGE_STORM_FLASH_1",
    6: "STATECHALLENGE_STORM_FLASH_2", 7: "STATECHALLENGE_STORM_FLASH_3",
    8: "STATECHALLENGE_ZEN_FADING", 9: "STATECHALLENGE_SCARY_POTTER_MALLETING",
    10: "STATECHALLENGE_LAST_STAND_ONSLAUGHT", 11: "STATECHALLENGE_TREE_JUST_GREW",
    12: "STATECHALLENGE_TREE_GIVE_WISDOM", 13: "STATECHALLENGE_TREE_WAITING_TO_BABBLE",
    14: "STATECHALLENGE_TREE_BABBLING",
}

PROJECTILE_MOTION = {
    0: "MOTION_STRAIGHT", 1: "MOTION_LOBBED", 2: "MOTION_THREEPEATER", 3: "MOTION_BEE",
    4: "MOTION_BEE_BACKWARDS", 5: "MOTION_PUFF", 6: "MOTION_BACKWARDS", 7: "MOTION_STAR",
    8: "MOTION_FLOAT_OVER", 9: "MOTION_HOMING",
}

PROJECTILE_TYPE = {
    0: "PROJECTILE_PEA", 1: "PROJECTILE_SNOWPEA", 2: "PROJECTILE_CABBAGE", 3: "PROJECTILE_MELON",
    4: "PROJECTILE_PUFF", 5: "PROJECTILE_WINTERMELON", 6: "PROJECTILE_FIREBALL",
    7: "PROJECTILE_STAR", 8: "PROJECTILE_SPIKE", 9: "PROJECTILE_BASKETBALL",
    10: "PROJECTILE_KERNEL", 11: "PROJECTILE_COBBIG", 12: "PROJECTILE_BUTTER",
    13: "PROJECTILE_ZOMBIE_PEA",
}

MUSIC_TUNE = {
    -1: "MUSIC_TUNE_NONE", 1: "MUSIC_TUNE_DAY_GRASSWALK", 2: "MUSIC_TUNE_NIGHT_MOONGRAINS",
    3: "MUSIC_TUNE_POOL_WATERYGRAVES", 4: "MUSIC_TUNE_FOG_RIGORMORMIST",
    5: "MUSIC_TUNE_ROOF_GRAZETHEROOF", 6: "MUSIC_TUNE_CHOOSE_YOUR_SEEDS",
    7: "MUSIC_TUNE_TITLE_CRAZY_DAVE_MAIN_THEME", 8: "MUSIC_TUNE_ZEN_GARDEN",
    9: "MUSIC_TUNE_PUZZLE_CEREBRAWL", 10: "MUSIC_TUNE_MINIGAME_LOONBOON",
    11: "MUSIC_TUNE_CONVEYER", 12: "MUSIC_TUNE_FINAL_BOSS_BRAINIAC_MANIAC",
    13: "MUSIC_TUNE_CREDITS_ZOMBIES_ON_YOUR_LAWN",
}
MUSIC_FILE = {-1: "MUSIC_FILE_NONE", 1: "MUSIC_FILE_MAIN_MUSIC", 2: "MUSIC_FILE_DRUMS",
              3: "MUSIC_FILE_HIHATS", 4: "MUSIC_FILE_CREDITS_ZOMBIES_ON_YOUR_LAWN"}
MUSIC_BURST_STATE = {0: "MUSIC_BURST_OFF", 1: "MUSIC_BURST_STARTING", 2: "MUSIC_BURST_ON",
                     3: "MUSIC_BURST_FINISHING"}
MUSIC_DRUMS_STATE = {0: "MUSIC_DRUMS_OFF", 1: "MUSIC_DRUMS_ON_QUEUED", 2: "MUSIC_DRUMS_ON",
                     3: "MUSIC_DRUMS_OFF_QUEUED", 4: "MUSIC_DRUMS_FADING"}

GARDEN_TYPE = {0: "GARDEN_MAIN", 1: "GARDEN_MUSHROOM", 2: "GARDEN_WHEELBARROW",
               3: "GARDEN_AQUARIUM"}
POTTED_PLANT_AGE = {0: "PLANTAGE_SPROUT", 1: "PLANTAGE_SMALL", 2: "PLANTAGE_MEDIUM",
                    3: "PLANTAGE_FULL"}
POTTED_PLANT_NEED = {0: "PLANTNEED_NONE", 1: "PLANTNEED_WATER", 2: "PLANTNEED_FERTILIZER",
                     3: "PLANTNEED_BUGSPRAY", 4: "PLANTNEED_PHONOGRAPH"}
FACING_DIRECTION = {0: "FACING_RIGHT", 1: "FACING_LEFT"}
DRAW_VARIATION = {
    0: "VARIATION_NORMAL", 1: "VARIATION_IMITATER", 2: "VARIATION_MARIGOLD_WHITE",
    3: "VARIATION_MARIGOLD_MAGENTA", 4: "VARIATION_MARIGOLD_ORANGE", 5: "VARIATION_MARIGOLD_PINK",
    6: "VARIATION_MARIGOLD_LIGHT_BLUE", 7: "VARIATION_MARIGOLD_RED", 8: "VARIATION_MARIGOLD_BLUE",
    9: "VARIATION_MARIGOLD_VIOLET", 10: "VARIATION_MARIGOLD_LAVENDER",
    11: "VARIATION_MARIGOLD_YELLOW", 12: "VARIATION_MARIGOLD_LIGHT_GREEN",
    13: "VARIATION_ZEN_GARDEN", 14: "VARIATION_ZEN_GARDEN_WATER", 15: "VARIATION_SPROUT_NO_FLOWER",
    16: "VARIATION_LESS", 17: "VARIATION_AQUARIUM",
}


# ============================================================================
# Binary layer (little-endian, matching src/Lawn/System/SaveGame.cpp)
# ============================================================================

class BinaryReader:
    def __init__(self, data: bytes):
        self.data = data
        self.pos = 0

    @property
    def remaining(self) -> int:
        return len(self.data) - self.pos

    def read_bytes(self, n: int) -> bytes:
        if self.pos + n > len(self.data):
            raise ConvError(f"unexpected end of data (need {n} bytes, have {self.remaining})")
        result = self.data[self.pos:self.pos + n]
        self.pos += n
        return result

    def read_u8(self) -> int:
        return self.read_bytes(1)[0]

    def read_u32(self) -> int:
        return struct.unpack("<I", self.read_bytes(4))[0]

    def read_i32(self) -> int:
        return struct.unpack("<i", self.read_bytes(4))[0]

    def read_i64(self) -> int:
        return struct.unpack("<q", self.read_bytes(8))[0]

    def read_f32(self) -> float:
        return struct.unpack("<f", self.read_bytes(4))[0]

    def read_bool(self) -> bool:
        return self.read_u8() != 0


class BinaryWriter:
    def __init__(self):
        self.data = bytearray()

    def write_bytes(self, data: bytes):
        self.data.extend(data)

    def write_u8(self, value: int):
        self.data.append(value & 0xFF)

    def write_u32(self, value: int):
        self.data.extend(struct.pack("<I", value & 0xFFFFFFFF))

    def write_i32(self, value: int):
        self.data.extend(struct.pack("<i", int(value)))

    def write_i64(self, value: int):
        self.data.extend(struct.pack("<q", int(value)))

    def write_f32(self, value: float):
        self.data.extend(struct.pack("<f", float(value)))

    def write_bool(self, value: bool):
        self.write_u8(1 if value else 0)

    def get_bytes(self) -> bytes:
        return bytes(self.data)


def parse_tlv_fields(data: bytes) -> dict[int, bytes]:
    """Parse a TLV blob; later duplicates of a field id win."""
    reader = BinaryReader(data)
    fields = {}
    while reader.remaining >= 8:
        field_id = reader.read_u32()
        field_size = reader.read_u32()
        if reader.remaining < field_size:
            break
        fields[field_id] = reader.read_bytes(field_size)
    return fields


def read_tlv_blob(reader: BinaryReader) -> bytes:
    """Read a length-prefixed TLV blob (WriteTLVBlob in SaveGame.cpp)."""
    return reader.read_bytes(reader.read_u32())


def write_tlv_blob(data: bytes) -> bytes:
    writer = BinaryWriter()
    writer.write_u32(len(data))
    writer.write_bytes(data)
    return writer.get_bytes()


def write_tlv_fields(fields: dict[int, bytes]) -> bytes:
    writer = BinaryWriter()
    for field_id in sorted(fields):
        data = fields[field_id]
        writer.write_u32(field_id)
        writer.write_u32(len(data))
        writer.write_bytes(data)
    return writer.get_bytes()


# ============================================================================
# YAML presentation helpers
# ============================================================================

class Flow(list):
    """A list that is dumped in compact flow style: [1, 2, 3]."""


class FlowDict(dict):
    """A dict that is dumped in compact flow style: {x: 1, y: 2}."""


class F32(float):
    """A float32 value dumped at the shortest precision that round-trips."""


def _f32_roundtrips(text: str, value: float) -> bool:
    try:
        return struct.unpack("<f", struct.pack("<f", float(text)))[0] == value
    except (OverflowError, ValueError):
        return False


def _f32_str(value: float) -> str:
    if math.isnan(value):
        return ".nan"
    if math.isinf(value):
        return ".inf" if value > 0 else "-.inf"
    for precision in range(1, 10):
        text = format(value, f".{precision}g")
        if _f32_roundtrips(text, value):
            break
    else:
        text = repr(value)
    if "e" in text or "E" in text:
        if abs(value) >= 1.0:
            # Large values: plain decimal is exact and reads better.
            expanded = format(value, ".20f").rstrip("0")
            if expanded[-1] == ".":
                expanded += "0"
            if _f32_roundtrips(expanded, value):
                return expanded
        # Small values (or failed expansion): keep scientific notation, but
        # make sure the mantissa has a dot so PyYAML treats it as a plain
        # float (e.g. "5e-05" -> "5.0e-05"); the %g text already round-trips.
        mantissa, _, exponent = text.replace("E", "e").partition("e")
        if "." not in mantissa:
            mantissa += ".0"
        return f"{mantissa}e{exponent}"
    if "." not in text:  # keep it a YAML float, not an int
        text += ".0"
    return text


def _flow_representer(dumper, data):
    return dumper.represent_sequence("tag:yaml.org,2002:seq", data, flow_style=True)


def _flowdict_representer(dumper, data):
    return dumper.represent_mapping("tag:yaml.org,2002:map", data, flow_style=True)


def _f32_representer(dumper, data):
    return dumper.represent_scalar("tag:yaml.org,2002:float", _f32_str(float(data)))


yaml.add_representer(Flow, _flow_representer, Dumper=yaml.SafeDumper)
yaml.add_representer(FlowDict, _flowdict_representer, Dumper=yaml.SafeDumper)
yaml.add_representer(F32, _f32_representer, Dumper=yaml.SafeDumper)


def _b64(data: bytes) -> str:
    return base64.b64encode(data).decode("ascii")


def _unb64(text: str, path: str) -> bytes:
    try:
        return base64.b64decode(text, validate=True)
    except Exception:
        raise ConvError(f"{path}: invalid base64 data")


# ============================================================================
# Field schema engine
# ============================================================================
#
# A schema is a list of field specs: (yaml_key, kind, arg, default, common)
#   kind "bool" | "i32" | "u32" | "i64" | "f32"     scalar, arg=None
#   kind "enum"                                      arg=enum dict
#   kind "i32s" | "u32s" | "bools" | "f32s"          arg=array length
#   kind "enums"                                     arg=(length, enum dict)
#   kind "rect"                                      TRect (4 x i32)
#   kind "bytes"                                     arg=length or None -> base64
#   kind "smooth"                                    arg=rows; TodSmoothArray rows x 4 x (i32 + 3 f32)
#   kind "magnet"                                    MAX_MAGNET_ITEMS MagnetItem
#   kind "potted"                                    PottedPlant
#   kind "trail"                                     NUM_MOTION_TRAIL_FRAMES x (3 x f32)
# `common` fields are exported at the top level of their section; the rest go
# into an "advanced" sub-key.

def enum_to_yaml(table: dict, value: int):
    return table.get(value, value)  # unknown values pass through as ints (lossless)


def enum_from_yaml(table: dict, value, path: str) -> int:
    if isinstance(value, str):
        for number, name in table.items():
            if name == value:
                return number
        raise ConvError(f"{path}: unknown enum name {value!r}")
    if isinstance(value, int) and not isinstance(value, bool):
        return value
    raise ConvError(f"{path}: expected an enum name, got {value!r}")


def _default_for(kind: str, arg):
    if kind == "bool":
        return False
    if kind in ("i32", "u32", "i64"):
        return 0
    if kind == "f32":
        return F32(0.0)
    if kind == "enum":
        return 0
    if kind == "rect":
        return FlowDict({"x": 0, "y": 0, "width": 0, "height": 0})
    if kind in ("i32s", "u32s"):
        return Flow([0] * arg)
    if kind == "bools":
        return Flow([False] * arg)
    if kind == "f32s":
        return Flow([F32(0.0)] * arg)
    if kind == "enums":
        return Flow([0] * arg[0])
    if kind == "bytes":
        return _b64(b"\x00" * (arg or 0))
    if kind == "smooth":
        return [[0, F32(0.0), F32(0.0), F32(0.0)] for _ in range(arg)]
    if kind == "magnet":
        return [_default_for("magnet_item", None) for _ in range(MAX_MAGNET_ITEMS)]
    if kind == "magnet_item":
        return FlowDict({"pos_x": F32(0.0), "pos_y": F32(0.0), "dest_offset_x": F32(0.0),
                         "dest_offset_y": F32(0.0), "item_type": 0})
    if kind == "potted":
        return {}
    if kind == "trail":
        return [Flow([F32(0.0), F32(0.0), F32(0.0)]) for _ in range(NUM_MOTION_TRAIL_FRAMES)]
    raise ConvError(f"internal: unknown field kind {kind!r}")


POTTED_PLANT_SCHEMA = [
    ("seed_type", "enum", SEED_TYPE, "SEED_NONE", False),
    ("which_zen_garden", "enum", GARDEN_TYPE, "GARDEN_MAIN", False),
    ("x", "i32", None, 0, False),
    ("y", "i32", None, 0, False),
    ("facing", "enum", FACING_DIRECTION, "FACING_RIGHT", False),
    ("last_watered_time", "i64", None, 0, False),
    ("draw_variation", "enum", DRAW_VARIATION, "VARIATION_NORMAL", False),
    ("plant_age", "enum", POTTED_PLANT_AGE, "PLANTAGE_SPROUT", False),
    ("times_fed", "i32", None, 0, False),
    ("feedings_per_grow", "i32", None, 0, False),
    ("plant_need", "enum", POTTED_PLANT_NEED, "PLANTNEED_NONE", False),
    ("last_need_fulfilled_time", "i64", None, 0, False),
    ("last_fertilized_time", "i64", None, 0, False),
    ("last_chocolate_time", "i64", None, 0, False),
    ("future_attribute", "i64", None, 0, False),
]


def parse_schema_field(kind, arg, reader: BinaryReader, path: str):
    if kind == "bool":
        return reader.read_bool()
    if kind == "i32":
        return reader.read_i32()
    if kind == "u32":
        return reader.read_u32()
    if kind == "i64":
        return reader.read_i64()
    if kind == "f32":
        return F32(reader.read_f32())
    if kind == "enum":
        return enum_to_yaml(arg, reader.read_i32())
    if kind == "i32s":
        return Flow([reader.read_i32() for _ in range(arg)])
    if kind == "u32s":
        return Flow([reader.read_u32() for _ in range(arg)])
    if kind == "bools":
        return Flow([reader.read_bool() for _ in range(arg)])
    if kind == "f32s":
        return Flow([F32(reader.read_f32()) for _ in range(arg)])
    if kind == "enums":
        count, table = arg
        return Flow([enum_to_yaml(table, reader.read_i32()) for _ in range(count)])
    if kind == "rect":
        return FlowDict({"x": reader.read_i32(), "y": reader.read_i32(),
                         "width": reader.read_i32(), "height": reader.read_i32()})
    if kind == "bytes":
        return _b64(reader.read_bytes(arg))
    if kind == "smooth":
        rows = []
        for _ in range(arg):
            rows.append([reader.read_i32(), F32(reader.read_f32()),
                         F32(reader.read_f32()), F32(reader.read_f32())])
        return rows
    if kind == "magnet":
        items = []
        for _ in range(MAX_MAGNET_ITEMS):
            items.append(FlowDict({
                "pos_x": F32(reader.read_f32()), "pos_y": F32(reader.read_f32()),
                "dest_offset_x": F32(reader.read_f32()), "dest_offset_y": F32(reader.read_f32()),
                "item_type": enum_to_yaml(MAGNET_ITEM_TYPE, reader.read_i32()),
            }))
        return items
    if kind == "potted":
        return parse_schema(POTTED_PLANT_SCHEMA, reader, path)
    if kind == "trail":
        return [Flow([F32(reader.read_f32()), F32(reader.read_f32()), F32(reader.read_f32())])
                for _ in range(NUM_MOTION_TRAIL_FRAMES)]
    raise ConvError(f"internal: unknown field kind {kind!r}")


def write_schema_field(kind, arg, value, writer: BinaryWriter, path: str):
    if kind == "bool":
        if isinstance(value, str):
            raise ConvError(f"{path}: expected true/false, got {value!r}")
        writer.write_bool(bool(value))
    elif kind in ("i32", "u32"):
        if isinstance(value, bool) or not isinstance(value, int):
            raise ConvError(f"{path}: expected an integer, got {value!r}")
        lo, hi = (-0x80000000, 0x7FFFFFFF) if kind == "i32" else (0, 0xFFFFFFFF)
        if not lo <= value <= hi:
            raise ConvError(f"{path}: {value} does not fit in {kind}")
        (writer.write_i32 if kind == "i32" else writer.write_u32)(value)
    elif kind == "i64":
        if isinstance(value, bool) or not isinstance(value, int):
            raise ConvError(f"{path}: expected an integer, got {value!r}")
        if not -0x8000000000000000 <= value <= 0x7FFFFFFFFFFFFFFF:
            raise ConvError(f"{path}: {value} does not fit in i64")
        writer.write_i64(value)
    elif kind == "f32":
        if isinstance(value, bool) or not isinstance(value, (int, float)):
            raise ConvError(f"{path}: expected a number, got {value!r}")
        writer.write_f32(float(value))
    elif kind == "enum":
        writer.write_i32(enum_from_yaml(arg, value, path))
    elif kind in ("i32s", "u32s", "bools", "f32s"):
        if not isinstance(value, list) or len(value) != arg:
            raise ConvError(f"{path}: expected a list of {arg} values")
        elem_kind = {"i32s": "i32", "u32s": "u32", "bools": "bool", "f32s": "f32"}[kind]
        for i, elem in enumerate(value):
            write_schema_field(elem_kind, None, elem, writer, f"{path}[{i}]")
    elif kind == "enums":
        count, table = arg
        if not isinstance(value, list) or len(value) != count:
            raise ConvError(f"{path}: expected a list of {count} values")
        for i, elem in enumerate(value):
            writer.write_i32(enum_from_yaml(table, elem, f"{path}[{i}]"))
    elif kind == "rect":
        if not isinstance(value, dict):
            raise ConvError(f"{path}: expected a mapping with x/y/width/height")
        for comp in ("x", "y", "width", "height"):
            writer.write_i32(_get_int(value, comp, f"{path}.{comp}"))
    elif kind == "bytes":
        data = _unb64(value, path)
        if arg is not None and len(data) != arg:
            raise ConvError(f"{path}: expected {arg} bytes, got {len(data)}")
        writer.write_bytes(data)
    elif kind == "smooth":
        if not isinstance(value, list) or len(value) != arg:
            raise ConvError(f"{path}: expected {arg} smooth-array entries")
        for i, entry in enumerate(value):
            if not isinstance(entry, list) or len(entry) != 4:
                raise ConvError(f"{path}[{i}]: expected [item, weight, last_picked, second_last_picked]")
            writer.write_i32(_as_int(entry[0], f"{path}[{i}][0]"))
            writer.write_f32(_as_float(entry[1], f"{path}[{i}][1]"))
            writer.write_f32(_as_float(entry[2], f"{path}[{i}][2]"))
            writer.write_f32(_as_float(entry[3], f"{path}[{i}][3]"))
    elif kind == "magnet":
        if not isinstance(value, list) or len(value) != MAX_MAGNET_ITEMS:
            raise ConvError(f"{path}: expected a list of {MAX_MAGNET_ITEMS} magnet items")
        for i, item in enumerate(value):
            if not isinstance(item, dict):
                raise ConvError(f"{path}[{i}]: expected a mapping")
            writer.write_f32(_as_float(item.get("pos_x", 0.0), f"{path}[{i}].pos_x"))
            writer.write_f32(_as_float(item.get("pos_y", 0.0), f"{path}[{i}].pos_y"))
            writer.write_f32(_as_float(item.get("dest_offset_x", 0.0), f"{path}[{i}].dest_offset_x"))
            writer.write_f32(_as_float(item.get("dest_offset_y", 0.0), f"{path}[{i}].dest_offset_y"))
            writer.write_i32(enum_from_yaml(MAGNET_ITEM_TYPE, item.get("item_type", 0),
                                            f"{path}[{i}].item_type"))
    elif kind == "potted":
        write_schema(POTTED_PLANT_SCHEMA, value or {}, writer, path)
    elif kind == "trail":
        if not isinstance(value, list) or len(value) != NUM_MOTION_TRAIL_FRAMES:
            raise ConvError(f"{path}: expected a list of {NUM_MOTION_TRAIL_FRAMES} trail frames")
        for i, frame in enumerate(value):
            if not isinstance(frame, list) or len(frame) != 3:
                raise ConvError(f"{path}[{i}]: expected [pos_x, pos_y, anim_time]")
            writer.write_f32(_as_float(frame[0], f"{path}[{i}][0]"))
            writer.write_f32(_as_float(frame[1], f"{path}[{i}][1]"))
            writer.write_f32(_as_float(frame[2], f"{path}[{i}][2]"))
    else:
        raise ConvError(f"internal: unknown field kind {kind!r}")


def _as_int(value, path: str) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise ConvError(f"{path}: expected an integer, got {value!r}")
    return value


def _as_float(value, path: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ConvError(f"{path}: expected a number, got {value!r}")
    return float(value)


def _get_int(mapping: dict, key: str, path: str) -> int:
    return _as_int(mapping.get(key, 0), path)


def parse_schema(schema: list, reader: BinaryReader, path: str) -> dict:
    values = {}
    for key, kind, arg, _default, _common in schema:
        values[key] = parse_schema_field(kind, arg, reader, f"{path}.{key}")
    return values


def write_schema(schema: list, values: dict, writer: BinaryWriter, path: str):
    for key, kind, arg, default, _common in schema:
        value = values.get(key, default)
        if value is None:
            value = _default_for(kind, arg)
        write_schema_field(kind, arg, value, writer, f"{path}.{key}")


def split_tiers(schema: list, values: dict) -> dict:
    """Split parsed values into {common fields..., advanced: {...}}."""
    common, advanced = {}, {}
    for key, _kind, _arg, _default, is_common in schema:
        if key not in values:
            continue
        (common if is_common else advanced)[key] = values[key]
    result = dict(common)
    if advanced:
        result["advanced"] = advanced
    return result


def merge_tiers(section: dict, path: str = "section") -> dict:
    """Merge a section's top-level fields with its advanced: sub-mapping."""
    merged = dict(section)
    advanced = merged.pop("advanced", None)
    if advanced is not None:
        if not isinstance(advanced, dict):
            raise ConvError(f"{path}.advanced: expected a mapping")
        for key, value in advanced.items():
            if key in merged:
                raise ConvError(f"{path}.{key}: present both at top level and under advanced:")
            merged[key] = value
    return merged


def schema_keys(schema: list) -> set:
    return {spec[0] for spec in schema}


# ============================================================================
# Schemas
# ============================================================================
#
# Field order within each schema MUST match the corresponding Sync*Portable
# function in src/Lawn/System/SaveGame.cpp byte for byte.

GAMEOBJECT_FIELDS = ["x", "y", "width", "height", "visible", "row", "render_order"]

# Board base: (field_id, yaml_key, kind, arg, common). Field ids must NOT be
# renumbered (on-disk format). "waves" is a special kind handled inline.
BOARD_SCHEMA = [
    (1, "paused", "bool", None, False),
    (2, "grid_square_type", "enums", (54, GRID_SQUARE_TYPE), False),
    (3, "grid_cel_look", "i32s", 54, False),
    (4, "grid_cel_offset", "i32s", 108, False),
    (5, "grid_cel_fog", "i32s", 63, False),
    (6, "enable_gravestones", "bool", None, False),
    (7, "special_gravestone_x", "i32", None, False),
    (8, "special_gravestone_y", "i32", None, False),
    (9, "fog_offset", "f32", None, False),
    (10, "fog_blown_countdown", "i32", None, False),
    (11, "plant_row", "enums", (6, PLANT_ROW_TYPE), False),
    (12, "wave_row_got_lawnmowered", "i32s", 6, False),
    (13, "bonus_lawnmowers_remaining", "i32", None, True),
    (14, "ice_min_x", "i32s", 6, False),
    (15, "ice_timer", "i32s", 6, False),
    (16, "ice_particle_id", "u32s", 6, False),
    (17, "row_picking_array", "smooth", 6, False),
    (18, "waves", "waves", None, True),
    (19, "zombie_allowed", "bools", 100, False),
    (20, "sun_countdown", "i32", None, False),
    (21, "num_suns_fallen", "i32", None, False),
    (22, "shake_counter", "i32", None, False),
    (23, "shake_amount_x", "i32", None, False),
    (24, "shake_amount_y", "i32", None, False),
    (25, "background", "enum", BACKGROUND_TYPE, False),
    (26, "level", "i32", None, False),
    (27, "sod_position", "i32", None, False),
    (28, "prev_mouse_x", "i32", None, False),
    (29, "prev_mouse_y", "i32", None, False),
    (30, "sun_money", "i32", None, True),
    (31, "num_waves", "i32", None, True),
    (32, "main_counter", "u32", None, False),
    (33, "effect_counter", "u32", None, False),
    (34, "draw_count", "u32", None, False),
    (35, "rise_from_grave_counter", "i32", None, True),
    (36, "out_of_money_counter", "i32", None, False),
    (37, "current_wave", "i32", None, True),
    (38, "total_spawned_waves", "i32", None, True),
    (39, "tutorial_state", "enum", TUTORIAL_STATE, False),
    (40, "tutorial_particle_id", "u32", None, False),
    (41, "tutorial_timer", "i32", None, False),
    (42, "last_bungee_wave", "i32", None, False),
    (43, "zombie_health_to_next_wave", "i32", None, False),
    (44, "zombie_health_wave_start", "i32", None, False),
    (45, "zombie_countdown", "i32", None, True),
    (46, "zombie_countdown_start", "i32", None, True),
    (47, "huge_wave_countdown", "i32", None, False),
    (48, "help_displayed", "bools", NUM_ADVICE_TYPES, False),
    (49, "help_index", "i32", None, False),
    (50, "final_boss_killed", "bool", None, False),
    (51, "show_shovel", "bool", None, False),
    (52, "coin_bank_fade_count", "i32", None, False),
    (53, "debug_text_mode", "enum", DEBUG_TEXT_MODE, False),
    (54, "level_complete", "bool", None, False),
    (55, "board_fade_out_counter", "i32", None, False),
    (56, "next_survival_stage_counter", "i32", None, False),
    (57, "score_next_mower_counter", "i32", None, False),
    (58, "level_award_spawned", "bool", None, False),
    (59, "progress_meter_width", "i32", None, False),
    (60, "flag_raise_counter", "i32", None, False),
    (61, "ice_trap_counter", "i32", None, False),
    (62, "board_rand_seed", "i32", None, False),
    (63, "pool_sparkly_particle_id", "u32", None, False),
    (64, "fwoosh_id", "u32s", 72, False),
    (65, "fwoosh_countdown", "i32", None, False),
    (66, "time_stop_counter", "i32", None, False),
    (67, "dropped_first_coin", "bool", None, False),
    (68, "final_wave_sound_counter", "i32", None, False),
    (69, "cob_cannon_cursor_delay_counter", "i32", None, False),
    (70, "cob_cannon_mouse_x", "i32", None, False),
    (71, "cob_cannon_mouse_y", "i32", None, False),
    (72, "killed_yeti", "bool", None, False),
    (73, "mustache_mode", "bool", None, True),
    (74, "super_mower_mode", "bool", None, True),
    (75, "future_mode", "bool", None, True),
    (76, "pinata_mode", "bool", None, True),
    (77, "dance_mode", "bool", None, True),
    (78, "daisy_mode", "bool", None, True),
    (79, "sukhbir_mode", "bool", None, True),
    (80, "prev_board_result", "enum", BOARD_RESULT, False),
    (81, "triggered_lawnmowers", "i32", None, False),
    (82, "play_time_active_level", "u32", None, False),
    (83, "play_time_inactive_level", "u32", None, False),
    (84, "max_sun_plants", "i32", None, False),
    (85, "start_draw_time", "i64", None, False),
    (86, "interval_draw_time", "i64", None, False),
    (87, "interval_draw_count_start", "u32", None, False),
    (88, "min_fps", "f32", None, False),
    (89, "preload_time", "i32", None, False),
    (90, "game_id", "i64", None, False),
    (91, "graves_cleared", "u32", None, False),
    (92, "plants_eaten", "u32", None, False),
    (93, "plants_shoveled", "u32", None, False),
    (94, "pea_shooter_used", "bool", None, False),
    (95, "catapult_plants_used", "bool", None, False),
    (96, "mushroom_and_coffee_beans_only", "bool", None, False),
    (97, "mushrooms_used", "bool", None, False),
    (98, "level_coins_collected", "u32", None, False),
    (99, "gargantuars_kills_by_corn_cob", "u32", None, False),
    (100, "coins_collected", "u32", None, False),
    (101, "diamonds_collected", "u32", None, False),
    (102, "potted_plants_collected", "u32", None, False),
    (103, "chocolate_collected", "u32", None, False),
]

ZOMBIE_SCHEMA = [
    ("zombie_type", "enum", ZOMBIE_TYPE, "ZOMBIE_NORMAL", True),
    ("zombie_phase", "enum", ZOMBIE_PHASE, "PHASE_ZOMBIE_NORMAL", False),
    ("pos_x", "f32", None, 0.0, True),
    ("pos_y", "f32", None, 0.0, True),
    ("vel_x", "f32", None, 0.0, False),
    ("anim_counter", "i32", None, 0, False),
    ("groan_counter", "i32", None, 0, False),
    ("anim_ticks_per_frame", "i32", None, 0, False),
    ("anim_frames", "i32", None, 0, False),
    ("frame", "i32", None, 0, False),
    ("prev_frame", "i32", None, 0, False),
    ("variant", "bool", None, False, False),
    ("is_eating", "bool", None, False, True),
    ("just_got_shot_counter", "i32", None, 0, False),
    ("shield_just_got_shot_counter", "i32", None, 0, False),
    ("shield_recoil_counter", "i32", None, 0, False),
    ("zombie_age", "i32", None, 0, False),
    ("zombie_height", "enum", ZOMBIE_HEIGHT, "HEIGHT_ZOMBIE_NORMAL", False),
    ("phase_counter", "i32", None, 0, False),
    ("from_wave", "i32", None, 0, True),
    ("dropped_loot", "bool", None, False, False),
    ("zombie_fade", "i32", None, 0, False),
    ("flat_tires", "bool", None, False, False),
    ("use_ladder_col", "i32", None, 0, False),
    ("target_col", "i32", None, 0, False),
    ("altitude", "f32", None, 0.0, False),
    ("hit_umbrella", "bool", None, False, False),
    ("zombie_rect", "rect", None, None, False),
    ("zombie_attack_rect", "rect", None, None, False),
    ("chilled_counter", "i32", None, 0, True),
    ("buttered_counter", "i32", None, 0, True),
    ("ice_trap_counter", "i32", None, 0, True),
    ("mind_controlled", "bool", None, False, True),
    ("blowing_away", "bool", None, False, False),
    ("has_head", "bool", None, True, False),
    ("has_arm", "bool", None, True, False),
    ("has_object", "bool", None, False, False),
    ("in_pool", "bool", None, False, False),
    ("on_high_ground", "bool", None, False, False),
    ("yucky_face", "bool", None, False, False),
    ("yucky_face_counter", "i32", None, 0, False),
    ("helm_type", "enum", HELM_TYPE, "HELMTYPE_NONE", True),
    ("body_health", "i32", None, 0, True),
    ("body_max_health", "i32", None, 0, True),
    ("helm_health", "i32", None, 0, True),
    ("helm_max_health", "i32", None, 0, True),
    ("shield_type", "enum", SHIELD_TYPE, "SHIELDTYPE_NONE", True),
    ("shield_health", "i32", None, 0, True),
    ("shield_max_health", "i32", None, 0, True),
    ("flying_health", "i32", None, 0, False),
    ("flying_max_health", "i32", None, 0, False),
    ("dead", "bool", None, False, True),
    ("related_zombie_id", "u32", None, 0, False),
    ("follower_zombie_ids", "u32s", MAX_ZOMBIE_FOLLOWERS, None, False),
    ("playing_song", "bool", None, False, False),
    ("particle_offset_x", "i32", None, 0, False),
    ("particle_offset_y", "i32", None, 0, False),
    ("attachment_id", "i32", None, 0, False),
    ("summon_counter", "i32", None, 0, False),
    ("body_reanim_id", "u32", None, 0, False),
    ("scale_zombie", "f32", None, 0.0, False),
    ("vel_z", "f32", None, 0.0, False),
    ("original_anim_rate", "f32", None, 0.0, False),
    ("target_plant_id", "u32", None, 0, False),
    ("boss_mode", "i32", None, 0, False),
    ("target_row", "i32", None, 0, False),
    ("boss_bungee_counter", "i32", None, 0, False),
    ("boss_stomp_counter", "i32", None, 0, False),
    ("boss_head_counter", "i32", None, 0, False),
    ("boss_fireball_reanim_id", "u32", None, 0, False),
    ("special_head_reanim_id", "u32", None, 0, False),
    ("fireball_row", "i32", None, 0, False),
    ("is_fireball", "bool", None, False, False),
    ("mowered_reanim_id", "u32", None, 0, False),
    ("last_portal_x", "i32", None, 0, False),
    ("zombatar_head_reanim_id", "u32", None, 0, False),
]

PLANT_SCHEMA = [
    ("seed_type", "enum", SEED_TYPE, "SEED_PEASHOOTER", True),
    ("col", "i32", None, 0, True),
    ("anim_counter", "i32", None, 0, False),
    ("frame", "i32", None, 0, False),
    ("frame_length", "i32", None, 0, False),
    ("num_frames", "i32", None, 0, False),
    ("state", "enum", PLANT_STATE, "STATE_NOTREADY", True),
    ("plant_health", "i32", None, 300, True),
    ("plant_max_health", "i32", None, 300, True),
    ("subclass", "enum", PLANT_SUBCLASS, "SUBCLASS_NORMAL", False),
    ("disappear_countdown", "i32", None, 0, False),
    ("do_special_countdown", "i32", None, 0, False),
    ("state_countdown", "i32", None, 0, False),
    ("launch_counter", "i32", None, 0, False),
    ("launch_rate", "i32", None, 0, False),
    ("plant_rect", "rect", None, None, False),
    ("plant_attack_rect", "rect", None, None, False),
    ("target_x", "i32", None, 0, False),
    ("target_y", "i32", None, 0, False),
    ("start_row", "i32", None, 0, False),
    ("particle_id", "u32", None, 0, False),
    ("shooting_counter", "i32", None, 0, False),
    ("body_reanim_id", "u32", None, 0, False),
    ("head_reanim_id", "u32", None, 0, False),
    ("head_reanim_id2", "u32", None, 0, False),
    ("head_reanim_id3", "u32", None, 0, False),
    ("blink_reanim_id", "u32", None, 0, False),
    ("light_reanim_id", "u32", None, 0, False),
    ("sleeping_reanim_id", "u32", None, 0, False),
    ("blink_countdown", "i32", None, 0, False),
    ("recently_eaten_countdown", "i32", None, 0, False),
    ("eaten_flash_countdown", "i32", None, 0, False),
    ("beghouled_flash_countdown", "i32", None, 0, False),
    ("shake_offset_x", "f32", None, 0.0, False),
    ("shake_offset_y", "f32", None, 0.0, False),
    ("magnet_items", "magnet", None, None, False),
    ("target_zombie_id", "u32", None, 0, False),
    ("wake_up_counter", "i32", None, 0, True),
    ("on_bungee_state", "enum", PLANT_ON_BUNGEE_STATE, "NOT_ON_BUNGEE", False),
    ("imitater_type", "enum", SEED_TYPE, "SEED_NONE", True),
    ("potted_plant_index", "i32", None, 0, False),
    ("anim_ping", "bool", None, False, False),
    ("dead", "bool", None, False, True),
    ("squished", "bool", None, False, True),
    ("is_asleep", "bool", None, False, True),
    ("is_on_board", "bool", None, True, False),
    ("highlighted", "bool", None, False, False),
]

PROJECTILE_SCHEMA = [
    ("frame", "i32", None, 0, False),
    ("num_frames", "i32", None, 0, False),
    ("anim_counter", "i32", None, 0, False),
    ("pos_x", "f32", None, 0.0, True),
    ("pos_y", "f32", None, 0.0, True),
    ("pos_z", "f32", None, 0.0, True),
    ("vel_x", "f32", None, 0.0, False),
    ("vel_y", "f32", None, 0.0, False),
    ("vel_z", "f32", None, 0.0, False),
    ("acc_z", "f32", None, 0.0, False),
    ("shadow_y", "f32", None, 0.0, False),
    ("dead", "bool", None, False, True),
    ("anim_ticks_per_frame", "i32", None, 0, False),
    ("motion_type", "enum", PROJECTILE_MOTION, "MOTION_STRAIGHT", True),
    ("projectile_type", "enum", PROJECTILE_TYPE, "PROJECTILE_PEA", True),
    ("projectile_age", "i32", None, 0, False),
    ("click_backoff_counter", "i32", None, 0, False),
    ("rotation", "f32", None, 0.0, False),
    ("rotation_speed", "f32", None, 0.0, False),
    ("on_high_ground", "bool", None, False, False),
    ("damage_range_flags", "i32", None, 0, False),
    ("hit_torchwood_grid_x", "i32", None, 0, False),
    ("attachment_id", "i32", None, 0, False),
    ("cob_target_x", "f32", None, 0.0, False),
    ("cob_target_row", "i32", None, 0, False),
    ("target_zombie_id", "u32", None, 0, False),
    ("last_portal_x", "i32", None, 0, False),
]

COIN_SCHEMA = [
    ("pos_x", "f32", None, 0.0, True),
    ("pos_y", "f32", None, 0.0, True),
    ("vel_x", "f32", None, 0.0, False),
    ("vel_y", "f32", None, 0.0, False),
    ("scale", "f32", None, 0.0, False),
    ("dead", "bool", None, False, True),
    ("fade_count", "i32", None, 0, False),
    ("collect_x", "f32", None, 0.0, False),
    ("collect_y", "f32", None, 0.0, False),
    ("ground_y", "i32", None, 0, False),
    ("coin_age", "i32", None, 0, False),
    ("is_being_collected", "bool", None, False, False),
    ("disappear_counter", "i32", None, 0, True),
    ("type", "enum", COIN_TYPE, "COIN_NONE", True),
    ("coin_motion", "enum", COIN_MOTION, "COIN_MOTION_FROM_SKY", False),
    ("attachment_id", "i32", None, 0, False),
    ("collection_distance", "f32", None, 0.0, False),
    ("usable_seed_type", "enum", SEED_TYPE, "SEED_NONE", True),
    ("potted_plant_spec", "potted", None, None, False),
    ("needs_bouncy_arrow", "bool", None, False, False),
    ("has_bouncy_arrow", "bool", None, False, False),
    ("hit_ground", "bool", None, False, False),
    ("times_dropped", "i32", None, 0, False),
]

MOWER_SCHEMA = [
    ("pos_x", "f32", None, 0.0, True),
    ("pos_y", "f32", None, 0.0, False),
    ("render_order", "i32", None, 0, False),
    ("row", "i32", None, 0, True),
    ("anim_ticks_per_frame", "i32", None, 0, False),
    ("reanim_id", "u32", None, 0, False),
    ("chomp_counter", "i32", None, 0, False),
    ("rolling_in_counter", "i32", None, 0, False),
    ("squished_counter", "i32", None, 0, False),
    ("mower_state", "enum", LAWN_MOWER_STATE, "MOWER_READY", True),
    ("dead", "bool", None, False, True),
    ("visible", "bool", None, True, False),
    ("mower_type", "enum", LAWN_MOWER_TYPE, "LAWNMOWER_LAWN", True),
    ("altitude", "f32", None, 0.0, False),
    ("mower_height", "enum", MOWER_HEIGHT, "MOWER_HEIGHT_LAND", False),
    ("last_portal_x", "i32", None, 0, False),
]

GRIDITEM_SCHEMA = [
    ("grid_item_type", "enum", GRID_ITEM_TYPE, "GRIDITEM_NONE", True),
    ("grid_item_state", "enum", GRID_ITEM_STATE, "GRIDITEM_STATE_NORMAL", True),
    ("grid_x", "i32", None, 0, True),
    ("grid_y", "i32", None, 0, True),
    ("grid_item_counter", "i32", None, 0, False),
    ("render_order", "i32", None, 0, False),
    ("dead", "bool", None, False, True),
    ("pos_x", "f32", None, 0.0, False),
    ("pos_y", "f32", None, 0.0, False),
    ("goal_x", "f32", None, 0.0, False),
    ("goal_y", "f32", None, 0.0, False),
    ("grid_item_reanim_id", "u32", None, 0, False),
    ("grid_item_particle_id", "u32", None, 0, False),
    ("zombie_type", "enum", ZOMBIE_TYPE, "ZOMBIE_INVALID", True),
    ("seed_type", "enum", SEED_TYPE, "SEED_NONE", True),
    ("scary_pot_type", "enum", SCARY_POT_TYPE, "SCARYPOT_NONE", True),
    ("highlighted", "bool", None, False, False),
    ("transparent_counter", "i32", None, 0, False),
    ("sun_count", "i32", None, 0, True),
    ("motion_trail_frames", "trail", None, None, False),
    ("motion_trail_count", "i32", None, 0, False),
]

SEED_PACKET_SCHEMA = [
    ("refresh_counter", "i32", None, 0, True),
    ("refresh_time", "i32", None, 0, True),
    ("index", "i32", None, 0, False),
    ("offset_x", "i32", None, 0, False),
    ("packet_type", "enum", SEED_TYPE, "SEED_NONE", True),
    ("imitater_type", "enum", SEED_TYPE, "SEED_NONE", True),
    ("slot_machine_count_down", "i32", None, 0, False),
    ("slot_machining_next_seed", "enum", SEED_TYPE, "SEED_NONE", False),
    ("slot_machining_position", "f32", None, 0.0, False),
    ("active", "bool", None, True, True),
    ("refreshing", "bool", None, False, True),
    ("times_used", "i32", None, 0, False),
]

SEEDBANK_SCHEMA = [
    ("num_packets", "i32", None, 6, True),
    ("cut_scene_darken", "i32", None, 0, False),
    ("conveyor_belt_counter", "i32", None, 0, False),
]

CURSOR_SCHEMA = [
    ("seed_bank_index", "i32", None, -1, True),
    ("type", "enum", SEED_TYPE, "SEED_NONE", True),
    ("imitater_type", "enum", SEED_TYPE, "SEED_NONE", True),
    ("cursor_type", "enum", CURSOR_TYPE, "CURSOR_TYPE_NORMAL", True),
    ("coin_id", "u32", None, 0, False),
    ("glove_plant_id", "u32", None, 0, False),
    ("duplicator_plant_id", "u32", None, 0, False),
    ("cob_cannon_plant_id", "u32", None, 0, False),
    ("hammer_down_counter", "i32", None, 0, False),
    ("reanim_cursor_id", "u32", None, 0, False),
]

CURSOR_PREVIEW_SCHEMA = [
    ("grid_x", "i32", None, 0, True),
    ("grid_y", "i32", None, 0, True),
]

CHALLENGE_SCHEMA = [
    ("beghouled_mouse_capture", "i32", None, 0, False),
    ("beghouled_mouse_down_x", "i32", None, 0, False),
    ("beghouled_mouse_down_y", "i32", None, 0, False),
    ("beghouled_eated", "i32s", 54, None, False),
    ("beghouled_purchased_upgrade", "i32s", 3, None, False),
    ("beghouled_matches_this_move", "i32", None, 0, False),
    ("challenge_state", "enum", CHALLENGE_STATE, "STATECHALLENGE_NORMAL", True),
    ("challenge_state_counter", "i32", None, 0, True),
    ("conveyor_belt_counter", "i32", None, 0, False),
    ("challenge_score", "i32", None, 0, True),
    ("show_bowling_line", "i32", None, 0, False),
    ("last_conveyor_seed_type", "enum", SEED_TYPE, "SEED_NONE", False),
    ("survival_stage", "i32", None, 0, True),
    ("slot_machine_roll_count", "i32", None, 0, False),
    ("reanim_challenge", "u32", None, 0, False),
    ("reanim_clouds", "u32s", 6, None, False),
    ("clouds_counter", "i32s", 6, None, False),
    ("challenge_grid_x", "i32", None, 0, False),
    ("challenge_grid_y", "i32", None, 0, False),
    ("scary_potter_pots", "i32", None, 0, True),
    ("rain_counter", "i32", None, 0, False),
    ("tree_of_wisdom_talk_index", "i32", None, 0, False),
]

MUSIC_SCHEMA = [
    ("cur_music_tune", "enum", MUSIC_TUNE, "MUSIC_TUNE_NONE", True),
    ("cur_music_file_main", "enum", MUSIC_FILE, "MUSIC_FILE_NONE", False),
    ("cur_music_file_drums", "enum", MUSIC_FILE, "MUSIC_FILE_NONE", False),
    ("cur_music_file_hihats", "enum", MUSIC_FILE, "MUSIC_FILE_NONE", False),
    ("burst_override", "i32", None, 0, False),
    ("base_bpm", "f32", None, 0.0, False),
    ("base_mod_speed", "f32", None, 0.0, False),
    ("music_burst_state", "enum", MUSIC_BURST_STATE, "MUSIC_BURST_OFF", False),
    ("burst_state_counter", "i32", None, 0, False),
    ("music_drums_state", "enum", MUSIC_DRUMS_STATE, "MUSIC_DRUMS_OFF", False),
    ("queued_drum_track_packed_order", "i32", None, 0, False),
    ("drums_state_counter", "i32", None, 0, False),
    ("pause_offset", "i32", None, 0, False),
    ("pause_offset_drums", "i32", None, 0, False),
    ("paused", "bool", None, False, False),
    ("music_disabled", "bool", None, False, False),
    ("fade_out_counter", "i32", None, 0, False),
    ("fade_out_duration", "i32", None, 0, False),
]

# Object-array chunks: section name -> (schema, has GameObject field 1, hoist row)
OBJECT_ARRAYS = {
    "zombies": (ZOMBIE_SCHEMA, True, True),
    "plants": (PLANT_SCHEMA, True, True),
    "projectiles": (PROJECTILE_SCHEMA, True, True),
    "coins": (COIN_SCHEMA, True, True),
    "mowers": (MOWER_SCHEMA, False, False),
    "grid_items": (GRIDITEM_SCHEMA, False, False),
}

# Single-object TLV chunks: section name -> (schema, has GameObject field 1)
SINGLETON_CHUNKS = {
    "seed_bank": (SEEDBANK_SCHEMA, True),
    "challenge": (CHALLENGE_SCHEMA, False),
    "cursor": (CURSOR_SCHEMA, True),
    "cursor_preview": (CURSOR_PREVIEW_SCHEMA, True),
    "music": (MUSIC_SCHEMA, False),
}


# ============================================================================
# GameObject base (field 1 of object items / some singleton chunks)
# ============================================================================

def parse_gameobject(data: bytes) -> dict:
    r = BinaryReader(data)
    return FlowDict({
        "x": r.read_i32(), "y": r.read_i32(), "width": r.read_i32(), "height": r.read_i32(),
        "visible": r.read_bool(), "row": r.read_i32(), "render_order": r.read_i32(),
    })


def write_gameobject(gobj: dict, path: str) -> bytes:
    if not isinstance(gobj, dict):
        raise ConvError(f"{path}: gameobject: expected a mapping")
    w = BinaryWriter()
    for key in ("x", "y", "width", "height"):
        w.write_i32(_as_int(gobj.get(key, 0), f"{path}.gameobject.{key}"))
    visible = gobj.get("visible", True)
    if isinstance(visible, str):
        raise ConvError(f"{path}.gameobject.visible: expected true/false")
    w.write_bool(bool(visible))
    w.write_i32(_as_int(gobj.get("row", 0), f"{path}.gameobject.row"))
    w.write_i32(_as_int(gobj.get("render_order", 0), f"{path}.gameobject.render_order"))
    return w.get_bytes()


# ============================================================================
# Object items
# ============================================================================

def parse_object_item(item_id: int, fields: dict, schema: list, has_base: bool,
                      hoist_row: bool, path: str) -> dict:
    values, gameobject, preserved = {}, None, {}
    for field_id, data in fields.items():
        if field_id == 1 and has_base:
            gameobject = parse_gameobject(data)
        elif field_id == 100:
            r = BinaryReader(data)
            values = parse_schema(schema, r, path)
            if r.remaining:
                preserved["_extra"] = _b64(data[r.pos:])
        else:
            preserved[f"_field_{field_id}"] = _b64(data)
    section = split_tiers(schema, values)
    if hoist_row and gameobject is not None:
        section = {"row": gameobject.pop("row"), **section}
    advanced = section.setdefault("advanced", {})
    if gameobject is not None:
        advanced["gameobject"] = gameobject
    advanced.update(preserved)
    advanced["_id"] = item_id
    return section


def write_object_item(item: dict, schema: list, has_base: bool, hoist_row: bool, path: str) -> bytes:
    merged = merge_tiers(item, path)
    raw = merged.pop("_raw_item", None)
    if raw is not None:
        return _unb64(raw, path)
    merged.pop("_id", None)
    fields = {}
    for key in [k for k in merged if k.startswith("_field_")]:
        try:
            field_id = int(key[len("_field_"):])
        except ValueError:
            raise ConvError(f"{path}: malformed preservation key {key!r}")
        fields[field_id] = _unb64(merged.pop(key), f"{path}.{key}")
    extra_tail = merged.pop("_extra", None)
    if has_base:
        gameobject = merged.pop("gameobject", {})
        if hoist_row:
            gameobject = dict(gameobject)
            gameobject["row"] = merged.pop("row", 0)
        fields[1] = write_gameobject(gameobject, path)
    tail = BinaryWriter()
    write_schema(schema, merged, tail, path)
    tail_bytes = tail.get_bytes()
    if extra_tail is not None:
        tail_bytes += _unb64(extra_tail, f"{path}._extra")
    fields[100] = tail_bytes
    return write_tlv_fields(fields)


# ============================================================================
# Object array chunks (DataArray serialization)
# ============================================================================

def parse_object_array(data: bytes, schema: list, has_base: bool, hoist_row: bool,
                       path: str) -> tuple[dict, list]:
    r = BinaryReader(data)
    header = {
        "free_list_head": r.read_u32(), "max_used_count": r.read_u32(),
        "size": r.read_u32(), "next_key": r.read_u32(), "max_size": r.read_u32(),
    }
    slot_ids = Flow()
    items = []
    for i in range(header["max_used_count"]):
        item_id = r.read_u32()
        item_size = r.read_u32()
        item_data = r.read_bytes(item_size) if item_size else b""
        slot_ids.append(item_id)
        if not (item_id & KEY_MASK):
            continue  # inactive slot: fully described by its id in slot_ids
        try:
            item = parse_object_item(item_id, parse_tlv_fields(item_data), schema,
                                     has_base, hoist_row, f"{path}[{i}]")
        except ConvError:
            item = {"advanced": {"_id": item_id, "_raw_item": _b64(item_data)}}
        items.append(item)
    internal = {
        "free_list_head": header["free_list_head"],
        "size": header["size"],
        "next_key": header["next_key"],
        "max_size": header["max_size"],
        "slot_ids": slot_ids,
    }
    return internal, items


def build_object_array(items: list, internal: dict, schema: list, has_base: bool,
                       hoist_row: bool, path: str, warnings: list) -> bytes:
    items = items or []
    internal = internal or {}
    max_size = internal.get("max_size")
    if not isinstance(max_size, int):
        raise ConvError(f"{path}: missing _internal data-array info for this chunk")
    if len(items) > max_size:
        raise ConvError(f"{path}: {len(items)} objects exceed the array capacity ({max_size})")

    exported_slots = internal.get("slot_ids")
    if exported_slots is not None and (
            not isinstance(exported_slots, list)
            or any(not isinstance(s, int) or isinstance(s, bool) for s in exported_slots)):
        raise ConvError(f"{path}: _internal slot_ids must be a list of integers")
    placement, lossless = {}, isinstance(exported_slots, list)
    if lossless:
        active_positions = {i for i, s in enumerate(exported_slots) if s & KEY_MASK}
        for item in items:
            item_id = _item_id(item)
            if not isinstance(item_id, int):
                lossless = False
                break
            index = item_id & INDEX_MASK
            if index >= len(exported_slots) or exported_slots[index] != item_id or index in placement:
                lossless = False
                break
            placement[index] = item
        if lossless and set(placement) != active_positions:
            lossless = False  # objects were deleted

    if lossless:
        # Every object sits at its original slot; free-list linkage is intact.
        slots = list(exported_slots)
        ordered = [placement.get(i) for i in range(len(slots))]
        free_list_head = _as_int(internal.get("free_list_head", len(slots)),
                                 f"{path}: free_list_head")
        next_key = _as_int(internal.get("next_key", KEY_FIRST), f"{path}: next_key")
    else:
        # Dense rebuild: objects in list order, fresh slot ids.
        if exported_slots is not None and (items or any(s & KEY_MASK for s in exported_slots)):
            warnings.append(
                f"{path}: object list changed; array rebuilt densely. "
                "Cross-object references (targets, followers, attachments) may dangle.")
        slots, ordered = [], []
        next_key = _as_int(internal.get("next_key", KEY_FIRST), f"{path}: next_key")
        for i, item in enumerate(items):
            item_id = _item_id(item)
            key_bits = item_id & KEY_MASK if isinstance(item_id, int) else 0
            if not key_bits:
                key_bits = (next_key << 16) & KEY_MASK
                next_key += 1
                if next_key >= 0x10000:
                    next_key = 1
            slots.append(key_bits | i)
            ordered.append(item)
        free_list_head = len(items)

    w = BinaryWriter()
    w.write_u32(free_list_head)
    w.write_u32(len(slots))       # max_used_count
    w.write_u32(len(items))       # size (active object count)
    w.write_u32(next_key)
    w.write_u32(max_size)
    for i, slot in enumerate(slots):
        w.write_u32(slot)
        item = ordered[i]
        if item is None:
            w.write_u32(0)
        else:
            item_data = write_object_item(item, schema, has_base, hoist_row, f"{path}[{i}]")
            w.write_u32(len(item_data))
            w.write_bytes(item_data)
    return w.get_bytes()


def _item_id(item: dict):
    if not isinstance(item, dict):
        return None
    advanced = item.get("advanced")
    if isinstance(advanced, dict) and isinstance(advanced.get("_id"), int):
        return advanced["_id"]
    return item.get("_id")


# ============================================================================
# Single-object TLV chunks
# ============================================================================

def parse_singleton(data: bytes, schema: list, has_base: bool, path: str) -> dict:
    values, gameobject, preserved = {}, None, {}
    for field_id, field_data in parse_tlv_fields(data).items():
        if field_id == 1 and has_base:
            gameobject = parse_gameobject(field_data)
        elif field_id == 100:
            r = BinaryReader(field_data)
            values = parse_schema(schema, r, path)
            if r.remaining:
                preserved["_extra"] = _b64(field_data[r.pos:])
        else:
            preserved[f"_field_{field_id}"] = _b64(field_data)
    section = split_tiers(schema, values)
    advanced = section.setdefault("advanced", {})
    if gameobject is not None:
        advanced["gameobject"] = gameobject
    advanced.update(preserved)
    return section


def write_singleton(section: dict, schema: list, has_base: bool, path: str) -> bytes:
    merged = merge_tiers(section or {}, path)
    raw = merged.pop("_raw_item", None)
    fields = {}
    for key in [k for k in merged if k.startswith("_field_")]:
        try:
            field_id = int(key[len("_field_"):])
        except ValueError:
            raise ConvError(f"{path}: malformed preservation key {key!r}")
        fields[field_id] = _unb64(merged.pop(key), f"{path}.{key}")
    extra_tail = merged.pop("_extra", None)
    if has_base:
        fields[1] = write_gameobject(merged.pop("gameobject", {}), path)
    tail = BinaryWriter()
    write_schema(schema, merged, tail, path)
    tail_bytes = tail.get_bytes()
    if extra_tail is not None:
        tail_bytes += _unb64(extra_tail, f"{path}._extra")
    fields[100] = tail_bytes
    if raw is not None:
        return _unb64(raw, path)
    return write_tlv_fields(fields)


# ============================================================================
# Seed packets (fixed SEEDBANK_MAX entries, not a DataArray)
# ============================================================================

def parse_seed_packets(data: bytes, path: str) -> list:
    r = BinaryReader(data)
    count = r.read_i32()
    if count != SEEDBANK_MAX:
        print(f"Warning: {path}: unexpected seed packet count {count} "
              f"(expected {SEEDBANK_MAX}); it will be written as {SEEDBANK_MAX}",
              file=sys.stderr)
    packets = []
    for i in range(min(count, SEEDBANK_MAX)):
        item_size = r.read_u32()
        item_data = r.read_bytes(item_size) if item_size else b""
        values, gameobject, preserved = {}, None, {}
        for field_id, field_data in parse_tlv_fields(item_data).items():
            if field_id == 1:
                gameobject = parse_gameobject(field_data)
            elif field_id == 100:
                tr = BinaryReader(field_data)
                values = parse_schema(SEED_PACKET_SCHEMA, tr, f"{path}[{i}]")
                if tr.remaining:
                    preserved["_extra"] = _b64(field_data[tr.pos:])
            else:
                preserved[f"_field_{field_id}"] = _b64(field_data)
        section = split_tiers(SEED_PACKET_SCHEMA, values)
        advanced = section.setdefault("advanced", {})
        if gameobject is not None:
            advanced["gameobject"] = gameobject
        advanced.update(preserved)
        packets.append(section)
    return packets


def write_seed_packets(packets: list, path: str) -> bytes:
    packets = list(packets or [])
    if len(packets) > SEEDBANK_MAX:
        raise ConvError(f"{path}: at most {SEEDBANK_MAX} seed packets, got {len(packets)}")
    while len(packets) < SEEDBANK_MAX:
        packets.append({})
    w = BinaryWriter()
    w.write_i32(SEEDBANK_MAX)
    for i, packet in enumerate(packets):
        item_data = write_singleton(packet, SEED_PACKET_SCHEMA, True, f"{path}[{i}]")
        w.write_u32(len(item_data))
        w.write_bytes(item_data)
    return w.get_bytes()


# ============================================================================
# Board base chunk
# ============================================================================

BOARD_BY_ID = {spec[0]: spec for spec in BOARD_SCHEMA}


def parse_waves(data: bytes, num_waves) -> tuple[list, list, bytes]:
    """Split the wave array into waves, per-wave tails, and the raw remainder.

    The on-disk array is 100 waves x 50 int32. Only the first num_waves waves
    are meaningful. Within a wave the game writes a -1 terminator after the
    last zombie but leaves the slots after it untouched (PutZombieInWave does
    not clear them), and the waves beyond num_waves are entirely stale data.
    Both kinds of leftover bytes are preserved verbatim for a lossless round
    trip; tails that are plain -1 padding are dropped (None)."""
    keep = num_waves if isinstance(num_waves, int) and not isinstance(num_waves, bool) \
        and 0 <= num_waves <= MAX_WAVES else MAX_WAVES
    r = BinaryReader(data)
    waves, tails = [], []
    for _ in range(keep):
        wave = Flow()
        for _ in range(MAX_ZOMBIES_IN_WAVE):
            value = r.read_i32()
            if value == -1:  # ZOMBIE_INVALID terminates the wave
                tail = r.read_bytes((MAX_ZOMBIES_IN_WAVE - len(wave) - 1) * 4)
                tails.append(_b64(tail) if tail != b"\xff" * len(tail) else None)
                break
            wave.append(enum_to_yaml(ZOMBIE_TYPE, value))
        else:
            tails.append(None)  # full wave: no terminator, no tail
        waves.append(wave)
    if not any(tails):
        tails = []
    remainder = data[keep * MAX_ZOMBIES_IN_WAVE * 4:] if keep < MAX_WAVES else b""
    return waves, tails, remainder


def write_waves(waves: list, tails: list, remainder_b64, path: str) -> bytes:
    if not isinstance(waves, list):
        raise ConvError(f"{path}: expected a list of waves")
    if len(waves) > MAX_WAVES:
        raise ConvError(f"{path}: at most {MAX_WAVES} waves, got {len(waves)}")
    if tails and (not isinstance(tails, list) or len(tails) != len(waves)):
        raise ConvError(f"{path}._waves_tails: expected a list of {len(waves)} entries")
    w = BinaryWriter()
    for wave_index, wave in enumerate(waves):
        if not isinstance(wave, list) or len(wave) > MAX_ZOMBIES_IN_WAVE:
            raise ConvError(f"{path}[{wave_index}]: a wave is a list of at most "
                            f"{MAX_ZOMBIES_IN_WAVE} zombie types")
        for i, name in enumerate(wave):
            w.write_i32(enum_from_yaml(ZOMBIE_TYPE, name, f"{path}[{wave_index}][{i}]"))
        tail_len = (MAX_ZOMBIES_IN_WAVE - len(wave) - 1) * 4
        if tail_len < 0:
            tail_len = 0
        if len(wave) < MAX_ZOMBIES_IN_WAVE:
            w.write_i32(-1)  # terminator
        tail = None
        if tails:
            tail = tails[wave_index]
        if tail is not None:
            raw = _unb64(tail, f"{path}._waves_tails[{wave_index}]")
            w.write_bytes(raw[:tail_len].ljust(tail_len, b"\xff"))
        else:
            w.write_bytes(b"\xff" * tail_len)
    body = w.get_bytes()
    remainder = _unb64(remainder_b64, f"{path}._waves_remainder") if remainder_b64 else b""
    if len(remainder) % 4:
        raise ConvError(f"{path}._waves_remainder: length must be a multiple of 4 bytes")
    total = body + remainder
    target = MAX_WAVES * MAX_ZOMBIES_IN_WAVE * 4
    if len(total) > target:
        raise ConvError(f"{path}: wave data exceeds {MAX_WAVES} waves")
    return total + b"\xff\xff\xff\xff" * ((target - len(total)) // 4)


def parse_board(data: bytes) -> dict:
    fields = parse_tlv_fields(data)
    waves_data = fields.pop(18, None)  # parsed after num_waves is known
    values, preserved = {}, {}
    for field_id, field_data in fields.items():
        spec = BOARD_BY_ID.get(field_id)
        if spec is None:
            preserved[f"_unknown_{field_id}"] = _b64(field_data)
            continue
        _, key, kind, arg, _common = spec
        values[key] = parse_schema_field(kind, arg, BinaryReader(field_data), f"board.{key}")
    if waves_data is not None:
        waves, tails, remainder = parse_waves(waves_data, values.get("num_waves"))
        values["waves"] = waves
        if tails:
            preserved["_waves_tails"] = tails
        if remainder:
            preserved["_waves_remainder"] = _b64(remainder)
    schema = [(key, kind, arg, None, common) for _fid, key, kind, arg, common in BOARD_SCHEMA]
    section = split_tiers(schema, values)
    if "waves" in section:
        # Keep the long wave list after the scalar common fields, before advanced.
        waves = section.pop("waves")
        reordered = {}
        for key, value in section.items():
            if key == "advanced":
                reordered["waves"] = waves
            reordered[key] = value
        section = reordered
        section.setdefault("waves", waves)
    if preserved:
        section.setdefault("advanced", {}).update(preserved)
    return section


def write_board(section: dict, path: str) -> bytes:
    merged = merge_tiers(section or {}, path)
    waves_remainder = merged.pop("_waves_remainder", None)
    waves_tails = merged.pop("_waves_tails", None)
    fields = {}
    for key in [k for k in merged if k.startswith("_unknown_")]:
        try:
            field_id = int(key[len("_unknown_"):])
        except ValueError:
            raise ConvError(f"{path}: malformed preservation key {key!r}")
        fields[field_id] = _unb64(merged.pop(key), f"{path}.{key}")
    for field_id, key, kind, arg, _common in BOARD_SCHEMA:
        if key not in merged:
            continue  # absent field: the game falls back to its own default
        value = merged[key]
        if kind == "waves":
            fields[field_id] = write_waves(value, waves_tails or [], waves_remainder,
                                           f"{path}.{key}")
        else:
            w = BinaryWriter()
            write_schema_field(kind, arg, value, w, f"{path}.{key}")
            fields[field_id] = w.get_bytes()
    return write_tlv_fields(fields)


# ============================================================================
# Save file <-> document
# ============================================================================

# chunk name -> (section name, kind)
CHUNK_SECTIONS = {
    "BOARD_BASE": ("board", "board"),
    "ZOMBIES": ("zombies", "array"),
    "PLANTS": ("plants", "array"),
    "PROJECTILES": ("projectiles", "array"),
    "COINS": ("coins", "array"),
    "MOWERS": ("mowers", "array"),
    "GRIDITEMS": ("grid_items", "array"),
    "SEEDBANK": ("seed_bank", "singleton"),
    "SEEDPACKETS": ("seed_packets", "packets"),
    "CHALLENGE": ("challenge", "singleton"),
    "CURSOR": ("cursor", "singleton"),
    "CURSOR_PREVIEW": ("cursor_preview", "singleton"),
    "MUSIC": ("music", "singleton"),
}

SECTION_ORDER = ["board", "seed_bank", "seed_packets", "challenge", "plants", "zombies",
                 "projectiles", "coins", "mowers", "grid_items", "cursor", "cursor_preview",
                 "music"]


def parse_save_file(data: bytes) -> dict:
    if len(data) < HEADER_SIZE:
        raise ConvError(f"file is too small ({len(data)} bytes); not a PvZ-Portable v4 save")
    magic = data[:12]
    if not magic.startswith(b"PVZP_SAVE"):
        raise ConvError("invalid magic; not a PvZ-Portable save file")
    if magic != SAVE_MAGIC:
        raise ConvError(f"unsupported save format {magic.rstrip(bytes(1)).decode(errors='replace')!r}; "
                        "only PVZP_SAVE4 is supported")
    version = struct.unpack("<I", data[12:16])[0]
    if version != SAVE_VERSION:
        raise ConvError(f"unsupported v4 format version {version} (expected {SAVE_VERSION}); "
                        "this save was written by a different version of the game")
    payload_size = struct.unpack("<I", data[16:20])[0]
    stored_crc = struct.unpack("<I", data[20:24])[0]
    payload = data[24:24 + payload_size]
    if len(payload) != payload_size:
        raise ConvError("truncated save file")
    if zlib.crc32(payload) & 0xFFFFFFFF != stored_crc:
        raise ConvError("CRC mismatch; the save file is corrupt")

    doc = {"version": version, "chunk_order": Flow(), "sections": {},
           "arrays_internal": {}, "binary_chunks": {}}
    reader = BinaryReader(payload)
    while reader.remaining >= 8:
        chunk_type = reader.read_u32()
        chunk_size = reader.read_u32()
        if reader.remaining < chunk_size:
            raise ConvError("truncated chunk data")
        chunk_data = reader.read_bytes(chunk_size)
        chunk_name = CHUNK_TYPES.get(chunk_type)
        doc["chunk_order"].append(chunk_name if chunk_name else chunk_type)
        label = chunk_name or f"chunk {chunk_type}"
        try:
            cr = BinaryReader(chunk_data)
            if cr.read_u32() != 1:
                raise ConvError("unsupported chunk version")
            if cr.read_u32() != 1:
                raise ConvError("unexpected field layout")
            inner = cr.read_bytes(cr.read_u32())
            section_info = CHUNK_SECTIONS.get(chunk_name)
            if section_info is None:
                raise ConvError("no readable parser")
            section, kind = section_info
            if kind == "board":
                doc["sections"][section] = parse_board(read_tlv_blob(BinaryReader(inner)))
            elif kind == "array":
                schema, has_base, hoist_row = OBJECT_ARRAYS[section]
                internal, items = parse_object_array(inner, schema, has_base, hoist_row, section)
                doc["sections"][section] = items
                doc["arrays_internal"][section] = internal
            elif kind == "singleton":
                schema, has_base = SINGLETON_CHUNKS[section]
                doc["sections"][section] = parse_singleton(
                    read_tlv_blob(BinaryReader(inner)), schema, has_base, section)
            elif kind == "packets":
                doc["sections"][section] = parse_seed_packets(inner, section)
        except ConvError:
            doc["binary_chunks"][label] = _b64(chunk_data)
    return doc


def write_save_file(doc: dict, warnings: list) -> bytes:
    payload = BinaryWriter()
    for entry in doc["chunk_order"]:
        chunk_name = entry if isinstance(entry, str) else None
        chunk_type = CHUNK_IDS.get(chunk_name) if chunk_name else None
        if chunk_type is None:
            if isinstance(entry, int):
                chunk_type = entry
                chunk_name = f"chunk {entry}"
            else:
                raise ConvError(f"_internal.chunk_order: unknown chunk {entry!r}")
        label = chunk_name if chunk_name in CHUNK_TYPES.values() else f"chunk {chunk_type}"

        binary = doc["binary_chunks"].get(label)
        if binary is not None:
            chunk_data = _unb64(binary, f"_internal.binary_chunks.{label}")
        else:
            section_info = CHUNK_SECTIONS.get(chunk_name)
            if section_info is None:
                raise ConvError(f"_internal.chunk_order: no data for chunk {label}")
            section, kind = section_info
            path = section
            if kind == "board":
                inner = write_tlv_blob(write_board(doc["sections"].get(section), path))
            elif kind == "array":
                schema, has_base, hoist_row = OBJECT_ARRAYS[section]
                inner = build_object_array(doc["sections"].get(section),
                                           doc["arrays_internal"].get(section),
                                           schema, has_base, hoist_row, path, warnings)
            elif kind == "singleton":
                schema, has_base = SINGLETON_CHUNKS[section]
                inner = write_tlv_blob(write_singleton(
                    doc["sections"].get(section), schema, has_base, path))
            elif kind == "packets":
                inner = write_seed_packets(doc["sections"].get(section), path)
            w = BinaryWriter()
            w.write_u32(1)
            w.write_u32(1)
            w.write_u32(len(inner))
            w.write_bytes(inner)
            chunk_data = w.get_bytes()
        payload.write_u32(chunk_type)
        payload.write_u32(len(chunk_data))
        payload.write_bytes(chunk_data)

    payload_bytes = payload.get_bytes()
    result = BinaryWriter()
    result.write_bytes(SAVE_MAGIC)
    result.write_u32(doc["version"])
    result.write_u32(len(payload_bytes))
    result.write_u32(zlib.crc32(payload_bytes) & 0xFFFFFFFF)
    result.write_bytes(payload_bytes)
    return result.get_bytes()


# ============================================================================
# YAML export
# ============================================================================

def export_document(doc: dict) -> dict:
    output = {"save": {"format": "PVZP_SAVE4", "version": doc["version"],
                       "yaml_layout": YAML_LAYOUT}}
    for section in SECTION_ORDER:
        data = doc["sections"].get(section)
        if data is None or data == [] or data == {}:
            continue
        output[section] = data
    internal = {"chunk_order": doc["chunk_order"]}
    if doc["arrays_internal"]:
        internal["data_arrays"] = doc["arrays_internal"]
    if doc["binary_chunks"]:
        internal["binary_chunks"] = doc["binary_chunks"]
    output["_internal"] = internal
    return output


def dump_yaml(output: dict) -> str:
    return yaml.dump(output, Dumper=yaml.SafeDumper, allow_unicode=True,
                     sort_keys=False, default_flow_style=False, width=100)


# ============================================================================
# YAML import + validation
# ============================================================================

def _warn_unknown_keys(mapping: dict, known: set, path: str, warnings: list):
    if not isinstance(mapping, dict):
        return
    for key in mapping:
        if key not in known and not str(key).startswith("_"):
            warnings.append(f"{path}: unknown key {key!r} (ignored)")


def _check_item(item, schema: list, has_base: bool, hoist_row: bool, path: str, warnings: list):
    if not isinstance(item, dict):
        raise ConvError(f"{path}: expected a mapping")
    common_keys = {k for k, _kd, _a, _d, c in schema if c}
    advanced_keys = {k for k, _kd, _a, _d, c in schema if not c}
    if hoist_row:
        common_keys.add("row")
    _warn_unknown_keys(item, common_keys | {"advanced"}, path, warnings)
    advanced = item.get("advanced")
    if advanced is not None:
        if not isinstance(advanced, dict):
            raise ConvError(f"{path}.advanced: expected a mapping")
        known = set(advanced_keys)
        if has_base:
            known.add("gameobject")
        _warn_unknown_keys(advanced, known, f"{path}.advanced", warnings)


def import_document(data, warnings: list) -> dict:
    if not isinstance(data, dict):
        raise ConvError("this YAML is not a PvZ-Portable save export")
    save_info = data.get("save")
    if not isinstance(save_info, dict) or save_info.get("yaml_layout") != YAML_LAYOUT:
        raise ConvError("unsupported YAML layout (expected save.yaml_layout: "
                        f"{YAML_LAYOUT}). Re-export from the .v4 file with this script.")
    version = save_info.get("version", SAVE_VERSION)
    if not isinstance(version, int):
        raise ConvError("save.version: expected an integer")

    internal = data.get("_internal")
    if not isinstance(internal, dict):
        raise ConvError("missing the _internal: section; do not delete it, "
                        "it carries preservation data")
    chunk_order = internal.get("chunk_order")
    if not isinstance(chunk_order, list) or not chunk_order:
        raise ConvError("_internal.chunk_order: expected a non-empty list")
    arrays_internal = internal.get("data_arrays", {})
    if not isinstance(arrays_internal, dict):
        raise ConvError("_internal.data_arrays: expected a mapping")
    binary_chunks = internal.get("binary_chunks", {})
    if not isinstance(binary_chunks, dict):
        raise ConvError("_internal.binary_chunks: expected a mapping")

    known_top = {"save", "_internal"} | set(SECTION_ORDER)
    _warn_unknown_keys(data, known_top, "(top level)", warnings)

    doc = {"version": version, "chunk_order": chunk_order, "sections": {},
           "arrays_internal": arrays_internal, "binary_chunks": binary_chunks}
    for section in SECTION_ORDER:
        value = data.get(section)
        if value is None:
            continue
        path = section
        if section == "board":
            if not isinstance(value, dict):
                raise ConvError("board: expected a mapping")
            common_keys = {k for _f, k, _kd, _a, c in BOARD_SCHEMA if c}
            advanced_keys = {k for _f, k, _kd, _a, c in BOARD_SCHEMA if not c}
            _warn_unknown_keys(value, common_keys | {"advanced"}, path, warnings)
            advanced = value.get("advanced")
            if advanced is not None:
                _warn_unknown_keys(advanced, advanced_keys, f"{path}.advanced", warnings)
            doc["sections"][section] = value
        elif section in OBJECT_ARRAYS:
            if not isinstance(value, list):
                raise ConvError(f"{path}: expected a list")
            schema, has_base, hoist_row = OBJECT_ARRAYS[section]
            for i, item in enumerate(value):
                _check_item(item, schema, has_base, hoist_row, f"{path}[{i}]", warnings)
            doc["sections"][section] = value
        elif section == "seed_packets":
            if not isinstance(value, list):
                raise ConvError(f"{path}: expected a list")
            for i, item in enumerate(value):
                _check_item(item, SEED_PACKET_SCHEMA, True, False, f"{path}[{i}]", warnings)
            doc["sections"][section] = value
        elif section in SINGLETON_CHUNKS:
            if not isinstance(value, dict):
                raise ConvError(f"{path}: expected a mapping")
            schema, has_base = SINGLETON_CHUNKS[section]
            _check_item(value, schema, has_base, False, path, warnings)
            doc["sections"][section] = value
    return doc


# ============================================================================
# info command
# ============================================================================

def _disp(value) -> str:
    """Display an enum/scalar value as-is; enum names double as documentation."""
    return value if isinstance(value, str) else str(value)


def _counts(items: list, key: str) -> dict:
    counts = {}
    for item in items:
        name = _disp(item.get(key))
        counts[name] = counts.get(name, 0) + 1
    return counts


def print_info(doc: dict, file_size: int):
    sections = doc["sections"]
    board = sections.get("board", {})
    board_all = merge_tiers(board) if board else {}

    print("=" * 60)
    print("PvZ-Portable Mid-Level Save (v4)")
    print("=" * 60)
    print(f"  File size: {file_size} bytes, format version: {doc['version']}")
    print(f"  Chunks: {len(doc['chunk_order'])}"
          + (f" ({len(doc['binary_chunks'])} binary-preserved)" if doc["binary_chunks"] else ""))

    if board:
        print("\n[Game State]")
        print(f"  Level: {board_all.get('level', '?')}  "
              f"Background: {_disp(board_all.get('background', '?'))}")
        print(f"  Sun: {board_all.get('sun_money', '?')}")
        print(f"  Wave: {board_all.get('current_wave', '?')} / {board_all.get('num_waves', '?')}"
              f"  (total spawned: {board_all.get('total_spawned_waves', '?')})")
        cheats = [k.replace("_mode", "").replace("_", " ") for k in
                  ("mustache_mode", "super_mower_mode", "future_mode", "pinata_mode",
                   "dance_mode", "daisy_mode", "sukhbir_mode") if board_all.get(k)]
        if cheats:
            print(f"  Cheats active: {', '.join(cheats)}")
        if board_all.get("paused"):
            print("  Paused: yes")
        if board_all.get("level_complete"):
            print("  Level complete: yes")

    challenge = sections.get("challenge")
    if challenge:
        challenge = merge_tiers(challenge)
        interesting = {k: challenge[k] for k in
                       ("survival_stage", "challenge_score", "scary_potter_pots")
                       if challenge.get(k)}
        if interesting:
            print("\n[Challenge]")
            for key, value in interesting.items():
                print(f"  {key}: {value}")

    packets = sections.get("seed_packets", [])
    if packets:
        print("\n[Seed Slots]")
        for i, packet in enumerate(packets):
            merged = merge_tiers(packet)
            packet_type = merged.get("packet_type", "SEED_NONE")
            if packet_type in ("SEED_NONE", -1):
                continue
            name = _disp(packet_type)
            imitater = merged.get("imitater_type")
            if isinstance(imitater, str) and imitater != "SEED_NONE":
                name += f" (Imitater: {_disp(imitater)})"
            if not merged.get("active", True):
                state = "inactive"
            elif merged.get("refreshing"):
                counter = merged.get("refresh_counter", 0) / 100.0
                total = merged.get("refresh_time", 0) / 100.0
                state = f"recharging {counter:.2f}s/{total:.2f}s"
            else:
                state = "ready"
            print(f"  [{i + 1}] {name} ({state})")

    plants = sections.get("plants", [])
    if plants:
        print(f"\n[Plants] {len(plants)} on field")
        counts = _counts(plants, "seed_type")
        print("  " + ", ".join(f"{name} x{count}" for name, count in
                               sorted(counts.items(), key=lambda kv: -kv[1])))

    zombies = sections.get("zombies", [])
    if zombies:
        print(f"\n[Zombies] {len(zombies)} on field")
        counts = _counts(zombies, "zombie_type")
        print("  " + ", ".join(f"{name} x{count}" for name, count in
                               sorted(counts.items(), key=lambda kv: -kv[1])))

    coins = sections.get("coins", [])
    if coins:
        counts = _counts(coins, "type")
        print(f"\n[Coins/Pickups] {len(coins)}")
        print("  " + ", ".join(f"{name} x{count}" for name, count in
                               sorted(counts.items(), key=lambda kv: -kv[1])))

    mowers = sections.get("mowers", [])
    if mowers:
        print(f"\n[Lawn Mowers]")
        for mower in mowers:
            merged = merge_tiers(mower)
            print(f"  row {merged.get('row', '?')}: "
                  f"{_disp(merged.get('mower_type', '?'))} "
                  f"({_disp(merged.get('mower_state', '?'))})")

    grid_items = sections.get("grid_items", [])
    if grid_items:
        counts = _counts(grid_items, "grid_item_type")
        print(f"\n[Grid Items] {len(grid_items)}")
        print("  " + ", ".join(f"{name} x{count}" for name, count in
                               sorted(counts.items(), key=lambda kv: -kv[1])))
        pots = [item for item in grid_items
                if item.get("grid_item_type") == "GRIDITEM_SCARY_POT"]
        if pots:
            print("  Vase contents:")
            for pot in pots:
                merged = merge_tiers(pot)
                content = merged.get("scary_pot_type", "SCARYPOT_NONE")
                detail = ""
                if content == "SCARYPOT_ZOMBIE":
                    detail = _disp(merged.get("zombie_type", "?"))
                elif content == "SCARYPOT_SEED":
                    detail = _disp(merged.get("seed_type", "?"))
                elif content == "SCARYPOT_SUN":
                    detail = f"{merged.get('sun_count', '?')} sun"
                print(f"    [{merged.get('grid_x', '?')},{merged.get('grid_y', '?')}] "
                      f"{_disp(content)} {detail}".rstrip())

    music = sections.get("music")
    if music:
        tune = merge_tiers(music).get("cur_music_tune")
        if tune not in (None, "MUSIC_TUNE_NONE", -1):
            print(f"\n[Music] {_disp(tune)}")


# ============================================================================
# CLI
# ============================================================================

def _read_file(path: str) -> bytes:
    try:
        with open(path, "rb") as f:
            return f.read()
    except OSError as e:
        raise ConvError(f"cannot read {path}: {e.strerror or e}")


def cmd_info(args):
    data = _read_file(args.input)
    print_info(parse_save_file(data), len(data))


def cmd_export(args):
    doc = parse_save_file(_read_file(args.input))
    text = dump_yaml(export_document(doc))
    try:
        with open(args.output, "w", encoding="utf-8") as f:
            f.write(text)
    except OSError as e:
        raise ConvError(f"cannot write {args.output}: {e.strerror or e}")
    print(f"Exported to: {args.output}")
    print("Edit the YAML, then rebuild the save with: "
          f"{args.prog} import {args.output} <output.v4>")


def cmd_import(args):
    try:
        with open(args.input, "r", encoding="utf-8") as f:
            data = yaml.safe_load(f)
    except yaml.YAMLError as e:
        raise ConvError(f"invalid YAML in {args.input}: {e}")
    except OSError as e:
        raise ConvError(f"cannot read {args.input}: {e.strerror or e}")
    warnings = []
    doc = import_document(data, warnings)
    result = write_save_file(doc, warnings)
    try:
        with open(args.output, "wb") as f:
            f.write(result)
    except OSError as e:
        raise ConvError(f"cannot write {args.output}: {e.strerror or e}")
    for warning in warnings:
        print(f"Warning: {warning}", file=sys.stderr)
    print(f"Imported to: {args.output} ({len(result)} bytes)")


def main():
    parser = argparse.ArgumentParser(
        prog="pvzp-v4-converter.py",
        description="PvZ-Portable v4 mid-level save editor: lossless .v4 <-> YAML conversion.",
        epilog="How to edit the YAML: enum fields take names such as ZOMBIE_NORMAL. "
               "Timers are in centiseconds, so 100 means one second. Positions are "
               "pixels on an 800x600 lawn; rows run 0-5 from top to bottom and "
               "columns 0-8 from left to right. Fields under 'advanced' are internal "
               "state that is preserved as-is and rarely needs changes, and '_internal' "
               "is preservation data that should not be edited. Adding or deleting "
               "objects rebuilds the whole array and may break references between "
               "objects, so editing existing values is safer. The game mode is not "
               "stored in the save; a save always loads in the level it came from.")
    subparsers = parser.add_subparsers(dest="command", required=True)

    p_info = subparsers.add_parser("info", help="print a summary of a .v4 save file")
    p_info.add_argument("input", help="input .v4 file")
    p_info.set_defaults(func=cmd_info)

    p_export = subparsers.add_parser("export", help="convert a .v4 save to editable YAML")
    p_export.add_argument("input", help="input .v4 file")
    p_export.add_argument("output", help="output .yaml file")
    p_export.set_defaults(func=cmd_export)

    p_import = subparsers.add_parser("import", help="rebuild a .v4 save from edited YAML")
    p_import.add_argument("input", help="input .yaml file")
    p_import.add_argument("output", help="output .v4 file")
    p_import.set_defaults(func=cmd_import)

    args = parser.parse_args()
    args.prog = parser.prog
    try:
        args.func(args)
    except ConvError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
