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

#pragma once

#include "ConstEnums.h"
constexpr const double PI = 3.141592653589793;

// ============================================================
// Constants
// ============================================================
constexpr const int BOARD_WIDTH = 800;
constexpr const int BOARD_HEIGHT = 600;
constexpr const int WIDE_BOARD_WIDTH = 800;
constexpr const int BOARD_OFFSET = 220;
constexpr const int BOARD_EDGE = -100;
constexpr const int BOARD_IMAGE_WIDTH_OFFSET = 1180;
constexpr const int BOARD_ICE_START = 800;
constexpr const int LAWN_XMIN = 40;
constexpr const int LAWN_YMIN = 80;
constexpr const int HIGH_GROUND_HEIGHT = 30;

constexpr const int SEEDBANK_MAX = 10;
constexpr const int SEED_BANK_OFFSET_X = 0;
constexpr const int SEED_BANK_OFFSET_X_END = 10;
constexpr const int SEED_CHOOSER_OFFSET_Y = 516;
constexpr const int SEED_PACKET_WIDTH = 50;
constexpr const int SEED_PACKET_HEIGHT = 70;

// ============================================================
// About levels
// ============================================================
constexpr const int ADVENTURE_AREAS = 5;
constexpr const int LEVELS_PER_AREA = 10;
constexpr const int NUM_LEVELS = ADVENTURE_AREAS * LEVELS_PER_AREA;
constexpr const int FINAL_LEVEL = NUM_LEVELS;
constexpr const int LAST_STAND_FLAGS = 5;
constexpr const int SURVIVAL_NORMAL_FLAGS = 5;
constexpr const int SURVIVAL_HARD_FLAGS = 10;
