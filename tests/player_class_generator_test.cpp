#include "game/player/player_class_generator.hpp"

#include <iostream>
#include <memory>

int main() {
  rpg::PlayerClassGenerator generator(RPG_PLAYER_CLASS_DATABASE_PATH);
  const auto& choices = generator.choices();
  if (choices.size() != 2 || choices[0].key != 'f' ||
      choices[0].name != "Fighter" || choices[1].key != 'm' ||
      choices[1].name != "Magician") {
    std::cerr << "Player class choices do not match the database\n";
    return 1;
  }

  const std::shared_ptr<rpg::Player> fighter = generator.create('f');
  if (!fighter || fighter->m_ClassName != "Fighter" ||
      fighter->m_Health != 100 || fighter->m_MaxHealth != 100 ||
      fighter->m_Strength != 10 || fighter->m_Intelligence != 5 ||
      fighter->m_Precision != 95 ||
      fighter->m_Mana != 5 ||
      fighter->m_MaxMana != 5 || fighter->m_ManaRegenerationMultiplier != 1 ||
      fighter->m_Money != 0 || fighter->m_Gems != 0) {
    std::cerr << "Fighter stats do not match the database\n";
    return 1;
  }

  const std::shared_ptr<rpg::Player> magician = generator.create('m');
  if (!magician || magician->m_ClassName != "Magician" ||
      magician->m_Health != 50 || magician->m_MaxHealth != 50 ||
      magician->m_Strength != 5 || magician->m_Intelligence != 10 ||
      magician->m_Precision != 85 ||
      magician->m_Mana != 25 ||
      magician->m_MaxMana != 25 || magician->m_ManaRegenerationMultiplier != 2) {
    std::cerr << "Magician stats do not match the database\n";
    return 1;
  }

  if (generator.create('?')) {
    std::cerr << "Unknown class key should not create a player\n";
    return 1;
  }
  return 0;
}