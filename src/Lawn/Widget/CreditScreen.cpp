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

#include "GameButton.h"
#include "CreditScreen.h"
#include "../../LawnApp.h"
#include "../../Resources.h"
#include "../System/Music.h"
#include "../../GameConstants.h"
#include "../System/PoolEffect.h"
#include "../../Sexy.TodLib/TodFoley.h"
#include "../../Sexy.TodLib/Attachment.h"
#include "../../Sexy.TodLib/Reanimator.h"
#include "../../Sexy.TodLib/TodParticle.h"
#include "widget/Dialog.h"
#include "../../Sexy.TodLib/EffectSystem.h"
#include "../../Sexy.TodLib/TodStringFile.h"
#include "graphics/Font.h"

static constexpr float CREDIT_SCREEN_ANIM_RATE = 0.3f;

static CreditsTiming gCreditsTiming[] = {
    {  128.5f,      CreditWordType::WORD_AW ,         0,     CreditBrainType::BRAIN_OFF       },
    {  133.0f,      CreditWordType::WORD_OH ,         0,     CreditBrainType::BRAIN_OFF       },
    {  136.5f,      CreditWordType::WORD_EE ,         0,     CreditBrainType::BRAIN_OFF       },
    {  140.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_OFF       },
    {  141.0f,      CreditWordType::WORD_AW ,       214,     CreditBrainType::BRAIN_NEXT_WORD },
    {  143.0f,      CreditWordType::WORD_AW ,       297,     CreditBrainType::BRAIN_NEXT_WORD },
    {  145.0f,      CreditWordType::WORD_AW ,       348,     CreditBrainType::BRAIN_NEXT_WORD },
    {  149.0f,      CreditWordType::WORD_EE ,       400,     CreditBrainType::BRAIN_NEXT_WORD },
    {  153.0f,      CreditWordType::WORD_AW ,       455,     CreditBrainType::BRAIN_NEXT_WORD },
    {  155.0f,      CreditWordType::WORD_OH ,       523,     CreditBrainType::BRAIN_NEXT_WORD },
    {  159.0f,      CreditWordType::WORD_AW ,       593,     CreditBrainType::BRAIN_NEXT_WORD },
    {  163.0f,      CreditWordType::WORD_AW ,       619,     CreditBrainType::BRAIN_NEXT_WORD },
    {  171.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FLY_OFF   },
    {  172.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_OFF       },
    {  173.0f,      CreditWordType::WORD_AW ,       214,     CreditBrainType::BRAIN_FAST_ON   },
    {  175.0f,      CreditWordType::WORD_AW ,       297,     CreditBrainType::BRAIN_NEXT_WORD },
    {  177.0f,      CreditWordType::WORD_AW ,       348,     CreditBrainType::BRAIN_NEXT_WORD },
    {  181.0f,      CreditWordType::WORD_EE ,       400,     CreditBrainType::BRAIN_NEXT_WORD },
    {  185.0f,      CreditWordType::WORD_AW ,       455,     CreditBrainType::BRAIN_NEXT_WORD },
    {  187.0f,      CreditWordType::WORD_OH ,       523,     CreditBrainType::BRAIN_NEXT_WORD },
    {  191.0f,      CreditWordType::WORD_AW ,       593,     CreditBrainType::BRAIN_NEXT_WORD },
    {  193.0f,      CreditWordType::WORD_AW ,       619,     CreditBrainType::BRAIN_NEXT_WORD },
    {  199.0f,      CreditWordType::WORD_AA ,         0,     CreditBrainType::BRAIN_FLY_OFF   },
    {  203.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_OFF       },
    {  205.0f,      CreditWordType::WORD_AW ,       214,     CreditBrainType::BRAIN_FLY_ON    },
    {  207.0f,      CreditWordType::WORD_AW ,       297,     CreditBrainType::BRAIN_NEXT_WORD },
    {  209.0f,      CreditWordType::WORD_AW ,       348,     CreditBrainType::BRAIN_NEXT_WORD },
    {  213.0f,      CreditWordType::WORD_EE ,       400,     CreditBrainType::BRAIN_NEXT_WORD },
    {  217.0f,      CreditWordType::WORD_AW ,       455,     CreditBrainType::BRAIN_NEXT_WORD },
    {  219.0f,      CreditWordType::WORD_OH ,       523,     CreditBrainType::BRAIN_NEXT_WORD },
    {  223.0f,      CreditWordType::WORD_AW ,       593,     CreditBrainType::BRAIN_NEXT_WORD },
    {  227.0f,      CreditWordType::WORD_AW ,       619,     CreditBrainType::BRAIN_NEXT_WORD },
    {  231.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FLY_OFF   },
    {  234.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_OFF       },
    {  235.0f,      CreditWordType::WORD_EE ,       150,     CreditBrainType::BRAIN_FAST_ON   },
    {  237.0f,      CreditWordType::WORD_OH ,       220,     CreditBrainType::BRAIN_NEXT_WORD },
    {  239.0f,      CreditWordType::WORD_AW ,       307,     CreditBrainType::BRAIN_NEXT_WORD },
    {  241.0f,      CreditWordType::WORD_AW ,       390,     CreditBrainType::BRAIN_NEXT_WORD },
    {  245.0f,      CreditWordType::WORD_EE ,       452,     CreditBrainType::BRAIN_NEXT_WORD },
    {  249.0f,      CreditWordType::WORD_AW ,       512,     CreditBrainType::BRAIN_NEXT_WORD },
    {  251.0f,      CreditWordType::WORD_AW ,       573,     CreditBrainType::BRAIN_NEXT_WORD },
    {  255.0f,      CreditWordType::WORD_AW ,       630,     CreditBrainType::BRAIN_NEXT_WORD },
    {  257.0f,      CreditWordType::WORD_AW ,       656,     CreditBrainType::BRAIN_NEXT_WORD },
    {  261.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FLY_OFF   },
    {  262.0f,      CreditWordType::WORD_AA ,         0,     CreditBrainType::BRAIN_OFF       },
    {  266.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_OFF       },
    {  266.5f,      CreditWordType::WORD_AW ,        96,     CreditBrainType::BRAIN_FAST_ON   },
    {  268.5f,      CreditWordType::WORD_OH ,       154,     CreditBrainType::BRAIN_NEXT_WORD },
    {  270.5f,      CreditWordType::WORD_OH ,       244,     CreditBrainType::BRAIN_NEXT_WORD },
    {  272.5f,      CreditWordType::WORD_AW ,       329,     CreditBrainType::BRAIN_NEXT_WORD },
    {  276.5f,      CreditWordType::WORD_AW ,       419,     CreditBrainType::BRAIN_NEXT_WORD },
    {  279.5f,      CreditWordType::WORD_AW ,       506,     CreditBrainType::BRAIN_NEXT_WORD },
    {  281.5f,      CreditWordType::WORD_AW ,       597,     CreditBrainType::BRAIN_NEXT_WORD },
    {  284.5f,      CreditWordType::WORD_AW ,       671,     CreditBrainType::BRAIN_NEXT_WORD },
    {  286.5f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FLY_OFF   },
    {  287.0f,      CreditWordType::WORD_OH ,        48,     CreditBrainType::BRAIN_FAST_ON   },
    {  288.0f,      CreditWordType::WORD_AW ,       125,     CreditBrainType::BRAIN_NEXT_WORD },
    {  290.0f,      CreditWordType::WORD_OH ,       193,     CreditBrainType::BRAIN_NEXT_WORD },
    {  291.0f,      CreditWordType::WORD_EE ,       254,     CreditBrainType::BRAIN_NEXT_WORD },
    {  294.5f,      CreditWordType::WORD_AW ,       318,     CreditBrainType::BRAIN_NEXT_WORD },
    {  295.0f,      CreditWordType::WORD_AW ,       375,     CreditBrainType::BRAIN_NEXT_WORD },
    {  296.0f,      CreditWordType::WORD_AW ,       438,     CreditBrainType::BRAIN_NEXT_WORD },
    {  297.0f,      CreditWordType::WORD_AW ,       480,     CreditBrainType::BRAIN_NEXT_WORD },
    {  299.0f,      CreditWordType::WORD_AW ,       556,     CreditBrainType::BRAIN_NEXT_WORD },
    {  301.0f,      CreditWordType::WORD_AW ,       619,     CreditBrainType::BRAIN_NEXT_WORD },
    {  303.0f,      CreditWordType::WORD_AW ,       675,     CreditBrainType::BRAIN_NEXT_WORD },
    {  305.0f,      CreditWordType::WORD_AW ,       744,     CreditBrainType::BRAIN_NEXT_WORD },
    {  307.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FLY_OFF   },
    {  309.5f,      CreditWordType::WORD_OFF,       207,     CreditBrainType::BRAIN_FLY_ON    },
    {  310.5f,      CreditWordType::WORD_OFF,       287,     CreditBrainType::BRAIN_NEXT_WORD },
    {  311.5f,      CreditWordType::WORD_OFF,       365,     CreditBrainType::BRAIN_NEXT_WORD },
    {  313.5f,      CreditWordType::WORD_OFF,       435,     CreditBrainType::BRAIN_NEXT_WORD },
    {  315.5f,      CreditWordType::WORD_OFF,       518,     CreditBrainType::BRAIN_NEXT_WORD },
    {  317.5f,      CreditWordType::WORD_OFF,       603,     CreditBrainType::BRAIN_NEXT_WORD },
    {  318.5f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FAST_OFF  },
    {  319.5f,      CreditWordType::WORD_OFF,       198,     CreditBrainType::BRAIN_FAST_ON   },
    {  320.5f,      CreditWordType::WORD_OFF,       264,     CreditBrainType::BRAIN_NEXT_WORD },
    {  322.5f,      CreditWordType::WORD_OFF,       335,     CreditBrainType::BRAIN_NEXT_WORD },
    {  323.5f,      CreditWordType::WORD_OFF,       411,     CreditBrainType::BRAIN_NEXT_WORD },
    {  324.5f,      CreditWordType::WORD_OFF,       474,     CreditBrainType::BRAIN_NEXT_WORD },
    {  326.5f,      CreditWordType::WORD_OFF,       527,     CreditBrainType::BRAIN_NEXT_WORD },
    {  328.5f,      CreditWordType::WORD_OFF,       595,     CreditBrainType::BRAIN_NEXT_WORD },
    {  332.5f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FLY_OFF   },
    {  337.5f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_OFF       },
    {  339.5f,      CreditWordType::WORD_AW ,       190,     CreditBrainType::BRAIN_FLY_ON    },
    {  340.5f,      CreditWordType::WORD_AW ,       260,     CreditBrainType::BRAIN_NEXT_WORD },
    {  342.5f,      CreditWordType::WORD_AA ,       314,     CreditBrainType::BRAIN_NEXT_WORD },
    {  344.5f,      CreditWordType::WORD_AW ,       364,     CreditBrainType::BRAIN_NEXT_WORD },
    {  347.5f,      CreditWordType::WORD_AW ,       426,     CreditBrainType::BRAIN_NEXT_WORD },
    {  349.5f,      CreditWordType::WORD_OH ,       474,     CreditBrainType::BRAIN_NEXT_WORD },
    {  350.5f,      CreditWordType::WORD_AW ,       538,     CreditBrainType::BRAIN_NEXT_WORD },
    {  352.5f,      CreditWordType::WORD_EE ,       606,     CreditBrainType::BRAIN_NEXT_WORD },
    {  353.5f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FAST_OFF  },
    {  354.5f,      CreditWordType::WORD_EE ,       187,     CreditBrainType::BRAIN_FAST_ON   },
    {  356.5f,      CreditWordType::WORD_AW ,       242,     CreditBrainType::BRAIN_NEXT_WORD },
    {  358.5f,      CreditWordType::WORD_OH ,       280,     CreditBrainType::BRAIN_NEXT_WORD },
    {  359.5f,      CreditWordType::WORD_AW ,       340,     CreditBrainType::BRAIN_NEXT_WORD },
    {  360.5f,      CreditWordType::WORD_AW ,       394,     CreditBrainType::BRAIN_NEXT_WORD },
    {  361.5f,      CreditWordType::WORD_EE ,       439,     CreditBrainType::BRAIN_NEXT_WORD },
    {  363.5f,      CreditWordType::WORD_AW ,       500,     CreditBrainType::BRAIN_NEXT_WORD },
    {  364.5f,      CreditWordType::WORD_AW ,       550,     CreditBrainType::BRAIN_NEXT_WORD },
    {  366.5f,      CreditWordType::WORD_EE ,       606,     CreditBrainType::BRAIN_NEXT_WORD },
    {  369.5f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FLY_OFF   },
    {  371.5f,      CreditWordType::WORD_OFF,       200,     CreditBrainType::BRAIN_FLY_ON    },
    {  372.5f,      CreditWordType::WORD_OH ,       258,     CreditBrainType::BRAIN_NEXT_WORD },
    {  374.5f,      CreditWordType::WORD_OFF,       332,     CreditBrainType::BRAIN_NEXT_WORD },
    {  376.5f,      CreditWordType::WORD_OFF,       416,     CreditBrainType::BRAIN_NEXT_WORD },
    {  378.5f,      CreditWordType::WORD_OFF,       494,     CreditBrainType::BRAIN_NEXT_WORD },
    {  380.5f,      CreditWordType::WORD_OFF,       576,     CreditBrainType::BRAIN_NEXT_WORD },
    {  381.5f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FAST_OFF  },
    {  382.5f,      CreditWordType::WORD_OFF,       255,     CreditBrainType::BRAIN_FAST_ON   },
    {  384.5f,      CreditWordType::WORD_OFF,       322,     CreditBrainType::BRAIN_NEXT_WORD },
    {  386.5f,      CreditWordType::WORD_OFF,       400,     CreditBrainType::BRAIN_NEXT_WORD },
    {  388.5f,      CreditWordType::WORD_OFF,       474,     CreditBrainType::BRAIN_NEXT_WORD },
    {  390.5f,      CreditWordType::WORD_OFF,       533,     CreditBrainType::BRAIN_NEXT_WORD },
    {  394.5f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FLY_OFF   },
    {  522.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_OFF       },
    {  523.0f,      CreditWordType::WORD_AW ,       214,     CreditBrainType::BRAIN_FAST_ON   },
    {  525.0f,      CreditWordType::WORD_AW ,       297,     CreditBrainType::BRAIN_NEXT_WORD },
    {  527.0f,      CreditWordType::WORD_AW ,       348,     CreditBrainType::BRAIN_NEXT_WORD },
    {  531.0f,      CreditWordType::WORD_EE ,       400,     CreditBrainType::BRAIN_NEXT_WORD },
    {  535.0f,      CreditWordType::WORD_AW ,       455,     CreditBrainType::BRAIN_NEXT_WORD },
    {  537.0f,      CreditWordType::WORD_OH ,       523,     CreditBrainType::BRAIN_NEXT_WORD },
    {  541.0f,      CreditWordType::WORD_AW ,       593,     CreditBrainType::BRAIN_NEXT_WORD },
    {  545.0f,      CreditWordType::WORD_AW ,       619,     CreditBrainType::BRAIN_NEXT_WORD },
    {  549.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FLY_OFF   },
    {  554.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_OFF       },
    {  555.0f,      CreditWordType::WORD_AW ,       214,     CreditBrainType::BRAIN_FAST_ON   },
    {  557.0f,      CreditWordType::WORD_AW ,       297,     CreditBrainType::BRAIN_NEXT_WORD },
    {  559.0f,      CreditWordType::WORD_AW ,       348,     CreditBrainType::BRAIN_NEXT_WORD },
    {  563.0f,      CreditWordType::WORD_EE ,       400,     CreditBrainType::BRAIN_NEXT_WORD },
    {  567.0f,      CreditWordType::WORD_AW ,       455,     CreditBrainType::BRAIN_NEXT_WORD },
    {  569.0f,      CreditWordType::WORD_OH ,       523,     CreditBrainType::BRAIN_NEXT_WORD },
    {  573.0f,      CreditWordType::WORD_AW ,       593,     CreditBrainType::BRAIN_NEXT_WORD },
    {  575.0f,      CreditWordType::WORD_AW ,       619,     CreditBrainType::BRAIN_NEXT_WORD },
    {  581.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FLY_OFF   },
    {  582.0f,      CreditWordType::WORD_AA ,         0,     CreditBrainType::BRAIN_OFF       },
    {  586.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_OFF       },
    {  587.0f,      CreditWordType::WORD_AW ,       214,     CreditBrainType::BRAIN_FAST_ON   },
    {  589.0f,      CreditWordType::WORD_AW ,       297,     CreditBrainType::BRAIN_NEXT_WORD },
    {  591.0f,      CreditWordType::WORD_AW ,       348,     CreditBrainType::BRAIN_NEXT_WORD },
    {  595.0f,      CreditWordType::WORD_EE ,       400,     CreditBrainType::BRAIN_NEXT_WORD },
    {  599.0f,      CreditWordType::WORD_AW ,       455,     CreditBrainType::BRAIN_NEXT_WORD },
    {  601.0f,      CreditWordType::WORD_OH ,       523,     CreditBrainType::BRAIN_NEXT_WORD },
    {  605.0f,      CreditWordType::WORD_AW ,       593,     CreditBrainType::BRAIN_NEXT_WORD },
    {  609.0f,      CreditWordType::WORD_AW ,       619,     CreditBrainType::BRAIN_NEXT_WORD },
    {  613.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FLY_OFF   },
    {  616.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_OFF       },
    {  617.0f,      CreditWordType::WORD_EE ,       150,     CreditBrainType::BRAIN_FAST_ON   },
    {  619.0f,      CreditWordType::WORD_OH ,       220,     CreditBrainType::BRAIN_NEXT_WORD },
    {  621.0f,      CreditWordType::WORD_AW ,       307,     CreditBrainType::BRAIN_NEXT_WORD },
    {  623.0f,      CreditWordType::WORD_AW ,       390,     CreditBrainType::BRAIN_NEXT_WORD },
    {  627.0f,      CreditWordType::WORD_EE ,       452,     CreditBrainType::BRAIN_NEXT_WORD },
    {  631.0f,      CreditWordType::WORD_AW ,       512,     CreditBrainType::BRAIN_NEXT_WORD },
    {  633.0f,      CreditWordType::WORD_AW ,       573,     CreditBrainType::BRAIN_NEXT_WORD },
    {  637.0f,      CreditWordType::WORD_AW ,       630,     CreditBrainType::BRAIN_NEXT_WORD },
    {  639.0f,      CreditWordType::WORD_AW ,       656,     CreditBrainType::BRAIN_NEXT_WORD },
    {  643.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FLY_OFF   },
    {  644.0f,      CreditWordType::WORD_AA ,         0,     CreditBrainType::BRAIN_OFF       },
    {  648.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_OFF       },
    {  649.0f,      CreditWordType::WORD_AA ,       196,     CreditBrainType::BRAIN_FAST_ON   },
    {  651.0f,      CreditWordType::WORD_EE ,       247,     CreditBrainType::BRAIN_NEXT_WORD },
    {  653.0f,      CreditWordType::WORD_AW ,       299,     CreditBrainType::BRAIN_NEXT_WORD },
    {  655.0f,      CreditWordType::WORD_AW ,       371,     CreditBrainType::BRAIN_NEXT_WORD },
    {  658.0f,      CreditWordType::WORD_OH ,       443,     CreditBrainType::BRAIN_NEXT_WORD },
    {  659.0f,      CreditWordType::WORD_EE ,       475,     CreditBrainType::BRAIN_NEXT_WORD },
    {  661.0f,      CreditWordType::WORD_AW ,       512,     CreditBrainType::BRAIN_NEXT_WORD },
    {  662.0f,      CreditWordType::WORD_AA ,       544,     CreditBrainType::BRAIN_NEXT_WORD },
    {  664.0f,      CreditWordType::WORD_OH ,       573,     CreditBrainType::BRAIN_NEXT_WORD },
    {  667.0f,      CreditWordType::WORD_AA ,       610,     CreditBrainType::BRAIN_NEXT_WORD },
    {  669.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FAST_OFF  },
    {  670.0f,      CreditWordType::WORD_OFF,        48,     CreditBrainType::BRAIN_FAST_ON   },
    {  671.0f,      CreditWordType::WORD_OFF,       110,     CreditBrainType::BRAIN_NEXT_WORD },
    {  673.0f,      CreditWordType::WORD_OFF,       185,     CreditBrainType::BRAIN_NEXT_WORD },
    {  674.0f,      CreditWordType::WORD_OFF,       262,     CreditBrainType::BRAIN_NEXT_WORD },
    {  676.0f,      CreditWordType::WORD_OFF,       317,     CreditBrainType::BRAIN_NEXT_WORD },
    {  677.0f,      CreditWordType::WORD_OFF,       357,     CreditBrainType::BRAIN_NEXT_WORD },
    {  678.0f,      CreditWordType::WORD_OFF,       417,     CreditBrainType::BRAIN_NEXT_WORD },
    {  679.0f,      CreditWordType::WORD_OFF,       491,     CreditBrainType::BRAIN_NEXT_WORD },
    {  682.0f,      CreditWordType::WORD_OFF,       558,     CreditBrainType::BRAIN_NEXT_WORD },
    {  685.0f,      CreditWordType::WORD_OFF,       628,     CreditBrainType::BRAIN_NEXT_WORD },
    {  687.0f,      CreditWordType::WORD_OFF,       720,     CreditBrainType::BRAIN_NEXT_WORD },
    {  689.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FLY_OFF   },
    {  690.0f,      CreditWordType::WORD_OFF,       172,     CreditBrainType::BRAIN_FAST_ON   },
    {  692.0f,      CreditWordType::WORD_OFF,       263,     CreditBrainType::BRAIN_NEXT_WORD },
    {  694.0f,      CreditWordType::WORD_OFF,       346,     CreditBrainType::BRAIN_NEXT_WORD },
    {  696.0f,      CreditWordType::WORD_OFF,       423,     CreditBrainType::BRAIN_NEXT_WORD },
    {  698.0f,      CreditWordType::WORD_OFF,       480,     CreditBrainType::BRAIN_NEXT_WORD },
    {  700.0f,      CreditWordType::WORD_OFF,       536,     CreditBrainType::BRAIN_NEXT_WORD },
    {  702.0f,      CreditWordType::WORD_OFF,       583,     CreditBrainType::BRAIN_NEXT_WORD },
    {  705.0f,      CreditWordType::WORD_OFF,       633,     CreditBrainType::BRAIN_NEXT_WORD },
    {  708.0f,      CreditWordType::WORD_OFF,       668,     CreditBrainType::BRAIN_NEXT_WORD },
    {  712.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FLY_OFF   },
    {  719.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_OFF       },
    {  720.0f,      CreditWordType::WORD_OFF,       182,     CreditBrainType::BRAIN_FAST_ON   },
    {  722.0f,      CreditWordType::WORD_OFF,       267,     CreditBrainType::BRAIN_NEXT_WORD },
    {  724.0f,      CreditWordType::WORD_OFF,       331,     CreditBrainType::BRAIN_NEXT_WORD },
    {  726.0f,      CreditWordType::WORD_OFF,       371,     CreditBrainType::BRAIN_NEXT_WORD },
    {  729.0f,      CreditWordType::WORD_OFF,       434,     CreditBrainType::BRAIN_NEXT_WORD },
    {  731.0f,      CreditWordType::WORD_OFF,       486,     CreditBrainType::BRAIN_NEXT_WORD },
    {  732.0f,      CreditWordType::WORD_OFF,       562,     CreditBrainType::BRAIN_NEXT_WORD },
    {  734.0f,      CreditWordType::WORD_OFF,       617,     CreditBrainType::BRAIN_NEXT_WORD },
    {  735.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FAST_OFF  },
    {  736.0f,      CreditWordType::WORD_AW ,       148,     CreditBrainType::BRAIN_FAST_ON   },
    {  738.0f,      CreditWordType::WORD_AW ,       211,     CreditBrainType::BRAIN_NEXT_WORD },
    {  740.0f,      CreditWordType::WORD_EE ,       298,     CreditBrainType::BRAIN_NEXT_WORD },
    {  742.0f,      CreditWordType::WORD_OH ,       367,     CreditBrainType::BRAIN_NEXT_WORD },
    {  744.0f,      CreditWordType::WORD_AW ,       440,     CreditBrainType::BRAIN_NEXT_WORD },
    {  746.0f,      CreditWordType::WORD_OH ,       506,     CreditBrainType::BRAIN_NEXT_WORD },
    {  747.0f,      CreditWordType::WORD_AW ,       533,     CreditBrainType::BRAIN_NEXT_WORD },
    {  748.0f,      CreditWordType::WORD_AW ,       601,     CreditBrainType::BRAIN_NEXT_WORD },
    {  749.0f,      CreditWordType::WORD_AW ,       645,     CreditBrainType::BRAIN_NEXT_WORD },
    {  750.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FAST_OFF  },
    {  753.0f,      CreditWordType::WORD_OFF,       123,     CreditBrainType::BRAIN_FLY_ON    },
    {  755.0f,      CreditWordType::WORD_OFF,       195,     CreditBrainType::BRAIN_NEXT_WORD },
    {  757.0f,      CreditWordType::WORD_OFF,       255,     CreditBrainType::BRAIN_NEXT_WORD },
    {  759.0f,      CreditWordType::WORD_OFF,       312,     CreditBrainType::BRAIN_NEXT_WORD },
    {  761.0f,      CreditWordType::WORD_OFF,       378,     CreditBrainType::BRAIN_NEXT_WORD },
    {  763.0f,      CreditWordType::WORD_OFF,       443,     CreditBrainType::BRAIN_NEXT_WORD },
    {  765.0f,      CreditWordType::WORD_OFF,       516,     CreditBrainType::BRAIN_NEXT_WORD },
    {  767.0f,      CreditWordType::WORD_OFF,       563,     CreditBrainType::BRAIN_NEXT_WORD },
    {  770.0f,      CreditWordType::WORD_OFF,       588,     CreditBrainType::BRAIN_NEXT_WORD },
    {  773.0f,      CreditWordType::WORD_OFF,       657,     CreditBrainType::BRAIN_NEXT_WORD },
    {  777.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FLY_OFF   },
    {  907.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_OFF       },
    {  908.0f,      CreditWordType::WORD_AW ,       214,     CreditBrainType::BRAIN_FAST_ON   },
    {  910.0f,      CreditWordType::WORD_AW ,       297,     CreditBrainType::BRAIN_NEXT_WORD },
    {  912.0f,      CreditWordType::WORD_AW ,       348,     CreditBrainType::BRAIN_NEXT_WORD },
    {  916.0f,      CreditWordType::WORD_EE ,       400,     CreditBrainType::BRAIN_NEXT_WORD },
    {  920.0f,      CreditWordType::WORD_AW ,       455,     CreditBrainType::BRAIN_NEXT_WORD },
    {  922.0f,      CreditWordType::WORD_OH ,       523,     CreditBrainType::BRAIN_NEXT_WORD },
    {  926.0f,      CreditWordType::WORD_AW ,       593,     CreditBrainType::BRAIN_NEXT_WORD },
    {  930.0f,      CreditWordType::WORD_AW ,       616,     CreditBrainType::BRAIN_NEXT_WORD },
    {  934.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FLY_OFF   },
    {  939.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_OFF       },
    {  940.0f,      CreditWordType::WORD_AW ,       214,     CreditBrainType::BRAIN_FAST_ON   },
    {  942.0f,      CreditWordType::WORD_AW ,       297,     CreditBrainType::BRAIN_NEXT_WORD },
    {  944.0f,      CreditWordType::WORD_AW ,       348,     CreditBrainType::BRAIN_NEXT_WORD },
    {  948.0f,      CreditWordType::WORD_EE ,       400,     CreditBrainType::BRAIN_NEXT_WORD },
    {  952.0f,      CreditWordType::WORD_AW ,       455,     CreditBrainType::BRAIN_NEXT_WORD },
    {  954.0f,      CreditWordType::WORD_OH ,       523,     CreditBrainType::BRAIN_NEXT_WORD },
    {  958.0f,      CreditWordType::WORD_AW ,       593,     CreditBrainType::BRAIN_NEXT_WORD },
    {  960.0f,      CreditWordType::WORD_AW ,       616,     CreditBrainType::BRAIN_NEXT_WORD },
    {  966.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FLY_OFF   },
    {  967.0f,      CreditWordType::WORD_AA ,         0,     CreditBrainType::BRAIN_OFF       },
    {  971.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_OFF       },
    {  972.0f,      CreditWordType::WORD_AW ,       214,     CreditBrainType::BRAIN_FAST_ON   },
    {  974.0f,      CreditWordType::WORD_AW ,       297,     CreditBrainType::BRAIN_NEXT_WORD },
    {  976.0f,      CreditWordType::WORD_AW ,       348,     CreditBrainType::BRAIN_NEXT_WORD },
    {  980.0f,      CreditWordType::WORD_EE ,       400,     CreditBrainType::BRAIN_NEXT_WORD },
    {  984.0f,      CreditWordType::WORD_AW ,       455,     CreditBrainType::BRAIN_NEXT_WORD },
    {  986.0f,      CreditWordType::WORD_OH ,       523,     CreditBrainType::BRAIN_NEXT_WORD },
    {  990.0f,      CreditWordType::WORD_AW ,       593,     CreditBrainType::BRAIN_NEXT_WORD },
    {  994.0f,      CreditWordType::WORD_AW ,       616,     CreditBrainType::BRAIN_NEXT_WORD },
    {  998.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FLY_OFF   },
    { 1001.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_OFF       },
    { 1002.0f,      CreditWordType::WORD_EE ,       150,     CreditBrainType::BRAIN_FAST_ON   },
    { 1004.0f,      CreditWordType::WORD_OH ,       220,     CreditBrainType::BRAIN_NEXT_WORD },
    { 1006.0f,      CreditWordType::WORD_AW ,       307,     CreditBrainType::BRAIN_NEXT_WORD },
    { 1008.0f,      CreditWordType::WORD_AW ,       390,     CreditBrainType::BRAIN_NEXT_WORD },
    { 1012.0f,      CreditWordType::WORD_EE ,       452,     CreditBrainType::BRAIN_NEXT_WORD },
    { 1016.0f,      CreditWordType::WORD_AW ,       512,     CreditBrainType::BRAIN_NEXT_WORD },
    { 1018.0f,      CreditWordType::WORD_AW ,       573,     CreditBrainType::BRAIN_NEXT_WORD },
    { 1022.0f,      CreditWordType::WORD_AW ,       630,     CreditBrainType::BRAIN_NEXT_WORD },
    { 1024.0f,      CreditWordType::WORD_AW ,       656,     CreditBrainType::BRAIN_NEXT_WORD },
    { 1028.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_FLY_OFF   },
    { 1029.0f,      CreditWordType::WORD_AA ,         0,     CreditBrainType::BRAIN_OFF       },
    { 1033.0f,      CreditWordType::WORD_OFF,         0,     CreditBrainType::BRAIN_OFF       }
};
static int gCreditsTimingCount = LENGTH(gCreditsTiming);

CreditsOverlay::CreditsOverlay(CreditScreen* theCreditScreen)
{
	mParent = theCreditScreen;
	mMouseVisible = false;
	mHasAlpha = true;
}

void CreditsOverlay::Draw(Graphics* g)
{
	mParent->DrawOverlay(g);
}

// GOTY @Patoke: 0x4367F0
CreditScreen::CreditScreen(LawnApp* theApp)
{
	mApp = theApp;
	mClip = false;
	mCreditsReanimID = ReanimationID::REANIMATIONID_NULL;
	mFogParticleID = ParticleSystemID::PARTICLESYSTEMID_NULL;
	mUpdateCount = 0;
	mDrawCount = 0;
    mLastDrawCount = 0;
	mBlinkCountdown = 700;
	mDontSync = false;
	mCreditsPaused = false;
	mPreloaded = false;

	mApp->mEffectSystem->EffectSystemFreeAll();
	mApp->mMusic->StopAllMusic();
	mLoadedResourceNames.push_back("DelayLoad_Credits");
	TodLoadResources("DelayLoad_Credits");

    mBrainPosX = 0.0f;
    mBrainPosY = 0.0f;
    mCreditsPhase = CreditsPhase::CREDITS_MAIN1;
    mCreditsPhaseCounter = 0;
    mDrawBrain = false;

	mMainMenuButton = MakeButton(CreditScreen::Credits_Button_MainMenu, this, "[CREDITS_MAIN_MENU_BUTTON]");
	mMainMenuButton->Resize(298, 554, 209, 46);
	mMainMenuButton->SetVisible(false);

	mReplayButton = MakeNewButton(CreditScreen::Credits_Button_Replay, this, "[CREDITS_REPLAY_BUTTON]", FONT_HOUSEOFTERROR16, IMAGE_CREDITS_PLAYBUTTON, nullptr, nullptr);
	mReplayButton->mTextDownOffsetX = 1;
	mReplayButton->mTextDownOffsetY = 1;
	mReplayButton->mColors[ButtonWidget::COLOR_LABEL] = Color(255, 255, 255);
	mReplayButton->mColors[ButtonWidget::COLOR_LABEL_HILITE] = Color(213, 159, 43);
	mReplayButton->Resize(10, 530, 125, 65);
    mReplayButton->SetVisible(false);
	mReplayButton->mTextOffsetX = 33;
	mReplayButton->mTextOffsetY = -5;

	mOverlayWidget = new CreditsOverlay(this);
	mOverlayWidget->Resize(0, 0, BOARD_WIDTH, BOARD_HEIGHT);
	
	mOriginalMusicVolume = mApp->mMusicVolume;
	if (mApp->mMusicVolume < 0.1)
	{
		mApp->SetMusicVolume(0.85);
	}
}

CreditScreen::~CreditScreen()
{
	mApp->SetMusicVolume(mOriginalMusicVolume);
	delete mReplayButton;
	delete mMainMenuButton;
	delete mOverlayWidget;
}

void CreditScreen::AddedToManager(WidgetManager* theWidgetManager)
{
	Widget::AddedToManager(theWidgetManager);
	AddWidget(mMainMenuButton);
	AddWidget(mReplayButton);
	AddWidget(mOverlayWidget);
}

void CreditScreen::RemovedFromManager(WidgetManager* theWidgetManager)
{
	Widget::RemovedFromManager(theWidgetManager);
	RemoveWidget(mMainMenuButton);
	RemoveWidget(mReplayButton);
	RemoveWidget(mOverlayWidget);
}

void CreditScreen::PreLoadCredits()
{
    mPreloaded = true;
	mLoadedResourceNames.push_back("DelayLoad_Background1");
	mLoadedResourceNames.push_back("DelayLoad_Background2");
	mLoadedResourceNames.push_back("DelayLoad_Background3");
	mLoadedResourceNames.push_back("DelayLoad_Background4");
	mLoadedResourceNames.push_back("DelayLoad_Background5");
	mLoadedResourceNames.push_back("DelayLoad_Background6");

	for (std::string& resource : mLoadedResourceNames)
		TodLoadResources(resource.c_str());

    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_CREDITS_MAIN, true);
    ReanimationPreload(ReanimationType::REANIM_CREDITS_MAIN);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_CREDITS_MAIN2, true);
    ReanimationPreload(ReanimationType::REANIM_CREDITS_MAIN2);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_CREDITS_MAIN3, true);
    ReanimationPreload(ReanimationType::REANIM_CREDITS_MAIN3);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_ZOMBIE_CREDITS_DANCE, true);
    ReanimationPreload(ReanimationType::REANIM_ZOMBIE_CREDITS_DANCE);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_CREDITS_BIGBRAIN, true);
    ReanimationPreload(ReanimationType::REANIM_CREDITS_BIGBRAIN);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_CREDITS_FLOWER_PETALS, true);
    ReanimationPreload(ReanimationType::REANIM_CREDITS_FLOWER_PETALS);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_CREDITS_INFANTRY, true);
    ReanimationPreload(ReanimationType::REANIM_CREDITS_INFANTRY);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_CREDITS_STAGE, true);
    ReanimationPreload(ReanimationType::REANIM_CREDITS_STAGE);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_CREDITS_THROAT, true);
    ReanimationPreload(ReanimationType::REANIM_CREDITS_THROAT);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_SUNFLOWER, true);
    ReanimationPreload(ReanimationType::REANIM_SUNFLOWER);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_SUN, true);
    ReanimationPreload(ReanimationType::REANIM_SUN);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_CREDITS_CRAZYDAVE, true);
    ReanimationPreload(ReanimationType::REANIM_CREDITS_CRAZYDAVE);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_ZOMBIE, true);
    ReanimationPreload(ReanimationType::REANIM_ZOMBIE);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_DIGGER, true);
    ReanimationPreload(ReanimationType::REANIM_DIGGER);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_ZOMBIE_FOOTBALL, true);
    ReanimationPreload(ReanimationType::REANIM_ZOMBIE_FOOTBALL);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_ZOMBIE_CREDITS_CONEHEAD, true);
    ReanimationPreload(ReanimationType::REANIM_ZOMBIE_CREDITS_CONEHEAD);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_ZOMBIE_CREDITS_SCREEN_DOOR, true);
    ReanimationPreload(ReanimationType::REANIM_ZOMBIE_CREDITS_SCREEN_DOOR);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_CREDITS_ZOMBIEARMY1, true);
    ReanimationPreload(ReanimationType::REANIM_CREDITS_ZOMBIEARMY1);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_CREDITS_ZOMBIEARMY2, true);
    ReanimationPreload(ReanimationType::REANIM_CREDITS_ZOMBIEARMY2);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_CREDITS_TOMBSTONES, true);
    ReanimationPreload(ReanimationType::REANIM_CREDITS_TOMBSTONES);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_CREDITS_SOLARPOWER, true);
    ReanimationPreload(ReanimationType::REANIM_CREDITS_SOLARPOWER);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_CREDITS_ANYHOUR, true);
    ReanimationPreload(ReanimationType::REANIM_CREDITS_ANYHOUR);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_CREDITS_WEARETHEUNDEAD, true);
    ReanimationPreload(ReanimationType::REANIM_CREDITS_WEARETHEUNDEAD);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_CREDITS_DISCOLIGHTS, true);
    ReanimationPreload(ReanimationType::REANIM_CREDITS_DISCOLIGHTS);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_CREDITS_BOSSDANCE, true);
    ReanimationPreload(ReanimationType::REANIM_CREDITS_BOSSDANCE);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_TALLNUT, true);
    ReanimationPreload(ReanimationType::REANIM_TALLNUT);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_SNOWPEA, true);
    ReanimationPreload(ReanimationType::REANIM_SNOWPEA);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_PEASHOOTER, true);
    ReanimationPreload(ReanimationType::REANIM_PEASHOOTER);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_REPEATER, true);
    ReanimationPreload(ReanimationType::REANIM_REPEATER);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_KERNELPULT, true);
    ReanimationPreload(ReanimationType::REANIM_KERNELPULT);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_WALLNUT, true);
    ReanimationPreload(ReanimationType::REANIM_WALLNUT);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_FUMESHROOM, true);
    ReanimationPreload(ReanimationType::REANIM_FUMESHROOM);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_PUFFSHROOM, true);
    ReanimationPreload(ReanimationType::REANIM_PUFFSHROOM);
    ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_SUNSHROOM, true);
    ReanimationPreload(ReanimationType::REANIM_SUNSHROOM);

    ReanimatorDefinition* aMain2Def = &gReanimatorDefArray[ReanimationType::REANIM_CREDITS_MAIN2];
    for (int aTrackIndex = 0; aTrackIndex < aMain2Def->mTracks.count; aTrackIndex++)
    {
        ReanimatorTrack* aTrack = &aMain2Def->mTracks.tracks[aTrackIndex];
        for (int aTransIndex = 0; aTransIndex < aTrack->mTransforms.count; aTransIndex++)
        {
            ReanimatorTransform& aTrans = aTrack->mTransforms.mTransforms[aTransIndex];
            if (aTransIndex < 124 && (strcmp(aTrack->mName, "Words") == 0 || strcmp(aTrack->mName, "Words2") == 0))
            {
                aTrans.mFont = FONT_BRIANNETOD32BLACK;
            }
        }
    }
    ReanimatorDefinition* aMain3Def = &gReanimatorDefArray[ReanimationType::REANIM_CREDITS_MAIN3];
    for (int aTrackIndex = 0; aTrackIndex < aMain3Def->mTracks.count; aTrackIndex++)
    {
        ReanimatorTrack* aTrack = &aMain3Def->mTracks.tracks[aTrackIndex];
        for (int aTransIndex = 0; aTransIndex < aTrack->mTransforms.count; aTransIndex++)
        {
            ReanimatorTransform& aTrans = aTrack->mTransforms.mTransforms[aTransIndex];
            if (aTransIndex < 124 && (strcmp(aTrack->mName, "Words") == 0 || strcmp(aTrack->mName, "Words2") == 0))
            {
                aTrans.mFont = FONT_BRIANNETOD32BLACK;
            }
        }
    }
    ReanimatorDefinition* aUndeadDef = &gReanimatorDefArray[ReanimationType::REANIM_CREDITS_WEARETHEUNDEAD];
    for (int aTrackIndex = 0; aTrackIndex < aUndeadDef->mTracks.count; aTrackIndex++)
    {
        ReanimatorTrack* aTrack = &aUndeadDef->mTracks.tracks[aTrackIndex];
        for (int aTransIndex = 0; aTransIndex < aTrack->mTransforms.count; aTransIndex++)
        {
            ReanimatorTransform& aTrans = aTrack->mTransforms.mTransforms[aTransIndex];
            if (aTrans.mFont)
            {
                aTrans.mFont = FONT_BRIANNETOD32BLACK;
            }
        }
    }
}

void CreditScreen::GetTiming(CreditsTiming** theBeforeTiming, CreditsTiming** theAfterTiming, float* theFraction)
{
    Reanimation* aCreditsReanim = mApp->ReanimationGet(mCreditsReanimID);
    float aFrameCount = aCreditsReanim->mDefinition->mTracks.tracks->mTransforms.count * aCreditsReanim->mAnimTime - 1.0f;
    if (mCreditsPhase == CreditsPhase::CREDITS_MAIN1)
    {
        aFrameCount += 2.0f;
    }
    else if (mCreditsPhase == CreditsPhase::CREDITS_MAIN2)
    {
        aFrameCount += 400.0f;
    }
    else if (mCreditsPhase == CreditsPhase::CREDITS_MAIN3)
    {
        aFrameCount += 785.0f;
    }
    else
    {
        *theBeforeTiming = nullptr;
        *theAfterTiming = nullptr;
        *theFraction = 0.0f;
        return;
    }

    if (aFrameCount < gCreditsTiming[0].mFrame)
    {
        *theBeforeTiming = nullptr;
        *theAfterTiming = &gCreditsTiming[0];
        *theFraction = aFrameCount / gCreditsTiming[0].mFrame;
    }
    else
    {
        for (int i = 0; i < gCreditsTimingCount - 1; i++)
        {
            CreditsTiming* aTiming1 = &gCreditsTiming[i];
            CreditsTiming* aTiming2 = &gCreditsTiming[i + 1];
            TOD_ASSERT(aTiming1->mFrame < aTiming2->mFrame);

            if (aTiming2->mFrame > aFrameCount)
            {
                *theBeforeTiming = aTiming1;
                *theAfterTiming = aTiming2;
                *theFraction = (aFrameCount - aTiming1->mFrame) / (aTiming2->mFrame - aTiming1->mFrame);
                return;
            }
        }

        *theBeforeTiming = &gCreditsTiming[gCreditsTimingCount - 1];
        *theAfterTiming = nullptr;
        *theFraction = 0.0f;
    }
}

Reanimation* CreditScreen::PlayReanim(int aIndex)
{
    Reanimation* aCreditsReanim = mApp->ReanimationTryToGet(mCreditsReanimID);
    if (aCreditsReanim)
    {
        aCreditsReanim->ReanimationDie();
    }

    if (aIndex == 1)
    {
        aCreditsReanim = mApp->AddReanimation(0.0f, 0.0f, 0, ReanimationType::REANIM_CREDITS_MAIN);
        aCreditsReanim->AssignRenderGroupToPrefix("Background", 1);
        aCreditsReanim->AssignRenderGroupToPrefix("attacher__Zombie", 2);
        aCreditsReanim->AssignRenderGroupToPrefix("Words", 3);
        aCreditsReanim->AssignRenderGroupToPrefix("SpotFront", 3);
    }
    else if (aIndex == 2)
    {
        aCreditsReanim = mApp->AddReanimation(0.0f, 0.0f, 0, ReanimationType::REANIM_CREDITS_MAIN2);
        aCreditsReanim->AssignRenderGroupToPrefix("Background", 1);
        aCreditsReanim->AssignRenderGroupToPrefix("attacher__Zombie", 2);
        aCreditsReanim->AssignRenderGroupToPrefix("Words", 3);
        aCreditsReanim->AssignRenderGroupToPrefix("SpotFront", 3);
        aCreditsReanim->AssignRenderGroupToPrefix("attacher__undead", 2);
    }
    else if (aIndex == 3)
    {
        aCreditsReanim = mApp->AddReanimation(0.0f, 0.0f, 0, ReanimationType::REANIM_CREDITS_MAIN3);
        aCreditsReanim->AssignRenderGroupToPrefix("Background", 1);
        aCreditsReanim->AssignRenderGroupToPrefix("attacher__Zombie", 2);
        aCreditsReanim->AssignRenderGroupToPrefix("attacher__DiscoLights", 2);
        aCreditsReanim->AssignRenderGroupToPrefix("Words", 3);
        aCreditsReanim->AssignRenderGroupToPrefix("attacher__cattail", 3);
        aCreditsReanim->AssignRenderGroupToPrefix("SpotFront", 3);
        aCreditsReanim->AssignRenderGroupToPrefix("attacher__undead", 2);
    }
    else
    {
        TOD_ASSERT(false);
        return nullptr;
    }

    aCreditsReanim->mIsAttachment = true;
    aCreditsReanim->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
    mCreditsReanimID = mApp->ReanimationGetID(aCreditsReanim);
	return aCreditsReanim;
}

void DrawDisco(Graphics* g, float aCenterX, float aCenterY, float theTime)
{
    if (!gSexyAppBase->Is3DAccelerated())
        return;

    float x1 = cos(theTime) * 600.0f;
    float y1 = sin(theTime) * 200.0f;
    float x2 = cos(theTime + PI / 2) * 600.0f;
    float y2 = sin(theTime + PI / 2) * 200.0f;
    float x3 = cos(theTime + PI) * 600.0f;
    float y3 = sin(theTime + PI) * 200.0f;
    float x4 = cos(theTime + PI + PI / 2) * 600.0f;
    float y4 = sin(theTime + PI + PI / 2) * 200.0f;

    TriVertex aVerts[2][3];
    int aColor = Color(255, 255, 255, 128).ToInt();
    aVerts[0][0].x = x1 + aCenterX;
    aVerts[0][0].y = y1 + aCenterY;
    aVerts[0][0].u = 0.0;
    aVerts[0][0].v = 0.0;
    aVerts[0][0].color = aColor;
    aVerts[0][1].x = x2 + aCenterX;
    aVerts[0][1].y = y2 + aCenterY;
    aVerts[0][1].u = 1.0;
    aVerts[0][1].v = 0.0;
    aVerts[0][1].color = aColor;
    aVerts[0][2].x = x4 + aCenterX;
    aVerts[0][2].y = y4 + aCenterY;
    aVerts[0][2].u = 0.0;
    aVerts[0][2].v = 1.0;
    aVerts[0][2].color = aColor;
    aVerts[1][0].x = x4 + aCenterX;
    aVerts[1][0].y = y4 + aCenterY;
    aVerts[1][0].u = 0.0;
    aVerts[1][0].v = 1.0;
    aVerts[1][0].color = aColor;
    aVerts[1][1].x = x2 + aCenterX;
    aVerts[1][1].y = y2 + aCenterY;
    aVerts[1][1].u = 1.0;
    aVerts[1][1].v = 0.0;
    aVerts[1][1].color = aColor;
    aVerts[1][2].x = x3 + aCenterX;
    aVerts[1][2].y = y3 + aCenterY;
    aVerts[1][2].u = 1.0;
    aVerts[1][2].v = 1.0;
    aVerts[1][2].color = aColor;

    g->mDestImage->BltTrianglesTex(
        IMAGE_REANIM_CREDITS_DISCOLIGHTS, 
        aVerts, 
        2, 
        Rect(0, 0, BOARD_WIDTH, BOARD_HEIGHT), 
        Color::White, 
        g->mDrawMode, 
        0.0f, 
        0.0f, 
        g->mLinearBlend
    );
}

void CreditScreen::DrawFogEffect(Graphics* g, float theTime)
{
    Reanimation* aCreditsReanim = mApp->ReanimationGet(mCreditsReanimID);
    Image* aFogImage = mApp->Is3DAccelerated() ? IMAGE_FOG : IMAGE_FOG_SOFTWARE;
    int aFadeAmount = theTime * 255.0f;

    for (int x = 0; x < 14; x++)
    {
        for (int y = 0; y < 7; y++)
        {
            // 取得格子内的雾的形状
            int aCelLook = x + (x + 17) * y;
            int aCelCol = aCelLook % 8;
            // 本格浓雾横坐标 = 列 * 80 + 浓雾偏移 - 15
            float aPosX = x * 80 - 15.0f;
            // 本格浓雾纵坐标 = 行 * 85 + 200
            float aPosY = y * 85 + 200.0f;
            // 开始计算周期变化的颜色，aAnimTime 为 MV 动画播放至当前时刻需要的时间（秒数）
            float aAnimTime = aCreditsReanim->mDefinition->mTracks.tracks->mTransforms.count * aCreditsReanim->mAnimTime / (aCreditsReanim->mAnimRate * SECONDS_PER_UPDATE);
            float aTime = aAnimTime * PI * 2;
            // 与行、列有关的初始相位
            float aPhaseX = 6 * PI * x / 14;
            float aPhaseY = 6 * PI * y / 7;
            // 根据初相和时间计算当前相位
            float aMotion = 13 + 4 * sin(aTime / 900 + aPhaseY) + 8 * sin(aTime / 500 + aPhaseX);

            int aColorVariant = 255 - (aCelLook % 20) * 1.5f - aMotion * 1.5f;
            int aLightnessVariant = 255 - (aCelLook % 20) - aMotion;
            if (!mApp->Is3DAccelerated())
            {
                aPosX += 10;
                aPosY += 23;
                aCelCol = aCelLook % Sexy::IMAGE_FOG_SOFTWARE->mNumCols;
                aColorVariant = 255;
                aLightnessVariant = 255;
            }

            g->SetColorizeImages(true);
            g->SetColor(Color(aColorVariant, aColorVariant, aLightnessVariant, aFadeAmount));
            g->DrawImageCel(aFogImage, aPosX, aPosY, aCelCol, 0);
            g->SetColorizeImages(false);
        }
    }
}

void CreditScreen::DrawOverlay(Graphics* g)
{
    if (mCreditsPhase == CreditsPhase::CREDITS_END)
    {
        int aFadeAlpha = TodAnimateCurve(50, 100, mCreditsPhaseCounter, 255, 0, TodCurves::CURVE_LINEAR);
        if (aFadeAlpha > 0)
        {
            g->SetColor(Color(0, 0, 0, aFadeAlpha));
            g->FillRect(0, 0, mWidth, mHeight);
        }
    }
}

static int DrawCreditsContent(Graphics* g, int theYPos, bool theDraw)
{
    _Font* aTitleFont = FONT_HOUSEOFTERROR28;
    _Font* aCreditsFont = FONT_HOUSEOFTERROR16;
    int aLineHeight = aCreditsFont->GetLineSpacing();

    constexpr int ROLES_RIGHT_X = BOARD_WIDTH / 2 - 15;
    constexpr int NAMES_LEFT_X = BOARD_WIDTH / 2 + 15;

    int aYPos = theYPos;
    int aStartY = aYPos;

    // Title
    if (theDraw && aYPos > -aTitleFont->GetLineSpacing() && aYPos < BOARD_HEIGHT + aTitleFont->GetLineSpacing())
        TodDrawString(g, "[CREDITS_GAMENAME]", BOARD_WIDTH / 2, aYPos, aTitleFont, Color::White, DrawStringJustification::DS_ALIGN_CENTER);
    aYPos += aTitleFont->GetLineSpacing() + 20;

    for (int aSection = 3; aSection <= 11; aSection++)
    {
        std::string aRolesKey = StrFormat("[CREDITS_ROLES%d]", aSection);
        if (!TodStringListExists(aRolesKey))
            continue;

        std::string aRoles = TodStringTranslate(aRolesKey);

        // ^ prefix: spacer or centered header
        if (!aRoles.empty() && aRoles[0] == '^')
        {
            if (aRoles.size() > 1)
            {
                if (theDraw && aYPos > -aLineHeight && aYPos < BOARD_HEIGHT + aLineHeight)
                    TodDrawString(g, aRoles.substr(1), BOARD_WIDTH / 2, aYPos, aCreditsFont, Color::White, DrawStringJustification::DS_ALIGN_CENTER);
            }
            aYPos += aLineHeight + 10;
            continue;
        }

        std::string aNamesKey = StrFormat("[CREDITS_NAMES%d]", aSection);
        std::string aNames = TodStringListExists(aNamesKey) ? TodStringTranslate(aNamesKey) : "";

        // Split roles and names by newline, draw side by side
        size_t aRolePos = 0;
        size_t aNamePos = 0;
        while (aRolePos < aRoles.size() || aNamePos < aNames.size())
        {
            bool aVisible = theDraw && aYPos > -aLineHeight && aYPos < BOARD_HEIGHT + aLineHeight;
            if (aRolePos < aRoles.size())
            {
                size_t aRoleEnd = aRoles.find('\n', aRolePos);
                if (aRoleEnd == std::string::npos) aRoleEnd = aRoles.size();
                if (aVisible && aRoleEnd > aRolePos)
                    TodDrawString(g, aRoles.substr(aRolePos, aRoleEnd - aRolePos), ROLES_RIGHT_X, aYPos, aCreditsFont, Color::White, DrawStringJustification::DS_ALIGN_RIGHT);
                aRolePos = aRoleEnd + 1;
            }
            if (aNamePos < aNames.size())
            {
                size_t aNameEnd = aNames.find('\n', aNamePos);
                if (aNameEnd == std::string::npos) aNameEnd = aNames.size();
                if (aVisible && aNameEnd > aNamePos)
                    TodDrawString(g, aNames.substr(aNamePos, aNameEnd - aNamePos), NAMES_LEFT_X, aYPos, aCreditsFont, Color::White, DrawStringJustification::DS_ALIGN_LEFT);
                aNamePos = aNameEnd + 1;
            }
            aYPos += aLineHeight;
        }
        aYPos += 10;
    }
    return aYPos - aStartY;
}

// Original 0x435550 version is replaced by this new version.
// Based on wszqkzqk's observations and speculations about GOTY's behavior.
void CreditScreen::DrawFinalCredits(Graphics* g)
{
    // Measure total content height (no draw)
    int aContentHeight = DrawCreditsContent(nullptr, 0, false);
    int aTotalCycle = aContentHeight + BOARD_HEIGHT;  // full loop: content scrolls entirely through screen
    int aScrollOffset = static_cast<int>(mCreditsPhaseCounter * CREDIT_SCREEN_ANIM_RATE) % aTotalCycle;

    DrawCreditsContent(g, BOARD_HEIGHT - aScrollOffset, true);
}

void DrawReanimToPreload(Graphics* g, ReanimationType theReanimType)
{
    Reanimation aReanim;
    aReanim.mAnimRate = CREDIT_SCREEN_ANIM_RATE;
    aReanim.ReanimationInitializeType(0.0f, 0.0f, theReanimType);
    aReanim.Draw(g);
}

void CreditScreen::Draw(Graphics* g)
{
    g->SetLinearBlend(true);

    if (!mPreloaded)
    {
        g->SetColor(Color::Black);
        g->FillRect(0, 0, mWidth, mHeight);
        mDrawCount = 1;
        return;
    }

    mDrawCount++;
    if (mDrawCount == 2)
    {
        g->DrawImage(IMAGE_BACKGROUND1, 0, 0);
        g->DrawImage(IMAGE_BACKGROUND2, 0, 0);
        g->DrawImage(IMAGE_BACKGROUND3, 0, 0);
        g->DrawImage(IMAGE_BACKGROUND4, 0, 0);
        g->DrawImage(IMAGE_BACKGROUND5, 0, 0);
        DrawReanimToPreload(g, ReanimationType::REANIM_CREDITS_BOSSDANCE);
        DrawReanimToPreload(g, ReanimationType::REANIM_CREDITS_CRAZYDAVE);
        DrawReanimToPreload(g, ReanimationType::REANIM_ZOMBIE_CREDITS_DANCE);
        DrawReanimToPreload(g, ReanimationType::REANIM_CREDITS_STAGE);
        DrawReanimToPreload(g, ReanimationType::REANIM_SUNFLOWER);
    }
    g->SetColor(Color::Black);
    g->FillRect(0, 0, mWidth, mHeight);

    Reanimation* aCreditsReanim = mApp->ReanimationGet(mCreditsReanimID);
    int aFrameCount = aCreditsReanim->mDefinition->mTracks.tracks->mTransforms.count - 1;
    float aFrameFactor = 1.0f / aFrameCount;
    int aBackground1Index = aCreditsReanim->FindTrackIndex("Background");
    ReanimatorTransform aTransformBackground1;
    aCreditsReanim->GetCurrentTransform(aBackground1Index, &aTransformBackground1);
    int aBackground2Index = aCreditsReanim->FindTrackIndex("Background2");
    ReanimatorTransform aTransformBackground2;
    aCreditsReanim->GetCurrentTransform(aBackground2Index, &aTransformBackground2);
    Graphics aBackground2G(*g);

    bool aDrawClippedBackgrounds1 = false;
    bool aDrawClippedBackgrounds2 = false;
    if (mCreditsPhase == CreditsPhase::CREDITS_MAIN2 && aFrameFactor * 125.0f > aCreditsReanim->mAnimTime)
    {
        aDrawClippedBackgrounds1 = true;
    }
    if (mCreditsPhase == CreditsPhase::CREDITS_MAIN3 && aFrameFactor * 125.0f > aCreditsReanim->mAnimTime)
    {
        aDrawClippedBackgrounds2 = true;
    }

    if (mCreditsPhase == CreditsPhase::CREDITS_END)
    {
        DrawFinalCredits(g);
    }
    if (aDrawClippedBackgrounds1)
    {
        int aBackground3Index = aCreditsReanim->FindTrackIndex("Background3");
        ReanimatorTransform aTransformBackground3;
        aCreditsReanim->GetCurrentTransform(aBackground3Index, &aTransformBackground3);
        int aBackground4Index = aCreditsReanim->FindTrackIndex("Background4");
        ReanimatorTransform aTransformBackground4;
        aCreditsReanim->GetCurrentTransform(aBackground4Index, &aTransformBackground4);
        
        if (aTransformBackground2.mFrame != -1.0f)
        {
            aBackground2G.ClipRect(aTransformBackground2.mTransX, aTransformBackground2.mTransY, aTransformBackground2.mImage->mWidth - 1, aTransformBackground2.mImage->mHeight - 1);
            aBackground2G.DrawImageF(IMAGE_BACKGROUND1, aTransformBackground2.mTransX - BOARD_WIDTH / 2, aTransformBackground2.mTransY - BOARD_HEIGHT / 2);
            aBackground2G.ClearClipRect();
        }
        if (aTransformBackground3.mFrame != -1.0f)
        {
            Graphics aBackground3G(*g);
            aBackground3G.ClipRect(aTransformBackground3.mTransX, aTransformBackground3.mTransY, aTransformBackground3.mImage->mWidth - 1, aTransformBackground3.mImage->mHeight - 1);
            aBackground3G.DrawImageF(IMAGE_BACKGROUND1, aTransformBackground3.mTransX - BOARD_WIDTH / 2, aTransformBackground3.mTransY - BOARD_HEIGHT / 2);
        }
        if (aTransformBackground4.mFrame != -1.0f)
        {
            Graphics aBackground4G(*g);
            aBackground4G.ClipRect(aTransformBackground4.mTransX, aTransformBackground4.mTransY, aTransformBackground4.mImage->mWidth - 1, aTransformBackground4.mImage->mHeight - 1);
            aBackground4G.DrawImageF(IMAGE_BACKGROUND2, aTransformBackground4.mTransX - BOARD_WIDTH / 2, aTransformBackground4.mTransY - BOARD_HEIGHT / 2);
        }
    }
    if (aDrawClippedBackgrounds2)
    {
        int aBackground1Index = aCreditsReanim->FindTrackIndex("Background");
        ReanimatorTransform aTransformBackground1;
        aCreditsReanim->GetCurrentTransform(aBackground1Index, &aTransformBackground1);
        int aBackground3Index = aCreditsReanim->FindTrackIndex("Background3");
        ReanimatorTransform aTransformBackground3;
        aCreditsReanim->GetCurrentTransform(aBackground3Index, &aTransformBackground3);
        int aBackground4Index = aCreditsReanim->FindTrackIndex("Background4");
        ReanimatorTransform aTransformBackground4;
        aCreditsReanim->GetCurrentTransform(aBackground4Index, &aTransformBackground4);

        if (aTransformBackground1.mFrame != -1.0f)
        {
            Graphics aBackground1G(*g);
            aBackground1G.ClipRect(aTransformBackground1.mTransX, aTransformBackground1.mTransY, aTransformBackground1.mImage->mWidth - 1, aTransformBackground1.mImage->mHeight - 1);
            // Digger zombie says "I like your tricycle" at this position (determined by adjusting and comparing it with the original version)
            aBackground1G.DrawImageF(IMAGE_BACKGROUND1, aTransformBackground1.mTransX - BOARD_WIDTH / 10, aTransformBackground1.mTransY - BOARD_HEIGHT * 0.6f);
        }
        if (aTransformBackground3.mFrame != -1.0f)
        {
            Graphics aBackground3G(*g);
            aBackground3G.mTransX += aTransformBackground3.mTransX - 20.0f;
            aBackground3G.mTransY += aTransformBackground3.mTransY - 260.0f;
            aBackground3G.ClipRect(20, 260, aTransformBackground3.mImage->mWidth - 1, aTransformBackground3.mImage->mHeight - 1);
            aBackground3G.DrawImageF(IMAGE_BACKGROUND3, -220.0f, 0.0f);
            aBackground3G.DrawImageF(IMAGE_POOL, 34.0f, 278.0f);
        }
        if (aTransformBackground4.mFrame != -1.0f)
        {
            Graphics aBackground4G(*g);
            aBackground4G.ClipRect(aTransformBackground4.mTransX, aTransformBackground4.mTransY, aTransformBackground4.mImage->mWidth - 1, aTransformBackground4.mImage->mHeight - 1);
            aBackground4G.DrawImageF(IMAGE_BACKGROUND2, aTransformBackground4.mTransX - BOARD_WIDTH / 2, aTransformBackground4.mTransY - BOARD_HEIGHT / 2);
        }
    }
    aCreditsReanim->DrawRenderGroup(g, 1);

    bool aDrawPool = false;
    bool aDrawNightPool = false;
    bool aDrawDoorBottom = false;
    bool aDrawChimney = false;
    bool aDrawDiscoLights = false;
    bool aDrawFog = false;
    if (mCreditsPhase == CreditsPhase::CREDITS_MAIN1)
    {
        if (aCreditsReanim->mAnimTime > aFrameFactor * 203.0f && aCreditsReanim->mAnimTime < aFrameFactor * 268.0f)
        {
            aDrawPool = true;
        }
        if (aCreditsReanim->mAnimTime > aFrameFactor * 305.0f && aCreditsReanim->mAnimTime < aFrameFactor * 339.0f)
        {
            aDrawPool = true;
            aDrawDoorBottom = true;
        }
    }
    if (mCreditsPhase == CreditsPhase::CREDITS_MAIN2)
    {
        if (aCreditsReanim->mAnimTime > aFrameFactor * 187.0f && aCreditsReanim->mAnimTime < aFrameFactor * 249.0f)
        {
            aDrawPool = true;
            aDrawNightPool = true;
            if (aTransformBackground1.mFrame != -1.0f || aTransformBackground2.mFrame != -1.0f)
            {
                aDrawDiscoLights = true;
            }
        }
        if (aCreditsReanim->mAnimTime > aFrameFactor * 123.0f && aCreditsReanim->mAnimTime < aFrameFactor * 189.0f)
        {
            if (aTransformBackground1.mFrame != -1.0f)
            {
                aDrawDiscoLights = true;
            }
        }
        if (aCreditsReanim->mAnimTime > aFrameFactor * 189.0f && aCreditsReanim->mAnimTime < aFrameFactor * 249.0f)
        {
            if (aTransformBackground2.mFrame != -1.0f)
            {
                aDrawDiscoLights = true;
                aDrawFog = true;
            }
        }
    }
    if (mCreditsPhase == CreditsPhase::CREDITS_MAIN3)
    {
        if (aCreditsReanim->mAnimTime > aFrameFactor * 123.0f && aCreditsReanim->mAnimTime < aFrameFactor * 218.0f)
        {
            aDrawChimney = true;
        }
        if (aCreditsReanim->mAnimTime > aFrameFactor * 217.0f)
        {
            if (aTransformBackground1.mFrame != -1.0f)
            {
                aDrawDiscoLights = true;
            }
        }
    }
    if (aTransformBackground2.mFrame == -1.0f)
    {
        //aDrawPool = false;
        //aDrawNightPool = false;
        aDrawDoorBottom = false;
        aDrawChimney = false;
    }
    else
    {
        aBackground2G.mTransX += aTransformBackground2.mTransX + 220.0f;
        if (aDrawPool || aDrawNightPool)
        {
            mApp->mPoolEffect->PoolEffectDraw(&aBackground2G, aDrawNightPool);
        }
    }
    
    if (aDrawDoorBottom)
    {
        aBackground2G.DrawImage(IMAGE_BACKGROUND3_GAMEOVER_INTERIOR_OVERLAY, -171, 241);
    }
    aCreditsReanim->Draw(g);

    if (aDrawDoorBottom)
    {
        g->ClipRect(48, 0, BOARD_WIDTH, BOARD_HEIGHT);
    }
    if (aDrawDiscoLights)
    {
        float aDiscoTime = aCreditsReanim->mDefinition->mTracks.tracks->mTransforms.count * aCreditsReanim->mAnimTime / aCreditsReanim->mAnimRate;
        DrawDisco(g, 600.0f, 450.0f, aDiscoTime);
        DrawDisco(g, 200.0f, 450.0f, aDiscoTime);
    }
    if (aDrawFog)
    {
        aBackground2G.DrawImage(IMAGE_REANIM_CREDITS_FOGMACHINE, 600, 200);
    }
    aCreditsReanim->DrawRenderGroup(g, 2);

    if (aDrawDoorBottom)
    {
        g->ClearClipRect();
        aBackground2G.DrawImage(IMAGE_BACKGROUND3_GAMEOVER_MASK, -172, 234);
    }
    if (aDrawChimney)
    {
        aBackground2G.DrawImage(IMAGE_BACKGROUND5_GAMEOVER_MASK, -220, 81);
    }
    aCreditsReanim->DrawRenderGroup(g, 3);

    TodParticleSystem* aParticle = nullptr;
    while (mApp->mEffectSystem->mParticleHolder->mParticleSystems.IterateNext(aParticle))
    {
        if (!aParticle->mIsAttachment && !aParticle->mDead)
        {
            aParticle->Draw(g);
        }
    }

    if (aDrawFog)
    {
        float aPercent = TodAnimateCurveFloatTime(aFrameFactor * 189.0f, aFrameFactor * 249.0f, aCreditsReanim->mAnimTime, 0.0f, 1.0f, TodCurves::CURVE_LINEAR);
        DrawFogEffect(&aBackground2G, aPercent);
    }
    aCreditsReanim->DrawRenderGroup(g, 3);

    if (mDrawBrain)
    {
        g->DrawImageF(IMAGE_BRAIN, mBrainPosX, mBrainPosY);
    }
}

Reanimation* CreditScreen::FindSubReanim(Reanimation* theReanim, ReanimationType theReanimType)
{
    if (theReanim->mReanimationType == theReanimType)
        return theReanim;

    for (int i = 0; i < theReanim->mDefinition->mTracks.count; i++)
    {
        Reanimation* aReanimation = FindReanimAttachment(theReanim->mTrackInstances[i].mAttachmentID);
        if (aReanimation != nullptr)
        {
            Reanimation* aSubReanim = FindSubReanim(aReanimation, theReanimType);
            if (aSubReanim != nullptr)
            {
                return aSubReanim;
            }
        }
    }

    return nullptr;
}

void CreditScreen::UpdateBlink()
{
    mBlinkCountdown--;
    if (mBlinkCountdown > 0)
    {
        return;
    }

    mBlinkCountdown = 700;
    Reanimation* aCreditsReanim = mApp->ReanimationGet(mCreditsReanimID);
    Reanimation* aSunFlowerReanim = FindSubReanim(aCreditsReanim, ReanimationType::REANIM_SUNFLOWER);
    if (aSunFlowerReanim)
    {
        if (mCreditsPhase == CreditsPhase::CREDITS_MAIN3)
        {
            int aFrameCount = aCreditsReanim->mDefinition->mTracks.tracks->mTransforms.count - 1;
            float aFrameFactor = 1.0f / aFrameCount;
            if (aCreditsReanim->mAnimTime > aFrameFactor * 200.0f)
            {
                return;
            }
        }

        Reanimation* aBlinkReanim = mApp->AddReanimation(0.0f, 0.0f, 0, ReanimationType::REANIM_SUNFLOWER);
        aBlinkReanim->SetFramesForLayer("anim_blink");
        aBlinkReanim->mAnimRate = 15.0f;
        aBlinkReanim->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_FULL_LAST_FRAME;
        aBlinkReanim->AttachToAnotherReanimation(aSunFlowerReanim, "anim_idle");
    }
}

void CreditScreen::Update()
{
    Widget::Update();
    if (!mCreditsPaused && !mMainMenuButton->mIsOver && !mReplayButton->mIsOver)
    {
        mApp->SetCursor(CURSOR_POINTER);
    }
    if (mDrawCount == 0 || mCreditsPaused)
    {
        return;
    }

    mUpdateCount++;
    if (mUpdateCount == 1)
    {
        PreLoadCredits();
        PlayReanim(1);
        mApp->mMusic->MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_CREDITS_ZOMBIES_ON_YOUR_LAWN);
        mApp->ClearUpdateBacklog(false);
        mTimerSinceStart.Start();
    }
    else if (mDontSync || mCreditsPhase == CreditsPhase::CREDITS_END)
    {
        UpdateMovie();
    }
    else if (mUpdateCount > 1)
    {
        Reanimation* aCreditsReanim = mApp->ReanimationGet(mCreditsReanimID);
        int aDurationSinceStart = mTimerSinceStart.GetDuration();
        int aDurationReanimation = (aCreditsReanim->mDefinition->mTracks.tracks->mTransforms.count * aCreditsReanim->mAnimTime / aCreditsReanim->mAnimRate) * 1000.0f;
        if (mCreditsPhase == CreditsPhase::CREDITS_MAIN2)
        {
            aDurationReanimation += 57142;
        }
        else if (mCreditsPhase == CreditsPhase::CREDITS_MAIN3)
        {
            aDurationReanimation += 112000;
        }

        int aUnsyncedDuration = aDurationSinceStart - aDurationReanimation;
        int aUnsyncedFrames = (aUnsyncedDuration + 5) / 10;
        if (aUnsyncedFrames < 0)
        {
            TodTrace("Movie playing too fast %d frames", 1 - aUnsyncedFrames);
        }
        else if (aUnsyncedFrames > 2)
        {
            TodTrace("Movie playing too slow %d frames", aUnsyncedFrames - 1);
        }

        if (aUnsyncedDuration > 10000)
        {
            JumpToFrame(static_cast<CreditsPhase>(static_cast<int>(mCreditsPhase) + 1), 0.0f);
            aUnsyncedFrames = 0;
        }
        while (aUnsyncedFrames > 0)
        {
            UpdateMovie();
            aUnsyncedFrames--;
        }
    }

    mLastDrawCount = mDrawCount;
    MarkDirty();
}

void CreditScreen::UpdateMovie()
{
    UpdateBlink();

    Reanimation* aCreditsReanim = mApp->ReanimationGet(mCreditsReanimID);
    float aFrameFactor = 1.0f / (aCreditsReanim->mDefinition->mTracks.tracks->mTransforms.count - 1);

    aCreditsReanim->Update();
    mApp->mEffectSystem->Update();
    mApp->mPoolEffect->PoolEffectUpdate();
    TurnOffTongues(aCreditsReanim, 0);

    if (mCreditsPhase == CreditsPhase::CREDITS_MAIN1 && aCreditsReanim->mLoopCount > 0)
    {
        aCreditsReanim = PlayReanim(2);
        mCreditsPhase = CreditsPhase::CREDITS_MAIN2;
    }
    else if (mCreditsPhase == CreditsPhase::CREDITS_MAIN2 && aCreditsReanim->mLoopCount > 0)
    {
        aCreditsReanim = PlayReanim(3);
        mCreditsPhase = CreditsPhase::CREDITS_MAIN3;
    }
    else if (mCreditsPhase == CreditsPhase::CREDITS_MAIN3 && aCreditsReanim->mLoopCount > 0)
    {
        mCreditsPhase = CreditsPhase::CREDITS_END;
    }
    else if (mCreditsPhase == CreditsPhase::CREDITS_END)
    {
        mCreditsPhaseCounter++;
        if (mCreditsPhaseCounter == 50)
        {
            mMainMenuButton->SetVisible(true);
            mReplayButton->SetVisible(true);
        }
    }

    if (mCreditsPhase == CreditsPhase::CREDITS_MAIN1)
    {
        if (aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 128.0f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 130.0f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 132.0f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 134.0f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 136.0f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 138.0f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 140.0f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 142.0f))
        {
            mApp->AddTodParticle(BOARD_WIDTH / 2, BOARD_HEIGHT / 2, static_cast<int>(RenderLayer::RENDER_LAYER_TOP), ParticleEffect::PARTICLE_CREDIT_STROBE);
        }
        if (aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 136.5f))
        {
            mApp->AddTodParticle(BOARD_WIDTH / 2, BOARD_HEIGHT / 2, static_cast<int>(RenderLayer::RENDER_LAYER_TOP), ParticleEffect::PARTICLE_CREDITS_RAYSWIPE);
        }
        if (aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 330.0f))
        {
            mApp->PlayFoley(FoleyType::FOLEY_SCREAM);
        }
        if (aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 336.0f))
        {
            mApp->mSoundSystem->StopFoley(FoleyType::FOLEY_SCREAM);
        }
    }
    if (mCreditsPhase == CreditsPhase::CREDITS_MAIN2 || mCreditsPhase == CreditsPhase::CREDITS_MAIN3)
    {
        ReanimatorTrackInstance* aTrackWords1 = aCreditsReanim->GetTrackInstanceByName("Words");
        ReanimatorTrackInstance* aTrackWords2 = aCreditsReanim->GetTrackInstanceByName("Words2");
        if (aCreditsReanim->mAnimTime < aFrameFactor * 124.0f)
        {
            aTrackWords1->mTrackColor = Color::Black;
            aTrackWords2->mTrackColor = Color::Black;
        }
        else
        {
            aTrackWords1->mTrackColor = Color::White;
            aTrackWords2->mTrackColor = Color::White;
        }

        Reanimation* aUndeadReanim = FindSubReanim(aCreditsReanim, ReanimationType::REANIM_CREDITS_WEARETHEUNDEAD);
        if (aUndeadReanim)
        {
            ReanimatorTrackInstance* aTrackWordsUndead1 = aUndeadReanim->GetTrackInstanceByName("bubbletext1");
            ReanimatorTrackInstance* aTrackWordsUndead2 = aUndeadReanim->GetTrackInstanceByName("bubbletext2");
            ReanimatorTrackInstance* aTrackWordsUndead3 = aUndeadReanim->GetTrackInstanceByName("bubbletext3");
            aTrackWordsUndead1->mTrackColor = Color::Black;
            aTrackWordsUndead2->mTrackColor = Color::Black;
            aTrackWordsUndead3->mTrackColor = Color::Black;

            ReanimatorTrackInstance* aTrackWordsShake = aUndeadReanim->GetTrackInstanceByName("ShakyText");
            if (aCreditsReanim->mAnimTime > aFrameFactor * 112.0f)
            {
                aTrackWordsShake->mShakeOverride = 2.0f;
            }
            else
            {
                aTrackWordsShake->mShakeOverride = 0.0f;
            }
        }

        if (aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 120.0f))
        {
            mApp->AddTodParticle(BOARD_WIDTH / 2, BOARD_HEIGHT / 2, static_cast<int>(RenderLayer::RENDER_LAYER_TOP), ParticleEffect::PARTICLE_CREDITS_ZOMBIEHEADWIPE);
        }
    }
    if (mCreditsPhase == CreditsPhase::CREDITS_MAIN2)
    {
        if (aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 111.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 115.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 119.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 121.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 123.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 125.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 127.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 131.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 135.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 139.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 143.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 147.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 151.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 155.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 159.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 163.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 167.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 171.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 175.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 179.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 183.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 187.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 191.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 195.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 199.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 203.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 207.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 211.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 215.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 219.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 223.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 227.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 231.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 235.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 239.5f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 243.5f))
        {
            mApp->AddTodParticle(BOARD_WIDTH / 2, BOARD_HEIGHT / 2, static_cast<int>(RenderLayer::RENDER_LAYER_TOP), ParticleEffect::PARTICLE_CREDIT_STROBE);
        }
        if (aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 332.75f))
        {
            mApp->AddTodParticle(678.0f, 352.0f, static_cast<int>(RenderLayer::RENDER_LAYER_TOP), ParticleEffect::PARTICLE_MELONSPLASH);
        }
        if (aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 336.0f))
        {
            TodParticleSystem* aParticle = nullptr;
            while (mApp->mEffectSystem->mParticleHolder->mParticleSystems.IterateNext(aParticle))
            {
                if (aParticle->mEffectType == ParticleEffect::PARTICLE_MELONSPLASH)
                {
                    aParticle->ParticleSystemDie();
                }
            }
        }
        if (aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 342.0f))
        {
            mApp->AddTodParticle(BOARD_WIDTH / 2, BOARD_HEIGHT / 2, static_cast<int>(RenderLayer::RENDER_LAYER_TOP), ParticleEffect::PARTICLE_CREDIT_STROBE);
        }

        int aBackground2Index = aCreditsReanim->FindTrackIndex("Background2");
        ReanimatorTransform aTransformBackground2;
        aCreditsReanim->GetCurrentTransform(aBackground2Index, &aTransformBackground2);

        float aFogPosX = aTransformBackground2.mTransX + 856.0f;
        if (aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 188.0f))
        {
            TodParticleSystem* aFogParticle = mApp->AddTodParticle(aFogPosX, 230.0f, static_cast<int>(RenderLayer::RENDER_LAYER_TOP), ParticleEffect::PARTICLE_CREDITS_FOG);
            mFogParticleID = mApp->ParticleGetID(aFogParticle);
        }
        TodParticleSystem* aFogParticle = mApp->ParticleTryToGet(mFogParticleID);
        if (aFogParticle)
        {
            if (aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 248.0f))
            {
                aFogParticle->ParticleSystemDie();
                mFogParticleID = ParticleSystemID::PARTICLESYSTEMID_NULL;
            }
            else
            {
                aFogParticle->SystemMove(aFogPosX, 230.0f);
            }
        }
    }
    if (mCreditsPhase == CreditsPhase::CREDITS_MAIN3)
    {
        if (aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 65.0f))
        {
            mApp->PlayFoley(FoleyType::FOLEY_DOLPHIN_APPEARS);
        }
        if (aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 111.0f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 115.0f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 119.0f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 121.0f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 123.0f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 219.0f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 223.0f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 227.0f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 231.0f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 235.0f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 239.0f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 243.0f) ||
            aCreditsReanim->ShouldTriggerTimedEvent(aFrameFactor * 247.0f))
        {
            mApp->AddTodParticle(BOARD_WIDTH / 2, BOARD_HEIGHT / 2, static_cast<int>(RenderLayer::RENDER_LAYER_TOP), ParticleEffect::PARTICLE_CREDIT_STROBE);
        }
    }

    float aFraction;
    CreditsTiming* aBeforeTiming;
    CreditsTiming* aAfterTiming;
    GetTiming(&aBeforeTiming, &aAfterTiming, &aFraction);

    int aSunflowerFrame = 0;
    mDrawBrain = false;
    if (aBeforeTiming && aAfterTiming)
    {
        if (aAfterTiming->mBrainType == CreditBrainType::BRAIN_FLY_ON)
        {
            mBrainPosX = TodAnimateCurveFloatTime(0.0f, 1.0f, aFraction, -50.0f, aAfterTiming->mWordX - 15.0f, TodCurves::CURVE_EASE_IN_OUT);
            mBrainPosY = TodAnimateCurveFloatTime(0.0f, 1.0f, aFraction, 505.0f, 485.0f, TodCurves::CURVE_BOUNCE_FAST_MIDDLE);
            mDrawBrain = true;
        }
        else if (aAfterTiming->mBrainType == CreditBrainType::BRAIN_FAST_ON)
        {
            mBrainPosX = TodAnimateCurveFloatTime(0.0f, 1.0f, aFraction, aAfterTiming->mWordX - 50.0f, aAfterTiming->mWordX - 15.0f, TodCurves::CURVE_EASE_IN_OUT);
            mBrainPosY = TodAnimateCurveFloatTime(0.0f, 1.0f, aFraction, 485.0f, 505.0f, TodCurves::CURVE_EASE_IN_OUT);
            mDrawBrain = true;
        }
        else if (aAfterTiming->mBrainType == CreditBrainType::BRAIN_FLY_OFF)
        {
            mBrainPosX = TodAnimateCurveFloatTime(0.0f, 1.0f, aFraction, aBeforeTiming->mWordX - 15.0f, 850.0f, TodCurves::CURVE_EASE_IN_OUT);
            mBrainPosY = TodAnimateCurveFloatTime(0.0f, 1.0f, aFraction, 505.0f, 485.0f, TodCurves::CURVE_BOUNCE_FAST_MIDDLE);
            mDrawBrain = true;
        }
        else if (aAfterTiming->mBrainType == CreditBrainType::BRAIN_FAST_OFF)
        {
            mBrainPosX = TodAnimateCurveFloatTime(0.0f, 1.0f, aFraction, aBeforeTiming->mWordX - 15.0f, aBeforeTiming->mWordX + 25.0f, TodCurves::CURVE_EASE_IN_OUT);
            mBrainPosY = TodAnimateCurveFloatTime(0.0f, 1.0f, aFraction, 505.0f, 485.0f, TodCurves::CURVE_EASE_IN_OUT);
            mDrawBrain = true;
        }
        else if (aBeforeTiming->mBrainType == CreditBrainType::BRAIN_FLY_ON || aBeforeTiming->mBrainType == CreditBrainType::BRAIN_FAST_ON || aBeforeTiming->mBrainType == CreditBrainType::BRAIN_NEXT_WORD)
        {
            mBrainPosX = TodAnimateCurveFloatTime(0.0f, 1.0f, aFraction, aBeforeTiming->mWordX - 15.0f, aAfterTiming->mWordX - 15.0f, TodCurves::CURVE_EASE_IN_OUT);
            mBrainPosY = TodAnimateCurveFloatTime(0.0f, 1.0f, aFraction, 505.0f, 485.0f, TodCurves::CURVE_BOUNCE_FAST_MIDDLE);
            mDrawBrain = true;
        }

        float aFramesForWord = aAfterTiming->mFrame - aBeforeTiming->mFrame;
        float aFramesTillEnd = (1.0f - aFraction) * aFramesForWord;
        if (aBeforeTiming->mWordType != CreditWordType::WORD_OFF && aFramesForWord * aFraction < 0.2f)
        {
            aSunflowerFrame = 1;
        }
        else if (aBeforeTiming->mWordType != CreditWordType::WORD_OFF && aFramesTillEnd < 0.4f)
        {
            aSunflowerFrame = 1;
        }
        else if (aBeforeTiming->mWordType == CreditWordType::WORD_AA)
        {
            aSunflowerFrame = 2;
        }
        else if (aBeforeTiming->mWordType == CreditWordType::WORD_EE)
        {
            aSunflowerFrame = 3;
        }
        else if (aBeforeTiming->mWordType == CreditWordType::WORD_AW)
        {
            aSunflowerFrame = 4;
        }
        else if (aBeforeTiming->mWordType == CreditWordType::WORD_OH)
        {
            aSunflowerFrame = 5;
        }
    }

    Reanimation* aSunFlowerReanim = FindSubReanim(aCreditsReanim, ReanimationType::REANIM_SUNFLOWER);
    if (aSunFlowerReanim)
    {
        if (mCreditsPhase == CreditsPhase::CREDITS_MAIN3 && aCreditsReanim->mAnimTime > aFrameFactor * 255.0f)
        {
            aSunFlowerReanim->SetImageOverride("anim_idle", IMAGE_REANIM_SUNFLOWER_HEAD_WINK);
            Reanimation* aStageReanim = FindSubReanim(aCreditsReanim, ReanimationType::REANIM_CREDITS_STAGE);
            aSunFlowerReanim->mAnimTime -= aSunFlowerReanim->mAnimRate * SECONDS_PER_UPDATE / aSunFlowerReanim->mFrameCount;
            aStageReanim->mAnimTime -= aStageReanim->mAnimRate * SECONDS_PER_UPDATE / aStageReanim->mFrameCount;
        }
        else
        {
            switch (aSunflowerFrame)
            {
            case 1:     aSunFlowerReanim->SetImageOverride("anim_idle", IMAGE_REANIM_SUNFLOWER_HEAD_SING1);     break;
            case 2:     aSunFlowerReanim->SetImageOverride("anim_idle", IMAGE_REANIM_SUNFLOWER_HEAD_SING2);     break;
            case 3:     aSunFlowerReanim->SetImageOverride("anim_idle", IMAGE_REANIM_SUNFLOWER_HEAD_SING3);     break;
            case 4:     aSunFlowerReanim->SetImageOverride("anim_idle", IMAGE_REANIM_SUNFLOWER_HEAD_SING4);     break;
            case 5:     aSunFlowerReanim->SetImageOverride("anim_idle", IMAGE_REANIM_SUNFLOWER_HEAD_SING5);     break;
            default:    aSunFlowerReanim->SetImageOverride("anim_idle", nullptr);                               break;
            }
        }
    }
}

void CreditScreen::TurnOffTongues(Reanimation* theReanim, int aParentTrack)
{
    for (int aTrackIndex = 0; aTrackIndex < theReanim->mDefinition->mTracks.count; aTrackIndex++)
    {
        ReanimatorTrackInstance* aTrackInstance = &theReanim->mTrackInstances[aTrackIndex];
        if (theReanim->mReanimationType == ReanimationType::REANIM_ZOMBIE_CREDITS_DANCE &&
            aParentTrack % 4 != 1 && strcasecmp(theReanim->mDefinition->mTracks.tracks[aTrackIndex].mName, "anim_tongue") == 0)
        {
            aTrackInstance->mRenderGroup = RENDER_GROUP_HIDDEN;
        }

        Reanimation* aAttachedReanim = FindReanimAttachment(aTrackInstance->mAttachmentID);
        if (aAttachedReanim)
        {
            TurnOffTongues(aAttachedReanim, aTrackIndex);
        }
    }
}

void CreditScreen::JumpToFrame(CreditsPhase thePhase, float theFrame)
{
    mMainMenuButton->SetVisible(false);
    mReplayButton->SetVisible(false);
    mCreditsPhaseCounter = 0;
    mApp->mEffectSystem->EffectSystemFreeAll();

    Reanimation* aReanim;
    if (thePhase == CreditsPhase::CREDITS_END)
    {
        aReanim = PlayReanim(3);
    }
    else
    {
        aReanim = PlayReanim(static_cast<int>(thePhase) + 1);
    }

    float aFrameFactor = 1.0f / (aReanim->mDefinition->mTracks.tracks->mTransforms.count - 1);
    int aMusicOffset = theFrame * 12142.0f;
    int aJumpMilliseconds = theFrame * 1000.0f / 7.0f;
    if (thePhase == CreditsPhase::CREDITS_MAIN1)
    {
        if (theFrame >= 368.0f)
        {
            aMusicOffset = 12142.0f * (theFrame - 368.0f) + 4634474.0f;
        }
        else if (theFrame >= 340.0f)
        {
            aMusicOffset = 12142.0f * (theFrame - 340.0f) + 4280738.0f;
        }
        else if (theFrame >= 304.0f)
        {
            aMusicOffset = 12142.0f * (theFrame - 304.0f) + 3825710.0f;
        }
        else if (theFrame >= 272.0f)
        {
            aMusicOffset = 12142.0f * (theFrame - 272.0f) + 3421764.0f;
        }
        else if (theFrame >= 144.0f)
        {
            aMusicOffset = 12142.0f * (theFrame - 144.0f) + 1805688.0f;
        }
        else if (theFrame >= 128.0)
        {
            aMusicOffset = 12142.0f * (theFrame - 128.0f) + 1603662.0f;
        }
    }
    else if (thePhase == CreditsPhase::CREDITS_MAIN2)
    {
        if (theFrame >= 320.0f)
        {
            aMusicOffset = 12142.0f * (theFrame - 320.0f) + 9069118.0f;
            aJumpMilliseconds += 57142;
        }
        else if (theFrame >= 248.0f)
        {
            aMusicOffset = 12142.0f * (theFrame - 248.0f) + 8159850.0f;
            aJumpMilliseconds += 57142;
        }
        else if (theFrame >= 188.0f)
        {
            aMusicOffset = 12142.0f * (theFrame - 188.0f) + 7401454.0f;
            aJumpMilliseconds += 57142;
        }
        else if (theFrame >= 124.0f)
        {
            aMusicOffset = 12142.0f * (theFrame - 124.0f) + 6593548.0f;
            aJumpMilliseconds += 57142;
        }
        else
        {
            aMusicOffset = 12142.0f * theFrame + 5026370.0f;
            aJumpMilliseconds += 57142;
        }
    }
    else if (thePhase == CreditsPhase::CREDITS_MAIN3)
    {
        if (theFrame >= 240.0f)
        {
            aMusicOffset = 12142.0f * (theFrame - 240.0f) + 12897822.0f;
            aJumpMilliseconds += 112000;
        }
        else if (theFrame >= 216.0f)
        {
            aMusicOffset = 12142.0f * (theFrame - 216.0f) + 12594510.0f;
            aJumpMilliseconds += 112000;
        }
        else if (theFrame >= 124.0f)
        {
            aMusicOffset = 12142.0f * (theFrame - 124.0f) + 11434414.0f;
            aJumpMilliseconds += 112000;
        }
        else
        {
            aMusicOffset = 12142.0f * theFrame + 9864866.0;
            aJumpMilliseconds += 112000;
        }
    }
    else if (thePhase == CreditsPhase::CREDITS_END)
    {
        aMusicOffset = 14047138;
        aJumpMilliseconds += 159142;
    }

    mApp->mMusic->PlayFromOffset(MusicFile::MUSIC_FILE_CREDITS_ZOMBIES_ON_YOUR_LAWN, aMusicOffset - 900, 1.0f);
    if (thePhase == CreditsPhase::CREDITS_END)
    {
        aReanim->mAnimTime = 1.0;
    }
    else
    {
        aReanim->mAnimTime = aFrameFactor * theFrame;
    }

    mCreditsPhase = thePhase;
    mTimerSinceStart.SetStartTime(aJumpMilliseconds);
}

void CreditScreen::KeyChar(char theChar)
{
    if (mCreditsPaused || !mApp->mDebugKeysEnabled)
        return;

    if (theChar == '1')
    {
        CreditScreen::JumpToFrame(CreditsPhase::CREDITS_MAIN1, 0.0f);
    }
    else if (theChar == '2')
    {
        CreditScreen::JumpToFrame(CreditsPhase::CREDITS_MAIN1, 128.0f);
    }
    else if (theChar == '3')
    {
        CreditScreen::JumpToFrame(CreditsPhase::CREDITS_MAIN1, 144.0f);
    }
    else if (theChar == '4')
    {
        CreditScreen::JumpToFrame(CreditsPhase::CREDITS_MAIN1, 272.0f);
    }
    else if (theChar == '5')
    {
        CreditScreen::JumpToFrame(CreditsPhase::CREDITS_MAIN1, 304.0f);
    }
    else if (theChar == '6')
    {
        CreditScreen::JumpToFrame(CreditsPhase::CREDITS_MAIN1, 340.0f);
    }
    else if (theChar == '7')
    {
        CreditScreen::JumpToFrame(CreditsPhase::CREDITS_MAIN1, 368.0f);
    }
    else if (theChar == 'q')
    {
        CreditScreen::JumpToFrame(CreditsPhase::CREDITS_MAIN2, 0.0f);
    }
    else if (theChar == 'w')
    {
        CreditScreen::JumpToFrame(CreditsPhase::CREDITS_MAIN2, 124.0f);
    }
    else if (theChar == 'e')
    {
        CreditScreen::JumpToFrame(CreditsPhase::CREDITS_MAIN2, 188.0f);
    }
    else if (theChar == 'r')
    {
        CreditScreen::JumpToFrame(CreditsPhase::CREDITS_MAIN2, 248.0f);
    }
    else if (theChar == 't')
    {
        CreditScreen::JumpToFrame(CreditsPhase::CREDITS_MAIN2, 320.0f);
    }
    else if (theChar == 'a')
    {
        CreditScreen::JumpToFrame(CreditsPhase::CREDITS_MAIN3, 0.0f);
    }
    else if (theChar == 's')
    {
        CreditScreen::JumpToFrame(CreditsPhase::CREDITS_MAIN3, 124.0f);
    }
    else if (theChar == 'd')
    {
        CreditScreen::JumpToFrame(CreditsPhase::CREDITS_MAIN3, 216.0f);
    }
    else if (theChar == 'f')
    {
        CreditScreen::JumpToFrame(CreditsPhase::CREDITS_MAIN3, 240.0f);
    }
    else if (theChar == 'g')
    {
        CreditScreen::JumpToFrame(CreditsPhase::CREDITS_MAIN3, 324.0f);
    }
    else if (theChar == 'n')
    {
        mDontSync = !mDontSync;
    }
}

void CreditScreen::PauseCredits()
{
    if (mCreditsPaused)
        return;

    mApp->mSoundSystem->StopFoley(FoleyType::FOLEY_SCREAM);
    mApp->PlaySample(SOUND_PAUSE);
    mCreditsPaused = true;
    int aDurationOnPause = mTimerSinceStart.GetDuration();
    mApp->mMusic->GameMusicPause(true);

    if (mApp->LawnMessageBox(
        Dialogs::DIALOG_MESSAGE, 
        "[CREDITS_PAUSE_HEADER]", 
        "[CREDITS_PAUSE_BODY]", 
        "[CREDITS_RESUME_BUTTON]", 
        "[MAIN_MENU_BUTTON]", 
        Dialog::BUTTONS_YES_NO) == Dialog::ID_NO)
    {
        mApp->KillCreditScreen();
        mApp->DoBackToMain();
    }

    mCreditsPaused = false;
    mApp->mMusic->GameMusicPause(false);
    mTimerSinceStart.SetStartTime(aDurationOnPause);
}

void CreditScreen::KeyDown(KeyCode theKey)
{
    if (theKey == KeyCode::KEYCODE_SPACE || theKey == KeyCode::KEYCODE_RETURN || theKey == KeyCode::KEYCODE_ESCAPE)
    {
        PauseCredits();
    }
}

void CreditScreen::ButtonPress(int theId)
{
    if (theId == CreditScreen::Credits_Button_MainMenu)
    {
        mApp->PlaySample(SOUND_GRAVEBUTTON);
    }
    else if (theId == CreditScreen::Credits_Button_Replay)
    {
        mApp->PlaySample(SOUND_TAP);
    }
}

void CreditScreen::ButtonDepress(int theId)
{
    if (theId == CreditScreen::Credits_Button_MainMenu)
    {
        mApp->KillCreditScreen();
        mApp->DoBackToMain();
    }
    else if (theId == CreditScreen::Credits_Button_Replay)
    {
        mApp->KillCreditScreen();
        mApp->ShowCreditScreen();
    }
}

void CreditScreen::MouseUp(int, int, int){}
