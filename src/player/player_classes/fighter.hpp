#pragma once

// local
#include "../player.hpp"

// std
#include <cstdint>

namespace rpg {

class Fighter : public Player {
public:
  Fighter() {
    m_Health = 100;
    m_Strength = 10;
    m_Defense = 1;
    m_Mana = 0;
    m_MaxMana = 10;
    m_ManaRegenerationMultiplier = 1;
    m_Exp = 0;
    m_MaxExp = 10;
    m_Level = 1;
  }
};

} // namespace rpg
