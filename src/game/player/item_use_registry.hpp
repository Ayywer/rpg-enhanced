#pragma once

#include "item.hpp"

#include <functional>
#include <string>
#include <unordered_map>

namespace rpg {

class Enemy;
class Player;

struct ItemUseResult {
  bool succeeded = false;
  std::string message;
  bool consume_item = true;
};

class ItemUseRegistry {
public:
  using Handler =
      std::function<ItemUseResult(Player&, Enemy*, const Item&)>;

  ItemUseRegistry();

  void register_function(std::string function_id, Handler handler);
  ItemUseResult use_item(Player& player, const std::string& item_id,
                         Enemy* enemy = nullptr) const;

private:
  std::unordered_map<std::string, Handler> m_Handlers;
};

} // namespace rpg