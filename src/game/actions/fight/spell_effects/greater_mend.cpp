#include "../spell_use_registry.hpp"

#include "../spells.hpp"

#include <algorithm>
#include <limits>

namespace rpg {

namespace {
[[maybe_unused]] const bool registered = [] {
  SpellUseRegistry::register_builtin_function(
      "greater_mend", [](Player& player, Enemy&, const SpellDefinition& spell) {
        const std::uint64_t capacity = static_cast<std::uint64_t>(
            std::numeric_limits<std::int64_t>::max() - player.m_Health);
        const std::uint64_t healed = std::min(spell.power, capacity);
        player.m_Health += static_cast<std::int64_t>(healed);
        return SpellEffectResult{healed, 0};
      });
      return true;
    }();
    } // namespace

} // namespace rpg