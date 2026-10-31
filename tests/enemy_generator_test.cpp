#include "game/enemy/enemy_generator.hpp"

#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

int main() {
  rpg::EnemyGenerator generator(RPG_ENEMY_DATABASE_PATH);
  const std::vector<std::pair<std::string, std::string>> path_enemies = {
      {"bramble_track", "Goblin"},
      {"wagon_lane", "Orc"},
      {"snow_chasm", "Troll"},
  };
  for (const auto& [path_id, expected_name] : path_enemies) {
    const std::shared_ptr<rpg::Enemy> enemy = generator.generate(3, path_id);
    if (enemy->m_Name != expected_name) {
      std::cerr << path_id << " generated an ineligible enemy: "
                << enemy->m_Name << '\n';
      return 1;
    }
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
                    enemy->m_Defense == 2 && enemy->m_Precision == 62 &&
                    enemy->m_RewardExp == 9 &&
                    enemy->m_RewardMoney == 9;
    } else if (enemy->m_Name == "Orc") {
      stats_match = enemy->m_Health == 130 && enemy->m_Strength == 12 &&
                    enemy->m_Defense == 6 && enemy->m_Precision == 72 &&
                    enemy->m_RewardExp == 26 &&
                    enemy->m_RewardMoney == 20;
    } else if (enemy->m_Name == "Troll") {
      stats_match = enemy->m_Health == 250 && enemy->m_Strength == 15 &&
                    enemy->m_Defense == 9 && enemy->m_Precision == 82 &&
                    enemy->m_RewardExp == 52 &&
                    enemy->m_RewardMoney == 40;
    }

    if (!stats_match) {
      std::cerr << "Generated " << enemy->m_Name
                << " with stats that do not match the database\n";
      return 1;
    }
  }

  for (const auto& [path_id, allowed_first, allowed_second] :
       std::vector<std::tuple<std::string, std::string, std::string>>{
           {"shrine_road", "Goblin", "Orc"},
           {"cutting_pass", "Orc", "Troll"}}) {
    for (std::uint64_t attempt = 0; attempt < 100; ++attempt) {
      const std::string& generated = generator.generate(3, path_id)->m_Name;
      if (generated != allowed_first && generated != allowed_second) {
        std::cerr << path_id << " generated an ineligible enemy: " << generated
                  << '\n';
        return 1;
      }
    }
  }

  try {
    generator.generate(3, "unknown_path");
    std::cerr << "An unconfigured path should not generate an enemy\n";
    return 1;
  } catch (const std::invalid_argument&) {
  }

  return 0;
}