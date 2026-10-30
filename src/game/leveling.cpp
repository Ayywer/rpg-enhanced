#include "leveling.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>

namespace rpg {
namespace {

std::uint64_t increase_stat(std::uint64_t value, const char* name) {
  const std::uint64_t increase = value / 5;
  if (increase > std::numeric_limits<std::uint64_t>::max() - value) {
    throw std::overflow_error(std::string("Player stat overflows: ") + name);
  }
  return value + increase;
}

std::int64_t increase_health(std::int64_t value) {
  if (value <= 0) {
    return value;
  }
  const std::int64_t increase = value / 5;
  if (increase > std::numeric_limits<std::int64_t>::max() - value) {
    throw std::overflow_error("Player health overflows");
  }
  return value + increase;
}

} // namespace

void apply_level_progress(std::shared_ptr<Player> player, std::uint64_t exp_gain) {
  if (exp_gain > std::numeric_limits<std::uint64_t>::max() - player->m_Exp) {
    throw std::overflow_error("Player experience overflows");
  }
  player->m_Exp += exp_gain;

  while (player->m_Exp >= player->m_MaxExp) {
    const std::uint64_t exp_increase = player->m_MaxExp / 2;
    if (player->m_Level == std::numeric_limits<std::uint64_t>::max() ||
        exp_increase > std::numeric_limits<std::uint64_t>::max() -
                           player->m_MaxExp) {
      throw std::overflow_error("Player level progression overflows");
    }

    const std::int64_t health = increase_health(player->m_Health);
    const std::uint64_t max_health = increase_stat(player->m_MaxHealth, "max health");
    const std::uint64_t strength = increase_stat(player->m_Strength, "strength");
    const std::uint64_t intelligence =
      increase_stat(player->m_Intelligence, "intelligence");
    const std::uint64_t defense = increase_stat(player->m_Defense, "defense");
    const std::uint64_t precision =
      std::min<std::uint64_t>(100, increase_stat(player->m_Precision, "precision"));
    const std::uint64_t mana = increase_stat(player->m_Mana, "mana");
    const std::uint64_t max_mana = increase_stat(player->m_MaxMana, "max mana");

    player->m_Exp -= player->m_MaxExp;
    ++player->m_Level;
    player->m_MaxExp += exp_increase;
    player->m_Health = health;
    player->m_MaxHealth = max_health;
    player->m_Strength = strength;
    player->m_Intelligence = intelligence;
    player->m_Defense = defense;
    player->m_Precision = precision;
    player->m_Mana = mana;
    player->m_MaxMana = max_mana;
  }
}

} // namespace rpg
