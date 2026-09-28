#pragma once

// local
#include "../player/player.hpp"

namespace rpg {

void apply_level_progress(std::shared_ptr<Player> player, std::uint64_t exp_gain);

} // namespace rpg
