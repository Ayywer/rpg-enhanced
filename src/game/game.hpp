#pragma once

// local
#include "player/player.hpp"
#include "player/player_class_generator.hpp"
#include "enemy/enemy_generator.hpp"
#include "actions/fight/spells.hpp"
#include "player/item_database.hpp"
#include "player/item_use_registry.hpp"
#include "region.hpp"
#include "regions/location_registry.hpp"
#include "regions/entry_condition_registry.hpp"

#include <stdexcept>

namespace rpg {

class Game {
public:
  Game()
    : m_EnemyGenerator(), m_PlayerClassGenerator(), m_SpellDatabase(),
      m_ItemDatabase(RPG_ITEM_DATABASE_PATH, RPG_EQUIPMENT_DATABASE_PATH) {
    for (const Region& region : m_RegionWorld.regions()) {
      for (const RegionLocation& location : region.locations) {
        if (!m_RegionLocationRegistry.contains(location.function_id)) {
          throw std::runtime_error(
              "Region '" + region.id + "' references unknown location function '" +
              location.function_id + "'.");
        }
      }
      for (const RegionEntryCondition& condition : region.entry_conditions) {
        if (!m_RegionEntryConditionRegistry.contains(condition.function_id)) {
          throw std::runtime_error(
              "Region '" + region.id +
              "' references unknown entry-condition function '" +
              condition.function_id + "'.");
        }
      }
    }
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
  RegionLocationRegistry m_RegionLocationRegistry;
  RegionEntryConditionRegistry m_RegionEntryConditionRegistry;
  RegionWorld m_RegionWorld;
};
} // namespace rpg
