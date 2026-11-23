#include "class_selection.hpp"

namespace rpg {

std::shared_ptr<Player> create_player_from_choice(
  const std::string& choice, const PlayerClassGenerator& class_generator) {
  return class_generator.create(choice);
}

} // namespace rpg
