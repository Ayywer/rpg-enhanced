#include "entry_condition_registry.hpp"

#include <stdexcept>
#include <utility>

namespace rpg {

RegionEntryConditionRegistry::HandlerMap&
RegionEntryConditionRegistry::builtin_handlers() {
    static HandlerMap handlers;
    return handlers;
}

void RegionEntryConditionRegistry::register_builtin_function(
    std::string function_id, Handler handler) {
    if (function_id.empty() || !handler) {
        throw std::invalid_argument(
            "Built-in entry-condition handler needs an ID and function");
    }
    if (!builtin_handlers().emplace(std::move(function_id), std::move(handler)).second) {
        throw std::invalid_argument(
            "Duplicate built-in entry-condition handler registration");
    }
}

bool RegionEntryConditionRegistry::has_builtin_function(
    const std::string& function_id) {
    return builtin_handlers().contains(function_id);
}

void RegionEntryConditionRegistry::register_function(std::string function_id,
                                                     Handler handler) {
    if (function_id.empty() || !handler) {
        throw std::invalid_argument(
            "Entry conditions need a function ID and handler");
    }
    m_Handlers.insert_or_assign(std::move(function_id), std::move(handler));
}

bool RegionEntryConditionRegistry::contains(
    const std::string& function_id) const {
    return m_Handlers.contains(function_id) ||
           has_builtin_function(function_id);
}

RegionEntryCheckResult RegionEntryConditionRegistry::check(
    const std::vector<RegionEntryCondition>& conditions,
    const Player& player) const {
    for (const RegionEntryCondition& condition : conditions) {
        const auto custom_handler = m_Handlers.find(condition.function_id);
        if (custom_handler != m_Handlers.end()) {
            RegionEntryCheckResult result =
                custom_handler->second(player, condition);
            if (!result.allowed) {
                return result;
            }
            continue;
        }
        const auto builtin_handler =
            builtin_handlers().find(condition.function_id);
        if (builtin_handler == builtin_handlers().end()) {
            return {false, "No entry-condition handler is registered for '" +
                               condition.function_id + "'."};
        }
        RegionEntryCheckResult result =
            builtin_handler->second(player, condition);
        if (!result.allowed) {
            return result;
        }
    }
    return {true, {}};
}

} // namespace rpg