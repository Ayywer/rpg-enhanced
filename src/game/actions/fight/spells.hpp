#pragma once

// local
#include "../../player/player.hpp"
#include "../../enemy/enemy.hpp"

// std
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace rpg {

enum class SpellEffect {
  HEAL,
  DAMAGE,
};

struct SpellDefinition {
  std::string id;
  std::string name;
  std::string description;
  SpellEffect effect;
  std::uint64_t mana_cost;
  std::uint64_t power;
  std::uint64_t success_chance;
  std::unordered_map<std::string, std::uint64_t> class_unlock_levels;
};

struct SpellResult {
  bool not_known = false;
  bool insufficient_mana = false;
  bool succeeded = false;
  std::uint64_t mana_spent = 0;
  std::uint64_t heal = 0;
  std::uint64_t damage = 0;
};

class SpellDatabase {
public:
  SpellDatabase();
  explicit SpellDatabase(const std::string& database_path);

  std::vector<std::string> learn_available(Player& player) const;
  bool learn(Player& player, const std::string& spell_id) const;
  std::vector<const SpellDefinition*> known_spells(
      const Player& player) const;
  const SpellDefinition* find(const std::string& spell_id) const;
  SpellResult cast(Player& player, Enemy& enemy,
                   const SpellDefinition& spell, bool succeeds) const;

private:
  std::vector<SpellDefinition> m_Spells;
};

} // namespace rpg
