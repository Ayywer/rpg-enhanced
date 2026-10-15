// header
#include "game.hpp"

// local
#include "console.hpp"
#include "actions/fight/fight.hpp"
#include "actions/player/class_selection.hpp"
#include "actions/shop/shop.hpp"

// std
#include <iostream>

namespace rpg {
void Game::setup() {
  clear_console();
  while (!m_pPlayer) {
    char input;
    std::cout << "\n========================================\n"
              << "             CHARACTER SELECT\n"
              << "========================================\n";
    for (const PlayerClassChoice& choice : m_PlayerClassGenerator.choices()) {
      std::cout << "  [" << choice.key << "] " << choice.name << '\n';
    }
    std::cout << "Choose your class: ";
    std::cin >> input;
    m_pPlayer = create_player_from_choice(input, m_PlayerClassGenerator);
    if (!m_pPlayer) {
      std::cout << "Unknown class. Try again.\n";
      pause_for_enter();
      clear_console();
    }
  }
}

void Game::loop() {
  while (m_pPlayer->m_Health > 0) {
    clear_console();
    char input;
    std::cout << "\n----------------------------------------\n"
              << "                 MENU\n"
              << "----------------------------------------\n"
              << "  [f] Fight\n"
              << "  [s] Shop\n"
              << "Choose an action: ";
    std::cin >> input;
    if (input == 'f') {
      clear_console();
      Fight fight(m_pPlayer, m_EnemyGenerator.generate(m_pPlayer->m_Level));
      fight.loop();
    } else if (input == 's') {
      Shop shop(m_pPlayer);
      shop.loop();
    } else {
      std::cout << "Unknown action.\n";
      pause_for_enter();
    }
  }
}
} // namespace rpg
