#include "../spell_use_registry.hpp"

#include "../spells.hpp"

#include <algorithm>
#include <limits>

namespace rpg {

namespace {
[[maybe_unused]] const bool registered = [] {
  SpellUseRegistry::register_builtin_function(
      "sunder", [](Player& player, Enemy& enemy,
                   const SpellDefinition& spell) {
        if (enemy.m_Health <= 0) return SpellEffectResult{};
        const std::uint64_t remaining = static_cast<std::uint64_t>(enemy.m_Health);
        const std::uint64_t intelligence = player.m_Intelligence;
        const std::uint64_t damage =
            intelligence != 0 && spell.power >
                std::numeric_limits<std::uint64_t>::max() / intelligence
                ? remaining
                : std::min(spell.power * intelligence / 10, remaining);
        enemy.m_Health -= static_cast<std::int64_t>(damage);
        return SpellEffectResult{0, damage};
      });
      return true;
    }();
    } // namespace

} // namespace rpg