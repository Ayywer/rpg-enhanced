#include "item_use_registry.hpp"

#include "items/item_handlers.hpp"
#include "player.hpp"

#include <utility>

namespace rpg {

ItemUseRegistry::ItemUseRegistry() {
  register_healing_potion(*this);
  register_mana_potion(*this);
  register_fire_scroll(*this);
  register_lightning_scroll(*this);
  register_amber_relic(*this);
}

void ItemUseRegistry::register_function(std::string function_id,
                                       Handler handler) {
  if (function_id.empty() || !handler) {
    return;
  }
  m_Handlers.insert_or_assign(std::move(function_id), std::move(handler));
}

ItemUseResult ItemUseRegistry::use_item(Player& player,
                                        const std::string& item_id,
                                        Enemy* enemy) const {
  for (std::size_t index = 0; index < player.m_Inventory.size(); ++index) {
    const Item& item = player.m_Inventory[index].item;
    if (item.id != item_id || player.m_Inventory[index].quantity == 0 ||
        item.type == ItemType::EQUIPMENT ||
        (item.type == ItemType::RELIC && enemy != nullptr)) {
      continue;
    }
    const auto handler = m_Handlers.find(item.use_function);
    if (handler == m_Handlers.end()) {
      return {false, "No handler is registered for this item's function."};
    }
    ItemUseResult result = handler->second(player, enemy, item);
    if (!result.succeeded) {
      return result;
    }
    if (!result.consume_item) {
      return result;
    }
    --player.m_Inventory[index].quantity;
    if (player.m_Inventory[index].quantity == 0) {
      player.m_Inventory.erase(player.m_Inventory.begin() + index);
    }
    return result;
  }
  return {false, "That item is not in your inventory or cannot be used."};
}

} // namespace rpg