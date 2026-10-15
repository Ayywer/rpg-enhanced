#pragma once

// local
#include "../../player/player.hpp"
#include "../../player/player_class_generator.hpp"

// std
#include <memory>

namespace rpg {

std::shared_ptr<Player> create_player_from_choice(
	char choice, const PlayerClassGenerator& class_generator);

} // namespace rpg
