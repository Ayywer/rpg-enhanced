#pragma once

// local
#include "../../player/equipment.hpp"
#include "../../player/player.hpp"
#include "../../enemy/enemy.hpp"
#include "fight_actions.hpp"

// std
#include <cstdint>
#include <memory>
#include <string>

namespace rpg {

class Fight {
public:
  Fight(std::shared_ptr<Player> player, std::shared_ptr<Enemy> enemy);

  void loop();

  void register_action(char key, std::string label,
                       FightActionHandler handler);

  std::uint64_t PlayerHit(std::shared_ptr<Player> who,
                          std::shared_ptr<Enemy> whom,
                          std::uint64_t attack_damage, BodyPart body_part);

  std::uint64_t EnemyHit(std::shared_ptr<Enemy> who,
                         std::shared_ptr<Player> whom,
                         std::uint64_t attack_damage, BodyPart body_part);

private:
  std::shared_ptr<Enemy> m_pEnemy;
  std::shared_ptr<Player> m_pPlayer;
  FightActions m_Actions;

  std::uint64_t m_ExpReward;
  std::uint64_t m_MoneyReward;
  std::uint64_t m_GemsReward;
};
} // namespace rpg
