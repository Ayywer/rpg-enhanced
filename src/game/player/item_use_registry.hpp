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

  static void register_builtin_function(std::string function_id,
                                        Handler handler);
  static bool has_builtin_function(const std::string& function_id);
  void register_function(std::string function_id, Handler handler);
  bool contains(const std::string& function_id) const;
  ItemUseResult use_item(Player& player, const std::string& item_id,
                         Enemy* enemy = nullptr) const;

private:
  using HandlerMap = std::unordered_map<std::string, Handler>;
  static HandlerMap& builtin_handlers();

  std::unordered_map<std::string, Handler> m_Handlers;
};

} // namespace rpg