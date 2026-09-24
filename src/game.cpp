// header
#include "game.hpp"

// local
#include "fight.hpp"
#include "enemy_classes/goblin.hpp"
#include "player_classes/fighter.hpp"
#include "player_classes/magician.hpp"
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
  switch (input) {
  case 'f':
    m_pPlayer = std::make_shared<Fighter>();
    break;

  case 'm':
    m_pPlayer = std::make_shared<Magician>();
    break;

  default:
    m_pPlayer = std::make_shared<Fighter>();
    break;
  }
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
