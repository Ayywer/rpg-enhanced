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
  Shop(std::shared_ptr<Player> player, const ItemDatabase& item_database)
      : m_pPlayer(player), m_ItemDatabase(item_database) {}

  void loop();

private:
  std::shared_ptr<Player> m_pPlayer;
  const ItemDatabase& m_ItemDatabase;
};

} // namespace rpg
