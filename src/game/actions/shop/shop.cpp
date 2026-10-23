// header
#include "shop.hpp"

#include "../../console.hpp"

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
    std::vector<std::string> choices;
    std::vector<const ItemDefinition*> stock;
    stock.reserve(m_ItemDatabase.items().size() +
                  m_ItemDatabase.equipment().size());
    for (const ItemDefinition& definition : m_ItemDatabase.items()) {
      stock.push_back(&definition);
      choices.push_back("Item  /  " + definition.item.name + "  /  " +
                        std::to_string(definition.price) + " gold");
    }
    for (const EquipmentDefinition& definition : m_ItemDatabase.equipment()) {
      stock.push_back(&definition);
      choices.push_back("Equipment  /  " + definition.item.name + "  /  " +
                        std::to_string(definition.price) + " gold");
    }
    choices.push_back("Return to the road");
    const int selected = ui::choose_option("Smith and provisions", choices, wallet);
    if (selected < 0 || selected >= static_cast<int>(stock.size())) {
      return;
    }
    const ItemDefinition& definition = *stock[static_cast<std::size_t>(selected)];
    const std::uint64_t price = definition.price;
    const Item& item = definition.item;
    if (price > m_pPlayer->m_Money) {
      ui::notify("Not enough gold", "You need " + std::to_string(price) +
                                      " gold, but have " +
                                      std::to_string(m_pPlayer->m_Money) + ".",
                 ftxui::Color::RGB(224, 95, 91));
      continue;
    }
    if (m_pPlayer->add_item(item) == 0) {
      ui::notify("Inventory full", "That item stack is already at its limit.",
                 ftxui::Color::RGB(224, 95, 91));
      continue;
    }
    m_pPlayer->m_Money -= price;
    ui::notify("Purchased", item.name + " added to inventory. Remaining gold: " +
                                      std::to_string(m_pPlayer->m_Money) + ".");
  }
}

} // namespace rpg