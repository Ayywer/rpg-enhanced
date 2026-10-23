
#include "../item_use_registry.hpp"

#include "../player.hpp"

namespace rpg {

void register_amber_relic(ItemUseRegistry& registry) {
  registry.register_function(
      "amber_relic", [](Player& player, Enemy*, const Item& item) {
        for (std::size_t index = 0; index < player.m_ActiveRelics.size(); ++index) {
          if (player.m_ActiveRelics[index].id != item.id) {
            continue;
          }
          player.m_ActiveRelics.erase(player.m_ActiveRelics.begin() + index);
          return ItemUseResult{true, item.name + " deactivated.", false};
        }
        player.m_ActiveRelics.push_back(item);
        return ItemUseResult{true, item.name + " activated: +" +
                                 std::to_string(item.bonus_strength) +
                                 " strength, +" +
                                 std::to_string(item.bonus_defense) +
                                 " defense.", false};
      });
}

} // namespace rpg