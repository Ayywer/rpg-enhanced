#pragma once

#include "player.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace rpg {

struct PlayerClassChoice {
  char key;
  std::string name;
};

class PlayerClassGenerator {
public:
  PlayerClassGenerator();
  explicit PlayerClassGenerator(const std::string& database_path);

  const std::vector<PlayerClassChoice>& choices() const;
  std::shared_ptr<Player> create(char key) const;

private:
  struct Stats {
    std::uint64_t health;
    std::uint64_t strength;
    std::uint64_t intelligence;
    std::uint64_t defense;
    std::uint64_t precision;
    std::uint64_t mana;
    std::uint64_t max_mana;
    std::uint64_t mana_regeneration;
    std::uint64_t experience;
    std::uint64_t max_experience;
    std::uint64_t level;
    std::uint64_t money;
    std::uint64_t gems;
  };

  struct Definition {
    PlayerClassChoice choice;
    Stats stats;
  };

  std::vector<Definition> m_Definitions;
  std::vector<PlayerClassChoice> m_Choices;
};

} // namespace rpg