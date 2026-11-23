#include "../passive_ability_registry.hpp"

#include "../../enemy/enemy.hpp"
#include "../player.hpp"

namespace rpg {
namespace {

[[maybe_unused]] const bool registered = [] {
  PassiveAbilityRegistry::register_builtin_function(
      "low_health_guard", [](const PassiveAbilityContext& context) {
        if (context.trigger != PassiveAbilityTrigger::DEFENSE ||
            context.owner.m_MaxHealth == 0 || context.owner.m_Health < 0 ||
            static_cast<std::uint64_t>(context.owner.m_Health) >
                context.owner.m_MaxHealth / 2) {
          return context.raw_damage;
        }
        constexpr std::uint64_t damage_reduction = 5;
        return context.raw_damage > damage_reduction
                   ? context.raw_damage - damage_reduction
                   : 0;
      });
  return true;
}();

} // namespace
} // namespace rpg