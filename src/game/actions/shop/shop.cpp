// header
#include "shop.hpp"

#include "../../console.hpp"

#include <charconv>
#include <string>

namespace rpg {
void Shop::loop() {
  while (true) {
    const auto wallet = ui::panel(
        "YOUR WALLET",
        ftxui::vbox({
            ftxui::text("GOLD  " + std::to_string(m_pPlayer->m_Money)) |
                ftxui::bold | ftxui::color(ftxui::Color::Gold1),
            ftxui::text("GEMS  " + std::to_string(m_pPlayer->m_Gems)) |
                ftxui::color(ftxui::Color::RedLight),
        }),
        ftxui::Color::RGB(232, 194, 111));
    const int selected = ui::choose_option(
        "Armor smith", {"Head", "Chest", "Boots", "Return to the road"},
        wallet);
    if (selected < 0 || selected == 3) {
      return;
    }

    const BodyPart body_part = selected == 0   ? BodyPart::HEAD
                               : selected == 1 ? BodyPart::CHEST
                                               : BodyPart::BOOTS;
    const std::string value = ui::input_text(
        "Armor reinforcement",
        "Protection value (paid in gold)");
    std::uint64_t armor_strength = 0;
    const char* begin = value.data();
    const char* end = begin + value.size();
    const auto [parsed_end, error] = std::from_chars(begin, end, armor_strength);
    if (value.empty() || error != std::errc{} || parsed_end != end) {
      ui::notify("Invalid amount", "Enter a whole number for the protection value.", ftxui::Color::RGB(232, 166, 76));
      continue;
    }

    if (armor_strength > m_pPlayer->m_Money) {
      ui::notify("Not enough gold", "You need " +
                                      std::to_string(armor_strength) +
                                      " gold, but have " +
                                      std::to_string(m_pPlayer->m_Money) + ".",
                 ftxui::Color::RGB(224, 95, 91));
      continue;
    }

    m_pPlayer->m_Equipment[body_part] = Equipment(armor_strength);
    m_pPlayer->m_Money -= armor_strength;
    ui::notify("Armor improved", "Protection " +
                                      std::to_string(armor_strength) +
                                      " added. Remaining gold: " +
                                      std::to_string(m_pPlayer->m_Money) + ".");
  }
}

} // namespace rpg