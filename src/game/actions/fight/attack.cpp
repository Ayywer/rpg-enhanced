#include "attack.hpp"

// local
#include "../../enemy/enemy.hpp"
#include "../../player/passive_ability_registry.hpp"
#include "../../player/player.hpp"

#include <array>

namespace rpg {
namespace {

std::uint64_t apply_passive_abilities(
  const Player& owner, const Enemy& opponent, std::uint64_t raw_damage,
  BodyPart body_part, PassiveAbilityTrigger trigger) {
  constexpr std::array slots{
    EquipmentSlot::HELMET, EquipmentSlot::CHESTPLATE, EquipmentSlot::BOOTS,
    EquipmentSlot::NECKLACE, EquipmentSlot::RING, EquipmentSlot::WEAPON,
    EquipmentSlot::SHIELD};
  for (const EquipmentSlot slot : slots) {
  const auto equipped = owner.m_Equipment.find(slot);
  if (equipped == owner.m_Equipment.end()) {
    continue;
  }
  const Equipment& equipment = equipped->second;
  if (!equipment.m_Item.passive_ability.empty()) {
    raw_damage = PassiveAbilityRegistry::apply(
      equipment.m_Item, owner, opponent, raw_damage, body_part, trigger);
  }
  }
  return raw_damage;
}

} // namespace

std::uint64_t Player::attack(std::shared_ptr<Enemy> enemy,
                             std::uint64_t attack_damage, BodyPart body_part) {
  const std::uint64_t base_damage =
      (this->m_Strength + this->weapon_strength() +
       this->active_relic_strength()) * attack_damage;
  const std::uint64_t raw_damage = apply_passive_abilities(
    *this, *enemy, base_damage, body_part, PassiveAbilityTrigger::ATTACK);
  const std::uint64_t defense =
      enemy->m_Defense + enemy->m_Equipment[body_part].m_Protection;
  const std::uint64_t damage = raw_damage > defense ? raw_damage - defense : 0;

  enemy->m_Health -= damage;
  return damage;
}

std::uint64_t Enemy::attack(std::shared_ptr<Player> player,
                            std::uint64_t attack_damage, BodyPart body_part) {
  const std::uint64_t base_damage = this->m_Strength * attack_damage;
  const std::uint64_t raw_damage = apply_passive_abilities(
      *player, *this, base_damage, body_part, PassiveAbilityTrigger::DEFENSE);
  const std::uint64_t defense =
      player->m_Defense + player->equipment_protection(body_part) +
      player->active_relic_defense();
  const std::uint64_t damage = raw_damage > defense ? raw_damage - defense : 0;

  player->m_Health -= damage;
  return damage;
}

std::uint64_t perform_attack(std::shared_ptr<Player> attacker,
                             std::shared_ptr<Enemy> target,
                             std::uint64_t attack_damage, BodyPart body_part) {
  return attacker->attack(target, attack_damage, body_part);
}

std::uint64_t perform_enemy_attack(std::shared_ptr<Enemy> attacker,
                                   std::shared_ptr<Player> target,
                                   std::uint64_t attack_damage,
                                   BodyPart body_part) {
  return attacker->attack(target, attack_damage, body_part);
}

} // namespace rpg
