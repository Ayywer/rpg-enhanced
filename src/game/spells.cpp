#include "spells.hpp"

namespace rpg {

SpellResult cast_heal(std::shared_ptr<Player> player) {
  SpellResult result;

  if (player->m_Mana < 1) {
    return result;
  }

  player->m_Mana -= 1;
  result.heal = 10;
  player->m_Health += result.heal;
  return result;
}

SpellResult cast_damage_spell(std::shared_ptr<Player> player, std::shared_ptr<Enemy> enemy) {
  SpellResult result;

  if (player->m_Mana < 2) {
    return result;
  }

  player->m_Mana -= 2;
  result.damage = 10;
  enemy->m_Health -= result.damage;
  return result;
}

} // namespace rpg
