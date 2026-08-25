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

"""PvZ-Portable / PvZ GOTY global user data editor:
lossless users.dat / user<N>.dat <-> YAML conversion."""

import argparse
import base64
import datetime
import os
import shutil
import struct
import sys
import tempfile
import time

try:
    import yaml
except ModuleNotFoundError:
    sys.exit("Error: PyYAML is required. Install it with: python3 -m pip install pyyaml")

# ============================================================================
# Constants
# ============================================================================

USER_FILE_VERSION = 12       # user<N>.dat
INDEX_FILE_VERSION = 14      # users.dat
YAML_LAYOUT = 1

MAX_POTTED_PLANTS = 200
MAX_ZOMBATAR_HEADS = 100
ZOMBATAR_SLOTS = 18          # 18 x u32 = 0x48 bytes per head
MINIGAME_FLAGS_LEN = 0x14    # regenerated from challenge records on write

POTTED_PLANT_SIZE = 0x58     # 88 bytes, matches the original game's record


class ConvError(Exception):
    """A user-facing conversion error (bad file or bad YAML input)."""


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
}

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
    16: "VARIATION_IMITATER_LESS", 17: "VARIATION_AQUARIUM",
}

STORE_ITEM = {
    0: "STORE_ITEM_PLANT_GATLINGPEA", 1: "STORE_ITEM_PLANT_TWINSUNFLOWER",
    2: "STORE_ITEM_PLANT_GLOOMSHROOM", 3: "STORE_ITEM_PLANT_CATTAIL",
    4: "STORE_ITEM_PLANT_WINTERMELON", 5: "STORE_ITEM_PLANT_GOLD_MAGNET",
    6: "STORE_ITEM_PLANT_SPIKEROCK", 7: "STORE_ITEM_PLANT_COBCANNON",
    8: "STORE_ITEM_PLANT_IMITATER", 9: "STORE_ITEM_BONUS_LAWN_MOWER",
    10: "STORE_ITEM_POTTED_MARIGOLD_1", 11: "STORE_ITEM_POTTED_MARIGOLD_2",
    12: "STORE_ITEM_POTTED_MARIGOLD_3", 13: "STORE_ITEM_GOLD_WATERINGCAN",
    14: "STORE_ITEM_FERTILIZER", 15: "STORE_ITEM_BUG_SPRAY", 16: "STORE_ITEM_PHONOGRAPH",
    17: "STORE_ITEM_GARDENING_GLOVE", 18: "STORE_ITEM_MUSHROOM_GARDEN",
    19: "STORE_ITEM_WHEEL_BARROW", 20: "STORE_ITEM_STINKY_THE_SNAIL",
    21: "STORE_ITEM_PACKET_UPGRADE", 22: "STORE_ITEM_POOL_CLEANER",
    23: "STORE_ITEM_ROOF_CLEANER", 24: "STORE_ITEM_RAKE", 25: "STORE_ITEM_AQUARIUM_GARDEN",
    26: "STORE_ITEM_CHOCOLATE", 27: "STORE_ITEM_TREE_OF_WISDOM", 28: "STORE_ITEM_TREE_FOOD",
    29: "STORE_ITEM_FIRSTAID", 30: "STORE_ITEM_PVZ",
}

# Game modes for challenge record names (index i in mChallengeRecords is
# game mode (i + 1), see PlayerInfo::ResetChallengeRecord).
GAME_MODE = {
    0: "GAMEMODE_ADVENTURE",
    1: "GAMEMODE_SURVIVAL_NORMAL_STAGE_1", 2: "GAMEMODE_SURVIVAL_NORMAL_STAGE_2",
    3: "GAMEMODE_SURVIVAL_NORMAL_STAGE_3", 4: "GAMEMODE_SURVIVAL_NORMAL_STAGE_4",
    5: "GAMEMODE_SURVIVAL_NORMAL_STAGE_5", 6: "GAMEMODE_SURVIVAL_HARD_STAGE_1",
    7: "GAMEMODE_SURVIVAL_HARD_STAGE_2", 8: "GAMEMODE_SURVIVAL_HARD_STAGE_3",
    9: "GAMEMODE_SURVIVAL_HARD_STAGE_4", 10: "GAMEMODE_SURVIVAL_HARD_STAGE_5",
    11: "GAMEMODE_SURVIVAL_ENDLESS_STAGE_1", 12: "GAMEMODE_SURVIVAL_ENDLESS_STAGE_2",
    13: "GAMEMODE_SURVIVAL_ENDLESS_STAGE_3", 14: "GAMEMODE_SURVIVAL_ENDLESS_STAGE_4",
    15: "GAMEMODE_SURVIVAL_ENDLESS_STAGE_5", 16: "GAMEMODE_CHALLENGE_WAR_AND_PEAS",
    17: "GAMEMODE_CHALLENGE_WALLNUT_BOWLING", 18: "GAMEMODE_CHALLENGE_SLOT_MACHINE",
    19: "GAMEMODE_CHALLENGE_RAINING_SEEDS", 20: "GAMEMODE_CHALLENGE_BEGHOULED",
    21: "GAMEMODE_CHALLENGE_INVISIGHOUL", 22: "GAMEMODE_CHALLENGE_SEEING_STARS",
    23: "GAMEMODE_CHALLENGE_ZOMBIQUARIUM", 24: "GAMEMODE_CHALLENGE_BEGHOULED_TWIST",
    25: "GAMEMODE_CHALLENGE_LITTLE_TROUBLE", 26: "GAMEMODE_CHALLENGE_PORTAL_COMBAT",
    27: "GAMEMODE_CHALLENGE_COLUMN", 28: "GAMEMODE_CHALLENGE_BOBSLED_BONANZA",
    29: "GAMEMODE_CHALLENGE_SPEED", 30: "GAMEMODE_CHALLENGE_WHACK_A_ZOMBIE",
    31: "GAMEMODE_CHALLENGE_LAST_STAND", 32: "GAMEMODE_CHALLENGE_WAR_AND_PEAS_2",
    33: "GAMEMODE_CHALLENGE_WALLNUT_BOWLING_2", 34: "GAMEMODE_CHALLENGE_POGO_PARTY",
    35: "GAMEMODE_CHALLENGE_FINAL_BOSS", 36: "GAMEMODE_CHALLENGE_ART_CHALLENGE_WALLNUT",
    37: "GAMEMODE_CHALLENGE_SUNNY_DAY", 38: "GAMEMODE_CHALLENGE_RESODDED",
    39: "GAMEMODE_CHALLENGE_BIG_TIME", 40: "GAMEMODE_CHALLENGE_ART_CHALLENGE_SUNFLOWER",
    41: "GAMEMODE_CHALLENGE_AIR_RAID", 42: "GAMEMODE_CHALLENGE_ICE",
    43: "GAMEMODE_CHALLENGE_ZEN_GARDEN", 44: "GAMEMODE_CHALLENGE_HIGH_GRAVITY",
    45: "GAMEMODE_CHALLENGE_GRAVE_DANGER", 46: "GAMEMODE_CHALLENGE_SHOVEL",
    47: "GAMEMODE_CHALLENGE_STORMY_NIGHT", 48: "GAMEMODE_CHALLENGE_BUNGEE_BLITZ",
    49: "GAMEMODE_CHALLENGE_SQUIRREL", 50: "GAMEMODE_TREE_OF_WISDOM",
    51: "GAMEMODE_SCARY_POTTER_1", 52: "GAMEMODE_SCARY_POTTER_2", 53: "GAMEMODE_SCARY_POTTER_3",
    54: "GAMEMODE_SCARY_POTTER_4", 55: "GAMEMODE_SCARY_POTTER_5", 56: "GAMEMODE_SCARY_POTTER_6",
    57: "GAMEMODE_SCARY_POTTER_7", 58: "GAMEMODE_SCARY_POTTER_8", 59: "GAMEMODE_SCARY_POTTER_9",
    60: "GAMEMODE_SCARY_POTTER_ENDLESS", 61: "GAMEMODE_PUZZLE_I_ZOMBIE_1",
    62: "GAMEMODE_PUZZLE_I_ZOMBIE_2", 63: "GAMEMODE_PUZZLE_I_ZOMBIE_3",
    64: "GAMEMODE_PUZZLE_I_ZOMBIE_4", 65: "GAMEMODE_PUZZLE_I_ZOMBIE_5",
    66: "GAMEMODE_PUZZLE_I_ZOMBIE_6", 67: "GAMEMODE_PUZZLE_I_ZOMBIE_7",
    68: "GAMEMODE_PUZZLE_I_ZOMBIE_8", 69: "GAMEMODE_PUZZLE_I_ZOMBIE_9",
    70: "GAMEMODE_PUZZLE_I_ZOMBIE_ENDLESS", 71: "GAMEMODE_UPSELL", 72: "GAMEMODE_INTRO",
}

ACHIEVEMENTS = [
    "HomeSecurity", "NovelPeasPrize", "BetterOffDead", "ChinaShop", "Spudow",
    "Explodonator", "Morticulturalist", "DontPea", "RollSomeHeads", "Grounded",
    "Zombologist", "PennyPincher", "SunnyDays", "PopcornParty", "GoodMorning",
    "NoFungusAmongUs", "BeyondTheGrave", "Immortal", "ToweringWisdom", "MustacheMode",
]

ZOMBATAR_SLOT_NAMES = [
    "skin", "skin_color", "clothes", "clothes_color", "tidbits", "tidbits_color",
    "accessory", "accessory_color", "facial_hair", "facial_hair_color",
    "hair", "hair_color", "eyewear", "eyewear_color", "hats", "hats_color",
    "background", "background_color",
]

# u32 detail fields after version, before potted plants (PlayerInfo::SyncDetails).
DETAIL_SCALARS = [
    "level", "coins", "finished_adventure",
    # mChallengeRecords[100] and mPurchases[80] are handled separately
    "play_time_active", "play_time_inactive", "has_used_cheat_keys",
    "has_woken_stinky", "didnt_purchase_packet_upgrade", "last_stinky_chocolate_time",
    "stinky_pos_x", "stinky_pos_y", "has_unlocked_minigames", "has_unlocked_puzzle_mode",
    "has_new_mini_game", "has_new_scary_potter", "has_new_i_zombie", "has_new_survival",
    "has_unlocked_survival_mode", "needs_message_on_game_selector",
    "needs_magic_taco_reward", "has_seen_stinky", "has_seen_upsell",
    "placeholder_player_stats",
]


# ============================================================================
# Binary layer (little-endian, matching src/Lawn/System/DataSync.cpp)
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

    def read_u16(self) -> int:
        return struct.unpack("<H", self.read_bytes(2))[0]

    def read_u32(self) -> int:
        return struct.unpack("<I", self.read_bytes(4))[0]

    def read_i32(self) -> int:
        return struct.unpack("<i", self.read_bytes(4))[0]

    def read_i64(self) -> int:
        return struct.unpack("<q", self.read_bytes(8))[0]


class BinaryWriter:
    def __init__(self):
        self.data = bytearray()

    def write_bytes(self, data: bytes):
        self.data.extend(data)

    def write_u8(self, value: int):
        self.data.append(value & 0xFF)

    def write_u16(self, value: int):
        self.data.extend(struct.pack("<H", value & 0xFFFF))

    def write_u32(self, value: int):
        self.data.extend(struct.pack("<I", value & 0xFFFFFFFF))

    def write_i32(self, value: int):
        self.data.extend(struct.pack("<i", int(value)))

    def write_i64(self, value: int):
        self.data.extend(struct.pack("<q", int(value)))

    def get_bytes(self) -> bytes:
        return bytes(self.data)


def _b64(data: bytes) -> str:
    return base64.b64encode(data).decode("ascii")


def _unb64(text: str, path: str) -> bytes:
    try:
        return base64.b64decode(text, validate=True)
    except Exception:
        raise ConvError(f"{path}: invalid base64 data")


def _as_int(value, path: str, lo: int = 0, hi: int = 0xFFFFFFFF) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise ConvError(f"{path}: expected an integer, got {value!r}")
    if not lo <= value <= hi:
        raise ConvError(f"{path}: {value} out of range [{lo}, {hi}]")
    return value


def _as_i64(value, path: str) -> int:
    return _as_int(value, path, -0x8000000000000000, 0x7FFFFFFFFFFFFFFF)


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


def _name_to_yaml(raw: bytes):
    """Profile names: plain string when valid UTF-8, base64 otherwise."""
    try:
        return raw.decode("utf-8")
    except UnicodeDecodeError:
        return {"_name_b64": _b64(raw)}


def _name_from_yaml(value, path: str) -> bytes:
    if isinstance(value, str):
        return value.encode("utf-8")
    if isinstance(value, dict) and set(value) == {"_name_b64"}:
        return _unb64(value["_name_b64"], path)
    raise ConvError(f"{path}: expected a string (or _name_b64 for non-UTF-8 names)")


# ============================================================================
# PottedPlant record (0x58 bytes, original game's layout; see PlayerInfo.h)
# ============================================================================

def parse_potted_plant(data: bytes) -> dict:
    r = BinaryReader(data)
    plant = {
        "seed_type": enum_to_yaml(SEED_TYPE, r.read_i32()),
        "which_zen_garden": enum_to_yaml(GARDEN_TYPE, r.read_i32()),
        "x": r.read_i32(),
        "y": r.read_i32(),
        "facing": enum_to_yaml(FACING_DIRECTION, r.read_i32()),
    }
    padding1 = r.read_bytes(4)
    plant["last_watered_time"] = r.read_i64()
    plant["draw_variation"] = enum_to_yaml(DRAW_VARIATION, r.read_i32())
    plant["plant_age"] = enum_to_yaml(POTTED_PLANT_AGE, r.read_i32())
    plant["times_fed"] = r.read_i32()
    plant["feedings_per_grow"] = r.read_i32()
    plant["plant_need"] = enum_to_yaml(POTTED_PLANT_NEED, r.read_i32())
    padding2 = r.read_bytes(4)
    plant["last_need_fulfilled_time"] = r.read_i64()
    plant["last_fertilized_time"] = r.read_i64()
    plant["last_chocolate_time"] = r.read_i64()
    plant["future_attribute"] = r.read_i64()
    if padding1 != b"\x00" * 4:
        plant["_padding1"] = _b64(padding1)
    if padding2 != b"\x00" * 4:
        plant["_padding2"] = _b64(padding2)
    return plant


def write_potted_plant(plant: dict, path: str) -> bytes:
    if not isinstance(plant, dict):
        raise ConvError(f"{path}: expected a mapping")
    w = BinaryWriter()
    w.write_i32(enum_from_yaml(SEED_TYPE, plant.get("seed_type", "SEED_NONE"),
                               f"{path}.seed_type"))
    w.write_i32(enum_from_yaml(GARDEN_TYPE, plant.get("which_zen_garden", "GARDEN_MAIN"),
                               f"{path}.which_zen_garden"))
    w.write_i32(_as_int(plant.get("x", 0), f"{path}.x", -0x80000000, 0x7FFFFFFF))
    w.write_i32(_as_int(plant.get("y", 0), f"{path}.y", -0x80000000, 0x7FFFFFFF))
    w.write_i32(enum_from_yaml(FACING_DIRECTION, plant.get("facing", "FACING_RIGHT"),
                               f"{path}.facing"))
    w.write_bytes(_unb64(plant["_padding1"], f"{path}._padding1")
                  if "_padding1" in plant else b"\x00" * 4)
    w.write_i64(_as_i64(plant.get("last_watered_time", 0), f"{path}.last_watered_time"))
    w.write_i32(enum_from_yaml(DRAW_VARIATION, plant.get("draw_variation", "VARIATION_NORMAL"),
                               f"{path}.draw_variation"))
    w.write_i32(enum_from_yaml(POTTED_PLANT_AGE, plant.get("plant_age", "PLANTAGE_SPROUT"),
                               f"{path}.plant_age"))
    w.write_i32(_as_int(plant.get("times_fed", 0), f"{path}.times_fed",
                        -0x80000000, 0x7FFFFFFF))
    w.write_i32(_as_int(plant.get("feedings_per_grow", 0), f"{path}.feedings_per_grow",
                        -0x80000000, 0x7FFFFFFF))
    w.write_i32(enum_from_yaml(POTTED_PLANT_NEED, plant.get("plant_need", "PLANTNEED_NONE"),
                               f"{path}.plant_need"))
    w.write_bytes(_unb64(plant["_padding2"], f"{path}._padding2")
                  if "_padding2" in plant else b"\x00" * 4)
    w.write_i64(_as_i64(plant.get("last_need_fulfilled_time", 0),
                        f"{path}.last_need_fulfilled_time"))
    w.write_i64(_as_i64(plant.get("last_fertilized_time", 0), f"{path}.last_fertilized_time"))
    w.write_i64(_as_i64(plant.get("last_chocolate_time", 0), f"{path}.last_chocolate_time"))
    w.write_i64(_as_i64(plant.get("future_attribute", 0), f"{path}.future_attribute"))
    result = w.get_bytes()
    assert len(result) == POTTED_PLANT_SIZE
    return result


# ============================================================================
# Zombatar head record (18 x u32; values above INT32_MAX mean "none")
# ============================================================================

def parse_zombatar_head(data: bytes) -> dict:
    r = BinaryReader(data)
    head = {}
    for slot, name in enumerate(ZOMBATAR_SLOT_NAMES):
        value = r.read_u32()
        head[name] = -1 if value == 0xFFFFFFFF else value
    return head


def write_zombatar_head(head: dict, path: str) -> bytes:
    if not isinstance(head, dict):
        raise ConvError(f"{path}: expected a mapping")
    w = BinaryWriter()
    for name in ZOMBATAR_SLOT_NAMES:
        value = head.get(name, -1)
        if not isinstance(value, int) or isinstance(value, bool):
            raise ConvError(f"{path}.{name}: expected an integer, got {value!r}")
        if value == -1:
            w.write_u32(0xFFFFFFFF)
        else:
            w.write_u32(_as_int(value, f"{path}.{name}", 0, 0xFFFFFFFF))
    return w.get_bytes()


# ============================================================================
# user<N>.dat (profile details, format version 12)
# ============================================================================

def parse_details(data: bytes) -> dict:
    r = BinaryReader(data)
    version = r.read_u32()
    if version != USER_FILE_VERSION:
        raise ConvError(f"unsupported user file version {version} "
                        f"(expected {USER_FILE_VERSION}); only the GOTY format is supported")
    doc = {"version": version}
    values = {}
    values["level"] = r.read_u32()
    values["coins"] = r.read_u32()
    values["finished_adventure"] = r.read_u32()
    challenge_records = [r.read_u32() for _ in range(100)]
    purchases = [r.read_u32() for _ in range(80)]
    for key in DETAIL_SCALARS[3:]:
        values[key] = r.read_u32()
    num_plants = r.read_u32()
    if num_plants > MAX_POTTED_PLANTS:
        raise ConvError(f"num_potted_plants {num_plants} exceeds {MAX_POTTED_PLANTS}; "
                        "the file is corrupt")
    plants = [parse_potted_plant(r.read_bytes(POTTED_PLANT_SIZE)) for _ in range(num_plants)]
    achievements = [r.read_u16() for _ in range(20)]

    # Zombatar tail: the game tolerates its absence (older files end earlier).
    zombatar = {"accepted": 0, "created_before": 0, "heads": []}
    tail_present = True
    tail_start = r.pos
    try:
        zombatar["accepted"] = r.read_u8()
        head_count = r.read_u32()
        if head_count > MAX_ZOMBATAR_HEADS:
            raise ConvError("zombatar head count out of range")
        zombatar["heads"] = [parse_zombatar_head(r.read_bytes(ZOMBATAR_SLOTS * 4))
                             for _ in range(head_count)]
        r.read_bytes(MINIGAME_FLAGS_LEN)  # derived from challenge records; discarded
        zombatar["created_before"] = r.read_u8()
        trailing = r.read_bytes(r.remaining) if r.remaining else b""
    except ConvError:
        zombatar = {"accepted": 0, "created_before": 0, "heads": []}
        tail_present = False
        # Keep the unconsumed tail verbatim: a truncated zombatar section
        # round-trips byte-identically instead of losing the partial bytes.
        trailing = data[tail_start:]

    # Sparse, named presentation; zeros are dropped and rewritten as zeros.
    doc["values"] = values
    doc["challenge_records"] = {
        (GAME_MODE.get(i + 1) or f"_record_{i}"): value
        for i, value in enumerate(challenge_records) if value != 0
    }
    doc["purchases"] = {
        (STORE_ITEM.get(i) or f"_purchase_{i}"): value
        for i, value in enumerate(purchases) if value != 0
    }
    doc["potted_plants"] = plants
    doc["achievements"] = [ACHIEVEMENTS[i] for i, v in enumerate(achievements) if v != 0]
    doc["zombatar"] = zombatar
    doc["zombatar_tail_present"] = tail_present
    if trailing:
        doc["trailing"] = _b64(trailing)
    return doc


def write_details(doc: dict) -> bytes:
    values = doc.get("values") or {}
    w = BinaryWriter()
    w.write_u32(_as_int(doc.get("version", USER_FILE_VERSION), "save.version", 0, 0xFFFFFFFF))
    for key in DETAIL_SCALARS[:3]:
        w.write_u32(_as_int(values.get(key, 0 if key != "level" else 1),
                            f"{key}"))
    records = [0] * 100
    for name, value in (doc.get("challenge_records") or {}).items():
        index = _record_index(name)
        records[index] = _as_int(value, f"challenge_records.{name}")
    for value in records:
        w.write_u32(value)
    purchase_slots = [0] * 80
    for name, value in (doc.get("purchases") or {}).items():
        index = _purchase_index(name)
        purchase_slots[index] = _as_int(value, f"purchases.{name}")
    for value in purchase_slots:
        w.write_u32(value)
    for key in DETAIL_SCALARS[3:]:
        w.write_u32(_as_int(values.get(key, 0), key))
    plants = doc.get("potted_plants") or []
    if not isinstance(plants, list) or len(plants) > MAX_POTTED_PLANTS:
        raise ConvError(f"potted_plants: expected a list of at most {MAX_POTTED_PLANTS}")
    w.write_u32(len(plants))
    for i, plant in enumerate(plants):
        w.write_bytes(write_potted_plant(plant, f"potted_plants[{i}]"))
    earned = set()
    for name in doc.get("achievements") or []:
        if name not in ACHIEVEMENTS:
            raise ConvError(f"achievements: unknown achievement {name!r}")
        earned.add(name)
    for name in ACHIEVEMENTS:
        w.write_u16(1 if name in earned else 0)

    # Older files end before the zombatar tail; keep them that way.
    if doc.get("zombatar_tail_present", True):
        zombatar = doc.get("zombatar") or {}
        heads = zombatar.get("heads") or []
        if not isinstance(heads, list) or len(heads) > MAX_ZOMBATAR_HEADS:
            raise ConvError(f"zombatar.heads: expected a list of at most {MAX_ZOMBATAR_HEADS}")
        w.write_u8(_as_int(zombatar.get("accepted", 0), "zombatar.accepted", 0, 0xFF))
        w.write_u32(len(heads))
        for i, head in enumerate(heads):
            w.write_bytes(write_zombatar_head(head, f"zombatar.heads[{i}]"))
        # The minigame flags are derived from the challenge records on save,
        # exactly like the game does (PlayerInfo::SyncDetails).
        for i in range(MINIGAME_FLAGS_LEN):
            w.write_u8(1 if records[i + 0x0F] > 0 else 0)
        w.write_u8(_as_int(zombatar.get("created_before", 0), "zombatar.created_before", 0, 0xFF))
    if doc.get("trailing"):
        w.write_bytes(_unb64(doc["trailing"], "_trailing"))
    return w.get_bytes()


def _record_index(name) -> int:
    if isinstance(name, int):
        index = name - 1
    elif isinstance(name, str) and name.startswith("_record_"):
        try:
            index = int(name[len("_record_"):])
        except ValueError:
            raise ConvError(f"challenge_records: malformed key {name!r}")
    elif isinstance(name, str):
        matches = [mode for mode, mode_name in GAME_MODE.items() if mode_name == name]
        if not matches:
            raise ConvError(f"challenge_records: unknown game mode {name!r}")
        index = matches[0] - 1
    else:
        raise ConvError(f"challenge_records: malformed key {name!r}")
    if not 0 <= index < 100:
        raise ConvError(f"challenge_records: index of {name!r} out of range")
    return index


def _purchase_index(name) -> int:
    if isinstance(name, int):
        index = name
    elif isinstance(name, str) and name.startswith("_purchase_"):
        try:
            index = int(name[len("_purchase_"):])
        except ValueError:
            raise ConvError(f"purchases: malformed key {name!r}")
    elif isinstance(name, str):
        matches = [i for i, item_name in STORE_ITEM.items() if item_name == name]
        if not matches:
            raise ConvError(f"purchases: unknown store item {name!r}")
        index = matches[0]
    else:
        raise ConvError(f"purchases: malformed key {name!r}")
    if not 0 <= index < 80:
        raise ConvError(f"purchases: index of {name!r} out of range")
    return index


# ============================================================================
# users.dat (profile index, format version 14)
# ============================================================================

def parse_index(data: bytes) -> dict:
    r = BinaryReader(data)
    version = r.read_u32()
    if version != INDEX_FILE_VERSION:
        raise ConvError(f"unsupported profile index version {version} "
                        f"(expected {INDEX_FILE_VERSION}); only the GOTY format is supported")
    profiles = []
    for _ in range(r.read_u16()):
        name = _name_to_yaml(r.read_bytes(r.read_u16()))
        profiles.append({
            "name": name,
            "use_seq": r.read_u32(),
            "id": r.read_u32(),
        })
    trailing = r.read_bytes(r.remaining) if r.remaining else b""
    doc = {"version": version, "profiles": profiles}
    if trailing:
        doc["trailing"] = _b64(trailing)
    return doc


def write_index(doc: dict) -> bytes:
    profiles = doc.get("profiles") or []
    if not isinstance(profiles, list):
        raise ConvError("profiles: expected a list")
    w = BinaryWriter()
    w.write_u32(_as_int(doc.get("version", INDEX_FILE_VERSION), "save.version", 0, 0xFFFFFFFF))
    w.write_u16(len(profiles))
    for i, profile in enumerate(profiles):
        if not isinstance(profile, dict):
            raise ConvError(f"profiles[{i}]: expected a mapping")
        raw_name = _name_from_yaml(profile.get("name", ""), f"profiles[{i}].name")
        if len(raw_name) > 0xFFFF:
            raise ConvError(f"profiles[{i}].name: {len(raw_name)} bytes exceeds the "
                            "65535-byte limit of the on-disk format")
        w.write_u16(len(raw_name))
        w.write_bytes(raw_name)
        w.write_u32(_as_int(profile.get("use_seq", 0), f"profiles[{i}].use_seq"))
        w.write_u32(_as_int(profile.get("id", 0), f"profiles[{i}].id"))
    if doc.get("trailing"):
        w.write_bytes(_unb64(doc["trailing"], "_trailing"))
    return w.get_bytes()


# ============================================================================
# File detection
# ============================================================================

def parse_user_file(data: bytes) -> tuple[str, dict]:
    """Auto-detect the file kind by its version field."""
    if len(data) < 4:
        raise ConvError("file is too small; not a user data file")
    version = struct.unpack("<I", data[:4])[0]
    if version == INDEX_FILE_VERSION:
        return "index", parse_index(data)
    if version == USER_FILE_VERSION:
        return "details", parse_details(data)
    raise ConvError(
        f"not a recognized user data file (version field is {version}, expected "
        f"{USER_FILE_VERSION} for user<N>.dat or {INDEX_FILE_VERSION} for users.dat). "
        "Only the GOTY formats are supported. Back up the file before letting the "
        "game touch it - the game overwrites files it cannot read.")


# ============================================================================
# YAML export / import
# ============================================================================

DETAIL_TOP_KEYS = ["level", "coins", "finished_adventure", "play_time_active",
                   "play_time_inactive", "last_stinky_chocolate_time",
                   "stinky_pos_x", "stinky_pos_y"]
DETAIL_FLAG_KEYS = ["has_used_cheat_keys", "has_woken_stinky",
                    "didnt_purchase_packet_upgrade", "has_unlocked_minigames",
                    "has_unlocked_puzzle_mode", "has_new_mini_game",
                    "has_new_scary_potter", "has_new_i_zombie", "has_new_survival",
                    "has_unlocked_survival_mode", "needs_message_on_game_selector",
                    "needs_magic_taco_reward", "has_seen_stinky", "has_seen_upsell"]


def export_details_yaml(doc: dict) -> dict:
    values = doc["values"]
    out = {"save": {"format": "PVZP_USER_DETAILS", "version": doc["version"],
                    "yaml_layout": YAML_LAYOUT}}
    for key in DETAIL_TOP_KEYS:
        out[key] = values[key]
    out["flags"] = {key: values[key] for key in DETAIL_FLAG_KEYS}
    if doc["challenge_records"]:
        out["challenge_records"] = doc["challenge_records"]
    if doc["purchases"]:
        out["purchases"] = doc["purchases"]
    if doc["potted_plants"]:
        out["potted_plants"] = doc["potted_plants"]
    if doc["achievements"]:
        out["achievements"] = doc["achievements"]
    zombatar = doc["zombatar"]
    if zombatar["accepted"] or zombatar["created_before"] or zombatar["heads"]:
        out["zombatar"] = zombatar
    internal = {"placeholder_player_stats": values["placeholder_player_stats"]}
    if not doc.get("zombatar_tail_present", True):
        internal["zombatar_tail_present"] = False
    if doc.get("trailing"):
        internal["trailing"] = doc["trailing"]
    out["_internal"] = internal
    return out


def export_index_yaml(doc: dict) -> dict:
    out = {"save": {"format": "PVZP_USER_INDEX", "version": doc["version"],
                    "yaml_layout": YAML_LAYOUT},
           "profiles": doc["profiles"]}
    if doc.get("trailing"):
        out["_internal"] = {"trailing": doc["trailing"]}
    return out


def _warn_unknown_keys(mapping: dict, known: set, path: str, warnings: list):
    if not isinstance(mapping, dict):
        return
    for key in mapping:
        if key not in known and not str(key).startswith("_"):
            warnings.append(f"{path}: unknown key {key!r} (ignored)")


def import_details_yaml(data: dict, warnings: list) -> dict:
    known = {"save", "flags", "challenge_records", "purchases", "potted_plants",
             "achievements", "zombatar", "_internal"} | set(DETAIL_TOP_KEYS)
    _warn_unknown_keys(data, known, "(top level)", warnings)
    internal = data.get("_internal") or {}
    _warn_unknown_keys(internal, {"placeholder_player_stats", "trailing",
                                  "zombatar_tail_present"}, "_internal", warnings)
    values = {key: data.get(key, 0 if key != "level" else 1) for key in DETAIL_TOP_KEYS}
    flags = data.get("flags") or {}
    _warn_unknown_keys(flags, set(DETAIL_FLAG_KEYS), "flags", warnings)
    for key in DETAIL_FLAG_KEYS:
        values[key] = _as_int(flags.get(key, 0), f"flags.{key}")
    values["placeholder_player_stats"] = _as_int(
        internal.get("placeholder_player_stats", 0), "_internal.placeholder_player_stats")
    doc = {
        "version": data["save"].get("version", USER_FILE_VERSION),
        "values": values,
        "challenge_records": data.get("challenge_records") or {},
        "purchases": data.get("purchases") or {},
        "potted_plants": data.get("potted_plants") or [],
        "achievements": data.get("achievements") or [],
        "zombatar": data.get("zombatar") or {},
        "zombatar_tail_present": bool(internal.get("zombatar_tail_present", True)),
    }
    if internal.get("trailing"):
        doc["trailing"] = internal["trailing"]
    return doc


def import_index_yaml(data: dict, warnings: list) -> dict:
    _warn_unknown_keys(data, {"save", "profiles", "_internal"}, "(top level)", warnings)
    internal = data.get("_internal") or {}
    doc = {"version": data["save"].get("version", INDEX_FILE_VERSION),
           "profiles": data.get("profiles") or []}
    if internal.get("trailing"):
        doc["trailing"] = internal["trailing"]
    return doc


def dump_yaml(output: dict) -> str:
    return yaml.dump(output, Dumper=yaml.SafeDumper, allow_unicode=True,
                     sort_keys=False, default_flow_style=False, width=100)


# ============================================================================
# info command
# ============================================================================

def _fmt_time(timestamp: int) -> str:
    try:
        return time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(timestamp))
    except (OSError, OverflowError, ValueError):
        return str(timestamp)


def _fmt_duration(seconds: int) -> str:
    hours, rem = divmod(seconds, 3600)
    minutes, secs = divmod(rem, 60)
    return f"{hours}h{minutes:02d}m" if hours else f"{minutes}m{secs:02d}s"


# Consumable store items store PURCHASE_COUNT_OFFSET (1000) + amount in stock;
# potted marigolds store the purchase day as days since 2000-01-01 (one per
# day); Stinky stores the purchase time as a unix timestamp.
_CONSUMABLE_ITEMS = {"STORE_ITEM_FERTILIZER", "STORE_ITEM_BUG_SPRAY",
                     "STORE_ITEM_CHOCOLATE", "STORE_ITEM_TREE_FOOD"}
_MARIGOLD_ITEMS = {"STORE_ITEM_POTTED_MARIGOLD_1", "STORE_ITEM_POTTED_MARIGOLD_2",
                   "STORE_ITEM_POTTED_MARIGOLD_3"}


def _purchase_note(name: str, value: int) -> str:
    if name in _CONSUMABLE_ITEMS and value >= 1000:
        return f" ({value - 1000} in stock)"
    if name in _MARIGOLD_ITEMS:
        try:
            day = datetime.date(2000, 1, 1) + datetime.timedelta(days=value)
            return f" (purchased on {day.isoformat()})"
        except OverflowError:
            return " (purchase day)"
    if name == "STORE_ITEM_STINKY_THE_SNAIL" and value:
        return f" (owned since {_fmt_time(value)})"
    return ""


def print_info(kind: str, doc: dict, file_size: int):
    print("=" * 60)
    if kind == "index":
        print("PvZ Profile Index (users.dat, GOTY format v%d)" % doc["version"])
        print("=" * 60)
        print(f"  File size: {file_size} bytes, {len(doc['profiles'])} profile(s)")
        for profile in doc["profiles"]:
            name = profile["name"]
            if isinstance(name, dict):
                name = "<non-UTF-8 name>"
            print(f"  [{profile['id']}] {name}  (use_seq: {profile['use_seq']})")
        return

    values = doc["values"]
    print("PvZ User Profile (GOTY format v%d)" % doc["version"])
    print("=" * 60)
    print(f"  File size: {file_size} bytes")
    print(f"  Adventure level: {values['level']}  "
          f"(finished: {values['finished_adventure']} time(s))")
    print(f"  Coins: ${values['coins'] * 10:,} (stored: {values['coins']})")
    print(f"  Play time: {_fmt_duration(values['play_time_active'])} active, "
          f"{_fmt_duration(values['play_time_inactive'])} idle")
    if values["finished_adventure"]:
        print("  Unlocked: all modes (adventure completed)")
    else:
        unlocked = [key for key in ("has_unlocked_minigames", "has_unlocked_puzzle_mode",
                                    "has_unlocked_survival_mode") if values[key]]
        print(f"  Unlocked: {', '.join(unlocked) if unlocked else '(adventure only)'}")
    if values["has_used_cheat_keys"]:
        print("  Cheat keys used: yes")

    if doc["challenge_records"]:
        print("\n[Challenge Records]")
        for name, value in sorted(doc["challenge_records"].items(),
                                  key=lambda kv: -kv[1]):
            print(f"  {name}: {value}")

    if doc["purchases"]:
        print("\n[Purchases]")
        for name, value in doc["purchases"].items():
            print(f"  {name}: {value}{_purchase_note(name, value)}")

    plants = doc["potted_plants"]
    if plants:
        counts = {}
        for plant in plants:
            garden = plant.get("which_zen_garden", "?")
            counts[garden] = counts.get(garden, 0) + 1
        print(f"\n[Zen Garden] {len(plants)} plant(s)")
        print("  " + ", ".join(f"{garden}: {count}" for garden, count in counts.items()))
        if values["has_woken_stinky"] or values["last_stinky_chocolate_time"]:
            print(f"  Stinky: at ({values['stinky_pos_x']}, {values['stinky_pos_y']}), "
                  f"last chocolate {_fmt_time(values['last_stinky_chocolate_time'])}")

    earned = doc["achievements"]
    print(f"\n[Achievements] {len(earned)} / {len(ACHIEVEMENTS)}")
    if earned:
        print("  " + ", ".join(earned))

    zombatar = doc["zombatar"]
    if zombatar["heads"] or zombatar["accepted"]:
        print(f"\n[Zombatar] {len(zombatar['heads'])} head(s), "
              f"TOS accepted: {bool(zombatar['accepted'])}")


# ============================================================================
# CLI
# ============================================================================

def _read_file(path: str) -> bytes:
    try:
        with open(path, "rb") as f:
            return f.read()
    except OSError as e:
        raise ConvError(f"cannot read {path}: {e.strerror or e}")


def _fsync_directory(directory: str):
    """Best-effort fsync of a directory so that a rename into it is durable."""
    if os.name != "posix":
        return
    try:
        fd = os.open(directory, os.O_RDONLY)
        try:
            os.fsync(fd)
        finally:
            os.close(fd)
    except OSError:
        pass


def _write_file(path: str, data: bytes):
    """Write data to path atomically.

    The data goes to a temp file in the same directory (hence the same
    filesystem), is fsynced, and is then moved onto path with os.replace(),
    so a crash can never leave a half-written file at path. The permission
    bits of an existing file are preserved. On failure the temp file is
    removed and path is left untouched.
    """
    directory = os.path.dirname(os.path.abspath(path))
    tmp = None
    try:
        fd, tmp = tempfile.mkstemp(dir=directory,
                                   prefix=os.path.basename(path) + ".",
                                   suffix=".tmp")
        with os.fdopen(fd, "wb") as f:
            f.write(data)
            f.flush()
            os.fsync(f.fileno())
        try:
            shutil.copymode(path, tmp)
        except OSError:
            pass  # path does not exist yet; keep the temp file's default mode
        os.replace(tmp, path)
        _fsync_directory(directory)
    except OSError as e:
        if tmp is not None:
            try:
                os.unlink(tmp)
            except OSError:
                pass
        raise ConvError(f"cannot write {path}: {e.strerror or e}")


def cmd_info(args):
    data = _read_file(args.input)
    kind, doc = parse_user_file(data)
    print_info(kind, doc, len(data))


def cmd_export(args):
    kind, doc = parse_user_file(_read_file(args.input))
    out = export_index_yaml(doc) if kind == "index" else export_details_yaml(doc)
    _write_file(args.output, dump_yaml(out).encode("utf-8"))
    print(f"Exported {kind} file to: {args.output}")


def _load_yaml_doc(path: str, warnings: list) -> tuple[str, dict]:
    try:
        with open(path, "r", encoding="utf-8") as f:
            data = yaml.safe_load(f)
    except yaml.YAMLError as e:
        raise ConvError(f"invalid YAML in {path}: {e}")
    except OSError as e:
        raise ConvError(f"cannot read {path}: {e.strerror or e}")
    if not isinstance(data, dict) or not isinstance(data.get("save"), dict):
        raise ConvError("this YAML is not a user data export")
    save_info = data["save"]
    if save_info.get("yaml_layout") != YAML_LAYOUT:
        raise ConvError(f"unsupported YAML layout (expected save.yaml_layout: {YAML_LAYOUT})")
    fmt = save_info.get("format")
    if fmt == "PVZP_USER_DETAILS":
        return "details", import_details_yaml(data, warnings)
    if fmt == "PVZP_USER_INDEX":
        return "index", import_index_yaml(data, warnings)
    raise ConvError(f"unsupported export format {fmt!r}")


def cmd_import(args):
    warnings = []
    kind, doc = _load_yaml_doc(args.input, warnings)
    result = write_index(doc) if kind == "index" else write_details(doc)
    _write_file(args.output, result)
    for warning in warnings:
        print(f"Warning: {warning}", file=sys.stderr)
    print(f"Imported to: {args.output} ({len(result)} bytes)")


SETTABLE_KEYS = DETAIL_TOP_KEYS + [f"flags.{key}" for key in DETAIL_FLAG_KEYS]


def cmd_set(args):
    kind, doc = parse_user_file(_read_file(args.input))
    if kind != "details":
        raise ConvError("set only works on user<N>.dat detail files")
    if args.key not in SETTABLE_KEYS:
        raise ConvError(f"unknown key {args.key!r}; settable keys: {', '.join(SETTABLE_KEYS)}")
    try:
        value = int(args.value, 0)
    except ValueError:
        raise ConvError(f"value must be an integer, got {args.value!r}")
    _as_int(value, args.key)
    if args.key.startswith("flags."):
        doc["values"][args.key[len("flags."):]] = value
    else:
        doc["values"][args.key] = value
    _write_file(args.input, write_details(doc))
    print(f"{args.input}: {args.key} = {value}"
          + (f" (${value * 10:,} in game)" if args.key == "coins" else ""))


def main():
    parser = argparse.ArgumentParser(
        prog="pvzp-user-converter.py",
        description="PvZ-Portable / PvZ GOTY global user data editor: "
                    "lossless users.dat / user<N>.dat <-> YAML conversion.",
        epilog="The file kind is detected automatically from the version field: "
               "user<N>.dat holds one profile (v12) and users.dat holds the profile "
               "index (v14). Only the GOTY formats are supported. The format is "
               "identical to the original game's, so files from the original game "
               "work too. Back up your files first: the game overwrites any file it "
               "cannot read. Note that stored coin values are 1/10 of the dollar "
               "amount shown in game.")
    subparsers = parser.add_subparsers(dest="command", required=True)

    p_info = subparsers.add_parser("info", help="print a summary of a user data file")
    p_info.add_argument("input", help="users.dat or user<N>.dat")
    p_info.set_defaults(func=cmd_info)

    p_export = subparsers.add_parser("export", help="convert a user data file to editable YAML")
    p_export.add_argument("input", help="input .dat file")
    p_export.add_argument("output", help="output .yaml file")
    p_export.set_defaults(func=cmd_export)

    p_import = subparsers.add_parser("import", help="rebuild a user data file from edited YAML")
    p_import.add_argument("input", help="input .yaml file")
    p_import.add_argument("output", help="output .dat file")
    p_import.set_defaults(func=cmd_import)

    p_set = subparsers.add_parser("set", help="change a single value in a user<N>.dat file")
    p_set.add_argument("input", help="user<N>.dat file (modified in place)")
    p_set.add_argument("key", help="one of: " + ", ".join(SETTABLE_KEYS))
    p_set.add_argument("value", help="integer value")
    p_set.set_defaults(func=cmd_set)

    args = parser.parse_args()
    try:
        args.func(args)
    except ConvError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
