#pragma once

// local
#include "../equipment.hpp"

// std
#include <cstdint>
#include <memory>
#include <unordered_map>

namespace rpg {

class Enemy;

class Player {
public:
  virtual ~Player() = default;

  virtual std::uint64_t attack(std::shared_ptr<Enemy> enemy, std::uint64_t attack_damage, BodyPart body_part);
  
  std::int64_t m_Health;
  std::uint64_t m_Defense;

  std::uint64_t m_Strength;
  std::uint64_t m_Mana = 0;
  std::uint64_t m_MaxMana = 0;
  std::uint64_t m_ManaRegenerationMultiplier = 1;

  std::uint64_t m_Exp = 0;
  std::uint64_t m_MaxExp = 10;
  std::uint64_t m_Level = 1;

  std::uint64_t m_Money;
  std::uint64_t m_Gems;
  std::unordered_map<BodyPart, Equipment> m_Equipment{};
};

} // namespace rpg
