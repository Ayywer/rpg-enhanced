#include "../passive_ability_registry.hpp"

#include "../../enemy/enemy.hpp"
#include "../player.hpp"

#include <limits>

namespace rpg {
namespace {

[[maybe_unused]] const bool registered = [] {
  PassiveAbilityRegistry::register_builtin_function(
      "target_below_health", [](const PassiveAbilityContext& context) {
        if (context.trigger != PassiveAbilityTrigger::ATTACK ||
            context.opponent.m_MaxHealth == 0 ||
            context.opponent.m_Health < 0 ||
            static_cast<std::uint64_t>(context.opponent.m_Health) >
                context.opponent.m_MaxHealth / 2) {
          return context.raw_damage;
        }
        constexpr std::uint64_t bonus_damage = 5;
        return context.raw_damage >
                       std::numeric_limits<std::uint64_t>::max() - bonus_damage
                   ? std::numeric_limits<std::uint64_t>::max()
                   : context.raw_damage + bonus_damage;
      });
  return true;
}();

} // namespace
} // namespace rpg