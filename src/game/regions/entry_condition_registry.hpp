#pragma once

#include "game/region.hpp"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace rpg {

class Player;

struct RegionEntryCheckResult {
    bool allowed;
    std::string message;
};

class RegionEntryConditionRegistry {
public:
    using Handler = std::function<RegionEntryCheckResult(
        const Player&, const RegionEntryCondition&)>;

    static void register_builtin_function(std::string function_id,
                                          Handler handler);
    static bool has_builtin_function(const std::string& function_id);
    void register_function(std::string function_id, Handler handler);
    bool contains(const std::string& function_id) const;
    RegionEntryCheckResult check(
        const std::vector<RegionEntryCondition>& conditions,
        const Player& player) const;

private:
    using HandlerMap = std::unordered_map<std::string, Handler>;
    static HandlerMap& builtin_handlers();

    std::unordered_map<std::string, Handler> m_Handlers;
};

} // namespace rpg