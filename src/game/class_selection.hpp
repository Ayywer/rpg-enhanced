#pragma once

// local
#include "../player/player.hpp"

// std
#include <memory>

namespace rpg {

std::shared_ptr<Player> create_player_from_choice(char choice);

} // namespace rpg
