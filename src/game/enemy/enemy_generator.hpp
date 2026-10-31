#pragma once

#include "enemy.hpp"

#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace rpg {

class EnemyGenerator {
public:
  EnemyGenerator();
  explicit EnemyGenerator(const std::string& database_path);

  std::shared_ptr<Enemy> generate(std::uint64_t level,
                                  const std::string& path_id);

private:
  struct Stats {
    std::uint64_t health;
    std::uint64_t strength;
    std::uint64_t defense;
    std::uint64_t precision;
    std::uint64_t experience;
    std::uint64_t money;
    std::uint64_t gems;
  };

  struct Definition {
    std::string name;
    std::uint64_t weight;
    std::vector<std::string> paths;
    Stats stats;
    Stats per_level;
  };

  std::vector<Definition> m_Definitions;
  std::mt19937 m_RandomEngine;
};

} // namespace rpg