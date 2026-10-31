#include "item_use_registry.hpp"

#include "player.hpp"

#include <stdexcept>
#include <utility>

namespace rpg {

ItemUseRegistry::HandlerMap& ItemUseRegistry::builtin_handlers() {
  static HandlerMap handlers;
  return handlers;
}

void ItemUseRegistry::register_builtin_function(std::string function_id,
                                                Handler handler) {
  if (function_id.empty() || !handler) {
    throw std::invalid_argument("Built-in item handler requires an ID and function");
  }
  if (!builtin_handlers().emplace(std::move(function_id), std::move(handler)).second) {
    throw std::invalid_argument("Duplicate built-in item handler registration");
  }
}

bool ItemUseRegistry::has_builtin_function(
    const std::string& function_id) {
  return builtin_handlers().find(function_id) != builtin_handlers().end();
}

void ItemUseRegistry::register_function(std::string function_id,
                                       Handler handler) {
  if (function_id.empty() || !handler) {
    return;
  }
  m_Handlers.insert_or_assign(std::move(function_id), std::move(handler));
}

bool ItemUseRegistry::contains(const std::string& function_id) const {
  return m_Handlers.find(function_id) != m_Handlers.end() ||
         has_builtin_function(function_id);
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
    const Handler* handler = nullptr;
    const auto custom_handler = m_Handlers.find(item.use_function);
    if (custom_handler != m_Handlers.end()) {
      handler = &custom_handler->second;
    } else {
      const auto builtin_handler = builtin_handlers().find(item.use_function);
      if (builtin_handler != builtin_handlers().end()) {
        handler = &builtin_handler->second;
      }
    }
    if (handler == nullptr) {
      return {false, "No handler is registered for this item's function."};
    }
    ItemUseResult result = (*handler)(player, enemy, item);
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