#include "../item_use_registry.hpp"

#include "../player.hpp"

#include <algorithm>
#include <cstdint>
#include <string>

namespace rpg {

namespace {
[[maybe_unused]] const bool registered = [] {
  ItemUseRegistry::register_builtin_function(
      "healing_potion", [](Player& player, Enemy*, const Item& item) {
        if (player.m_Health >= static_cast<std::int64_t>(player.m_MaxHealth)) {
          return ItemUseResult{false, "Health is already full."};
        }
        const std::uint64_t current = static_cast<std::uint64_t>(
            std::max<std::int64_t>(0, player.m_Health));
        const std::uint64_t restored =
            std::min(item.power, player.m_MaxHealth - current);
        player.m_Health += static_cast<std::int64_t>(restored);
        return ItemUseResult{true, item.name + " restored " +
                                       std::to_string(restored) + " health."};
      });
      return true;
    }();
    } // namespace

} // namespace rpg