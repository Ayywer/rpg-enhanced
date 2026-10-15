#pragma once

#include <iostream>
#include <limits>
#include <string>

namespace rpg {

inline void clear_console() {
  std::cout << "\033[2J\033[H" << std::flush;
}

inline void pause_for_enter() {
  std::cout << "\nPress Enter to continue..." << std::flush;
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  std::string line;
  std::getline(std::cin, line);
}

} // namespace rpg