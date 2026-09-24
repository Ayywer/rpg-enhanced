#pragma once

// local
#include "../enemy.hpp"

// std
#include <cstdint>

namespace rpg {

class Goblin : public Enemy {
public:
  Goblin() {
    m_Strength = 10;
    m_Defense = 1;
    m_Exp = 1;
    m_Health = 100;
  }

  Goblin(std::uint64_t strength) {
    m_Strength = 10 * strength;
    m_Defense = 10 * strength;
    m_Exp = strength;
    m_Health = 100 * strength;
  }
};

} // namespace rpg
