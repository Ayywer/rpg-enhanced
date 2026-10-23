#pragma once

// local
#include "player/player.hpp"
#include "player/player_class_generator.hpp"
#include "enemy/enemy_generator.hpp"
#include "actions/fight/spells.hpp"
#include "player/item_database.hpp"
#include "player/item_use_registry.hpp"

namespace rpg {

class Game {
public:
  Game()
    : m_EnemyGenerator(), m_PlayerClassGenerator(), m_SpellDatabase(),
      m_ItemDatabase(RPG_ITEM_DATABASE_PATH, RPG_EQUIPMENT_DATABASE_PATH) {
    setup();
  }

  void setup();
  void loop();

private:
  std::shared_ptr<Player> m_pPlayer;
  EnemyGenerator m_EnemyGenerator;
  PlayerClassGenerator m_PlayerClassGenerator;
  SpellDatabase m_SpellDatabase;
  ItemDatabase m_ItemDatabase;
  ItemUseRegistry m_ItemUseRegistry;
};
} // namespace rpg
