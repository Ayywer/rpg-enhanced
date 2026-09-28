#pragma once

// local
#include "../equipment.hpp"

// std
#include <cstdint>
#include <memory>
#include <unordered_map>

namespace rpg {

class Player;

class Enemy {
public:
  virtual ~Enemy() = default;

  virtual std::uint64_t attack(std::shared_ptr<Player> player, std::uint64_t attack_damage, BodyPart body_part);

  std::int64_t m_Health;
  std::uint64_t m_Defense;
  
  std::uint64_t m_Strength;
  std::uint64_t m_Mana = 0;
  
  std::uint64_t m_Level = 1;

  std::uint64_t m_RewardMoney;
  std::uint64_t m_RewardGems;
  std::uint64_t m_RewardExp;
  
  std::unordered_map<BodyPart, Equipment> m_Equipment{};
};

} // namespace rpg
