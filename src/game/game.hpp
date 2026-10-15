#pragma once

// local
#include "player/player.hpp"
#include "player/player_class_generator.hpp"
#include "enemy/enemy_generator.hpp"

namespace rpg {

class Game {
public:
  Game() : m_EnemyGenerator(), m_PlayerClassGenerator() { setup(); }

  void setup();
  void loop();

private:
  std::shared_ptr<Player> m_pPlayer;
  EnemyGenerator m_EnemyGenerator;
  PlayerClassGenerator m_PlayerClassGenerator;
};
} // namespace rpg
