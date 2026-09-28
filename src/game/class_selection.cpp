#include "class_selection.hpp"

// local
#include "../player/player_classes/fighter.hpp"
#include "../player/player_classes/magician.hpp"

namespace rpg {

std::shared_ptr<Player> create_player_from_choice(char choice) {
  switch (choice) {
  case 'f':
    return std::make_shared<Fighter>();
  case 'm':
    return std::make_shared<Magician>();
  default:
    return std::make_shared<Fighter>();
  }
}

} // namespace rpg
