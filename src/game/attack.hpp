#pragma once

// local
#include "../equipment.hpp"
#include "../enemy/enemy.hpp"
#include "../player/player.hpp"

// std
#include <cstdint>
#include <memory>

namespace rpg {

std::uint64_t perform_attack(std::shared_ptr<Player> attacker,
                             std::shared_ptr<Enemy> target,
                             std::uint64_t attack_damage,
                             BodyPart body_part);

std::uint64_t perform_enemy_attack(std::shared_ptr<Enemy> attacker,
                                  std::shared_ptr<Player> target,
                                  std::uint64_t attack_damage,
                                  BodyPart body_part);

} // namespace rpg
