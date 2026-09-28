#pragma once

// local
#include "equipment.hpp"
#include "../player/player.hpp"
#include "../player/player_classes/player_classes.hpp"
#include "../enemy/enemy.hpp"
#include "../enemy/enemy_classes/enemy_classes.hpp"
#include "attack.hpp"
#include "spells.hpp"
#include "leveling.hpp"
// std
#include <cstdint>
#include <iostream>
#include <random>

namespace rpg {

class Fight {
public:
  Fight(std::shared_ptr<Player> player, std::shared_ptr<Enemy> enemy);

  void loop();

  std::uint64_t PlayerHit(std::shared_ptr<Player> who,
                          std::shared_ptr<Enemy> whom,
                          std::uint64_t attack_damage, BodyPart body_part);

  std::uint64_t EnemyHit(std::shared_ptr<Enemy> who,
                         std::shared_ptr<Player> whom,
                         std::uint64_t attack_damage, BodyPart body_part);

private:
  std::shared_ptr<Enemy> m_pEnemy;
  std::shared_ptr<Player> m_pPlayer;

  std::uint64_t m_ExpReward;
  std::uint64_t m_MoneyReward;
  std::uint64_t m_GemsReward;
};
} // namespace rpg
