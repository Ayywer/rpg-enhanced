#include "attack.hpp"

// local
#include "../enemy/enemy.hpp"
#include "../player/player.hpp"

namespace rpg {

std::uint64_t Player::attack(std::shared_ptr<Enemy> enemy,
                             std::uint64_t attack_damage,
                             BodyPart body_part) {
  const std::uint64_t raw_damage = this->m_Strength * attack_damage;
  const std::uint64_t defense = enemy->m_Defense + enemy->m_Equipment[body_part].m_Protection;
  const std::uint64_t damage = raw_damage > defense ? raw_damage - defense : 0;

  enemy->m_Health -= damage;
  return damage;
}

std::uint64_t Enemy::attack(std::shared_ptr<Player> player,
                           std::uint64_t attack_damage,
                           BodyPart body_part) {
  const std::uint64_t raw_damage = this->m_Strength * attack_damage;
  const std::uint64_t defense = player->m_Defense + player->m_Equipment[body_part].m_Protection;
  const std::uint64_t damage = raw_damage > defense ? raw_damage - defense : 0;

  player->m_Health -= damage;
  return damage;
}

std::uint64_t perform_attack(std::shared_ptr<Player> attacker,
                             std::shared_ptr<Enemy> target,
                             std::uint64_t attack_damage,
                             BodyPart body_part) {
  return attacker->attack(target, attack_damage, body_part);
}

std::uint64_t perform_enemy_attack(std::shared_ptr<Enemy> attacker,
                                  std::shared_ptr<Player> target,
                                  std::uint64_t attack_damage,
                                  BodyPart body_part) {
  return attacker->attack(target, attack_damage, body_part);
}

} // namespace rpg
