#include "../item_use_registry.hpp"

#include "../player.hpp"

#include <algorithm>
#include <string>

namespace rpg {

namespace {
[[maybe_unused]] const bool registered = [] {
  ItemUseRegistry::register_builtin_function(
      "mana_potion", [](Player& player, Enemy*, const Item& item) {
        if (player.m_Mana >= player.m_MaxMana) {
          return ItemUseResult{false, "Mana is already full."};
        }
        const std::uint64_t restored =
            std::min(item.power, player.m_MaxMana - player.m_Mana);
        player.m_Mana += restored;
        return ItemUseResult{true, item.name + " restored " +
                                       std::to_string(restored) + " mana."};
      });
      return true;
    }();
    } // namespace

} // namespace rpg