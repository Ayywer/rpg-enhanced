#include "game/enemy/enemy_generator.hpp"

#include <cstdint>
#include <iostream>
#include <memory>

int main() {
  rpg::EnemyGenerator generator(RPG_ENEMY_DATABASE_PATH);
  for (std::uint64_t attempt = 0; attempt < 100; ++attempt) {
    const std::shared_ptr<rpg::Enemy> enemy = generator.generate(3);
    if (enemy->m_Level != 3) {
      std::cerr << "Generated enemy has the wrong level\n";
      return 1;
    }
    if (enemy->m_MaxHealth != static_cast<std::uint64_t>(enemy->m_Health)) {
      std::cerr << "Generated enemy has an incorrect maximum health value\n";
      return 1;
    }

    bool stats_match = false;
    if (enemy->m_Name == "Goblin") {
      stats_match = enemy->m_Health == 58 && enemy->m_Strength == 6 &&
                    enemy->m_Defense == 2 && enemy->m_RewardExp == 9 &&
                    enemy->m_RewardMoney == 9;
    } else if (enemy->m_Name == "Orc") {
      stats_match = enemy->m_Health == 130 && enemy->m_Strength == 12 &&
                    enemy->m_Defense == 6 && enemy->m_RewardExp == 26 &&
                    enemy->m_RewardMoney == 20;
    } else if (enemy->m_Name == "Troll") {
      stats_match = enemy->m_Health == 250 && enemy->m_Strength == 15 &&
                    enemy->m_Defense == 9 && enemy->m_RewardExp == 52 &&
                    enemy->m_RewardMoney == 40;
    }

    if (!stats_match) {
      std::cerr << "Generated " << enemy->m_Name
                << " with stats that do not match the database\n";
      return 1;
    }
  }
  return 0;
}