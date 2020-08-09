/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#ifndef _INTRO_H_
#define _INTRO_H_

#include "common.h"

struct rct_drawpixelinfo;

enum class IntroState : uint8_t
{
    None,
    PublisherBegin,
    PublisherScroll,
    DeveloperBegin,
    DeveloperScroll,
    LogoFadeIn,
    LogoWait,
    LogoFadeOut,
    Disclaimer1,
    Disclaimer2,
    Clear = 254,
    Finish = 255,
};

extern IntroState gIntroState;

void intro_update();
void intro_draw(rct_drawpixelinfo* dpi);

#endif
