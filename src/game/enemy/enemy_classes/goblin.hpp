#pragma once

// local
#include "../enemy.hpp"

// std
#include <cstdint>

namespace rpg {

class Goblin : public Enemy {
public:
  Goblin() {
    m_Health = 50;
    m_Strength = 2;
    m_Defense = 1;
    m_RewardExp = 5;
    m_RewardMoney = 2;
    m_RewardGems = 0;
  }
};

} // namespace rpg
