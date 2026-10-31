#include "../entry_condition_registry.hpp"

#include "game/player/player.hpp"

#include <cstdint>
#include <string>
#include <variant>

namespace rpg {
namespace {

std::uint64_t player_stat(const Player& player, const std::string& stat) {
    if (stat == "strength") {
        return player.m_Strength;
    }
    if (stat == "intelligence") {
        return player.m_Intelligence;
    }
    if (stat == "precision") {
        return player.m_Precision;
    }
    return 0;
}

} // namespace

namespace {
[[maybe_unused]] const bool registered = [] {
    RegionEntryConditionRegistry::register_builtin_function(
        "minimum_stat",
        [](const Player& player, const RegionEntryCondition& condition) {
            const auto stat_parameter = condition.parameters.find("stat");
            const auto value_parameter = condition.parameters.find("value");
            if (stat_parameter == condition.parameters.end() ||
                value_parameter == condition.parameters.end()) {
                return RegionEntryCheckResult{
                    false,
                    "The minimum-stat condition needs 'stat' and 'value'."};
            }
            const auto* stat = std::get_if<std::string>(&stat_parameter->second);
            const auto* required_value =
                std::get_if<std::int64_t>(&value_parameter->second);
            if (stat == nullptr || required_value == nullptr ||
                *required_value < 0 ||
                (*stat != "strength" && *stat != "intelligence" &&
                 *stat != "precision")) {
                return RegionEntryCheckResult{
                    false, "The minimum-stat condition has invalid parameters."};
            }
            if (player_stat(player, *stat) <
                static_cast<std::uint64_t>(*required_value)) {
                return RegionEntryCheckResult{
                    false, "You need at least " + std::to_string(*required_value) +
                               " " + *stat + " to enter."};
            }
            return RegionEntryCheckResult{true, {}};
        });
    return true;
}();
} // namespace

} // namespace rpg