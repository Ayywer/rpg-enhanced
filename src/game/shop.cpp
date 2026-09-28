// header
#include "shop.hpp"

namespace rpg {
void Shop::loop() {
  char input = 0;
  while (input != 'q') {
    BodyPart body_part;
    std::uint64_t armor_strength;
    std::cout << "\n----------------------------------------\n"
              << "                 ARMOR SHOP\n"
              << "----------------------------------------\n"
              << "  Your gold: " << m_pPlayer->m_Money << '\n'
              << "  [b] Boots\n"
              << "  [c] Chest\n"
              << "  [h] Head\n"
              << "  [q] Leave shop\n"
              << "Choose an option: ";
    std::cin >> input;
    switch (input) {
    case 'b':
      body_part = BodyPart::BOOTS;
      break;

    case 'c':
      body_part = BodyPart::CHEST;
      break;

    case 'h':
      body_part = BodyPart::HEAD;
      break;

    default:
      break;
    }

    if (input != 'q') {
      std::cout << "Enter armor protection value: ";
      std::cin >> armor_strength;

      if (armor_strength <= m_pPlayer->m_Money) {
        m_pPlayer->m_Equipment[body_part] = Equipment(armor_strength);
        m_pPlayer->m_Money -= armor_strength;
        std::cout << "Armor purchased successfully.\n";
      } else {
        std::cout << "You do not have enough gold for that armor.\n";
      }
    }
  }
}

} // namespace rpg