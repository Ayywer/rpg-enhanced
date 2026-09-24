#pragma once

// local
#include "equipment.hpp"

// std
#include <cstdint>
#include <memory>
#include <unordered_map>

namespace rpg {

class Player;

class Enemy {
public:
  virtual ~Enemy() = default;

  virtual std::uint64_t attack(std::shared_ptr<Player> player,
                               std::uint64_t attack_damage,
                               BodyPart body_part);

  std::uint64_t m_Strength;
  std::uint64_t m_Defense;
  std::uint64_t m_Exp;
  std::int64_t m_Health;
  std::uint64_t m_Money;
  std::unordered_map<BodyPart, Equipment> m_Equipment{};
};

} // namespace rpg
