#include "game/leveling.hpp"

#include <iostream>
#include <memory>

namespace {

std::shared_ptr<rpg::Player> make_player() {
  auto player = std::make_shared<rpg::Player>();
  player->m_Health = 40;
  player->m_MaxHealth = 100;
  player->m_Strength = 10;
  player->m_Intelligence = 10;
  player->m_Defense = 5;
  player->m_Precision = 75;
  player->m_Mana = 4;
  player->m_MaxMana = 20;
  player->m_ManaRegenerationMultiplier = 2;
  player->m_MaxExp = 10;
  player->m_Money = 7;
  player->m_Gems = 3;
  return player;
}

} // namespace

int main() {
  auto player = make_player();
  rpg::apply_level_progress(player, 9);
  if (player->m_Level != 1 || player->m_Health != 40 ||
      player->m_Strength != 10) {
    std::cerr << "Stats should not change before reaching the next level\n";
    return 1;
  }

  rpg::apply_level_progress(player, 1);
  if (player->m_Level != 2 || player->m_Exp != 0 || player->m_MaxExp != 15 ||
      player->m_Health != 48 || player->m_MaxHealth != 120 ||
      player->m_Strength != 12 || player->m_Defense != 6 ||
      player->m_Intelligence != 12 ||
      player->m_Precision != 90 ||
      player->m_Mana != 4 || player->m_MaxMana != 24 ||
      player->m_ManaRegenerationMultiplier != 2) {
    std::cerr << "Stats should increase by 20 percent after one level-up\n";
    return 1;
  }

  player = make_player();
  rpg::apply_level_progress(player, 30);
  if (player->m_Level != 3 || player->m_Exp != 5 || player->m_MaxExp != 22 ||
      player->m_Health != 57 || player->m_MaxHealth != 144 ||
      player->m_Strength != 14 || player->m_Defense != 7 ||
      player->m_Intelligence != 14 ||
      player->m_Precision != 100 ||
      player->m_Mana != 4 || player->m_MaxMana != 28 ||
      player->m_ManaRegenerationMultiplier != 2) {
    std::cerr << "Stats should grow moderately across multiple levels\n";
    return 1;
  }
  if (player->m_Money != 7 || player->m_Gems != 3) {
    std::cerr << "Leveling should not change currency\n";
    return 1;
  }

  return 0;
}
