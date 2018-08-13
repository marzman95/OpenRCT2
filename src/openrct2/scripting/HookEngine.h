/*****************************************************************************
 * Copyright (c) 2014-2018 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include "../common.h"

#include <dukglue/dukglue.h>
#include <memory>
#include <string>
#include <vector>

namespace OpenRCT2::Scripting
{
    class ScriptExecutionInfo;
    class Plugin;

    enum class HOOK_TYPE
    {
        INTERVAL_TICK,
        INTERVAL_DAY,
        COUNT,
        UNDEFINED = -1,
    };
    constexpr size_t NUM_HOOK_TYPES = static_cast<size_t>(HOOK_TYPE::COUNT);
    HOOK_TYPE GetHookType(const std::string& name);

    struct Hook
    {
        uint32_t Cookie;
        std::shared_ptr<Plugin> Owner;
        DukValue Function;

        Hook();
        Hook(uint32_t cookie, std::shared_ptr<Plugin> owner, const DukValue& function)
            : Cookie(cookie)
            , Owner(owner)
            , Function(function)
        {
        }
    };

    struct HookList
    {
        HOOK_TYPE Type;
        std::vector<Hook> Hooks;

        HookList()
        {
        }
        HookList(const HookList&) = delete;
        HookList(HookList&& src)
            : Type(std::move(src.Type))
            , Hooks(std::move(src.Hooks))
        {
        }
    };

    class HookEngine
    {
    private:
        ScriptExecutionInfo& _execInfo;
        std::vector<HookList> _hookMap;
        size_t _numHooks{};
        uint32_t _nextCookie = 1;

    public:
        HookEngine(ScriptExecutionInfo& execInfo);
        HookEngine(const HookEngine&) = delete;
        uint32_t Subscribe(HOOK_TYPE type, std::shared_ptr<Plugin> owner, const DukValue& function);
        void Unsubscribe(HOOK_TYPE type, uint32_t cookie);
        void UnsubscribeAll(std::shared_ptr<const Plugin> owner);
        void Call(HOOK_TYPE type);

    private:
        HookList& GetHookList(HOOK_TYPE type);
    };
} // namespace OpenRCT2::Scripting