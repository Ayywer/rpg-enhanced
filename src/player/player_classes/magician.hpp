#pragma once

// local
#include "../player.hpp"

// std
#include <cstdint>

namespace rpg {

class Magician : public Player {
public:
  Magician() {
    m_Health = 100;
    m_Strength = 5;
    m_Defense = 1;
    m_Mana = 0;
    m_MaxMana = 25;
    m_ManaRegenerationMultiplier = 2;
    m_Exp = 0;
    m_MaxExp = 10;
    m_Level = 1;
  }
};

} // namespace rpg
