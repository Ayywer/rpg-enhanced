#pragma once

// local
#include "../player/player.hpp"
#include "../enemy/enemy.hpp"

// std
#include <cstdint>
#include <memory>

namespace rpg {

struct SpellResult {
  std::uint64_t heal = 0;
  std::uint64_t damage = 0;
};

SpellResult cast_heal(std::shared_ptr<Player> player);
SpellResult cast_damage_spell(std::shared_ptr<Player> player, std::shared_ptr<Enemy> enemy);

} // namespace rpg
