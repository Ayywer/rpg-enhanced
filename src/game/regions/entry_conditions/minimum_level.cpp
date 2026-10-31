#include "../entry_condition_registry.hpp"

#include "game/player/player.hpp"

#include <cstdint>
#include <string>
#include <variant>

namespace rpg {

namespace {
[[maybe_unused]] const bool registered = [] {
    RegionEntryConditionRegistry::register_builtin_function(
        "minimum_level",
        [](const Player& player, const RegionEntryCondition& condition) {
            const auto parameter = condition.parameters.find("level");
            if (parameter == condition.parameters.end()) {
                return RegionEntryCheckResult{
                    false, "The minimum-level condition has no 'level' value."};
            }
            const auto* required_level =
                std::get_if<std::int64_t>(&parameter->second);
            if (required_level == nullptr || *required_level < 1) {
                return RegionEntryCheckResult{
                    false, "The minimum-level condition has an invalid level."};
            }
            if (player.m_Level < static_cast<std::uint64_t>(*required_level)) {
                return RegionEntryCheckResult{
                    false, "You must be at least level " +
                               std::to_string(*required_level) + " to enter."};
            }
            return RegionEntryCheckResult{true, {}};
        });
    return true;
}();
} // namespace

} // namespace rpg