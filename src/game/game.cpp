// header
#include "game.hpp"

// local
#include "fight.hpp"
#include "class_selection.hpp"
#include "../enemy/enemy_classes/goblin.hpp"
#include "shop.hpp"

// std
#include <iostream>

namespace rpg {
void Game::setup() {
  char input;
  std::cout << "\n========================================\n"
            << "             CHARACTER SELECT\n"
            << "========================================\n"
            << "  [f] Fighter\n"
            << "  [m] Magician\n"
            << "Choose your class: ";
  std::cin >> input;
  m_pPlayer = create_player_from_choice(input);
}

void Game::loop() {
  while (m_pPlayer->m_Health > 0) {
    char input;
    std::cout << "\n----------------------------------------\n"
              << "                 MENU\n"
              << "----------------------------------------\n"
              << "  [f] Fight\n"
              << "  [s] Shop\n"
              << "Choose an action: ";
    std::cin >> input;
    if (input == 'f') {
      Fight fight(m_pPlayer, std::make_shared<Goblin>());
      fight.loop();
    } else if (input == 's') {
      Shop shop(m_pPlayer);
      shop.loop();
    }
  }
}
} // namespace rpg
