/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#ifdef ENABLE_SCRIPTING

#    include "../common.h"
#    include "../ride/Ride.h"
#    include "../world/Map.h"
#    include "Duktape.hpp"
#    include "ScEntity.hpp"
#    include "ScRide.hpp"
#    include "ScTile.hpp"

namespace OpenRCT2::Scripting
{
    class ScMap
    {
    private:
        duk_context* _context;

    public:
        ScMap(duk_context* ctx)
            : _context(ctx)
        {
        }

        DukValue size_get() const
        {
            auto ctx = _context;
            auto objIdx = duk_push_object(ctx);
            duk_push_number(ctx, gMapSize);
            duk_put_prop_string(ctx, objIdx, "x");
            duk_push_number(ctx, gMapSize);
            duk_put_prop_string(ctx, objIdx, "y");
            return DukValue::take_from_stack(ctx);
        }

        int32_t numRides_get() const
        {
            return static_cast<int32_t>(GetRideManager().size());
        }

        int32_t numEntities_get() const
        {
            return MAX_SPRITES;
        }

        std::vector<std::shared_ptr<ScRide>> rides_get() const
        {
            std::vector<std::shared_ptr<ScRide>> result;
            auto rideManager = GetRideManager();
            for (const auto& ride : rideManager)
            {
                result.push_back(std::make_shared<ScRide>(ride.id));
            }
            return result;
        }

        std::shared_ptr<ScRide> getRide(int32_t id) const
        {
            auto rideManager = GetRideManager();
            if (id >= 0 && id < static_cast<int32_t>(rideManager.size()))
            {
                auto ride = rideManager[static_cast<ride_id_t>(id)];
                if (ride != nullptr)
                {
                    return std::make_shared<ScRide>(ride->id);
                }
            }
            return {};
        }

        std::shared_ptr<ScTile> getTile(int32_t x, int32_t y) const
        {
            auto coords = TileCoordsXY(x, y).ToCoordsXY();
            return std::make_shared<ScTile>(coords);
        }

        DukValue getEntity(int32_t id) const
        {
            if (id >= 0 && id < MAX_SPRITES)
            {
                auto spriteId = static_cast<uint16_t>(id);
                auto sprite = GetEntity(spriteId);
                if (sprite != nullptr && sprite->sprite_identifier != SPRITE_IDENTIFIER_NULL)
                {
                    return GetEntityAsDukValue(sprite);
                }
            }
            duk_push_null(_context);
            return DukValue::take_from_stack(_context);
        }

        std::vector<DukValue> getAllEntities(const std::string& type) const
        {
            EntityListId targetList{};
            uint8_t targetType{};
            if (type == "balloon")
            {
                targetList = EntityListId::Misc;
                targetType = SPRITE_MISC_BALLOON;
            }
            if (type == "car")
            {
                targetList = EntityListId::TrainHead;
            }
            else if (type == "litter")
            {
                targetList = EntityListId::Litter;
            }
            else if (type == "duck")
            {
                targetList = EntityListId::Misc;
                targetType = SPRITE_MISC_DUCK;
            }
            else if (type == "peep")
            {
                targetList = EntityListId::Peep;
            }
            else
            {
                duk_error(_context, DUK_ERR_ERROR, "Invalid entity type.");
            }

            std::vector<DukValue> result;
            for (auto sprite : EntityList(targetList))
            {
                // Only the misc list checks the type property
                if (targetList != EntityListId::Misc || sprite->type == targetType)
                {
                    if (targetList == EntityListId::Peep)
                    {
                        if (sprite->Is<Staff>())
                            result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScStaff>(sprite->sprite_index)));
                        else
                            result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScGuest>(sprite->sprite_index)));
                    }
                    else if (targetList == EntityListId::TrainHead)
                    {
                        for (auto carId = sprite->sprite_index; carId != SPRITE_INDEX_NULL;)
                        {
                            auto car = GetEntity<Vehicle>(carId);
                            result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScVehicle>(carId)));
                            carId = car->next_vehicle_on_train;
                        }
                    }
                    else
                    {
                        result.push_back(GetObjectAsDukValue(_context, std::make_shared<ScEntity>(sprite->sprite_index)));
                    }
                }
            }
            return result;
        }

        static void Register(duk_context* ctx)
        {
            dukglue_register_property(ctx, &ScMap::size_get, nullptr, "size");
            dukglue_register_property(ctx, &ScMap::numRides_get, nullptr, "numRides");
            dukglue_register_property(ctx, &ScMap::numEntities_get, nullptr, "numEntities");
            dukglue_register_property(ctx, &ScMap::rides_get, nullptr, "rides");
            dukglue_register_method(ctx, &ScMap::getRide, "getRide");
            dukglue_register_method(ctx, &ScMap::getTile, "getTile");
            dukglue_register_method(ctx, &ScMap::getEntity, "getEntity");
            dukglue_register_method(ctx, &ScMap::getAllEntities, "getAllEntities");
        }

    private:
        DukValue GetEntityAsDukValue(const SpriteBase* sprite) const
        {
            auto spriteId = sprite->sprite_index;
            switch (sprite->sprite_identifier)
            {
                case SPRITE_IDENTIFIER_VEHICLE:
                    return GetObjectAsDukValue(_context, std::make_shared<ScVehicle>(spriteId));
                case SPRITE_IDENTIFIER_PEEP:
                {
                    if (sprite->Is<Staff>())
                        return GetObjectAsDukValue(_context, std::make_shared<ScStaff>(spriteId));
                    else
                        return GetObjectAsDukValue(_context, std::make_shared<ScGuest>(spriteId));
                }
                default:
                    return GetObjectAsDukValue(_context, std::make_shared<ScEntity>(spriteId));
            }
        }
    };
} // namespace OpenRCT2::Scripting

#endif
