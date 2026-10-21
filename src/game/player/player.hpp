#pragma once

// local
#include "equipment.hpp"

// std
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace rpg {

class Enemy;

class Player {
public:
  virtual ~Player() = default;

  virtual std::uint64_t attack(std::shared_ptr<Enemy> enemy, std::uint64_t attack_damage, BodyPart body_part);

  std::string m_ClassName;
  std::vector<std::string> m_KnownSpells;

  bool knows_spell(const std::string& spell_id) const {
    for (const std::string& known_spell : m_KnownSpells) {
      if (known_spell == spell_id) {
        return true;
      }
    }
    return false;
  }

  bool learn_spell(const std::string& spell_id) {
    if (spell_id.empty() || knows_spell(spell_id)) {
      return false;
    }
    m_KnownSpells.push_back(spell_id);
    return true;
  }
  
  std::int64_t m_Health;
  std::uint64_t m_MaxHealth = 0;
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
