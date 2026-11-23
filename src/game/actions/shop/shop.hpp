#pragma once

// local
#include "../../player/item_database.hpp"
#include "../../player/player.hpp"

// std
#include <cstdint>
#include <memory>

namespace rpg {
typedef std::uint64_t cost;

class Shop {
public:
  Shop(std::shared_ptr<Player> player, const ItemDatabase& item_database,
       std::string shop_id)
      : m_pPlayer(player), m_ItemDatabase(item_database),
        m_ShopId(std::move(shop_id)) {}

  void loop();

private:
  std::shared_ptr<Player> m_pPlayer;
  const ItemDatabase& m_ItemDatabase;
  std::string m_ShopId;
};

} // namespace rpg
