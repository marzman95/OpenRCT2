/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include "../Context.h"
#include "../interface/Window.h"
#include "../localisation/Localisation.h"
#include "../localisation/StringIds.h"
#include "../peep/Staff.h"
#include "../windows/Intent.h"
#include "../world/Sprite.h"
#include "GameAction.h"

DEFINE_GAME_ACTION(StaffSetOrdersAction, GAME_COMMAND_SET_STAFF_ORDERS, GameActionResult)
{
private:
    uint16_t _spriteIndex;
    uint8_t _ordersId;

public:
    StaffSetOrdersAction()
    {
    }
    StaffSetOrdersAction(uint16_t spriteIndex, uint8_t ordersId)
        : _spriteIndex(spriteIndex)
        , _ordersId(ordersId)
    {
    }

    uint16_t GetActionFlags() const override
    {
        return GameAction::GetActionFlags() | GA_FLAGS::ALLOW_WHILE_PAUSED;
    }

    void Serialise(DataSerialiser & stream) override
    {
        GameAction::Serialise(stream);

        stream << DS_TAG(_spriteIndex) << DS_TAG(_ordersId);
    }

    GameActionResult::Ptr Query() const override
    {
        if (_spriteIndex >= MAX_SPRITES)
        {
            return std::make_unique<GameActionResult>(GA_ERROR::INVALID_PARAMETERS, STR_NONE);
        }

        auto* staff = TryGetEntity<Staff>(_spriteIndex);
        if (staff == nullptr || (staff->StaffType != STAFF_TYPE_HANDYMAN && staff->StaffType != STAFF_TYPE_MECHANIC))
        {
            log_warning("Invalid game command for sprite %u", _spriteIndex);
            return std::make_unique<GameActionResult>(GA_ERROR::INVALID_PARAMETERS, STR_NONE);
        }

        return std::make_unique<GameActionResult>();
    }

    GameActionResult::Ptr Execute() const override
    {
        auto* staff = TryGetEntity<Staff>(_spriteIndex);
        if (staff == nullptr)
        {
            log_warning("Invalid game command for sprite %u", _spriteIndex);
            return std::make_unique<GameActionResult>(GA_ERROR::INVALID_PARAMETERS, STR_NONE);
        }
        staff->StaffOrders = _ordersId;

        window_invalidate_by_number(WC_PEEP, _spriteIndex);
        auto intent = Intent(INTENT_ACTION_REFRESH_STAFF_LIST);
        context_broadcast_intent(&intent);

        auto res = std::make_unique<GameActionResult>();
        res->Position.x = staff->x;
        res->Position.y = staff->y;
        res->Position.z = staff->z;
        return res;
    }
};
