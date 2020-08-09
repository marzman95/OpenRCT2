/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "../../config/Config.h"
#include "../../drawing/LightFX.h"
#include "../../interface/Viewport.h"
#include "../../peep/Peep.h"
#include "../../world/Sprite.h"
#include "../Paint.h"
#include "Paint.Sprite.h"

/**
 *
 *  rct2: 0x0068F0FB
 */
void peep_paint(paint_session* session, const Peep* peep, int32_t imageDirection)
{
#ifdef __ENABLE_LIGHTFX__
    if (lightfx_is_available())
    {
        if (peep->AssignedPeepType == PeepType::Staff)
        {
            int16_t peep_x, peep_y, peep_z;

            peep_x = peep->x;
            peep_y = peep->y;
            peep_z = peep->z;

            switch (peep->sprite_direction)
            {
                case 0:
                    peep_x -= 10;
                    break;
                case 8:
                    peep_y += 10;
                    break;
                case 16:
                    peep_x += 10;
                    break;
                case 24:
                    peep_y -= 10;
                    break;
                default:
                    return;
            }

            lightfx_add_3d_light(
                peep->sprite_index, 0x0000 | LIGHTFX_LIGHT_QUALIFIER_SPRITE, peep_x, peep_y, peep_z, LightType::Spot1);
        }
    }
#endif

    rct_drawpixelinfo* dpi = &session->DPI;
    if (dpi->zoom_level > 2)
    {
        return;
    }

    if (session->ViewFlags & VIEWPORT_FLAG_INVISIBLE_PEEPS)
    {
        return;
    }

    rct_peep_animation_entry sprite = g_peep_animation_entries[peep->SpriteType];

    PeepActionSpriteType spriteType = peep->ActionSpriteType;
    uint8_t imageOffset = peep->ActionSpriteImageOffset;

    if (peep->Action == PEEP_ACTION_NONE_1)
    {
        spriteType = peep->NextActionSpriteType;
        imageOffset = 0;
    }

    // In the following 4 calls to sub_98197C/sub_98199C, we add 5 (instead of 3) to the
    //  bound_box_offset_z to make sure peeps are drawn on top of railways
    uint32_t baseImageId = (imageDirection >> 3) + sprite.sprite_animation[spriteType].base_image + imageOffset * 4;
    uint32_t imageId = baseImageId | peep->TshirtColour << 19 | peep->TrousersColour << 24 | IMAGE_TYPE_REMAP
        | IMAGE_TYPE_REMAP_2_PLUS;
    sub_98197C(session, imageId, 0, 0, 1, 1, 11, peep->z, 0, 0, peep->z + 5);

    if (baseImageId >= 10717 && baseImageId < 10749)
    {
        imageId = (baseImageId + 32) | peep->HatColour << 19 | IMAGE_TYPE_REMAP;
        sub_98199C(session, imageId, 0, 0, 1, 1, 11, peep->z, 0, 0, peep->z + 5);
        return;
    }

    if (baseImageId >= 10781 && baseImageId < 10813)
    {
        imageId = (baseImageId + 32) | peep->BalloonColour << 19 | IMAGE_TYPE_REMAP;
        sub_98199C(session, imageId, 0, 0, 1, 1, 11, peep->z, 0, 0, peep->z + 5);
        return;
    }

    if (baseImageId >= 11197 && baseImageId < 11229)
    {
        imageId = (baseImageId + 32) | peep->UmbrellaColour << 19 | IMAGE_TYPE_REMAP;
        sub_98199C(session, imageId, 0, 0, 1, 1, 11, peep->z, 0, 0, peep->z + 5);
        return;
    }
}
